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

#include "vulkanBackend.hpp"
#include "renderEngine.hpp"
#include <iostream>

namespace re {
bool getIndexCount(Indices const *const handle, unsigned long *const count) {
  *count = handle->indices.size();
  return true;
}

void printErrors(RenderEngine const *const handle) {
  if (!handle)
    return;

  if (!handle->logs.errors.size())
    return;

  std::cerr << "Error (" << handle->logs.errors.size() << "): ";
  std::size_t mlt = 0;
  for (std::size_t i = handle->logs.errors.size() - 1; i > 0; --i) {
    for (std::size_t j = 0; j < mlt; ++j)
      std::cerr << "  ";
    std::cerr << handle->logs.errors[i] << "\n";
    ++mlt;
  }
  for (std::size_t j = 0; j < mlt; ++j)
    std::cerr << "  ";
  std::cerr << handle->logs.errors[0] << "\n";
}

bool createWindow(RenderEngine *const handle, Window **const p) {
  handle->vulkanBackend->window = std::make_unique<Window>();
  *p = handle->vulkanBackend->window.get();
  return true;
}

void addErrMsg(RenderEngine const *const engine, std::string const &msg) {
  addErrMsg(&engine->logs, msg);
}

bool setMatrix(TransformMatrix *const handle, glm::mat4 const *const p) {
  handle->transform = *p;
  return true;
}

bool getMatrix(TransformMatrix const *const handle, glm::mat4 *const p) {
  *p = handle->transform;
  return true;
}

bool addInstance(Instances *const handle, unsigned long *const p) {
  unsigned long id = handle->transformData.size();
  handle->transformData.push_back(InstanceTransform{});
  *p = id;
  return true;
}

void addErrMsg(Instances const *const handle,
               std::string const &msg,
               VkResult r = VK_SUCCESS) {
  addErrMsg(handle->logs, msg, r);
}

bool setTransform(Instances *const handle,
                  unsigned long const instanceId,
                  glm::mat4 const *const transform) {
  if (!handle)
    return false;

  if (instanceId >= handle->transformData.size()) {
    addErrMsg(handle, "setTransform: instance id > instance count");
    return false;
  }

  if (!transform) {
    addErrMsg(handle, "setTransform: transform = nullptr");
    return false;
  }

  handle->transformData.at(instanceId).transform = *transform;
  return true;
}

bool getData(Instances const *const handle,
             void const **const data,
             unsigned long *const size) {
  *data = handle->transformData.data();
  *size = handle->transformData.size() * sizeof(InstanceTransform);
  return true;
}

bool getInstanceCount(Instances const *const handle,
                      unsigned long *const count) {
  *count = handle->transformData.size();
  return true;
}

bool addIndex(Indices *const handle, unsigned const p) {
  handle->indices.push_back(p);
  return true;
}

bool getData(Indices const *const handle,
             void const **const data,
             unsigned long *const size) {
  *data = handle->indices.data();
  *size = handle->indices.size() * sizeof(handle->indices.front());
  return true;
}

bool addVertex(Vertices *const handle, Vertex **const p) {
  handle->vertices.push_back(Vertex{});
  *p = &handle->vertices.back();
  return true;
}

bool getData(Vertices const *const handle,
             void const **const data,
             unsigned long *const size) {
  *data = handle->vertices.data();
  *size = handle->vertices.size() * sizeof(handle->vertices.front());
  return true;
}

bool uploadInstances(VulkanBackend *const backend,
                     Instances const *const instances);

bool uploadIndices(VulkanBackend *const backend, Indices const *const indices);

bool uploadVertices(VulkanBackend *const backend,
                    Vertices const *const vertices);

bool uploadVertices(RenderEngine *const handle) {
  return uploadVertices(handle->vulkanBackend.get(), handle->vertices.get());
}

bool uploadIndices(RenderEngine *const handle) {
  return uploadIndices(handle->vulkanBackend.get(), handle->indices.get());
}

bool uploadInstances(RenderEngine *const handle) {
  return uploadInstances(handle->vulkanBackend.get(), handle->instances.get());
}

bool initialize(RenderEngine *const engine) {
  if (!engine)
    return false;

  if (!engine->vulkanBackend) {
    addErrMsg(engine, "initialize: The backend was not created");
    return false;
  }

  if (!createInstance(engine->vulkanBackend.get())) {
    addErrMsg(engine, "initialize: Failed to create vulkan instance");
    return false;
  }

  if (!createOptimalGPU(engine->vulkanBackend.get())) {
    addErrMsg(engine, "initialize: Failed to create logical device");
    return false;
  }

  return true;
}

bool render(RenderEngine *const engine) {
  if (!engine)
    return false;

  if (!engine->vulkanBackend || !engine->backendInitialized) {
    addErrMsg(engine, "render: The backend is not initialized");
    return false;
  }

  if (!engine->vulkanBackend->window) {
    addErrMsg(engine, "render: The window is not created");
    return false;
  }

  if (!engine->vertices) {
    addErrMsg(engine, "render: No vertices set");
    return false;
  }

  if (!engine->indices) {
    addErrMsg(engine, "render: No indices set");
    return false;
  }

  if (!engine->instances) {
    addErrMsg(engine, "render: No instances set");
    return false;
  }

  unsigned long instanceCount{1};
  if (!getInstanceCount(engine->instances.get(), &instanceCount)) {
    addErrMsg(engine, "render: Failed to get instance count");
    return false;
  }

  unsigned long indexCount{36};
  if (!getIndexCount(engine->indices.get(), &indexCount)) {
    addErrMsg(engine, "render: Failed to get index count");
    return false;
  }

  if (!render(engine->vulkanBackend.get(), indexCount, instanceCount)) {
    addErrMsg(engine, "render: Rendering failed");
    return false;
  }

  return true;
}

bool createVertices(RenderEngine *const handle, Vertices **const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle, "createVertices: The parameter 'p' = nullptr");
    return false;
  }

  handle->vertices = std::make_unique<Vertices>(&handle->logs);
  *p = handle->vertices.get();
  return true;
}

