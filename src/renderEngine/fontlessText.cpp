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

#include <badline/vertexData.hpp>
#include <unordered_map>
#include <vector>

namespace re {
void updateVerticesAndIndices(
    VertexData const &v1,
    VertexData const &v2,
    VertexData const &v3,
    std::vector<VertexData> &vertices,
    std::vector<uint32_t> &indices,
    std::unordered_map<VertexData, std::size_t> &indexMap) {

  if (!indexMap.contains(v1)) {
    vertices.push_back(std::move(v1));
    indices.push_back(vertices.size() - 1);
    indexMap.emplace(v1, indices.back());
  } else
    indices.push_back(indexMap.at(v1));

  if (!indexMap.contains(v2)) {
    vertices.push_back(std::move(v2));
    indices.push_back(vertices.size() - 1);
    indexMap.emplace(v2, indices.back());
  } else
    indices.push_back(indexMap.at(v2));

  if (!indexMap.contains(v3)) {
    vertices.push_back(std::move(v3));
    indices.push_back(vertices.size() - 1);
    indexMap.emplace(v3, indices.back());
  } else
    indices.push_back(indexMap.at(v3));
}

/*    -----
 *    |  /
 *    | /
 *    |/
 */
void mktl(glm::vec3 const &offset,
          float const x,
          float const y,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices,
          std::unordered_map<VertexData, std::size_t> &indexMap) {
  VertexData v1{}, v2{}, v3{};

  v1.quad0.x = offset.x + x;
  v1.quad0.y = offset.y - y;

  v2.quad0.x = offset.x + (x + 1);
  v2.quad0.y = v1.quad0.y;

  v3.quad0.x = v1.quad0.x;
  v3.quad0.y = offset.y - (y + 1);

  updateVerticesAndIndices(v1, v2, v3, vertices, indices, indexMap);
}

/*    -----
 *     \  |
 *      \ |
 *       \|
 */
void mktr(glm::vec3 const &offset,
          float const x,
          float const y,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices,
          std::unordered_map<VertexData, std::size_t> &indexMap) {
  VertexData v1{}, v2{}, v3{};

  v1.quad0.x = offset.x + x;
  v1.quad0.y = offset.y - y;

  v2.quad0.x = offset.x + (x + 1);
  v2.quad0.y = v1.quad0.y;

  v3.quad0.x = v2.quad0.x;
  v3.quad0.y = offset.y - (y + 1);

  updateVerticesAndIndices(v1, v2, v3, vertices, indices, indexMap);
}

/*    |\
 *    | \
 *    |  \
 *    -----
 */
void mkbl(glm::vec3 const &offset,
          float const x,
          float const y,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices,
          std::unordered_map<VertexData, std::size_t> &indexMap) {
  VertexData v1{}, v2{}, v3{};

  v1.quad0.x = offset.x + x;
  v1.quad0.y = offset.y - y;

  v2.quad0.x = offset.x + (x + 1);
  v2.quad0.y = offset.y - (y + 1);

  v3.quad0.x = v1.quad0.x;
  v3.quad0.y = v2.quad0.y;

  updateVerticesAndIndices(v1, v2, v3, vertices, indices, indexMap);
}

/*       /|
 *      / |
 *     /  |
 *    -----
 */
void mkbr(glm::vec3 const &offset,
          float const x,
          float const y,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices,
          std::unordered_map<VertexData, std::size_t> &indexMap) {
  VertexData v1{}, v2{}, v3{};

  v1.quad0.x = offset.x + (x + 1);
  v1.quad0.y = offset.y - y;

  v2.quad0.x = offset.x + (x + 1);
  v2.quad0.y = offset.y - (y + 1);

  v3.quad0.x = offset.x + x;
  v3.quad0.y = v2.quad0.y;

  updateVerticesAndIndices(v1, v2, v3, vertices, indices, indexMap);
}

/*    -----
 *    |   |
 *    |   |
 *    -----
 */
void mksq(glm::vec3 const &offset,
          float const x,
          float const y,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices,
          std::unordered_map<VertexData, std::size_t> &indexMap) {
  mktl(offset, x, y, vertices, indices, indexMap);
  mkbr(offset, x, y, vertices, indices, indexMap);
}

void addA(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  // top
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mktl(offset, 1, 1, vertices, indices, indexMap);
  mkbr(offset, 1, 0, vertices, indices, indexMap);
  mkbr(offset, 0, 1, vertices, indices, indexMap);
  mkbl(offset, 5, 0, vertices, indices, indexMap);
  mktr(offset, 5, 1, vertices, indices, indexMap);
  mkbl(offset, 6, 1, vertices, indices, indexMap);

  // bridge
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);

