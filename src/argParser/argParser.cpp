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
#include "private/argParser.hpp"

// PUBLIC API IMPLEMENTATION
namespace ap {
int createArgParser(ArgParser *const p) {
  if (auto ptr = new ArgParserT{}; !ptr)
    return 1;
  else
    *p = ptr;
  return 0;
}

void destroyArgParser(ArgParser const p) { delete p; }

UniqueArgParser createArgParser() {
  ArgParser p{};
  createArgParser(&p);
  return UniqueArgParser{p, destroyArgParser};
}

int addFlag(ArgParser const parser, std::string const &l, char const s) {
  return addArg(parser, ArgTypeT::Flag, l, s);
}

int addOption(ArgParser const parser, std::string const &l, char const s) {
  return addArg(parser, ArgTypeT::Option, l, s);
}

int parse(ArgParser const parser, InputBindingT const *const binding) {
  try {
    if (!parser)
      return 1;
    if (!binding)
      return 2;
    if (!binding->input)
      return 3;
    if (binding->begin > binding->end)
      return 4;
    if (binding->begin == binding->end)
      return 0;

    for (std::size_t i = binding->begin; i < std::size_t(binding->end); ++i) {
      std::string const token = binding->input[i];
      std::string next{}, *nextPtr{nullptr};
      if (i + 1 < std::size_t(binding->end)) {
        next = binding->input[i + 1];
        nextPtr = &next;
      }

      auto error = handleShortArg(parser, i - binding->begin, &token, nextPtr);
      if (error == 4)
        return 5;
      if (error == 5)
        ++i;
      if (!error || error == 5)
        continue;

      error = handleLongArg(parser, i - binding->begin, &token, nextPtr);
      if (error == 3)
        return 5;
      if (error == 4)
        ++i;
      if (!error || error == 4)
        continue;

      parser->freeValues.push_back(
          {.position = i - binding->begin, .value = token});
    }
  } catch (...) {
    return 255;
  }
  return 0;
}

int getFlagOccurrence(ArgParser const parser, std::string const &flag,
                      std::size_t *count) {
  if (!parser)
    return 1;
  if (!count)
    return 2;
  if (parser->flags.longForm.contains(flag))
    *count = parser->flags.longForm.at(flag).size();
  else
    *count = 0;
  return 0;
}

int getOptionValues(ArgParser const parser, std::string const &opt,
                    std::vector<std::string> *const out) {
  if (!parser)
    return 1;
  if (!out)
    return 2;
  if (!parser->options.longForm.contains(opt)) {
    out->clear();
    return 0;
  }

  auto &instances = parser->options.longForm.at(opt);
  out->resize(instances.size());
  auto it = instances.begin();
  for (std::size_t i = 0; i < instances.size(); ++i) {
    out->at(i) = it->value;
    it = std::next(it);
  }

  return 0;
}

int getFreeValues(ArgParser const parser, std::vector<std::string> *const out) {
  if (!parser)
    return 1;
  if (!out)
    return 2;

  auto &instances = parser->freeValues;
  out->resize(instances.size());
  auto it = instances.begin();
  for (std::size_t i = 0; i < instances.size(); ++i) {
    out->at(i) = it->value;
    it = std::next(it);
  }
  return 0;
}
} // namespace ap

// PRIVATE API IMPLEMENTATION
namespace ap {
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
                  std::string const *const id, std::string const *const value) {
  try {
    if (!parser)
      return 1;
    if (!id)
      return 2;
    if (id->size() < parser->longArgPrefix.size() + 1)
      return 2;
    if (!id->starts_with(parser->longArgPrefix))
      return 2;

    auto [key, assignedVal] = getArgVal(*id, parser->longArgPrefix.size());

    if (parser->flags.longForm.contains(key)) {
      auto &info = parser->flags.longForm.at(key);
      info.push_back({.position = pos, .value = ""});
      return 0;
    }

    if (parser->options.longForm.contains(key)) {
      auto &info = parser->options.longForm.at(key);
      if (assignedVal.empty()) {
        if (value && value->size()) {
          info.push_back({.position = pos, .value = *value});
          return 4;
        }
        return 3;
      }
      info.push_back({.position = pos, .value = assignedVal});
      return 0;
    }
  } catch (...) {
    return 255;
  }
  return 2;
}

