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
#include <string>

namespace re {
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

Result createRenderEngine(RenderEngineT **const handle,
                          std::string const &appName,
                          bool debug) {
  if (!handle)
    return Result::ErrorNullptrHandle;

  if (glfwInit() != GLFW_TRUE)
    return Result::ErrorFailedToInitGLFW;

  if (*handle = new RenderEngineT{}; !*handle)
    return Result::ErrorMemoryAllocationFailure;

  auto result = createVulkanInstance(appName, debug, &(*handle)->instance);
  if (result != re::Result::Success)
    return Result::ErrorVulkanInstanceCreationFailure;

  if (result = selectOptimalGPU(*handle); result != re::Result::Success)
    return result;

  return Result::Success;
}

Result run(RenderEngineT *const handle) {
  if (!handle)
    return Result::ErrorNullptrHandle;
  if (!handle->window.handle)
    return Result::ErrorNullptrWindow;

  while (!glfwWindowShouldClose(handle->window.handle.get())) {
    glfwPollEvents();

    if (glfwGetKey(handle->window.handle.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(handle->window.handle.get(), GLFW_TRUE);
  }

  return Result::Success;
}

Result createWindow(RenderEngineT *const engine,
                    uint32_t const width,
                    uint32_t const height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

  GLFWwindow *win =
      glfwCreateWindow(width, height, engine->instance.title.c_str(), 0, 0);
  if (!win)
    return Result::ErrorGLFWindowCreationFailure;

  engine->window.handle = {win, [](GLFWwindow *const ptr) {
                             if (ptr)
                               glfwDestroyWindow(ptr);
                           }};
  engine->window.width = width;
  engine->window.height = height;

  VkSurfaceKHR surf{};
  if (auto result =
          glfwCreateWindowSurface(engine->instance.handle.get(), win, 0, &surf);
      result != VK_SUCCESS)
    return Result::ErrorVulkanSurfaceCreationFailure;

  auto inst = engine->instance.handle.get();
  engine->window.surface = {surf, [inst](VkSurfaceKHR_T *const ptr) {
                              if (ptr)
                                vkDestroySurfaceKHR(inst, ptr, 0);
                            }};

  engine->window.presentModes.clear();
  uint32_t count{};
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          engine->device.identifier,
          engine->window.surface.get(),
          &count,
          nullptr);
      result != VK_SUCCESS)
    return Result::ErrorPresentModesQueryFailure;

  engine->window.presentModes.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          engine->device.identifier,
          engine->window.surface.get(),
          &count,
          engine->window.presentModes.data());
      result != VK_SUCCESS)
    return Result::ErrorPresentModesFillFailure;

  bool found = false;
  for (auto mode : engine->window.presentModes) {
    if (mode == engine->window.presentMode) {
      found = true;
      break;
    }
  }
  if (!found)
    return Result::ErrorRequestedPresentModeNotAvailable;

  if (auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          engine->device.identifier,
          engine->window.surface.get(),
          &engine->window.surfaceCaps);
      result != VK_SUCCESS)
    return Result::ErrorSurfaceCapabilitiesQueryFailure;

  auto numberOfImages = engine->window.surfaceCaps.minImageCount + 1;
  if ((engine->window.surfaceCaps.maxImageCount > 0) &&
      (numberOfImages > engine->window.surfaceCaps.maxImageCount)) {
    numberOfImages = engine->window.surfaceCaps.maxImageCount;
  }

  if (engine->window.width > engine->window.surfaceCaps.maxImageExtent.width)
    return Result::ErrorRequestedSurfaceWidthTooLarge;

  if (engine->window.height > engine->window.surfaceCaps.maxImageExtent.height)
    return Result::ErrorRequestedSurfaceHeightTooLarge;

  if (!(engine->window.surfaceCaps.supportedUsageFlags &
        VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
    return Result::ErrorColorAttachmentBitNotSupported;

  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          engine->device.identifier, engine->window.surface.get(), &count, 0);
      result != VK_SUCCESS)
    return Result::ErrorSurfaceFormatQueryFailure;

  engine->window.surfaceFormats.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          engine->device.identifier,
          engine->window.surface.get(),
          &count,
          engine->window.surfaceFormats.data());
      result != VK_SUCCESS)
    return Result::ErrorSurfaceFormatQueryFailure;

  if ((1 == engine->window.surfaceFormats.size()) &&
      (VK_FORMAT_UNDEFINED == engine->window.surfaceFormats[0].format)) {
    engine->window.surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    engine->window.surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  } else if (engine->window.surfaceFormats.size()) {
    engine->window.surfaceFormat = engine->window.surfaceFormats[0];
  } else
    return Result::ErrorNoSurfaceFormatsAvailable;

  VkSwapchainCreateInfoKHR swpInfo{};
  swpInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swpInfo.compositeAlpha =
      VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swpInfo.imageArrayLayers = 1;
  swpInfo.clipped = VK_TRUE;
  swpInfo.imageColorSpace = engine->window.surfaceFormat.colorSpace;
  swpInfo.imageFormat = engine->window.surfaceFormat.format;
  swpInfo.imageExtent = VkExtent2D{width, height};
  swpInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swpInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swpInfo.surface = engine->window.surface.get();
  swpInfo.presentMode = engine->window.presentMode;
  swpInfo.minImageCount = numberOfImages;
  swpInfo.preTransform =
      VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

  VkSwapchainKHR swapchain{};
  if (auto result = vkCreateSwapchainKHR(
          engine->device.handle.get(), &swpInfo, 0, &swapchain);
      result != VK_SUCCESS)
    return Result::ErrorVulkanSwapchainCreationFailure;

  auto dev = engine->device.handle.get();
  engine->window.swapchain = {swapchain, [dev](VkSwapchainKHR_T *const ptr) {
                                vkDestroySwapchainKHR(dev, ptr, 0);
                              }};

  if (auto result = vkGetSwapchainImagesKHR(engine->device.handle.get(),
                                            engine->window.swapchain.get(),
                                            &count,
                                            0);
      result != VK_SUCCESS)
    return Result::ErrorSwapchainImageQueryFailure;

  engine->window.swapImages.resize(count);

  if (auto result = vkGetSwapchainImagesKHR(engine->device.handle.get(),
                                            engine->window.swapchain.get(),
                                            &count,
                                            engine->window.swapImages.data());
      result != VK_SUCCESS)
    return Result::ErrorSwapchainImageFillFailure;

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

