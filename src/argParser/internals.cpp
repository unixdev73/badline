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
bool initParseChart(ArgParser *const parser, std::string const *const input) {
  auto const database = &parser->database;
  database->back = {
      input->size(),
      {input->size(), {database->grammar.size(), std::vector<BackPtr>{}}}};

  database->chart = {
      input->size(),
      {input->size(), std::vector<bool>(database->grammar.size(), false)}};

  for (std::size_t i = 0; i < input->size(); ++i) {
    char const tokenValue = (*input)[i];
    bool validToken = false;

    for (auto const &[nterms, terms] : database->termMapping) {
      for (auto const &term : terms) {
        if (term == tokenValue) {
          for (auto const &nterm : nterms) {
            database->back[0][i][nterm].push_back(
                BackPtr{.variant = 0,
                        .ruleLHS = RuleInfo{.identifier = nterm,
                                            .locationY = 0,
                                            .begin = i,
                                            .end = i + 1},
                        .ruleRHS = RuleInfo{}});
            database->chart[0][i][nterm] = true;
            validToken = true;
          }
        }
      }
    }

    if (!validToken) {
      auto const m =
          std::string{"initParseChart: Failed to map token: "} + tokenValue;
      setErrMsg(parser, m + " to nterm");
      return false;
    }
  }

  return true;
}

bool tracePostorderPath(ParsingDatabase *const database,
                        std::size_t const variant) {
  std::size_t const start = database->startSymbol;
  std::size_t const row = database->back.size() - 1;

  std::size_t currentRule{start};
  auto entry = database->back[row][0][start][variant];
  std::list<ParsingDatabase::RuleDesc> visitQueue{};

  do {
    while (entry.ruleLHS.locationY || entry.ruleRHS.end) {
      visitQueue.push_back({currentRule, entry});
      visitQueue.push_back({currentRule, entry});

      auto const l = entry.ruleLHS;
      auto const &el = database->back[l.locationY][l.begin][l.identifier];
      if (!el.size())
        break;

      currentRule = l.identifier;
      entry = el[0];
    }

    if (visitQueue.empty())
      return true;

    currentRule = visitQueue.back().first;
    entry = visitQueue.back().second;
    visitQueue.pop_back();

    if (visitQueue.size() && visitQueue.back().second == entry) {
      auto const r = entry.ruleRHS;
      auto const &el = database->back[r.locationY][r.begin][r.identifier];
      if (el.size()) {
        currentRule = r.identifier;
        entry = el[0];
      }
    } else {
      database->serialized.push_back({currentRule, entry});
      entry = {};
    }
  } while (visitQueue.size());

  return true;
}

bool handleArgList(ArgParser *const handle,
                   std::size_t const position,
                   std::string const *const token) {
  auto const &ti = handle->database.tokenInfo;
  auto const &op = handle->options;
  auto const &fl = handle->flags;
  auto &to = handle->targetOption;

  std::string errMsg{};
  bool result = true;

  for (std::size_t i = 0; i < ti.argName.size() - 1; ++i) {
    if (!fl.shortForm.contains(ti.argName[i])) {
      errMsg = "handleArgList: Expected an argument list token";
      result = false;
      break;
    }
  }

  if (result) {
    if (!fl.shortForm.contains(ti.argName.back()) &&
        !op.shortForm.contains(ti.argName.back())) {
      errMsg = "handleArgList: Expected an argument list token";
      result = false;
    }
  }

  if (result) {
    for (std::size_t i = 0; i < ti.argName.size() - 1; ++i)
      fl.shortForm.at(ti.argName[i])->push_back({position, ""});

    if (fl.shortForm.contains(ti.argName.back()))
      fl.shortForm.at(ti.argName.back())->push_back({position, ""});
    else {
      op.shortForm.at(ti.argName.back())->push_back({position, ""});
      if (ti.argVal.size())
        op.shortForm.at(ti.argName.back())->back().value = ti.argVal;
      else {
        handle->currentState = State::HandleOptionValue;
        to = op.shortForm.at(ti.argName.back());
      }
    }
  }

  if (!result) {
    if (handle->mode == Mode::Lenient) {
      handle->freeValues.push_back({position, *token});
      result = true;
    }
  }

  if (!result)
    setErrMsg(handle, errMsg);
  return result;
}

bool handleLongArg(ArgParser *const handle, std::size_t const position) {
  auto const &ti = handle->database.tokenInfo;
  auto const &op = handle->options;
  auto const &fl = handle->flags;
  auto &to = handle->targetOption;

  if (op.longForm.contains(ti.argName)) {
    op.longForm.at(ti.argName)->push_back({position, ""});
    if (ti.argVal.size())
      op.longForm.at(ti.argName)->back().value = ti.argVal;
    else {
      handle->currentState = State::HandleOptionValue;
      to = op.longForm.at(ti.argName).get();
    }
  }

  else if (fl.longForm.contains(ti.argName))
    fl.longForm.at(ti.argName)->push_back({position, ""});

  else {
    setErrMsg(handle, "handleLongArg: The argument long form is not valid");
    return false;
  }

  return true;
}

