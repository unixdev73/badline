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

#include <badline/renderEngine.hpp>
#include "internals.hpp"
#include "instance.hpp"
#include "device.hpp"
#include "window.hpp"

namespace re {
Result createRenderEngine(RenderEngineT **const handle,
                          std::string const &appName,
                          bool debug) {
  if (!handle)
    return Result::ErrorNullptrHandle;

  if (glfwInit() != GLFW_TRUE)
    return Result::ErrorFailedToInitGLFW;

  if (*handle = new RenderEngineT{}; !*handle)
    return Result::ErrorMemoryAllocationFailure;

	(*handle)->instance = std::make_unique<InstanceT>();
	(*handle)->device = std::make_unique<DeviceT>();
	(*handle)->window = std::make_unique<WindowT>();

  auto result = createVulkanInstance(appName, debug, (*handle)->instance.get());
  if (result != re::Result::Success)
    return Result::ErrorVulkanInstanceCreationFailure;

  if (result = selectOptimalGPU(*handle); result != re::Result::Success)
    return result;

  return Result::Success;
}

void destroyRenderEngine(RenderEngineT *const handle) {
  delete handle;
  glfwTerminate();
}

UniqueRenderEngine createRenderEngine(std::string const &appName, bool debug) {
  RenderEngineT *engine{};
  createRenderEngine(&engine, appName, debug);
  return UniqueRenderEngine{engine, destroyRenderEngine};
}

Result run(RenderEngineT *const handle) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!handle->window->handle)
    return Result::ErrorNullptrWindow;

  while (!glfwWindowShouldClose(handle->window->handle.get())) {
    glfwPollEvents();

    if (glfwGetKey(handle->window->handle.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(handle->window->handle.get(), GLFW_TRUE);
  }

  return Result::Success;
}

Result toString(Result const result, std::string *const output) {
  if (!output)
    return Result::ErrorNullptrOutput;

  switch (result) {
  case Result::Success:
    *output = "Success";
    break;
  case Result::ErrorNullptrHandle:
    *output = "ErrorNullptrHandle";
    break;
  case Result::ErrorNullptrWindow:
    *output = "ErrorNullptrWindow";
    break;
  case Result::ErrorNullptrMessage:
    *output = "ErrorNullptrMessage";
    break;
  case Result::ErrorNullptrOutput:
    *output = "ErrorNullptrOutput";
    break;
  case Result::ErrorFailedToInitGLFW:
    *output = "ErrorFailedToInitGLFW";
    break;
  case Result::ErrorMemoryAllocationFailure:
    *output = "ErrorMemoryAllocationFailure";
    break;
  case Result::ErrorVulkanInstanceCreationFailure:
    *output = "ErrorVulkanInstanceCreationFailure";
    break;
  case Result::ErrorNoVulkanDevicesAvailable:
    *output = "ErrorNoVulkanDevicesAvailable";
    break;
  case Result::ErrorVulkanDeviceCreationFailure:
    *output = "ErrorVulkanDeviceCreationFailure";
    break;
  case Result::ErrorGLFWindowCreationFailure:
    *output = "ErrorGLFWindowCreationFailure";
    break;
  case Result::ErrorVulkanSurfaceCreationFailure:
    *output = "ErrorVulkanSurfaceCreationFailure";
    break;
  case Result::ErrorPresentModesQueryFailure:
    *output = "ErrorPresentModesQueryFailure";
    break;
  case Result::ErrorPresentModesFillFailure:
    *output = "ErrorPresentModesFillFailure";
    break;
  case Result::ErrorRequestedPresentModeNotAvailable:
    *output = "ErrorRequestedPresentModeNotAvailable";
    break;
  case Result::ErrorSurfaceCapabilitiesQueryFailure:
    *output = "ErrorSurfaceCapabilitiesQueryFailure";
    break;
  case Result::ErrorRequestedSurfaceWidthTooLarge:
    *output = "ErrorRequestedSurfaceWidthTooLarge";
    break;
  case Result::ErrorRequestedSurfaceHeightTooLarge:
    *output = "ErrorRequestedSurfaceHeightTooLarge";
    break;
  case Result::ErrorColorAttachmentBitNotSupported:
    *output = "ErrorColorAttachmentBitNotSupported";
    break;
  case Result::ErrorSurfaceFormatQueryFailure:
    *output = "ErrorSurfaceFormatQueryFailure";
    break;
  case Result::ErrorSurfaceFormatFillFailure:
    *output = "ErrorSurfaceFormatFillFailure";
    break;
  case Result::ErrorNoSurfaceFormatsAvailable:
    *output = "ErrorNoSurfaceFormatsAvailable";
    break;
  case Result::ErrorVulkanSwapchainCreationFailure:
    *output = "ErrorVulkanSwapchainCreationFailure";
    break;
  case Result::ErrorSwapchainImageQueryFailure:
    *output = "ErrorSwapchainImageQueryFailure";
    break;
  case Result::ErrorSwapchainImageFillFailure:
    *output = "ErrorSwapchainImageFillFailure";
    break;
  case Result::ErrorNoErrorMessage:
    *output = "ErrorNoErrorMessage";
    break;
  }

  return Result::Success;
}

Result getErrorMessage(RenderEngineT const *const handle,
                       std::string *const message) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!message)
    return Result::ErrorNullptrMessage;
  if (handle->errorMessage.empty())
    return Result::ErrorNoErrorMessage;

  *message = handle->errorMessage;
  return Result::Success;
}
} // namespace re
