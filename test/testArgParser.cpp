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
#include <print>

int main(int const argc, char const *const *const argv) {
  if (argc < 2) {
    std::println("The number of arguments is too small.");
    return 1;
  }

  auto parser = ap::createArgParser();
  auto handle = parser.get();

  if (!handle) {
    std::println("Failed to create parser instance.");
    return 2;
  }

  ap::addFlag(handle, "flagA", 'a');
  ap::addFlag(handle, "flagB", 'b');
  ap::addFlag(handle, "flagC", 'c');
  ap::addFlag(handle, "flagD", 'd');
  ap::addFlag(handle, "flagE", 'e');
  ap::addFlag(handle, "flagF", 'f');

  ap::addOption(handle, "optionG", 'g');
  ap::addOption(handle, "optionH", 'h');

  std::size_t const offset = 1;
  auto result = ap::parse(handle, argv, offset, argc);

  if (result != ap::Result::Success) {
    std::size_t errPos{};
    ap::getErrorPosition(handle, &errPos);
    std::string error{};
    ap::toString(result, &error);

    std::println("Parsing failed at position {}: '{}', with error : {}.",
                 errPos,
                 argv[errPos + offset],
                 error);
    return 2;
  }

  std::size_t freeValCount{};
  ap::getFreeValueCount(handle, &freeValCount);
  if (freeValCount) {
    std::println("Free values are not allowed in this test.");
    return 3;
  }

  return 0;
}
