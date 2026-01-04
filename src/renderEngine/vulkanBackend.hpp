/* Copyright (c) 2026 unixdev73@gmail.com

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

#pragma once

#include "smartResource.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

struct VmaAllocator_T;
typedef struct VmaAllocator_T *VmaAllocator;

namespace re {
struct ErrorLogs;
struct Instances;

struct Vertex {
  glm::vec3 position{};
  glm::vec4 color{};

  static VkVertexInputBindingDescription bindingDescription() {
    return VkVertexInputBindingDescription{.binding = 0,
                                           .stride = sizeof(Vertex),
                                           .inputRate =
                                               VK_VERTEX_INPUT_RATE_VERTEX};
  }

  static std::array<VkVertexInputAttributeDescription, 2>
  attributeDescription() {
    return {
        VkVertexInputAttributeDescription{.location = 0,
                                          .binding = 0,
                                          .format = VK_FORMAT_R32G32B32_SFLOAT,
                                          .offset = 0},
        VkVertexInputAttributeDescription{.location = 1,
                                          .binding = 0,
                                          .format =
                                              VK_FORMAT_R32G32B32A32_SFLOAT,
                                          .offset = sizeof(position)}};
  }
};

struct InstanceTransformData {
  glm::mat4 transform{};
};

struct PushConstants {
  glm::mat4 camProj{glm::mat4(1)};
  glm::mat4 camView{glm::mat4(1)};
};

struct Instance {
  UniqueRes<VkInstance_T> handle{nullptr, nullptr};
  std::string title{};
  std::vector<std::string> missingReqExts{};
  std::vector<std::string> requestedExts{};
};

struct QueueInfo {
  uint32_t famIndex{};
  uint32_t count{};
};

struct DeviceInfo {
  std::vector<VkQueueFamilyProperties> queues{};
  std::vector<VkExtensionProperties> exts{};
  VkPhysicalDeviceFeatures feats{};
  VkPhysicalDeviceProperties props{};
  QueueInfo graphicsQueue{};
  QueueInfo presentQueue{};
};

struct Device {
  VkPhysicalDevice identifier{VK_NULL_HANDLE};
  UniqueRes<VkDevice_T> handle{nullptr, nullptr};

  VkQueue present{VK_NULL_HANDLE};
  uint32_t presentFamIndex{};
  VkQueue graphics{VK_NULL_HANDLE};
  uint32_t graphicsFamIndex{};
};

struct Window {
  ErrorLogs *logs{};

  UniqueRes<GLFWwindow> handle{};
  uint32_t width{640}, height{480};

  UniqueRes<VkSurfaceKHR_T> surface{};
  std::vector<VkSurfaceFormatKHR> surfaceFormats{};
  VkSurfaceFormatKHR surfaceFormat{};
  VkSurfaceCapabilitiesKHR surfaceCaps{};

  VkPresentModeKHR presentMode{VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR};
  std::vector<VkPresentModeKHR> presentModes{};

  UniqueRes<VkSwapchainKHR_T> swapchain{};
  std::vector<VkImage> swapImages{};
  std::vector<UniqueRes<VkImageView_T>> swapImgViews{};

  UniqueRes<VkImage_T> depthImg{};
  UniqueRes<VkImageView_T> depthImgView{};

  UniqueRes<VkSemaphore_T> acquireSem{};
  std::vector<UniqueRes<VkSemaphore_T>> renderSem{};
  VkCommandBuffer graphicsBuf{VK_NULL_HANDLE};

  UniqueRes<VkFence_T> fence{};

  std::size_t activePipelineLayout{};
  std::size_t activePipeline{};
};

struct ErrorLogs;

struct VulkanBackend {
  VulkanBackend(ErrorLogs *p) : logs{p} {}

  static constexpr auto VULKAN_API_VERSION = VK_API_VERSION_1_3;
  std::string appName{""};
  bool validation{false};

  mutable ErrorLogs *logs{};

  std::unique_ptr<Instance> instance{};
  std::unique_ptr<Device> device{};

  UniqueRes<VkCommandPool_T> graphicsCmdPool{nullptr, nullptr};
  UniqueRes<VmaAllocator_T> allocator{};
  std::vector<UniqueRes<VkPipelineLayout_T>> pipelineLayouts{};
  std::vector<UniqueRes<VkPipeline_T>> pipelines{};

  std::unique_ptr<Window> window{};

  PushConstants camMats{};

  UniqueRes<VkBuffer_T> vertexBuf{};
  UniqueRes<VkBuffer_T> indexBuf{};
  UniqueRes<VkBuffer_T> instanceBuf{};

  UniqueRes<VkDescriptorPool_T> descPool{};
  UniqueRes<VkDescriptorSetLayout_T> descLayout{};
  std::vector<VkDescriptorSet> descSets{};

  static constexpr std::size_t maxDescriptors{12};
};

bool createShaderModule(VulkanBackend *const engine,
                        std::string const &spirvFile,
                        UniqueRes<VkShaderModule_T> *const out);

bool setupRenderingInfo(VulkanBackend *const backend,
                        VkRenderingInfo *const renderingInfo,
                        VkRenderingAttachmentInfo *const color,
                        VkRenderingAttachmentInfo *const depth,
                        uint32_t const img);

bool getNextImage(VulkanBackend *const backend, uint32_t *const imgIndex);

bool setRenderBarriers(VulkanBackend *const backend, VkImage const image);

bool submitDrawCalls(VulkanBackend *const backend, uint32_t const imageIndex);

bool present(VulkanBackend *const backend, uint32_t const imageIndex);

bool queryDevices(VkInstance const instance,
                  std::vector<VkPhysicalDevice> *const out);

void queryDeviceInfo(VkPhysicalDevice_T *const handle, DeviceInfo *const info);

bool queryEligibleDevices(
    VkInstance const instance,
    std::unordered_map<VkPhysicalDevice, DeviceInfo> *const out);

bool selectOptimalDevice(
    std::unordered_map<VkPhysicalDevice, DeviceInfo> const &devs,
    std::unordered_map<VkPhysicalDevice, DeviceInfo>::const_iterator *const
        best);

void setupCreateInfo(VkDeviceCreateInfo *const devCreateInfo,
                     std::vector<VkDeviceQueueCreateInfo> *const qCreateInfos,
                     VkPhysicalDeviceFeatures *const features,
                     DeviceInfo const *const devInfo);

bool createCommandPools(VulkanBackend *const backend);

bool createAllocator(VulkanBackend const *const backend,
                     UniqueRes<VmaAllocator_T> *const allocator);

bool createDeviceResources(VulkanBackend *const backend);

bool createLogicalDevice(VulkanBackend *const backend,
                         VkDeviceCreateInfo *const info,
                         VkPhysicalDevice const phy,
                         DeviceInfo const *const devInfo);

bool createDescriptors(VulkanBackend *const backend);

bool createDepthImage(VulkanBackend *const backend,
                      uint32_t imageWidth,
                      uint32_t imageHeight,
                      UniqueRes<VkImage_T> *const out);

bool createGLFWindow(VulkanBackend *const backend,
                     uint32_t width,
                     uint32_t height);

bool createWindowSurface(VulkanBackend *const backend);

bool queryPresentModes(VulkanBackend *const backend);

bool isPresentModeAvailable(VulkanBackend *const backend);

bool checkSurfaceEligibility(VulkanBackend *const backend);

bool createWindowFence(VulkanBackend *const backend);

bool createWindowSemaphores(VulkanBackend *const backend);

bool createSwapchainImageViews(VulkanBackend *const backend);

bool transitionSwapchainImages(VulkanBackend *const backend);

bool fetchSwapchainImages(VulkanBackend *const backend);

bool createWindowSwapchain(VulkanBackend *const backend);

bool createDepthAttachment(VulkanBackend *const backend);

bool allocateCommandBuffers(VulkanBackend *const backend);

bool createPipelineLayout(VulkanBackend *const backend);

bool createGraphicsPipeline(VulkanBackend *const backend);

bool createInstance(VulkanBackend *const backend);

bool createOptimalGPU(VulkanBackend *const backend);

bool render(VulkanBackend *const backend,
            unsigned long const indexCnt,
            unsigned long const instanceCnt);

void addErrMsg(VulkanBackend const *const backend,
               std::string const &msg,
               VkResult r = VkResult::VK_SUCCESS);

void addErrMsg(VulkanBackend const *const backend,
               char const *const *const msg,
               unsigned const size);

void addErrMsg(Window const *const window,
               std::string const &msg,
               VkResult r = VkResult::VK_SUCCESS);

void addErrMsg(Window const *const window,
               char const *const *const msg,
               unsigned const size);
} // namespace re
