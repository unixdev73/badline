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
#include "engine.hpp"
#include "device.hpp"
#include "window.hpp"
#include <functional>
#include <memory>

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

Result setVertices(RenderEngineT *const engine,
                   std::vector<Vertex> const *const vertices) {
  VkBufferCreateInfo bufCreateInfo{};
  bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufCreateInfo.size = sizeof(Vertex) * vertices->size();
  bufCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocCreateInfo.flags =
      VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

  VkBuffer buf;
  VmaAllocation alloc;
  VmaAllocator allocator = engine->device->allocator.get()->handle;
  auto r = vmaCreateBuffer(
      allocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, nullptr);
  if (r != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create vertex buffer", r);
    return Result::ErrorVertexBufferCreationFailure;
  }

  auto const dev = engine->device->handle.get();
  engine->vertexBuf = {buf, [allocator, alloc](VkBuffer_T *const p) {
                         vmaDestroyBuffer(allocator, p, alloc);
                       }};
  engine->vertexBufSize = bufCreateInfo.size;

  vmaCopyMemoryToAllocation(
      allocator, vertices->data(), alloc, 0, bufCreateInfo.size);
  return Result::Success;
}

Result createDepthImage(
    RenderEngineT *const engine,
    std::unique_ptr<VkImage_T, std::function<void(VkImage_T *const)>> *const
        out) {

  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.extent = {.width = engine->window->width,
                 .height = engine->window->height,
                 .depth = 1};
  info.mipLevels = 1;
  info.arrayLayers = 1;
  info.format = VK_FORMAT_D32_SFLOAT;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  info.samples = VK_SAMPLE_COUNT_1_BIT;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  allocInfo.priority = 1.f;

  VkImage handle{};
  VmaAllocation allocation{};
  auto allocator = engine->device->allocator->handle;
  auto result =
      vmaCreateImage(allocator, &info, &allocInfo, &handle, &allocation, 0);

  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create depth image", result);
    return Result::ErrorDepthImageCreationFailure;
  }

  auto const dev = engine->device->handle.get();
  *out = {handle, [allocator, allocation](VkImage_T *const p) {
            vmaDestroyImage(allocator, p, allocation);
          }};
  return Result::Success;
}
} // namespace re
