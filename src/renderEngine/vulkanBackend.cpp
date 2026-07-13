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

#include "smartResource.hpp"
#include "object.hpp"
#include "logs.hpp"

#include <badline/vulkanBackend.hpp>
#include <vulkan/vk_enum_string_helper.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#include "stb_image.h"

#include <filesystem>
#include <fstream>
#include <cstddef>
#include <string>
#include <vector>
#include <regex>

namespace re {
struct BackendInfo {
  static constexpr unsigned MIN_REQUIRED_VK_API_VERSION = VK_API_VERSION_1_3;
  bool enableValidationLayers{false};
  unsigned requiredVersionOfAPI{};
  std::string preferredGPU;
  std::string appName;
};

struct VulkanInstance {
  CustomUniqPtr<VkInstance_T> handle;
  unsigned activeVersionOfAPI{};
};

struct VulkanDevice {
  CustomUniqPtr<VkDevice_T> handle;
  VkPhysicalDevice physical;
  VkQueue graphics{}, presentation{};
  uint32_t presentationFamilyIndex{};
  uint32_t graphicsFamilyIndex{};
  float maxAnisotropy{};

  CustomUniqPtr<VkCommandPool_T> cmdPool;
  CustomUniqPtr<VmaAllocator_T> allocator;
  VulkanBuffer stagingBuffer;
  VulkanBuffer camProjBuffer;
};

struct VulkanFrame {
  CustomUniqPtr<VkSemaphore_T> imageAcquiredSem;
  CustomUniqPtr<VkSemaphore_T> renderDoneSem;
  CustomUniqPtr<VkFence_T> syncFence;
  VkCommandBuffer gCmdBuf;
  VkCommandBuffer pCmdBuf;
};

struct VulkanWindow {
  Logs *logs{};
  CustomUniqPtr<GLFWwindow> handle;
  CustomUniqPtr<VkSurfaceKHR_T> surface;
  CustomUniqPtr<VkSwapchainKHR_T> swapchain;
  std::vector<CustomUniqPtr<VkImageView_T>> imageViews;
  std::vector<VkImage> images;
  std::unordered_map<VkImage, VkImageLayout> imageLayouts;
  std::vector<VulkanFrame> frames;
  VkSurfaceFormatKHR surfaceFormat;
  CustomUniqPtr<VkCommandPool_T> presentationCmdPool;
  CustomUniqPtr<VkCommandPool_T> graphicsCmdPool;
  CustomUniqPtr<VkImage_T> depthImage;
  CustomUniqPtr<VkImageView_T> depthView;
  uint32_t windowWidth, windowHeight; // Cached values
  std::size_t frameIndex{};
};

struct PushConstants {
  glm::mat4 model;
  glm::mat4 view;
};

struct VulkanBackend {
  VulkanBackend(Logs *const l) : logs{l} {}
  Logs *logs{};

  std::unique_ptr<BackendInfo> generalInfo;
  std::unique_ptr<VulkanInstance> instance;
  std::unique_ptr<VulkanDevice> device;
  std::vector<std::unique_ptr<VulkanWindow>> windows;

  CustomUniqPtr<VkDescriptorPool_T> descPool;
  CustomUniqPtr<VkSampler_T> imageSampler;
  VkDescriptorSet camProjDescSet;
  CustomUniqPtr<VkDescriptorSetLayout_T> camProjDescSetLayout;
  CustomUniqPtr<VkDescriptorSetLayout_T> samplerDescSetLayout;
  CustomUniqPtr<VkPipelineLayout_T> pipelineNoSamplerLayout;
  CustomUniqPtr<VkPipelineLayout_T> pipelineSamplerLayout;
  CustomUniqPtr<VkPipeline_T> pipelineNoSampler;
  CustomUniqPtr<VkPipeline_T> pipelineSampler;

  std::vector<CustomUniqPtr<Texture>> textures;
  std::vector<CustomUniqPtr<Object>> objects;
  PushConstants constants;
  glm::mat4 camProjection;

  std::vector<ObjectInstance> objectsToRender;
  CustomUniqPtr<void> resourceUsageDoneGuard;
  float r{}, g{}, b{};
};
} // namespace re

namespace re {
bool setClearColor(VulkanBackend *const handle,
                   float const r,
                   float const g,
                   float const b) {
  if (!handle)
    return false;
  handle->r = r;
  handle->g = g;
  handle->b = b;
  return true;
}

bool createBuffer(VulkanBackend *const backend,
                  VkDeviceSize const bufferSize,
                  VkBufferUsageFlags const bufferUsage,
                  VmaAllocationCreateFlags const allocFlags,
                  CustomUniqPtr<VkBuffer_T> *const out,
                  VmaAllocation *const outAlloc);

bool createStagingBuffer(VulkanBackend *const backend,
                         VkDeviceSize const bufferSize,
                         VkBufferUsageFlags const bufferUsage,
                         CustomUniqPtr<VkBuffer_T> *const outBuf,
                         VmaAllocation *const outAlloc);

void addErrMsg(VulkanBackend const *const handle,
               std::string const &tag,
               std::string const &msg,
               VkResult const r);

bool addVertex(Object *const handle,
               glm::vec3 const &position,
               glm::vec2 const &texCoord,
               glm::vec3 const &normal,
               glm::vec4 const &color) {
  if (!handle)
    return false;

  handle->vertices.push_back(VertexData{});
  auto &data = handle->vertices.back();
  data.quad0.x = position.x;
  data.quad0.y = position.y;
  data.quad0.z = position.z;
  data.quad0.w = texCoord.x;
  data.quad1.x = texCoord.y;
  data.quad1.y = normal.x;
  data.quad1.z = normal.y;
  data.quad1.w = normal.z;
  data.quad2 = color;
  return true;
}

bool setIndices(Object *const handle, std::vector<uint32_t> indices) {
  if (!handle)
    return false;
  handle->indices = std::move(indices);
  return true;
}

bool stage(VulkanBackend *const handle,
           Object const *const object,
           glm::mat4 const &instance) {
  if (!handle)
    return false;

  if (!object) {
    addErrMsg(handle->logs, __func__, "The object handle = nullptr");
    return false;
  }

  handle->objectsToRender.push_back({object, instance});
  return true;
}

bool submitDrawCalls(VulkanBackend *const backend, VulkanWindow *const window) {

  auto const &frame = window->frames[window->frameIndex];
  auto const renderDone = frame.renderDoneSem.get();
  auto const acquireDone = frame.imageAcquiredSem.get();

  VkSubmitInfo2 submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

  submitInfo.commandBufferInfoCount = 1;
  VkCommandBufferSubmitInfo cbsi{};
  cbsi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cbsi.commandBuffer = frame.gCmdBuf;
  submitInfo.pCommandBufferInfos = &cbsi;

  submitInfo.waitSemaphoreInfoCount = 1;
  VkSemaphoreSubmitInfo wsi{};
  wsi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  wsi.semaphore = acquireDone;
  wsi.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.pWaitSemaphoreInfos = &wsi;

  submitInfo.signalSemaphoreInfoCount = 1;
  VkSemaphoreSubmitInfo ssi{};
  ssi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  ssi.semaphore = renderDone;
  ssi.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  submitInfo.pSignalSemaphoreInfos = &ssi;

  auto const fence = window->frames[window->frameIndex].syncFence.get();
  auto r = vkQueueSubmit2(backend->device->graphics, 1, &submitInfo, fence);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to submit commands", r);
    return false;
  }
  return true;
}

bool present(VulkanBackend *const backend,
             VulkanWindow *const window,
             uint32_t const imageIndex) {
  if (!backend)
    return false;

  auto const &frame = window->frames[window->frameIndex];
  auto const renderDone = frame.renderDoneSem.get();
  auto const swp = window->swapchain.get();

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderDone;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swp;
  presentInfo.pImageIndices = &imageIndex;

  auto r = vkQueuePresentKHR(backend->device->presentation, &presentInfo);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to queue presentation", r);
    return false;
  }

  return true;
}

bool setupRenderingInfo(VulkanWindow *const window,
                        VkRenderingInfo *const renderingInfo,
                        VkRenderingAttachmentInfo *const color,
                        VkRenderingAttachmentInfo *const depth,
                        uint32_t const imageIndex,
                        float const r,
                        float const g,
                        float const b) {

  *color = VkRenderingAttachmentInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = 0,
      .imageView = window->imageViews[imageIndex].get(),
      .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .resolveMode = {},
      .resolveImageView = {},
      .resolveImageLayout = {},
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.color = {.float32 = {r, g, b, 1.f}}}};

  *depth = VkRenderingAttachmentInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = 0,
      .imageView = window->depthView.get(),
      .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .resolveMode = {},
      .resolveImageView = {},
      .resolveImageLayout = {},
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.depthStencil =
                         VkClearDepthStencilValue{.depth = 1.f, .stencil = 0}}};

  renderingInfo->sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo->renderArea = {{0, 0},
                               {window->windowWidth, window->windowHeight}};
  renderingInfo->layerCount = 1;
  renderingInfo->colorAttachmentCount = 1;
  renderingInfo->pColorAttachments = color;
  renderingInfo->pDepthAttachment = depth;
  return true;
}

bool getNextImage(VulkanBackend *const backend,
                  VulkanWindow *const window,
                  uint32_t *const imgIndex) {
  if (!backend)
    return false;

  auto const &frame = window->frames[window->frameIndex];
  auto const acquireDone = frame.imageAcquiredSem.get();
  auto const swp = window->swapchain.get();
  auto const dev = backend->device->handle.get();

  uint32_t img{};
  auto r = vkAcquireNextImageKHR(dev, swp, UINT64_MAX, acquireDone, 0, &img);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Fetching swapchain image failed", r);
    return false;
  }

  *imgIndex = img;
  return true;
}

