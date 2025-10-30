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
#include "private/renderEngine.hpp"
#include <iostream>
#include <string>

#define MCR_LOG(on, msg)                                                       \
  if (on)                                                                      \
    std::cerr << "Error: " << __func__ << ": " << msg << std::endl;

namespace re {
int run(RenderEngine const engine) {
  bool const dbg = engine->instance.debug;
  if (!engine->window.handle) {
    MCR_LOG(dbg, "The window is not initialized.");
    return Result::ErrorNullptrParameter;
  }

  while (!glfwWindowShouldClose(engine->window.handle.get())) {
    glfwPollEvents();

    if (glfwGetKey(engine->window.handle.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(engine->window.handle.get(), GLFW_TRUE);
  }

  return Result::Success;
}

int createWindow(RenderEngine const engine, uint32_t width, uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  bool const dbg = engine->instance.debug;

  GLFWwindow *win =
      glfwCreateWindow(width, height, engine->instance.title.c_str(), 0, 0);
  if (!win) {
    MCR_LOG(dbg, "Failed to create GLFW window.");
    return Result::ErrorVulkanWindowCreationFailure;
  }

  engine->window.handle = {win, [](GLFWwindow *const ptr) {
                             if (ptr)
                               glfwDestroyWindow(ptr);
                           }};
  engine->window.width = width;
  engine->window.height = height;

  VkSurfaceKHR surf{};
  if (auto result =
          glfwCreateWindowSurface(engine->instance.handle.get(), win, 0, &surf);
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Failed to create the window surface with error code: " +
                     std::to_string(result));
    return Result::ErrorVulkanWindowCreationFailure;
  }
  auto inst = engine->instance.handle.get();
  engine->window.surface = {surf, [inst](VkSurfaceKHR_T *const ptr) {
                              if (ptr)
                                vkDestroySurfaceKHR(inst, ptr, 0);
                            }};

  engine->window.presentModes.clear();
  uint32_t count{};
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          engine->device.identifier, engine->window.surface.get(), &count,
          nullptr);
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Failed to query present modes with code: " +
                     std::to_string(result));
    return Result::ErrorVulkanWindowCreationFailure;
  }

  engine->window.presentModes.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          engine->device.identifier, engine->window.surface.get(), &count,
          engine->window.presentModes.data());
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Failed to fill queried present modes with code: " +
                     std::to_string(result));
    return Result::ErrorVulkanWindowCreationFailure;
  }

  bool found = false;
  for (auto mode : engine->window.presentModes) {
    if (mode == engine->window.presentMode) {
      found = true;
      break;
    }
  }
  if (!found) {
    MCR_LOG(dbg, "The requested present mode is not available: " +
                     std::to_string(engine->window.presentMode));
    return Result::ErrorVulkanWindowCreationFailure;
  }

  if (auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          engine->device.identifier, engine->window.surface.get(),
          &engine->window.surfaceCaps);
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Failed to query surface capabilities with error code: " +
                     std::to_string(engine->window.presentMode));
    return Result::ErrorVulkanWindowCreationFailure;
  }

  auto numberOfImages = engine->window.surfaceCaps.minImageCount + 1;
  if ((engine->window.surfaceCaps.maxImageCount > 0) &&
      (numberOfImages > engine->window.surfaceCaps.maxImageCount)) {
    numberOfImages = engine->window.surfaceCaps.maxImageCount;
  }

  if (engine->window.width > engine->window.surfaceCaps.maxImageExtent.width) {
    MCR_LOG(dbg, "The requested width of the surface is too great: " +
                     std::to_string(engine->window.width) + " > " +
                     std::to_string(
                         engine->window.surfaceCaps.maxImageExtent.width));
    return Result::ErrorVulkanWindowCreationFailure;
  }

  if (engine->window.height >
      engine->window.surfaceCaps.maxImageExtent.height) {
    MCR_LOG(dbg, "The requested height of the surface is too great: " +
                     std::to_string(engine->window.height) + " > " +
                     std::to_string(
                         engine->window.surfaceCaps.maxImageExtent.height));
    return Result::ErrorVulkanWindowCreationFailure;
  }

  if (!(engine->window.surfaceCaps.supportedUsageFlags &
        VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
    MCR_LOG(dbg, "The surface does not support the color attachment bit.");
    return Result::ErrorVulkanWindowCreationFailure;
  }

  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          engine->device.identifier, engine->window.surface.get(), &count, 0);
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Querying the surface formats failed");
    return Result::ErrorVulkanWindowCreationFailure;
  }

  engine->window.surfaceFormats.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          engine->device.identifier, engine->window.surface.get(), &count,
          engine->window.surfaceFormats.data());
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Filling the surface formats failed");
    return Result::ErrorVulkanWindowCreationFailure;
  }

  if ((1 == engine->window.surfaceFormats.size()) &&
      (VK_FORMAT_UNDEFINED == engine->window.surfaceFormats[0].format)) {
    engine->window.surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    engine->window.surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  } else if (engine->window.surfaceFormats.size()) {
    engine->window.surfaceFormat = engine->window.surfaceFormats[0];
  } else {
    MCR_LOG(dbg, "No surface formats available.");
    return Result::ErrorVulkanWindowCreationFailure;
  }

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
  if (auto result = vkCreateSwapchainKHR(engine->device.handle.get(), &swpInfo,
                                         0, &swapchain);
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Creating the swapchain failed with error code: " +
                     std::to_string(result));
    return Result::ErrorVulkanWindowCreationFailure;
  }

  auto dev = engine->device.handle.get();
  engine->window.swapchain = {swapchain, [dev](VkSwapchainKHR_T *const ptr) {
                                vkDestroySwapchainKHR(dev, ptr, 0);
                              }};

  if (auto result =
          vkGetSwapchainImagesKHR(engine->device.handle.get(),
                                  engine->window.swapchain.get(), &count, 0);
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Failed to query swapchain images.");
    return Result::ErrorVulkanWindowCreationFailure;
  }

  engine->window.swapImages.resize(count);

  if (auto result = vkGetSwapchainImagesKHR(
          engine->device.handle.get(), engine->window.swapchain.get(), &count,
          engine->window.swapImages.data());
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Failed to retrieve swapchain images.");
    return Result::ErrorVulkanWindowCreationFailure;
  }

  return Result::Success;
}

