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

#include <badline/vertex.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace re {
enum class Result : int {
  Success,
  ErrorNullptrHandle,
  ErrorNullptrWindow,
  ErrorNullptrMessage,
  ErrorNullptrOutput,
  ErrorFailedToInitGLFW,
  ErrorMemoryAllocationFailure,
  ErrorVulkanInstanceCreationFailure,
  ErrorNoVulkanDevicesAvailable,
  ErrorVulkanDeviceCreationFailure,
  ErrorGLFWindowCreationFailure,
  ErrorVulkanSurfaceCreationFailure,
  ErrorPresentModesQueryFailure,
  ErrorPresentModesFillFailure,
  ErrorRequestedPresentModeNotAvailable,
  ErrorSurfaceCapabilitiesQueryFailure,
  ErrorRequestedSurfaceWidthTooLarge,
  ErrorRequestedSurfaceHeightTooLarge,
  ErrorColorAttachmentBitNotSupported,
  ErrorSurfaceFormatQueryFailure,
  ErrorSurfaceFormatFillFailure,
  ErrorNoSurfaceFormatsAvailable,
  ErrorVulkanSwapchainCreationFailure,
  ErrorSwapchainImageQueryFailure,
  ErrorSwapchainImageFillFailure,
  ErrorVulkanCommandPoolCreationFailure,
  ErrorVulkanResultMappingFailure,
  ErrorVulkanCommandBufferAllocationFailure,
  ErrorVulkanMemoryAllocatorCreationFailure,
  ErrorVulkanSemaphoreCreationFailure,
  ErrorVulkanImageViewCreationFailure,
  ErrorSwapchainImageAcquisitionFailure,
  ErrorVulkanFenceCreationFailure,
  ErrorVulkanShaderModuleCreationFailure,
  ErrorVulkanPipelineLayoutCreationFailure,
  ErrorVulkanPipelineCreationFailure,
  ErrorVulkanBufferCreationFailure,
  ErrorDepthImageCreationFailure,
  ErrorCopyToStagingBufferFailure,
  ErrorUnsupportedBufferPurpose,
  ErrorVulkanDescriptorPoolCreationFailure,
  ErrorVulkanDescriptorSetLayoutCreationFailure,
  ErrorVulkanDescriptorSetAllocationFailure,
  ErrorNoErrorMessage
};

Result toString(Result const result, std::string *const output);
} // namespace re

namespace re {
template <typename T>
using UniqueResource = std::unique_ptr<T, std::function<void(T *const)>>;

static constexpr auto BADLINE_VK_API_VERSION = VK_API_VERSION_1_3;

struct RenderEngineT;

using UniqueRenderEngine = UniqueResource<RenderEngineT>;

Result createRenderEngine(RenderEngineT **const handle,
                          std::string const &appName,
                          bool debug);

void destroyRenderEngine(RenderEngineT const *const handle);

UniqueRenderEngine createRenderEngine(std::string const &appName, bool debug);

Result createWindow(RenderEngineT *const handle,
                    uint32_t const width,
                    uint32_t const height);

Result setProjection(RenderEngineT *const handle, glm::mat4 const projection);
Result setView(RenderEngineT *const handle, glm::mat4 const view);

Result setVertices(RenderEngineT *const handle,
                   std::vector<Vertex> const *const vertices);
Result setIndices(RenderEngineT *const handle,
                  std::vector<uint32_t> const *const indices);
Result setInstances(RenderEngineT *const handle,
                    std::vector<glm::mat4> const *const instances,
                    std::size_t const allocElem = 0);

Result isWindowOpen(RenderEngineT *const handle, bool *const output);
Result closeWindow(RenderEngineT *const handle);
Result isKeyPressed(RenderEngineT *const handle, int key, bool *const output);

Result render(RenderEngineT *const handle);

Result getErrorMessage(RenderEngineT const *const handle,
                       std::string *const message);
} // namespace re
