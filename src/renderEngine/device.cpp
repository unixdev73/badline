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

#include <unordered_map>
#include "internals.hpp"
#include "instance.hpp"
#include "device.hpp"

namespace re {
std::vector<VkPhysicalDevice> queryDevices(VkInstance instance) {
  std::vector<VkPhysicalDevice> devs{};
  uint32_t count{};

  vkEnumeratePhysicalDevices(instance, &count, 0);
  if (!count)
    return {};

  devs.resize(count);
  vkEnumeratePhysicalDevices(instance, &count, devs.data());

  return devs;
}

DeviceInfoT queryDeviceInfo(VkPhysicalDevice_T *const handle) {
  DeviceInfoT info{};
  uint32_t count{};

  vkEnumerateDeviceExtensionProperties(handle, 0, &count, 0);
  info.exts.resize(count);
  vkEnumerateDeviceExtensionProperties(handle, 0, &count, info.exts.data());

  vkGetPhysicalDeviceFeatures(handle, &info.feats);
  vkGetPhysicalDeviceProperties(handle, &info.props);

  vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, 0);
  info.queues.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, info.queues.data());
  return info;
}

std::unordered_map<VkPhysicalDevice, DeviceInfoT>
queryEligibleDevices(VkInstance const instance) {
  std::unordered_map<VkPhysicalDevice, DeviceInfoT> devInfo{};
  auto devs = queryDevices(instance);

  for (auto device : devs) {
    bool graphicsFound = false, presentFound = false;
    auto info = queryDeviceInfo(device);

    for (std::size_t i = 0; i < info.queues.size(); ++i) {
      if (graphicsFound && presentFound)
        break;

      if (!graphicsFound && info.queues[i].queueCount &&
          info.queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        graphicsFound = true;

      if (presentFound)
        continue;

      if (glfwGetPhysicalDevicePresentationSupport(instance, device, i) ==
          GLFW_TRUE)
        presentFound = true;
    }

    if (graphicsFound && presentFound)
      devInfo.emplace(device, std::move(info));
  }

  return devInfo;
}

std::unordered_map<VkPhysicalDevice, DeviceInfoT>::const_iterator
selectOptimalDevice(
    std::unordered_map<VkPhysicalDevice, DeviceInfoT> const &devs) {
  auto currentBest = devs.begin();
  for (auto it = devs.begin(); it != devs.end(); ++it) {
    auto const cMaxSize = currentBest->second.props.limits.maxImageDimension2D;
    auto const itMaxSize = it->second.props.limits.maxImageDimension2D;
    auto const &itFeats = it->second.feats;

    if (itMaxSize > cMaxSize && itFeats.wideLines == VK_TRUE)
      currentBest = it;
  }

  return currentBest;
}

Result selectOptimalGPU(RenderEngineT *const engine) {
  auto devs = queryEligibleDevices(engine->instance->handle.get());
  if (!devs.size())
    return Result::ErrorNoVulkanDevicesAvailable;

  auto const &[phy, devInfo] = *selectOptimalDevice(devs);

  VkDeviceCreateInfo devCreateInfo{};
  devCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  devCreateInfo.queueCreateInfoCount =
      (devInfo.graphicsQueue.famIndex == devInfo.presentQueue.famIndex) ? 1 : 2;

  static char const *exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  devCreateInfo.enabledExtensionCount = sizeof(exts) / sizeof(exts[0]);
  devCreateInfo.ppEnabledExtensionNames = exts;

  std::vector<VkDeviceQueueCreateInfo> qCreateInfos;
  static const float prio = 1.f;
  VkDeviceQueueCreateInfo q{};
  q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  q.queueCount = 1;
  q.queueFamilyIndex = devInfo.graphicsQueue.famIndex;
  q.pQueuePriorities = &prio;
  qCreateInfos.push_back(q);

  if (devInfo.graphicsQueue.famIndex != devInfo.presentQueue.famIndex) {
    q.queueFamilyIndex = devInfo.presentQueue.famIndex;
    qCreateInfos.push_back(q);
  }
  devCreateInfo.pQueueCreateInfos = qCreateInfos.data();

  VkPhysicalDeviceFeatures reqF{};
  reqF.wideLines = VK_TRUE;
  devCreateInfo.pEnabledFeatures = &reqF;

  VkDevice dev{};
  if (auto result = vkCreateDevice(phy, &devCreateInfo, 0, &dev);
      result != VK_SUCCESS)
    return Result::ErrorVulkanDeviceCreationFailure;

  auto &graphicsQ = engine->device->graphics;
  auto &presentQ = engine->device->presentation;
  vkGetDeviceQueue(dev, qCreateInfos.front().queueFamilyIndex, 0, &graphicsQ);
  presentQ = graphicsQ;
  if (devInfo.graphicsQueue.famIndex != devInfo.presentQueue.famIndex)
    vkGetDeviceQueue(dev, qCreateInfos.back().queueFamilyIndex, 0, &presentQ);

  engine->device->identifier = phy;
  engine->device->handle = {dev, [](VkDevice ptr) {
                              vkDeviceWaitIdle(ptr);
                              vkDestroyDevice(ptr, 0);
                            }};
  return Result::Success;
}
} // namespace re
