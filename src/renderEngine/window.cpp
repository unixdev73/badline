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
#include "engine.hpp"
#include "instance.hpp"
#include "device.hpp"
#include "window.hpp"
#include "shader.hpp"

namespace re {
Result
createGLFWindow(RenderEngineT *const engine, uint32_t width, uint32_t height) {
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

  GLFWwindow *win =
      glfwCreateWindow(width, height, engine->instance->title.c_str(), 0, 0);
  if (!win)
    return Result::ErrorGLFWindowCreationFailure;

  engine->window->handle = {win, [](GLFWwindow *const ptr) {
                              if (ptr)
                                glfwDestroyWindow(ptr);
                            }};
  engine->window->width = width;
  engine->window->height = height;

  return Result::Success;
}

Result createWindowSurface(RenderEngineT *const engine) {
  auto const win = engine->window->handle.get();
  VkSurfaceKHR surf{};
  if (auto result = glfwCreateWindowSurface(
          engine->instance->handle.get(), win, 0, &surf);
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create vulkan surface", result);
    return Result::ErrorVulkanSurfaceCreationFailure;
  }

  auto inst = engine->instance->handle.get();
  engine->window->surface = {surf, [inst](VkSurfaceKHR_T *const ptr) {
                               if (ptr)
                                 vkDestroySurfaceKHR(inst, ptr, 0);
                             }};

  return Result::Success;
}

Result queryPresentModes(RenderEngineT *const engine) {
  engine->window->presentModes.clear();
  uint32_t count{};
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          engine->device->identifier,
          engine->window->surface.get(),
          &count,
          nullptr);
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to query present modes", result);
    return Result::ErrorPresentModesQueryFailure;
  }

  engine->window->presentModes.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(
          engine->device->identifier,
          engine->window->surface.get(),
          &count,
          engine->window->presentModes.data());
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to fill present modes", result);
    return Result::ErrorPresentModesFillFailure;
  }

  return Result::Success;
}

Result checkRequestedPresentModeAvailable(RenderEngineT *const engine) {
  bool found = false;
  for (auto mode : engine->window->presentModes) {
    if (mode == engine->window->presentMode) {
      found = true;
      break;
    }
  }
  if (!found) {
    std::string mode = std::to_string(engine->window->presentMode);
    setErrMsg(engine, "The requested present mode: " + mode);
    return Result::ErrorRequestedPresentModeNotAvailable;
  }

  return Result::Success;
}

Result checkSurfaceEligibility(RenderEngineT *const engine) {
  uint32_t count{};
  if (auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
          engine->device->identifier,
          engine->window->surface.get(),
          &engine->window->surfaceCaps);
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to query surface capabilities", result);
    return Result::ErrorSurfaceCapabilitiesQueryFailure;
  }

  if (engine->window->width > engine->window->surfaceCaps.maxImageExtent.width)
    return Result::ErrorRequestedSurfaceWidthTooLarge;

  if (engine->window->height >
      engine->window->surfaceCaps.maxImageExtent.height)
    return Result::ErrorRequestedSurfaceHeightTooLarge;

  if (!(engine->window->surfaceCaps.supportedUsageFlags &
        VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT))
    return Result::ErrorColorAttachmentBitNotSupported;

  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          engine->device->identifier, engine->window->surface.get(), &count, 0);
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to query surface formats", result);
    return Result::ErrorSurfaceFormatQueryFailure;
  }

  engine->window->surfaceFormats.resize(count);
  if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(
          engine->device->identifier,
          engine->window->surface.get(),
          &count,
          engine->window->surfaceFormats.data());
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to fill surface formats", result);
    return Result::ErrorSurfaceFormatQueryFailure;
  }

  if ((1 == engine->window->surfaceFormats.size()) &&
      (VK_FORMAT_UNDEFINED == engine->window->surfaceFormats[0].format)) {
    engine->window->surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
    engine->window->surfaceFormat.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  } else if (engine->window->surfaceFormats.size()) {
    engine->window->surfaceFormat = engine->window->surfaceFormats[0];
  } else
    return Result::ErrorNoSurfaceFormatsAvailable;

  return Result::Success;
}

