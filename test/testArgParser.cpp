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

  auto parser = ap::createArgParser();
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
        return ap::Result::ErrorArgLongFormNotValid;
      }
    }
    if (keyVal.empty() || keyVal[0].empty()) {
      std::cerr << "At least a long form must be provided." << std::endl;
      return ap::Result::ErrorArgLongFormNotValid;
    }

    bool const shV = keyVal.size() == 2 && keyVal[1][0] != 0;
    return f(parser.get(), keyVal[0], shV ? keyVal[1][0] : 0);
  };

  std::size_t const offset = 3;

  for (auto const &a : fdef) {
    if (auto r = splitAndRegister(a, ap::addFlag); r != ap::Result::Success) {
      std::string code{};
      ap::toString(r, &code);
      std::cerr << "Failed to add flag with error: " << code << std::endl;
      return 4;
    }
  }

  for (auto const &a : odef) {
    if (auto r = splitAndRegister(a, ap::addOption); r != ap::Result::Success) {
      std::string code{};
      ap::toString(r, &code);
      std::cerr << "Failed to add option with error: " << code << std::endl;
      return 4;
    }
  }

  auto result = ap::parse(handle, argv, offset, argc);
  if (result != ap::Result::Success) {
    std::size_t errPos{};
    ap::getErrorPosition(handle, &errPos);
    std::string error{};
    ap::toString(result, &error);

    std::cerr << "Parsing failed at position " << errPos;
    std::cerr << ": '" << argv[errPos + offset] << "', with error: ";
    std::cerr << error << "." << std::endl;
    return 5;
  }

  return 0;
}
