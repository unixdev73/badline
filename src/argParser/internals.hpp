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

#include <badline/argParser.hpp>
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <list>
#include <memory>

namespace ap {
enum class ModeT { Strict, Lenient };

enum class StateT {
  ParseInputToken,
  HandleOptionValue,
  HandleOptionRogueValue,
  HandleRogueFreeValue
};

struct ArgInstanceInfoT {
  std::size_t position{};
  std::string value{};
};

struct ArgInstanceDatabaseT {
  std::unordered_map<std::string,
                     std::unique_ptr<std::vector<ArgInstanceInfoT>>>
      longForm{};
  std::unordered_map<char, std::vector<ArgInstanceInfoT> *> shortForm{};
};

struct GrammarRuleT {
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

  static Result toString(std::size_t const id, std::string *const output);
};

struct GrammarRuleVariantT {
  std::size_t nonTermA{}, nonTermB{};
  std::function<void(std::string const &,
                     std::size_t const beginA,
                     std::size_t const endA,
                     std::size_t const beginB,
                     std::size_t const endB)>
      semanticAction{};
};

struct RuleInfoT {
  std::size_t identifier;
  std::size_t locationY;
  std::size_t begin, end;

  bool operator==(RuleInfoT const &o) const {
    return identifier == o.identifier && locationY == o.locationY &&
           begin == o.begin && end == o.end;
  }
};

struct BackPtrT {
  std::size_t variant;
  RuleInfoT ruleLHS;
  RuleInfoT ruleRHS;

  bool operator==(BackPtrT const &o) const {
    return variant == o.variant && ruleLHS == o.ruleLHS && ruleRHS == o.ruleRHS;
  }
};

struct TokenInfoT {
  std::string argName{};
  std::string argExt{};
  std::string argVal{};
  bool isArgList{};
  bool isFreeVal{};
};

struct ParsingDatabaseT {
  using NonTermId = std::size_t;
  using TermId = char;
  using TermPairT = std::pair<std::vector<NonTermId>, std::vector<TermId>>;
  std::vector<TermPairT> termMapping{};

  using GrammarRuleT = std::vector<GrammarRuleVariantT>;
  std::vector<GrammarRuleT> grammar{};
  NonTermId startSymbol{};

  using ParseChartT = std::vector<std::vector<std::vector<bool>>>;
  ParseChartT chart{};

  using RuleVariationsT = std::vector<BackPtrT>;
  using BackChartT = std::vector<std::vector<std::vector<RuleVariationsT>>>;
  BackChartT back{};

  using RuleDescT = std::pair<NonTermId, BackPtrT>;
  std::list<RuleDescT> serialized{};

  TokenInfoT tokenInfo{};
};

struct ArgParserT {
  std::vector<ArgInstanceInfoT> freeValues{};
  ArgInstanceDatabaseT options{};
  ArgInstanceDatabaseT flags{};

  ParsingDatabaseT database{};
  StateT currentState{};
  ModeT mode{};

  std::vector<ArgInstanceInfoT> *targetOption{};
  std::size_t errorPosition{};
};
} // namespace ap

namespace ap {
Result updateArguments(ArgParserT *const handle,
                       std::string const *const token,
                       std::size_t const position);

Result tracePostorderPath(ParsingDatabaseT *const database,
                          std::size_t const variant);

Result initParseChart(ParsingDatabaseT *const database,
                      std::string const *const input);

Result parseCYK(ParsingDatabaseT *const database,
                std::string const *const input);

Result handleState(ArgParserT *const handle,
                   std::string const *const token,
                   std::size_t const position,
                   bool * const skip);

Result fillParsingDatabaseWithAlphabet(ParsingDatabaseT *const database);
Result fillParsingDatabaseWithDigits(ParsingDatabaseT *const database);
Result fillParsingDatabaseWithMisc(ParsingDatabaseT *const database);
Result fillParsingDatabase(ParsingDatabaseT *const database);

Result checkThatAllOptionsAreAssigned(ArgParserT const *const handle);
} // namespace ap
