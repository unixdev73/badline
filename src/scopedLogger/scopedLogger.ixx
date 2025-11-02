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
#include <memory>
#include <string>

export module ScopedLogger;

export namespace sl {
using UniqueLogger = std::unique_ptr<Logger, void(*)(Logger* const)>;
UniqueLogger createLogger(std::string const&);
int createLogger(Logger** const handle, std::string const&);
void destroyLogger(Logger* const handle);

int outputToConsole(Logger* const, bool const);
int outputToFile(Logger* const, bool const);
int outputToBuffer(Logger* const, bool const);

int logLevelInf(Logger* const, bool const);
int logLevelWrn(Logger* const, bool const);
int logLevelErr(Logger* const, bool const);

int appendNewLine(Logger* const, bool const);
int flushStream(Logger* const, bool const);
int prefixTime(Logger* const, bool const);
int prefixLevel(Logger* const, bool const);
int prefixFunc(Logger* const, bool const);

int oneTimeOutputToConsole(Logger* const, bool const);
int oneTimeOutputToFile(Logger* const, bool const);
int oneTimeOutputToBuffer(Logger* const, bool const);

int oneTimeLogLevelInf(Logger* const, bool const);
int oneTimeLogLevelWrn(Logger* const, bool const);
int oneTimeLogLevelErr(Logger* const, bool const);

int oneTimeAppendNewLine(Logger* const, bool const);
int oneTimeFlushStream(Logger* const, bool const);
int oneTimePrefixTime(Logger* const, bool const);
int oneTimePrefixLevel(Logger* const, bool const);
int oneTimePrefixFunc(Logger* const, bool const);

int inf(Logger* const, std::string const& msg);
int wrn(Logger* const, std::string const& msg);
int err(Logger* const, std::string const& msg);

int printTrace(Logger* const);

class FunctionScope {
public:
	FunctionScope(Logger* const logger, std::string const& funcName);
	FunctionScope(FunctionScope const&) = delete;
	FunctionScope& operator=(FunctionScope const&) = delete;
	FunctionScope(FunctionScope&&);
	FunctionScope& operator=(FunctionScope&&);
	~FunctionScope();
private:
	Logger* logger_;
};
}
