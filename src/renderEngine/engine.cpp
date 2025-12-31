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

#include "engine.hpp"
#include "device.hpp"
#include "window.hpp"
#include "vkresult.hpp"

namespace re {
Result present(RenderEngineT *const engine, uint32_t const imageIndex) {
  auto const renderDone = engine->window->renderSem[imageIndex].get();
  auto const swp = engine->window->swapchain.get();

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderDone;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swp;
  presentInfo.pImageIndices = &imageIndex;
  vkQueuePresentKHR(engine->device->present, &presentInfo);

  return Result::Success;
}

Result submitDrawCalls(RenderEngineT *const engine, uint32_t const imageIndex) {
  auto const renderDone = engine->window->renderSem[imageIndex].get();
  auto const acquireDone = engine->window->acquireSem.get();
  auto const fence = engine->window->fence.get();

  VkSubmitInfo2 submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;

  submitInfo.commandBufferInfoCount = 1;
  VkCommandBufferSubmitInfo cbsi{};
  cbsi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cbsi.commandBuffer = engine->window->graphicsBuf;
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
  vkQueueSubmit2(engine->device->graphics, 1, &submitInfo, fence);

  return Result::Success;
}

Result setRenderBarriers(RenderEngineT *const engine, VkImage const image) {
  VkCommandBuffer const cmd = engine->window->graphicsBuf;
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
  return Result::Success;
}

Result getNextImage(RenderEngineT *const engine, uint32_t *const imgIndex) {
  auto const acquireDone = engine->window->acquireSem.get();
  auto const swp = engine->window->swapchain.get();
  auto const dev = engine->device->handle.get();

  uint32_t img{};
  auto r = vkAcquireNextImageKHR(dev, swp, UINT64_MAX, acquireDone, 0, &img);
  if (r != VK_SUCCESS) {
    setErrMsg(engine, "Fetching swapchain image failed", r);
    return Result::ErrorSwapchainImageAcquisitionFailure;
  }

  *imgIndex = img;
  return Result::Success;
}

Result render(RenderEngineT *const engine) {
  VkCommandBuffer const cmd = engine->window->graphicsBuf;
  auto const dev = engine->device->handle.get();

  uint32_t img{};
  if (auto r = getNextImage(engine, &img); r != Result::Success)
    return r;

  VkCommandBufferBeginInfo cbi{};
  cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &cbi);

  setRenderBarriers(engine, engine->window->swapImages[img]);

  VkRenderingAttachmentInfo const colorAttachment{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = 0,
      .imageView = engine->window->swapImgViews[img].get(),
      .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .resolveMode = {},
      .resolveImageView = {},
      .resolveImageLayout = {},
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.color = VkClearColorValue{.float32{0.f, 0.f, 0.f, 1.f}}}};

  VkRenderingAttachmentInfo const depthAttachment{
      .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext = 0,
      .imageView = engine->window->depthImgView.get(),
      .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
      .resolveMode = {},
      .resolveImageView = {},
      .resolveImageLayout = {},
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .clearValue = {.depthStencil =
                         VkClearDepthStencilValue{.depth = 1.f, .stencil = 0}}};

  VkRenderingInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo.renderArea = {{0, 0},
                              {engine->window->width, engine->window->height}};
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachment;
  renderingInfo.pDepthAttachment = &depthAttachment;

  vkCmdBeginRendering(cmd, &renderingInfo);

  VkViewport const vp{.x = 0,
                      .y = 0,
                      .width = float(engine->window->width),
                      .height = float(engine->window->height),
                      .minDepth = 0.f,
                      .maxDepth = 1.f};
  vkCmdSetViewportWithCount(cmd, 1, &vp);

  VkRect2D const sc{.offset = {0, 0},
                    .extent = {engine->window->width, engine->window->height}};
  vkCmdSetScissorWithCount(cmd, 1, &sc);
  auto const pipe =
      engine->device->pipelines[engine->window->activePipeline].get();
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
  auto const vertexBuf = engine->vertexBuf.get();
  VkDeviceSize const offsets[] = {0};
  vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuf, offsets);
  vkCmdDraw(cmd, engine->vertexBufSize / sizeof(Vertex), 1, 0, 0);

  vkCmdEndRendering(cmd);
  vkEndCommandBuffer(cmd);

  auto const fence = engine->window->fence.get();
  if (auto r = submitDrawCalls(engine, img); r != Result::Success)
    return r;

  vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  vkResetFences(dev, 1, &fence);

  return present(engine, img);
}

void setErrMsg(RenderEngineT *const e, std::string const &msg, VkResult r) {
  e->errorMessage = msg;
  std::string code{};

  if (r != VK_SUCCESS) {
    auto convRes = toString(r, &code);
    if (convRes != Result::Success)
      e->errorMessage += ", VkError code: " + std::to_string(r);
    else
      e->errorMessage += ", VkError code: " + code;
  }
}
} // namespace re
