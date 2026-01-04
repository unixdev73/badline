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

#include <badline/vulkanBackend.hpp>
#include "vulkanBackend.hpp"
#include <unordered_map>
#include "error.hpp"
#include <fstream>

namespace re {
template <typename T, template <typename> typename C>
std::vector<T> subtract(C<T> const &minuend, C<T> const &subtrahend) {
  std::vector<T> difference{};

  for (auto const &minuendElement : minuend) {
    bool exists = false;

    for (auto const &subtrahendElement : subtrahend) {
      if (minuendElement == subtrahendElement) {
        exists = true;
        break;
      }
    }

    if (!exists)
      difference.push_back(minuendElement);
  }

  return difference;
}

bool getInstanceBuffer(VulkanBackend *const backend,
                       UniqueRes<VkBuffer_T> **const p) {
  *p = &backend->instanceBuf;
  return true;
}

bool getIndexBuffer(VulkanBackend *const backend,
                    UniqueRes<VkBuffer_T> **const p) {
  *p = &backend->indexBuf;
  return true;
}

bool getVertexBuffer(VulkanBackend *const backend,
                     UniqueRes<VkBuffer_T> **const p) {
  *p = &backend->vertexBuf;
  return true;
}

bool setResolution(Window *const handle, unsigned const w, unsigned const h) {
  if (!handle)
    return false;
  handle->width = w;
  handle->height = h;
  return true;
}

bool createWindow(VulkanBackend *const backend,
                  uint32_t const width,
                  uint32_t const height);

bool open(Window *const window, VulkanBackend *const vk) {
  if (!window)
    return false;

  if (window->handle) {
    glfwShowWindow(window->handle.get());
    return true;
  }

  return createWindow(vk, window->width, window->height);
}

void addErrMsg(Window const *const window,
               std::string const &msg,
               VkResult const r) {
  addErrMsg(window->logs, msg, r);
}

bool getVulkanInstance(VulkanBackend const *const backend,
                       VkInstance *const p) {
  *p = backend->instance->handle.get();
  return true;
}

bool getPhysicalDevice(VulkanBackend const *const backend,
                       VkPhysicalDevice *const p) {
  *p = backend->device->identifier;
  return true;
}

bool getLogicalDevice(VulkanBackend const *const backend, VkDevice *const p) {
  *p = backend->device->handle.get();
  return true;
}

bool getMemoryAllocator(VulkanBackend const *const backend,
                        VmaAllocator *const p) {
  *p = backend->allocator.get();
  return true;
}

bool getCommandPool(VulkanBackend const *const backend,
                    VkCommandPool *const p) {
  *p = backend->graphicsCmdPool.get();
  return true;
}

bool getVulkanFence(VulkanBackend const *const backend, VkFence *const p) {
  *p = backend->window->fence.get();
  return true;
}

bool getGraphicsQueue(VulkanBackend const *const backend, VkQueue *const p) {
  *p = backend->device->graphics;
  return true;
}

bool getVertexBuffer(VulkanBackend const *const backend, VkBuffer *const p) {
  *p = backend->vertexBuf.get();
  return true;
}

bool getVersionOfAPI(VulkanBackend const *const handle, uint32_t *const p) {
  *p = handle->VULKAN_API_VERSION;
  return true;
}

bool getIndexBuffer(VulkanBackend const *const backend, VkBuffer *const p) {
  *p = backend->indexBuf.get();
  return true;
}

bool getInstanceBuffer(VulkanBackend const *const backend, VkBuffer *const p) {
  *p = backend->instanceBuf.get();
  return true;
}

void addErrMsg(VulkanBackend const *const backend,
               std::string const &msg,
               VkResult r) {
  addErrMsg(backend->logs, msg, r);
}

bool setPosition(Vertex *const handle, glm::vec3 *const p) {
  handle->position = *p;
  return true;
}

bool setColor(Vertex *const handle, glm::vec4 *const p) {
  handle->color = *p;
  return true;
}

bool getPosition(Vertex const *const handle, glm::vec3 *const p) {
  *p = handle->position;
  return true;
}

bool getColor(Vertex const *const handle, glm::vec4 *const p) {
  *p = handle->color;
  return true;
}

bool close(Window const *const window) {
  auto handle = window->handle.get();
  glfwSetWindowShouldClose(handle, VK_TRUE);
  return true;
}

bool setApplicationName(VulkanBackend *const handle, char const *const p) {
  if (!handle || !p)
    return false;
  handle->appName = std::string{p};
  return true;
}

void setValidationLayersOn(VulkanBackend *const handle) {
  if (!handle)
    return;
  handle->validation = true;
}

bool createWindow(VulkanBackend *const backend,
                  uint32_t const width,
                  uint32_t const height) {
  if (!backend->window)
    backend->window = std::make_unique<Window>();
  if (!createGLFWindow(backend, width, height)) {
    addErrMsg(backend, "createWindow: Failed to create GLFW window");
    return false;
  }

  if (!createWindowSurface(backend)) {
    addErrMsg(backend, "createWindow: Failed to create window surface");
    return false;
  }

  if (!queryPresentModes(backend)) {
    addErrMsg(backend, "createWindow: Failed to query present modes");
    return false;
  }

  if (!isPresentModeAvailable(backend)) {
    addErrMsg(backend, "createWindow: Requested present mode is not available");
    return false;
  }

  if (!checkSurfaceEligibility(backend)) {
    addErrMsg(backend, "createWindow: The surface is not eligible");
    return false;
  }

  if (!createWindowFence(backend)) {
    addErrMsg(backend, "createWindow: Failed to create window fence");
    return false;
  }

  if (!createWindowSwapchain(backend)) {
    addErrMsg(backend, "createWindow: Failed to create swapchain");
    return false;
  }

  if (!fetchSwapchainImages(backend)) {
    addErrMsg(backend, "createWindow: Failed to fetch swapchain images");
    return false;
  }

  if (!createDepthAttachment(backend)) {
    addErrMsg(backend, "createWindow: Failed to create depth attachment");
    return false;
  }

  if (!allocateCommandBuffers(backend)) {
    addErrMsg(backend, "createWindow: Failed to allocate command buffers");
    return false;
  }

  if (!transitionSwapchainImages(backend)) {
    addErrMsg(backend, "createWindow: Failed to transition swapchain images");
    return false;
  }

  if (!createSwapchainImageViews(backend)) {
    addErrMsg(backend, "createWindow: Failed to create swapchain image views");
    return false;
  }

  if (!createWindowSemaphores(backend)) {
    addErrMsg(backend, "createWindow: Failed to create window semaphores");
    return false;
  }

  if (!createPipelineLayout(backend)) {
    addErrMsg(backend, "createWindow: Failed to create pipeline layout");
    return false;
  }

  if (!createGraphicsPipeline(backend)) {
    addErrMsg(backend, "createWindow: Failed to create graphics pipeline");
    return false;
  }

  return true;
}

bool createDepthAttachment(VulkanBackend *const backend) {
  UniqueRes<VkImage_T> img{};
  if (!createDepthImage(
          backend, backend->window->width, backend->window->height, &img)) {
    addErrMsg(backend, "createDepthAttachment: Failed to create depth image");
    return false;
  }

  VkImageViewCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  info.image = img.get();
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
    addErrMsg(
        backend, "createDepthAttachment: Failed to create depth image view", r);
    return false;
  }

