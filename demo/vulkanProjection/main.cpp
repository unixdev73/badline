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

#include <GLFW/glfw3.h>
#include <array>
#include <badline/renderEngine.hpp>
#include <badline/vulkanBackend.hpp>
#include <chrono>
#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <list>
#include <memory>
#include <random>

namespace demo {
template <typename T>
using CustomUniqPtr = std::unique_ptr<T, std::function<void(T *const)>>;

enum class Direction { Left, Right, Up, Down };

struct AppData {
  CustomUniqPtr<void> glfw{};
  CustomUniqPtr<re::RenderEngine> engineRAII{};
  int windowWidth{640};
  int windowHeight{480};

  re::RenderEngine *engine{};
  re::VulkanBackend *backend{};
  re::VulkanWindow *window{};
  GLFWwindow *glfwWin{};

  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};

  std::size_t cubeSide{32};
  re::Object *cube{};
  glm::mat4 model{1.f};
};

bool initialize(AppData *const);
bool run(AppData *const);
} // namespace demo

int main(int, char **) {
  demo::AppData data{};

  try {
    if (!demo::initialize(&data))
      return 1;
    if (!demo::run(&data))
      return 1;

  } catch (std::exception const &e) {
    std::cerr << "Error: Caught exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Error: Caught unknown exception" << std::endl;
  }

  return 0;
}

namespace demo {
bool createCube(AppData *const a, re::Texture *const tex,
                re::Object **const cube) {
  if (!re::createObject(a->backend, tex, cube)) {
    std::cerr << "Failed to create cube" << std::endl;
    return false;
  }

  float const z{0.1001f};
  re::addVertex(*cube, {-0.5f, 0.5f, z}, {0.0f, 0.0f}, {}, {1.f, 0, 0, 1.f});
  re::addVertex(*cube, {0.5f, 0.5f, z}, {1.0f, 0.0f}, {}, {1.f, 0, 0, 1.f});
  re::addVertex(*cube, {0.5f, -0.5f, z}, {1.0f, 1.0f}, {}, {1.f, 0, 0, 1.f});
  re::addVertex(*cube, {-0.5f, -0.5f, z}, {0.0f, 1.0f}, {}, {1.f, 0, 0, 1.f});

  std::vector<uint32_t> cubeIndices = {0, 1, 2, 2, 3, 0};

  re::setIndices(*cube, std::move(cubeIndices));
  re::uploadObjectDataToGPU(*cube);
  return true;
}

bool initializeScene(AppData *const a) {
  re::setClearColor(a->backend, 0.4f, 0.0f, 0.6f);
  glfwSetWindowPos(a->glfwWin, 0, 0);

  float l{0.f}, b{0.f}, n{0.1f};
  float r{640.f}, t{480.f}, f{1000.f};
  a->projection = glm::mat4{1};
  a->projection[0][0] = 2 * n / (r - l);
  a->projection[1][1] = 2 * n / (b - t);
  a->projection[2][2] = f / (f - n);
  a->projection[2][0] = -(r + l) / (r - l);
  a->projection[2][1] = -(b + t) / (b - t);
  a->projection[2][3] = 1;
  a->projection[3][2] = -f * n / (f - n);
  a->projection[3][3] = 0;

  a->view = glm::mat4{1};
  a->model = glm::scale(glm::translate(glm::mat4{1}, glm::vec3{0.5, -0.5, 0}),
                        glm::vec3{100.f, 100.f, 1.});

  re::setCameraProjection(a->backend, a->projection);
  re::setCameraView(a->backend, a->view);

  return createCube(a, 0, &a->cube);
}

bool run(AppData *const a) {
  static auto frame = std::chrono::steady_clock::now();
  static float dist = -0.001;
  while (!glfwWindowShouldClose(a->glfwWin)) {
    glfwPollEvents();
    if (glfwGetKey(a->glfwWin, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      break;

    auto now = std::chrono::steady_clock::now();
    if (now - frame >= std::chrono::milliseconds(17)) {
      frame = now;
      if (glfwGetKey(a->glfwWin, GLFW_KEY_B) == GLFW_PRESS) {
        a->view =
            glm::inverse(glm::translate(glm::mat4{1}, glm::vec3{0, 0, dist}));
        dist -= 0.001;
        re::setCameraView(a->backend, a->view);
      }
    }

    if (!re::stage(a->backend, a->cube, a->model))
      return false;

    if (!re::render(a->backend, a->window))
      return false;
  }

  return true;
}

CustomUniqPtr<re::RenderEngine> createRenderEngine() {
  re::RenderEngine *handle{};
  re::create(&handle);
  return {handle, re::destroy};
}

bool initializeGLFW(AppData *const data) {
  if (!glfwInit()) {
    std::cerr << "Error: Failed to initialize GLFW" << std::endl;
    return false;
  }
  data->glfw = {malloc(1), [](auto *p) {
                  free(p);
                  glfwTerminate();
                }};
  return true;
}

bool initializeEngine(AppData *const data) {
  data->engineRAII = demo::createRenderEngine();
  if (!data->engineRAII) {
    std::cerr << "Error: Failed to create render engine" << std::endl;
    return false;
  }
  data->engine = data->engineRAII.get();

  re::enableInfoLogs(data->engine);
  re::enableWarnings(data->engine);

  if (!re::createBackend(data->engine, &data->backend))
    return false;

  if (!re::enableValidationLayers(data->backend))
    return false;

  if (!re::initialize(data->backend))
    return false;

  if (!re::createWindow(data->backend, data->windowWidth, data->windowHeight,
                        "Demo", &data->window))
    return false;

  if (!re::getWindowHandle(data->window, &data->glfwWin))
    return false;

  return true;
}

bool initialize(AppData *const data) {
  if (!initializeGLFW(data))
    return false;

  if (!initializeEngine(data))
    return false;

  if (!initializeScene(data)) {
    std::cerr << "Failed to initialize scene" << std::endl;
    return false;
  }

  return true;
}
} // namespace demo
