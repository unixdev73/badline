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
Result render(RenderEngineT *const engine) {
  auto dev = engine->device->handle.get();
  auto swp = engine->window->swapchain.get();
  auto presentDone = engine->window->presentSem.get();
  auto fence = engine->fence.get();

  static std::vector<bool> undef(engine->window->swapImages.size(), true);

  uint32_t img{};
  auto r = vkAcquireNextImageKHR(
      dev, swp, UINT64_MAX, presentDone, VK_NULL_HANDLE, &img);
  if (r == VK_NOT_READY)
    return Result::Success;
  if (r != VK_SUCCESS) {
    setErrMsg(engine, "Fetching swapchain image failed", r);
    return Result::ErrorSwapchainImageAcquisitionFailure;
  }

  auto renderDone = engine->window->renderSem[img].get();

  VkCommandBuffer cmd = engine->device->graphicsBuff;
  VkCommandBufferBeginInfo cbi{};
  cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(cmd, &cbi);

  // Transition: PRESENT -> COLOR_ATTACHMENT
  VkImageMemoryBarrier2 barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.srcAccessMask = 0;
  barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  if (undef[img]) {
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    undef[img] = false;
  } else
    barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
  barrier.image = engine->window->swapImages[img];
  barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  VkDependencyInfo depInfo{};
  depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  depInfo.imageMemoryBarrierCount = 1;
  depInfo.pImageMemoryBarriers = &barrier;
  vkCmdPipelineBarrier2(cmd, &depInfo);

  // Dynamic rendering
  VkRenderingAttachmentInfo colorAttachment{};
  colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAttachment.imageView = engine->window->swapImgViews[img].get();
  colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.clearValue.color = {{1.f, 1.f, 0.f, 1.f}};

  VkRenderingInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo.renderArea = {{0, 0},
                              {engine->window->width, engine->window->height}};
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachment;

  vkCmdBeginRendering(cmd, &renderingInfo);
  vkCmdEndRendering(cmd);

  // Transition: COLOR_ATTACHMENT -> PRESENT
  barrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
  barrier.dstAccessMask = 0;
  vkCmdPipelineBarrier2(cmd, &depInfo);
  vkEndCommandBuffer(cmd);

  // Submit
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &cmd;
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = &presentDone;
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = &renderDone;
  vkQueueSubmit(engine->device->graphics, 1, &submitInfo, fence);

  // Present
  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &renderDone;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swp;
  presentInfo.pImageIndices = &img;
  vkQueuePresentKHR(engine->device->present, &presentInfo);

  vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
  vkResetFences(dev, 1, &fence);
  return Result::Success;
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