  backend->window->depthImg = std::move(img);
  backend->window->depthImgView = {
      handle, [dev](VkImageView_T *const p) { vkDestroyImageView(dev, p, 0); }};

  return true;
}

bool createGraphicsPipeline(VulkanBackend *const backend) {
  UniqueRes<VkShaderModule_T> vertex{}, fragment{};
  if (!createShaderModule(backend, "shaders/vertex.spv", &vertex)) {
    addErrMsg(backend,
              "createGraphicsPipeline: Failed to create vertex shader");
    return false;
  }

  if (!createShaderModule(backend, "shaders/fragment.spv", &fragment)) {
    addErrMsg(backend,
              "createGraphicsPipeline: Failed to create fragment shader");
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
       .module = fragment.get(),
       .pName = "main",
       .pSpecializationInfo = 0}};

  VkPipelineVertexInputStateCreateInfo vertexInputState{};
  vertexInputState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  auto const attribDesc = Vertex::attributeDescription();
  vertexInputState.pVertexAttributeDescriptions = attribDesc.data();
  vertexInputState.vertexAttributeDescriptionCount = attribDesc.size();
  auto const bindDesc = Vertex::bindingDescription();
  vertexInputState.pVertexBindingDescriptions = &bindDesc;
  vertexInputState.vertexBindingDescriptionCount = 1;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
  inputAssemblyState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyState.primitiveRestartEnable = VK_FALSE;
  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

  VkPipelineRasterizationStateCreateInfo rasterizationState{};
  rasterizationState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationState.depthClampEnable = VK_FALSE;
  rasterizationState.rasterizerDiscardEnable = VK_FALSE;
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.lineWidth = 1.0f;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
  colorBlendState.logicOpEnable = VK_TRUE;
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
  nextInfo.colorAttachmentCount = 1;

  VkFormat formats[] = {backend->window->surfaceFormat.format};
  nextInfo.pColorAttachmentFormats = formats;

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
  info.layout =
      backend->pipelineLayouts[backend->window->activePipelineLayout].get();
  info.basePipelineHandle = VK_NULL_HANDLE;
  info.basePipelineIndex = 0;

  auto const dev = backend->device->handle.get();
  VkPipeline handle{VK_NULL_HANDLE};
  auto r = vkCreateGraphicsPipelines(dev, 0, 1, &info, 0, &handle);
  if (r != VK_SUCCESS) {
    addErrMsg(backend,
              "createGraphicsPipeline: Failed to create graphics pipeline",
              r);
    return false;
  }

  backend->window->activePipeline = backend->pipelines.size();
  backend->pipelines.push_back(UniqueRes<VkPipeline_T>{
      handle, [dev](VkPipeline_T *const p) { vkDestroyPipeline(dev, p, 0); }});
  return true;
}

bool createPipelineLayout(VulkanBackend *const backend) {
  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  VkDescriptorSetLayout layouts[] = {{}};
  info.pSetLayouts = layouts;
  info.setLayoutCount = 0;
  VkPushConstantRange ranges[] = {
      VkPushConstantRange{.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                          .offset = 0,
                          .size = sizeof(backend->camMats)}};
  info.pushConstantRangeCount = sizeof(ranges) / sizeof(ranges[0]);
  info.pPushConstantRanges = ranges;

  auto const descLayout = backend->descLayout.get();
  info.pSetLayouts = &descLayout;
  info.setLayoutCount = 1;

  auto const dev = backend->device->handle.get();
  VkPipelineLayout handle{};
  auto result = vkCreatePipelineLayout(dev, &info, 0, &handle);

  if (result != VK_SUCCESS) {
    addErrMsg(backend,
              "createPipelineLayout: Failed to create pipeline layout",
              result);
    return false;
  }

  backend->window->activePipelineLayout = backend->pipelineLayouts.size();
  backend->pipelineLayouts.push_back(
      {handle, [dev](VkPipelineLayout_T *const p) {
         vkDestroyPipelineLayout(dev, p, 0);
       }});

  return true;
}

