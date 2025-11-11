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
int createArgParser(ArgParserT **const handle) {
  if (!handle)
    return Result::ErrorNullptrHandle;

  if (auto parser = new ArgParserT{}; parser)
    *handle = parser;
  else
    return Result::ErrorMemoryAllocationFailure;

  fillParsingDatabase(&(*handle)->database);
  return Result::Success;
}

void destroyArgParser(ArgParserT const *const handle) { delete handle; }

int addFlag(ArgParserT *const handle, std::string const &argLongForm,
            char const argShortForm) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (argLongForm.empty())
    return Result::ErrorArgLongFormNotValid;
  if (handle->flags.longForm.contains(argLongForm))
    return Result::ErrorArgLongFormNotUnique;
  if (argShortForm && handle->flags.shortForm.contains(argShortForm))
    return Result::ErrorArgShortFormNotUnique;

  auto &shortFormDB = handle->flags.shortForm;
  auto &longFormDB = handle->flags.longForm;
  longFormDB.emplace(argLongForm,
                     std::make_unique<std::vector<ArgInstanceInfoT>>());
  if (argShortForm)
    shortFormDB.emplace(argShortForm, longFormDB.at(argLongForm).get());
  return Result::Success;
}

int addOption(ArgParserT *const handle, std::string const &argLongForm,
              char const argShortForm) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (argLongForm.empty())
    return Result::ErrorArgLongFormNotValid;
  if (handle->options.longForm.contains(argLongForm))
    return Result::ErrorArgLongFormNotUnique;
  if (argShortForm && handle->options.shortForm.contains(argShortForm))
    return Result::ErrorArgShortFormNotUnique;

  auto &shortFormDB = handle->options.shortForm;
  auto &longFormDB = handle->options.longForm;
  longFormDB.emplace(argLongForm,
                     std::make_unique<std::vector<ArgInstanceInfoT>>());
  if (argShortForm)
    shortFormDB.emplace(argShortForm, longFormDB.at(argLongForm).get());
  return Result::Success;
}

int parse(ArgParserT *const handle, char const *const *const input,
          std::size_t const begin, std::size_t const end) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!input)
    return Result::ErrorNullptrInput;
  if (begin >= end)
    return Result::ErrorBeginEndRangeNotValid;

  for (std::size_t i = begin; i < end; ++i) {
    handle->database.back.clear();
    handle->database.chart.clear();
    handle->database.serialized.clear();
    handle->database.tokenInfo = {};
    std::string const token = input[i];
    std::size_t const pos = i - begin;

    if (token == "--") {
      if (handle->currentState == StateT::HandleOptionValue)
        handle->currentState = StateT::HandleOptionRogueValue;
      else
        handle->currentState = StateT::HandleRogueFreeValue;
      continue;
    }

    if (handle->currentState == StateT::HandleRogueFreeValue) {
      handle->freeValues.push_back({pos, token});
      handle->currentState = StateT::ParseInputToken;
      continue;
    }

    if (handle->currentState == StateT::HandleOptionValue ||
        handle->currentState == StateT::HandleOptionRogueValue) {
      if (token[0] == '-' &&
          handle->currentState != StateT::HandleOptionRogueValue) {
        handle->errorPosition = pos;
        return Result::ErrorOptionRequiresValue;
      }
      handle->targetOption->back().value = token;
      handle->currentState = StateT::ParseInputToken;
      continue;
    }

    if (std::string{token}.size() == 1) {
      handle->freeValues.push_back({pos, token});
      continue;
    }

    if (auto r = parseCYK(&handle->database, &token); r != Result::Success)
      return r;
    if (auto r = tracePostorderPath(&handle->database, 0); r != Result::Success)
      return r;
    if (auto r = updateArguments(handle, &token, pos); r != Result::Success)
      return r;
  }

  return Result::Success;
}

int getErrorPosition(ArgParserT *const handle, std::size_t *const output) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!output)
    return Result::ErrorNullptrOutput;
  *output = handle->errorPosition;
  return Result::Success;
}

int getFlagCount(ArgParserT const *const handle, std::string const &argLongForm,
                 std::size_t *const count) {
  if (!handle)
    return Result::ErrorNullptrHandle;

  if (!count)
    return Result::ErrorNullptrCount;

  if (handle->flags.longForm.contains(argLongForm))
    *count = handle->flags.longForm.at(argLongForm)->size();
  else
    *count = 0;

  return Result::Success;
}

int getFlagInstancePosition(ArgParserT const *const handle,
                            std::string const &argLongForm,
                            std::size_t const instanceIndex,
                            std::size_t *const position) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!position)
    return Result::ErrorNullptrPosition;
  if (!handle->flags.longForm.contains(argLongForm))
    return Result::ErrorArgLongFormNotValid;

  auto const &instances = handle->flags.longForm.at(argLongForm);
  if (instanceIndex >= instances->size())
    return Result::ErrorInstanceIndexNotValid;

  *position = instances->at(instanceIndex).position;
  return Result::Success;
}

