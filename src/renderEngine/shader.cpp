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

#include "shader.hpp"
#include "engine.hpp"
#include "device.hpp"
#include <fstream>

namespace re {
Result createShaderModule(RenderEngineT *const engine,
                          std::string const &spirvFile,
                          UniqueShader *const out) {
  std::ifstream in{spirvFile, std::ios::ate | std::ios::binary};
  std::vector<uint32_t> byteCode{};

  if (!in.is_open()) {
    setErrMsg(engine,
              "Failed to open shader module byte code file: " + spirvFile);
    return Result::ErrorVulkanShaderModuleCreationFailure;
  }

  std::size_t const dataSize = in.tellg(); // In bytes
  byteCode.resize(dataSize / sizeof(uint32_t));
  in.seekg(0);
  in.read(reinterpret_cast<char *>(byteCode.data()), dataSize);

  VkShaderModuleCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  info.codeSize = dataSize;
  info.pCode = byteCode.data();
  auto const dev = engine->device->handle.get();
  VkShaderModule shader{};
  auto result = vkCreateShaderModule(dev, &info, 0, &shader);

  if (result != VK_SUCCESS) {
    setErrMsg(engine, "Failed to create shader module", result);
    return Result::ErrorVulkanShaderModuleCreationFailure;
  }

  *out = {shader, [dev](VkShaderModule_T *const p) {
            vkDestroyShaderModule(dev, p, 0);
          }};

  return Result::Success;
}
} // namespace re