bool createWindowFence(VulkanBackend *const backend) {
  VkFenceCreateInfo finf{};
  finf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VkFence f{};
  auto dev = backend->device->handle.get();
  auto r = vkCreateFence(backend->device->handle.get(), &finf, 0, &f);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, "createWindowFence: Failed to create fence", r);
    return false;
  }
  auto &fence = backend->window->fence;
  fence = {f, [dev](VkFence_T *const p) { vkDestroyFence(dev, p, 0); }};

  return true;
}

bool allocateCommandBuffers(VulkanBackend *const backend) {
  auto const dev = backend->device->handle.get();

  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.commandPool = backend->graphicsCmdPool.get();
  info.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  auto &buf = backend->window->graphicsBuf;
  info.commandBufferCount = 1;

  auto result = vkAllocateCommandBuffers(dev, &info, &buf);
  if (result != VK_SUCCESS) {
    addErrMsg(backend,
              "allocateCommandBuffers: Failed to allocate cmd buffer",
              result);
    return false;
  }

  return true;
}

bool createWindowSemaphores(VulkanBackend *const backend) {
  auto const dev = backend->device->handle.get();
  VkSemaphoreCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  backend->window->renderSem.resize(backend->window->swapImages.size());
  std::vector<UniqueRes<VkSemaphore_T> *> sem = {&backend->window->acquireSem};
  for (auto &s : backend->window->renderSem)
    sem.push_back(&s);

  for (std::size_t i = 0; i < sem.size(); ++i) {
    VkSemaphore semaphore{};
    auto result = vkCreateSemaphore(dev, &info, 0, &semaphore);
    if (result != VK_SUCCESS) {
      addErrMsg(backend,
                "createWindowSemaphores: Failed to create render semaphore",
                result);
      return false;
    }

    *sem[i] = {semaphore, [dev](VkSemaphore_T *const p) {
                 vkDestroySemaphore(dev, p, 0);
               }};
  }

  return true;
}

bool createSwapchainImageViews(VulkanBackend *const backend) {
  auto const size = backend->window->swapImages.size();
  auto const dev = backend->device->handle.get();
  auto &views = backend->window->swapImgViews;
  views.resize(size);

  for (std::size_t i = 0; i < size; ++i) {
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
    info.image = backend->window->swapImages[i];
    info.components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_R,
                                         .g = VK_COMPONENT_SWIZZLE_G,
                                         .b = VK_COMPONENT_SWIZZLE_B,
                                         .a = VK_COMPONENT_SWIZZLE_A};

    info.format = backend->window->surfaceFormat.format;
    info.subresourceRange = VkImageSubresourceRange{
        .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1};

    VkImageView view{};
    auto result = vkCreateImageView(dev, &info, 0, &view);
    if (result != VK_SUCCESS) {
      addErrMsg(backend,
                "createSwapchainImageViews: Failed to create image view",
                result);
      return false;
    }

    views[i] = {
        view, [dev](VkImageView_T *const p) { vkDestroyImageView(dev, p, 0); }};
  }

  return true;
}

bool transitionSwapchainImages(VulkanBackend *const backend) {
  VkCommandBuffer const cmd = backend->window->graphicsBuf;
  VkCommandBufferBeginInfo cbi{};
  cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (auto r = vkBeginCommandBuffer(cmd, &cbi); r != VK_SUCCESS) {
    addErrMsg(
        backend,
        "transitionSwapchainImages: Failed to begin recording command buffer",
        r);
    return false;
  }

  VkImageMemoryBarrier2 barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.srcAccessMask = VK_ACCESS_2_NONE;
  barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.dstAccessMask = VK_ACCESS_2_NONE;
  barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  depInfo.imageMemoryBarrierCount = 1;
  depInfo.pImageMemoryBarriers = &barrier;

  for (std::size_t i = 0; i < backend->window->swapImages.size(); ++i) {
    barrier.image = backend->window->swapImages[i];
    vkCmdPipelineBarrier2(cmd, &depInfo);
  }

  if (auto r = vkEndCommandBuffer(cmd); r != VK_SUCCESS) {
    addErrMsg(
        backend,
        "transitionSwapchainImages: Failed to end recording command buffer",
        r);
    return false;
  }

  auto const fence = backend->window->fence.get();
  VkSubmitInfo2 submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

  submitInfo.commandBufferInfoCount = 1;
  VkCommandBufferSubmitInfo cbsi{};
  cbsi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cbsi.commandBuffer = backend->window->graphicsBuf;
  submitInfo.pCommandBufferInfos = &cbsi;

  if (auto r = vkQueueSubmit2(backend->device->graphics, 1, &submitInfo, fence);
      r != VK_SUCCESS) {
    addErrMsg(backend,
              "transitionSwapchainImages: Failed to submit command buffer",
              r);
    return false;
  }

  auto const dev = backend->device->handle.get();
  if (auto r = vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
      r != VK_SUCCESS) {
    addErrMsg(
        backend, "transitionSwapchainImages: Failed to wait for fence", r);
    return false;
  }

  if (auto r = vkResetFences(dev, 1, &fence); r != VK_SUCCESS) {
    addErrMsg(backend, "transitionSwapchainImages: Failed to reset fence", r);
    return false;
  }

  return true;
}

