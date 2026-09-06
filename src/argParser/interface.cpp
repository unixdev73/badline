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

#include <badline/argParser.hpp>
#include "smartResource.hpp"
#include "internals.hpp"
#include "logs.hpp"

namespace ap {
void create(ArgParser **const handle) {
  if (!handle)
    return;

  Logs *ptr{};
  create(&ptr);
  if (!ptr)
    return;

  CustomUniqPtr<Logs> logs{ptr, [](auto *const p) { destroy(p); }};
  if (!*handle)
    *handle = new ArgParser{};
  else
    (*handle)->customAlloc = true;
  if (handle)
    (*handle)->logs = std::move(logs);
}

bool fillParsingDatabase(ArgParser *const handle) {
  if (!handle)
    return false;

  if (!fillParsingDatabase(&handle->database)) {
    addErrMsg(handle, __func__, "Failed to create parsing database");
    return false;
  }

  return true;
}

std::size_t getSizeOfArgParser() { return sizeof(ArgParser); }

std::size_t getAlignOfArgParser() { return alignof(ArgParser); }

void destroy(ArgParser *const handle) {
  if (handle && !handle->customAlloc)
    delete handle;
}

bool addFlag(ArgParser *const handle,
             char const *const argLongFormPtr,
             char const argShortForm) {
  if (!handle)
    return false;

  if (!argLongFormPtr) {
    addErrMsg(handle, __func__, "The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    addErrMsg(handle, __func__, "The argument long form is empty");
    return false;
  }

  if (handle->flags.longForm.contains(argLongForm)) {
    addErrMsg(handle,
              __func__,
              "The argument long form: " + argLongForm + " is alread in use");
    return false;
  }

  if (argShortForm && handle->flags.shortForm.contains(argShortForm)) {
    auto msg = std::string{"The argument short form: "} + argShortForm;
    addErrMsg(handle, __func__, msg + " is alread in use");
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
    addErrMsg(handle, __func__, "The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    addErrMsg(handle, __func__, "The argument long form is empty");
    return false;
  }

  if (handle->options.longForm.contains(argLongForm)) {
    addErrMsg(handle,
              __func__,
              "The argument long form: " + argLongForm + " is alread in use");
    return false;
  }

  if (argShortForm && handle->options.shortForm.contains(argShortForm)) {
    auto m = std::string{"The argument short form: "} + argShortForm;
    addErrMsg(handle, __func__, m + " is alread in use");
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

bool parse(ArgParser *const handle,
           char const *const *const input,
           unsigned const begin,
           unsigned const end) {
  if (!handle)
    return false;

  if (!input) {
    addErrMsg(handle, __func__, "The input handle is a nullptr");
    return false;
  }

  if (begin >= end) {
    addErrMsg(handle, __func__, "The begin range parameter >= end");
    return false;
  }

  if (!handle->databaseFilled) {
    if (!fillParsingDatabase(&handle->database)) {
      addErrMsg(handle, __func__, "Failed to fill parsing database");
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
    addErrMsg(handle, __func__, "The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    addErrMsg(handle, __func__, "The argument long form is empty");
    return false;
  }

  if (!count) {
    addErrMsg(handle, __func__, "The output parameter 'count' = nullptr");
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
    addErrMsg(handle, __func__, "The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    addErrMsg(handle, __func__, "The argument long form is empty");
    return false;
  }

  if (!position) {
    addErrMsg(handle, __func__, "The argument 'position' is a nullptr");
    return false;
  }

  if (!handle->flags.longForm.contains(argLongForm)) {
    addErrMsg(handle,
              __func__,
              "The argument long form" + argLongForm + " is not valid");
    return false;
  }

  auto const &instances = handle->flags.longForm.at(argLongForm);
  if (instance >= instances->size()) {
    addErrMsg(handle, __func__, "The argument 'instance' is not valid");
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
    addErrMsg(handle, __func__, "The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    addErrMsg(handle, __func__, "The argument long form is empty");
    return false;
  }

  if (!count) {
    addErrMsg(handle, __func__, "The output parameter 'count' = nullptr");
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
    addErrMsg(handle, __func__, "The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    addErrMsg(handle, __func__, "The argument long form is empty");
    return false;
  }

  if (!position) {
    addErrMsg(handle, __func__, "The output parameter 'position' = 0");
    return false;
  }

  if (!handle->options.longForm.contains(argLongForm)) {
    addErrMsg(handle,
              __func__,
              "The argument long form" + argLongForm + " is not valid");
    return false;
  }

  auto const &instances = handle->options.longForm.at(argLongForm);
  if (instance >= instances->size()) {
    addErrMsg(handle, __func__, "The argument 'instance' is not valid");
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
    addErrMsg(handle, __func__, "The argument 'value' is a nullptr");
    return false;
  }

  if (!argLongFormPtr) {
    addErrMsg(handle, __func__, "The argument long form is a nullptr");
    return false;
  }

  std::string const argLongForm{argLongFormPtr};

  if (argLongForm.empty()) {
    addErrMsg(handle, __func__, "The argument long form is empty");
    return false;
  }

  if (!handle->options.longForm.contains(argLongForm)) {
    addErrMsg(handle,
              __func__,
              "The argument long form" + argLongForm + " is not valid");
    return false;
  }

  auto const &instances = handle->options.longForm.at(argLongForm);
  if (instance >= instances->size()) {
    addErrMsg(handle, __func__, "The argument 'instance' is not valid");
    return false;
  }

  *value = instances->at(instance).value.c_str();
  return true;
}

bool getFreeValueCount(ArgParser const *const handle, unsigned *const count) {
  if (!handle)
    return false;

  if (!count) {
    addErrMsg(handle, __func__, "The argument 'count' = nullptr");
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
    addErrMsg(handle, __func__, "The argument 'position' = nullptr");
    return false;
  }

  if (instance >= handle->freeValues.size()) {
    addErrMsg(handle, __func__, "The argument 'instance' is not valid");
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
    addErrMsg(handle, __func__, "The argument 'value' = nullptr");
    return false;
  }

  if (instance >= handle->freeValues.size()) {
    addErrMsg(handle, __func__, "The argument 'instance' is not valid");
    return false;
  }

  *value = handle->freeValues.at(instance).value.c_str();
  return true;
}

void addErrMsg(ArgParser const *const handle,
               std::string const &tag,
               std::string const &msg) {
  addErrMsg(handle->logs.get(), tag, msg);
}

void addInfMsg(ArgParser const *const handle,
               std::string const &tag,
               std::string const &msg) {
  addInfMsg(handle->logs.get(), tag, msg);
}

void addWrnMsg(ArgParser const *const handle,
               std::string const &tag,
               std::string const &msg) {
  addWrnMsg(handle->logs.get(), tag, msg);
}
} // namespace ap