int getOptionCount(ArgParserT const *const handle,
                   std::string const &argLongForm, std::size_t *const count) {
  if (!handle)
    return Result::ErrorNullptrHandle;

  if (!count)
    return Result::ErrorNullptrCount;

  if (handle->options.longForm.contains(argLongForm))
    *count = handle->options.longForm.at(argLongForm)->size();
  else
    *count = 0;

  return Result::Success;
}

int getOptionInstancePosition(ArgParserT const *const handle,
                              std::string const &argLongForm,
                              std::size_t const instanceIndex,
                              std::size_t *const position) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!position)
    return Result::ErrorNullptrPosition;
  if (!handle->options.longForm.contains(argLongForm))
    return Result::ErrorArgLongFormNotValid;

  auto const &instances = handle->options.longForm.at(argLongForm);
  if (instanceIndex >= instances->size())
    return Result::ErrorInstanceIndexNotValid;

  *position = instances->at(instanceIndex).position;
  return Result::Success;
}

int getOptionInstanceValue(ArgParserT const *const handle,
                           std::string const &argLongForm,
                           std::size_t const instanceIndex,
                           std::string *const value) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!value)
    return Result::ErrorNullptrValue;
  if (!handle->options.longForm.contains(argLongForm))
    return Result::ErrorArgLongFormNotValid;

  auto const &instances = handle->options.longForm.at(argLongForm);
  if (instanceIndex >= instances->size())
    return Result::ErrorInstanceIndexNotValid;

  *value = instances->at(instanceIndex).value;
  return Result::Success;
}

int getFreeValueCount(ArgParserT const *const handle,
                      std::size_t *const count) {
  if (!handle)
    return Result::ErrorNullptrHandle;

  if (!count)
    return Result::ErrorNullptrCount;

  *count = handle->freeValues.size();
  return Result::Success;
}

int getFreeValueInstancePosition(ArgParserT const *const handle,
                                 std::size_t const instanceIndex,
                                 std::size_t *const position) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!position)
    return Result::ErrorNullptrPosition;
  if (instanceIndex >= handle->freeValues.size())
    return Result::ErrorInstanceIndexNotValid;

  *position = handle->freeValues.at(instanceIndex).position;
  return Result::Success;
}

int getFreeValueInstanceValue(ArgParserT const *const handle,
                              std::size_t const instanceIndex,
                              std::string *const value) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!value)
    return Result::ErrorNullptrValue;
  if (instanceIndex >= handle->freeValues.size())
    return Result::ErrorInstanceIndexNotValid;

  *value = handle->freeValues.at(instanceIndex).value;
  return Result::Success;
}
} // namespace ap

namespace ap::Result {
int toString(int const result, std::string *const output) {
  switch (result) {
  case Success:
    *output = "Success";
    break;
  case ErrorNullptrHandle:
    *output = "ErrorNullptrHandle";
    break;
  case ErrorNullptrInput:
    *output = "ErrorNullptrInput";
    break;
  case ErrorNullptrCount:
    *output = "ErrorNullptrCount";
    break;
  case ErrorNullptrPosition:
    *output = "ErrorNullptrPosition";
    break;
  case ErrorNullptrValue:
    *output = "ErrorNullptrValue";
    break;
  case ErrorNullptrOutput:
    *output = "ErrorNullptrOutput";
    break;
  case ErrorArgLongFormNotUnique:
    *output = "ErrorArgLongFormNotUnique";
    break;
  case ErrorArgShortFormNotUnique:
    *output = "ErrorArgShortFormNotUnique";
    break;
  case ErrorArgLongFormNotValid:
    *output = "ErrorArgLongFormNotValid";
    break;
  case ErrorArgShortFormNotValid:
    *output = "ErrorArgShortFormNotValid";
    break;
  case ErrorBeginEndRangeNotValid:
    *output = "ErrorBeginEndRangeNotValid";
    break;
  case ErrorInstanceIndexNotValid:
    *output = "ErrorInstanceIndexNotValid";
    break;
  case ErrorTermTokenNotValid:
    *output = "ErrorTermTokenNotValid";
    break;
  case ErrorMemoryAllocationFailure:
    *output = "ErrorMemoryAllocationFailure";
    break;
  case ErrorResultCodeNotValid:
    *output = "ErrorResultCodeNotValid";
    break;
  case ErrorStartSymbolNotDerivedFromInput:
    *output = "ErrorStartSymbolNotDerivedFromInput";
    break;
  case ErrorExpectedArgListToken:
    *output = "ErrorExpectedArgListToken";
    break;
  case ErrorOptionRequiresValue:
    *output = "ErrorOptionRequiresValue";
    break;
  default:
    return ErrorResultCodeNotValid;
  }

  return Success;
}
} // namespace ap::Result