bool fetchSwapchainImages(VulkanBackend *const backend) {
  uint32_t count{};

  if (auto result = vkGetSwapchainImagesKHR(backend->device->handle.get(),
                                            backend->window->swapchain.get(),
                                            &count,
                                            0);
      result != VK_SUCCESS) {
    addErrMsg(backend,
              "fetchSwapchainImages: Failed to query swapchain image count",
              result);
    return false;
  }

  backend->window->swapImages.resize(count);

  if (auto result = vkGetSwapchainImagesKHR(backend->device->handle.get(),
                                            backend->window->swapchain.get(),
                                            &count,
                                            backend->window->swapImages.data());
      result != VK_SUCCESS) {
    addErrMsg(backend,
              "fetchSwapchainImages: Failed to fetch swapchain images",
              result);
    return false;
  }

  return true;
}

bool createWindowSwapchain(VulkanBackend *const backend) {
  auto numberOfImages = backend->window->surfaceCaps.minImageCount + 1;
  if ((backend->window->surfaceCaps.maxImageCount > 0) &&
      (numberOfImages > backend->window->surfaceCaps.maxImageCount)) {
    numberOfImages = backend->window->surfaceCaps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swpInfo{};
  swpInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swpInfo.compositeAlpha =
      VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swpInfo.imageArrayLayers = 1;
  swpInfo.clipped = VK_TRUE;
  swpInfo.imageColorSpace = backend->window->surfaceFormat.colorSpace;
  swpInfo.imageFormat = backend->window->surfaceFormat.format;

  uint32_t const width = backend->window->width,
                 height = backend->window->height;
  swpInfo.imageExtent = VkExtent2D{width, height};
  swpInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swpInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swpInfo.surface = backend->window->surface.get();
  swpInfo.presentMode = backend->window->presentMode;
  swpInfo.minImageCount = numberOfImages;
  swpInfo.preTransform =
      VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

  VkSwapchainKHR swapchain{};
  if (auto result = vkCreateSwapchainKHR(
          backend->device->handle.get(), &swpInfo, 0, &swapchain);
      result != VK_SUCCESS) {
    addErrMsg(backend,
              "createWindowSwapchain: Failed to create window swapchain",
              result);
    return false;
  }

  auto dev = backend->device->handle.get();
  backend->window->swapchain = {swapchain, [dev](VkSwapchainKHR_T *const ptr) {
                                  vkDestroySwapchainKHR(dev, ptr, 0);
                                }};

  return true;
}

bool checkSurfaceEligibility(VulkanBackend *const backend) {
  uint32_t count{};
  if (auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          backend->device->identifier,
          backend->window->surface.get(),
          &backend->window->surfaceCaps);
      result != VK_SUCCESS) {
    addErrMsg(backend,
              "checkSurfaceEligibility: Failed to query surface capabilities",
              result);
    return false;
  }

  if (backend->window->width >
      backend->window->surfaceCaps.maxImageExtent.width) {
    addErrMsg(backend,
              "checkSurfaceEligibility: Requested surface width too large");
    return false;
  }

  if (backend->window->height >
      backend->window->surfaceCaps.maxImageExtent.height) {
    addErrMsg(backend,
              "checkSurfaceEligibility: Requested surface height too large");
    return false;
  }

  if (!(backend->window->surfaceCaps.supportedUsageFlags &
        VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
    addErrMsg(backend,
              "checkSurfaceEligibility: Color attachment not supported");
    return false;
  }

  if (auto result =
          vkGetPhysicalDeviceSurfaceFormatsKHR(backend->device->identifier,
                                               backend->window->surface.get(),
                                               &count,
                                               0);
      result != VK_SUCCESS) {
    addErrMsg(backend,
              "checkSurfaceEligibility: Failed to query surface formats",
              result);
    return false;
  }

  backend->window->surfaceFormats.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          backend->device->identifier,
          backend->window->surface.get(),
          &count,
          backend->window->surfaceFormats.data());
      result != VK_SUCCESS) {
    addErrMsg(backend,
              "checkSurfaceEligibility: Failed to fill surface formats",
              result);
    return false;
  }

  if ((1 == backend->window->surfaceFormats.size()) &&
      (VK_FORMAT_UNDEFINED == backend->window->surfaceFormats[0].format)) {
    backend->window->surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    backend->window->surfaceFormat.colorSpace =
        VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  } else if (backend->window->surfaceFormats.size()) {
    backend->window->surfaceFormat = backend->window->surfaceFormats[0];
  } else {
    addErrMsg(backend, "checkSurfaceEligibility: No surface formats available");
    return false;
  }

  return true;
}
bool isPresentModeAvailable(VulkanBackend *const backend) {
  bool found = false;
  for (auto mode : backend->window->presentModes) {
    if (mode == backend->window->presentMode) {
      found = true;
      break;
    }
  }

  if (!found) {
    std::string mode = std::to_string(backend->window->presentMode);
    addErrMsg(backend,
              "isPresentModeAvailable: The requested present mode: " + mode);
    return false;
  }

  return true;
}

bool queryPresentModes(VulkanBackend *const backend) {
  backend->window->presentModes.clear();
  uint32_t count{};
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          backend->device->identifier,
          backend->window->surface.get(),
          &count,
          nullptr);
      result != VK_SUCCESS) {
    addErrMsg(
        backend, "queryPresentModes: Failed to query present modes", result);
    return false;
  }

  backend->window->presentModes.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          backend->device->identifier,
          backend->window->surface.get(),
          &count,
          backend->window->presentModes.data());
      result != VK_SUCCESS) {
    addErrMsg(
        backend, "queryPresentModes: Failed to fill present modes", result);
    return false;
  }

  return true;
}

