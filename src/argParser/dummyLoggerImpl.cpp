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

#include "requiredLoggerInterface.hpp"

namespace sl {
struct LoggerT {};
using UniqueLogger = std::unique_ptr<LoggerT, void (*)(LoggerT *const)>;

UniqueLogger createLogger(std::string const &) { return {0, 0}; }
int createLogger(LoggerT **const, std::string const &) { return 0; }
void destroyLogger(LoggerT *const) {}

int outputToConsole(LoggerT *const, bool const) { return 0; }
int outputToFile(LoggerT *const, bool const) { return 0; }
int outputToBuffer(LoggerT *const, bool const) { return 0; }

int logLevelInf(LoggerT *const, bool const) { return 0; }
int logLevelWrn(LoggerT *const, bool const) { return 0; }
int logLevelErr(LoggerT *const, bool const) { return 0; }

int appendNewLine(LoggerT *const, bool const) { return 0; }
int flushStream(LoggerT *const, bool const) { return 0; }
int prefixTime(LoggerT *const, bool const) { return 0; }
int prefixLevel(LoggerT *const, bool const) { return 0; }
int prefixFunc(LoggerT *const, bool const) { return 0; }

int oneTimeOutputToConsole(LoggerT *const, bool const) { return 0; }
int oneTimeOutputToFile(LoggerT *const, bool const) { return 0; }
int oneTimeOutputToBuffer(LoggerT *const, bool const) { return 0; }

int oneTimeLogLevelInf(LoggerT *const, bool const) { return 0; }
int oneTimeLogLevelWrn(LoggerT *const, bool const) { return 0; }
int oneTimeLogLevelErr(LoggerT *const, bool const) { return 0; }

int oneTimeAppendNewLine(LoggerT *const, bool const) { return 0; }
int oneTimeFlushStream(LoggerT *const, bool const) { return 0; }
int oneTimePrefixTime(LoggerT *const, bool const) { return 0; }
int oneTimePrefixLevel(LoggerT *const, bool const) { return 0; }
int oneTimePrefixFunc(LoggerT *const, bool const) { return 0; }

int inf(LoggerT *const, std::string const &) { return 0; }
int wrn(LoggerT *const, std::string const &) { return 0; }
int err(LoggerT *const, std::string const &) { return 0; }

int printTrace(LoggerT *const) { return 0; }

FunctionScope::FunctionScope(LoggerT *const, std::string const &) {
  logger_ = 0;
}
FunctionScope::FunctionScope(FunctionScope &&) {}
FunctionScope &FunctionScope::operator=(FunctionScope &&) { return *this; }
FunctionScope::~FunctionScope() {}
} // namespace sl