Result createVulkanInstance(std::string const &appName,
                            bool debug,
                            InstanceT *const instance) {
  VkInstanceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  VkApplicationInfo app{};
  app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app.pApplicationName = appName.c_str();
  app.apiVersion = VK_API_VERSION_1_3;
  info.pApplicationInfo = &app;

  info.ppEnabledExtensionNames =
      glfwGetRequiredInstanceExtensions(&info.enabledExtensionCount);

  for (uint32_t i = 0; i < info.enabledExtensionCount; ++i)
    instance->requestedExts.push_back(info.ppEnabledExtensionNames[i]);

  storeMissingInstanceExts(&instance->requestedExts, &instance->missingReqExts);

  char const *validation[] = {"VK_LAYER_KHRONOS_validation"};
  info.ppEnabledLayerNames = debug ? validation : nullptr;
  info.enabledLayerCount = debug ? 1 : 0;

  VkInstance handle{};
  auto result = vkCreateInstance(&info, nullptr, &handle);
  if (result != VK_SUCCESS) {
    instance->detailedErrorCode = result;
    return Result::ErrorVulkanInstanceCreationFailure;
  }

  instance->handle = {handle,
                      [](VkInstance ptr) { vkDestroyInstance(ptr, nullptr); }};

  return Result::Success;
}

