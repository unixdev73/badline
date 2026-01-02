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

namespace ap {
struct ArgParser;

/* Creates a new instance of the ArgParser struct.
 * If creation fails *handle = nullptr.
 * If handle = nullptr, the procedure returns.
 */
void createArgParser(ArgParser **const handle);

/* Deletes an instance of ArgParser.
 */
void destroyArgParser(ArgParser const *const handle);

/* Every function that returns bool will leave an error message
 * if it returns false.
 *
 * If handle = nullptr, the procedure returns.
 */
void getErrorMessage(ArgParser const *const handle, char const **const message);

bool addFlag(ArgParser *const handle,
             char const *const argLongForm,
             char const argShortForm = 0);

bool addOption(ArgParser *const handle,
               char const *const argLongForm,
               char const argShortForm = 0);

bool parse(ArgParser *const handle,
           char const *const *const input,
           unsigned const begin,
           unsigned const end);

bool getFlagCount(ArgParser const *const handle,
                  char const *const argLongForm,
                  unsigned *const count);

bool getFlagPosition(ArgParser const *const handle,
                     char const *const argLongForm,
                     unsigned const instance,
                     unsigned *const position);

bool getOptionCount(ArgParser const *const handle,
                    char const *const argLongForm,
                    unsigned *const count);

bool getOptionPosition(ArgParser const *const handle,
                       char const *const argLongForm,
                       unsigned const instance,
                       unsigned *const position);

bool getOptionValue(ArgParser const *const handle,
                    char const *const argLongForm,
                    unsigned const instance,
                    char const **const value);

bool getFreeValueCount(ArgParser const *const handle, unsigned *const count);

bool getFreeValuePosition(ArgParser const *const handle,
                          unsigned const instance,
                          unsigned *const position);

bool getFreeValue(ArgParser const *const handle,
                  unsigned const instance,
                  char const **const value);
} // namespace ap
