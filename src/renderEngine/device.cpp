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

#include "internals.hpp"

namespace re {
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
