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

#include <badline/argParser.hpp>
#include "internals.hpp"
#include <list>

namespace ap {
int initParseChart(ParsingDatabaseT *const database,
                   std::string const *const input) {
  database->back = std::vector<std::vector<std::vector<std::vector<BackPtrT>>>>{
      input->size(), std::vector<std::vector<std::vector<BackPtrT>>>{
                         input->size(), std::vector<std::vector<BackPtrT>>(
                                            database->grammar.size(),
                                            std::vector<BackPtrT>{})}};

  database->chart = std::vector<std::vector<std::vector<bool>>>{
      input->size(),
      std::vector<std::vector<bool>>{
          input->size(), std::vector<bool>(database->grammar.size(), false)}};

  for (std::size_t i = 0; i < input->size(); ++i) {
    bool validToken = false;
    for (auto const &[nterm, term] : database->termMapping) {
      if (term == (*input)[i]) {
        database->back[0][i][nterm].push_back(
            BackPtrT{.variant = 0,
                     .splitPoint = i,
                     .ruleLHS = RuleInfoT{.identifier = nterm,
                                          .locationY = 0,
                                          .locationX = i,
                                          .begin = i,
                                          .end = i + 1},
                     .ruleRHS = RuleInfoT{.identifier = 0,
                                          .locationY = 0,
                                          .locationX = 0,
                                          .begin = 0,
                                          .end = 0}});
        database->chart[0][i][nterm] = true;
        validToken = true;
      }
    }
    if (!validToken)
      return Result::ErrorTermTokenNotValid;
  }

  return Result::Success;
}

int tracePostorderPath(ParsingDatabaseT *const database,
                       std::size_t const variant) {
  std::size_t const start = GrammarRuleT::Identifier::Start;
  std::size_t const row = database->back.size() - 1;

  GrammarRuleT::Identifier currentRule{GrammarRuleT::Identifier::Start};
  auto entry = database->back[row][0][start][variant];
  std::list<ParsingDatabaseT::RuleDescT> visitQueue{};

  do {
    while (entry.ruleLHS.locationY || entry.ruleRHS.end) {
      visitQueue.push_back({currentRule, entry});
      visitQueue.push_back({currentRule, entry});

      auto const l = entry.ruleLHS;
      auto const &el = database->back[l.locationY][l.locationX][l.identifier];
      if (!el.size())
        break;

      currentRule = static_cast<GrammarRuleT::Identifier>(l.identifier);
      entry = el[0];
    }

    if (visitQueue.empty())
      return Result::Success;

    currentRule =
        static_cast<GrammarRuleT::Identifier>(visitQueue.back().first);
    entry = visitQueue.back().second;
    visitQueue.pop_back();

    if (visitQueue.size() && visitQueue.back().second == entry) {
      auto const r = entry.ruleRHS;
      auto const &el = database->back[r.locationY][r.locationX][r.identifier];
      if (el.size()) {
        currentRule = static_cast<GrammarRuleT::Identifier>(r.identifier);
        entry = el[0];
      }
    } else {
      database->serialized.push_back({currentRule, entry});
      entry = {};
    }
  } while (visitQueue.size());

  return Result::Success;
}

int handleArgList(ArgParserT *const handle, std::size_t const position,
                  std::string const *const token) {
  auto const &ti = handle->database.tokenInfo;
  auto const &op = handle->options;
  auto const &fl = handle->flags;
  auto &to = handle->targetOption;

  int result = Result::Success;

  for (std::size_t i = 0; i < ti.argName.size() - 1; ++i) {
    if (!fl.shortForm.contains(ti.argName[i])) {
      result = Result::ErrorExpectedArgListToken;
      break;
    }
  }

  if (result == Result::Success) {
    if (!fl.shortForm.contains(ti.argName.back()) &&
        !op.shortForm.contains(ti.argName.back()))
      result = Result::ErrorExpectedArgListToken;
  }

  if (result == Result::Success) {
    for (std::size_t i = 0; i < ti.argName.size() - 1; ++i)
      fl.shortForm.at(ti.argName[i])->push_back({position, ""});

    if (fl.shortForm.contains(ti.argName.back()))
      fl.shortForm.at(ti.argName.back())->push_back({position, ""});
    else {
      op.shortForm.at(ti.argName.back())->push_back({position, ""});
      if (ti.argVal.size())
        op.shortForm.at(ti.argName.back())->back().value = ti.argVal;
      else {
        handle->currentState = StateT::HandleOptionValue;
        to = op.shortForm.at(ti.argName.back());
      }
    }
  }

  if (result == Result::ErrorExpectedArgListToken) {
    if (handle->mode == ModeT::Lenient) {
      handle->freeValues.push_back({position, *token});
      result = Result::Success;
    }
  }

  return result;
}

int handleLongArg(ArgParserT *const handle, std::size_t const position) {
  auto const &ti = handle->database.tokenInfo;
  auto const &op = handle->options;
  auto const &fl = handle->flags;
  auto &to = handle->targetOption;

  if (op.longForm.contains(ti.argName)) {
    op.longForm.at(ti.argName)->push_back({position, ""});
    if (ti.argVal.size())
      op.longForm.at(ti.argName)->back().value = ti.argVal;
    else {
      handle->currentState = StateT::HandleOptionValue;
      to = op.longForm.at(ti.argName).get();
    }
  }

  else if (fl.longForm.contains(ti.argName))
    fl.longForm.at(ti.argName)->push_back({position, ""});

  else
    return Result::ErrorArgLongFormNotValid;

  return Result::Success;
}

int handleShortArg(ArgParserT *const handle, std::size_t const position) {
  auto const &ti = handle->database.tokenInfo;
  auto const &op = handle->options;
  auto const &fl = handle->flags;
  auto &to = handle->targetOption;

  if (op.shortForm.contains(ti.argName[0])) {
    op.shortForm.at(ti.argName[0])->push_back({position, ""});
    if (ti.argVal.size())
      op.shortForm.at(ti.argName[0])->back().value = ti.argVal;
    else {
      handle->currentState = StateT::HandleOptionValue;
      to = op.shortForm.at(ti.argName[0]);
    }
  }

  else if (fl.shortForm.contains(ti.argName[0]))
    fl.shortForm.at(ti.argName[0])->push_back({position, ""});

  else
    return Result::ErrorArgShortFormNotValid;

  return Result::Success;
}

int updateArguments(ArgParserT *const handle, std::string const *const token,
                    std::size_t const position) {
  auto const &g = handle->database.grammar;

  for (auto const &[rule, info] : handle->database.serialized) {
    auto const action = g[rule][info.variant].semanticAction;
    if (action) {
      action(*token, info.ruleLHS.begin, info.ruleLHS.end, info.ruleRHS.begin,
             info.ruleRHS.end);
    }
  }

  if (handle->database.tokenInfo.isFreeVal) {
    handle->freeValues.push_back({position, *token});
    return Result::Success;
  }

  else if (handle->database.tokenInfo.isArgList) {
    if (auto result = handleArgList(handle, position, token);
        result != Result::Success)
      return result;
  }

  else if (handle->database.tokenInfo.argName.size() == 1) {
    if (auto result = handleShortArg(handle, position);
        result != Result::Success)
      return result;
  }

  else if (auto result = handleLongArg(handle, position);
           result != Result::Success)
    return result;

  return Result::Success;
}

int parseCYK(ParsingDatabaseT *const database, std::string const *const input) {
  if (auto code = initParseChart(database, input); code != Result::Success)
    return code;

  auto const &g = database->grammar;
  auto &chart = database->chart;
  auto &back = database->back;

  for (std::size_t row = 1; row < input->size(); ++row) {
    for (std::size_t col = 0; col < input->size() - row; ++col) {
      for (std::size_t it = 0; it < row; ++it) {
        for (std::size_t nTerm = 0; nTerm < g.size(); ++nTerm) {
          back[row][col][nTerm].reserve(g[nTerm].size());
          for (std::size_t variant = 0; variant < g[nTerm].size(); ++variant) {
            auto const &[lhs, rhs, cb] = g[nTerm][variant];
            if (chart[it][col][lhs] && chart[row - it - 1][col + it + 1][rhs]) {
              back[row][col][nTerm].push_back(
                  {variant,
                   it,
                   {lhs, it, col, col, col + it + 1},
                   {rhs, row - it - 1, col + it + 1, col + it + 1,
                    col + row + 1}});
              chart[row][col][nTerm] = true;
            }
          }
        }
      }
    }
  }

  if (chart[input->size() - 1][0][GrammarRuleT::Identifier::Start])
    return Result::Success;
  return Result::ErrorStartSymbolNotDerivedFromInput;
}

int createGrammar(ParsingDatabaseT *const database) {
  using R = GrammarRuleT::Identifier;
  auto &g = database->grammar;
  g.resize(R::Size);

  auto &info = database->tokenInfo;
  auto addNameR = [&info](std::string const &input, std::size_t const,
                          std::size_t const, std::size_t const beginB,
                          std::size_t const endB) {
    info.argName += input.substr(beginB, endB - beginB);
  };

  auto argListAddNameR = [&info](std::string const &input, std::size_t const,
                                 std::size_t const, std::size_t const beginB,
                                 std::size_t const endB) {
    info.argName = input.substr(beginB, endB - beginB);
    info.isArgList = true;
  };

  auto mergeExt = [&info](std::string const &, std::size_t const,
                          std::size_t const, std::size_t const,
                          std::size_t const) { info.argName += info.argExt; };

  auto addExt = [&info](std::string const &input, std::size_t const beginA,
                        std::size_t const endA, std::size_t const beginB,
                        std::size_t const endB) {
    std::string ext = input.substr(beginA, endA - beginA) +
                      input.substr(beginB, endB - beginB);
    info.argExt += ext;
  };

  auto assignR = [&info](std::string const &input, std::size_t const,
                         std::size_t const, std::size_t const beginB,
                         std::size_t const endB) {
    info.argVal = input.substr(beginB, endB - beginB);
  };

  auto freeVal = [&info](std::string const &, std::size_t const,
                         std::size_t const, std::size_t const,
                         std::size_t const) { info.isFreeVal = true; };

  g[R::ArgTerm] = {{R::ShortArgPrefix, R::ShortArgPrefix}};

  g[R::LongArgPrefix] = {{R::ShortArgPrefix, R::ShortArgPrefix}};

  g[R::AlnumString] = {{R::Alnum, R::Alnum}, {R::Alnum, R::AlnumString}};

  g[R::PrintableString] = {{R::Printable, R::Printable},
                           {R::Printable, R::PrintableString}};

  g[R::ShortArg] = {{R::ShortArgPrefix, R::Alnum, addNameR}};

  g[R::CompoundArg] = {{R::ShortArgPrefix, R::AlnumString, argListAddNameR}};

  g[R::SimpleLongArg] = {{R::LongArgPrefix, R::Alnum, addNameR},
                         {R::LongArgPrefix, R::AlnumString, addNameR}};

  g[R::UnderscoreExtension] = {{R::Underscore, R::AlnumString, addExt},
                               {R::Underscore, R::Alnum, addExt}};

  g[R::DashExtension] = {{R::ShortArgPrefix, R::AlnumString, addExt},
                         {R::ShortArgPrefix, R::Alnum, addExt}};

  g[R::LongArgExtension] = {{R::Underscore, R::AlnumString, addExt},
                            {R::Underscore, R::Alnum, addExt},
                            {R::ShortArgPrefix, R::Alnum, addExt},
                            {R::ShortArgPrefix, R::AlnumString, addExt},
                            {R::UnderscoreExtension, R::LongArgExtension},
                            {R::DashExtension, R::LongArgExtension}};

  g[R::LongArg] = {{R::SimpleLongArg, R::LongArgExtension, mergeExt},
                   {R::LongArgPrefix, R::Alnum, addNameR},
                   {R::LongArgPrefix, R::AlnumString, addNameR}};

  g[R::FreeValue] = {{R::NonShortArgPrefix, R::PrintableString}};

  g[R::AssignmentRight] = {{R::AssignmentOp, R::PrintableString, assignR}};

  g[R::Start] = {{R::LongArgPrefix, R::Alnum, addNameR},
                 {R::LongArgPrefix, R::AlnumString, addNameR},
                 {R::SimpleLongArg, R::LongArgExtension, mergeExt},
                 {R::ShortArgPrefix, R::Alnum, addNameR},
                 {R::ShortArgPrefix, R::AlnumString, argListAddNameR},
                 {R::NonShortArgPrefix, R::PrintableString, freeVal},
                 {R::CompoundArg, R::AssignmentRight},
                 {R::LongArg, R::AssignmentRight},
                 {R::ShortArg, R::AssignmentRight}};
  return Result::Success;
}

int fillParsingDatabase(ParsingDatabaseT *const database) {
  database->termMapping.reserve(500);

  fillParsingDatabaseWithAlphabet(database);
  fillParsingDatabaseWithDigits(database);
  fillParsingDatabaseWithMisc(database);
  createGrammar(database);

  return Result::Success;
}

int fillParsingDatabaseWithMisc(ParsingDatabaseT *const database) {
  auto &mapping = database->termMapping;
  using R = GrammarRuleT::Identifier;
  mapping.push_back({R::ShortArgPrefix, '-'});
  mapping.push_back({R::Comma, ','});
  mapping.push_back({R::AssignmentOp, '='});
  mapping.push_back({R::Underscore, '_'});

  for (std::size_t i = 33; i < 127; ++i) {
    mapping.push_back({R::Printable, char(i)});
    if (char(i) != '-')
      mapping.push_back({R::NonShortArgPrefix, char(i)});
  }

  for (std::size_t i = 33; i < 48; ++i)
    mapping.push_back({R::NonAlnum, char(i)});
  for (std::size_t i = 58; i < 65; ++i)
    mapping.push_back({R::NonAlnum, char(i)});
  for (std::size_t i = 123; i < 127; ++i)
    mapping.push_back({R::NonAlnum, char(i)});
  return Result::Success;
}

int fillParsingDatabaseWithDigits(ParsingDatabaseT *const database) {
  std::string const digits{"0123456789"};
  auto &mapping = database->termMapping;

  for (char c : digits) {
    mapping.push_back({GrammarRuleT::Identifier::Digit, c});
    mapping.push_back({GrammarRuleT::Identifier::Alnum, c});
  }
  return Result::Success;
}

int fillParsingDatabaseWithAlphabet(ParsingDatabaseT *const database) {
  std::string const alphabet{"abcdefghijklmnopqrstuvwxyz"};
  auto &mapping = database->termMapping;

  for (char c : alphabet) {
    mapping.push_back({GrammarRuleT::Identifier::SmallLetter, c});
    mapping.push_back({GrammarRuleT::Identifier::Letter, c});
    mapping.push_back({GrammarRuleT::Identifier::Alnum, c});
  }

  for (char c : alphabet) {
    mapping.push_back({GrammarRuleT::Identifier::BigLetter, std::toupper(c)});
    mapping.push_back({GrammarRuleT::Identifier::Letter, c});
    mapping.push_back({GrammarRuleT::Identifier::Alnum, c});
  }
  return Result::Success;
}

int split(std::string const *const input, char const delimiter,
          std::pair<std::string, std::string> *const output) {
  if (!input->size())
    return Result::Success;

  std::size_t mark = input->find(delimiter);
  if (mark != std::string::npos) {
    output->first = std::string(*input, 0, mark);
    output->second = std::string(*input, mark + 1, input->size() - mark - 1);
  } else
    output->first = *input;

  return Result::Success;
}

int GrammarRuleT::toString(std::size_t const id, std::string *const output) {
  switch (id) {
  case Identifier::ShortArgPrefix:
    *output = "ShortArgPrefix";
    break;
  case Identifier::AssignmentOp:
    *output = "AssignmentOp";
    break;
  case Identifier::Comma:
    *output = "Comma";
    break;
  case Identifier::Digit:
    *output = "Digit";
    break;
  case Identifier::Underscore:
    *output = "Underscore";
    break;
  case Identifier::SmallLetter:
    *output = "SmallLetter";
    break;
  case Identifier::BigLetter:
    *output = "BigLetter";
    break;
  case Identifier::Letter:
    *output = "Letter";
    break;
  case Identifier::Alnum:
    *output = "Alnum";
    break;
  case Identifier::NonAlnum:
    *output = "NonAlnum";
    break;
  case Identifier::Printable:
    *output = "Printable";
    break;
  case Identifier::ArgTerm:
    *output = "ArgTerm";
    break;
  case Identifier::LongArgPrefix:
    *output = "LongArgPrefix";
    break;
  case Identifier::ShortArg:
    *output = "ShortArg";
    break;
  case Identifier::AlnumString:
    *output = "AlnumString";
    break;
  case Identifier::PrintableString:
    *output = "PrintableString";
    break;
  case Identifier::SimpleLongArg:
    *output = "SimpleLongArg";
    break;
  case Identifier::LongArg:
    *output = "LongArg";
    break;
  case Identifier::LongArgExtension:
    *output = "LongArgExtension";
    break;
  case Identifier::UnderscoreExtension:
    *output = "UnderscoreExtension";
    break;
  case Identifier::DashExtension:
    *output = "DashExtension";
    break;
  case Identifier::AssignmentRight:
    *output = "AssignmentRight";
    break;
  case Identifier::ArgAssignment:
    *output = "ArgAssignment";
    break;
  case Identifier::CompoundArg:
    *output = "CompoundArg";
    break;
  case Identifier::Start:
    *output = "Start";
    break;
  default:
    return Result::ErrorRuleIdentifierNotValid;
  }

  return Result::Success;
}
} // namespace ap
