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
#include <vector>

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
  bool debug{false};
};
} // namespace ap

namespace ap {
int handleLongArg(ArgParserT *const parser, std::size_t const pos,
                  std::string const *const id,
                  std::vector<char const *> const *const tokens,
                  bool *const skipToken);

int handleShortArg(ArgParserT *const parser, std::size_t const pos,
                   std::string const *const id,
                   std::vector<char const *> const *const tokens,
                   bool *const skipToken);

int recognizeAndRegisterArg(ArgParserT *const parser, char const id,
                            ArgInstanceInfoT **const info,
                            ArgTypeT *const type);

int addArg(ArgParserT *const parser, ArgTypeT const type,
           std::string const &longForm, char const shortForm = 0);

int split(KeyValueT *const pair, std::string const *const input,
          char const delimiter = '=');

int validateParseParameters(ArgParserT *const parser,
                            char const *const *const input,
                            std::size_t const begin, std::size_t end);

std::vector<char const *> lookAhead(char const *const *input,
                                    std::size_t const inputSz, std::size_t i,
                                    std::size_t const n);
} // namespace ap
