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

int main(int const argc, char const *const *const argv) {
  ap::InputBindingT b{.input = argv, .begin = 1, .end = argc};
  auto p = ap::createArgParser();

  ap::addFlag(p.get(), "verbose", 'v');
  ap::addFlag(p.get(), "help", 'h');
  ap::addFlag(p.get(), "yes", 'y');
  ap::addOption(p.get(), "output", 'o');

  auto error = ap::parse(p.get(), &b);
  if (error) {
    std::cerr << "Failed to parse the input with error code: " << error
              << std::endl;
  }

  auto reportFlag = [ptr = p.get()](std::string const &id) {
    std::size_t count{};
    ap::getFlagOccurrence(ptr, id, &count);
    std::cout << id << " count: " << count << std::endl;
  };

  auto reportOpt = [ptr = p.get()](std::string const &id) {
    std::vector<std::string> values{};
    ap::getOptionValues(ptr, id, &values);
    std::cout << id << " values: ";
    for (auto const &v : values)
      std::cout << v << " ";
    std::cout << std::endl;
  };

  reportFlag("verbose");
  reportFlag("help");
  reportFlag("yes");
  reportOpt("output");

  std::vector<std::string> free{};
  ap::getFreeValues(p.get(), &free);

  for (auto const &v : free)
    std::cout << "Free val: " << v << std::endl;
  return 0;
}