bool createIndices(RenderEngine *const handle, Indices **const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle, "createIndices: The parameter 'p' = nullptr");
    return false;
  }

  handle->indices = std::make_unique<Indices>(&handle->logs);
  *p = handle->indices.get();
  return true;
}

bool createInstances(RenderEngine *const handle, Instances **const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle, "createInstances: The parameter 'p' = nullptr");
    return false;
  }

  handle->instances = std::make_unique<Instances>(&handle->logs);
  *p = handle->instances.get();
  return true;
}

void create(RenderEngine **const handle) {
  if (!handle)
    return;

  *handle = new RenderEngine{};
}

void destroy(RenderEngine *const handle) { delete handle; }

bool createBackend(RenderEngine *const handle, VulkanBackend **const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle, "createBackend: The vulkan backend parameter = nullptr");
    return false;
  }

  handle->vulkanBackend = std::make_unique<VulkanBackend>(&handle->logs);
  if (!handle->vulkanBackend) {
    addErrMsg(handle, "createBackend: Failed to create vulkan backend");
    return false;
  }

  *p = handle->vulkanBackend.get();
  handle->backendInitialized = true;
  return true;
}

bool createProjection(RenderEngine *const handle, TransformMatrix **const m) {
  if (!handle)
    return false;

  if (!m) {
    addErrMsg(handle, "createProjection: The parameter 'm' = nullptr");
    return false;
  }

  handle->projection = std::make_unique<TransformMatrix>(&handle->logs);
  *m = handle->projection.get();
  return true;
}

bool setProjection(RenderEngine *const handle, TransformMatrix const *const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle,
              "setProjection: The transform matrix parameter = nullptr");
    return false;
  }

  if (!getMatrix(p, &handle->cam.projection)) {
    addErrMsg(handle,
              "setProjection: The transform matrix parameter = nullptr");
    return false;
  }

  if (!handle->vulkanBackend) {
    addErrMsg(handle, "setProjection: The backend = nullptr");
    return false;
  }

  getMatrix(p, &handle->cam.projection);
  handle->vulkanBackend->camMats.camProj = handle->cam.projection;
  return true;
}

bool createView(RenderEngine *const handle, TransformMatrix **const m) {
  if (!handle)
    return false;

  if (!m) {
    addErrMsg(handle, "createView: The parameter 'm' = nullptr");
    return false;
  }

  handle->view = std::make_unique<TransformMatrix>(&handle->logs);
  *m = handle->view.get();
  return true;
}

bool setView(RenderEngine *const handle, TransformMatrix const *const p) {
  if (!handle)
    return false;

  if (!p) {
    addErrMsg(handle, "setView: The transform matrix parameter = nullptr");
    return false;
  }

  if (!getMatrix(p, &handle->cam.view)) {
    addErrMsg(handle, "setView: The transform matrix parameter = nullptr");
    return false;
  }

  if (!handle->vulkanBackend) {
    addErrMsg(handle, "setProjection: The backend = nullptr");
    return false;
  }

  getMatrix(p, &handle->cam.view);
  handle->vulkanBackend->camMats.camView = handle->cam.view;
  return true;
}

bool getErrorMessages(RenderEngine const *const handle,
                      char const *const **const messages,
                      unsigned long *const size) {
  if (!handle)
    return false;

  if (!messages) {
    addErrMsg(handle, "getErrorMessages: The parameter 'messages' = nullptr");
    return false;
  }

  if (!size) {
    addErrMsg(handle, "getErrorMessages: The parameter 'size' = nullptr");
    return false;
  }

  if (handle->logs.errors.empty()) {
    *messages = nullptr;
    *size = 0;
    return true;
  }

  *messages = handle->logs.errptr.data();
  *size = handle->logs.errptr.size();
  return true;
}
} // namespace re
