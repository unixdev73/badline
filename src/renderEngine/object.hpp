#pragma once

#include "vulkanBuffer.hpp"
#include <glm/glm.hpp>

namespace re {
struct VertexData {
  glm::vec4 quad0; // posX, posY, posZ, texX
  glm::vec4 quad1; // texY, norX, norY, norZ
  glm::vec4 quad2; // colR, colG, colB, colA
};

inline bool operator==(re::VertexData const &a, re::VertexData const &b) {
  return a.quad0 == b.quad0 && a.quad1 == b.quad1 && a.quad2 == b.quad2;
}
} // namespace re

namespace std {
template <> struct hash<re::VertexData> {
  size_t operator()(const re::VertexData &v) const {
    size_t h = 0;
    auto combine = [](size_t lhs, size_t rhs) {
      return lhs ^ (rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2));
    };

    h = combine(h, hash<float>()(v.quad0.x));
    h = combine(h, hash<float>()(v.quad0.y));
    h = combine(h, hash<float>()(v.quad0.z));
    h = combine(h, hash<float>()(v.quad0.w));

    h = combine(h, hash<float>()(v.quad1.x));
    h = combine(h, hash<float>()(v.quad1.y));
    h = combine(h, hash<float>()(v.quad1.z));
    h = combine(h, hash<float>()(v.quad1.w));

    h = combine(h, hash<float>()(v.quad2.x));
    h = combine(h, hash<float>()(v.quad2.y));
    h = combine(h, hash<float>()(v.quad2.z));
    h = combine(h, hash<float>()(v.quad2.w));

    return h;
  }
};
} // namespace std

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
