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

#include "smartResource.hpp"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace re {
struct VulkanBackend;
bool getVersionOfAPI(VulkanBackend const *const backend,
                     unsigned *const version);

bool getVulkanInstance(VulkanBackend const *const backend,
                       VkInstance *const instance);

bool getPhysicalDevice(VulkanBackend const *const backend,
                       VkPhysicalDevice *const physical);

bool getLogicalDevice(VulkanBackend const *const backend,
                      VkDevice *const logical);

bool getVulkanAllocator(VulkanBackend const *const backend,
                        VmaAllocator *const alloc);

bool getGeneralCommandPool(VulkanBackend const *const backend,
                           VkCommandPool *const cmdPool);

void addErrMsg(VulkanBackend const *const handle,
               std::string const &tag,
               std::string const &msg,
               VkResult const r = VK_SUCCESS);

bool fillVmaAllocCreateInfo(VulkanBackend const *const backend,
                            VmaAllocatorCreateInfo *const info) {
  if (!backend)
    return false;

  if (!info) {
    addErrMsg(backend, __func__, "The parameter 'info' = nullptr");
    return false;
  }

  if (!getVersionOfAPI(backend, &info->vulkanApiVersion)) {
    addErrMsg(backend, __func__, "Failed to get vulkan API version");
    return false;
  }

  if (!getVulkanInstance(backend, &info->instance)) {
    addErrMsg(backend, __func__, "Failed to get vulkan instance");
    return false;
  }

  if (!getPhysicalDevice(backend, &info->physicalDevice)) {
    addErrMsg(backend, __func__, "Failed to get vulkan physical device");
    return false;
  }

  if (!getLogicalDevice(backend, &info->device)) {
    addErrMsg(backend, __func__, "Failed to get vulkan logical device");
    return false;
  }

  return true;
}

bool createVulkanAllocator(VulkanBackend *const backend,
                           CustomUniqPtr<VmaAllocator_T> *const allocator) {
  VmaAllocatorCreateInfo info{};
  VmaAllocator handle{};

  if (!fillVmaAllocCreateInfo(backend, &info)) {
    addErrMsg(backend, __func__, "Failed to fill create info");
    return false;
  }

  if (auto r = vmaCreateAllocator(&info, &handle); r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create VMA allocator", r);
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
                   CustomUniqPtr<VkImage_T> *const out) {
  if (!backend)
    return false;

  if (!out) {
    addErrMsg(backend, __func__, "The parameter 'out' = nullptr");
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

  if (!getVulkanAllocator(backend, &allocator)) {
    addErrMsg(backend, __func__, "Failed to get VMA allocator");
    return false;
  }

  auto r = vmaCreateImage(allocator, &info, &allocInfo, &img, &allocation, 0);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create image", r);
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
                      CustomUniqPtr<VkImage_T> *const out) {

  if (!createImage2D(backend,
                     imageWidth,
                     imageHeight,
                     VK_FORMAT_D32_SFLOAT,
                     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                     out)) {
    addErrMsg(backend, __func__, "Failed to create depth image");
    return false;
  }

  return true;
}

struct VulkanBuffer;

bool createBuffer(VulkanBackend *const backend,
                  VkDeviceSize const bufferSize,
                  VkBufferUsageFlags const bufferUsage,
                  VmaAllocationCreateFlags const allocFlags,
                  CustomUniqPtr<VkBuffer_T> *const out,
                  VmaAllocation *const outAlloc) {
  if (!backend)
    return false;

  if (!out) {
    addErrMsg(backend, __func__, "The parameter 'out' = nullptr");
    return false;
  }

  if (!outAlloc) {
    addErrMsg(backend, __func__, "The parameter 'outAlloc' = nullptr");
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
  if (!getVulkanAllocator(backend, &allocator)) {
    addErrMsg(backend, __func__, "Failed to get VMA allocator");
    return false;
  }

  VmaAllocation allocation;
  VkBuffer buffer{};

  auto r =
      vmaCreateBuffer(allocator, &bufInfo, &allocInfo, &buffer, &allocation, 0);

  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create buffer", r);
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
                         CustomUniqPtr<VkBuffer_T> *const outBuf,
                         VmaAllocation *const outAlloc) {

  if (!createBuffer(backend,
                    bufferSize,
                    bufferUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                        VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    outBuf,
                    outAlloc)) {
    addErrMsg(backend, __func__, "Failed to create buffer");
    return false;
  }

  return true;
}
} // namespace re
