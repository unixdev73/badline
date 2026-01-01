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

Result stagingCopy(RenderEngineT *const engine,
                   void const *const data,
                   std::size_t const size,
                   VkBufferUsageFlags const usage,
                   UniqueBuf *const out) {

  VkBufferCreateInfo bufCreateInfo{};
  bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufCreateInfo.size = size;
  bufCreateInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

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
    setErrMsg(engine, "Failed to create staging buffer", r);
    return Result::ErrorVulkanBufferCreationFailure;
  }

  auto const dev = engine->device->handle.get();
  *out = {buf, [allocator, alloc](VkBuffer_T *const p) {
            vmaDestroyBuffer(allocator, p, alloc);
          }};

  r = vmaCopyMemoryToAllocation(allocator, data, alloc, 0, bufCreateInfo.size);
  if (r != VK_SUCCESS) {
    setErrMsg(engine, "Failed to copy data to staging buffer", r);
    return Result::ErrorCopyToStagingBufferFailure;
  }

  return Result::Success;
}

Result bufferCopy(RenderEngineT *const engine,
                  UniqueBuf const *const src,
                  std::size_t const size,
                  VkBufferUsageFlags const usage,
                  UniqueBuf *const dst) {

  auto const dev = engine->device->handle.get();
  VkCommandPool cmdPool{};
  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = engine->device->graphicsFamIndex;
  auto result = vkCreateCommandPool(dev, &poolInfo, 0, &cmdPool);
  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create command pool for buffer copy", result);
    return Result::ErrorVulkanCommandPoolCreationFailure;
  }
  UniqueRes<VkCommandPool_T> pool = {cmdPool, [dev](VkCommandPool_T *const p) {
                                       vkDestroyCommandPool(dev, p, 0);
                                     }};

  VkCommandBuffer cmd{};
  VkCommandBufferAllocateInfo cmdInfo{};
  cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdInfo.commandBufferCount = 1;
  cmdInfo.commandPool = engine->device->graphicsCmdPool.get();
  cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  result = vkAllocateCommandBuffers(dev, &cmdInfo, &cmd);
  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to allocate command buf for buffer copy", result);
    return Result::ErrorVulkanCommandBufferAllocationFailure;
  }

  VkBufferCreateInfo bufCreateInfo{};
  bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufCreateInfo.size = size;
  bufCreateInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

  VmaAllocationCreateInfo allocCreateInfo{};
  allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

  VkBuffer buf;
  VmaAllocation alloc;
  VmaAllocator allocator = engine->device->allocator.get()->handle;
  auto r = vmaCreateBuffer(
      allocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, nullptr);
  if (r != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create dst buffer for copy", r);
    return Result::ErrorVulkanBufferCreationFailure;
  }

  *dst = {buf, [allocator, alloc](VkBuffer_T *const p) {
            vmaDestroyBuffer(allocator, p, alloc);
          }};

  VkBufferCopy bufCopy{};
  bufCopy.dstOffset = 0;
  bufCopy.srcOffset = 0;
  bufCopy.size = size;

  VkCommandBufferBeginInfo cmdBegInfo{};
  cmdBegInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBegInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(cmd, &cmdBegInfo);
  vkCmdCopyBuffer(cmd, src->get(), dst->get(), 1, &bufCopy);
  vkEndCommandBuffer(cmd);

  VkSubmitInfo2 subInf{};
  subInf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  subInf.commandBufferInfoCount = 1;
  VkCommandBufferSubmitInfo cmdSubInf{};
  cmdSubInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmdSubInf.commandBuffer = cmd;
  subInf.pCommandBufferInfos = &cmdSubInf;
  auto const fence = engine->window->fence.get();
  vkQueueSubmit2(engine->device->graphics, 1, &subInf, fence);
  vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  vkResetFences(dev, 1, &fence);
  return Result::Success;
}

Result setVertices(RenderEngineT *const engine,
                   std::vector<Vertex> const *const vertices) {

  auto const size = sizeof(Vertex) * vertices->size();
  UniqueBuf stagingBuf{};
  auto result = stagingCopy(engine,
                            vertices->data(),
                            size,
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            &stagingBuf);

  if (result != Result::Success)
    return result;

  result = bufferCopy(engine,
                      &stagingBuf,
                      size,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                      &engine->vertexBuf);

  if (result != Result::Success)
    return result;
  engine->vertexBufSize = size;
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
