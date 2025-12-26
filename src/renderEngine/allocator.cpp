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

#include <badline/renderEngine.hpp>
#include "allocator.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace re {
AllocatorT::~AllocatorT() {
  if (handle)
    vmaDestroyAllocator(handle);
}

std::unique_ptr<AllocatorT> createAllocator(VkInstance const inst,
                                            VkPhysicalDevice const phy,
                                            VkDevice const dev,
                                            VkResult *const res) {
  auto alloc = std::make_unique<AllocatorT>();
  VmaAllocatorCreateInfo info{};
  info.vulkanApiVersion = BADLINE_VK_API_VERSION;
  info.instance = inst;
  info.physicalDevice = phy;
  info.device = dev;

  if (auto r = vmaCreateAllocator(&info, &alloc->handle); r != VK_SUCCESS) {
    if (res)
      *res = r;
    return {};
  }

  return alloc;
}
} // namespace re
