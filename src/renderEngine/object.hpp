#pragma once

#include <badline/vertexData.hpp>
#include "vulkanBuffer.hpp"

namespace re {
struct VulkanBackend;

struct Texture {
  Texture(VulkanBackend *const b) : backend{b} {}
  VulkanBackend *backend{};
  CustomUniqPtr<void> stbImagePtr;
  unsigned width{}, height{}, channels{};
  CustomUniqPtr<VkImage_T> image;
  CustomUniqPtr<VkImageView_T> view;
};

struct Object {
  Object(VulkanBackend *const b, Texture const *const t)
      : backend{b}, texture{t} {}
  VulkanBackend *backend{};
  Texture const *texture{};
  std::vector<VertexData> vertices;
  std::vector<unsigned> indices;
  VulkanBuffer vertexBuffer;
  VulkanBuffer indexBuffer;
  CustomUniqPtr<VkDescriptorSet_T> descriptorSet;
  VkPipelineLayout pipelineLayout{};
  VkPipeline pipeline{};
};

struct ObjectInstance {
  Object const *object;
  glm::mat4 instance;
};
} // namespace re