int checkShortArgPreconditions(ArgParserT *const parser,
                               std::string const *const id) {
  if (!parser)
    return 1;
  if (!id || id->size() < 2)
    return 2;
  if (id->at(0) != parser->shortArgPrefix)
    return 2;
  if (id->size() == 2 && !std::isalpha(id->at(1)))
    return 2;
  if (id->size() < 4 && id->find('=') != std::string::npos)
    return 2;

  return 0;
}

int checkArgListPreconditions(ArgParserT *const parser, std::string const &key,
                              std::string const &assignedVal) {
  for (std::size_t i = 0; i < key.size() - 1; ++i) {
    if (!std::isalpha(key[i]))
      return 2;
    if (!parser->flags.shortForm.contains(key[i]))
      return 2;
  }

  if (!parser->flags.shortForm.contains(key.back()) &&
      !parser->options.shortForm.contains(key.back()))
    return 2;

  if (!assignedVal.empty() && !parser->options.shortForm.contains(key.back()))
    return 3;
  return 0;
}

int handleShortArg(ArgParserT *const parser, std::size_t const pos,
                   std::string const *const id,
                   std::string const *const value) {
  try {
    if (auto error = checkShortArgPreconditions(parser, id); error)
      return error;

    auto [key, assignedVal] = getArgVal(*id, 1);

    if (auto error = checkArgListPreconditions(parser, key, assignedVal); error)
      return error;

    for (std::size_t i = 0; i < key.size(); ++i) {
      ArgInstanceInfoT *info{};
      ArgTypeT t{};

      if (auto e = recognizeAndRegisterArg(parser, key[i], &info, &t); e)
        continue;

      info->position = pos;
      if (t != ArgTypeT::Option)
        continue;
      if (assignedVal.size()) {
        info->value = std::move(assignedVal);
        continue;
      }
      if (!value || value->empty()) {
        parser->options.shortForm.at(key[i])->pop_back();
        return 4;
      }
      info->value = *value;
    }
  } catch (...) {
    return 255;
  }
  return 0;
}

int recognizeAndRegisterArg(ArgParserT *const parser, char const id,
                            ArgInstanceInfoT **info, ArgTypeT *type) {
  try {
    if (!parser)
      return 1;
    if (!info)
      return 2;
    if (!type)
      return 4;

    if (parser->options.shortForm.contains(id)) {
      parser->options.shortForm.at(id)->push_back({});
      *info = &parser->options.shortForm.at(id)->back();
      *type = ArgTypeT::Option;
    } else if (parser->flags.shortForm.contains(id)) {
      parser->flags.shortForm.at(id)->push_back({});
      *info = &parser->flags.shortForm.at(id)->back();
      *type = ArgTypeT::Flag;
    } else
      return 3;
  } catch (...) {
    return 255;
  }

  return 0;
}

int addArg(ArgParserT *const parser, ArgTypeT const type,
           std::string const &longForm, char const shortForm) noexcept {
  try {
    if (!parser)
      return 1;

    if (longForm.empty())
      return 2;

    bool const isFlag = (type == ArgTypeT::Flag);
    auto &larg = isFlag ? parser->flags.longForm : parser->options.longForm;
    auto &sarg = isFlag ? parser->flags.shortForm : parser->options.shortForm;

    if (larg.contains(longForm))
      return 3;

    if (shortForm && sarg.contains(shortForm))
      return 4;

    for (auto const c : longForm)
      if (!std::isalnum(c))
        return 5;

    if (!std::isalnum(shortForm))
      return 6;

    larg.emplace(longForm, std::list<ArgInstanceInfoT>{});
    if (shortForm)
      sarg.emplace(shortForm, &larg.at(longForm));
  } catch (...) {
    return 255;
  }

  return 0;
}

int split(KeyValueT *const pair, std::string const *const input,
          char const delimiter) noexcept {
  try {
    if (!pair)
      return 1;
    if (!input)
      return 2;
    if (!input->size())
      return 0;

    std::size_t mark = input->find(delimiter);
    if (mark == std::string::npos) {
      pair->key = *input;
      return 0;
    }

    pair->key = std::string(*input, 0, mark);
    pair->value = std::string(*input, mark + 1, input->size() - mark - 1);
  } catch (...) {
    return 255;
  }

  return 0;
}
} // namespace ap
