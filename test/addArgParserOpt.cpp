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
#include <functional>
#include <iostream>
#include <ranges>
#include <vector>

using SmartArgParser =
    std::unique_ptr<ap::ArgParser, std::function<void(ap::ArgParser *const)>>;

SmartArgParser createSmartArgParser() {
  ap::ArgParser *parser{};
  ap::create(&parser);
  return SmartArgParser{parser, ap::destroy};
}

void printErrorMessage(ap::ArgParser const *const parser) {
  char const *errorString{};
  ap::getErrorMessage(parser, &errorString);
  if (!errorString)
    return;
  std::cerr << "ERROR: " << errorString << std::endl;
}

using ap::addOption;

int main(int const argc, char const *const *const argv) {
  if (argc < 2) {
    std::cerr << "The number of arguments is too small." << std::endl;
    return 1;
  }

  auto parser = createSmartArgParser();
  auto handle = parser.get();

  if (!handle) {
    std::cerr << "Failed to create parser instance." << std::endl;
    return 2;
  }

  std::size_t const offset = 1;
  for (std::size_t i = offset; i < std::size_t(argc); ++i) {
    std::string const arg = argv[i];
    auto pairView = std::ranges::views::split(arg, ':');
    std::vector<std::string> keyVal{};
    for (auto pv : pairView) {
      std::string elem{};
      for (auto e : pv)
        elem.push_back(e);
      keyVal.push_back(std::move(elem));
      if (keyVal.empty() || keyVal[0].empty()) {
        std::cerr << "At least a long form must be provided." << std::endl;
        return 3;
      }
    }

    bool const shV = keyVal.size() == 2 && keyVal[1][0] != 0;
    auto result =
        addOption(parser.get(), keyVal[0].c_str(), shV ? keyVal[1][0] : 0);
    if (!result) {
      printErrorMessage(handle);
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