bool createWindowSurface(VulkanBackend *const backend) {
  auto const win = backend->window->handle.get();
  VkSurfaceKHR surf{};

  if (auto result = glfwCreateWindowSurface(
          backend->instance->handle.get(), win, 0, &surf);
      result != VK_SUCCESS) {
    addErrMsg(backend,
              "createWindowSurface: Failed to create vulkan surface",
              result);
    return false;
  }

  auto inst = backend->instance->handle.get();
  backend->window->surface = {surf, [inst](VkSurfaceKHR_T *const ptr) {
                                if (ptr)
                                  vkDestroySurfaceKHR(inst, ptr, 0);
                              }};

  return true;
}

bool createGLFWindow(VulkanBackend *const backend,
                     uint32_t width,
                     uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

  GLFWwindow *win =
      glfwCreateWindow(width, height, backend->instance->title.c_str(), 0, 0);
  if (!win) {
    addErrMsg(backend, "createGLFWindow: Failed to create GLFW window");
    return false;
  }

  backend->window->handle = {win, [](GLFWwindow *const ptr) {
                               if (ptr)
                                 glfwDestroyWindow(ptr);
                             }};
  backend->window->width = width;
  backend->window->height = height;

  return true;
}

bool isKeyPressed(Window const *const window, int key, bool *const output) {
  if (!window)
    return false;

  if (!output) {
    addErrMsg(window, "isKeyPressed: The parameter 'output' = nullptr");
    return false;
  }

  auto const win = window->handle.get();
  *output = glfwGetKey(win, key) == GLFW_PRESS;
  return true;
}

bool isOpen(Window const *const window, bool *const output) {
  if (!window)
    return false;

  if (!output) {
    addErrMsg(window, "isOpen: The parameter 'output' = nullptr");
    return false;
  }

  *output = false;

  auto win = window->handle.get();
  if (win && glfwWindowShouldClose(window->handle.get()) == GLFW_FALSE)
    *output = true;

  return true;
}

bool createOptimalGPU(VulkanBackend *const backend) {
  backend->device = std::make_unique<Device>();

  std::unordered_map<VkPhysicalDevice, DeviceInfo> devs{};
  if (!queryEligibleDevices(backend->instance->handle.get(), &devs) ||
      !devs.size()) {
    addErrMsg(backend, "createOptimalGPU: Failed to query eligible devices");
    return false;
  }

  std::unordered_map<VkPhysicalDevice, DeviceInfo>::const_iterator it{};
  if (!selectOptimalDevice(devs, &it)) {
    addErrMsg(backend, "createOptimalGPU: Failed to select optimal device");
    return false;
  }

  auto const &[phy, devInfo] = *it;
  std::vector<VkDeviceQueueCreateInfo> qCreateInfos;
  VkDeviceCreateInfo devCreateInfo{};
  VkPhysicalDeviceFeatures reqF{};
  devCreateInfo.pEnabledFeatures = &reqF;

  setupCreateInfo(&devCreateInfo, &qCreateInfos, &reqF, &devInfo);

  if (!createLogicalDevice(backend, &devCreateInfo, phy, &devInfo)) {
    addErrMsg(backend, "createOptimalGPU: Failed to create logical device");
    return false;
  }

  if (!createDescriptors(backend)) {
    addErrMsg(backend, "createOptimalGPU: Failed to create descriptors");
    return false;
  }

  return true;
}

bool createDescriptors(VulkanBackend *const backend) {
  auto const dev = backend->device->handle.get();
  constexpr uint32_t descriptorCount = VulkanBackend::maxDescriptors;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.maxSets = 1;

  VkDescriptorPoolSize poolSizes[] = {VkDescriptorPoolSize{
      .type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = descriptorCount}};

  poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
  poolInfo.pPoolSizes = poolSizes;

  VkDescriptorPool pool{};
  auto r = vkCreateDescriptorPool(dev, &poolInfo, 0, &pool);
  if (r != VK_SUCCESS) {
    addErrMsg(
        backend, "createDescriptors: Failed to create descriptor pool", r);
    return false;
  }

  backend->descPool = {pool, [dev](VkDescriptorPool_T *const p) {
                         vkDeviceWaitIdle(dev);
                         vkDestroyDescriptorPool(dev, p, 0);
                       }};

  VkDescriptorSetLayoutCreateInfo descInfo{};
  descInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

  VkDescriptorSetLayoutBinding bindings[] = {VkDescriptorSetLayoutBinding{
      .binding = 0,
      .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      .descriptorCount = descriptorCount,
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .pImmutableSamplers = 0}};

  descInfo.bindingCount = sizeof(bindings) / sizeof(bindings[0]);
  descInfo.pBindings = bindings;

  VkDescriptorSetLayout descLayout{};
  r = vkCreateDescriptorSetLayout(dev, &descInfo, 0, &descLayout);
  if (r != VK_SUCCESS) {
    addErrMsg(backend,
              "createDescriptors: Failed to create descriptor set layout",
              r);
    return false;
  }

  backend->descLayout = {descLayout, [dev](VkDescriptorSetLayout_T *const p) {
                           vkDestroyDescriptorSetLayout(dev, p, 0);
                         }};

  VkDescriptorSetAllocateInfo descAlloc{};
  descAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  descAlloc.descriptorPool = backend->descPool.get();
  descAlloc.descriptorSetCount = 1;
  descAlloc.pSetLayouts = &descLayout;

  VkDescriptorSet descriptor{};
  r = vkAllocateDescriptorSets(dev, &descAlloc, &descriptor);

  if (r != VK_SUCCESS) {
    addErrMsg(
        backend, "createDescriptors: Failed to allocate descriptor set", r);
    return true;
  }

  backend->descSets.push_back(descriptor);
  return true;
}

