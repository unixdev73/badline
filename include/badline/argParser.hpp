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
struct ArgParserT;

namespace Result {
enum Type : int {
  Success,

  ErrorNullptrHandle,
  ErrorNullptrInput,
  ErrorNullptrCount,
  ErrorNullptrPosition,
  ErrorNullptrValue,
  ErrorNullptrOutput,

  ErrorArgLongFormNotUnique,
  ErrorArgShortFormNotUnique,

  ErrorArgLongFormNotValid,
  ErrorArgShortFormNotValid,
  ErrorBeginEndRangeNotValid,
  ErrorInstanceIndexNotValid,

  ErrorTermTokenNotValid,
  ErrorMemoryAllocationFailure,
  ErrorResultCodeNotValid,
  ErrorRuleIdentifierNotValid,
  ErrorExpectedArgListToken,

  ErrorStartSymbolNotDerivedFromInput,
  ErrorInputTokenNotValid,
  ErrorOptionRequiresValue
};

int toString(int const result, std::string *const output);
} // namespace Result

int createArgParser(ArgParserT **const handle);

void destroyArgParser(ArgParserT const *const handle);

int addFlag(ArgParserT *const handle, std::string const &argLongForm,
            char const argShortForm = 0);

int addOption(ArgParserT *const handle, std::string const &argLongForm,
              char const argShortForm = 0);

int parse(ArgParserT *const handle, char const *const *const input,
          std::size_t const begin, std::size_t const end);

int getErrorPosition(ArgParserT *const handle, std::size_t *const output);

int getFlagCount(ArgParserT const *const handle, std::string const &argLongForm,
                 std::size_t *const count);

int getFlagInstancePosition(ArgParserT const *const handle,
                            std::string const &argLongForm,
                            std::size_t const instanceIndex,
                            std::size_t *const position);

int getOptionCount(ArgParserT const *const handle,
                   std::string const &argLongForm, std::size_t *const count);

int getOptionInstancePosition(ArgParserT const *const handle,
                              std::string const &argLongForm,
                              std::size_t const instanceIndex,
                              std::size_t *const position);

int getOptionInstanceValue(ArgParserT const *const handle,
                           std::string const &argLongForm,
                           std::size_t const instanceIndex,
                           std::string *const value);

int getFreeValueCount(ArgParserT const *const handle, std::size_t *const count);

int getFreeValueInstancePosition(ArgParserT const *const handle,
                                 std::size_t const instanceIndex,
                                 std::size_t *const position);

int getFreeValueInstanceValue(ArgParserT const *const handle,
                              std::size_t const instanceIndex,
                              std::string *const value);
} // namespace ap
