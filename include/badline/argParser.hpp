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

#include <memory>
#include <vector>

namespace ap {
struct InputBindingT {
  char const *const *input;
  int begin;
  int end;
};

struct ArgParserT;
using ArgParser = ArgParserT *;
using UniqueArgParser = std::unique_ptr<ArgParserT, void (*)(ArgParser const)>;

/* DESCRIPTION:
 *
 * Creates an arg parser.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The creation failed.
 */
int createArgParser(ArgParser *const);
void destroyArgParser(ArgParser const);

/* DESCRIPTION:
 *
 * Creates an arg parser, and wraps it into a smart pointer.
 * Returns a nullptr if creation fails.
 *
 */
UniqueArgParser createArgParser();

/* DESCRIPTION:
 *
 * Adds a flag description to the database.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'l' argument is not a valid long form.
 *
 * 3 - The 's' argument is not a valid short form.
 *
 * 255 - FATAL_ERROR: OOM or an external API threw. Abandon all hope :(
 */
int addFlag(ArgParser const parser, std::string const &l, char const s = 0);

/* DESCRIPTION:
 *
 * Adds an option description to the database.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'l' argument is not a valid long form.
 *
 * 3 - The 's' argument is not a valid short form.
 *
 * 255 - FATAL_ERROR: OOM or an external API threw. Abandon all hope :(
 */
int addOption(ArgParser const parser, std::string const &l, char const s = 0);

/* DESCRIPTION:
 *
 * Parses the input, and updates the database structures.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'binding' argument is a nullptr.
 *
 * 3 - The 'input' member of the 'binding' argument is a nullptr.
 *
 * 4 - The 'binding' range is not valid. 'begin' > 'end'.
 *
 * 5 - Parsing failed.
 *
 * 255 - FATAL_ERROR: OOM or an external API threw. Abandon all hope :(
 */
int parse(ArgParser const parser, InputBindingT const *const binding);

/* DESCRIPTION:
 *
 * Sets the 'count' argument to the number of occurrences of the 'flag'.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'count' argument is a nullptr.
 */
int getFlagOccurrence(ArgParser const parser, std::string const &flag,
                      std::size_t *count);

/* DESCRIPTION:
 *
 * Fetches the values of the queried option and stores them in the 'out'
 * container.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'out' argument is a nullptr.
 */
int getOptionValues(ArgParser const parser, std::string const &opt,
                    std::vector<std::string> *const out);

/* DESCRIPTION:
 *
 * Fetches free values.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'out' argument is a nullptr.
 */
int getFreeValues(ArgParser const parser, std::vector<std::string> *const out);
} // namespace ap