  // left leg
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  // right leg
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addB(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  // left
  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  // top bridge
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mkbl(offset, 6, 0, vertices, indices, indexMap);

  // bridge
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);

  // bottom bridge
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);

  // right
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mktl(offset, 6, 6, vertices, indices, indexMap);
  mkbr(offset, 5, 6, vertices, indices, indexMap);
  mkbl(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
}

void addC(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mkbr(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mktr(offset, 0, 14, vertices, indices, indexMap);

  // top bridge
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mkbl(offset, 6, 0, vertices, indices, indexMap);

  // bottom bridge
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);
}

void addD(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  // top bridge
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mkbl(offset, 6, 0, vertices, indices, indexMap);

  // bottom bridge
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);

  // link
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
}

void addE(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  // left
  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  // top bridge
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  // bridge
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  // bottom bridge
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addF(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  // left
  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  // top bridge
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  // bridge
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
}

void addG(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mkbr(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mktr(offset, 0, 14, vertices, indices, indexMap);

  // top bridge
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mkbl(offset, 6, 0, vertices, indices, indexMap);

  // middle bridge
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  // link
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);

  // bottom bridge
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);
}

void addH(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  // bridge
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);

  // left leg
  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  // right leg
  mksq(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addI(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 3, 1, vertices, indices, indexMap);
  mksq(offset, 3, 2, vertices, indices, indexMap);
  mksq(offset, 3, 3, vertices, indices, indexMap);
  mksq(offset, 3, 4, vertices, indices, indexMap);
  mksq(offset, 3, 5, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 3, 9, vertices, indices, indexMap);
  mksq(offset, 3, 10, vertices, indices, indexMap);
  mksq(offset, 3, 11, vertices, indices, indexMap);
  mksq(offset, 3, 12, vertices, indices, indexMap);
  mksq(offset, 3, 13, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addJ(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mktl(offset, 6, 13, vertices, indices, indexMap);
  mkbr(offset, 5, 13, vertices, indices, indexMap);

  mkbl(offset, 1, 13, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mktl(offset, 5, 14, vertices, indices, indexMap);

  mktr(offset, 1, 14, vertices, indices, indexMap);
  mktr(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
}

void addK(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 1, 5, vertices, indices, indexMap);
  mksq(offset, 2, 4, vertices, indices, indexMap);
  mksq(offset, 3, 3, vertices, indices, indexMap);
  mksq(offset, 4, 2, vertices, indices, indexMap);
  mksq(offset, 5, 1, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mkbr(offset, 1, 4, vertices, indices, indexMap);
  mkbr(offset, 2, 3, vertices, indices, indexMap);
  mkbr(offset, 3, 2, vertices, indices, indexMap);
  mkbr(offset, 4, 1, vertices, indices, indexMap);
  mkbr(offset, 5, 0, vertices, indices, indexMap);

  mksq(offset, 1, 6, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 4, 9, vertices, indices, indexMap);
  mksq(offset, 5, 10, vertices, indices, indexMap);
  mkbl(offset, 6, 11, vertices, indices, indexMap);

  mktr(offset, 1, 7, vertices, indices, indexMap);
  mktr(offset, 2, 8, vertices, indices, indexMap);
  mktr(offset, 3, 9, vertices, indices, indexMap);
  mktr(offset, 4, 10, vertices, indices, indexMap);
  mktr(offset, 5, 11, vertices, indices, indexMap);

  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addL(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addM(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 1, 1, vertices, indices, indexMap);
  mksq(offset, 1, 2, vertices, indices, indexMap);
  mksq(offset, 1, 3, vertices, indices, indexMap);

  mksq(offset, 5, 1, vertices, indices, indexMap);
  mksq(offset, 5, 2, vertices, indices, indexMap);
  mksq(offset, 5, 3, vertices, indices, indexMap);

  mksq(offset, 2, 2, vertices, indices, indexMap);
  mksq(offset, 2, 3, vertices, indices, indexMap);
  mksq(offset, 2, 4, vertices, indices, indexMap);

  mksq(offset, 4, 2, vertices, indices, indexMap);
  mksq(offset, 4, 3, vertices, indices, indexMap);
  mksq(offset, 4, 4, vertices, indices, indexMap);

  mksq(offset, 3, 3, vertices, indices, indexMap);
  mksq(offset, 3, 4, vertices, indices, indexMap);
  mksq(offset, 3, 5, vertices, indices, indexMap);
}

void addN(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 1, 2, vertices, indices, indexMap);
  mksq(offset, 1, 3, vertices, indices, indexMap);
  mksq(offset, 1, 4, vertices, indices, indexMap);

  mksq(offset, 2, 4, vertices, indices, indexMap);
  mksq(offset, 2, 5, vertices, indices, indexMap);
  mksq(offset, 2, 6, vertices, indices, indexMap);

  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);

  mksq(offset, 4, 8, vertices, indices, indexMap);
  mksq(offset, 4, 9, vertices, indices, indexMap);
  mksq(offset, 4, 10, vertices, indices, indexMap);

  mksq(offset, 5, 10, vertices, indices, indexMap);
  mksq(offset, 5, 11, vertices, indices, indexMap);
  mksq(offset, 5, 12, vertices, indices, indexMap);
}

void addO(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mkbr(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mktr(offset, 0, 14, vertices, indices, indexMap);

  mkbl(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);

  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
}

void addP(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  // left
  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  // top bridge
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  // bridge
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  // link
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
}

void addQ(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mkbr(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mktr(offset, 0, 14, vertices, indices, indexMap);

  mkbl(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);

  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);

  mksq(offset, 2, 10, vertices, indices, indexMap);
  mksq(offset, 3, 11, vertices, indices, indexMap);
  mksq(offset, 4, 12, vertices, indices, indexMap);
  mksq(offset, 5, 13, vertices, indices, indexMap);
  mkbl(offset, 2, 9, vertices, indices, indexMap);
  mkbl(offset, 3, 10, vertices, indices, indexMap);
  mkbl(offset, 4, 11, vertices, indices, indexMap);
  mkbl(offset, 5, 12, vertices, indices, indexMap);
  mktr(offset, 2, 11, vertices, indices, indexMap);
  mktr(offset, 3, 12, vertices, indices, indexMap);
  mktr(offset, 4, 13, vertices, indices, indexMap);
}

void addR(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  // left
  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  // top bridge
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mkbl(offset, 6, 0, vertices, indices, indexMap);

  // bridge
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);

  // right
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mktl(offset, 6, 6, vertices, indices, indexMap);
  mkbr(offset, 5, 6, vertices, indices, indexMap);
  mkbl(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addS(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mkbr(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mktl(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mktr(offset, 0, 7, vertices, indices, indexMap);

  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mkbl(offset, 6, 7, vertices, indices, indexMap);

  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);

  mkbr(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);
}

void addT(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 3, 1, vertices, indices, indexMap);
  mksq(offset, 3, 2, vertices, indices, indexMap);
  mksq(offset, 3, 3, vertices, indices, indexMap);
  mksq(offset, 3, 4, vertices, indices, indexMap);
  mksq(offset, 3, 5, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 3, 9, vertices, indices, indexMap);
  mksq(offset, 3, 10, vertices, indices, indexMap);
  mksq(offset, 3, 11, vertices, indices, indexMap);
  mksq(offset, 3, 12, vertices, indices, indexMap);
  mksq(offset, 3, 13, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
}

void addU(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mktr(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
}

void addV(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 3, 13, vertices, indices, indexMap);
  mksq(offset, 3, 12, vertices, indices, indexMap);
  mksq(offset, 2, 12, vertices, indices, indexMap);
  mksq(offset, 2, 11, vertices, indices, indexMap);
  mksq(offset, 2, 10, vertices, indices, indexMap);
  mksq(offset, 1, 10, vertices, indices, indexMap);
  mksq(offset, 1, 9, vertices, indices, indexMap);
  mksq(offset, 1, 8, vertices, indices, indexMap);

  mksq(offset, 4, 12, vertices, indices, indexMap);
  mksq(offset, 4, 11, vertices, indices, indexMap);
  mksq(offset, 4, 10, vertices, indices, indexMap);
  mksq(offset, 5, 10, vertices, indices, indexMap);
  mksq(offset, 5, 9, vertices, indices, indexMap);
  mksq(offset, 5, 8, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 0, vertices, indices, indexMap);
}

void addW(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mktr(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);

  mksq(offset, 3, 3, vertices, indices, indexMap);
  mksq(offset, 3, 4, vertices, indices, indexMap);
  mksq(offset, 3, 5, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 3, 9, vertices, indices, indexMap);
  mksq(offset, 3, 10, vertices, indices, indexMap);
  mksq(offset, 3, 11, vertices, indices, indexMap);
  mksq(offset, 3, 12, vertices, indices, indexMap);
  mksq(offset, 3, 13, vertices, indices, indexMap);
  mktl(offset, 3, 14, vertices, indices, indexMap);
}

void addX(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 1, 2, vertices, indices, indexMap);
  mksq(offset, 1, 3, vertices, indices, indexMap);
  mksq(offset, 1, 4, vertices, indices, indexMap);
  mksq(offset, 2, 4, vertices, indices, indexMap);
  mksq(offset, 2, 5, vertices, indices, indexMap);
  mksq(offset, 2, 6, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 4, 8, vertices, indices, indexMap);
  mksq(offset, 4, 9, vertices, indices, indexMap);
  mksq(offset, 4, 10, vertices, indices, indexMap);
  mksq(offset, 5, 10, vertices, indices, indexMap);
  mksq(offset, 5, 11, vertices, indices, indexMap);
  mksq(offset, 5, 12, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 1, 12, vertices, indices, indexMap);
  mksq(offset, 1, 11, vertices, indices, indexMap);
  mksq(offset, 1, 10, vertices, indices, indexMap);
  mksq(offset, 2, 10, vertices, indices, indexMap);
  mksq(offset, 2, 9, vertices, indices, indexMap);
  mksq(offset, 2, 8, vertices, indices, indexMap);
  mksq(offset, 4, 6, vertices, indices, indexMap);
  mksq(offset, 4, 5, vertices, indices, indexMap);
  mksq(offset, 4, 4, vertices, indices, indexMap);
  mksq(offset, 5, 4, vertices, indices, indexMap);
  mksq(offset, 5, 3, vertices, indices, indexMap);
  mksq(offset, 5, 2, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);
}

void addY(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 1, 2, vertices, indices, indexMap);
  mksq(offset, 1, 3, vertices, indices, indexMap);
  mksq(offset, 1, 4, vertices, indices, indexMap);
  mksq(offset, 2, 4, vertices, indices, indexMap);
  mksq(offset, 2, 5, vertices, indices, indexMap);
  mksq(offset, 2, 6, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);

  mksq(offset, 4, 6, vertices, indices, indexMap);
  mksq(offset, 4, 5, vertices, indices, indexMap);
  mksq(offset, 4, 4, vertices, indices, indexMap);
  mksq(offset, 5, 4, vertices, indices, indexMap);
  mksq(offset, 5, 3, vertices, indices, indexMap);
  mksq(offset, 5, 2, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 3, 9, vertices, indices, indexMap);
  mksq(offset, 3, 10, vertices, indices, indexMap);
  mksq(offset, 3, 11, vertices, indices, indexMap);
  mksq(offset, 3, 12, vertices, indices, indexMap);
  mksq(offset, 3, 13, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
}

void addZ(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 1, 12, vertices, indices, indexMap);
  mksq(offset, 1, 11, vertices, indices, indexMap);
  mksq(offset, 1, 10, vertices, indices, indexMap);
  mksq(offset, 2, 10, vertices, indices, indexMap);
  mksq(offset, 2, 9, vertices, indices, indexMap);
  mksq(offset, 2, 8, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 4, 6, vertices, indices, indexMap);
  mksq(offset, 4, 5, vertices, indices, indexMap);
  mksq(offset, 4, 4, vertices, indices, indexMap);
  mksq(offset, 5, 4, vertices, indices, indexMap);
  mksq(offset, 5, 3, vertices, indices, indexMap);
  mksq(offset, 5, 2, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);
}

void add0(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);

  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
}

void add1(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 2, 1, vertices, indices, indexMap);
  mksq(offset, 1, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);

  mkbr(offset, 2, 0, vertices, indices, indexMap);
  mkbr(offset, 1, 1, vertices, indices, indexMap);
  mkbr(offset, 0, 2, vertices, indices, indexMap);
  mktl(offset, 1, 3, vertices, indices, indexMap);
  mktl(offset, 2, 2, vertices, indices, indexMap);

  mksq(offset, 3, 1, vertices, indices, indexMap);
  mksq(offset, 3, 2, vertices, indices, indexMap);
  mksq(offset, 3, 3, vertices, indices, indexMap);
  mksq(offset, 3, 4, vertices, indices, indexMap);
  mksq(offset, 3, 5, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 3, 9, vertices, indices, indexMap);
  mksq(offset, 3, 10, vertices, indices, indexMap);
  mksq(offset, 3, 11, vertices, indices, indexMap);
  mksq(offset, 3, 12, vertices, indices, indexMap);
  mksq(offset, 3, 13, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void add2(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mkbr(offset, 0, 1, vertices, indices, indexMap);
  mkbr(offset, 1, 0, vertices, indices, indexMap);
  mktl(offset, 1, 1, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mkbl(offset, 5, 0, vertices, indices, indexMap);
  mktr(offset, 5, 1, vertices, indices, indexMap);
  mkbl(offset, 6, 1, vertices, indices, indexMap);

  mksq(offset, 6, 2, vertices, indices, indexMap);
  mktl(offset, 6, 3, vertices, indices, indexMap);
  mkbr(offset, 5, 3, vertices, indices, indexMap);
  mktl(offset, 5, 4, vertices, indices, indexMap);
  mkbr(offset, 4, 4, vertices, indices, indexMap);
  mktl(offset, 4, 5, vertices, indices, indexMap);
  mkbr(offset, 3, 5, vertices, indices, indexMap);
  mktl(offset, 3, 6, vertices, indices, indexMap);
  mkbr(offset, 2, 6, vertices, indices, indexMap);
  mktl(offset, 2, 7, vertices, indices, indexMap);
  mkbr(offset, 1, 7, vertices, indices, indexMap);
  mktl(offset, 1, 8, vertices, indices, indexMap);
  mkbr(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void add3(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mkbl(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mktl(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);

  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
}

void add4(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 6, 0, vertices, indices, indexMap);
  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);

  // bridge
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
}

void add5(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);

  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void add6(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void add7(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void add8(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 0, 9, vertices, indices, indexMap);
  mksq(offset, 0, 10, vertices, indices, indexMap);
  mksq(offset, 0, 11, vertices, indices, indexMap);
  mksq(offset, 0, 12, vertices, indices, indexMap);
  mksq(offset, 0, 13, vertices, indices, indexMap);
  mksq(offset, 0, 14, vertices, indices, indexMap);

  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);

  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void add9(glm::vec3 const &offset,
          std::vector<VertexData> &vertices,
          std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 0, vertices, indices, indexMap);
  mksq(offset, 1, 0, vertices, indices, indexMap);
  mksq(offset, 2, 0, vertices, indices, indexMap);
  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 4, 0, vertices, indices, indexMap);
  mksq(offset, 5, 0, vertices, indices, indexMap);
  mksq(offset, 6, 0, vertices, indices, indexMap);

  mksq(offset, 0, 1, vertices, indices, indexMap);
  mksq(offset, 0, 2, vertices, indices, indexMap);
  mksq(offset, 0, 3, vertices, indices, indexMap);
  mksq(offset, 0, 4, vertices, indices, indexMap);
  mksq(offset, 0, 5, vertices, indices, indexMap);
  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 0, 7, vertices, indices, indexMap);

  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  mksq(offset, 6, 1, vertices, indices, indexMap);
  mksq(offset, 6, 2, vertices, indices, indexMap);
  mksq(offset, 6, 3, vertices, indices, indexMap);
  mksq(offset, 6, 4, vertices, indices, indexMap);
  mksq(offset, 6, 5, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);

  mksq(offset, 6, 8, vertices, indices, indexMap);
  mksq(offset, 6, 9, vertices, indices, indexMap);
  mksq(offset, 6, 10, vertices, indices, indexMap);
  mksq(offset, 6, 11, vertices, indices, indexMap);
  mksq(offset, 6, 12, vertices, indices, indexMap);
  mksq(offset, 6, 13, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addColon(glm::vec3 const &offset,
              std::vector<VertexData> &vertices,
              std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 3, 5, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 3, 9, vertices, indices, indexMap);
}

void addMinus(glm::vec3 const &offset,
              std::vector<VertexData> &vertices,
              std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);
}

void addPlus(glm::vec3 const &offset,
             std::vector<VertexData> &vertices,
             std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 7, vertices, indices, indexMap);
  mksq(offset, 1, 7, vertices, indices, indexMap);
  mksq(offset, 2, 7, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 4, 7, vertices, indices, indexMap);
  mksq(offset, 5, 7, vertices, indices, indexMap);
  mksq(offset, 6, 7, vertices, indices, indexMap);

  mksq(offset, 3, 4, vertices, indices, indexMap);
  mksq(offset, 3, 5, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 3, 9, vertices, indices, indexMap);
  mksq(offset, 3, 10, vertices, indices, indexMap);
}

void addEq(glm::vec3 const &offset,
           std::vector<VertexData> &vertices,
           std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 6, vertices, indices, indexMap);
  mksq(offset, 1, 6, vertices, indices, indexMap);
  mksq(offset, 2, 6, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 4, 6, vertices, indices, indexMap);
  mksq(offset, 5, 6, vertices, indices, indexMap);
  mksq(offset, 6, 6, vertices, indices, indexMap);

  mksq(offset, 0, 8, vertices, indices, indexMap);
  mksq(offset, 1, 8, vertices, indices, indexMap);
  mksq(offset, 2, 8, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 4, 8, vertices, indices, indexMap);
  mksq(offset, 5, 8, vertices, indices, indexMap);
  mksq(offset, 6, 8, vertices, indices, indexMap);
}

void addUnderscore(glm::vec3 const &offset,
                   std::vector<VertexData> &vertices,
                   std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 0, 14, vertices, indices, indexMap);
  mksq(offset, 1, 14, vertices, indices, indexMap);
  mksq(offset, 2, 14, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
  mksq(offset, 5, 14, vertices, indices, indexMap);
  mksq(offset, 6, 14, vertices, indices, indexMap);
}

void addDot(glm::vec3 const &offset,
            std::vector<VertexData> &vertices,
            std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 3, 13, vertices, indices, indexMap);
  mksq(offset, 4, 13, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
  mksq(offset, 4, 14, vertices, indices, indexMap);
}

void addEx(glm::vec3 const &offset,
           std::vector<VertexData> &vertices,
           std::vector<uint32_t> &indices) {

  std::unordered_map<VertexData, std::size_t> indexMap;

  mksq(offset, 3, 0, vertices, indices, indexMap);
  mksq(offset, 3, 1, vertices, indices, indexMap);
  mksq(offset, 3, 2, vertices, indices, indexMap);
  mksq(offset, 3, 3, vertices, indices, indexMap);
  mksq(offset, 3, 4, vertices, indices, indexMap);
  mksq(offset, 3, 5, vertices, indices, indexMap);
  mksq(offset, 3, 6, vertices, indices, indexMap);
  mksq(offset, 3, 7, vertices, indices, indexMap);
  mksq(offset, 3, 8, vertices, indices, indexMap);
  mksq(offset, 3, 9, vertices, indices, indexMap);
  mksq(offset, 3, 10, vertices, indices, indexMap);

  mksq(offset, 3, 13, vertices, indices, indexMap);
  mksq(offset, 3, 14, vertices, indices, indexMap);
}

struct Object;

bool loadFromRAM(Object *const handle,
                 std::vector<VertexData> vertices,
                 std::vector<uint32_t> indices);

bool setFontlessText(Object *const object,
                     std::string const &text) {

  constexpr std::size_t size = 256, gridWidth = 7;
  constexpr float spacing = 1.f;

  typedef void (*createSig)(glm::vec3 const &offset,
                            std::vector<VertexData> &vertices,
                            std::vector<uint32_t> &indices);

  static createSig dispatch[size];
  static bool init = true;

  if (init) {
    for (std::size_t i = 0; i < size; ++i)
      dispatch[i] = 0;

    dispatch['A'] = addA;
    dispatch['B'] = addB;
    dispatch['C'] = addC;
    dispatch['D'] = addD;
    dispatch['E'] = addE;
    dispatch['F'] = addF;
    dispatch['G'] = addG;
    dispatch['H'] = addH;
    dispatch['I'] = addI;
    dispatch['J'] = addJ;
    dispatch['K'] = addK;
    dispatch['L'] = addL;
    dispatch['M'] = addM;
    dispatch['N'] = addN;
    dispatch['O'] = addO;
    dispatch['P'] = addP;
    dispatch['Q'] = addQ;
    dispatch['R'] = addR;
    dispatch['S'] = addS;
    dispatch['T'] = addT;
    dispatch['U'] = addU;
    dispatch['V'] = addV;
    dispatch['W'] = addW;
    dispatch['X'] = addX;
    dispatch['Y'] = addY;
    dispatch['Z'] = addZ;

    dispatch['0'] = add0;
    dispatch['1'] = add1;
    dispatch['2'] = add2;
    dispatch['3'] = add3;
    dispatch['4'] = add4;
    dispatch['5'] = add5;
    dispatch['6'] = add6;
    dispatch['7'] = add7;
    dispatch['8'] = add8;
    dispatch['9'] = add9;

    dispatch['-'] = addMinus;
    dispatch['+'] = addPlus;
    dispatch['='] = addEq;
    dispatch[':'] = addColon;
    dispatch['_'] = addUnderscore;
    dispatch['.'] = addDot;
    dispatch['!'] = addEx;

    init = false;
  }

  std::vector<VertexData> vertices;
  std::vector<uint32_t> indices;
  glm::vec3 offset{};

  for (std::size_t i = 0; i < text.size(); ++i) {
    auto createFunc = dispatch[(std::size_t)text[i]];
    if (createFunc || text[i] == ' ') {
      if (createFunc)
        createFunc(offset, vertices, indices);
      offset.x += gridWidth + spacing;
    }
  }

  return loadFromRAM(object, std::move(vertices), std::move(indices));
}
} // namespace re
