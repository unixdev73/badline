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

namespace ap {
int validateParseParameters(ArgParserT *const parser,
                            char const *const *const input,
                            std::size_t const begin, std::size_t end) {
  if (!parser) {
    return Result::ErrorNullptrParameter;
  }

  if (!input) {
    return Result::ErrorNullptrParameter;
  }

  if (begin > end) {
    return Result::ErrorRangeBeginGreaterThanEnd;
  }
  return Result::Success;
}

std::vector<char const *> lookAhead(char const *const *input,
                                    std::size_t const inputSz, std::size_t i,
                                    std::size_t const n) {
  std::vector<char const *> tokens;
  std::size_t counter{};

  tokens.reserve(n);

  for (++i; i < inputSz && counter < n; ++i, ++counter) {
    tokens.push_back(input[i]);
  }

  return tokens;
}

std::pair<std::string, std::string> getArgVal(std::string const &token,
                                              std::size_t const offset) {
  std::string argPart(token, offset, token.size() - offset);
  KeyValueT kv{};
  split(&kv, &token, '=');
  if (!kv.value.empty())
    for (std::size_t i = 0; i < kv.value.size() + 1; ++i)
      argPart.pop_back();
  return {std::move(argPart), std::move(kv.value)};
}

int handleLongArg(ArgParserT *const parser, std::size_t const pos,
                  std::string const *const id,
                  std::vector<char const *> const *const tokens,
                  bool *const skipToken) {

  if (id->size() < parser->longArgPrefix.size() + 1)
    return Result::TokenNotHandled;
  if (!id->starts_with(parser->longArgPrefix))
    return Result::TokenNotHandled;

  auto [key, assignedVal] = getArgVal(*id, parser->longArgPrefix.size());

  if (parser->flags.longForm.contains(key)) {
    auto &info = parser->flags.longForm.at(key);
    info.push_back({.position = pos, .value = ""});
    return Result::Success;
  }

  if (parser->options.longForm.contains(key)) {
    auto &info = parser->options.longForm.at(key);
    if (assignedVal.empty()) {
      if (tokens && tokens->size()) {
        info.push_back({.position = pos, .value = tokens->front()});
        *skipToken = true;
        return Result::Success;
      }
      return Result::ErrorOptionHasNoValue;
    }

    info.push_back({.position = pos, .value = assignedVal});
    *skipToken = false;
    return Result::Success;
  }

  return Result::TokenNotHandled;
}

int checkShortArgPreconditions(ArgParserT *const parser,
                               std::string const *const id) {
  if (!id || id->size() < 2)
    return Result::TokenNotHandled;
  if (id->at(0) != parser->shortArgPrefix)
    return Result::TokenNotHandled;
  if (id->size() == 2 && !std::isalpha(id->at(1)))
    return Result::TokenNotHandled;
  if (id->size() < 4 && id->find('=') != std::string::npos)
    return Result::TokenNotHandled;

  return Result::Success;
}

int checkArgListPreconditions(ArgParserT *const parser, std::string const &key,
                              std::string const &assignedVal) {
  for (std::size_t i = 0; i < key.size() - 1; ++i) {
    if (!std::isalpha(key[i]))
      return Result::TokenNotHandled;
    if (!parser->flags.shortForm.contains(key[i]))
      return Result::TokenNotHandled;
  }

  if (!parser->flags.shortForm.contains(key.back()) &&
      !parser->options.shortForm.contains(key.back()))
    return Result::TokenNotHandled;

  if (!assignedVal.empty() && !parser->options.shortForm.contains(key.back()))
    return Result::ErrorFlagAssignedValue;
  return Result::Success;
}

int handleShortArg(ArgParserT *const parser, std::size_t const pos,
                   std::string const *const id,
                   std::vector<char const *> const *const tokens,
                   bool *const skipToken) {

  if (auto r = checkShortArgPreconditions(parser, id); r != Result::Success)
    return r;

  auto [key, assignedVal] = getArgVal(*id, 1);

  if (auto r = checkArgListPreconditions(parser, key, assignedVal);
      r != Result::Success)
    return r;

  for (std::size_t i = 0; i < key.size(); ++i) {
    ArgInstanceInfoT *info{};
    ArgTypeT t{};

    recognizeAndRegisterArg(parser, key[i], &info, &t);
    info->position = pos;

    if (t != ArgTypeT::Option)
      continue;
    if (assignedVal.size()) {
      info->value = std::move(assignedVal);
      continue;
    }

    if (!tokens || tokens->empty()) {
      parser->options.shortForm.at(key[i])->pop_back();
      return Result::ErrorOptionHasNoValue;
    }

    *skipToken = true;
    info->value = tokens->front();
  }

  return Result::Success;
}

int recognizeAndRegisterArg(ArgParserT *const parser, char const id,
                            ArgInstanceInfoT **info, ArgTypeT *type) {
  if (parser->options.shortForm.contains(id)) {
    parser->options.shortForm.at(id)->push_back({});
    *info = &parser->options.shortForm.at(id)->back();
    *type = ArgTypeT::Option;
  } else if (parser->flags.shortForm.contains(id)) {
    parser->flags.shortForm.at(id)->push_back({});
    *info = &parser->flags.shortForm.at(id)->back();
    *type = ArgTypeT::Flag;
  } else
    return Result::ErrorIdNotValid;
  return Result::Success;
}

int addArg(ArgParserT *const p, ArgTypeT const type,
           std::string const &longForm, char const shortForm) {

  if (longForm.empty()) {
    return Result::ErrorEmptyStringParameter;
  }

  bool const isFlag = (type == ArgTypeT::Flag);
  auto &larg = isFlag ? p->flags.longForm : p->options.longForm;
  auto &sarg = isFlag ? p->flags.shortForm : p->options.shortForm;

  if (larg.contains(longForm)) {
    return Result::ErrorIdAlreadyInUse;
  }

  if (shortForm && sarg.contains(shortForm)) {
    return Result::ErrorIdAlreadyInUse;
  }

  for (auto const c : longForm)
    if (!std::isalnum(c)) {
      return Result::ErrorStringNotValid;
    }

  if (!std::isalnum(shortForm)) {
    return Result::ErrorCharacterNotValid;
  }

  larg.emplace(longForm, std::list<ArgInstanceInfoT>{});
  if (shortForm)
    sarg.emplace(shortForm, &larg.at(longForm));

  return Result::Success;
}

int split(KeyValueT *const pair, std::string const *const input,
          char const delimiter) {
  if (!input->size())
    return Result::Success;

  std::size_t mark = input->find(delimiter);
  if (mark == std::string::npos) {
    pair->key = *input;
    return Result::Success;
  }

  pair->key = std::string(*input, 0, mark);
  pair->value = std::string(*input, mark + 1, input->size() - mark - 1);
  return Result::Success;
}
} // namespace ap
