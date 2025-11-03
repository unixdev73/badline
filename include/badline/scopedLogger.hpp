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

#pragma once

#include <memory>
#include <string>

namespace sl {
namespace Result {
constexpr int Success = 0;
constexpr int ErrorMemoryAllocationFailure = 1;
constexpr int ErrorNullptrParameter = 2;
constexpr int ErrorLogBufferSizeNotValid = 3;
}; // namespace Result

struct LoggerT;
using UniqueLogger = std::unique_ptr<LoggerT, void (*)(LoggerT *const)>;

UniqueLogger createLogger(std::string const &);
int createLogger(LoggerT **const handle, std::string const &);
void destroyLogger(LoggerT *const handle);

int outputToConsole(LoggerT *const, bool const);
int outputToFile(LoggerT *const, bool const);
int outputToBuffer(LoggerT *const, bool const);

int logLevelInf(LoggerT *const, bool const);
int logLevelWrn(LoggerT *const, bool const);
int logLevelErr(LoggerT *const, bool const);

int appendNewLine(LoggerT *const, bool const);
int flushStream(LoggerT *const, bool const);
int prefixTime(LoggerT *const, bool const);
int prefixLevel(LoggerT *const, bool const);
int prefixFunc(LoggerT *const, bool const);

int oneTimeOutputToConsole(LoggerT *const, bool const);
int oneTimeOutputToFile(LoggerT *const, bool const);
int oneTimeOutputToBuffer(LoggerT *const, bool const);

int oneTimeLogLevelInf(LoggerT *const, bool const);
int oneTimeLogLevelWrn(LoggerT *const, bool const);
int oneTimeLogLevelErr(LoggerT *const, bool const);

int oneTimeAppendNewLine(LoggerT *const, bool const);
int oneTimeFlushStream(LoggerT *const, bool const);
int oneTimePrefixTime(LoggerT *const, bool const);
int oneTimePrefixLevel(LoggerT *const, bool const);
int oneTimePrefixFunc(LoggerT *const, bool const);

int inf(LoggerT *const, std::string const &msg);
int wrn(LoggerT *const, std::string const &msg);
int err(LoggerT *const, std::string const &msg);

int resizeLogBuffer(LoggerT *const, std::size_t const size);
int printTrace(LoggerT *const);

class FunctionScope {
public:
  FunctionScope(LoggerT *const logger, std::string const &funcName);
  FunctionScope(FunctionScope const &) = delete;
  FunctionScope &operator=(FunctionScope const &) = delete;
  FunctionScope(FunctionScope &&);
  FunctionScope &operator=(FunctionScope &&);
  ~FunctionScope();

private:
  LoggerT *logger_;
};
} // namespace sl
