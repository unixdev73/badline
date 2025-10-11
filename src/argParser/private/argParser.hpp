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

#include <unordered_map>
#include <string>
#include <list>

namespace ap {
struct KeyValueT {
  std::string key;
  std::string value;
};

enum class ArgTypeT { Flag, Option };

struct ArgInstanceInfoT {
  std::size_t position{};
  std::string value{};
};

struct ArgInstanceDatabaseT {
  std::unordered_map<std::string, std::list<ArgInstanceInfoT>> longForm{};
  std::unordered_map<char, std::list<ArgInstanceInfoT> *> shortForm{};
};

struct ArgParserT {
  std::list<ArgInstanceInfoT> freeValues{};
  ArgInstanceDatabaseT options{};
  ArgInstanceDatabaseT flags{};

  std::string longArgPrefix{"--"};
  char shortArgPrefix{'-'};
};
} // namespace ap

namespace ap {
/* DESCRIPTION:
 *
 * Recognizes if an argument in its long form is passed.
 * If so, the appropriate structures are updated.
 * If the argument is ended by an assignment, then the value
 * is assigned to that option instance.
 * If there is no assignment, then the 'value' argument will be checked,
 * and if it is valid, then the option will be assigned the value.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful, and the argument was recognized,
 * and if there was an assignment in the same token,
 * then the value was assigned.
 *
 * 4 - The execution was successful, and the argument was recognized as an
 * option, but no assignment is part of the token, so the 'value' argument
 * was used for assignment.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'id' argument is not a recognized argument.
 *
 * 3 - The argument is an option identifier,
 * and expects a value, yet none was given.
 *
 * 255 - FATAL_ERROR: OOM or an external API threw. Abandon all hope :(
 */
int handleLongArg(ArgParserT *const parser, std::size_t const pos,
                  std::string const *const id, std::string const *const value);

/* DESCRIPTION:
 *
 * Recognizes if an argument list is passed.
 * If so, the appropriate structures are updated.
 * If the argument list is ended by an assignment, and the last
 * argument prior to the assignment is an option, then the value
 * is assigned to that option instance.
 * If the argument list is terminated by an option, but there is no assignment,
 * then the 'value' argument will be checked, and if it is valid, then
 * the option will be assigned the value.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful, and the argument list was recognized,
 * and if an option was the last argument in the list, then it also was
 * assigned in the same token.
 *
 * 5 - The execution was successful, and the argument list was recognized,
 * and if an option was the last argument in the list, then it was assigned
 * using the 'value' argument.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'id' argument is not an argument list.
 *
 * 3 - The argument list includes an assignment, as if the last
 * argument before the assignment were an option identifier,
 * but that is not the case.
 *
 * 4 - The argument list ended with an option identifier,
 * and expects a value, yet none was given.
 *
 * 255 - FATAL_ERROR: OOM or an external API threw. Abandon all hope :(
 */
int handleShortArg(ArgParserT *const parser, std::size_t const pos,
                   std::string const *const id, std::string const *const value);

/* DESCRIPTION:
 *
 * Recognizes if an argument from the database was passed.
 * If so, the appropriate structures are updated.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful, and the character was recognized.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'info' argument is a nullptr.
 *
 * 3 - The 'id' argument is not a known argument short form.
 *
 * 4 - The 'type' argument is a nullptr.
 *
 * 255 - FATAL_ERROR: OOM or an external API threw. Abandon all hope :(
 */
int recognizeAndRegisterArg(ArgParserT *const parser, char const id,
                            ArgInstanceInfoT **info, ArgTypeT *type);

/* DESCRIPTION:
 *
 * Add a new argument to the database, if the name is not already taken.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The 'parser' argument is a nullptr.
 *
 * 2 - The 'longForm' argument is empty.
 *
 * 3 - The 'longForm' argument is not unique.
 *
 * 4 - The 'shortForm' argument is not unique.
 *
 * 5 - The 'longForm' argument contains characters other than alnum.
 *
 * 6 - The 'shortForm' argument is not an alnum.
 *
 * 255 - FATAL_ERROR: OOM or an external API threw. Abandon all hope :(
 */
int addArg(ArgParserT *const parser, ArgTypeT const type,
           std::string const &longForm, char const shortForm = 0) noexcept;

/* DESCRIPTION:
 *
 * Splits a string using a delimiter into a key (left hand side),
 * and a value (right hand side).
 *
 * The first encountered delimiter is the one used for the split.
 *
 * If the delimiter is not found within the input,
 * the input is copied into the key member of the 'pair' argument.
 *
 * If the input is an empty string, the 'pair' argument is not touched.
 *
 * EXIT STATUS:
 *
 * 0 - The execution was successful.
 *
 * 1 - The argument 'pair' is a nullptr.
 *
 * 2 - The argument 'input' is a nullptr.
 *
 * 255 - FATAL_ERROR: OOM or an external API threw. Abandon all hope :(
 */
int split(KeyValueT *const pair, std::string const *const input,
          char const delimiter = '=') noexcept;
} // namespace ap