bool setRenderBarriers(VulkanBackend *const backend,
                       VulkanWindow *const window,
                       VkCommandBuffer const cmd,
                       VkImage const image) {
  if (!backend)
    return false;
  VkImageMemoryBarrier2 barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  barrier.image = image;
  barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

  barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.srcAccessMask = VK_ACCESS_2_NONE;
  barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.oldLayout = window->imageLayouts.at(image);
  barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
  window->imageLayouts.at(image) = barrier.newLayout;

  VkImageMemoryBarrier2 barriers[] = {barrier};
  depInfo.imageMemoryBarrierCount = sizeof(barriers) / sizeof(barriers[0]);
  depInfo.pImageMemoryBarriers = barriers;

  vkCmdPipelineBarrier2(cmd, &depInfo);
  return true;
}

bool setPresentBarriers(VulkanBackend *const backend,
                       VulkanWindow* const window,
                       VkCommandBuffer const cmd,
                       VkImage const image) {
  if (!backend)
    return false;
  VkImageMemoryBarrier2 barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  barrier.image = image;
  barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;

  barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.dstAccessMask = VK_ACCESS_2_NONE;
  barrier.oldLayout = window->imageLayouts.at(image);
  barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  window->imageLayouts.at(image) = barrier.newLayout;

  VkImageMemoryBarrier2 barriers[] = {barrier};
  depInfo.imageMemoryBarrierCount = sizeof(barriers) / sizeof(barriers[0]);
  depInfo.pImageMemoryBarriers = barriers;

  vkCmdPipelineBarrier2(cmd, &depInfo);
  return true;
}

bool render(VulkanBackend *const backend, VulkanWindow *const window) {
  if (!backend)
    return false;
  if (!window) {
    addErrMsg(backend->logs, __func__, "The window handle = nullptr");
    return false;
  }

  VkCommandBuffer const cmd = window->frames[window->frameIndex].gCmdBuf;
  auto const fence = window->frames[window->frameIndex].syncFence.get();
  auto const dev = backend->device->handle.get();

  vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  vkResetFences(dev, 1, &fence);

  uint32_t img{};
  if (!getNextImage(backend, window, &img)) {
    addErrMsg(backend->logs, __func__, "Failed to acquire image");
    return false;
  }

  VkCommandBufferBeginInfo cbi{};
  cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  auto r = vkBeginCommandBuffer(cmd, &cbi);
  if (r != VK_SUCCESS) {
    addErrMsg(backend->logs, __func__, "Failed to begin command buffer");
    return false;
  }

  if (!setRenderBarriers(backend, window, cmd, window->images[img])) {
    addErrMsg(backend->logs, __func__, "Failed to set memory barriers");
    return false;
  }

  VkRenderingAttachmentInfo colorAttachment{};
  VkRenderingAttachmentInfo depthAttachment{};
  VkRenderingInfo renderingInfo{};

  if (!setupRenderingInfo(window,
                          &renderingInfo,
                          &colorAttachment,
                          &depthAttachment,
                          img,
                          backend->r,
                          backend->g,
                          backend->b)) {
    addErrMsg(backend->logs, __func__, "Failed to setup rendering info");
    return false;
  }

  vkCmdBeginRendering(cmd, &renderingInfo);

  VkViewport const vp{.x = 0,
                      .y = 0,
                      .width = float(window->windowWidth),
                      .height = float(window->windowHeight),
                      .minDepth = 0.f,
                      .maxDepth = 1.f};
  vkCmdSetViewportWithCount(cmd, 1, &vp);

  VkRect2D const sc{.offset = {0, 0},
                    .extent = {window->windowWidth, window->windowHeight}};
  vkCmdSetScissorWithCount(cmd, 1, &sc);

  for (auto const &obj : backend->objectsToRender) {
    if (!obj.object->indices.size() || !obj.object->vertices.size())
      continue;

    vkCmdBindPipeline(
        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, obj.object->pipeline);

    if (obj.object->descriptorSet.get()) {
      VkDescriptorSet const descSets[] = {backend->camProjDescSet,
                                          obj.object->descriptorSet.get()};
      vkCmdBindDescriptorSets(cmd,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              obj.object->pipelineLayout,
                              0,
                              sizeof(descSets) / sizeof(descSets[0]),
                              descSets,
                              0,
                              0);
    } else {
      VkDescriptorSet const descSets[] = {backend->camProjDescSet};
      vkCmdBindDescriptorSets(cmd,
                              VK_PIPELINE_BIND_POINT_GRAPHICS,
                              obj.object->pipelineLayout,
                              0,
                              sizeof(descSets) / sizeof(descSets[0]),
                              descSets,
                              0,
                              0);
    }

    backend->constants.model = obj.instance;
    vkCmdPushConstants(cmd,
                       obj.object->pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0,
                       sizeof(PushConstants),
                       &backend->constants);

    auto const vertexBuf = obj.object->vertexBuffer.handle.get();
    VkDeviceSize const offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuf, offsets);

    auto const indexBuf = obj.object->indexBuffer.handle.get();
    vkCmdBindIndexBuffer(cmd, indexBuf, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, obj.object->indices.size(), 1, 0, 0, 0);
  }

  vkCmdEndRendering(cmd);

  if (!setPresentBarriers(backend, window, cmd, window->images[img])) {
    addErrMsg(backend->logs, __func__, "Failed to set present barriers");
    return false;
  }
  vkEndCommandBuffer(cmd);

  if (!submitDrawCalls(backend, window)) {
    addErrMsg(backend->logs, __func__, "Failed to submit draw calls");
    return false;
  }

  if (!present(backend, window, img)) {
    addErrMsg(backend->logs, __func__, "Failed to present image");
    return false;
  }

  window->frameIndex = (window->frameIndex + 1) % window->frames.size();
  backend->objectsToRender.clear();
  return true;
}

bool createShaderModule(VulkanBackend *const backend,
                        std::string const &spirvFile,
                        CustomUniqPtr<VkShaderModule_T> *const out) {
  if (!backend)
    return false;

  if (!out) {
    addErrMsg(backend->logs, __func__, "The parameter 'out' = nullptr");
    return false;
  }

  std::ifstream in{spirvFile, std::ios::ate | std::ios::binary};
  std::vector<uint32_t> byteCode{};

  if (!in.is_open()) {
    addErrMsg(backend->logs,
              __func__,
              "Failed to open shader module byte code file: " + spirvFile);
    return false;
  }

  if (!backend->device) {
    addErrMsg(backend->logs, __func__, "Device not initialized");
    return false;
  }

  auto const dev = backend->device->handle.get();
  if (!dev) {
    addErrMsg(backend->logs, __func__, "Device not initialized");
    return false;
  }

  std::size_t const dataSize = in.tellg(); // In bytes
  byteCode.resize(dataSize / sizeof(uint32_t));
  in.seekg(0);
  in.read(reinterpret_cast<char *>(byteCode.data()), dataSize);

  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = dataSize;
  info.pCode = byteCode.data();
  VkShaderModule shader{};
  auto result = vkCreateShaderModule(dev, &info, 0, &shader);

  if (result != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create shader module", result);
    return false;
  }

  *out = {shader, [dev](VkShaderModule_T *const p) {
            vkDestroyShaderModule(dev, p, 0);
          }};

  return true;
}

void getPosition(tinyobj::attrib_t const *const attrib,
                 tinyobj::index_t const *const idx,
                 VertexData *const vertex) {
  if (!attrib || !idx || !vertex)
    return;

  vertex->quad0.x = attrib->vertices[3 * size_t(idx->vertex_index) + 0];
  vertex->quad0.y = attrib->vertices[3 * size_t(idx->vertex_index) + 1];
  vertex->quad0.z = attrib->vertices[3 * size_t(idx->vertex_index) + 2];
}

void getNormal(tinyobj::attrib_t const *const attrib,
               tinyobj::index_t const *const idx,
               VertexData *const vertex) {
  if (!attrib || !idx || !vertex)
    return;

  if (idx->normal_index < 0) {
    vertex->quad1.y = 0.f;
    vertex->quad1.z = 0.f;
    vertex->quad1.w = 1.f;
    return;
  }

  vertex->quad1.y = attrib->normals[3 * size_t(idx->normal_index) + 0];
  vertex->quad1.z = attrib->normals[3 * size_t(idx->normal_index) + 1];
  vertex->quad1.w = attrib->normals[3 * size_t(idx->normal_index) + 2];
}

void getTexture(tinyobj::attrib_t const *const attrib,
                tinyobj::index_t const *const idx,
                VertexData *const vertex) {
  if (!attrib || !idx || !vertex)
    return;

  if (idx->texcoord_index < 0) {
    vertex->quad0.w = 0.f;
    vertex->quad1.x = 0.f;
    return;
  }

  vertex->quad0.w = attrib->texcoords[2 * size_t(idx->texcoord_index) + 0];
  vertex->quad1.x = attrib->texcoords[2 * size_t(idx->texcoord_index) + 1];
}

void getColor(const tinyobj::attrib_t *attrib,
              const tinyobj::index_t *idx,
              VertexData *vertex) {
  if (!attrib || !idx || !vertex)
    return;

  size_t i = 3 * size_t(idx->vertex_index);

  if (idx->vertex_index < 0 || attrib->colors.size() < i + 3) {
    vertex->quad2 = glm::vec4(1.0f); // default white
    return;
  }

  vertex->quad2 = glm::vec4(attrib->colors[i + 0],
                            attrib->colors[i + 1],
                            attrib->colors[i + 2],
                            1.0f);
}

