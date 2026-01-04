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

#pragma once

#include <badline/transformMatrix.hpp>
#include <badline/vertices.hpp>
#include <badline/indices.hpp>
#include <badline/instances.hpp>
#include "vulkanBackend.hpp"
#include "error.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace re {
struct VulkanBackend;

struct Vertices {
  Vertices(ErrorLogs *p) : logs{p} {}
  mutable ErrorLogs *logs{};
  std::vector<Vertex> vertices{};
};

struct Indices {
  Indices(ErrorLogs *p) : logs{p} {}
  mutable ErrorLogs *logs{};
  std::vector<uint32_t> indices{};
};

struct Instances {
  Instances(ErrorLogs *p) : logs{p} {}
  mutable ErrorLogs *logs{};
  std::vector<InstanceTransform> transformData{};
};

struct TransformMatrix {
  TransformMatrix(ErrorLogs *p) : logs{p} {}
  mutable ErrorLogs *logs{};
  glm::mat4 transform{};
};

struct CameraTransforms {
  glm::mat4 projection{};
  glm::mat4 view{};
};

struct RenderEngine {
  mutable ErrorLogs logs{};
  CameraTransforms cam{};

  std::unique_ptr<VulkanBackend> vulkanBackend{};
  bool backendInitialized{false};

  std::unique_ptr<TransformMatrix> projection{};
  std::unique_ptr<TransformMatrix> view{};

  std::unique_ptr<Vertices> vertices{};
  std::unique_ptr<Indices> indices{};
  std::unique_ptr<Instances> instances{};
};

void addErrMsg(RenderEngine const *const engine, std::string const &msg);
} // namespace re
