#pragma once

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
