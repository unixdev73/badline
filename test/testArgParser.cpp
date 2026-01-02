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

#include <badline/argParser.hpp>
#include <iostream>
#include <fstream>
#include <ranges>
#include <vector>

using SmartArgParser =
    std::unique_ptr<ap::ArgParser, void (*)(ap::ArgParser const *const)>;

SmartArgParser createSmartArgParser() {
  ap::ArgParser *parser{};
  ap::createArgParser(&parser);
  return SmartArgParser{parser, ap::destroyArgParser};
}

void printErrorMessage(ap::ArgParser const *const parser) {
  char const *errorString{};
  ap::getErrorMessage(parser, &errorString);
  if (!errorString)
    return;
  std::cerr << "ERROR: " << errorString << std::endl;
}

int main(int const argc, char const *const *const argv) {
  if (argc < 4) {
    std::cerr << "The number of arguments is too small." << std::endl;
    std::cerr << "Usage: <flag defs file> <opt defs file> <input>" << std::endl;
    return 1;
  }

  std::ifstream flags{argv[1]}, opts{argv[2]};
  if (!flags.is_open() || !opts.is_open()) {
    std::cerr << "Failed to open resource files." << std::endl;
    return 1;
  }

  std::vector<std::string> fdef{};
  std::vector<std::string> odef{};
  std::string line{};

  while (std::getline(flags, line))
    fdef.push_back(line);
  while (std::getline(opts, line))
    odef.push_back(line);

  auto parser = createSmartArgParser();
  auto handle = parser.get();

  if (!handle) {
    std::cerr << "Failed to create parser instance." << std::endl;
    return 2;
  }

  auto splitAndRegister = [&parser](std::string const &arg, auto &&f) {
    auto pairView = std::ranges::views::split(arg, ':');
    std::vector<std::string> keyVal{};
    for (auto pv : pairView) {
      std::string elem{};
      for (auto e : pv)
        elem.push_back(e);
      keyVal.push_back(std::move(elem));
      if (keyVal.empty() || keyVal[0].empty()) {
        std::cerr << "At least a long form must be provided." << std::endl;
        return false;
      }
    }
    if (keyVal.empty() || keyVal[0].empty()) {
      std::cerr << "At least a long form must be provided." << std::endl;
      return false;
    }

    bool const shV = keyVal.size() == 2 && keyVal[1][0] != 0;
    return f(parser.get(), keyVal[0].c_str(), shV ? keyVal[1][0] : 0);
  };

  std::size_t const offset = 3;

  for (auto const &a : fdef) {
    if (!splitAndRegister(a, ap::addFlag)) {
      std::cerr << "Failed to add flag" << std::endl;
      return 4;
    }
  }

  for (auto const &a : odef) {
    if (!splitAndRegister(a, ap::addOption)) {
      std::cerr << "Failed to add option" << std::endl;
      return 4;
    }
  }

  auto result = ap::parse(handle, argv, offset, argc);
  if (!result) {
    printErrorMessage(handle);
    return 5;
  }

  return 0;
}
