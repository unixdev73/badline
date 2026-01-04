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
#include <badline/instances.hpp>
#include <badline/vertices.hpp>
#include <badline/indices.hpp>
#include "smartResource.hpp"
#include <iostream>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace re {
void addErrMsg(VulkanBackend const *const backend,
               std::string const &msg,
               VkResult r = VkResult::VK_SUCCESS);

bool fillVmaAllocCreateInfo(VulkanBackend const *const backend,
                            VmaAllocatorCreateInfo *const info) {
  if (!backend)
    return false;

  if (!info) {
    addErrMsg(backend,
              "fillVmaAllocCreateInfo: The parameter 'info' = nullptr");
    return false;
  }

  if (!getVersionOfAPI(backend, &info->vulkanApiVersion)) {
    addErrMsg(backend,
              "fillVmaAllocCreateInfo: Failed to get vulkan API version");
    return false;
  }

  if (!getVulkanInstance(backend, &info->instance)) {
    addErrMsg(backend, "fillVmaAllocCreateInfo: Failed to get vulkan instance");
    return false;
  }

  if (!getPhysicalDevice(backend, &info->physicalDevice)) {
    addErrMsg(backend,
              "fillVmaAllocCreateInfo: Failed to get vulkan physical device");
    return false;
  }

  if (!getLogicalDevice(backend, &info->device)) {
    addErrMsg(backend,
              "fillVmaAllocCreateInfo: Failed to get vulkan logical device");
    return false;
  }

  return true;
}

bool createAllocator(VulkanBackend const *const backend,
                     UniqueRes<VmaAllocator_T> *const allocator) {
  VmaAllocatorCreateInfo info{};
  VmaAllocator handle{};

  if (!fillVmaAllocCreateInfo(backend, &info)) {
    addErrMsg(backend, "createAllocator: Failed to fill create info");
    return false;
  }

  if (auto r = vmaCreateAllocator(&info, &handle); r != VK_SUCCESS) {
    addErrMsg(backend, "createAllocator: Failed to create VMA allocator", r);
    return false;
  }

  *allocator = {handle,
                [](VmaAllocator_T *const p) { vmaDestroyAllocator(p); }};
  return true;
}

bool createImage2D(VulkanBackend *const backend,
                   uint32_t const imageWidth,
                   uint32_t const imageHeight,
                   VkFormat const imageFormat,
                   VkImageUsageFlags const imageUsage,
                   UniqueRes<VkImage_T> *const out) {
  if (!backend)
    return false;

  if (!out) {
    addErrMsg(backend, "createImage2D: The parameter 'out' = nullptr");
    return false;
  }

  VkImageCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  info.imageType = VK_IMAGE_TYPE_2D;
  info.extent = {.width = imageWidth, .height = imageHeight, .depth = 1};
  info.mipLevels = 1;
  info.arrayLayers = 1;
  info.format = imageFormat;
  info.tiling = VK_IMAGE_TILING_OPTIMAL;
  info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  info.usage = imageUsage;
  info.samples = VK_SAMPLE_COUNT_1_BIT;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
  allocInfo.priority = 1.f;

  VmaAllocation allocation{};
  VmaAllocator allocator{};
  VkImage img{};

  if (!getMemoryAllocator(backend, &allocator)) {
    addErrMsg(backend, "createImage2D: Failed to get VMA allocator");
    return false;
  }

  auto r = vmaCreateImage(allocator, &info, &allocInfo, &img, &allocation, 0);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, "createImage2D: Failed to create image", r);
    return false;
  }

  *out = {img, [allocator, allocation](VkImage_T *const p) {
            vmaDestroyImage(allocator, p, allocation);
          }};
  return true;
}

bool createDepthImage(VulkanBackend *const backend,
                      uint32_t imageWidth,
                      uint32_t imageHeight,
                      UniqueRes<VkImage_T> *const out) {

  if (!createImage2D(backend,
                     imageWidth,
                     imageHeight,
                     VK_FORMAT_D32_SFLOAT,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     out)) {
    addErrMsg(backend, "createDepthImage: Failed to create depth image");
    return false;
  }

  return true;
}

