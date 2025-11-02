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

#include <badline/scopedLogger.hpp>
#include "internals.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace sl {
std::string getTimestamp(std::string const &dateF, std::string const &timeF) {
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

EntryComponents makeEntryComponents(LoggerT *const l, std::string const &msg,
                                    int const level) {
  EntryComponents comps{};
  comps.timestamp = getTimestamp(l->dateFormat, l->timeFormat);
  comps.logLevel = logLevelToString(level);
  comps.function = l->functions.back();
  comps.message = msg;
  return comps;
}

std::string makeLogEntry(LoggerT *const l, EntryComponents const &comps) {
  std::string entry{};
  if ((l->oneTimeBehavior ? l->oneTimeBehavior : l->behavior) &
      Behavior::PrefixTime)
    entry += comps.timestamp + " ";

  if ((l->oneTimeBehavior ? l->oneTimeBehavior : l->behavior) &
      Behavior::PrefixLevel)
    entry += comps.logLevel + " ";

  if ((l->oneTimeBehavior ? l->oneTimeBehavior : l->behavior) &
      Behavior::PrefixFunc)
    entry += comps.function + ": ";

  entry += comps.message;

  if ((l->oneTimeBehavior ? l->oneTimeBehavior : l->behavior) &
      Behavior::AppendNewLine)
    entry += '\n';
  return entry;
}

void logToConsole(LoggerT *const l, std::string const &entry, int const level) {
  auto const flush = [l](std::ostream &s, std::string const &entry) {
    s << entry;
    if ((l->oneTimeBehavior ? l->oneTimeBehavior : l->behavior) &
        Behavior::FlushStream)
      s << std::flush;
  };

  if ((l->oneTimeOutputMode ? l->oneTimeOutputMode : l->outputMode) &
      OutputMode::Console) {
    switch (level) {
    case LogLevel::Error:
      flush(std::cerr, entry);
      break;
    default:
      flush(std::cout, entry);
    }
  }
}

void logToBuffer(LoggerT *const l, EntryComponents const &comps,
                 int const level) {
  if ((l->oneTimeOutputMode ? l->oneTimeOutputMode : l->outputMode) &
      OutputMode::Buffer) {
    if (l->logBuffer.entries.size() == l->logBuffer.maxEntries)
      l->logBuffer.entries.pop_front();
    l->logBuffer.entries.push_back(
        {std::move(comps), l->functions.size() - 1, level});
  }
}

int log(LoggerT *const l, std::string const &msg, int logLevel) {
  if (!((l->oneTimeLogLevel ? l->oneTimeLogLevel : l->logLevel) & logLevel))
    return Result::Success;

  auto comps = makeEntryComponents(l, msg, logLevel);
  auto entry = makeLogEntry(l, comps);

  logToConsole(l, entry, logLevel);
  logToBuffer(l, comps, logLevel);

  if ((l->oneTimeOutputMode ? l->oneTimeOutputMode : l->outputMode) &
      OutputMode::File) {
    // TO BE DONE
  }

  l->oneTimePropertiesInitialized = false;
  l->oneTimeOutputMode = false;
  l->oneTimeLogLevel = false;
  l->oneTimeBehavior = false;
  return Result::Success;
}

int stepIn(LoggerT *const l, std::string const &func) {
  l->functions.push_back(func);
  return Result::Success;
}

int stepOut(LoggerT *const l) {
  if (l->functions.size())
    l->functions.pop_back();
  return Result::Success;
}

void copyPropertiesToOneTimeVariants(LoggerT *const l) {
  if (!l->oneTimePropertiesInitialized) {
    l->oneTimeOutputMode = l->outputMode;
    l->oneTimeBehavior = l->behavior;
    l->oneTimeLogLevel = l->logLevel;
  }
  l->oneTimePropertiesInitialized = true;
}
} // namespace sl