bool prepareToLoadFromFile(Object *const handle,
                           char const *const p,
                           tinyobj::ObjReaderConfig *const reader_config,
                           tinyobj::ObjReader *const reader) {
  if (!handle || !handle->backend)
    return false;

  if (!p) {
    addErrMsg(
        handle->backend->logs, __func__, "The file path handle = nullptr");
    return false;
  }

  if (!reader_config) {
    addErrMsg(
        handle->backend->logs, __func__, "The reader config handle = nullptr");
    return false;
  }

  if (!reader) {
    addErrMsg(handle->backend->logs, __func__, "The reader handle = nullptr");
    return false;
  }

  namespace fs = std::filesystem;
  std::string const filePath{p};

  reader_config->mtl_search_path = fs::path(filePath).parent_path().string();
  reader_config->triangulate = true;

  if (!reader->ParseFromFile(filePath, *reader_config)) {
    if (!reader->Error().empty()) {
      std::string err = reader->Error();
      err.pop_back();
      addErrMsg(handle->backend->logs, __func__, "TinyObjReader: " + err);
    }

    else
      addErrMsg(
          handle->backend->logs, __func__, "Failed to load object from file");
    return false;
  }

  if (!reader->Warning().empty()) {
    std::istringstream wrn{reader->Warning()};
    std::string tmp{};
    while (std::getline(wrn, tmp))
      addWrnMsg(handle->backend->logs, __func__, "TinyObjReader: " + tmp);
  }

  return true;
}
} // namespace re

namespace re {
bool createDescriptorPoolAndLayouts(VulkanBackend *const backend) {
  auto const dev = backend->device->handle.get();

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  poolInfo.maxSets = 100;

  VkDescriptorPoolSize poolSizes[] = {
      {.type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       .descriptorCount = 1000},
      {.type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
       .descriptorCount = 1000}};

  poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
  poolInfo.pPoolSizes = poolSizes;

  VkDescriptorPool pool{};
  auto r = vkCreateDescriptorPool(dev, &poolInfo, 0, &pool);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create descriptor pool", r);
    return false;
  }

  backend->descPool = {pool, [dev](VkDescriptorPool_T *const p) {
                         vkDeviceWaitIdle(dev);
                         vkDestroyDescriptorPool(dev, p, 0);
                       }};

  VkDescriptorSetLayoutCreateInfo descInfo{};
  descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

  VkDescriptorSetLayoutBinding bindings[] = {
      {.binding = 0,
       .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       .descriptorCount = 1,
       .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
       .pImmutableSamplers = 0}};

  descInfo.bindingCount = sizeof(bindings) / sizeof(bindings[0]);
  descInfo.pBindings = bindings;

  VkDescriptorSetLayout descLayout{};
  r = vkCreateDescriptorSetLayout(dev, &descInfo, 0, &descLayout);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create proj desc set layout", r);
    return false;
  }
  backend->camProjDescSetLayout = {descLayout,
                                   [dev](VkDescriptorSetLayout_T *const p) {
                                     vkDestroyDescriptorSetLayout(dev, p, 0);
                                   }};

  bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  r = vkCreateDescriptorSetLayout(dev, &descInfo, 0, &descLayout);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create sampler desc set layout", r);
    return false;
  }
  backend->samplerDescSetLayout = {descLayout,
                                   [dev](VkDescriptorSetLayout_T *const p) {
                                     vkDestroyDescriptorSetLayout(dev, p, 0);
                                   }};

  VkDescriptorSetAllocateInfo descAlloc{};
  descAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descAlloc.descriptorPool = backend->descPool.get();
  descAlloc.descriptorSetCount = 1;
  auto const camProj = backend->camProjDescSetLayout.get();
  descAlloc.pSetLayouts = &camProj;

  VkDescriptorSet descriptor{};
  r = vkAllocateDescriptorSets(dev, &descAlloc, &descriptor);

  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to allocate descriptor set", r);
    return false;
  }
  backend->camProjDescSet = descriptor;

  VkDescriptorBufferInfo bufInf{};
  bufInf.buffer = backend->device->camProjBuffer.handle.get();
  bufInf.range = sizeof(glm::mat4);
  bufInf.offset = 0;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = backend->camProjDescSet;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &bufInf;
  descriptorWrite.dstArrayElement = 0;
  vkUpdateDescriptorSets(dev, 1, &descriptorWrite, 0, nullptr);
  return true;
}

bool createGraphicsPipeline(VulkanBackend *const backend) {
  CustomUniqPtr<VkShaderModule_T> vertex{}, fragment{}, fragNoSampler{};
  if (!createShaderModule(backend, "shaders/vertex.spv", &vertex)) {
    addErrMsg(backend->logs, __func__, "Failed to create vertex shader");
    return false;
  }

  if (!createShaderModule(backend, "shaders/fragment.spv", &fragment)) {
    addErrMsg(backend->logs, __func__, "Failed to create fragment shader");
    return false;
  }

  if (!createShaderModule(
          backend, "shaders/fragNoSampler.spv", &fragNoSampler)) {
    addErrMsg(
        backend->logs, __func__, "Failed to create fragment no sampler shader");
    return false;
  }

  VkPipelineShaderStageCreateInfo stages[] = {
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext = 0,
       .flags = 0,
       .stage = VK_SHADER_STAGE_VERTEX_BIT,
       .module = vertex.get(),
       .pName = "main",
       .pSpecializationInfo = 0},
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .pNext = 0,
       .flags = 0,
       .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = fragNoSampler.get(),
       .pName = "main",
       .pSpecializationInfo = 0}};

  VkPipelineVertexInputStateCreateInfo vertexInputState{};
  vertexInputState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  VkVertexInputAttributeDescription const attribDesc[] = {
      {.location = 0,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32A32_SFLOAT,
       .offset = offsetof(VertexData, quad0)},
      {.location = 1,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32A32_SFLOAT,
       .offset = offsetof(VertexData, quad1)},
      {.location = 2,
       .binding = 0,
       .format = VK_FORMAT_R32G32B32A32_SFLOAT,
       .offset = offsetof(VertexData, quad2)}};
  vertexInputState.pVertexAttributeDescriptions = attribDesc;
  vertexInputState.vertexAttributeDescriptionCount =
      sizeof(attribDesc) / sizeof(attribDesc[0]);

  VkVertexInputBindingDescription const bindDesc[] = {
      {.binding = 0,
       .stride = sizeof(VertexData),
       .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}};
  vertexInputState.pVertexBindingDescriptions = bindDesc;
  vertexInputState.vertexBindingDescriptionCount =
      sizeof(bindDesc) / sizeof(bindDesc[0]);

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
  inputAssemblyState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyState.primitiveRestartEnable = VK_FALSE;
  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineRasterizationStateCreateInfo rasterizationState{};
  rasterizationState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationState.depthClampEnable = VK_FALSE;
  rasterizationState.rasterizerDiscardEnable = VK_FALSE;
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.lineWidth = 1.0f;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampleState{};
  multisampleState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDepthStencilStateCreateInfo depthStencilState{};
  depthStencilState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilState.depthTestEnable = VK_TRUE;
  depthStencilState.depthWriteEnable = VK_TRUE;
  depthStencilState.minDepthBounds = 0.f;
  depthStencilState.maxDepthBounds = 1.f;
  depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.blendEnable = VK_TRUE;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colorBlendState{};
  colorBlendState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendState.logicOpEnable = VK_FALSE;
  colorBlendState.logicOp = VK_LOGIC_OP_COPY;
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments = &colorBlendAttachment;
  colorBlendState.blendConstants[0] = 0.0f;
  colorBlendState.blendConstants[1] = 0.0f;
  colorBlendState.blendConstants[2] = 0.0f;
  colorBlendState.blendConstants[3] = 0.0f;

  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  VkDynamicState dynamicStates[] = {
      VkDynamicState::VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
      VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT};
  dynamicState.dynamicStateCount =
      sizeof(dynamicStates) / sizeof(dynamicStates[0]);
  dynamicState.pDynamicStates = dynamicStates;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

  VkPipelineRenderingCreateInfo nextInfo{};
  nextInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  nextInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

  VkFormat formats[] = {VK_FORMAT_B8G8R8A8_UNORM};
  nextInfo.pColorAttachmentFormats = formats;
  nextInfo.colorAttachmentCount = 1;

  VkGraphicsPipelineCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  info.pNext = &nextInfo;
  info.stageCount = sizeof(stages) / sizeof(stages[0]);
  info.pStages = stages;
  info.pVertexInputState = &vertexInputState;
  info.pInputAssemblyState = &inputAssemblyState;
  info.pViewportState = &viewportState;
  info.pRasterizationState = &rasterizationState;
  info.pMultisampleState = &multisampleState;
  info.pDepthStencilState = &depthStencilState;
  info.pColorBlendState = &colorBlendState;
  info.pDynamicState = &dynamicState;
  info.basePipelineHandle = VK_NULL_HANDLE;
  info.basePipelineIndex = 0;

  auto const dev = backend->device->handle.get();
  VkPipeline handle{VK_NULL_HANDLE};

  info.layout = backend->pipelineNoSamplerLayout.get();
  auto r = vkCreateGraphicsPipelines(dev, 0, 1, &info, 0, &handle);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create graphics pipeline", r);
    return false;
  }
  backend->pipelineNoSampler = {
      handle, [dev](VkPipeline_T *const p) { vkDestroyPipeline(dev, p, 0); }};

  info.layout = backend->pipelineSamplerLayout.get();
  stages[1].module = fragment.get();
  r = vkCreateGraphicsPipelines(dev, 0, 1, &info, 0, &handle);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create graphics pipeline", r);
    return false;
  }

  backend->pipelineSampler = {
      handle, [dev](VkPipeline_T *const p) { vkDestroyPipeline(dev, p, 0); }};
  return true;
}

