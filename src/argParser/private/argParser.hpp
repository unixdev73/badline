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

#pragma once

#include <string>

namespace ap {
struct InputBindingT {
  char const *const *input;
  int begin;
  int end;
};

struct KeyValueT {
  std::string key;
  std::string value;
};
} // namespace ap

namespace ap {
/* DESCRIPTION:
 *
 * Splits a string using a delimiter into a key (left hand side),
 * and a value (right hand side).
 * The first encountered delimiter is the one used for the split.
 * If the delimiter is not found within the input,
 * the input is copied into the key member.
 * If the input is an empty string, the 'pair' argument is cleared.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The argument 'pair' is nullptr.
 *
 * 2 - The argument 'input' is nullptr.
 *
 * 3 - The argument 'delimiter' is not present within
 * the input sequence.
 */
int split(KeyValueT *const pair, std::string const *const input,
          char const delimiter);

/* EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The argument 'binding' is nullptr.
 *
 * 2 - The argument 'argv' is nullptr.
 *
 * 3 - The argument 'begin' is not valid.
 *
 * 4 - The argument 'end' is not valid.
 */
int bind(InputBindingT *const binding, char const *const *const argv,
         int const begin, int const end);
} // namespace ap
