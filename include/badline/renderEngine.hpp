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

#include "vulkan/vulkan_core.h"
#include <memory>

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
  ErrorNoErrorMessage
};

Result toString(Result const result, std::string *const output);
} // namespace re

namespace re {
static constexpr auto BADLINE_VK_API_VERSION = VK_API_VERSION_1_3;

struct RenderEngineT;

using UniqueRenderEngine =
    std::unique_ptr<RenderEngineT, void (*)(RenderEngineT *const)>;

Result createRenderEngine(RenderEngineT **const handle,
                          std::string const &appName,
                          bool debug);

void destroyRenderEngine(RenderEngineT const *const handle);

UniqueRenderEngine createRenderEngine(std::string const &appName, bool debug);

Result createWindow(RenderEngineT *const handle,
                    uint32_t const width,
                    uint32_t const height);

Result run(RenderEngineT *const handle);

Result getErrorMessage(RenderEngineT const *const handle,
                       std::string *const message);
} // namespace re
