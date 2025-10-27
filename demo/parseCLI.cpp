/* Copyright (c) 2025 unixdev73@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "parseCLI.hpp"
#include <unordered_map>
#include <iostream>
#include <string>

#define MCR_ERR(msg, code)                                                     \
  do {                                                                         \
    std::cerr << "Error: " << __func__ << ": " << msg << std::endl;            \
    return code;                                                               \
  } while (0)

namespace demo {
template <typename T> struct ArgInfoT {
  char shortForm{};
  T *value{};
};

struct ArgDatabaseT {
  std::unordered_map<std::string, ArgInfoT<std::string>> strOpts{};
  std::unordered_map<std::string, ArgInfoT<uint32_t>> numOpts{};
  std::unordered_map<std::string, ArgInfoT<bool>> flags{};
};

template <typename T, typename F>
void registerArgs(ap::UniqueArgParser &p, F &&func, T const &args) {
  for (auto const &[lf, info] : args)
    func(p.get(), lf, info.shortForm);
}

void populateDatabase(ap::UniqueArgParser &p, ArgDatabaseT *db,
                      AppCLI *const data) {
  db->flags.emplace("debug", ArgInfoT{'d', &data->debug});
  registerArgs(p, ap::addFlag, db->flags);

  db->numOpts.emplace("width", ArgInfoT{'w', &data->width});
  db->numOpts.emplace("height", ArgInfoT{'h', &data->height});
  registerArgs(p, ap::addOption, db->numOpts);

  registerArgs(p, ap::addOption, db->strOpts);
}

void printValues(ArgDatabaseT *const db) {
  std::cout << "Listing command line argument parsing summary:\n";
  for (auto const &[id, info] : db->flags)
    std::cout << "\tID: '" << id << "', value: '" << std::boolalpha
              << *info.value << "'\n";

  for (auto const &[id, info] : db->strOpts)
    std::cout << "\tID: '" << id << "', value: '" << *info.value << "'\n";

  for (auto const &[id, info] : db->numOpts)
    std::cout << "\tID: '" << id << "', value: '" << *info.value << "'\n";
}

int readStr(ap::UniqueArgParser &p, std::string const &opt, std::string *out) {
  try {
    std::vector<std::string> vals{};

    if (auto error = ap::getOptionValues(p.get(), opt, &vals); error) {
      std::string const err = std::to_string(error);
      MCR_ERR("Failed to get option values with error code: " + err, error);
    }

    if (!vals.size())
      return 0;

    *out = vals.front();
  } catch (...) {
    MCR_ERR("Failed to read " + opt + " option value.", -1);
  }

  return 0;
}

int readUint32(ap::UniqueArgParser &p, std::string const &opt, uint32_t *out) {
  std::string tmp{};
  try {
    if (auto error = readStr(p, opt, &tmp); error) {
      std::string const err = std::to_string(error);
      MCR_ERR("Failed to get option values with error code: " + err, 1);
    }

    if (tmp.empty())
      return 0;

    *out = std::stoul(tmp);
  } catch (...) {
    MCR_ERR("Failed to convert: '" << tmp << "' to a number.", 1);
  }

  return 0;
}

int readValues(ap::UniqueArgParser &p, ArgDatabaseT *db) {
  for (auto const &[id, info] : db->flags) {
    std::size_t count{};
    if (auto error = ap::getFlagOccurrence(p.get(), id, &count); error)
      MCR_ERR("Failed to get flag occurrence: " + id, 1);
    *info.value = static_cast<bool>(count);
  }

  for (auto const &[id, info] : db->numOpts)
    if (auto error = readUint32(p, id, info.value); error)
      return 2;

  for (auto const &[id, info] : db->strOpts)
    if (auto error = readStr(p, id, info.value); error)
      return 3;

  return 0;
}

int parseCLI(AppCLI *const data, ap::InputBinding *const binding) {
  if (!data || !binding)
    MCR_ERR("One of the input parameters is a nullptr.", 1);

  auto p = ap::createArgParser();
  if (!p)
    MCR_ERR("Creating an arg parser instance failed.", 2);

  ArgDatabaseT db{};
  populateDatabase(p, &db, data);

  std::size_t errPos{};
  if (auto error = ap::parse(p.get(), binding, &errPos); error) {
    std::vector<std::string> fv{};

    if (error == ap::Result::ErrorOptionHasNoValue) {
      std::string const option = binding->input[errPos];
      MCR_ERR("The option: '" + option + "' requires a value.", 3);
    }

    if (ap::getFreeValues(p.get(), &fv); fv.size())
      MCR_ERR("Free values are not allowed: '" + fv.front() + "'", 4);

    MCR_ERR("Failed with error code: " + std::to_string(error), 5);
  }

  if (auto error = readValues(p, &db); error)
    return 6;

  printValues(&db);
  return 0;
}
} // namespace demo
