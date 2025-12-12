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
Result toString(Result const result, std::string *const output) {
  switch (result) {
  case Result::Success:
    *output = "Success";
    break;
  case Result::ErrorNullptrHandle:
    *output = "ErrorNullptrHandle";
    break;
  case Result::ErrorNullptrInput:
    *output = "ErrorNullptrInput";
    break;
  case Result::ErrorNullptrCount:
    *output = "ErrorNullptrCount";
    break;
  case Result::ErrorNullptrPosition:
    *output = "ErrorNullptrPosition";
    break;
  case Result::ErrorNullptrValue:
    *output = "ErrorNullptrValue";
    break;
  case Result::ErrorNullptrOutput:
    *output = "ErrorNullptrOutput";
    break;
  case Result::ErrorArgLongFormNotUnique:
    *output = "ErrorArgLongFormNotUnique";
    break;
  case Result::ErrorArgShortFormNotUnique:
    *output = "ErrorArgShortFormNotUnique";
    break;
  case Result::ErrorArgLongFormNotValid:
    *output = "ErrorArgLongFormNotValid";
    break;
  case Result::ErrorArgShortFormNotValid:
    *output = "ErrorArgShortFormNotValid";
    break;
  case Result::ErrorBeginEndRangeNotValid:
    *output = "ErrorBeginEndRangeNotValid";
    break;
  case Result::ErrorInstanceIndexNotValid:
    *output = "ErrorInstanceIndexNotValid";
    break;
  case Result::ErrorTermTokenNotValid:
    *output = "ErrorTermTokenNotValid";
    break;
  case Result::ErrorMemoryAllocationFailure:
    *output = "ErrorMemoryAllocationFailure";
    break;
  case Result::ErrorStartSymbolNotDerivedFromInput:
    *output = "ErrorStartSymbolNotDerivedFromInput";
    break;
  case Result::ErrorExpectedArgListToken:
    *output = "ErrorExpectedArgListToken";
    break;
  case Result::ErrorOptionRequiresValue:
    *output = "ErrorOptionRequiresValue";
    break;
  case Result::ErrorInputTokenNotValid:
    *output = "ErrorInputTokenNotValid";
    break;
  case Result::ErrorRuleIdentifierNotValid:
    *output = "ErrorRuleIdentifierNotValid";
    break;
  }

  return Result::Success;
}

Result createArgParser(ArgParserT **const handle) {
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

UniqueArgParser createArgParser() {
  ArgParserT *handle{};
  createArgParser(&handle);
  return {handle, destroyArgParser};
}

Result addFlag(ArgParserT *const handle,
               std::string const &argLongForm,
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

Result addOption(ArgParserT *const handle,
                 std::string const &argLongForm,
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

Result parse(ArgParserT *const handle,
             char const *const *const input,
             std::size_t const begin,
             std::size_t const end) {
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
		bool skip = false;

		if (auto r = handleState(handle, &token, pos, &skip); r != Result::Success)
			return r;
		if (skip)
			continue;

    if (auto r = parseCYK(&handle->database, &token); r != Result::Success)
      return r;
    if (auto r = tracePostorderPath(&handle->database, 0); r != Result::Success)
      return r;
    if (auto r = updateArguments(handle, &token, pos); r != Result::Success)
      return r;
  }

  return checkThatAllOptionsAreAssigned(handle);
}

Result getErrorPosition(ArgParserT *const handle, std::size_t *const output) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!output)
    return Result::ErrorNullptrOutput;
  *output = handle->errorPosition;
  return Result::Success;
}

Result getFlagCount(ArgParserT const *const handle,
                    std::string const &argLongForm,
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

Result getFlagInstancePosition(ArgParserT const *const handle,
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

Result getOptionCount(ArgParserT const *const handle,
                      std::string const &argLongForm,
                      std::size_t *const count) {
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

Result getOptionInstancePosition(ArgParserT const *const handle,
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

Result getOptionInstanceValue(ArgParserT const *const handle,
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

Result getFreeValueCount(ArgParserT const *const handle,
                         std::size_t *const count) {
  if (!handle)
    return Result::ErrorNullptrHandle;

  if (!count)
    return Result::ErrorNullptrCount;

  *count = handle->freeValues.size();
  return Result::Success;
}

Result getFreeValueInstancePosition(ArgParserT const *const handle,
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

Result getFreeValueInstance(ArgParserT const *const handle,
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
