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

#include <iostream>

namespace ap {
struct Logs {
  bool enableWrn{false};
  bool enableInf{false};
};

void create(Logs **const handle) {
  if (!handle)
    return;
  *handle = new Logs{};
}

void destroy(Logs *const handle) { delete handle; }

void enableInf(Logs *const handle) {
  if (handle)
    handle->enableInf = true;
}

void enableWrn(Logs *const handle) {
  if (handle)
    handle->enableWrn = true;
}

void printLog(std::string const &entry,
              std::size_t const indentSize = 0,
              bool flush = true) {
  if (!entry.size())
    return;

  std::string const indentStr(indentSize, ' ');
  std::ostream *out = nullptr;

  switch (entry[0]) {
  case 'E':
    out = &std::cerr;
    break;
  default:
    out = &std::cout;
  }

  *out << indentStr << entry;

  switch (int(flush)) {
  case 0:
    *out << "\n";
    break;
  default:
    *out << std::endl;
  }
}

void addInfMsg(Logs *const handle,
               std::string const &tag,
               std::string const &msg) {
  if (msg.empty())
    return;
  if (handle->enableInf)
    printLog("INF: " + tag + ": " + msg, 0);
}

void addWrnMsg(Logs *const handle,
               std::string const &tag,
               std::string const &msg) {
  if (msg.empty())
    return;
  if (handle->enableWrn)
    printLog("WRN: " + tag + ": " + msg, 0);
}

void addErrMsg(Logs *const, std::string const &tag, std::string const &msg) {
  if (msg.empty())
    return;

  printLog("ERR: " + tag + ": " + msg, 0);
}
} // namespace ap
