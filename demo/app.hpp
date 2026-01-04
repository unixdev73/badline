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

#include <glm/gtc/matrix_transform.hpp>
#include <badline/renderEngine.hpp>
#include <badline/argParser.hpp>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <functional>
#include <memory>
#include <vector>

using SmartArgParser =
    std::unique_ptr<ap::ArgParser, std::function<void(ap::ArgParser *const)>>;

using SmartRenderEngine =
    std::unique_ptr<re::RenderEngine,
                    std::function<void(re::RenderEngine *const)>>;

namespace demo {
class raiiGLFW {
public:
  raiiGLFW(raiiGLFW const &) = delete;
  raiiGLFW(raiiGLFW &&) = delete;
  raiiGLFW() {
    if (!glfwInit())
      throw std::runtime_error{"Failed to init GLFW"};
  }
  ~raiiGLFW() { glfwTerminate(); }

private:
};

struct App {
  std::size_t argc{};
  char const *const *argv{};
  uint32_t windowWidth{}, windowHeight{};

  SmartArgParser parser{0, 0};
  SmartRenderEngine engine{0, 0};
  re::Window *win{};

  glm::mat4 proj{1.f}, view{1.f};
  std::vector<glm::mat4> instanceTransforms{};
};

bool initialize(App *const);
bool initializeArgParser(App *const);
bool extractWindowArgs(App *const);
bool openWindow(App *const, re::VulkanBackend *const vk);
bool createScene(App *const);
bool run(App *const);
} // namespace demo
