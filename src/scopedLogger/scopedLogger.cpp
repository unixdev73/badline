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

module;

#include "private/scopedLogger.hpp"
#include "result.hpp"
#include <iostream>
#include <ctime>

module ScopedLogger;

namespace sl {
int printTrace(Logger *const l) {
  std::string const indent{"  "};
  auto &s = std::cout;

  for (auto const &rec : l->logBuffer.entries) {
    for (std::size_t i = 0; i < rec.depth; ++i)
      s << indent;
    s << rec.components.timestamp << " ";
    s << rec.components.logLevel << " ";
    s << rec.components.function << ": ";
    s << rec.components.message << "\n";
  }

  return Result::Success;
}

int inf(Logger *const l, std::string const &msg) {
  return log(l, msg, LogLevel::Info);
}

int wrn(Logger *const l, std::string const &msg) {
  return log(l, msg, LogLevel::Warning);
}

int err(Logger *const l, std::string const &msg) {
  return log(l, msg, LogLevel::Error);
}

int addOrRemoveBit(Logger *const l, bool const v, int &target, int const c) {
  if (!l)
    return Result::ErrorNullptrParameter;

  if (v)
    target |= c;
  else
    target &= ~c;

  return Result::Success;
}

int outputToConsole(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->outputMode, OutputMode::Console);
}

int outputToFile(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->outputMode, OutputMode::File);
}

int outputToBuffer(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->outputMode, OutputMode::Buffer);
}

int logLevelInf(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->logLevel, LogLevel::Info);
}

int logLevelWrn(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->logLevel, LogLevel::Warning);
}

int logLevelErr(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->logLevel, LogLevel::Error);
}

int appendNewLine(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->behavior, Behavior::AppendNewLine);
}

int flushStream(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->behavior, Behavior::FlushStream);
}

int prefixTime(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->behavior, Behavior::PrefixTime);
}

int prefixLevel(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->behavior, Behavior::PrefixLevel);
}

int prefixFunc(Logger *const l, bool const v) {
  return addOrRemoveBit(l, v, l->behavior, Behavior::PrefixFunc);
}

int oneTimeOutputToConsole(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeOutputMode, OutputMode::Console);
}

int oneTimeOutputToFile(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeOutputMode, OutputMode::File);
}

int oneTimeOutputToBuffer(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeOutputMode, OutputMode::Buffer);
}

int oneTimeLogLevelInf(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeLogLevel, LogLevel::Info);
}

int oneTimeLogLevelWrn(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeLogLevel, LogLevel::Warning);
}

int oneTimeLogLevelErr(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeLogLevel, LogLevel::Error);
}

int oneTimeAppendNewLine(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeBehavior, Behavior::AppendNewLine);
}

int oneTimeFlushStream(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeBehavior, Behavior::FlushStream);
}

int oneTimePrefixTime(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeBehavior, Behavior::PrefixTime);
}

int oneTimePrefixLevel(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeBehavior, Behavior::PrefixLevel);
}

int oneTimePrefixFunc(Logger *const l, bool const v) {
  copyPropertiesToOneTimeVariants(l);
  return addOrRemoveBit(l, v, l->oneTimeBehavior, Behavior::PrefixFunc);
}

int createLogger(Logger **const handle, std::string const &func) {
  if (!handle)
    return Result::ErrorNullptrParameter;

  if (auto ptr = new Logger{}; ptr) {
    *handle = ptr;
    stepIn(ptr, func);
    return Result::Success;
  }

  return Result::ErrorMemoryAllocationFailure;
}

void destroyLogger(Logger *const handle) { delete handle; }

UniqueLogger createLogger(std::string const &func) {
  Logger *handle{};
  createLogger(&handle, func);
  return {handle, destroyLogger};
}

FunctionScope::FunctionScope(Logger *const logger, std::string const &func) {
  logger_ = logger;
  stepIn(logger_, func);
}

FunctionScope::FunctionScope(FunctionScope &&o) {
  logger_ = o.logger_;
  o.logger_ = nullptr;
}

FunctionScope &FunctionScope::operator=(FunctionScope &&o) {
  logger_ = o.logger_;
  o.logger_ = nullptr;
  return *this;
}

FunctionScope::~FunctionScope() {
  if (logger_)
    stepOut(logger_);
}
} // namespace sl
