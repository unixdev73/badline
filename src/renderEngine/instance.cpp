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

Result storeMissingInstanceExts(std::vector<std::string> const *const requested,
                                std::vector<std::string> *const missing) {

  uint32_t availExtCount{};
  std::vector<VkExtensionProperties> avail{};

  vkEnumerateInstanceExtensionProperties(0, &availExtCount, 0);
  avail.resize(availExtCount);
  vkEnumerateInstanceExtensionProperties(0, &availExtCount, avail.data());

  std::vector<std::string> available{};
  for (auto const &e : avail)
    available.push_back(e.extensionName);

  *missing = subtract<std::string>(available, *requested);
  return Result::Success;
}
} // namespace re