bool createPipelineLayouts(VulkanBackend *const backend) {
  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  VkPushConstantRange ranges[] = {
      VkPushConstantRange{.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                          .offset = 0,
                          .size = sizeof(backend->constants)}};
  info.pushConstantRangeCount = sizeof(ranges) / sizeof(ranges[0]);
  info.pPushConstantRanges = ranges;

  auto const dev = backend->device->handle.get();
  VkPipelineLayout handle{};

  { // This is the simple, no sampler variant
    VkDescriptorSetLayout layouts[] = {backend->camProjDescSetLayout.get()};
    info.pSetLayouts = layouts;
    info.setLayoutCount = sizeof(layouts) / sizeof(layouts[0]);
    auto result = vkCreatePipelineLayout(dev, &info, 0, &handle);

    if (result != VK_SUCCESS) {
      addErrMsg(backend, __func__, "Failed to create pipeline layout", result);
      return false;
    }

    backend->pipelineNoSamplerLayout = {handle,
                                        [dev](VkPipelineLayout_T *const p) {
                                          vkDestroyPipelineLayout(dev, p, 0);
                                        }};
  }

  { // This is the sampler variant
    VkDescriptorSetLayout layouts[] = {backend->camProjDescSetLayout.get(),
                                       backend->samplerDescSetLayout.get()};
    info.pSetLayouts = layouts;
    info.setLayoutCount = sizeof(layouts) / sizeof(layouts[0]);
    auto result = vkCreatePipelineLayout(dev, &info, 0, &handle);

    if (result != VK_SUCCESS) {
      addErrMsg(backend, __func__, "Failed to create pipeline layout", result);
      return false;
    }

    backend->pipelineSamplerLayout = {handle,
                                      [dev](VkPipelineLayout_T *const p) {
                                        vkDestroyPipelineLayout(dev, p, 0);
                                      }};
  }
  return true;
}

bool createTexture(VulkanBackend *const handle, Texture **const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle->logs, __func__, "The texture handle = nullptr");
    return false;
  }

  handle->textures.push_back(std::make_unique<Texture>(handle));
  *p = handle->textures.back().get();
  return true;
}

bool createImage2D(VulkanBackend *const backend,
                   uint32_t const imageWidth,
                   uint32_t const imageHeight,
                   VkFormat const imageFormat,
                   VkImageUsageFlags const imageUsage,
                   CustomUniqPtr<VkImage_T> *const out);

bool transitionImageLayout(VulkanBackend *const handle,
                           VkImage const dst,
                           VkImageAspectFlags const aspect,
                           VkPipelineStageFlags2 const srcStage,
                           VkPipelineStageFlags2 const dstStage,
                           VkAccessFlags2 const srcFlags,
                           VkAccessFlags2 const dstFlags,
                           VkImageLayout const oldLayout,
                           VkImageLayout const newLayout) {
  if (!handle)
    return false;
  if (!dst) {
    addErrMsg(handle->logs, __func__, "The destination buffer is not valid");
    return false;
  }

  auto const dev = handle->device->handle.get();
  CustomUniqPtr<VkFence_T> tmpFence{};
  VkFence fence{};

  VkFenceCreateInfo finf{};
  finf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if (auto r = vkCreateFence(dev, &finf, 0, &fence); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to create fence");
    return false;
  }
  tmpFence = {fence, [dev](auto *const p) { vkDestroyFence(dev, p, 0); }};

  CustomUniqPtr<VkCommandPool_T> tmpPool{};
  VkCommandPoolCreateInfo pinf{};
  VkCommandPool pool{};
  pinf.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

  if (auto r = vkCreateCommandPool(dev, &pinf, 0, &pool); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to create command pool");
    return false;
  }
  tmpPool = {pool, [dev](auto *const p) { vkDestroyCommandPool(dev, p, 0); }};

  VkCommandBuffer cmd{};
  VkCommandBufferAllocateInfo cbinf{};
  cbinf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cbinf.commandBufferCount = 1;
  cbinf.commandPool = pool;
  cbinf.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  if (auto r = vkAllocateCommandBuffers(dev, &cbinf, &cmd); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to allocate command buffer");
    return false;
  }

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &beginInfo);

  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  depInfo.imageMemoryBarrierCount = 1;
  VkImageMemoryBarrier2 imb{};
  imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imb.image = dst;
  imb.srcStageMask = srcStage;
  imb.srcAccessMask = srcFlags;
  imb.dstStageMask = dstStage;
  imb.dstAccessMask = dstFlags;
  imb.oldLayout = oldLayout;
  imb.newLayout = newLayout;
  imb.subresourceRange.aspectMask = aspect;
  imb.subresourceRange.layerCount = 1;
  imb.subresourceRange.levelCount = 1;
  depInfo.pImageMemoryBarriers = &imb;
  vkCmdPipelineBarrier2(cmd, &depInfo);
  vkEndCommandBuffer(cmd);

  VkSubmitInfo sinf{};
  sinf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  sinf.commandBufferCount = 1;
  sinf.pCommandBuffers = &cmd;
  vkQueueSubmit(handle->device->graphics, 1, &sinf, fence);
  vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  return true;
}

bool copyBufferToImage(VulkanBackend *const handle,
                       void const *src,
                       VkDeviceSize const size,
                       VkImage const dst,
                       uint32_t const width,
                       uint32_t const height,
                       uint32_t const bytesPerPixel) {
  if (!handle)
    return false;
  if (!size) {
    return true;
  }
  if (!dst) {
    addErrMsg(handle->logs, __func__, "The destination buffer is not valid");
    return false;
  }
  if (!src) {
    addErrMsg(handle->logs, __func__, "The data = nullptr");
    return false;
  }

  auto const dev = handle->device->handle.get();
  CustomUniqPtr<VkFence_T> tmpFence{};
  VkFence fence{};

  VkFenceCreateInfo finf{};
  finf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if (auto r = vkCreateFence(dev, &finf, 0, &fence); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to create fence");
    return false;
  }
  tmpFence = {fence, [dev](auto *const p) { vkDestroyFence(dev, p, 0); }};

  CustomUniqPtr<VkCommandPool_T> tmpPool{};
  VkCommandPoolCreateInfo pinf{};
  VkCommandPool pool{};
  pinf.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pinf.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (auto r = vkCreateCommandPool(dev, &pinf, 0, &pool); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to create command pool");
    return false;
  }
  tmpPool = {pool, [dev](auto *const p) { vkDestroyCommandPool(dev, p, 0); }};

  VkCommandBuffer cmd{};
  VkCommandBufferAllocateInfo cbinf{};
  cbinf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cbinf.commandBufferCount = 1;
  cbinf.commandPool = pool;
  cbinf.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  if (auto r = vkAllocateCommandBuffers(dev, &cbinf, &cmd); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to allocate command buffer");
    return false;
  }

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &beginInfo);

  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  depInfo.imageMemoryBarrierCount = 1;
  VkImageMemoryBarrier2 imb{};
  imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  imb.image = dst;
  imb.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  imb.srcAccessMask = VK_ACCESS_2_NONE;
  imb.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  imb.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imb.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  imb.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  imb.subresourceRange.layerCount = 1;
  imb.subresourceRange.levelCount = 1;
  depInfo.pImageMemoryBarriers = &imb;
  vkCmdPipelineBarrier2(cmd, &depInfo);

  VkSubmitInfo sinf{};
  sinf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  sinf.commandBufferCount = 1;
  sinf.pCommandBuffers = &cmd;

  vkEndCommandBuffer(cmd);
  vkQueueSubmit(handle->device->graphics, 1, &sinf, fence);
  vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  vkResetFences(dev, 1, &fence);

  auto const stagingData = handle->device->stagingBuffer.allocInfo.pMappedData;
  auto const stagingSize = handle->device->stagingBuffer.allocInfo.size;
  auto const stagingBuf = handle->device->stagingBuffer.handle.get();
  VkBufferImageCopy r{};
  r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  r.imageSubresource.layerCount = 1;
  r.imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0};

  auto const rowSizeInBytes = width * bytesPerPixel;
  if (rowSizeInBytes > stagingSize) {
    addErrMsg(handle->logs,
              __func__,
              "The image size is too great to fit in the staging buffer with "
              "the current algo");
    return false;
  }

  if (stagingSize >= size) {
    std::memcpy(stagingData, src, size);
    r.imageExtent = {width, height, 1};

    vkBeginCommandBuffer(cmd, &beginInfo);
    vkCmdCopyBufferToImage(
        cmd, stagingBuf, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &r);
    vkEndCommandBuffer(cmd);
    vkQueueSubmit(handle->device->graphics, 1, &sinf, fence);
    vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(dev, 1, &fence);
    return true;
  }

  r.imageExtent = {width, 1, 1};
  for (std::size_t i = 0; i < height; ++i) {
    auto const bufByteOffset = i * rowSizeInBytes;
    std::memcpy(stagingData, (char const *)src + bufByteOffset, rowSizeInBytes);
    r.imageOffset.y = i;

    vkBeginCommandBuffer(cmd, &beginInfo);
    vkCmdCopyBufferToImage(
        cmd, stagingBuf, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &r);
    vkEndCommandBuffer(cmd);
    vkQueueSubmit(handle->device->graphics, 1, &sinf, fence);
    vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(dev, 1, &fence);
  }

  return true;
}

bool createImageSampler(VulkanBackend *const handle) {
  if (!handle)
    return false;

  VkSampler sampler;
  VkSamplerCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  info.magFilter = VkFilter::VK_FILTER_LINEAR;
  info.minFilter = VkFilter::VK_FILTER_LINEAR;
  info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  info.anisotropyEnable = VK_TRUE;
  info.maxAnisotropy = handle->device->maxAnisotropy;

  auto const dev = handle->device->handle.get();
  if (auto r = vkCreateSampler(dev, &info, 0, &sampler); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to create sampler");
    return false;
  }
  handle->imageSampler = {
      sampler, [dev](auto *const p) { vkDestroySampler(dev, p, 0); }};

  return true;
}