bool createBuffer(VulkanBackend *const backend,
                  VkDeviceSize const bufferSize,
                  VkBufferUsageFlags const bufferUsage,
                  VmaAllocationCreateFlags const allocFlags,
                  UniqueRes<VkBuffer_T> *const out,
                  VmaAllocation *const outAlloc) {
  if (!backend)
    return false;

  if (!out) {
    addErrMsg(backend, "createBuffer: The parameter 'out' = nullptr");
    return false;
  }

  if (!outAlloc) {
    addErrMsg(backend, "createBuffer: The parameter 'outAlloc' = nullptr");
    return false;
  }

  VkBufferCreateInfo bufInfo{};
  bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufInfo.size = bufferSize;
  bufInfo.usage = bufferUsage;

  VmaAllocationCreateInfo allocInfo{};
  allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  allocInfo.flags = allocFlags;

  VmaAllocator allocator{};
  if (!getMemoryAllocator(backend, &allocator)) {
    addErrMsg(backend, "createBuffer: Failed to get VMA allocator");
    return false;
  }

  VmaAllocation allocation;
  VkBuffer buffer{};

  auto r =
      vmaCreateBuffer(allocator, &bufInfo, &allocInfo, &buffer, &allocation, 0);

  if (r != VK_SUCCESS) {
    addErrMsg(backend, "createBuffer: Failed to create buffer", r);
    return false;
  }

  *outAlloc = allocation;
  *out = {buffer, [allocator, allocation](VkBuffer_T *const p) {
            vmaDestroyBuffer(allocator, p, allocation);
          }};
  return true;
}

bool createStagingBuffer(VulkanBackend *const backend,
                         VkDeviceSize const bufferSize,
                         VkBufferUsageFlags const bufferUsage,
                         UniqueRes<VkBuffer_T> *const outBuf,
                         VmaAllocation *const outAlloc) {

  if (!createBuffer(backend,
                    bufferSize,
                    bufferUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                    outBuf,
                    outAlloc)) {
    addErrMsg(backend, "createStagingBuffer: Failed to create buffer");
    return false;
  }

  return true;
}

bool copyToStagingBuffer(VulkanBackend *const backend,
                         void const *const data,
                         VkDeviceSize const dataSize,
                         VkBufferUsageFlags const usage,
                         UniqueRes<VkBuffer_T> *const out,
                         VmaAllocation *const outAlloc,
                         VkDeviceSize const allocSize = 0) {

  if (!backend)
    return false;

  if (!data) {
    addErrMsg(backend, "copyToStagingBuffer: The parameter 'data' = nullptr");
    return false;
  }

  if (!dataSize) {
    addErrMsg(backend, "copyToStagingBuffer: The parameter 'dataSize' = 0");
    return false;
  }

  if (!out) {
    addErrMsg(backend, "copyToStagingBuffer: The parameter 'out' = nullptr");
    return false;
  }

  if (!outAlloc) {
    addErrMsg(backend,
              "copyToStagingBuffer: The parameter 'outAlloc' = nullptr");
    return false;
  }

  if (!createStagingBuffer(
          backend, allocSize ? allocSize : dataSize, usage, out, outAlloc)) {
    addErrMsg(backend, "copyToStagingBuffer: Creating staging buffer failed");
    return false;
  }

  VmaAllocator allocator{};
  if (!getMemoryAllocator(backend, &allocator)) {
    addErrMsg(backend, "copyToStagingBuffer: Failed to get VMA allocator");
    return false;
  }

  auto r = vmaCopyMemoryToAllocation(allocator, data, *outAlloc, 0, dataSize);
  if (r != VK_SUCCESS) {
    addErrMsg(backend,
              "copyToStagingBuffer: Failed to copy data to staging buffer",
              r);
    return false;
  }

  return true;
}

bool allocateCommandBuffer(VulkanBackend *const backend,
                           UniqueRes<VkCommandBuffer_T> *const p) {
  if (!backend)
    return false;

  if (!p) {
    addErrMsg(backend, "allocateCommandBuffer: The output parameter = nullptr");
    return false;
  }

  VkDevice device{};
  if (!getLogicalDevice(backend, &device)) {
    addErrMsg(backend, "allocateCommandBuffer: Failed to get logical device");
    return false;
  }

  VkCommandPool cmdPool{};
  if (!getCommandPool(backend, &cmdPool)) {
    addErrMsg(backend, "allocateCommandBuffer: Failed to get command pool");
    return false;
  }

  VkCommandBufferAllocateInfo cmdInfo{};

  cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdInfo.commandBufferCount = 1;
  cmdInfo.commandPool = cmdPool;
  cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  VkCommandBuffer cmd{};
  auto result = vkAllocateCommandBuffers(device, &cmdInfo, &cmd);
  if (result != VK_SUCCESS) {
    addErrMsg(
        backend,
        "allocateCommandBuffer: Failed to allocate command buf for buffer copy",
        result);
    return false;
  }

  *p = {cmd, [device, cmdPool](VkCommandBuffer_T *const p) {
          vkFreeCommandBuffers(device, cmdPool, 1, &p);
        }};
  return true;
}

