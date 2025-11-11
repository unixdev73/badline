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
#include <badline/scopedLogger.hpp>
#include <badline/argParser.hpp>
#include <iostream>

int main(int const argc, char const *const *const argv) {
  try {
    if (argc == 1)
      return 1;

    ap::ArgParserT *handle{};
    ap::createArgParser(&handle);

    std::vector<std::string> fid{};
    std::vector<std::string> oid{};

    auto addF = [&fid, handle](std::string const &s, char const c) {
      ap::addFlag(handle, s, c);
      fid.push_back(s);
    };

    auto addO = [&oid, handle](std::string const &s, char const c) {
      ap::addOption(handle, s, c);
      oid.push_back(s);
    };

    addF("help", 'h');
    addF("quiet", 'q');
    addF("clean", 'c');
    addO("value", 'v');
    addO("output", 'o');

    std::size_t const begin = 1;
    auto resultCode = ap::parse(handle, argv, begin, argc);
    std::string result{};
    ap::Result::toString(resultCode, &result);
    std::cout << "Parsing status: " << result;
    if (resultCode != ap::Result::Success) {
      std::size_t position{};
      ap::getErrorPosition(handle, &position);
      std::cout << ", problematic token: '" << argv[begin + position] << "' ";
      std::cout << "at position: " << position << std::endl;
      return 1;
    }
    std::cout << std::endl << std::endl;

    std::size_t count{};
    std::string value;
    std::size_t pos;

    for (auto const &f : fid) {
      ap::getFlagCount(handle, f, &count);
      std::cout << f << " occurred: " << count << " times\n";
    }

    for (auto const &o : oid) {
      ap::getOptionCount(handle, o, &count);
      std::cout << o << " occurred: " << count << " times\n";
      for (std::size_t i = 0; i < count; ++i) {
        ap::getOptionInstanceValue(handle, o, i, &value);
        ap::getOptionInstancePosition(handle, o, i, &pos);
        std::cout << "instance: " << i << ": value: " << value
                  << " pos: " << pos << std::endl;
      }
    }

    ap::getFreeValueCount(handle, &count);
    for (std::size_t i = 0; i < count; ++i) {
      ap::getFreeValueInstanceValue(handle, i, &value);
      ap::getFreeValueInstancePosition(handle, i, &pos);
      std::cout << "free val instance: " << i << ": value: " << value
                << " pos: " << pos << std::endl;
    }

    ap::destroyArgParser(handle);
  } catch (std::exception const &e) {
    std::cerr << "Error: Caught exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Error: Caught unknown exception." << std::endl;
  }
}
