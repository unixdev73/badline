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

#include <vector>
#include <string>

namespace ap {
struct ArgParserT;

namespace Result {
constexpr int Success = 0;
constexpr int ErrorNullptrParameter = 1;
constexpr int ErrorMemoryAllocationFailure = 2;
constexpr int ErrorRangeBeginGreaterThanEnd = 3;
constexpr int ErrorOptionHasNoValue = 4;
constexpr int ErrorEmptyStringParameter = 5;
constexpr int ErrorIdAlreadyInUse = 6;
constexpr int ErrorStringNotValid = 7;
constexpr int ErrorCharacterNotValid = 8;
constexpr int ErrorIdNotValid = 9;
constexpr int ErrorFlagAssignedValue = 11;
constexpr int TokenNotHandled = 12;
} // namespace Result

int createArgParser(ArgParserT **const, bool debug = false);

void destroyArgParser(ArgParserT *const);

int addFlag(ArgParserT *const parser, std::string const &l, char const s = 0);

int addOption(ArgParserT *const parser, std::string const &l, char const s = 0);

int parse(ArgParserT *const parser, char const *const *const input,
          std::size_t const begin, std::size_t const end,
          std::size_t *const errorPosition = nullptr);

int getFlagOccurrence(ArgParserT *const parser, std::string const &flag,
                      std::size_t *count);

int getOptionValues(ArgParserT *const parser, std::string const &opt,
                    std::vector<std::string> *const out);

int getFreeValues(ArgParserT *const parser,
                  std::vector<std::string> *const out);
}
