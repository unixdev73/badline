#pragma once

#include "smartResource.hpp"
#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h>

namespace re {
struct VulkanBuffer {
  CustomUniqPtr<VkBuffer_T> handle;
  VmaAllocationInfo allocInfo;
};
} // namespace re
