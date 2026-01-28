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

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <functional>
#include <vector>
#include <string>
#include <memory>

#pragma once

struct GLFWwindow;

namespace re {
struct VulkanBackend;
struct VulkanWindow;
struct Texture;
struct Object;

bool enableValidationLayers(VulkanBackend *const handle);

bool setPreferredGPU(VulkanBackend *const handle, char const *const p);

bool setApplicationName(VulkanBackend *const handle, char const *const p);

/* Do not call any of the functions below until this one has been called */
bool initialize(VulkanBackend *const handle);

bool createWindow(VulkanBackend *const handle,
                  uint32_t const width,
                  uint32_t const height,
                  char const *const title,
                  VulkanWindow **const window);

bool getWindowHandle(VulkanWindow const *const handle,
                     GLFWwindow **const window);

bool setCameraProjection(VulkanBackend *const handle, glm::mat4 const &m);

bool setCameraView(VulkanBackend *const handle, glm::mat4 const &m);

bool createObject(VulkanBackend *const handle,
                  Texture const *const t,
                  Object **const object);

bool addVertex(Object *const handle,
               glm::vec3 const &position,
               glm::vec2 const &texCoord,
               glm::vec3 const &normal,
               glm::vec4 const &color);

bool setIndices(Object *const handle, std::vector<uint32_t> indices);

bool uploadObjectDataToGPU(Object *const handle);

bool loadFromFile(Object *const handle, std::string const &path);

bool createTexture(VulkanBackend *const handle, Texture **const object);

bool loadFromFile(Texture *const handle, std::string const &path);

template <typename T>
using CustomUniqPtr = std::unique_ptr<T, std::function<void(T *const)>>;

bool setClearColor(VulkanBackend *const handle,
                   float const r,
                   float const g,
                   float const b);

bool stage(VulkanBackend *const handle,
           Object const *const object,
           glm::mat4 const &transform);

bool render(VulkanBackend *const handle, VulkanWindow *const target);
} // namespace re
