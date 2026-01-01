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

#pragma once

#include <badline/renderEngine.hpp>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include "allocator.hpp"
#include <functional>
#include <vector>

namespace re {
struct QueueInfo {
  uint32_t famIndex{};
  uint32_t count{};
};

struct DeviceInfoT {
  std::vector<VkQueueFamilyProperties> queues{};
  std::vector<VkExtensionProperties> exts{};
  VkPhysicalDeviceFeatures feats{};
  VkPhysicalDeviceProperties props{};
  QueueInfo graphicsQueue{};
  QueueInfo presentQueue{};
};

struct AllocatorT;

struct DeviceT {
  VkPhysicalDevice identifier{VK_NULL_HANDLE};
  UniqueResource<VkDevice_T> handle{nullptr, nullptr};

  VkQueue present{VK_NULL_HANDLE};
  uint32_t presentFamIndex{};
  VkQueue graphics{VK_NULL_HANDLE};
  uint32_t graphicsFamIndex{};

  UniqueResource<VkCommandPool_T> graphicsCmdPool{nullptr, nullptr};
  std::unique_ptr<AllocatorT> allocator{};
  std::vector<UniqueResource<VkPipelineLayout_T>> pipelineLayouts{};
  std::vector<UniqueResource<VkPipeline_T>> pipelines{};
};

Result createOptimalGPU(RenderEngineT *const engine);

DeviceInfoT queryDeviceInfo(VkPhysicalDevice_T *const handle);
} // namespace re
