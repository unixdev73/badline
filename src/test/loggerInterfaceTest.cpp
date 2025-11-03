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

/* DESCRIPTION:
 *
 * This binary tests if a module implements
 * the entire scoped logger interface.
 *
 * The idea is to update this test every time the scoped logger
 * interface is updated. Then this test will fail
 * if the dummy implementation is not updated.
 *
 * If the interface is missing, it will result in a linker error.
 *
 * EXIT STATUS:
 *
 * 0 - Success
 *
 */

#include <badline/scopedLogger.hpp>

void f(sl::LoggerT *const l) {
  sl::FunctionScope fs{l, __func__};
  sl::outputToConsole(l, true);
  sl::outputToFile(l, true);
  sl::outputToBuffer(l, true);

  sl::logLevelInf(l, true);
  sl::logLevelWrn(l, true);
  sl::logLevelErr(l, true);

  sl::appendNewLine(l, true);
  sl::flushStream(l, true);
  sl::prefixTime(l, true);
  sl::prefixLevel(l, true);
  sl::prefixFunc(l, true);

  sl::oneTimeOutputToConsole(l, true);
  sl::oneTimeOutputToFile(l, true);
  sl::oneTimeOutputToBuffer(l, true);

  sl::oneTimeLogLevelInf(l, true);
  sl::oneTimeLogLevelWrn(l, true);
  sl::oneTimeLogLevelErr(l, true);

  sl::oneTimeAppendNewLine(l, true);
  sl::oneTimeFlushStream(l, true);
  sl::oneTimePrefixTime(l, true);
  sl::oneTimePrefixLevel(l, true);
  sl::oneTimePrefixFunc(l, true);

  sl::inf(l, "msg");
  sl::wrn(l, "msg");
  sl::err(l, "msg");

  sl::resizeLogBuffer(l, 1);
  sl::printTrace(l);
}

int main() {
  {
    sl::LoggerT *logger{};
    sl::createLogger(&logger, __func__);
    sl::destroyLogger(logger);
  }

  auto l = sl::createLogger(__func__);
  f(l.get());

  return 0;
}
