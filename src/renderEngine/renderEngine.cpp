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

#include <badline/vulkanBackend.hpp>
#include "smartResource.hpp"
#include "logs.hpp"

namespace re {
struct RenderEngine {
  mutable CustomUniqPtr<Logs> logs;
  CustomUniqPtr<VulkanBackend> vulkanBackend;
  bool customAlloc{};
};

std::size_t getSizeOfRenderEngine() { return sizeof(RenderEngine); }

std::size_t getAlignOfRenderEngine() { return alignof(RenderEngine); }

void create(RenderEngine **const handle) {
  if (!handle)
    return;

  Logs *ptr{};
  create(&ptr);
  if (!ptr)
    return;
  CustomUniqPtr<Logs> logs{ptr, [](auto *const p) { destroy(p); }};

  if (!*handle)
    *handle = new RenderEngine{};
  else
    (*handle)->customAlloc = true;

  if (*handle)
    (*handle)->logs = std::move(logs);
}

void destroy(RenderEngine *const handle) {
  if (handle && !handle->customAlloc)
    delete handle;
}

void create(Logs *const l, VulkanBackend **const handle);

void destroy(VulkanBackend *const handle);

bool createBackend(RenderEngine *const handle, VulkanBackend **const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle->logs.get(), __func__, "The backend handle = nullptr");
    return false;
  }

  if (handle->vulkanBackend) {
    addErrMsg(
        handle->logs.get(), __func__, "The backend is already initialized");
    return false;
  }

  VulkanBackend *ptr{};
  create(handle->logs.get(), &ptr);
  if (!ptr) {
    addErrMsg(handle->logs.get(), __func__, "Failed to create backend");
    return false;
  }

  handle->vulkanBackend = {ptr, [](auto *const p) { destroy(p); }};
  *p = ptr;

  return true;
}

void enableInfoLogs(RenderEngine *const handle) {
  if (handle)
    enableInf(handle->logs.get());
}

void enableWarnings(RenderEngine *const handle) {
  if (handle)
    enableWrn(handle->logs.get());
}
} // namespace re