bool submitCmdAndWaitForFence(VulkanBackend *const backend,
                              VkCommandBuffer const cmdBuffer) {
  if (!backend)
    return false;

  if (cmdBuffer == VK_NULL_HANDLE) {
    addErrMsg(backend, "submitCmdAndWaitForFence: cmdBuffer = nullptr");
    return false;
  }

  VkSubmitInfo2 subInf{};
  subInf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  subInf.commandBufferInfoCount = 1;

  VkCommandBufferSubmitInfo cmdSubInf{};
  cmdSubInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmdSubInf.commandBuffer = cmdBuffer;
  subInf.pCommandBufferInfos = &cmdSubInf;

  VkFence fence{};
  if (!getVulkanFence(backend, &fence)) {
    addErrMsg(backend, "submitCmdAndWaitForFence: Failed to get fence");
    return false;
  }

  VkQueue q{};
  if (!getGraphicsQueue(backend, &q)) {
    addErrMsg(backend,
              "submitCmdAndWaitForFence: Failed to get graphics queue");
    return false;
  }

  VkDevice device{};
  if (!getLogicalDevice(backend, &device)) {
    addErrMsg(backend,
              "submitCmdAndWaitForFence: Failed to get logical device");
    return false;
  }

  if (auto r = vkQueueSubmit2(q, 1, &subInf, fence); r != VK_SUCCESS) {
    addErrMsg(
        backend, "submitCmdAndWaitForFence: Failed to submit commands", r);
    return false;
  }

  if (auto r = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
      r != VK_SUCCESS) {
    addErrMsg(backend, "submitCmdAndWaitForFence: Failed to wait for fence", r);
    return false;
  }

  if (auto r = vkResetFences(device, 1, &fence); r != VK_SUCCESS) {
    addErrMsg(backend, "submitCmdAndWaitForFence: Failed to reset fence", r);
    return false;
  }

  return true;
}

bool copyBufferToBuffer(VulkanBackend *const backend,
                        UniqueRes<VkBuffer_T> const *const src,
                        std::size_t const size,
                        VkBufferUsageFlags const usage,
                        UniqueRes<VkBuffer_T> *const dst) {
  if (!backend)
    return false;

  if (!src) {
    addErrMsg(backend, "copyBufferToBuffer: The parameter 'src' = nullptr");
    return false;
  }

  if (!size) {
    addErrMsg(backend, "copyBufferToBuffer: The parameter 'size' = 0");
    return false;
  }

  if (!dst) {
    addErrMsg(backend, "copyBufferToBuffer: The parameter 'dst' = nullptr");
    return false;
  }

  VkDevice device{};
  if (!getLogicalDevice(backend, &device)) {
    addErrMsg(backend, "copyBufferToBuffer: Failed to get logical device");
    return false;
  }

  UniqueRes<VkCommandBuffer_T> cmd{};
  if (!allocateCommandBuffer(backend, &cmd)) {
    addErrMsg(backend, "copyBufferToBuffer: Failed to allocate command buffer");
    return false;
  }

  if (!*dst) {
    VmaAllocation allocation{};
    if (!createBuffer(backend,
                      size,
                      usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                      dst,
                      &allocation)) {
      addErrMsg(backend, "copyBufferToBuffer: Failed to create device buffer");
      return false;
    }
  }

  VkBufferCopy bufCopy{};
  bufCopy.dstOffset = 0;
  bufCopy.srcOffset = 0;
  bufCopy.size = size;

  VkCommandBufferBeginInfo cmdBegInfo{};
  cmdBegInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmdBegInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  if (auto r = vkBeginCommandBuffer(cmd.get(), &cmdBegInfo); r != VK_SUCCESS) {
    addErrMsg(backend, "copyBufferToBuffer: Failed to begin command buffer", r);
    return false;
  }

  vkCmdCopyBuffer(cmd.get(), src->get(), dst->get(), 1, &bufCopy);

  if (auto r = vkEndCommandBuffer(cmd.get()); r != VK_SUCCESS) {
    addErrMsg(backend, "copyBufferToBuffer: Failed to end command buffer", r);
    return false;
  }

  if (!submitCmdAndWaitForFence(backend, cmd.get())) {
    addErrMsg(backend, "copyBufferToBuffer: Failed to submit command buffer");
    return false;
  }

  return true;
}