bool createLogicalDevice(VulkanBackend *const backend,
                         VkDeviceCreateInfo *const info,
                         VkPhysicalDevice const phy,
                         DeviceInfo const *const devInfo) {

  VkPhysicalDeviceSynchronization2Features sync2Feature{};
  sync2Feature.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
  sync2Feature.synchronization2 = VK_TRUE;

  VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
  dynamicRenderingFeature.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
  dynamicRenderingFeature.pNext = &sync2Feature;
  dynamicRenderingFeature.dynamicRendering = VK_TRUE;

  info->pNext = &dynamicRenderingFeature;

  VkDevice dev{};
  if (auto result = vkCreateDevice(phy, info, 0, &dev); result != VK_SUCCESS) {
    addErrMsg(backend, "createLogicalDevice: Failed to create device", result);
    return false;
  }

  auto &graphicsQ = backend->device->graphics;
  auto &presentQ = backend->device->present;

  backend->device->graphicsFamIndex = devInfo->graphicsQueue.famIndex;
  backend->device->presentFamIndex = devInfo->presentQueue.famIndex;

  vkGetDeviceQueue(dev, devInfo->graphicsQueue.famIndex, 0, &graphicsQ);
  presentQ = graphicsQ;

  if (devInfo->graphicsQueue.famIndex != devInfo->presentQueue.famIndex)
    vkGetDeviceQueue(dev, devInfo->presentQueue.famIndex, 0, &presentQ);

  backend->device->identifier = phy;
  backend->device->handle = {dev, [](VkDevice ptr) {
                               vkDeviceWaitIdle(ptr);
                               vkDestroyDevice(ptr, 0);
                             }};

  if (!createDeviceResources(backend)) {
    addErrMsg(backend,
              "createLogicalDevice: Failed to create device resources");
    return false;
  }

  return true;
}

bool createDeviceResources(VulkanBackend *const backend) {
  if (!createCommandPools(backend)) {
    addErrMsg(backend, "createDeviceResources: Failed to create command pool");
    return false;
  }

  if (!createAllocator(backend, &backend->allocator)) {
    addErrMsg(backend, "createDeviceResources: Failed to create allocator");
    return false;
  }

  return true;
}

bool createCommandPools(VulkanBackend *const backend) {
  VkCommandPoolCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
               VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  auto const dev = backend->device->handle.get();
  VkCommandPool pool{};

  info.queueFamilyIndex = backend->device->graphicsFamIndex;
  auto result = vkCreateCommandPool(dev, &info, 0, &pool);
  if (result != VK_SUCCESS) {
    addErrMsg(
        backend, "createCommandPools: Failed to create command pool", result);
    return false;
  }
  backend->graphicsCmdPool = {pool, [dev](VkCommandPool_T *const p) {
                                vkDestroyCommandPool(dev, p, 0);
                              }};

  return true;
}

void setupCreateInfo(VkDeviceCreateInfo *const devCreateInfo,
                     std::vector<VkDeviceQueueCreateInfo> *const qCreateInfos,
                     VkPhysicalDeviceFeatures *const features,
                     DeviceInfo const *const devInfo) {
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
  features->logicOp = VK_TRUE;
}

bool selectOptimalDevice(
    std::unordered_map<VkPhysicalDevice, DeviceInfo> const &devs,
    std::unordered_map<VkPhysicalDevice, DeviceInfo>::const_iterator *const
        best) {

  *best = devs.begin();
  for (auto it = devs.begin(); it != devs.end(); ++it) {
    auto const cMaxSize = (*best)->second.props.limits.maxImageDimension2D;
    auto const itMaxSize = it->second.props.limits.maxImageDimension2D;
    auto const &itFeats = it->second.feats;

    if (itMaxSize > cMaxSize && itFeats.wideLines == VK_TRUE)
      *best = it;
  }

  if (*best == devs.end())
    return false;
  return true;
}

bool queryEligibleDevices(
    VkInstance const instance,
    std::unordered_map<VkPhysicalDevice, DeviceInfo> *const out) {

  std::vector<VkPhysicalDevice> devs{};
  if (!queryDevices(instance, &devs)) {
    return false;
  }

  for (auto device : devs) {
    bool graphicsFound = false, presentFound = false;
    DeviceInfo info{};
    queryDeviceInfo(device, &info);

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
      out->emplace(device, std::move(info));
  }

  return true;
}

void queryDeviceInfo(VkPhysicalDevice_T *const handle, DeviceInfo *const info) {
  uint32_t count{};

  vkEnumerateDeviceExtensionProperties(handle, 0, &count, 0);
  info->exts.resize(count);
  vkEnumerateDeviceExtensionProperties(handle, 0, &count, info->exts.data());

  vkGetPhysicalDeviceFeatures(handle, &info->feats);
  vkGetPhysicalDeviceProperties(handle, &info->props);

  vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, 0);
  info->queues.resize(count);
  vkGetPhysicalDeviceQueueFamilyProperties(handle, &count, info->queues.data());
}