bool loadFromFile(Texture *const handle, std::string const &p) {
  if (!handle)
    return false;

  if (!handle->backend->device) {
    addErrMsg(handle->backend->logs, __func__, "The device is not initialized");
    return false;
  }

  auto const sbuf = handle->backend->device->stagingBuffer.handle.get();
  if (!sbuf) {
    addErrMsg(handle->backend->logs,
              __func__,
              "The staging buffer is not initialized");
    return false;
  }

  if (!std::filesystem::exists(p)) {
    addErrMsg(handle->backend->logs,
              __func__,
              "The file: " + std::string{p} + " does not exist");
    return false;
  }

  stbi_set_flip_vertically_on_load(true);
  int x, y, ch;
  void *image = stbi_load(p.c_str(), &x, &y, &ch, STBI_rgb_alpha);
  if (!image) {
    addErrMsg(handle->backend->logs,
              __func__,
              "Loading img failed: " + std::string{p});
    return false;
  }
  handle->stbImagePtr = {image, [](void *const ptr) { stbi_image_free(ptr); }};

  auto constexpr channels = uint32_t{4};
  VkDeviceSize const imgSize = x * y * channels;
  if (!createImage2D(handle->backend,
                     x,
                     y,
                     VK_FORMAT_R8G8B8A8_SRGB,
                     VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                         VK_IMAGE_USAGE_SAMPLED_BIT,
                     &handle->image)) {
    addErrMsg(
        handle->backend->logs, __func__, "Failed to create texture image");
    return false;
  }

  if (!copyBufferToImage(handle->backend,
                         image,
                         imgSize,
                         handle->image.get(),
                         x,
                         y,
                         channels)) {
    addErrMsg(
        handle->backend->logs, __func__, "Failed to copy texture image to gpu");
    return false;
  }

  if (!transitionImageLayout(handle->backend,
                             handle->image.get(),
                             VK_IMAGE_ASPECT_COLOR_BIT,
                             VK_PIPELINE_STAGE_2_NONE,
                             VK_PIPELINE_STAGE_2_NONE,
                             VK_ACCESS_2_NONE,
                             VK_ACCESS_2_NONE,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)) {
    addErrMsg(handle->backend->logs,
              __func__,
              "Failed to transition image to shader readable layout");
    return false;
  }

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = handle->image.get();
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  auto const dev = handle->backend->device->handle.get();
  VkImageView view{};
  if (auto r = vkCreateImageView(dev, &viewInfo, 0, &view); r != VK_SUCCESS) {
    addErrMsg(handle->backend->logs, __func__, "Failed to create image view");
    return false;
  }
  handle->view = {view,
                  [dev](auto *const p) { vkDestroyImageView(dev, p, 0); }};

  addInfMsg(handle->backend->logs, __func__, "Success: " + std::string{p});
  return true;
}

bool copyDataToBuffer(VulkanBackend *const handle,
                      void const *const data,
                      VkDeviceSize const size,
                      VkBuffer const dst) {
  if (!handle)
    return false;
  if (!data || !size) {
    addErrMsg(handle->logs, __func__, "The data is not valid");
    return false;
  }
  if (!dst) {
    addErrMsg(handle->logs, __func__, "The destination buffer is not valid");
    return false;
  }
  if (!handle->device->stagingBuffer.handle) {
    addErrMsg(handle->logs, __func__, "The staging buffer is not valid");
    return false;
  }

  auto const dev = handle->device->handle.get();
  CustomUniqPtr<VkFence_T> tmpFence{};
  VkFence fence{};

  VkFenceCreateInfo finf{};
  finf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if (auto r = vkCreateFence(dev, &finf, 0, &fence); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to create fence");
    return false;
  }
  tmpFence = {fence, [dev](auto *const p) { vkDestroyFence(dev, p, 0); }};

  CustomUniqPtr<VkCommandPool_T> tmpPool{};
  VkCommandPoolCreateInfo pinf{};
  VkCommandPool pool{};
  pinf.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pinf.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  if (auto r = vkCreateCommandPool(dev, &pinf, 0, &pool); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to create command pool");
    return false;
  }
  tmpPool = {pool, [dev](auto *const p) { vkDestroyCommandPool(dev, p, 0); }};

  VkCommandBuffer cmd{};
  VkCommandBufferAllocateInfo cbinf{};
  cbinf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cbinf.commandBufferCount = 1;
  cbinf.commandPool = pool;
  cbinf.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  if (auto r = vkAllocateCommandBuffers(dev, &cbinf, &cmd); r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to allocate command buffer");
    return false;
  }

  auto const stagingData = handle->device->stagingBuffer.allocInfo.pMappedData;
  auto const stagingSize = handle->device->stagingBuffer.allocInfo.size;
  auto const stagingBuf = handle->device->stagingBuffer.handle.get();
  VkBufferCopy r{.srcOffset = 0, .dstOffset = 0, .size = size};
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VkSubmitInfo sinf{};
  sinf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  sinf.commandBufferCount = 1;
  sinf.pCommandBuffers = &cmd;

  std::size_t const cycleCount = size / stagingSize;
  std::size_t const remainder = size % stagingSize;

  for (std::size_t i = 0; i < cycleCount; ++i) {
    void const *srcOffset = (char const *)data + i * stagingSize;
    std::memcpy(stagingData, srcOffset, stagingSize);

    vkBeginCommandBuffer(cmd, &beginInfo);
    r.srcOffset = 0;
    r.dstOffset = i * stagingSize;
    r.size = stagingSize;
    vkCmdCopyBuffer(cmd, stagingBuf, dst, 1, &r);
    vkEndCommandBuffer(cmd);

    vkQueueSubmit(handle->device->graphics, 1, &sinf, fence);
    vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
    vkResetFences(dev, 1, &fence);
  }

  std::memcpy(
      stagingData, (char const *)data + cycleCount * stagingSize, remainder);

  vkBeginCommandBuffer(cmd, &beginInfo);
  r.srcOffset = 0;
  r.dstOffset = cycleCount * stagingSize;
  r.size = remainder;
  vkCmdCopyBuffer(cmd, stagingBuf, dst, 1, &r);
  vkEndCommandBuffer(cmd);

  vkQueueSubmit(handle->device->graphics, 1, &sinf, fence);
  vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  vkResetFences(dev, 1, &fence);
  return true;
}

bool uploadObjectDataToGPU(Object *const handle) {
  if (!handle)
    return false;

  auto sz = [](auto const &cont) { return cont.size() * sizeof(cont[0]); };
  auto &vb = handle->vertexBuffer;
  auto &vi = handle->indexBuffer;
  auto vk = handle->backend;
  VmaAllocation alloc;

  vkQueueWaitIdle(handle->backend->device->graphics);

  if (!createBuffer(vk,
                    sz(handle->vertices),
                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                    &vb.handle,
                    &alloc)) {
    addErrMsg(vk->logs, __func__, "Failed to create vertex buffer");
    return false;
  }
  vmaGetAllocationInfo(vk->device->allocator.get(), alloc, &vb.allocInfo);

  if (!createBuffer(vk,
                    sz(handle->indices),
                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
                    &vi.handle,
                    &alloc)) {
    addErrMsg(vk->logs, __func__, "Failed to create index buffer");
    return false;
  }
  vmaGetAllocationInfo(vk->device->allocator.get(), alloc, &vi.allocInfo);

  if (!copyDataToBuffer(
          vk, handle->vertices.data(), sz(handle->vertices), vb.handle.get())) {
    addErrMsg(vk->logs, __func__, "Failed to copy data to vertex buffer");
    return false;
  }

  if (!copyDataToBuffer(
          vk, handle->indices.data(), sz(handle->indices), vi.handle.get())) {
    addErrMsg(vk->logs, __func__, "Failed to copy data to index buffer");
    return false;
  }

  return true;
}

bool loadFromRAM(Object *const handle,
                 std::vector<VertexData> vertices,
                 std::vector<uint32_t> indices) {
  if (!handle)
    return false;
  if (!handle->backend)
    return false;

  handle->vertices = std::move(vertices);
  handle->indices = std::move(indices);

  if (!uploadObjectDataToGPU(handle)) {
    addErrMsg(
        handle->backend->logs, __func__, "Failed to upload object data to GPU");
    return false;
  }

  return true;
}

bool loadFromFile(Object *const handle, std::string const &p) {
  tinyobj::ObjReaderConfig reader_config;
  tinyobj::ObjReader reader;

  if (!prepareToLoadFromFile(handle, p.c_str(), &reader_config, &reader))
    return false;

  std::unordered_map<VertexData, unsigned> vertIndex{};
  auto &attrib = reader.GetAttrib();
  auto &shapes = reader.GetShapes();

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      for (size_t v = 0; v < fv; v++) {
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        VertexData vertex{};
        getPosition(&attrib, &idx, &vertex);
        getNormal(&attrib, &idx, &vertex);
        getTexture(&attrib, &idx, &vertex);
        getColor(&attrib, &idx, &vertex);

        if (!vertIndex.contains(vertex)) {
          vertIndex.emplace(vertex, handle->vertices.size());
          handle->vertices.push_back(vertex);
        }

        handle->indices.push_back(vertIndex.at(vertex));
      }

      index_offset += fv;
    }
  }

  if (!uploadObjectDataToGPU(handle)) {
    addErrMsg(
        handle->backend->logs, __func__, "Failed to upload object data to GPU");
    return false;
  }

  std::string summary = "Success: " + std::string{p};
  summary += ", indices: " + std::to_string(handle->indices.size());
  summary += ", vertices: " + std::to_string(handle->vertices.size());
  addInfMsg(handle->backend->logs, __func__, summary);
  return true;
}

bool createObject(VulkanBackend *const handle,
                  Texture const *const t,
                  Object **const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle->logs, __func__, "The object handle = nullptr");
    return false;
  }

  handle->objects.push_back(std::make_unique<Object>(handle, t));
  if (!handle->objects.back()) {
    addErrMsg(handle->logs, __func__, "Failed to allocate memory for object");
    return false;
  }

  auto &obj = handle->objects.back();

  if (!t) {
    obj->pipelineLayout = handle->pipelineNoSamplerLayout.get();
    obj->pipeline = handle->pipelineNoSampler.get();
  } else {
    obj->pipelineLayout = handle->pipelineSamplerLayout.get();
    obj->pipeline = handle->pipelineSampler.get();

    VkDescriptorSetAllocateInfo descAlloc{};
    descAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    auto const pool = obj->backend->descPool.get();
    descAlloc.descriptorPool = pool;
    descAlloc.descriptorSetCount = 1;
    auto const camProj = obj->backend->samplerDescSetLayout.get();
    descAlloc.pSetLayouts = &camProj;

    auto const dev = obj->backend->device->handle.get();
    VkDescriptorSet descriptor{};
    auto r = vkAllocateDescriptorSets(dev, &descAlloc, &descriptor);
    if (r != VK_SUCCESS) {
      addErrMsg(obj->backend,
                __func__,
                "Failed to allocate sampler descriptor set",
                r);
      return false;
    }
    obj->descriptorSet = {descriptor, [dev, pool](auto *const p) {
                            vkFreeDescriptorSets(dev, pool, 1, &p);
                          }};

    VkDescriptorImageInfo imgInf{};
    imgInf.sampler = obj->backend->imageSampler.get();
    imgInf.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    if (!obj->texture->view) {
      addErrMsg(
          obj->backend->logs, __func__, "The texture view is not initialized");
      return false;
    }
    imgInf.imageView = obj->texture->view.get();

    VkWriteDescriptorSet descriptorWrite = {};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = obj->descriptorSet.get();
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imgInf;
    vkUpdateDescriptorSets(dev, 1, &descriptorWrite, 0, nullptr);
  }

  *p = obj.get();
  return true;
}