bool handleShortArg(ArgParser *const handle, std::size_t const position) {
  auto const &ti = handle->database.tokenInfo;
  auto const &op = handle->options;
  auto const &fl = handle->flags;
  auto &to = handle->targetOption;

  if (op.shortForm.contains(ti.argName[0])) {
    op.shortForm.at(ti.argName[0])->push_back({position, ""});
    if (ti.argVal.size())
      op.shortForm.at(ti.argName[0])->back().value = ti.argVal;
    else {
      handle->currentState = State::HandleOptionValue;
      to = op.shortForm.at(ti.argName[0]);
    }
  }

  else if (fl.shortForm.contains(ti.argName[0]))
    fl.shortForm.at(ti.argName[0])->push_back({position, ""});

  else {
    setErrMsg(handle, "handleShortArg: The argument short form is not valid");
    return false;
  }

  return true;
}

bool updateArguments(ArgParser *const handle,
                     std::string const *const token,
                     std::size_t const position) {
  auto const &g = handle->database.grammar;

  for (auto const &[rule, info] : handle->database.serialized) {
    auto const action = g[rule][info.variant].semanticAction;
    if (action) {
      action(*token,
             info.ruleLHS.begin,
             info.ruleLHS.end,
             info.ruleRHS.begin,
             info.ruleRHS.end);
    }
  }

  if (handle->database.tokenInfo.isFreeVal) {
    handle->freeValues.push_back({position, *token});
    return true;
  }

  else if (handle->database.tokenInfo.isArgList) {
    if (!handleArgList(handle, position, token))
      return false;
  }

  else if (handle->database.tokenInfo.argName.size() == 1) {
    if (!handleShortArg(handle, position))
      return false;
  }

  else if (!handleLongArg(handle, position))
    return false;

  return true;
}

bool parseCYK(ArgParser *const parser, std::string const *const input) {
  auto const database = &parser->database;
  if (!initParseChart(parser, input))
    return false;

  auto const start = database->startSymbol;
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
                   {lhs, it, col, col + it + 1},
                   {rhs, row - it - 1, col + it + 1, col + row + 1}});
              chart[row][col][nTerm] = true;
            }
          }
        }
      }
    }
  }

  if (chart[input->size() - 1][0][start])
    return true;

  setErrMsg(parser,
            "parseCYK: The start symbol was not derived from the input");
  return false;
}

bool handleState(ArgParser *const handle,
                 std::string const *const token,
                 std::size_t const position,
                 bool *const skip) {
  *skip = true;

  if (*token == "--" && handle->currentState != State::HandleOptionRogueValue) {
    if (handle->currentState == State::HandleOptionValue)
      handle->currentState = State::HandleOptionRogueValue;
    else
      handle->currentState = State::HandleRogueFreeValue;
  }

  else if (handle->currentState == State::HandleRogueFreeValue) {
    handle->freeValues.push_back({position, *token});
    handle->currentState = State::ParseInputToken;
  }

  else if (handle->currentState == State::HandleOptionValue ||
           handle->currentState == State::HandleOptionRogueValue) {
    if ((*token)[0] == '-' &&
        handle->currentState != State::HandleOptionRogueValue) {
      handle->errorPosition = position;
      setErrMsg(handle, "handleState: Every option requires a value");
      return false;
    }
    handle->targetOption->back().value = *token;
    handle->currentState = State::ParseInputToken;
  }

  else if (token->size() == 1)
    handle->freeValues.push_back({position, *token});

  else
    *skip = false;

  return true;
}

