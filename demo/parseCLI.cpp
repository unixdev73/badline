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

#include <badline/scopedLogger.hpp>
#include <badline/argParser.hpp>
#include "parseCLI.hpp"
#include <unordered_map>
#include <string>

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

void populateDatabase(ap::ArgParserT *const p, ArgDatabaseT *db,
                      AppCLI *const data, sl::LoggerT *const l = nullptr) {
  sl::FunctionScope fs{l, __func__};
  db->flags.emplace("debug", ArgInfoT{'d', &data->debug});
  for (auto const &desc : db->flags)
    ap::addFlag(p, desc.first, desc.second.shortForm);

  db->numOpts.emplace("width", ArgInfoT{'w', &data->width});
  db->numOpts.emplace("height", ArgInfoT{'h', &data->height});
  for (auto const &desc : db->numOpts)
    ap::addOption(p, desc.first, desc.second.shortForm);

  for (auto const &desc : db->strOpts)
    ap::addOption(p, desc.first, desc.second.shortForm);
}

void printValues(ArgDatabaseT *const db, sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  using sl::inf;
  inf(l, "Listing command line argument parsing summary:");

  auto sp = [l](std::string const &str) {
    sl::oneTimePrefixLevel(l, false);
    sl::oneTimePrefixFunc(l, false);
    sl::inf(l, str);
  };

  for (auto const &[id, info] : db->flags)
    sp("\tID: '" + id + "', value: '" + (*info.value ? "true" : "false"));

  for (auto const &[id, info] : db->strOpts)
    sp("\tID: '" + id + "', value: '" + *info.value);

  for (auto const &[id, info] : db->numOpts)
    sp("\tID: '" + id + "', value: '" + std::to_string(*info.value));
}

int readStr(ap::ArgParserT *const p, std::string const &opt, std::string *out,
            sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  using sl::err;
  std::vector<std::string> vals{};

  if (auto error = ap::getOptionValues(p, opt, &vals); error) {
    std::string const c = std::to_string(error);
    err(l, "Fetching option: '" + opt + "' values failed with error: " + c);
    return 1;
  }

  if (!vals.size())
    return 0;

  *out = vals.front();
  return 0;
}

int readUint32(ap::ArgParserT *const p, std::string const &opt, uint32_t *out,
               sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  std::string tmp{};
  using sl::err;

  try {
    if (auto error = readStr(p, opt, &tmp, l); error) {
      std::string const c = std::to_string(error);
      err(l, "Fetching option: '" + opt + "' values failed with error: " + c);
      return 1;
    }

    if (tmp.empty())
      return 0;

    *out = std::stoul(tmp);
  } catch (...) {
    sl::err(l, "Failed to convert: '" + tmp + "' to a number.");
    return 1;
  }

  return 0;
}

int readValues(ap::ArgParserT *const p, ArgDatabaseT *db,
               sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  for (auto const &[id, info] : db->flags) {
    std::size_t count{};
    if (auto error = ap::getFlagOccurrence(p, id, &count); error)
      return 1;
    *info.value = static_cast<bool>(count);
  }

  for (auto const &[id, info] : db->numOpts)
    if (auto error = readUint32(p, id, info.value, l); error)
      return 2;

  for (auto const &[id, info] : db->strOpts)
    if (auto error = readStr(p, id, info.value, l); error)
      return 3;

  return 0;
}

int parseCLI(AppCLI *const data, char const *const *const input,
             std::size_t const begin, std::size_t const end,
             sl::LoggerT *const l) {
  if (!l) {
    // Should we log something??? To be discussed.
    return 255;
  }
  sl::FunctionScope fs(l, __func__);

  if (!data || !input)
    sl::err(l, "One of the input parameters is a nullptr.");

  ap::ArgParserT *p{};
  if (auto error = ap::createArgParser(&p, true); error) {
    sl::err(l, "Failed to create argument parser.");
    return 2;
  }

  ArgDatabaseT db{};
  populateDatabase(p, &db, data, l);

  std::vector<std::string> fv{};
  std::size_t errPos{};
  if (auto error = ap::parse(p, input, begin, end, &errPos); error) {
    if (error == ap::Result::ErrorOptionHasNoValue) {
      std::string const option = input[errPos];
      sl::err(l, "The option: '" + option + "' requires a value.");
    }

    sl::err(l, "Failed with error code: " + std::to_string(error));
  }
  if (ap::getFreeValues(p, &fv); fv.size())
    sl::err(l, "Free values are not allowed: '" + fv.front() + "'");

  if (auto error = readValues(p, &db, l); error)
    return 6;

  printValues(&db, l);
  return 0;
}
} // namespace demo
