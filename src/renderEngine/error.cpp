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

#include "error.hpp"

namespace re {
void addErrMsg(ErrorLogs const *const e, std::string const &msg, VkResult r) {
  if (!e)
    return;

  if (msg.empty())
    return;

  std::string message{msg}, code{};

  if (r != VK_SUCCESS) {
    if (toString(r, &code))
      message += ", VkError code: " + std::to_string(r);
    else
      message += ", VkError code: " + code;
  }

  e->errors.push_back(std::move(message));
  e->errptr.push_back(e->errors.back().c_str());
}

bool toString(VkResult const result, std::string *const output) {
  std::string asStr = string_VkResult(result);
  if (asStr == "Unhandled VkResult")
    return false;

  *output = std::move(asStr);
  return true;
}
} // namespace re
