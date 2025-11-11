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

/* DESCRIPTION:
 *
 * This binary tests the 'split' method. It takes three parameters.
 * The first one is the input, which is expected to be some kind of assignment.
 * The second one is the expected left hand side of the assignment,
 * and the third one is the expected right hand side of the assignment.
 *
 * That is to say, if your input is 'left=right', then the expected lhs
 * would be 'left', and the rhs would be 'right'.
 *
 * This binary splits the input, and checks if the lhs and rhs
 * are as expected.
 *
 * EXIT STATUS:
 *
 * 0 - The LHS and RHS match the expectations.
 *
 * 1 - The LHS or RHS don't match the expectations.
 */

#include <argParser/internals.hpp>
#include <iostream>

int main(int const argc, char const *const *const argv) {
  if (argc != 4) {
    std::cerr << "Too few arguments; Usage: <input> <lhs> <rhs>\n";
    return 1;
  }

  std::string const input = argv[1];
  std::string const left = argv[2];
  std::string const right = argv[3];
  std::pair<std::string, std::string> kv{};

  ap::split(&input, '=', &kv);
  std::cout << "input: " << input << std::endl;
  std::cout << "left: " << kv.first << std::endl;
  std::cout << "right: " << kv.second << std::endl;

  if (kv.first == left && kv.second == right)
    return 0;
  return 1;
}
