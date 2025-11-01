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

#include <memory>
#include <string>
#include <list>

export module ScopedLogger;

export namespace sl {
namespace Result {
constexpr int Success = 0;
constexpr int ErrorMemoryAllocationFailure = 1;
constexpr int ErrorNullptrParameter = 2;
};

struct Logger;
using UniqueLogger = std::unique_ptr<Logger, void(*)(Logger* const)>;

int createLogger(Logger** const handle, std::string const&);
void destroyLogger(Logger* const handle);
UniqueLogger createLogger(std::string const&);

int outputToConsole(Logger* const, bool const);
int outputToFile(Logger* const, bool const);
int outputToBuffer(Logger* const, bool const);

int logInf(Logger* const, bool const);
int logWrn(Logger* const, bool const);
int logErr(Logger* const, bool const);

int appendNewLine(Logger* const, bool const);
int flushStream(Logger* const, bool const);
int prefixTime(Logger* const, bool const);
int prefixLevel(Logger* const, bool const);
int prefixFunc(Logger* const, bool const);

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

namespace sl {
namespace OutputMode {
constexpr int Console = 1 << 0;
constexpr int File = 1 << 1;
constexpr int Buffer = 1 << 2;
}

namespace LogLevel {
constexpr int Info = 1 << 0;
constexpr int Warning = 1 << 1;
constexpr int Error = 1 << 2;
}

namespace Behavior {
constexpr int AppendNewLine = 1 << 0;
constexpr int FlushStream = 1 << 1;
constexpr int PrefixTime = 1 << 2;
constexpr int PrefixLevel = 1 << 3;
constexpr int PrefixFunc = 1 << 4;
}

struct EntryComponents {
	std::string timestamp{};
	std::string logLevel{};
	std::string function{};
	std::string message{};
};

struct LogEntry {
	EntryComponents components{};
	std::size_t depth{};
	int type{};
};

struct Buffer {
	std::list<LogEntry> entries{};
	std::size_t maxEntries{1000};
};

struct Logger {
	int logLevel{LogLevel::Info | LogLevel::Warning | LogLevel::Error};
	int outputMode{OutputMode::Console};
	int behavior{Behavior::AppendNewLine | Behavior::FlushStream |
		Behavior::PrefixTime | Behavior::PrefixLevel | Behavior::PrefixFunc};

	std::list<std::string> functions{};

	std::string dateFormat{"%Y/%m/%d"};
	std::string timeFormat{"%H:%M:%S"};

	std::string logFile{};
	Buffer logBuffer{};
};

int stepIn(Logger* const, std::string const& func);
int stepOut(Logger* const);
int log(Logger* const l, std::string const& msg, int const logLevel);

std::string makeLogEntry(Logger* const l, std::string const& msg, int const lv);
std::string logLevelToString(int const level);
std::string getTimestamp(std::string const& dateF, std::string const& timeF);
}
