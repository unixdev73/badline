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
#include "requiredLoggerInterface.hpp"
#include "internals.hpp"

namespace ap {
int createArgParser(ArgParserT **const p, bool debug, sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  using sl::err;

  if (!p) {
    if (l)
      err(l, "The ArgParser parameter is a nullptr.");
    return Result::ErrorNullptrParameter;
  }

  *p = new ArgParserT{};
  if (!*p) {
    if (l)
      err(l, "Failed to allocate memory for the ArgParser.");
    return Result::ErrorMemoryAllocationFailure;
  }

  (*p)->debug = debug;
  return Result::Success;
}

void destroyArgParser(ArgParserT *const p) { delete p; }

int addFlag(ArgParserT *const parser, std::string const &lf, char const s,
            sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  return addArg(parser, ArgTypeT::Flag, lf, s, l);
}

int addOption(ArgParserT *const parser, std::string const &lf, char const s,
              sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  return addArg(parser, ArgTypeT::Option, lf, s, l);
}

int parse(ArgParserT *const p, char const *const *const input,
          std::size_t const begin, std::size_t const end, std::size_t *errPos,
          sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  using sl::err;

  if (auto r = validateParseParameters(p, input, begin, end, l);
      r != Result::Success)
    return r;
  if (begin == end)
    return Result::Success;

  for (std::size_t i = begin; i < end; ++i) {
    auto const tokens = lookAhead(input, end, i, 1);
    std::size_t const tokenPos = i - begin;
    std::string const token = input[i];
    bool skipToken{false};

    auto result = handleShortArg(p, tokenPos, &token, &tokens, &skipToken, l);
    if (result == Result::Success) {
      if (skipToken)
        ++i;
      continue;
    } else if (result != Result::TokenNotHandled) {
      if (errPos)
        *errPos = i;
      return result;
    }

    result = handleLongArg(p, tokenPos, &token, &tokens, &skipToken, l);
    if (result == Result::Success) {
      if (skipToken)
        ++i;
      continue;
    } else if (result != Result::TokenNotHandled) {
      if (errPos)
        *errPos = i;
      return result;
    }

    p->freeValues.push_back({.position = tokenPos, .value = token});
  }

  return Result::Success;
}

int getFlagOccurrence(ArgParserT *const p, std::string const &flag,
                      std::size_t *count, sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  using sl::err;

  if (!p) {
    if (l)
      err(l, "The ArgParser parameter is a nullptr.");
    return Result::ErrorNullptrParameter;
  }

  if (!count) {
    if (l)
      err(l, "The count parameter is a nullptr.");
    return Result::ErrorNullptrParameter;
  }

  if (p->flags.longForm.contains(flag))
    *count = p->flags.longForm.at(flag).size();
  else
    *count = 0;

  return Result::Success;
}

int getOptionValues(ArgParserT *const p, std::string const &opt,
                    std::vector<std::string> *const out, sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  using sl::err;

  if (!p) {
    if (l)
      err(l, "The ArgParser parameter is a nullptr.");
    return Result::ErrorNullptrParameter;
  }

  if (!out) {
    if (l)
      err(l, "The out parameter is a nullptr.");
    return Result::ErrorNullptrParameter;
  }

  if (!p->options.longForm.contains(opt)) {
    if (l)
      err(l, "The id: '" + opt + "' is not a valid option.");
    return Result::ErrorIdNotValid;
  }

  if (!p->options.longForm.at(opt).size()) {
    out->clear();
    return Result::Success;
  }

  auto &instances = p->options.longForm.at(opt);
  out->resize(instances.size());
  auto it = instances.begin();
  for (std::size_t i = 0; i < instances.size(); ++i) {
    out->at(i) = it->value;
    it = std::next(it);
  }

  return Result::Success;
}

int getFreeValues(ArgParserT *const p, std::vector<std::string> *const out,
                  sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  using sl::err;

  if (!p) {
    if (l)
      err(l, "The ArgParser parameter is a nullptr.");
    return Result::ErrorNullptrParameter;
  }

  if (!out) {
    if (l)
      err(l, "The out parameter is a nullptr.");
    return Result::ErrorNullptrParameter;
  }

  auto &instances = p->freeValues;
  out->resize(instances.size());
  auto it = instances.begin();
  for (std::size_t i = 0; i < instances.size(); ++i) {
    out->at(i) = it->value;
    it = std::next(it);
  }

  return Result::Success;
}
} // namespace ap