bool queryDevices(VkInstance const instance,
                  std::vector<VkPhysicalDevice> *const out) {
  std::vector<VkPhysicalDevice> devs{};
  uint32_t count{};

  vkEnumeratePhysicalDevices(instance, &count, 0);
  if (!count)
    return false;

  devs.resize(count);
  vkEnumeratePhysicalDevices(instance, &count, devs.data());

  *out = std::move(devs);
  return true;
}

bool render(VulkanBackend *const backend,
            unsigned long const indexCount,
            unsigned long const instanceCount) {
  if (!backend)
    return false;

  VkCommandBuffer const cmd = backend->window->graphicsBuf;
  auto const dev = backend->device->handle.get();

  uint32_t img{};
  if (!getNextImage(backend, &img)) {
    addErrMsg(backend, "render: Failed to acquire image");
    return false;
  }

  VkCommandBufferBeginInfo cbi{};
  cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  auto r = vkBeginCommandBuffer(cmd, &cbi);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, "render: Failed to begin command buffer");
    return false;
  }

  if (!setRenderBarriers(backend, backend->window->swapImages[img])) {
    addErrMsg(backend, "render: Failed to set memory barriers");
    return false;
  }

  VkRenderingAttachmentInfo colorAttachment{};
  VkRenderingAttachmentInfo depthAttachment{};
  VkRenderingInfo renderingInfo{};

  if (!setupRenderingInfo(
          backend, &renderingInfo, &colorAttachment, &depthAttachment, img)) {
    addErrMsg(backend, "render: Failed to setup rendering info");
    return false;
  }

  vkCmdBeginRendering(cmd, &renderingInfo);

  auto const pipe = backend->pipelines[backend->window->activePipeline].get();
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);

  VkViewport const vp{.x = 0,
                      .y = 0,
                      .width = float(backend->window->width),
                      .height = float(backend->window->height),
                      .minDepth = 0.f,
                      .maxDepth = 1.f};
  vkCmdSetViewportWithCount(cmd, 1, &vp);

  VkRect2D const sc{
      .offset = {0, 0},
      .extent = {backend->window->width, backend->window->height}};
  vkCmdSetScissorWithCount(cmd, 1, &sc);

  auto const layout =
      backend->pipelineLayouts[backend->window->activePipelineLayout].get();
  vkCmdPushConstants(cmd,
                     layout,
                     VK_SHADER_STAGE_VERTEX_BIT,
                     0,
                     sizeof(backend->camMats),
                     &backend->camMats);

  auto const vertexBuf = backend->vertexBuf.get();
  VkDeviceSize const offsets[] = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuf, offsets);

  VkDescriptorBufferInfo bufInf{};
  bufInf.buffer = backend->instanceBuf.get();
  bufInf.offset = 0;
  bufInf.range = instanceCount * sizeof(InstanceTransform);

  auto const descSet = backend->descSets.at(0);
  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = descSet;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &bufInf;
  descriptorWrite.dstArrayElement = 0;
  std::vector<VkWriteDescriptorSet> writeInfo(VulkanBackend::maxDescriptors,
                                              descriptorWrite);

  for (std::size_t i = 0; i < VulkanBackend::maxDescriptors; ++i)
    writeInfo[i].dstArrayElement = i;

  vkUpdateDescriptorSets(dev, writeInfo.size(), writeInfo.data(), 0, nullptr);
  vkCmdBindDescriptorSets(
      cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descSet, 0, 0);

  vkCmdBindIndexBuffer(cmd, backend->indexBuf.get(), 0, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(cmd, indexCount, instanceCount, 0, 0, 0);

  vkCmdEndRendering(cmd);
  vkEndCommandBuffer(cmd);

  auto const fence = backend->window->fence.get();
  if (!submitDrawCalls(backend, img)) {
    addErrMsg(backend, "render: Failed to submit draw calls");
    return false;
  }

  r = vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, "render: Failed to wait for fence", r);
    return false;
  }

  r = vkResetFences(dev, 1, &fence);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, "render: Failed to reset fence", r);
    return false;
  }

  if (!present(backend, img)) {
    addErrMsg(backend, "render: Failed to present image");
    return false;
  }

  return true;
}

bool setupRenderingInfo(VulkanBackend *const backend,
                        VkRenderingInfo *const renderingInfo,
                        VkRenderingAttachmentInfo *const color,
                        VkRenderingAttachmentInfo *const depth,
                        uint32_t const img) {
  if (!backend)
    return false;

  if (!renderingInfo) {
    addErrMsg(backend,
              "setupRenderingInfo: The parameter 'renderingInfo' = nullptr");
    return false;
  }

  if (!color) {
    addErrMsg(backend, "setupRenderingInfo: The parameter 'color' = nullptr");
    return false;
  }

  if (!depth) {
    addErrMsg(backend, "setupRenderingInfo: The parameter 'depth' = nullptr");
    return false;
  }

  *color = VkRenderingAttachmentInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = 0,
      .imageView = backend->window->swapImgViews[img].get(),
      .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .resolveMode = {},
      .resolveImageView = {},
      .resolveImageLayout = {},
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.color = VkClearColorValue{.float32{0.f, 0.f, 0.f, 1.f}}}};

  *depth = VkRenderingAttachmentInfo{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = 0,
      .imageView = backend->window->depthImgView.get(),
      .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .resolveMode = {},
      .resolveImageView = {},
      .resolveImageLayout = {},
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.depthStencil =
                         VkClearDepthStencilValue{.depth = 1.f, .stencil = 0}}};

  renderingInfo->sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo->renderArea = {
      {0, 0}, {backend->window->width, backend->window->height}};
  renderingInfo->layerCount = 1;
  renderingInfo->colorAttachmentCount = 1;
  renderingInfo->pColorAttachments = color;
  renderingInfo->pDepthAttachment = depth;
  return true;
}