Result createWindowSwapchain(RenderEngineT *const engine) {
  auto numberOfImages = engine->window->surfaceCaps.minImageCount + 1;
  if ((engine->window->surfaceCaps.maxImageCount > 0) &&
      (numberOfImages > engine->window->surfaceCaps.maxImageCount)) {
    numberOfImages = engine->window->surfaceCaps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR swpInfo{};
  swpInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swpInfo.compositeAlpha =
      VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swpInfo.imageArrayLayers = 1;
  swpInfo.clipped = VK_TRUE;
  swpInfo.imageColorSpace = engine->window->surfaceFormat.colorSpace;
  swpInfo.imageFormat = engine->window->surfaceFormat.format;

  uint32_t const width = engine->window->width, height = engine->window->height;
  swpInfo.imageExtent = VkExtent2D{width, height};
  swpInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swpInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swpInfo.surface = engine->window->surface.get();
  swpInfo.presentMode = engine->window->presentMode;
  swpInfo.minImageCount = numberOfImages;
  swpInfo.preTransform =
      VkSurfaceTransformFlagBitsKHR::VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

  VkSwapchainKHR swapchain{};
  if (auto result = vkCreateSwapchainKHR(
          engine->device->handle.get(), &swpInfo, 0, &swapchain);
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create window swapchain", result);
    return Result::ErrorVulkanSwapchainCreationFailure;
  }

  auto dev = engine->device->handle.get();
  engine->window->swapchain = {swapchain, [dev](VkSwapchainKHR_T *const ptr) {
                                 vkDestroySwapchainKHR(dev, ptr, 0);
                               }};

  return Result::Success;
}

Result fetchSwapchainImages(RenderEngineT *const engine) {
  uint32_t count{};

  if (auto result = vkGetSwapchainImagesKHR(engine->device->handle.get(),
                                            engine->window->swapchain.get(),
                                            &count,
                                            0);
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to query swapchain image count", result);
    return Result::ErrorSwapchainImageQueryFailure;
  }

  engine->window->swapImages.resize(count);

  if (auto result = vkGetSwapchainImagesKHR(engine->device->handle.get(),
                                            engine->window->swapchain.get(),
                                            &count,
                                            engine->window->swapImages.data());
      result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to fetch swapchain images", result);
    return Result::ErrorSwapchainImageFillFailure;
  }

  return Result::Success;
}

Result transitionSwapchainImages(RenderEngineT *const engine) {
  VkCommandBuffer const cmd = engine->window->graphicsBuf;
  VkCommandBufferBeginInfo cbi{};
  cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &cbi);

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

  for (std::size_t i = 0; i < engine->window->swapImages.size(); ++i) {
    barrier.image = engine->window->swapImages[i];
    vkCmdPipelineBarrier2(cmd, &depInfo);
  }

  vkEndCommandBuffer(cmd);

  auto const fence = engine->window->fence.get();
  VkSubmitInfo2 submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

  submitInfo.commandBufferInfoCount = 1;
  VkCommandBufferSubmitInfo cbsi{};
  cbsi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cbsi.commandBuffer = engine->window->graphicsBuf;
  submitInfo.pCommandBufferInfos = &cbsi;

  vkQueueSubmit2(engine->device->graphics, 1, &submitInfo, fence);

  auto const dev = engine->device->handle.get();
  vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  vkResetFences(dev, 1, &fence);
  return Result::Success;
}

Result createWindowSemaphores(RenderEngineT *const engine) {
  auto const dev = engine->device->handle.get();
  VkSemaphoreCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  engine->window->renderSem.resize(engine->window->swapImages.size());
  std::vector<UniqueResource<VkSemaphore_T> *> sem = {
      &engine->window->acquireSem};
  for (auto &s : engine->window->renderSem)
    sem.push_back(&s);

  for (std::size_t i = 0; i < sem.size(); ++i) {
    VkSemaphore semaphore{};
    auto result = vkCreateSemaphore(dev, &info, 0, &semaphore);
    if (result != VK_SUCCESS) {
      setErrMsg(engine, "Failed to create render semaphore", result);
      return Result::ErrorVulkanSemaphoreCreationFailure;
    }
    *sem[i] = {semaphore, [dev](VkSemaphore_T *const p) {
                 vkDestroySemaphore(dev, p, 0);
               }};
  }

  return Result::Success;
}

Result createSwapchainImageViews(RenderEngineT *const engine) {
  auto const size = engine->window->swapImages.size();
  auto const dev = engine->device->handle.get();
  auto &views = engine->window->swapImgViews;
  views.resize(size);

  for (std::size_t i = 0; i < size; ++i) {
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
    info.image = engine->window->swapImages[i];
    info.components = VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_R,
                                         .g = VK_COMPONENT_SWIZZLE_G,
                                         .b = VK_COMPONENT_SWIZZLE_B,
                                         .a = VK_COMPONENT_SWIZZLE_A};

    info.format = engine->window->surfaceFormat.format;
    info.subresourceRange = VkImageSubresourceRange{
        .aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1};

    VkImageView view{};
    auto result = vkCreateImageView(dev, &info, 0, &view);
    if (result != VK_SUCCESS) {
      setErrMsg(engine, "Failed to create image view", result);
      return Result::ErrorVulkanImageViewCreationFailure;
    }

    views[i] = {
        view, [dev](VkImageView_T *const p) { vkDestroyImageView(dev, p, 0); }};
  }
  return Result::Success;
}

Result allocateCommandBuffers(RenderEngineT *const engine) {
  auto const dev = engine->device->handle.get();

  VkCommandBufferAllocateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  info.commandPool = engine->device->graphicsCmdPool.get();
  info.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  auto &buf = engine->window->graphicsBuf;
  info.commandBufferCount = 1;

  auto result = vkAllocateCommandBuffers(dev, &info, &buf);
  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to allocate cmd buffer", result);
    return Result::ErrorVulkanCommandBufferAllocationFailure;
  }

  return Result::Success;
}

