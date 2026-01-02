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
void createArgParser(ArgParser **const handle) {
  if (handle)
    *handle = new ArgParser{};
}

void destroyArgParser(ArgParser const *const handle) { delete handle; }

bool addFlag(ArgParser *const handle,
             char const *const argLongFormPtr,
             char const argShortForm) {
  if (!handle)
    return false;

  if (!argLongFormPtr) {
    setErrMsg(handle, "addFlag: The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    setErrMsg(handle, "addFlag: The argument long form is empty");
    return false;
  }

  if (handle->flags.longForm.contains(argLongForm)) {
    setErrMsg(handle,
              "addFlag: The argument long form: " + argLongForm +
                  " is alread in use");
    return false;
  }

  if (argShortForm && handle->flags.shortForm.contains(argShortForm)) {
    auto msg = std::string{"addFlag: The argument short form: "} + argShortForm;
    setErrMsg(handle, msg + " is alread in use");
    return false;
  }

  auto &shortFormDB = handle->flags.shortForm;
  auto &longFormDB = handle->flags.longForm;
  longFormDB.emplace(argLongForm,
                     std::make_unique<std::vector<ArgInstanceInfo>>());
  if (argShortForm)
    shortFormDB.emplace(argShortForm, longFormDB.at(argLongForm).get());
  return true;
}

bool addOption(ArgParser *const handle,
               char const *const argLongFormPtr,
               char const argShortForm) {
  if (!handle)
    return false;

  if (!argLongFormPtr) {
    setErrMsg(handle, "addOption: The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    setErrMsg(handle, "addOption: The argument long form is empty");
    return false;
  }

  if (handle->options.longForm.contains(argLongForm)) {
    setErrMsg(handle,
              "addOption: The argument long form: " + argLongForm +
                  " is alread in use");
    return false;
  }

  if (argShortForm && handle->options.shortForm.contains(argShortForm)) {
    auto m = std::string{"addOption: The argument short form: "} + argShortForm;
    setErrMsg(handle, m + " is alread in use");
    return false;
  }

  auto &shortFormDB = handle->options.shortForm;
  auto &longFormDB = handle->options.longForm;
  longFormDB.emplace(argLongForm,
                     std::make_unique<std::vector<ArgInstanceInfo>>());
  if (argShortForm)
    shortFormDB.emplace(argShortForm, longFormDB.at(argLongForm).get());
  return true;
}

void getErrorMessage(ArgParser const *const handle, char const **const output) {
  if (handle)
    *output = handle->errorMessage.c_str();
}

bool parse(ArgParser *const handle,
           char const *const *const input,
           unsigned const begin,
           unsigned const end) {
  if (!handle)
    return false;

  if (!input) {
    setErrMsg(handle, "parse: The input handle is a nullptr");
    return false;
  }

  if (begin >= end) {
    setErrMsg(handle, "parse: The begin range parameter >= end");
    return false;
  }

  if (!handle->databaseFilled) {
    if (!fillParsingDatabase(&handle->database)) {
      setErrMsg(handle, "parse: Failed to fill parsing database");
      return false;
    }
    handle->databaseFilled = true;
  }

  for (std::size_t i = begin; i < end; ++i) {
    handle->database.back.clear();
    handle->database.chart.clear();
    handle->database.serialized.clear();
    handle->database.tokenInfo = {};
    std::string const token = input[i];
    std::size_t const pos = i - begin;
    bool skip = false;

    if (!handleState(handle, &token, pos, &skip))
      return false;
    if (skip)
      continue;

    if (!parseCYK(handle, &token))
      return false;
    if (!tracePostorderPath(&handle->database, 0))
      return false;
    if (!updateArguments(handle, &token, pos))
      return false;
  }

  return areOptionsAssigned(handle);
}

bool getFlagCount(ArgParser const *const handle,
                  char const *const argLongFormPtr,
                  unsigned *const count) {
  if (!handle)
    return false;

  if (!argLongFormPtr) {
    setErrMsg(handle, "getFlagCount: The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    setErrMsg(handle, "getFlagCount: The argument long form is empty");
    return false;
  }

  if (!count) {
    setErrMsg(handle, "getFlagCount: The output parameter 'count' = nullptr");
    return false;
  }

  if (handle->flags.longForm.contains(argLongForm))
    *count = handle->flags.longForm.at(argLongForm)->size();
  else
    *count = 0;

  return true;
}

bool getFlagPosition(ArgParser const *const handle,
                     char const *const argLongFormPtr,
                     std::size_t const instance,
                     std::size_t *const position) {
  if (!handle)
    return false;

  if (!argLongFormPtr) {
    setErrMsg(handle, "getFlagPosition: The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    setErrMsg(handle, "getFlagPosition: The argument long form is empty");
    return false;
  }

  if (!position) {
    setErrMsg(handle, "getFlagPosition: The argument 'position' is a nullptr");
    return false;
  }

  if (!handle->flags.longForm.contains(argLongForm)) {
    setErrMsg(handle,
              "getFlagPosition: The argument long form" + argLongForm +
                  " is not valid");
    return false;
  }

  auto const &instances = handle->flags.longForm.at(argLongForm);
  if (instance >= instances->size()) {
    setErrMsg(handle, "getFlagPosition: The argument 'instance' is not valid");
    return false;
  }

  *position = instances->at(instance).position;
  return true;
}

bool getOptionCount(ArgParser const *const handle,
                    char const *const argLongFormPtr,
                    unsigned *const count) {
  if (!handle)
    return false;

  if (!argLongFormPtr) {
    setErrMsg(handle, "getOptionCount: The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    setErrMsg(handle, "getOptionCount: The argument long form is empty");
    return false;
  }

  if (!count) {
    setErrMsg(handle, "getOptionCount: The output parameter 'count' = nullptr");
    return false;
  }

  if (handle->options.longForm.contains(argLongForm))
    *count = handle->options.longForm.at(argLongForm)->size();
  else
    *count = 0;

  return true;
}

bool getOptionPosition(ArgParser const *const handle,
                       char const *const argLongFormPtr,
                       unsigned const instance,
                       unsigned *const position) {
  if (!handle)
    return false;

  if (!argLongFormPtr) {
    setErrMsg(handle, "getOptionPosition: The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    setErrMsg(handle, "getOptionPosition: The argument long form is empty");
    return false;
  }

  if (!position) {
    setErrMsg(handle, "getOptionPosition: The output parameter 'position' = 0");
    return false;
  }

  if (!handle->options.longForm.contains(argLongForm)) {
    setErrMsg(handle,
              "getOptionPosition: The argument long form" + argLongForm +
                  " is not valid");
    return false;
  }

  auto const &instances = handle->options.longForm.at(argLongForm);
  if (instance >= instances->size()) {
    setErrMsg(handle,
              "getOptionPosition: The argument 'instance' is not valid");
    return false;
  }

  *position = instances->at(instance).position;
  return true;
}

bool getOptionValue(ArgParser const *const handle,
                    char const *const argLongFormPtr,
                    unsigned const instance,
                    char const **const value) {
  if (!handle)
    return false;

  if (!value) {
    setErrMsg(handle, "getOptionValue: The argument 'value' is a nullptr");
    return false;
  }

  if (!argLongFormPtr) {
    setErrMsg(handle, "getOptionValue: The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    setErrMsg(handle, "getOptionValue: The argument long form is empty");
    return false;
  }

  if (!handle->options.longForm.contains(argLongForm)) {
    setErrMsg(handle,
              "getOptionValue: The argument long form" + argLongForm +
                  " is not valid");
    return false;
  }

  auto const &instances = handle->options.longForm.at(argLongForm);
  if (instance >= instances->size()) {
    setErrMsg(handle, "getOptionValue: The argument 'instance' is not valid");
    return false;
  }

  *value = instances->at(instance).value.c_str();
  return true;
}

bool getFreeValueCount(ArgParser const *const handle, unsigned *const count) {
  if (!handle)
    return false;

  if (!count) {
    setErrMsg(handle, "getFreeValueCount: The argument 'count' = nullptr");
    return false;
  }

  *count = handle->freeValues.size();
  return true;
}

bool getFreeValuePosition(ArgParser const *const handle,
                          unsigned const instance,
                          unsigned *const position) {
  if (!handle)
    return false;

  if (!position) {
    setErrMsg(handle,
              "getFreeValuePosition: The argument 'position' = nullptr");
    return false;
  }

  if (instance >= handle->freeValues.size()) {
    setErrMsg(handle,
              "getFreeValuePosition: The argument 'instance' is not valid");
    return false;
  }

  *position = handle->freeValues.at(instance).position;
  return true;
}

bool getFreeValue(ArgParser const *const handle,
                  unsigned const instance,
                  char const **const value) {
  if (!handle)
    return false;

  if (!value) {
    setErrMsg(handle, "getFreeValue: The argument 'value' = nullptr");
    return false;
  }

  if (instance >= handle->freeValues.size()) {
    setErrMsg(handle, "getFreeValue: The argument 'instance' is not valid");
    return false;
  }

  *value = handle->freeValues.at(instance).value.c_str();
  return true;
}
} // namespace ap