bool getNextImage(VulkanBackend *const backend, uint32_t *const imgIndex) {
  if (!backend)
    return false;

  auto const acquireDone = backend->window->acquireSem.get();
  auto const swp = backend->window->swapchain.get();
  auto const dev = backend->device->handle.get();

  uint32_t img{};
  auto r = vkAcquireNextImageKHR(dev, swp, UINT64_MAX, acquireDone, 0, &img);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, "getNextImage: Fetching swapchain image failed", r);
    return false;
  }

  *imgIndex = img;
  return true;
}

bool setRenderBarriers(VulkanBackend *const backend, VkImage const image) {
  if (!backend)
    return false;

  VkCommandBuffer const cmd = backend->window->graphicsBuf;
  VkImageMemoryBarrier2 barrier{}, barrier2{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  barrier.image = image;
  barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
  barrier2 = barrier;
  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  depInfo.imageMemoryBarrierCount = 1;
  VkImageMemoryBarrier2 barriers[] = {barrier, barrier2};
  depInfo.pImageMemoryBarriers = barriers;

  barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.srcAccessMask = VK_ACCESS_2_NONE;
  barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

  barrier2.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  barrier2.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  barrier2.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier2.dstAccessMask = VK_ACCESS_2_NONE;
  barrier2.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
  barrier2.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  vkCmdPipelineBarrier2(cmd, &depInfo);
  return true;
}

bool submitDrawCalls(VulkanBackend *const backend, uint32_t const imageIndex) {
  if (!backend)
    return false;

  auto const renderDone = backend->window->renderSem[imageIndex].get();
  auto const acquireDone = backend->window->acquireSem.get();
  auto const fence = backend->window->fence.get();

  VkSubmitInfo2 submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

  submitInfo.commandBufferInfoCount = 1;
  VkCommandBufferSubmitInfo cbsi{};
  cbsi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cbsi.commandBuffer = backend->window->graphicsBuf;
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

  auto r = vkQueueSubmit2(backend->device->graphics, 1, &submitInfo, fence);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, "submitDrawCalls: Failed to submit commands", r);
    return false;
  }

  return true;
}

bool present(VulkanBackend *const backend, uint32_t const imageIndex) {
  if (!backend)
    return false;

  auto const renderDone = backend->window->renderSem[imageIndex].get();
  auto const swp = backend->window->swapchain.get();

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderDone;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swp;
  presentInfo.pImageIndices = &imageIndex;

  auto r = vkQueuePresentKHR(backend->device->present, &presentInfo);
  if (r != VK_SUCCESS) {
    addErrMsg(backend, "present: Failed to queue presentation", r);
    return false;
  }

  return true;
}

void storeMissingInstanceExts(std::vector<std::string> const *const requested,
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
}

bool createInstance(VulkanBackend *const backend) {
  VkInstanceCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

  VkApplicationInfo app{};
  app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app.pApplicationName = backend->appName.c_str();
  app.apiVersion = VulkanBackend::VULKAN_API_VERSION;
  info.pApplicationInfo = &app;

  info.ppEnabledExtensionNames =
      glfwGetRequiredInstanceExtensions(&info.enabledExtensionCount);

  backend->instance = std::make_unique<Instance>();
  auto &requestedExts = backend->instance->requestedExts;
  auto &missingReqExts = backend->instance->missingReqExts;

  for (uint32_t i = 0; i < info.enabledExtensionCount; ++i)
    requestedExts.push_back(info.ppEnabledExtensionNames[i]);

  storeMissingInstanceExts(&requestedExts, &missingReqExts);

  char const *validation[] = {"VK_LAYER_KHRONOS_validation"};
  info.ppEnabledLayerNames = backend->validation ? validation : nullptr;
  info.enabledLayerCount = backend->validation ? 1 : 0;

  VkInstance handle{};
  auto result = vkCreateInstance(&info, nullptr, &handle);
  if (result != VK_SUCCESS) {
    addErrMsg(backend, "Failed to create vulkan instance", result);
    return false;
  }

  backend->instance->handle = {
      handle, [](VkInstance ptr) { vkDestroyInstance(ptr, nullptr); }};

  return true;
}

bool createShaderModule(VulkanBackend *const backend,
                        std::string const &spirvFile,
                        UniqueRes<VkShaderModule_T> *const out) {
  if (!backend)
    return false;

  if (!out) {
    addErrMsg(backend, "createShaderModule: The parameter 'out' = nullptr");
    return false;
  }

  std::ifstream in{spirvFile, std::ios::ate | std::ios::binary};
  std::vector<uint32_t> byteCode{};

  if (!in.is_open()) {
    addErrMsg(
        backend,
        "createShaderModule: Failed to open shader module byte code file: " +
            spirvFile);
    return false;
  }

  VkDevice device{};
  if (!getLogicalDevice(backend, &device)) {
    addErrMsg(backend, "createShaderModule: Failed to get logical device");
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
  auto result = vkCreateShaderModule(device, &info, 0, &shader);

  if (result != VK_SUCCESS) {
    addErrMsg(
        backend, "createShaderModule: Failed to create shader module", result);
    return false;
  }

  *out = {shader, [device](VkShaderModule_T *const p) {
            vkDestroyShaderModule(device, p, 0);
          }};

  return true;
}
} // namespace re