bool getWindowHandle(VulkanWindow const *const vkWin,
                     GLFWwindow **const glfwWin) {
  if (!vkWin)
    return false;
  if (!glfwWin) {
    addErrMsg(vkWin->logs, __func__, "The window handle = nullptr");
    return false;
  }
  *glfwWin = vkWin->handle.get();
  return true;
}

bool setCameraProjection(VulkanBackend *const handle, glm::mat4 const &m) {
  if (!handle)
    return false;
  if (!handle->device) {
    addErrMsg(handle->logs, __func__, "The device is not initialized");
    return false;
  }
  if (!handle->device->camProjBuffer.allocInfo.pMappedData) {
    addErrMsg(handle->logs, __func__, "The projection buffer is not mapped");
    return false;
  }

  std::memcpy(
      handle->device->camProjBuffer.allocInfo.pMappedData, &m, sizeof(m));
  return true;
}

bool setCameraView(VulkanBackend *const handle, glm::mat4 const &m) {
  if (!handle)
    return false;
  handle->constants.view = m;
  return true;
}

bool createWindowSwapchain(VulkanBackend *const backend,
                           VulkanWindow *const window) {
  if (!backend)
    return false;

  if (!backend->device) {
    addErrMsg(backend->logs, __func__, "The backend device = nullptr");
    return false;
  }

  VkPhysicalDevice phy = backend->device->physical;
  VkSurfaceCapabilitiesKHR surfaceCaps;
  auto const surface = window->surface.get();

  if (auto r =
          vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phy, surface, &surfaceCaps);
      r != VK_SUCCESS) {
    addErrMsg(backend->logs, __func__, "Failed to query surface capabilities");
    return false;
  }

  window->surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
  window->surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  auto numberOfImages = surfaceCaps.minImageCount + 1;
  if ((surfaceCaps.maxImageCount > 0) &&
      (numberOfImages > surfaceCaps.maxImageCount)) {
    numberOfImages = surfaceCaps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swpInfo{};
  swpInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swpInfo.compositeAlpha =
      VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swpInfo.imageArrayLayers = 1;
  swpInfo.clipped = VK_TRUE;
  swpInfo.imageColorSpace = window->surfaceFormat.colorSpace;
  swpInfo.imageFormat = window->surfaceFormat.format;

  int width, height;
  glfwGetWindowSize(window->handle.get(), &width, &height);

  swpInfo.imageExtent = VkExtent2D{(uint32_t)width, (uint32_t)height};
  swpInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swpInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swpInfo.surface = window->surface.get();
  swpInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  swpInfo.minImageCount = numberOfImages;
  swpInfo.preTransform =
      VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

  VkSwapchainKHR swapchain{};
  if (auto result = vkCreateSwapchainKHR(
          backend->device->handle.get(), &swpInfo, 0, &swapchain);
      result != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create window swapchain", result);
    return false;
  }

  auto dev = backend->device->handle.get();
  window->swapchain = {swapchain, [dev](VkSwapchainKHR_T *const ptr) {
                         vkDestroySwapchainKHR(dev, ptr, 0);
                       }};

  return true;
}

bool createCommandPool(VulkanBackend *const backend,
                       uint32_t const queueFamilyIndex,
                       CustomUniqPtr<VkCommandPool_T> *const cmdPool) {
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  auto const dev = backend->device->handle.get();
  VkCommandPool pool{};

  info.queueFamilyIndex = queueFamilyIndex;
  auto result = vkCreateCommandPool(dev, &info, 0, &pool);
  if (result != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create command pool", result);
    return false;
  }

  *cmdPool = {pool, [dev](VkCommandPool_T *const p) {
                vkDestroyCommandPool(dev, p, 0);
              }};
  return true;
}

bool createFrameResources(VulkanBackend *const backend,
                          VulkanWindow *const window) {
  auto const dev = backend->device->handle.get();
  window->frames.resize(window->images.size());

  for (auto &frame : window->frames) {
    VkCommandBufferAllocateInfo cmdAlloc{};
    cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAlloc.commandBufferCount = 1;
    cmdAlloc.commandPool = window->graphicsCmdPool.get();
    cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    if (auto r = vkAllocateCommandBuffers(dev, &cmdAlloc, &frame.gCmdBuf);
        r != VK_SUCCESS) {
      addErrMsg(backend->logs, __func__, "Failed to allocate graphics buffer");
      return false;
    }

    cmdAlloc.commandPool = window->presentationCmdPool.get();
    if (auto r = vkAllocateCommandBuffers(dev, &cmdAlloc, &frame.pCmdBuf);
        r != VK_SUCCESS) {
      addErrMsg(
          backend->logs, __func__, "Failed to allocate presentation buffer");
      return false;
    }

    VkFenceCreateInfo fi{};
    fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VkFence fence;
    if (auto r = vkCreateFence(dev, &fi, 0, &fence); r != VK_SUCCESS) {
      addErrMsg(backend->logs, __func__, "Failed to create frame fence");
      return false;
    }
    frame.syncFence = {fence,
                       [dev](auto *const p) { vkDestroyFence(dev, p, 0); }};

    VkSemaphoreCreateInfo si{};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore sem;
    if (auto r = vkCreateSemaphore(dev, &si, 0, &sem); r != VK_SUCCESS) {
      addErrMsg(backend->logs, __func__, "Failed to create frame semaphore");
      return false;
    }
    frame.imageAcquiredSem = {
        sem, [dev](auto *const p) { vkDestroySemaphore(dev, p, 0); }};

    if (auto r = vkCreateSemaphore(dev, &si, 0, &sem); r != VK_SUCCESS) {
      addErrMsg(backend->logs, __func__, "Failed to create frame semaphore");
      return false;
    }
    frame.renderDoneSem = {
        sem, [dev](auto *const p) { vkDestroySemaphore(dev, p, 0); }};
  }

  return true;
}

bool createDepthImage(VulkanBackend *const backend,
                      uint32_t imageWidth,
                      uint32_t imageHeight,
                      CustomUniqPtr<VkImage_T> *const out);

bool createDepthAttachment(VulkanBackend *const backend,
                           VulkanWindow *const window) {
  int width, height;
  glfwGetWindowSize(window->handle.get(), &width, &height);

  if (!createDepthImage(backend, width, height, &window->depthImage)) {
    addErrMsg(backend->logs, __func__, "Failed to create depth image");
    return false;
  }

  if (!transitionImageLayout(backend,
                             window->depthImage.get(),
                             VK_IMAGE_ASPECT_DEPTH_BIT,
                             VK_PIPELINE_STAGE_2_NONE,
                             VK_PIPELINE_STAGE_2_NONE,
                             VK_ACCESS_2_NONE,
                             VK_ACCESS_2_NONE,
                             VK_IMAGE_LAYOUT_UNDEFINED,
                             VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL)) {
    addErrMsg(
        backend->logs, __func__, "Failed to transition depth image layout");
    return false;
  }

  VkImageViewCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.image = window->depthImage.get();
  info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  info.components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                       .a = VK_COMPONENT_SWIZZLE_IDENTITY};
  info.subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
  };

  info.format = VK_FORMAT_D32_SFLOAT;

  auto const dev = backend->device->handle.get();
  VkImageView handle{};
  if (auto r = vkCreateImageView(dev, &info, 0, &handle); r != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to create depth image view", r);
    return false;
  }

  window->depthView = {
      handle, [dev](VkImageView_T *const p) { vkDestroyImageView(dev, p, 0); }};

  return true;
}

bool createWindowResources(VulkanBackend *const backend,
                           VulkanWindow *const window) {
  uint32_t count{};

  if (auto result = vkGetSwapchainImagesKHR(
          backend->device->handle.get(), window->swapchain.get(), &count, 0);
      result != VK_SUCCESS) {
    addErrMsg(
        backend, __func__, "Failed to query swapchain image count", result);
    return false;
  }

  window->imageViews.resize(count);
  window->images.resize(count);

  if (auto result = vkGetSwapchainImagesKHR(backend->device->handle.get(),
                                            window->swapchain.get(),
                                            &count,
                                            window->images.data());
      result != VK_SUCCESS) {
    addErrMsg(backend, __func__, "Failed to fetch swapchain images", result);
    return false;
  }

  for (auto image : window->images)
    window->imageLayouts.emplace(image, VK_IMAGE_LAYOUT_UNDEFINED);

  VkImageView view{};
  VkImageViewCreateInfo vci{};
  vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
  vci.components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                      .a = VK_COMPONENT_SWIZZLE_IDENTITY};
  vci.format = window->surfaceFormat.format;
  vci.subresourceRange =
      VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                              .baseMipLevel = 0,
                              .levelCount = 1,
                              .baseArrayLayer = 0,
                              .layerCount = 1};

  auto const dev = backend->device->handle.get();

  for (std::size_t i = 0; i < window->images.size(); ++i) {
    vci.image = window->images[i];
    if (auto r = vkCreateImageView(dev, &vci, 0, &view); r != VK_SUCCESS) {
      addErrMsg(backend->logs, __func__, "Failed to create image view");
      return false;
    }
    window->imageViews[i] = {
        view, [dev](auto *const p) { vkDestroyImageView(dev, p, 0); }};
  }

  if (!createDepthAttachment(backend, window)) {
    addErrMsg(backend->logs, __func__, "Failed to create depth image");
    return false;
  }

  return createFrameResources(backend, window);
}