int createRenderEngine(RenderEngine *const engine, std::string const &appName,
                       bool debug) {
  if (!engine) {
    MCR_LOG(debug, "The engine parameter is a nullptr.");
    return Result::ErrorNullptrParameter;
  }

  if (glfwInit() != GLFW_TRUE) {
    MCR_LOG(debug, "GLFW initialization failed.");
    return Result::ErrorFailedToInitGLFW;
  }

  *engine = new RenderEngineT{};
  if (!*engine) {
    MCR_LOG(debug, "Memory allocation for a render engine instance failed.");
    return Result::ErrorMemoryAllocationFailure;
  }

  VkInstance instance{};
  VkResult code{};
  if (auto err = createVulkanInstance(appName, debug, &instance, &code); err) {
    MCR_LOG(debug, "Creating Vkinstance failed, code: " + std::to_string(code));
    return err;
  }

  (*engine)->instance.handle = {
      instance, [](VkInstance ptr) { vkDestroyInstance(ptr, nullptr); }};
  (*engine)->instance.title = appName;
  (*engine)->instance.debug = debug;

  VkPhysicalDevice phy{};
  VkDevice dev{};
  VkQueue graph{}, pres{};
  if (auto err = selectOptimalGPU(instance, debug, &phy, &dev, &pres, &graph);
      err) {
    MCR_LOG(debug, "Failed to select a GPU");
    return Result::ErrorVulkanDeviceCreationFailure;
  }

  (*engine)->device.identifier = phy;
  (*engine)->device.handle = {dev, [](VkDevice ptr) {
                                vkDeviceWaitIdle(ptr);
                                vkDestroyDevice(ptr, 0);
                              }};
  (*engine)->device.graphics = graph;
  (*engine)->device.presentation = pres;

  return Result::Success;
}