Result selectOptimalGPU(RenderEngineT *const engine) {
  uint32_t count{};
  std::vector<VkPhysicalDevice> devs{};

  std::unordered_map<VkPhysicalDevice, std::vector<VkExtensionProperties>>
      devExtProps{};
  std::unordered_map<VkPhysicalDevice, VkPhysicalDeviceFeatures> devFeats{};
  std::unordered_map<VkPhysicalDevice, VkPhysicalDeviceProperties> devProps{};
  std::unordered_map<VkPhysicalDevice, std::vector<VkQueueFamilyProperties>>
      devQFamProps{};

  struct QueueInfo {
    uint32_t famIndex{};
    uint32_t count{};
  };

  struct QueueInfoDB {
    QueueInfo graphics{};
    QueueInfo present{};
  };

  std::unordered_map<VkPhysicalDevice, QueueInfoDB> devQFams{};
  auto const instance = engine->instance.handle.get();

  vkEnumeratePhysicalDevices(instance, &count, 0);
  if (!count)
    return Result::ErrorNoVulkanDevicesAvailable;

  devs.resize(count);
  vkEnumeratePhysicalDevices(instance, &count, devs.data());

  for (auto device : devs) {
    count = 0;
    std::vector<VkExtensionProperties> props{};
    vkEnumerateDeviceExtensionProperties(device, 0, &count, 0);
    props.resize(count);
    vkEnumerateDeviceExtensionProperties(device, 0, &count, props.data());
    devExtProps.emplace(device, std::move(props));

    VkPhysicalDeviceFeatures feats{};
    vkGetPhysicalDeviceFeatures(device, &feats);
    devFeats.emplace(device, std::move(feats));

    VkPhysicalDeviceProperties devProperties{};
    vkGetPhysicalDeviceProperties(device, &devProperties);
    devProps.emplace(device, std::move(devProperties));

    std::vector<VkQueueFamilyProperties> qFamProps{};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, 0);
    qFamProps.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, qFamProps.data());
    devQFamProps.emplace(device, std::move(qFamProps));
  }

  for (auto device : devs) {
    QueueInfo graphics{}, present{};
    bool graphicsFound = false, presentFound = false;

    for (std::size_t i = 0; i < devQFamProps.at(device).size(); ++i) {
      auto const &qp = devQFamProps.at(device)[i];
      if (!graphicsFound && qp.queueCount &&
          qp.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        graphics.count = qp.queueCount;
        graphics.famIndex = i;
        graphicsFound = true;
      }
      if (graphicsFound && presentFound)
        break;
      if (presentFound)
        continue;
      if (glfwGetPhysicalDevicePresentationSupport(instance, device, i) ==
          GLFW_TRUE) {
        present.count = qp.queueCount;
        present.famIndex = i;
        presentFound = true;
      }
    }

    if (graphicsFound && presentFound)
      devQFams.emplace(device, QueueInfoDB{graphics, present});
  }

  VkPhysicalDevice currentBest = devQFams.begin()->first;
  for (auto const &[device, db] : devQFams) {
    auto itMaxSize = devProps.at(device).limits.maxImageDimension2D;
    auto curMaxSize = devProps.at(currentBest).limits.maxImageDimension2D;
    auto const &itFeats = devFeats.at(device);
    if (itMaxSize > curMaxSize && itFeats.wideLines == VK_TRUE)
      currentBest = device;
  }

  auto const &qdb = devQFams.at(currentBest);
  VkDeviceCreateInfo devInfo{};
  devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  devInfo.queueCreateInfoCount =
      (qdb.graphics.famIndex == qdb.present.famIndex) ? 1 : 2;

  static char const *extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  devInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);
  devInfo.ppEnabledExtensionNames = extensions;

  std::vector<VkDeviceQueueCreateInfo> qCreateInfos;
  static const float prio = 1.f;
  VkDeviceQueueCreateInfo q{};
  q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  q.queueCount = 1;
  q.queueFamilyIndex = qdb.graphics.famIndex;
  q.pQueuePriorities = &prio;
  qCreateInfos.push_back(q);

  if (qdb.graphics.famIndex != qdb.present.famIndex) {
    q.queueFamilyIndex = qdb.present.famIndex;
    qCreateInfos.push_back(q);
  }

  devInfo.pQueueCreateInfos = qCreateInfos.data();
  VkPhysicalDeviceFeatures reqF{};
  reqF.wideLines = VK_TRUE;
  devInfo.pEnabledFeatures = &reqF;

  VkDevice dev{};
  if (auto result = vkCreateDevice(currentBest, &devInfo, 0, &dev);
      result != VK_SUCCESS)
    return Result::ErrorVulkanDeviceCreationFailure;

  auto &graphicsQ = engine->device.graphics;
  auto &presentQ = engine->device.presentation;
  vkGetDeviceQueue(dev, qCreateInfos.front().queueFamilyIndex, 0, &graphicsQ);
  presentQ = graphicsQ;
  if (qdb.graphics.famIndex != qdb.present.famIndex)
    vkGetDeviceQueue(dev, qCreateInfos.back().queueFamilyIndex, 0, &presentQ);

  engine->device.identifier = currentBest;
  engine->device.handle = {dev, [](VkDevice ptr) {
                             vkDeviceWaitIdle(ptr);
                             vkDestroyDevice(ptr, 0);
                           }};
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
