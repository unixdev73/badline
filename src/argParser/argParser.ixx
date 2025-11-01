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

module;

#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <vector>
#include <list>

export module ArgParser;

export namespace ap {
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

struct InputBinding {
  char const *const *input;
  int begin;
  int end;
};

struct ArgParserT;
using ArgParser = ArgParserT *;
using UniqueArgParser = std::unique_ptr<ArgParserT, void (*)(ArgParser const)>;

int createArgParser(ArgParser *const, bool debug = false);
void destroyArgParser(ArgParser const);
UniqueArgParser createArgParser(bool debug = false);

int addFlag(ArgParser const parser, std::string const &l, char const s = 0);
int addOption(ArgParser const parser, std::string const &l, char const s = 0);

int parse(ArgParser const parser, InputBinding const *const binding,
          std::size_t *const errorPosition = nullptr);

int getFlagOccurrence(ArgParser const parser, std::string const &flag,
                      std::size_t *count);
int getOptionValues(ArgParser const parser, std::string const &opt,
                    std::vector<std::string> *const out);
int getFreeValues(ArgParser const parser, std::vector<std::string> *const out);

int splitTest(int const argc, char const *const *const argv);
} // namespace ap

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

int handleLongArg(ArgParserT *const parser, std::size_t const pos,
                  std::string const *const id,
                  std::vector<char const *> const *const tokens,
                  bool *const skipToken);

int handleShortArg(ArgParserT *const parser, std::size_t const pos,
                   std::string const *const id,
                   std::vector<char const *> const *const tokens,
                   bool *const skipToken);

int recognizeAndRegisterArg(ArgParserT *const parser, char const id,
                            ArgInstanceInfoT **info, ArgTypeT *type);

int addArg(ArgParserT *const parser, ArgTypeT const type,
           std::string const &longForm, char const shortForm = 0);

int split(KeyValueT *const pair, std::string const *const input,
          char const delimiter = '=');
} // namespace ap