void destroyRenderEngine(RenderEngine const engine) {
  delete engine;
  glfwTerminate();
}

UniqueRenderEngine createRenderEngine(std::string const &appName, bool debug) {
  RenderEngine engine{};
  createRenderEngine(&engine, appName, debug);
  return UniqueRenderEngine{engine, destroyRenderEngine};
}

int createVulkanInstance(std::string const &appName, bool debug,
                         VkInstance *handle, VkResult *code) {
  VkInstanceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  VkApplicationInfo app{};
  app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app.pApplicationName = appName.c_str();
  app.apiVersion = VK_API_VERSION_1_4;
  info.pApplicationInfo = &app;

  info.ppEnabledExtensionNames =
      glfwGetRequiredInstanceExtensions(&info.enabledExtensionCount);

  if (debug) {
    std::cout << "Listing requested Vulkan instance extensions by GLFW:\n";
    for (uint32_t i = 0; i < info.enabledExtensionCount; ++i)
      std::cout << "\t" << info.ppEnabledExtensionNames[i] << "\n";
  }

  uint32_t availableExtCount{};
  std::vector<VkExtensionProperties> exts{};
  vkEnumerateInstanceExtensionProperties(0, &availableExtCount, 0);
  exts.resize(availableExtCount);
  vkEnumerateInstanceExtensionProperties(0, &availableExtCount, exts.data());

  for (uint32_t i = 0; i < info.enabledExtensionCount; ++i) {
    std::string const req = info.ppEnabledExtensionNames[i];
    bool isAvailable = false;

    for (std::size_t j = 0; j < exts.size(); ++j) {
      std::string const available = exts[j].extensionName;
      if (req == available) {
        isAvailable = true;
        break;
      }
    }

    if (!isAvailable) {
      MCR_LOG(debug, "The requested instance extension: '" + req +
                         "' "
                         "is not available.");
    }
  }

  char const *validation[] = {"VK_LAYER_KHRONOS_validation"};
  info.ppEnabledLayerNames = debug ? validation : nullptr;
  info.enabledLayerCount = debug ? 1 : 0;

  auto result = vkCreateInstance(&info, nullptr, handle);
  if (result != VK_SUCCESS) {
    if (code)
      *code = result;
    return Result::ErrorVulkanInstanceCreationFailure;
  }

  return Result::Success;
}

int selectOptimalGPU(VkInstance const instance, bool const dbg,
                     VkPhysicalDevice *phy, VkDevice *dev, VkQueue *presentQ,
                     VkQueue *graphicsQ) {

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

  vkEnumeratePhysicalDevices(instance, &count, 0);
  if (!count) {
    MCR_LOG(dbg, "No devices that support Vulkan were found.\n");
    return Result::ErrorNoVulkanDevicesAvailable;
  }
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

  auto const &selectedName = devProps.at(currentBest).deviceName;
  if (dbg)
    std::cout << "Selected optimal GPU: " << selectedName << std::endl;

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

  if (auto result = vkCreateDevice(currentBest, &devInfo, 0, dev);
      result != VK_SUCCESS) {
    MCR_LOG(dbg, "Failed to create logical device with error: " +
                     std::to_string(result));
    return Result::ErrorVulkanDeviceCreationFailure;
  }
  *phy = currentBest;

  vkGetDeviceQueue(*dev, qCreateInfos.front().queueFamilyIndex, 0, graphicsQ);
  *presentQ = *graphicsQ;
  if (qdb.graphics.famIndex != qdb.present.famIndex)
    vkGetDeviceQueue(*dev, qCreateInfos.back().queueFamilyIndex, 0, presentQ);

  return Result::Success;
}
} // namespace re
