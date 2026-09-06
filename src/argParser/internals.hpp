/* Copyright (c) 2026 unixdev73@gmail.com

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

#include "smartResource.hpp"
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <list>
#include <memory>

namespace ap {
enum class Mode { Strict, Lenient };

enum class State {
  ParseInputToken,
  HandleOptionValue,
  HandleOptionRogueValue,
  HandleRogueFreeValue
};

struct ArgInstanceInfo {
  std::size_t position{};
  std::string value{};
};

struct ArgInstanceDatabase {
  std::unordered_map<std::string, std::unique_ptr<std::vector<ArgInstanceInfo>>>
      longForm{};
  std::unordered_map<char, std::vector<ArgInstanceInfo> *> shortForm{};
};

struct GrammarRule {
  enum Identifier : std::size_t {
    // TERMS
    ShortArgPrefix,
    AssignmentOp,
    Comma,
    Digit,
    Underscore,
    SmallLetter,
    BigLetter,
    Letter,
    Alnum,
    NonAlnum,
    Printable,
    NonShortArgPrefix,

    // NTERMS
    ArgTerm,
    LongArgPrefix,
    ShortArg,
    AlnumString,
    PrintableString,
    SimpleLongArg,
    LongArg,
    LongArgExtension,
    UnderscoreExtension,
    DashExtension,
    AssignmentRight,
    ArgAssignment,
    CompoundArg,
    FreeValue,

    // UTIL
    Start,
    Size
  };

  static bool toString(std::size_t const id, std::string *const output);
};

struct GrammarRuleVariant {
  std::size_t nonTermA{}, nonTermB{};
  std::function<void(std::string const &,
                     std::size_t const beginA,
                     std::size_t const endA,
                     std::size_t const beginB,
                     std::size_t const endB)>
      semanticAction{};
};

struct RuleInfo {
  std::size_t identifier;
  std::size_t locationY;
  std::size_t begin, end;

  bool operator==(RuleInfo const &o) const {
    return identifier == o.identifier && locationY == o.locationY &&
           begin == o.begin && end == o.end;
  }
};

struct BackPtr {
  std::size_t variant;
  RuleInfo ruleLHS;
  RuleInfo ruleRHS;

  bool operator==(BackPtr const &o) const {
    return variant == o.variant && ruleLHS == o.ruleLHS && ruleRHS == o.ruleRHS;
  }
};

struct TokenInfo {
  std::string argName{};
  std::string argExt{};
  std::string argVal{};
  bool isArgList{};
  bool isFreeVal{};
};

struct ParsingDatabase {
  using NonTermId = std::size_t;
  using TermId = char;
  using TermPair = std::pair<std::vector<NonTermId>, std::vector<TermId>>;
  std::vector<TermPair> termMapping{};

  using GrammarRule = std::vector<GrammarRuleVariant>;
  std::vector<GrammarRule> grammar{};
  NonTermId startSymbol{};

  using ParseChart = std::vector<std::vector<std::vector<bool>>>;
  ParseChart chart{};

  using RuleVariations = std::vector<BackPtr>;
  using BackChart = std::vector<std::vector<std::vector<RuleVariations>>>;
  BackChart back{};

  using RuleDesc = std::pair<NonTermId, BackPtr>;
  std::list<RuleDesc> serialized{};

  TokenInfo tokenInfo{};
};

struct Logs;

struct ArgParser {
  mutable CustomUniqPtr<Logs> logs{};
  std::vector<ArgInstanceInfo> freeValues{};
  ArgInstanceDatabase options{};
  ArgInstanceDatabase flags{};

  ParsingDatabase database{};
  State currentState{};
  Mode mode{};

  std::vector<ArgInstanceInfo> *targetOption{};
  std::size_t errorPosition{};
  bool databaseFilled{false};
  bool customAlloc{};
};
} // namespace ap

namespace ap {

bool updateArguments(ArgParser *const handle,
                     std::string const *const token,
                     std::size_t const position);

bool tracePostorderPath(ParsingDatabase *const database,
                        std::size_t const variant);

bool initParseChart(ArgParser *const database, std::string const *const input);

bool parseCYK(ArgParser *const database, std::string const *const input);

bool handleState(ArgParser *const handle,
                 std::string const *const token,
                 std::size_t const position,
                 bool *const skip);

bool fillParsingDatabaseWithAlphabet(ParsingDatabase *const database);
bool fillParsingDatabaseWithDigits(ParsingDatabase *const database);
bool fillParsingDatabaseWithMisc(ParsingDatabase *const database);
bool fillParsingDatabase(ParsingDatabase *const database);
bool areOptionsAssigned(ArgParser const *const handle);

void addErrMsg(ArgParser const *const handle,
               std::string const &tag,
               std::string const &msg);

void addWrnMsg(ArgParser const *const handle,
               std::string const &tag,
               std::string const &msg);

void addInfMsg(ArgParser const *const handle,
               std::string const &tag,
               std::string const &msg);
} // namespace ap