bool createGrammar(ParsingDatabase *const database) {
  using R = GrammarRule::Identifier;
  auto &g = database->grammar;
  g.resize(R::Size);

  auto &info = database->tokenInfo;
  auto addNameR = [&info](std::string const &input,
                          std::size_t const,
                          std::size_t const,
                          std::size_t const beginB,
                          std::size_t const endB) {
    info.argName += input.substr(beginB, endB - beginB);
  };

  auto argListAddNameR = [&info](std::string const &input,
                                 std::size_t const,
                                 std::size_t const,
                                 std::size_t const beginB,
                                 std::size_t const endB) {
    info.argName = input.substr(beginB, endB - beginB);
    info.isArgList = true;
  };

  auto mergeExt = [&info](std::string const &,
                          std::size_t const,
                          std::size_t const,
                          std::size_t const,
                          std::size_t const) { info.argName += info.argExt; };

  auto addExt = [&info](std::string const &input,
                        std::size_t const beginA,
                        std::size_t const endA,
                        std::size_t const beginB,
                        std::size_t const endB) {
    std::string ext = input.substr(beginA, endA - beginA) +
                      input.substr(beginB, endB - beginB);
    info.argExt += ext;
  };

  auto assignR = [&info](std::string const &input,
                         std::size_t const,
                         std::size_t const,
                         std::size_t const beginB,
                         std::size_t const endB) {
    info.argVal = input.substr(beginB, endB - beginB);
  };

  auto freeVal = [&info](std::string const &,
                         std::size_t const,
                         std::size_t const,
                         std::size_t const,
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
  return true;
}

bool fillParsingDatabase(ParsingDatabase *const database) {
  database->startSymbol = GrammarRule::Identifier::Start;
  database->termMapping.reserve(500);

  if (!fillParsingDatabaseWithAlphabet(database))
    return false;

  if (!fillParsingDatabaseWithDigits(database))
    return false;

  if (!fillParsingDatabaseWithMisc(database))
    return false;

  if (!createGrammar(database))
    return false;

  return true;
}

bool fillParsingDatabaseWithMisc(ParsingDatabase *const database) {
  auto &mapping = database->termMapping;
  using R = GrammarRule::Identifier;
  mapping.push_back({{R::ShortArgPrefix}, {'-'}});
  mapping.push_back({{R::Comma}, {','}});
  mapping.push_back({{R::AssignmentOp}, {'='}});
  mapping.push_back({{R::Underscore}, {'_'}});

  std::vector<ParsingDatabase::TermId> terms{};
  terms.reserve(128);

  for (std::size_t i = 32; i < 127; ++i)
    if (char(i) != '-')
      terms.push_back(char(i));

  mapping.push_back({{R::NonShortArgPrefix, R::Printable}, std::move(terms)});
  mapping.push_back({{R::Printable}, {'-'}});

  terms.clear();
  terms.reserve(128);

  for (std::size_t i = 33; i < 48; ++i)
    terms.push_back(char(i));
  for (std::size_t i = 58; i < 65; ++i)
    terms.push_back(char(i));
  for (std::size_t i = 123; i < 127; ++i)
    terms.push_back(char(i));

  mapping.push_back({{R::NonAlnum}, std::move(terms)});
  return true;
}

bool fillParsingDatabaseWithDigits(ParsingDatabase *const database) {
  std::vector<ParsingDatabase::TermId> digits{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  auto &mapping = database->termMapping;

  mapping.push_back(
      {{GrammarRule::Identifier::Digit, GrammarRule::Identifier::Alnum},
       std::move(digits)});
  return true;
}

bool fillParsingDatabaseWithAlphabet(ParsingDatabase *const database) {
  std::vector<ParsingDatabase::TermId> alphabet{
      char('a'), char('b'), char('c'), char('d'), char('e'), char('f'),
      char('g'), char('h'), char('i'), char('j'), char('k'), char('l'),
      char('m'), char('n'), char('o'), char('p'), char('q'), char('r'),
      char('s'), char('t'), char('u'), char('v'), char('w'), char('x'),
      char('y'), char('z')};

  auto &mapping = database->termMapping;

  mapping.push_back({{GrammarRule::Identifier::SmallLetter,
                      GrammarRule::Identifier::Letter,
                      GrammarRule::Identifier::Alnum},
                     std::move(alphabet)});

  std::vector<ParsingDatabase::TermId> Alphabet{
      char('A'), char('B'), char('C'), char('D'), char('E'), char('F'),
      char('G'), char('H'), char('I'), char('J'), char('K'), char('L'),
      char('M'), char('N'), char('O'), char('P'), char('Q'), char('R'),
      char('S'), char('T'), char('U'), char('V'), char('W'), char('X'),
      char('Y'), char('Z')};

  mapping.push_back({{GrammarRule::Identifier::BigLetter,
                      GrammarRule::Identifier::Letter,
                      GrammarRule::Identifier::Alnum},
                     std::move(Alphabet)});
  return true;
}

bool areOptionsAssigned(ArgParser const *const handle) {
  auto const &opts = handle->options.longForm;
  for (auto const &opt : opts) {
    auto const &instances = *opt.second;
    for (auto const &inst : instances)
      if (inst.value.empty()) {
        setErrMsg(handle,
                  "The option: " + opt.first + " was not given a value");
        return false;
      }
  }

  return true;
}

bool GrammarRule::toString(std::size_t const id, std::string *const output) {
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
    return false;
  }

  return true;
}
} // namespace ap
