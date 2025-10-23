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

// PUBLIC API IMPLEMENTATION
namespace re {
int run(RenderEngine const engine) {
  if (!engine->window) {
    MCR_LOG(engine->debug, "The window is not initialized.");
    return Result::ErrorNullptrParameter;
  }

  while (!glfwWindowShouldClose(engine->window.get())) {
    glfwPollEvents();

    if (glfwGetKey(engine->window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(engine->window.get(), GLFW_TRUE);
  }

  return Result::Success;
}

int createWindow(RenderEngine const engine, uint32_t width, uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  GLFWwindow *win =
      glfwCreateWindow(width, height, engine->title.c_str(), 0, 0);
  if (!win) {
    MCR_LOG(engine->debug, "Failed to create GLFW window.");
    return Result::ErrorVulkanWindowCreationFailure;
  }

  engine->window = {win, [](GLFWwindow *const ptr) {
                      if (ptr)
                        glfwDestroyWindow(ptr);
                    }};

  VkSurfaceKHR surf{};
  if (auto result =
          glfwCreateWindowSurface(engine->instance.get(), win, 0, &surf);
      result != VK_SUCCESS) {
    MCR_LOG(engine->debug,
            "Failed to create the window surface with error code: " +
                std::to_string(result));
    return Result::ErrorVulkanWindowCreationFailure;
  }
  auto inst = engine->instance.get();
  engine->surface = {surf, [inst](VkSurfaceKHR_T *const ptr) {
                       if (ptr)
                         vkDestroySurfaceKHR(inst, ptr, 0);
                     }};

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

  (*engine)->instance = {
      instance, [](VkInstance ptr) { vkDestroyInstance(ptr, nullptr); }};
  (*engine)->title = appName;
  (*engine)->debug = debug;

  VkPhysicalDevice phy{};
  VkDevice dev{};
  VkQueue graph{}, pres{};
  if (auto err = selectOptimalGPU(instance, debug, &phy, &dev, &pres, &graph);
      err) {
    MCR_LOG(debug, "Failed to select a GPU");
    return Result::ErrorVulkanDeviceCreationFailure;
  }

  (*engine)->physicalDev = phy;
  (*engine)->device = {dev, [](VkDevice ptr) {
                         vkDeviceWaitIdle(ptr);
                         vkDestroyDevice(ptr, 0);
                       }};
  (*engine)->graphics = graph;
  (*engine)->present = pres;

  return Result::Success;
}

void destroyRenderEngine(RenderEngine const engine) {
  glfwTerminate();
  delete engine;
}

UniqueRenderEngine createRenderEngine(std::string const &appName, bool debug) {
  RenderEngine engine{};
  createRenderEngine(&engine, appName, debug);
  return UniqueRenderEngine{engine, destroyRenderEngine};
}
} // namespace re

// PRIVATE API IMPLEMENTATION
namespace re {
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
