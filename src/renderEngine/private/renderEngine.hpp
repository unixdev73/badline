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

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <string>
#include <memory>

namespace re {
using UniqueInstance =
    std::unique_ptr<VkInstance_T, void (*)(VkInstance_T *const)>;

using UniqueDevice = std::unique_ptr<VkDevice_T, void (*)(VkDevice_T *const)>;

using UniqueWindow = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *const)>;

using UniqueSurface =
    std::unique_ptr<VkSurfaceKHR_T, std::function<void(VkSurfaceKHR_T *const)>>;

using UniqueSwapchain =
    std::unique_ptr<VkSwapchainKHR_T,
                    std::function<void(VkSwapchainKHR_T *const)>>;

struct InstanceT {
  UniqueInstance handle{nullptr, nullptr};
  std::string title{};
  bool debug{false};
};

struct DeviceT {
  VkPhysicalDevice identifier{VK_NULL_HANDLE};
  UniqueDevice handle{nullptr, nullptr};
  VkQueue presentation{VK_NULL_HANDLE};
  VkQueue graphics{VK_NULL_HANDLE};
};

struct WindowT {
  UniqueWindow handle{nullptr, nullptr};
  uint32_t width{}, height{};

  UniqueSurface surface{nullptr, nullptr};
  std::vector<VkSurfaceFormatKHR> surfaceFormats{};
  VkSurfaceFormatKHR surfaceFormat{};
  VkSurfaceCapabilitiesKHR surfaceCaps{};

  VkPresentModeKHR presentMode{VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR};
  std::vector<VkPresentModeKHR> presentModes{};

  UniqueSwapchain swapchain{nullptr, nullptr};
  std::vector<VkImage> swapImages{};
};

struct RenderEngineT {
  InstanceT instance{};
  DeviceT device{};
  WindowT window{};
};

int createVulkanInstance(std::string const &appName, bool validate,
                         VkInstance *handle, VkResult *error);

int selectOptimalGPU(VkInstance const instance, bool const dbg,
                     VkPhysicalDevice *phy, VkDevice *dev, VkQueue *present,
                     VkQueue *graphics);

int createWindow(RenderEngineT *const engine, uint32_t width, uint32_t height);
} // namespace re
