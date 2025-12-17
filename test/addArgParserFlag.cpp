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
#include <ranges>
#include <vector>

int main(int const argc, char const *const *const argv) {
  if (argc < 2) {
    std::cerr << "The number of arguments is too small." << std::endl;
    return 1;
  }

  auto parser = ap::createArgParser();
  auto handle = parser.get();

  if (!handle) {
    std::cerr << "Failed to create parser instance." << std::endl;
    return 2;
  }

  std::size_t const offset = 1;
  for (std::size_t i = offset; i < std::size_t(argc); ++i) {
    std::string const arg = argv[i];
    auto pairView = std::ranges::views::split(arg, ':');
    auto keyVal = std::ranges::to<std::vector<std::string>>(pairView);
    if (keyVal.empty() || keyVal[0].empty()) {
      std::cerr << "At least a long form must be provided." << std::endl;
      return 3;
    }

    bool const shV = keyVal.size() == 2 && keyVal[1][0] != 0;
    auto result = ap::addFlag(parser.get(), keyVal[0], shV ? keyVal[1][0] : 0);
    if (result != ap::Result::Success) {
      std::string code{};
      ap::toString(result, &code);
      std::cerr << "Failed to add argument: " << keyVal[0] << " with error: ";
      std::cerr << code << std::endl;
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
