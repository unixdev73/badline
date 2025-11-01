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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

module ScopedLogger;

namespace sl {
int printTrace(Logger* const l) {
	std::string const indent{"  "};
	auto& s = std::cout;

	for (auto const& rec : l->logBuffer.entries) {
		for (std::size_t i = 0; i < rec.depth; ++i)
			s << indent;
		s << rec.components.timestamp << " ";
		s << rec.components.logLevel << " ";
		s << rec.components.function << ": ";
		s << rec.components.message << "\n";
	}

	return Result::Success;
}

std::string getTimestamp(std::string const& dateF, std::string const& timeF) {
	using clock_t = std::chrono::system_clock;
	std::time_t currentTime{clock_t::to_time_t(clock_t::now())};
	std::tm timeMember = *std::localtime(&currentTime);
	std::stringstream buffer{};
	buffer << std::put_time(&timeMember, dateF.c_str()) << " ";
	buffer << std::put_time(&timeMember, timeF.c_str());
	return buffer.str();
}

std::string logLevelToString(int const level) {
	switch (level) {
		case LogLevel::Warning:
			return "[Warning]";
		case LogLevel::Error:
			return "[Error]";
		default:
			return "[Info]";
	}
}

EntryComponents makeEntryComponents(Logger* const l, std::string const& msg, int const level) {
	EntryComponents comps{};
	comps.timestamp = getTimestamp(l->dateFormat, l->timeFormat);
	comps.logLevel = logLevelToString(level);
	comps.function = l->functions.back();
	comps.message = msg;
	return comps;
}

std::string makeLogEntry(Logger* const l, EntryComponents const& comps) {
	std::string entry{};
	if (l->behavior & Behavior::PrefixTime)
		entry += comps.timestamp;

	if (l->behavior & Behavior::PrefixLevel)
		entry += " " + comps.logLevel;

	if (l->behavior & Behavior::PrefixFunc)
		entry += comps.function + ":";

	entry += " " + comps.message;

	if (l->behavior & Behavior::AppendNewLine)
		entry += '\n';
	return entry;
}

int log(Logger* const l, std::string const& msg, int logLevel) {
	if (!(l->logLevel & logLevel))
		return Result::Success;
	
	auto const flush = [l](std::ostream& s, std::string const& entry) {
		s << entry;
		if (l->behavior & Behavior::FlushStream) s << std::flush;
	};

	auto comps = makeEntryComponents(l, msg, logLevel);
	auto entry = makeLogEntry(l, comps);

	if (l->outputMode & OutputMode::Console) {
		switch (logLevel) {
			case LogLevel::Error:
				flush(std::cerr, entry);
				break;
			default:
				flush(std::cout, entry);
		}
	}

	if (l->outputMode & OutputMode::File) {
		// TO BE DONE
	}

	if (l->outputMode & OutputMode::Buffer) {
		if (l->logBuffer.entries.size() == l->logBuffer.maxEntries)
			l->logBuffer.entries.pop_front();
		l->logBuffer.entries.push_back({
				std::move(comps), l->functions.size() - 1, logLevel});
	}

	return Result::Success;
}

int inf(Logger* const l, std::string const& msg) {
	return log(l, msg, LogLevel::Info);
}

int wrn(Logger* const l, std::string const& msg) {
	return log(l, msg, LogLevel::Warning);
}

int err(Logger* const l, std::string const& msg) {
	return log(l, msg, LogLevel::Error);
}

int addOrRemoveBit(Logger* const l, bool const v, int& target, int const c) {
	if (!l)
		return Result::ErrorNullptrParameter;

	if (v)
		target |= c;
	else
		target &= ~c;

	return Result::Success;
}

int outputToConsole(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->outputMode, OutputMode::Console);
}

int outputToFile(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->outputMode, OutputMode::File);
}

int outputToBuffer(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->outputMode, OutputMode::Buffer);
}

int logInf(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->logLevel, LogLevel::Info);
}

int logWrn(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->logLevel, LogLevel::Warning);
}

int logErr(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->logLevel, LogLevel::Error);
}

int appendNewLine(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->behavior, Behavior::AppendNewLine);
}

int flushStream(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->behavior, Behavior::FlushStream);
}

int prefixTime(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->behavior, Behavior::PrefixTime);
}

int prefixLevel(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->behavior, Behavior::PrefixLevel);
}

int prefixFunc(Logger* const l, bool const v) {
	return addOrRemoveBit(l, v, l->behavior, Behavior::PrefixFunc);
}

int stepIn(Logger* const l, std::string const& func) {
	l->functions.push_back(func);
	return Result::Success;
}

int stepOut(Logger* const l) {
	if (l->functions.size())
		l->functions.pop_back();
	return Result::Success;
}

int createLogger(Logger** const handle, std::string const& func) {
	if (!handle)
		return Result::ErrorNullptrParameter;

	if (auto ptr = new Logger{}; ptr) {
		*handle = ptr;
		stepIn(ptr, func);
		return Result::Success;
	}

	return Result::ErrorMemoryAllocationFailure;
}

void destroyLogger(Logger* const handle) {
	delete handle;
}

UniqueLogger createLogger(std::string const& func) {
	Logger* handle{};
	createLogger(&handle, func);
	return {handle, destroyLogger};
}

FunctionScope::FunctionScope(Logger* const logger, std::string const& func) {
	logger_ = logger;
	stepIn(logger_, func);
}

FunctionScope::FunctionScope(FunctionScope&& o) {
	logger_ = o.logger_;
	o.logger_ = nullptr;
}

FunctionScope& FunctionScope::operator=(FunctionScope&& o) {
	logger_ = o.logger_;
	o.logger_ = nullptr;
	return *this;
}

FunctionScope::~FunctionScope() {
	if (logger_)
		stepOut(logger_);
}
}
