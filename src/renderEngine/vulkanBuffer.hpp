#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace re {
struct VulkanBuffer {
  CustomUniqPtr<VkBuffer_T> handle;
  VmaAllocationInfo allocInfo;
};
} // namespace re
