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

#include <badline/renderEngine.hpp>
#include <badline/argParser.hpp>
#include <functional>
#include <iostream>
#include <optional>
#include <list>

struct AppCLI {
  uint32_t width{1280};
  uint32_t height{720};
  bool debug{false};
  std::string output{};
};

int parseCLI(AppCLI *const data, ap::InputBinding *const binding);

int main(int const argc, char const *const *const argv) {
  ap::InputBinding binding{.input = argv, .begin = 1, .end = argc};
  AppCLI data{};

  if (auto error = parseCLI(&data, &binding); error)
    return 1;

  auto engine = re::createRenderEngine("Demo", data.debug);
  if (!engine) {
    std::cerr << "Failed to create game engine instance.\n";
    return 1;
  }

  if (auto err = re::createWindow(engine.get(), data.width, data.height); err) {
    std::cerr << "Failed to create window.\n";
    return 1;
  }

  return re::run(engine.get());
}

int readStr(ap::UniqueArgParser &p, std::string const &opt, std::string *out) {
  try {
    std::vector<std::string> vals{};
    std::optional<uint32_t> value{};

    if (auto error = ap::getOptionValues(p.get(), opt, &vals); error)
      return 1;

    if (!vals.size())
      return 0; // The option was not specified. Not a problem.
                // Simply use the default values.

    *out = vals.front();
  } catch (...) {
    std::cerr << "Failed to read a string option value.\n";
    return 1;
  }

  return 0;
}

int readUint32(ap::UniqueArgParser &p, std::string const &opt, uint32_t *out) {
  std::string tmp{};
  try {
    std::vector<std::string> vals{};
    std::optional<uint32_t> value{};

    if (auto error = ap::getOptionValues(p.get(), opt, &vals); error)
      return 1;

    if (!vals.size())
      return 0; // The option was not specified. Not a problem.
                // Simply use the default values.

    tmp = vals.front();
    *out = std::stoul(tmp);
  } catch (...) {
    std::cerr << "Failed to convert: '" << tmp << "' to a number.\n";
    return 1;
  }

  return 0;
}

#define MCR_READ_OPT_NUM_VAL(option)                                           \
  if (auto error = readUint32(p, #option, &data->option); error) {             \
    std::cerr << "Failed to read value of option: '" #option "'\n";            \
    return 1;                                                                  \
  }                                                                            \
  printCallbacks.push_back([data]() {                                          \
    std::cout << "\t" << #option << ": " << data->option << std::endl;         \
  })

#define MCR_READ_OPT_STR_VAL(option)                                           \
  if (auto error = readStr(p, #option, &data->option); error) {                \
    std::cerr << "Failed to read value of option: '" #option "'\n";            \
    return 1;                                                                  \
  }                                                                            \
  printCallbacks.push_back([data]() {                                          \
    std::cout << "\t" << #option << ": " << data->option << std::endl;         \
  })

#define MCR_READ_FLAG(flag)                                                    \
  std::size_t count{};                                                         \
  if (auto error = ap::getFlagOccurrence(p.get(), #flag, &count); error)       \
    return 1;                                                                  \
  if (count)                                                                   \
    data->flag = true;                                                         \
  printCallbacks.push_back([data]() {                                          \
    std::cout << "\t" << #flag ": " << std::boolalpha << data->flag << "\n";   \
  })

int parseCLI(AppCLI *const data, ap::InputBinding *const binding) {
  if (!data || !binding)
    return 1;

  auto p = ap::createArgParser();

  ap::addFlag(p.get(), "debug", 'd');
  ap::addOption(p.get(), "width", 'w');
  ap::addOption(p.get(), "height", 'h');
  ap::addOption(p.get(), "output", 'o');

  std::size_t errPos{};
  if (auto error = ap::parse(p.get(), binding, &errPos); error) {
    if (error == ap::Result::ErrorOptionHasNoValue)
      std::cerr << "Parsing failed because the option in token: '"
                << binding->input[errPos] << "' wasn't assigned a value.\n";
    else
      std::cerr << "Parsing failed with error code: " << error << std::endl;
    return 1;
  }

  std::list<std::function<void()>> printCallbacks{};
  MCR_READ_OPT_NUM_VAL(width);
  MCR_READ_OPT_NUM_VAL(height);
  MCR_READ_OPT_STR_VAL(output);
  MCR_READ_FLAG(debug);

  std::cout << "Listing command line argument parsing summary:\n";
  for (auto cb : printCallbacks)
    cb();
  return 0;
}