Result createWindowFence(RenderEngineT *const engine) {
  VkFenceCreateInfo finf{};
  finf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VkFence f{};
  auto dev = engine->device->handle.get();
  auto r = vkCreateFence(engine->device->handle.get(), &finf, 0, &f);
  if (r != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create fence", r);
    return Result::ErrorVulkanFenceCreationFailure;
  }
  auto &fence = engine->window->fence;
  fence = {f, [dev](VkFence_T *const p) { vkDestroyFence(dev, p, 0); }};

  return Result::Success;
}

Result createPipelineLayout(RenderEngineT *const engine) {
  VkPipelineLayoutCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  VkDescriptorSetLayout layouts[] = {{}};
  info.pSetLayouts = layouts;
  info.setLayoutCount = 0;

  auto const dev = engine->device->handle.get();
  VkPipelineLayout handle{};
  auto result = vkCreatePipelineLayout(dev, &info, 0, &handle);

  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create pipeline layout", result);
    return Result::ErrorVulkanPipelineLayoutCreationFailure;
  }

  engine->window->activePipelineLayout = engine->device->pipelineLayouts.size();
  engine->device->pipelineLayouts.push_back(
      {handle, [dev](VkPipelineLayout_T *const p) {
         vkDestroyPipelineLayout(dev, p, 0);
       }});
  return Result::Success;
}

Result createGraphicsPipeline(RenderEngineT *const engine) {
  UniqueResource<VkShaderModule_T> vertex{}, fragment{};
  auto result = createShaderModule(engine, "shaders/vertex.spv", &vertex);
  if (result != Result::Success)
    return result;

  result = createShaderModule(engine, "shaders/fragment.spv", &fragment);
  if (result != Result::Success)
    return result;

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

  VkPipelineColorBlendStateCreateInfo colorBlendState{};
  colorBlendState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

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
      engine->device->pipelineLayouts[engine->window->activePipelineLayout]
          .get();
  info.basePipelineHandle = VK_NULL_HANDLE;
  info.basePipelineIndex = 0;

  auto const dev = engine->device->handle.get();
  VkPipeline handle{VK_NULL_HANDLE};
  auto r = vkCreateGraphicsPipelines(dev, 0, 1, &info, 0, &handle);
  if (r != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create graphics pipeline", r);
    return Result::ErrorVulkanPipelineCreationFailure;
  }

  engine->window->activePipeline = engine->device->pipelines.size();
  engine->device->pipelines.push_back(UniqueResource<VkPipeline_T>{
      handle, [dev](VkPipeline_T *const p) { vkDestroyPipeline(dev, p, 0); }});
  return Result::Success;
}

Result createDepthImage(
    RenderEngineT *const engine,
    std::unique_ptr<VkImage_T, std::function<void(VkImage_T *const)>> *const
        out);

Result createDepthAttachment(RenderEngineT *const engine) {
  std::unique_ptr<VkImage_T, std::function<void(VkImage_T *const)>> img{};
  if (auto r = createDepthImage(engine, &img); r != Result::Success)
    return r;

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

  auto const dev = engine->device->handle.get();
  VkImageView handle{};
  if (auto r = vkCreateImageView(dev, &info, 0, &handle); r != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create depth image view", r);
    return Result::ErrorDepthImageCreationFailure;
  }

  engine->window->depthImg = std::move(img);
  engine->window->depthImgView = {
      handle, [dev](VkImageView_T *const p) { vkDestroyImageView(dev, p, 0); }};

  return Result::Success;
}

Result createWindow(RenderEngineT *const engine,
                    uint32_t const width,
                    uint32_t const height) {

  engine->window = std::make_unique<WindowT>();
  if (auto r = createGLFWindow(engine, width, height); r != Result::Success)
    return r;

  if (auto r = createWindowSurface(engine); r != Result::Success)
    return r;

  if (auto r = queryPresentModes(engine); r != Result::Success)
    return r;

  if (auto r = checkRequestedPresentModeAvailable(engine); r != Result::Success)
    return r;

  if (auto r = checkSurfaceEligibility(engine); r != Result::Success)
    return r;

  if (auto r = createWindowFence(engine); r != Result::Success)
    return r;

  if (auto r = createWindowSwapchain(engine); r != Result::Success)
    return r;

  if (auto r = fetchSwapchainImages(engine); r != Result::Success)
    return r;

  if (auto r = createDepthAttachment(engine); r != Result::Success)
    return r;

  if (auto r = allocateCommandBuffers(engine); r != Result::Success)
    return r;

  if (auto r = transitionSwapchainImages(engine); r != Result::Success)
    return r;

  if (auto r = createSwapchainImageViews(engine); r != Result::Success)
    return r;

  if (auto r = createWindowSemaphores(engine); r != Result::Success)
    return r;

  if (auto r = createPipelineLayout(engine); r != Result::Success)
    return r;

  if (auto r = createGraphicsPipeline(engine); r != Result::Success)
    return r;

  return Result::Success;
}
} // namespace re
