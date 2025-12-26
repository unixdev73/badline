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
#include "engine.hpp"
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

Result setupCreateInfo(VkDeviceCreateInfo *const devCreateInfo,
                       std::vector<VkDeviceQueueCreateInfo> *const qCreateInfos,
                       VkPhysicalDeviceFeatures *const features,
                       DeviceInfoT const *const devInfo) {
  devCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  devCreateInfo->queueCreateInfoCount =
      (devInfo->graphicsQueue.famIndex == devInfo->presentQueue.famIndex) ? 1
                                                                          : 2;

  static char const *exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  devCreateInfo->enabledExtensionCount = sizeof(exts) / sizeof(exts[0]);
  devCreateInfo->ppEnabledExtensionNames = exts;

  static const float prio = 1.f;
  VkDeviceQueueCreateInfo q{};
  q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  q.queueCount = 1;
  q.queueFamilyIndex = devInfo->graphicsQueue.famIndex;
  q.pQueuePriorities = &prio;
  qCreateInfos->push_back(q);

  if (devInfo->graphicsQueue.famIndex != devInfo->presentQueue.famIndex) {
    q.queueFamilyIndex = devInfo->presentQueue.famIndex;
    qCreateInfos->push_back(q);
  }
  devCreateInfo->pQueueCreateInfos = qCreateInfos->data();

  features->wideLines = VK_TRUE;
  return Result::Success;
}

Result createCommandPools(RenderEngineT *const engine) {
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  auto const dev = engine->device->handle.get();
  VkCommandPool pool{};

  info.queueFamilyIndex = engine->device->graphicsFamIndex;
  auto result = vkCreateCommandPool(dev, &info, 0, &pool);
  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create command pool", result);
    return Result::ErrorVulkanCommandPoolCreationFailure;
  }
  engine->device->graphicsCmdPool = {pool, [dev](VkCommandPool_T *const p) {
                                       vkDestroyCommandPool(dev, p, 0);
                                     }};

  info.queueFamilyIndex = engine->device->presentFamIndex;
  result = vkCreateCommandPool(dev, &info, 0, &pool);
  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create command pool", result);
    return Result::ErrorVulkanCommandPoolCreationFailure;
  }
  engine->device->presentCmdPool = {pool, [dev](VkCommandPool_T *const p) {
                                      vkDestroyCommandPool(dev, p, 0);
                                    }};

  return Result::Success;
}

Result allocateCommandBuffers(RenderEngineT *const engine) {
  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.commandBufferCount = 1;
  info.commandPool = engine->device->graphicsCmdPool.get();
  info.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  auto const dev = engine->device->handle.get();
  VkCommandBuffer buff{};

  auto result = vkAllocateCommandBuffers(dev, &info, &buff);
  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to allocate cmd buffer", result);
    return Result::ErrorVulkanCommandBufferAllocationFailure;
  }
  engine->device->graphicsBuff = buff;

  info.commandPool = engine->device->presentCmdPool.get();
  result = vkAllocateCommandBuffers(dev, &info, &buff);
  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to allocate cmd buffer", result);
    return Result::ErrorVulkanCommandBufferAllocationFailure;
  }
  engine->device->presentBuff = buff;

  return Result::Success;
}

Result createMemoryAllocator(RenderEngineT *const engine) {
  auto const inst = engine->instance->handle.get();
  auto const dev = engine->device->handle.get();
  auto const phy = engine->device->identifier;

  auto &alloc = engine->device->allocator;
  VkResult res{};
  alloc = createAllocator(inst, phy, dev, &res);
  if (!alloc) {
    setErrMsg(engine, "Failed to create VMA", res);
    return Result::ErrorVulkanMemoryAllocatorCreationFailure;
  }

  return Result::Success;
}

Result createDeviceResources(RenderEngineT *const engine) {
  if (auto r = createCommandPools(engine); r != Result::Success)
    return r;

  if (auto r = allocateCommandBuffers(engine); r != Result::Success)
    return r;

  if (auto r = createMemoryAllocator(engine); r != Result::Success)
    return r;

  return Result::Success;
}

Result createLogicalDevice(RenderEngineT *const engine,
                           VkDeviceCreateInfo const *const info,
                           VkPhysicalDevice const phy,
                           DeviceInfoT const *const devInfo) {
  VkDevice dev{};
  if (auto result = vkCreateDevice(phy, info, 0, &dev); result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create device", result);
    return Result::ErrorVulkanDeviceCreationFailure;
  }

  auto &graphicsQ = engine->device->graphics;
  auto &presentQ = engine->device->present;

  engine->device->graphicsFamIndex = devInfo->graphicsQueue.famIndex;
  engine->device->presentFamIndex = devInfo->presentQueue.famIndex;

  vkGetDeviceQueue(dev, devInfo->graphicsQueue.famIndex, 0, &graphicsQ);
  presentQ = graphicsQ;

  if (devInfo->graphicsQueue.famIndex != devInfo->presentQueue.famIndex)
    vkGetDeviceQueue(dev, devInfo->presentQueue.famIndex, 0, &presentQ);

  engine->device->identifier = phy;
  engine->device->handle = {dev, [](VkDevice ptr) {
                              vkDeviceWaitIdle(ptr);
                              vkDestroyDevice(ptr, 0);
                            }};

  if (auto r = createDeviceResources(engine); r != Result::Success)
    return r;
  return Result::Success;
}

Result createOptimalGPU(RenderEngineT *const engine) {
  engine->device = std::make_unique<DeviceT>();

  auto devs = queryEligibleDevices(engine->instance->handle.get());
  if (!devs.size())
    return Result::ErrorNoVulkanDevicesAvailable;

  auto const &[phy, devInfo] = *selectOptimalDevice(devs);
  std::vector<VkDeviceQueueCreateInfo> qCreateInfos;
  VkDeviceCreateInfo devCreateInfo{};
  VkPhysicalDeviceFeatures reqF{};
  devCreateInfo.pEnabledFeatures = &reqF;

  if (auto r = setupCreateInfo(&devCreateInfo, &qCreateInfos, &reqF, &devInfo);
      r != Result::Success)
    return r;

  if (auto r = createLogicalDevice(engine, &devCreateInfo, phy, &devInfo);
      r != Result::Success)
    return r;

  return Result::Success;
}
} // namespace re