bool createWindow(VulkanBackend *const handle,
                  uint32_t const width,
                  uint32_t const height,
                  char const *const title,
                  VulkanWindow **const window) {
  if (!handle)
    return false;

  uint32_t windowWidth = width, windowHeight = height;
  if (width < 640) {
    addWrnMsg(
        handle->logs, __func__, "The requested window width is too small");
    windowWidth = 640;
  }

  if (height < 480) {
    addWrnMsg(
        handle->logs, __func__, "The requested window height is too small");
    windowHeight = 480;
  }

  if (!window) {
    addErrMsg(handle->logs, __func__, "The window handle = nullptr");
    return false;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  auto w =
      glfwCreateWindow(windowWidth, windowHeight, title ? title : "", 0, 0);
  if (!w) {
    addErrMsg(handle->logs, __func__, "Failed to create GLFW window");
    return false;
  }

  if (!handle->instance) {
    addErrMsg(handle->logs, __func__, "The instance hasn't been initialized");
    return false;
  }

  auto const inst = handle->instance->handle.get();
  auto windowData = std::make_unique<VulkanWindow>();
  if (!inst) {
    addErrMsg(handle->logs, __func__, "The instance hasn't been initialized");
    return false;
  }
  windowData->handle = {w, [](auto *const p) { glfwDestroyWindow(p); }};

  if (!createCommandPool(handle,
                         handle->device->graphicsFamilyIndex,
                         &windowData->graphicsCmdPool)) {
    addErrMsg(handle->logs, __func__, "Failed to create window command pool");
    return false;
  }

  if (!createCommandPool(handle,
                         handle->device->presentationFamilyIndex,
                         &windowData->presentationCmdPool)) {
    addErrMsg(handle->logs, __func__, "Failed to create window command pool");
    return false;
  }

  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(inst, w, 0, &surface)) {
    addErrMsg(handle->logs, __func__, "Failed to create surface");
    return false;
  }
  windowData->surface = {
      surface, [inst](auto *const p) { vkDestroySurfaceKHR(inst, p, 0); }};

  if (!createWindowSwapchain(handle, windowData.get())) {
    addErrMsg(handle->logs, __func__, "Failed to create swapchain");
    return false;
  }

  if (!createWindowResources(handle, windowData.get())) {
    addErrMsg(handle->logs, __func__, "Failed to create frame resources");
    return false;
  }

  windowData->windowWidth = width;
  windowData->windowHeight = height;
  windowData->logs = handle->logs;
  handle->windows.push_back(std::move(windowData));
  *window = handle->windows.back().get();
  return true;
}

bool getGeneralCommandPool(VulkanBackend const *const backend,
                           VkCommandPool *const cmdPool) {
  if (!backend)
    return false;

  if (!cmdPool) {
    addErrMsg(backend->logs, __func__, "The output handle = nullptr");
    return false;
  }

  if (!backend->device) {
    addErrMsg(
        backend->logs, __func__, "The backend device has not been initialized");
    return false;
  }

  if (!backend->device->cmdPool) {
    addErrMsg(
        backend->logs, __func__, "The command pool has not been initialized");
    return false;
  }

  *cmdPool = backend->device->cmdPool.get();
  return true;
}

void addErrMsg(VulkanBackend const *const handle,
               std::string const &tag,
               std::string const &msg,
               VkResult const r) {
  std::string vkmsg = std::string{", Vulkan code: "} + string_VkResult(r);
  addErrMsg(handle->logs, tag, msg + vkmsg);
}

bool getVulkanAllocator(VulkanBackend const *const backend,
                        VmaAllocator *const alloc) {
  if (!backend)
    return false;

  if (!alloc) {
    addErrMsg(backend->logs, __func__, "The allocator handle = nullptr");
    return false;
  }

  if (!backend->device) {
    addErrMsg(backend->logs, __func__, "The device has not beed initialized");
    return false;
  }

  auto const &a = backend->device->allocator;
  if (!a) {
    addErrMsg(
        backend->logs, __func__, "The allocator has not beed initialized");
    return false;
  }

  *alloc = a.get();
  return true;
}

bool createVulkanDevice(VulkanBackend *const handle) {
  if (!handle)
    return false;

  handle->device = std::make_unique<VulkanDevice>();
  VkDeviceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  VkPhysicalDeviceSynchronization2Features sync2{};
  VkPhysicalDeviceDynamicRenderingFeatures dynamicR{};
  VkPhysicalDeviceFeatures2 features{};
  features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  dynamicR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
  dynamicR.dynamicRendering = VK_TRUE;
  sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
  sync2.synchronization2 = VK_TRUE;
  sync2.pNext = &dynamicR;
  features.pNext = &sync2;
  features.features.samplerAnisotropy = VK_TRUE;

  char const *exts[] = {"VK_KHR_swapchain"};
  info.ppEnabledExtensionNames = exts;
  info.enabledExtensionCount = sizeof(exts) / sizeof(exts[0]);

  uint32_t count;
  std::vector<VkPhysicalDevice> devices;
  auto const instance = handle->instance->handle.get();
  if (auto r = vkEnumeratePhysicalDevices(instance, &count, 0);
      r != VK_SUCCESS) {
    addErrMsg(handle->logs, __func__, "Failed to enumerate devices");
    return false;
  }
  devices.resize(count);
  vkEnumeratePhysicalDevices(instance, &count, devices.data());

  struct DevInfo {
    uint32_t graphicsFam, presentFam;
    VkPhysicalDevice device;
    uint32_t maxImgSize;
    std::string devName;
    float maxAnisotropy;
  };

  std::vector<DevInfo> minCapDevs;
  for (auto dev : devices) {
    VkPhysicalDeviceSynchronization2Features sync2{};
    VkPhysicalDeviceDynamicRenderingFeatures dynamicR{};
    dynamicR.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
    sync2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    VkPhysicalDeviceFeatures2 f2{};
    f2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    sync2.pNext = &dynamicR;
    f2.pNext = &sync2;
    vkGetPhysicalDeviceFeatures2(dev, &f2);

    if (dynamicR.dynamicRendering == VK_FALSE)
      continue;
    if (sync2.synchronization2 == VK_FALSE)
      continue;
    if (f2.features.multiDrawIndirect == VK_FALSE)
      continue;
    if (f2.features.samplerAnisotropy == VK_FALSE)
      continue;

    VkPhysicalDeviceProperties p{};
    vkGetPhysicalDeviceProperties(dev, &p);
    if (p.limits.maxImageDimension2D < 1920)
      continue;

    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, 0);
    std::vector<VkQueueFamilyProperties> q(count, VkQueueFamilyProperties{});
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &count, q.data());

    std::vector<std::uint32_t> graphics, present;
    for (std::size_t i = 0; i < q.size(); ++i) {
      if (!(q[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
        continue;
      graphics.push_back(i);
    }
    for (std::size_t i = 0; i < q.size(); ++i) {
      if (glfwGetPhysicalDevicePresentationSupport(instance, dev, i) ==
          GLFW_FALSE)
        continue;
      present.push_back(i);
    }

    if (!graphics.size() || !present.size())
      continue;

    minCapDevs.push_back({.graphicsFam = graphics.front(),
                          .presentFam = present.front(),
                          .device = dev,
                          .maxImgSize = p.limits.maxImageDimension2D,
                          .devName = p.deviceName,
                          .maxAnisotropy = p.limits.maxSamplerAnisotropy});
  }

  if (!minCapDevs.size()) {
    addErrMsg(handle->logs, __func__, "No eligible device available");
    return false;
  }

  DevInfo *best = &minCapDevs.front();
  if (!handle->generalInfo->preferredGPU.size()) {
    uint32_t maxSz{};
    for (auto &dev : minCapDevs) {
      if (dev.maxImgSize > maxSz) {
        maxSz = dev.maxImgSize;
        best = &dev;
      }
    }
  } else {
    std::regex gpuName{handle->generalInfo->preferredGPU};
    for (auto &dev : minCapDevs)
      if (std::regex_match(dev.devName, gpuName)) {
        handle->generalInfo->preferredGPU = dev.devName;
        best = &dev;
        break;
      }
    if (!best) {
      addErrMsg(handle->logs,
                __func__,
                "The preferred GPU does not meet the requirements: " +
                    handle->generalInfo->preferredGPU);
      return false;
    }
  }

  float prio = 1.f;
  std::vector<VkDeviceQueueCreateInfo> qInfo;
  qInfo.push_back({});
  qInfo.back().sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  qInfo.back().queueCount = 1;
  qInfo.back().pQueuePriorities = &prio;
  qInfo.back().queueFamilyIndex = best->graphicsFam;

  if (best->graphicsFam != best->presentFam) {
    qInfo.push_back({});
    qInfo.back().sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qInfo.back().queueCount = 1;
    qInfo.back().pQueuePriorities = &prio;
    qInfo.back().queueFamilyIndex = best->graphicsFam;
  }

  info.queueCreateInfoCount = (best->graphicsFam == best->presentFam ? 1 : 2);
  info.pQueueCreateInfos = qInfo.data();
  info.pNext = &features;

  addInfMsg(handle->logs, __func__, "Selected device: " + best->devName);

  VkDevice logical{};
  auto result = vkCreateDevice(best->device, &info, 0, &logical);
  if (result != VK_SUCCESS) {
    addErrMsg(handle->logs,
              __func__,
              "Failed to create device with error code: " +
                  std::to_string(result));
    return false;
  }

  handle->device->handle = {logical,
                            [](auto *ptr) { vkDestroyDevice(ptr, 0); }};
  VkQueue pres, graph;
  vkGetDeviceQueue(logical, best->graphicsFam, 0, &graph);
  vkGetDeviceQueue(logical, best->presentFam, 0, &pres);
  handle->device->presentation = pres;
  handle->device->graphics = graph;
  handle->device->physical = best->device;
  handle->device->graphicsFamilyIndex = best->graphicsFam;
  handle->device->presentationFamilyIndex = best->presentFam;
  handle->device->maxAnisotropy = best->maxAnisotropy;
  return true;
}

bool enableValidationLayers(VulkanBackend *const handle) {
  if (!handle)
    return false;

  if (!handle->generalInfo) {
    addErrMsg(handle->logs, __func__, "generalInfo = nullptr");
    return false;
  }

  handle->generalInfo->enableValidationLayers = true;
  return true;
}

bool setPreferredGPU(VulkanBackend *const handle, char const *const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle->logs, __func__, "The GPU name handle = nullptr");
    return false;
  }

  handle->generalInfo->preferredGPU = p;
  return true;
}