bool getVertexBuffer(VulkanBackend *const backend,
                     UniqueRes<VkBuffer_T> **const p);

bool uploadVertices(VulkanBackend *const backend,
                    Vertices const *const vertices) {
  if (!backend)
    return false;

  if (!vertices) {
    addErrMsg(backend, "uploadVertices: The parameter 'vertices' = nullptr");
    return false;
  }

  UniqueRes<VkBuffer_T> *dstBuf{};
  if (!getVertexBuffer(backend, &dstBuf)) {
    addErrMsg(backend, "uploadVertices: Failed to get destination buffer");
    return false;
  }

  unsigned long dataSize{};
  void const *data{};

  if (!getData(vertices, &data, &dataSize)) {
    addErrMsg(backend, "uploadVertices: Failed to get vertex data");
    return false;
  }

  UniqueRes<VkBuffer_T> stagingBuf{};
  VmaAllocation allocation{};

  auto const usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

  if (!copyToStagingBuffer(
          backend, data, dataSize, usage, &stagingBuf, &allocation)) {
    addErrMsg(backend, "uploadVertices: Failed to copy data to staging buffer");
    return false;
  }

  if (!copyBufferToBuffer(backend, &stagingBuf, dataSize, usage, dstBuf)) {
    addErrMsg(backend, "uploadVertices: Failed to copy data to device buffer");
    return false;
  }

  return true;
}

bool getIndexBuffer(VulkanBackend *const backend,
                    UniqueRes<VkBuffer_T> **const p);

bool uploadIndices(VulkanBackend *const backend, Indices const *const indices) {
  if (!backend)
    return false;

  if (!indices) {
    addErrMsg(backend, "uploadIndices: The parameter 'indices' = nullptr");
    return false;
  }

  UniqueRes<VkBuffer_T> *dstBuf{};
  if (!getIndexBuffer(backend, &dstBuf)) {
    addErrMsg(backend, "uploadIndices: Failed to get destination buffer");
    return false;
  }

  unsigned long dataSize{};
  void const *data{};

  if (!getData(indices, &data, &dataSize)) {
    addErrMsg(backend, "uploadIndices: Failed to get vertex data");
    return false;
  }

  UniqueRes<VkBuffer_T> stagingBuf{};
  VmaAllocation allocation{};

  auto const usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

  if (!copyToStagingBuffer(
          backend, data, dataSize, usage, &stagingBuf, &allocation)) {
    addErrMsg(backend, "uploadIndices: Failed to copy data to staging buffer");
    return false;
  }

  if (!copyBufferToBuffer(backend, &stagingBuf, dataSize, usage, dstBuf)) {
    addErrMsg(backend, "uploadIndices: Failed to copy data to device buffer");
    return false;
  }

  return true;
}

bool getInstanceBuffer(VulkanBackend *const backend,
                       UniqueRes<VkBuffer_T> **const p);

bool uploadInstances(VulkanBackend *const backend,
                     Instances const *const instances) {
  if (!backend)
    return false;

  if (!instances) {
    addErrMsg(backend, "uploadInstances: The parameter 'instances' = nullptr");
    return false;
  }

  UniqueRes<VkBuffer_T> *dstBuf{};
  if (!getInstanceBuffer(backend, &dstBuf)) {
    addErrMsg(backend, "uploadInstances: Failed to get destination buffer");
    return false;
  }

  unsigned long dataSize{};
  void const *data{};

  if (!getData(instances, &data, &dataSize)) {
    addErrMsg(backend, "uploadInstances: Failed to get vertex data");
    return false;
  }

  UniqueRes<VkBuffer_T> stagingBuf{};
  VmaAllocation allocation{};

  auto const usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

  if (!copyToStagingBuffer(
          backend, data, dataSize, usage, &stagingBuf, &allocation)) {
    addErrMsg(backend,
              "uploadInstances: Failed to copy data to staging buffer");
    return false;
  }

  if (!copyBufferToBuffer(backend, &stagingBuf, dataSize, usage, dstBuf)) {
    addErrMsg(backend, "uploadInstances: Failed to copy data to device buffer");
    return false;
  }

  return true;
}
} // namespace re
