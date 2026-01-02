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

#pragma once

#include <badline/renderEngine.hpp>
#include <vulkan/vulkan.h>
#include <string>
#include <memory>

namespace re {
struct InstanceT;
struct DeviceT;
struct WindowT;

struct PushConstants {
  glm::mat4 camProj{glm::mat4(1)};
  glm::mat4 camView{glm::mat4(1)};
};

enum class BufferUsage { VertexBuffer, IndexBuffer, InstanceBuffer };

struct RenderEngineT {
  std::unique_ptr<InstanceT> instance{};
  std::unique_ptr<DeviceT> device{};
  std::unique_ptr<WindowT> window{};

  std::string errorMessage{};

  PushConstants camMats{};

  UniqueResource<VkBuffer_T> vertexBuf{};
  std::size_t vertexBufSize{};

  UniqueResource<VkBuffer_T> indexBuf{};
  std::size_t indexBufSize{};

  UniqueResource<VkBuffer_T> instanceBuf{};
  std::size_t instanceBufSize{};
  uint32_t instanceCount{};

  UniqueResource<VkDescriptorPool_T> descPool{};
  UniqueResource<VkDescriptorSetLayout_T> descLayout{};
  std::vector<VkDescriptorSet> descSets{};

  static constexpr std::size_t maxDescriptors{12};
};

Result render(RenderEngineT *const engine);

void setErrMsg(RenderEngineT *const engine,
               std::string const &msg,
               VkResult r = VkResult::VK_SUCCESS);
} // namespace re
