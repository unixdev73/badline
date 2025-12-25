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
Result
createGLFWindow(RenderEngineT *const engine, uint32_t width, uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

  GLFWwindow *win =
      glfwCreateWindow(width, height, engine->instance->title.c_str(), 0, 0);
  if (!win)
    return Result::ErrorGLFWindowCreationFailure;

  engine->window->handle = {win, [](GLFWwindow *const ptr) {
                              if (ptr)
                                glfwDestroyWindow(ptr);
                            }};
  engine->window->width = width;
  engine->window->height = height;

  return Result::Success;
}

Result createWindowSurface(RenderEngineT *const engine) {
  auto const win = engine->window->handle.get();
  VkSurfaceKHR surf{};
  if (auto result = glfwCreateWindowSurface(
          engine->instance->handle.get(), win, 0, &surf);
      result != VK_SUCCESS)
    return Result::ErrorVulkanSurfaceCreationFailure;

  auto inst = engine->instance->handle.get();
  engine->window->surface = {surf, [inst](VkSurfaceKHR_T *const ptr) {
                               if (ptr)
                                 vkDestroySurfaceKHR(inst, ptr, 0);
                             }};

  return Result::Success;
}

Result queryPresentModes(RenderEngineT *const engine) {
  engine->window->presentModes.clear();
  uint32_t count{};
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          engine->device->identifier,
          engine->window->surface.get(),
          &count,
          nullptr);
      result != VK_SUCCESS)
    return Result::ErrorPresentModesQueryFailure;

  engine->window->presentModes.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          engine->device->identifier,
          engine->window->surface.get(),
          &count,
          engine->window->presentModes.data());
      result != VK_SUCCESS)
    return Result::ErrorPresentModesFillFailure;

  return Result::Success;
}

Result checkRequestedPresentModeAvailable(RenderEngineT *const engine) {
  bool found = false;
  for (auto mode : engine->window->presentModes) {
    if (mode == engine->window->presentMode) {
      found = true;
      break;
    }
  }
  if (!found)
    return Result::ErrorRequestedPresentModeNotAvailable;

  return Result::Success;
}

Result checkSurfaceEligibility(RenderEngineT *const engine) {
  uint32_t count{};
  if (auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          engine->device->identifier,
          engine->window->surface.get(),
          &engine->window->surfaceCaps);
      result != VK_SUCCESS)
    return Result::ErrorSurfaceCapabilitiesQueryFailure;

  if (engine->window->width > engine->window->surfaceCaps.maxImageExtent.width)
    return Result::ErrorRequestedSurfaceWidthTooLarge;

  if (engine->window->height >
      engine->window->surfaceCaps.maxImageExtent.height)
    return Result::ErrorRequestedSurfaceHeightTooLarge;

  if (!(engine->window->surfaceCaps.supportedUsageFlags &
        VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
    return Result::ErrorColorAttachmentBitNotSupported;

  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          engine->device->identifier, engine->window->surface.get(), &count, 0);
      result != VK_SUCCESS)
    return Result::ErrorSurfaceFormatQueryFailure;

  engine->window->surfaceFormats.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          engine->device->identifier,
          engine->window->surface.get(),
          &count,
          engine->window->surfaceFormats.data());
      result != VK_SUCCESS)
    return Result::ErrorSurfaceFormatQueryFailure;

  if ((1 == engine->window->surfaceFormats.size()) &&
      (VK_FORMAT_UNDEFINED == engine->window->surfaceFormats[0].format)) {
    engine->window->surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    engine->window->surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  } else if (engine->window->surfaceFormats.size()) {
    engine->window->surfaceFormat = engine->window->surfaceFormats[0];
  } else
    return Result::ErrorNoSurfaceFormatsAvailable;

  return Result::Success;
}

Result createWindowSwapchain(RenderEngineT *const engine) {
  auto numberOfImages = engine->window->surfaceCaps.minImageCount + 1;
  if ((engine->window->surfaceCaps.maxImageCount > 0) &&
      (numberOfImages > engine->window->surfaceCaps.maxImageCount)) {
    numberOfImages = engine->window->surfaceCaps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swpInfo{};
  swpInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swpInfo.compositeAlpha =
      VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swpInfo.imageArrayLayers = 1;
  swpInfo.clipped = VK_TRUE;
  swpInfo.imageColorSpace = engine->window->surfaceFormat.colorSpace;
  swpInfo.imageFormat = engine->window->surfaceFormat.format;

  uint32_t const width = engine->window->width, height = engine->window->height;
  swpInfo.imageExtent = VkExtent2D{width, height};
  swpInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swpInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swpInfo.surface = engine->window->surface.get();
  swpInfo.presentMode = engine->window->presentMode;
  swpInfo.minImageCount = numberOfImages;
  swpInfo.preTransform =
      VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

  VkSwapchainKHR swapchain{};
  if (auto result = vkCreateSwapchainKHR(
          engine->device->handle.get(), &swpInfo, 0, &swapchain);
      result != VK_SUCCESS)
    return Result::ErrorVulkanSwapchainCreationFailure;

  auto dev = engine->device->handle.get();
  engine->window->swapchain = {swapchain, [dev](VkSwapchainKHR_T *const ptr) {
                                 vkDestroySwapchainKHR(dev, ptr, 0);
                               }};

  return Result::Success;
}

Result fetchSwapchainImages(RenderEngineT *const engine) {
  uint32_t count{};

  if (auto result = vkGetSwapchainImagesKHR(engine->device->handle.get(),
                                            engine->window->swapchain.get(),
                                            &count,
                                            0);
      result != VK_SUCCESS)
    return Result::ErrorSwapchainImageQueryFailure;

  engine->window->swapImages.resize(count);

  if (auto result = vkGetSwapchainImagesKHR(engine->device->handle.get(),
                                            engine->window->swapchain.get(),
                                            &count,
                                            engine->window->swapImages.data());
      result != VK_SUCCESS)
    return Result::ErrorSwapchainImageFillFailure;

  return Result::Success;
}

Result createWindow(RenderEngineT *const engine,
                    uint32_t const width,
                    uint32_t const height) {

  if (auto r = createGLFWindow(engine, width, height); r != Result::Success)
    return r;

  if (auto r = createWindowSurface(engine); r != Result::Success)
    return r;

  if (auto r = queryPresentModes(engine); r != Result::Success)
    return r;

  if (auto r = checkRequestedPresentModeAvailable(engine); r != Result::Success)
    return r;

  if (auto r = checkSurfaceEligibility(engine); r != Result::Success)
    return r;

  if (auto r = createWindowSwapchain(engine); r != Result::Success)
    return r;

  if (auto r = fetchSwapchainImages(engine); r != Result::Success)
    return r;

  return Result::Success;
}
} // namespace re