bool setApplicationName(VulkanBackend *const handle, char const *const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle->logs, __func__, "The application name handle = nullptr");
    return false;
  }

  handle->generalInfo->appName = p;
  return true;
}

bool enableExtensions(VulkanBackend *const handle,
                      VkInstanceCreateInfo *const info,
                      char const **const extensions,
                      unsigned const size) {
  static char const *layers[] = {"VK_LAYER_KHRONOS_validation"};
  static unsigned const sz = sizeof(layers) / sizeof(layers[0]);

  if (handle->generalInfo->enableValidationLayers) {
    info->ppEnabledLayerNames = layers;
    info->enabledLayerCount = sz;
  }

  std::vector<VkExtensionProperties> properties;
  uint32_t count;

  auto result = vkEnumerateInstanceExtensionProperties(0, &count, 0);
  if (result != VK_SUCCESS) {
    std::string errCode = string_VkResult(result);
    addErrMsg(handle->logs,
              __func__,
              "Failed to enumerate instance extensions with error code: " +
                  errCode);
    return false;
  }

  properties.resize(count);
  result = vkEnumerateInstanceExtensionProperties(0, &count, properties.data());
  if (result != VK_SUCCESS) {
    std::string errCode = string_VkResult(result);
    addErrMsg(handle->logs,
              __func__,
              "Failed to enumerate instance extensions with error code: " +
                  errCode);
    return false;
  }

  std::vector<std::string> requiredInstanceExtensions;
  for (auto const &req : requiredInstanceExtensions) {
    std::string missing;
    for (auto const &ext : properties) {
      if (req == std::string{ext.extensionName}) {
        missing = req;
        break;
      }
    }

    if (missing.size()) {
      addErrMsg(handle->logs,
                __func__,
                "The required instance extension is missing: " + missing);
      return false;
    }
  }

  info->ppEnabledExtensionNames = extensions;
  info->enabledExtensionCount = size;
  return true;
}

bool createVulkanInstance(VulkanBackend *const handle) {
  VkInstanceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  VkApplicationInfo app{};
  app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app.pApplicationName = handle->generalInfo->appName.c_str();
  app.pEngineName = "BADLINE";
  app.engineVersion = VK_MAKE_VERSION(0, 1, 0);
  app.apiVersion = BackendInfo::MIN_REQUIRED_VK_API_VERSION;

  std::vector<std::string> requiredInstanceExtensions;
  uint32_t count{};
  char const *const *const glfwExts = glfwGetRequiredInstanceExtensions(&count);
  for (uint32_t i = 0; i < count; ++i)
    requiredInstanceExtensions.push_back(glfwExts[i]);

  std::vector<char const *> extensions;
  for (auto const &ext : requiredInstanceExtensions)
    extensions.push_back(ext.c_str());

  if (!enableExtensions(handle, &info, extensions.data(), extensions.size())) {
    addErrMsg(handle->logs, __func__, "Failed to enable extensions / layers");
    return false;
  }

  info.pApplicationInfo = &app;
  VkInstance instance{};

  auto result = vkCreateInstance(&info, 0, &instance);
  if (result != VK_SUCCESS) {
    std::string errCode = string_VkResult(result);
    addErrMsg(handle->logs, __func__, "Failed with error code: " + errCode);
    return false;
  }

  handle->instance = std::make_unique<VulkanInstance>();
  handle->instance->handle = {instance,
                              [](auto *p) { vkDestroyInstance(p, 0); }};
  return true;
}

bool getVersionOfAPI(VulkanBackend const *const backend,
                     unsigned *const version) {
  if (!backend)
    return false;
  if (!version) {
    addErrMsg(backend->logs, __func__, "The output handle = nullptr");
    return false;
  }
  *version = BackendInfo::MIN_REQUIRED_VK_API_VERSION;
  return true;
}

bool getVulkanInstance(VulkanBackend const *const backend,
                       VkInstance *const instance) {
  if (!backend)
    return false;
  if (!instance) {
    addErrMsg(backend->logs, __func__, "The output handle = nullptr");
    return false;
  }
  if (!backend->instance) {
    addErrMsg(backend->logs, __func__, "The backend instance handle = nullptr");
    return false;
  }
  *instance = backend->instance->handle.get();
  return true;
}

bool getPhysicalDevice(VulkanBackend const *const backend,
                       VkPhysicalDevice *const physical) {
  if (!backend)
    return false;
  if (!physical) {
    addErrMsg(backend->logs, __func__, "The output handle = nullptr");
    return false;
  }
  if (!backend->device->physical) {
    addErrMsg(backend->logs,
              __func__,
              "The backend physical device handle = nullptr");
    return false;
  }
  *physical = backend->device->physical;
  return true;
}

bool getLogicalDevice(VulkanBackend const *const backend,
                      VkDevice *const logical) {
  if (!backend)
    return false;
  if (!logical) {
    addErrMsg(backend->logs, __func__, "The output handle = nullptr");
    return false;
  }
  if (!backend->device->handle.get()) {
    addErrMsg(backend->logs, __func__, "The backend device handle = nullptr");
    return false;
  }
  *logical = backend->device->handle.get();
  return true;
}

bool createVulkanAllocator(VulkanBackend *const backend,
                           CustomUniqPtr<VmaAllocator_T> *const allocator);

bool allocateMemory(VulkanBackend *const handle) {
  if (!handle)
    return false;
  if (!handle->device) {
    addErrMsg(handle->logs, __func__, "The backend device is not initialized");
    return false;
  }

  if (!createVulkanAllocator(handle, &handle->device->allocator)) {
    addErrMsg(handle->logs, __func__, "Failed to create allocator");
    return false;
  }

  VkDevice dev;
  if (!getLogicalDevice(handle, &dev)) {
    addErrMsg(handle->logs, __func__, "Failed to get logical device");
    return false;
  }

  VmaAllocator allocator = handle->device->allocator.get();
  VmaAllocationInfo allocInfo;
  VmaAllocation alloc;

  if (!createStagingBuffer(handle,
                           64 * 1024 * 1024, // 64 MiB
                           VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                           &handle->device->stagingBuffer.handle,
                           &alloc)) {
    addErrMsg(handle->logs, __func__, "Failed to create staging buffer");
    return false;
  }

  vmaGetAllocationInfo(allocator, alloc, &allocInfo);
  handle->device->stagingBuffer.allocInfo = allocInfo;

  if (!createBuffer(handle,
                    sizeof(glm::mat4),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                        VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    &handle->device->camProjBuffer.handle,
                    &alloc)) {
    addErrMsg(handle->logs, __func__, "Failed to create cam proj buffer");
    return false;
  }
  vmaGetAllocationInfo(allocator, alloc, &allocInfo);
  handle->device->camProjBuffer.allocInfo = allocInfo;

  glm::mat4 projection{1.f};
  std::memcpy(handle->device->camProjBuffer.allocInfo.pMappedData,
              &projection,
              sizeof(projection));
  return true;
}

bool initialize(VulkanBackend *const handle) {
  if (!handle)
    return false;

  if (!createVulkanInstance(handle)) {
    addErrMsg(handle->logs, __func__, "Failed to create Vulkan instance");
    return false;
  }

  if (!createVulkanDevice(handle)) {
    addErrMsg(handle->logs, __func__, "Failed to create Vulkan device");
    return false;
  }

  if (!createCommandPool(handle,
                         handle->device->graphicsFamilyIndex,
                         &handle->device->cmdPool)) {
    addErrMsg(handle->logs, __func__, "Failed to create general command pool");
    return false;
  }

  if (!allocateMemory(handle)) {
    addErrMsg(handle->logs, __func__, "Failed to allocate device buffers");
    return false;
  }

  if (!createImageSampler(handle)) {
    addErrMsg(handle->logs, __func__, "Failed to create image sampler");
    return false;
  }

  if (!createDescriptorPoolAndLayouts(handle)) {
    addErrMsg(handle->logs, __func__, "Failed to create descriptors");
    return false;
  }

  if (!createPipelineLayouts(handle)) {
    addErrMsg(handle->logs, __func__, "Failed to create pipeline layout");
    return false;
  }

  if (!createGraphicsPipeline(handle)) {
    addErrMsg(handle->logs, __func__, "Failed to create graphics pipeline");
    return false;
  }
  return true;
}

void create(Logs *const l, VulkanBackend **const handle) {
  if (!l || !handle)
    return;
  auto guard = std::make_unique<VulkanBackend>(l);
  if (!guard)
    return;

  guard->generalInfo = std::make_unique<BackendInfo>();
  if (!guard->generalInfo)
    return;

  std::unique_ptr<VulkanDevice> *devPtr = &guard->device;
  guard->resourceUsageDoneGuard = {malloc(1), [devPtr](auto *const p) {
                                     free(p);
                                     if (devPtr->get() && devPtr->get()->handle)
                                       vkDeviceWaitIdle(
                                           devPtr->get()->handle.get());
                                   }};

  if (!guard->resourceUsageDoneGuard)
    return;
  *handle = guard.release();
}

void destroy(VulkanBackend *const handle) { delete handle; }
} // namespace re
