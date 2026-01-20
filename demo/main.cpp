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

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <badline/vulkanBackend.hpp>
#include <badline/renderEngine.hpp>
#include <badline/argParser.hpp>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <chrono>
#include <array>

namespace demo {
template <typename T>
using CustomUniqPtr = std::unique_ptr<T, std::function<void(T *const)>>;

struct AppData {
  int argc{};
  char const *const *argv{};

  CustomUniqPtr<ap::ArgParser> parserRAII{};
  CustomUniqPtr<void> glfw{};
  CustomUniqPtr<re::RenderEngine> engineRAII{};

  ap::ArgParser *parser{};
  int windowWidth{640};
  int windowHeight{480};
  bool printHelp{};

  re::RenderEngine *engine{};
  re::VulkanBackend *backend{};
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};
  glm::mat4 vikingModel{1.f};
  glm::mat4 cubeModel{1.f};
  re::VulkanWindow *window{};
  GLFWwindow *glfwWin{};
  re::Texture *trump{};
  re::Object *cube{};
  re::Texture *vikingTex{};
  re::Object *vikingObj{};

  std::array<unsigned short, 100> frameTimes{};
  std::size_t frameIdx{};
};

bool initialize(AppData *const);
void updateCWD(AppData *const);
bool initializeArgParser(AppData *const);
bool extractIf(ap::ArgParser *const, std::string const &, std::string *const);
bool convert(std::string const &input, int *const output);
bool extractCLI(AppData *const);
CustomUniqPtr<ap::ArgParser> createArgParser();
bool initializeGLFW(AppData *const);
bool initializeEngine(AppData *const);
CustomUniqPtr<re::RenderEngine> createRenderEngine();
bool loadAssets(AppData *const);
bool isFlag(ap::ArgParser *const, std::string const &flag);
void printHelpMsg();
bool handleInput(AppData *const, bool *const);
bool createCube(AppData *const a);
bool run(AppData *const);
} // namespace demo

int main(int const argc, char const *const *const argv) {
  demo::AppData data{.argc = argc, .argv = argv};

  try {
    if (!demo::initialize(&data))
      return 1;
    if (!data.printHelp && !demo::run(&data))
      return 1;

  } catch (std::exception const &e) {
    std::cerr << "Error: Caught exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Error: Caught unknown exception" << std::endl;
  }

  return 0;
}

namespace demo {
bool run(AppData *const a) {
  a->projection = glm::perspective(
      45.f, float(a->windowWidth) / float(a->windowHeight), 0.1f, 100.f);
  re::setCameraProjection(a->backend, a->projection);

  a->view =
      glm::lookAt(glm::vec3{0, 4.f, 0.1f}, glm::vec3{}, glm::vec3{0, 1.f, 0});
  re::setCameraView(a->backend, a->view);

  if (!createCube(a))
    return false;
  float const cubeSc = 0.5f;
  a->cubeModel = glm::scale(glm::mat4{1.f}, glm::vec3{cubeSc, cubeSc, cubeSc});
  a->cubeModel = glm::translate(a->cubeModel, glm::vec3{0, 0, 2.5f});

  float const vkSc = 2.f;
  a->vikingModel = glm::scale(a->vikingModel, glm::vec3{vkSc, vkSc, vkSc});
  a->vikingModel = glm::translate(a->vikingModel, glm::vec3{0, 0, -0.5f});

  auto const minTime = std::chrono::milliseconds(17); // ~60 FPS
  auto beginFrame = std::chrono::steady_clock::now();
  while (!glfwWindowShouldClose(a->glfwWin)) {
    glfwPollEvents();
    bool quit = false;

    namespace ch = std::chrono;
    auto const diff = ch::duration_cast<ch::milliseconds>(
        ch::steady_clock::now() - beginFrame);

    a->frameTimes[a->frameIdx] = diff.count();
    a->frameIdx = (++a->frameIdx) % a->frameTimes.size();

    if (diff >= minTime) {
      if (!handleInput(a, &quit))
        return false;
      if (quit)
        break;

      if (!re::stage(a->backend, a->vikingObj, a->vikingModel)) {
        re::printLogs(a->engine);
        return false;
      }

      if (!re::stage(a->backend, a->cube, a->cubeModel)) {
        re::printLogs(a->engine);
        return false;
      }

      if (!re::render(a->backend, a->window)) {
        re::printLogs(a->engine);
        return false;
      }

      beginFrame = std::chrono::steady_clock::now();
    }
  }

  float sum = 0.f;
  for (auto const &e : a->frameTimes)
    sum += static_cast<float>(e);
  auto const avgFrameTime = sum / a->frameTimes.size();
  std::cout << "AVG: " << avgFrameTime << " MS/F = ";
  std::cout << (1.0 / avgFrameTime) * 1000.0 << " FPS" << std::endl;
  return true;
}

bool createCube(AppData *const a) {
  if (!re::createObject(a->backend, a->trump, &a->cube)) {
    std::cerr << "Failed to create cube" << std::endl;
    return false;
  }

  // --- Front face (Z = 0.5) ---
  re::addVertex(a->cube, {-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}, {}, {});
  re::addVertex(a->cube, {-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}, {}, {});

  // --- Back face (Z = -0.5) ---
  re::addVertex(a->cube, {0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}, {}, {});

  // --- Left face (X = -0.5) ---
  re::addVertex(a->cube, {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f}, {}, {});
  re::addVertex(a->cube, {-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}, {}, {});

  // --- Right face (X = 0.5) ---
  re::addVertex(a->cube, {0.5f, -0.5f, 0.5f}, {1.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}, {}, {});

  // --- Top face (Y = 0.5) ---
  re::addVertex(a->cube, {-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, 0.5f, -0.5f}, {0.0f, 1.0f}, {}, {});
  re::addVertex(a->cube, {-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f}, {}, {});

  // --- Bottom face (Y = -0.5) ---
  re::addVertex(a->cube, {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}, {}, {});
  re::addVertex(a->cube, {0.5f, -0.5f, 0.5f}, {0.0f, 1.0f}, {}, {});
  re::addVertex(a->cube, {-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f}, {}, {});

  std::vector<uint32_t> cubeIndices;

  for (uint32_t i = 0; i < 6; ++i) {
    uint32_t start = i * 4;
    cubeIndices.push_back(start + 0);
    cubeIndices.push_back(start + 1);
    cubeIndices.push_back(start + 2);
    cubeIndices.push_back(start + 2);
    cubeIndices.push_back(start + 3);
    cubeIndices.push_back(start + 0);
  }

  re::setIndices(a->cube, std::move(cubeIndices));
  re::uploadObjectDataToGPU(a->cube);
  return true;
}

bool handleInput(AppData *const a, bool *const quit) {
  if (GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_ESCAPE)) {
    *quit = true;
    return true;
  }

  float const angle = 0.05;

  if (GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_X) &&
      GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_RIGHT)) {
    a->view = glm::rotate(a->view, angle, glm::vec3{1.f, 0, 0});
    re::setCameraView(a->backend, a->view);
  } else if (GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_X) &&
             GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_LEFT)) {
    a->view = glm::rotate(a->view, -angle, glm::vec3{1.f, 0, 0});
    re::setCameraView(a->backend, a->view);
  } else if (GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_Y) &&
             GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_RIGHT)) {
    a->view = glm::rotate(a->view, angle, glm::vec3{0, 1.f, 0});
    re::setCameraView(a->backend, a->view);
  } else if (GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_Y) &&
             GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_LEFT)) {
    a->view = glm::rotate(a->view, -angle, glm::vec3{0, 1.f, 0});
    re::setCameraView(a->backend, a->view);
  } else if (GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_Z) &&
             GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_RIGHT)) {
    a->view = glm::rotate(a->view, angle, glm::vec3{0, 0, 1.f});
    re::setCameraView(a->backend, a->view);
  } else if (GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_Z) &&
             GLFW_PRESS == glfwGetKey(a->glfwWin, GLFW_KEY_LEFT)) {
    a->view = glm::rotate(a->view, -angle, glm::vec3{0, 0, 1.f});
    re::setCameraView(a->backend, a->view);
  }
  return true;
}

void printHelpMsg() {
  std::cout << "This is a badline demo. Usage: [--width] [--height] [--help]";
  std::cout << "\nThe width and height options define the window resolution";
  std::cout << std::endl;
}

bool initialize(AppData *const data) {
  updateCWD(data);

  if (!initializeArgParser(data)) {
    ap::printLogs(data->parser);
    return false;
  }

  if (data->printHelp) {
    demo::printHelpMsg();
    return true;
  }

  if (!initializeGLFW(data))
    return false;

  if (!initializeEngine(data)) {
    re::printLogs(data->engine);
    return false;
  }

  if (!loadAssets(data)) {
    re::printLogs(data->engine);
    return false;
  }

  return true;
}

bool loadAssets(AppData *const data) {
  if (!re::createTexture(data->backend, &data->trump))
    return false;

  if (!re::loadFromFile(data->trump, "./assets/trump.jpg"))
    return false;

  if (!re::createTexture(data->backend, &data->vikingTex))
    return false;

  if (!re::loadFromFile(data->vikingTex, "./assets/viking_room.png"))
    return false;

  if (!re::createObject(data->backend, data->vikingTex, &data->vikingObj))
    return false;

  if (!re::loadFromFile(data->vikingObj, "./assets/viking_room.obj"))
    return false;

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

  // if (!re::setPreferredGPU(data->backend, ".*llvmpipe.*"))
  //  return false;

  if (!re::initialize(data->backend))
    return false;

  if (!re::createWindow(data->backend,
                        data->windowWidth,
                        data->windowHeight,
                        "Demo",
                        &data->window))
    return false;

  if (!re::getWindowHandle(data->window, &data->glfwWin))
    return false;

  return true;
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

bool initializeArgParser(AppData *const data) {
  data->parserRAII = createArgParser();
  if (!data->parserRAII) {
    std::cerr << "Error: Failed to create parser" << std::endl;
    return false;
  }
  data->parser = data->parserRAII.get();

  if (!ap::addOption(data->parser, "width"))
    return false;

  if (!ap::addOption(data->parser, "height"))
    return false;

  if (!ap::addFlag(data->parser, "help"))
    return false;

  if (data->argc > 1 && !ap::parse(data->parser, data->argv, 1, data->argc))
    return false;

  if (!extractCLI(data))
    return false;

  return true;
}

bool extractCLI(AppData *const data) {
  if (!data)
    return false;

  std::string tmp{};
  if (!extractIf(data->parser, "width", &tmp))
    return false;
  if (tmp.size() && !convert(tmp, &data->windowWidth))
    return false;

  if (!extractIf(data->parser, "height", &tmp))
    return false;
  if (tmp.size() && !convert(tmp, &data->windowHeight))
    return false;

  if (isFlag(data->parser, "help"))
    data->printHelp = true;

  return true;
}

bool isFlag(ap::ArgParser *const handle, std::string const &flag) {
  unsigned count{};
  if (!ap::getFlagCount(handle, flag.c_str(), &count))
    return false;
  if (!count)
    return false;

  return true;
}

bool convert(std::string const &input, int *const output) {
  int numVal = 0;
  try {
    numVal = std::stoi(input);
  } catch (...) {
    std::cerr << "Error: Failed to convert: " << input << " to number\n";
    return false;
  }

  *output = numVal;
  return true;
}

bool extractIf(ap::ArgParser *const handle,
               std::string const &option,
               std::string *const value) {
  if (!handle)
    return false;

  if (!option.size()) {
    std::cerr << "Error: The option identifier is empty" << std::endl;
    return false;
  }

  if (!value) {
    std::cerr << "Error: The value output variable = nullptr" << std::endl;
    return false;
  }

  unsigned count{};
  if (!ap::getOptionCount(handle, option.c_str(), &count)) {
    std::cerr << "Error: Failed to get option count" << std::endl;
    return false;
  }

  if (count) {
    char const *val{};
    if (!ap::getOptionValue(handle, option.c_str(), 0, &val)) {
      std::cerr << "Error: Failed to get option value" << std::endl;
      return false;
    }
    *value = val;
  }

  return true;
}

void updateCWD(AppData *const data) {
  namespace fs = std::filesystem;
  fs::current_path(fs::canonical(fs::path{data->argv[0]}.parent_path()));
}

CustomUniqPtr<ap::ArgParser> createArgParser() {
  ap::ArgParser *handle{};
  ap::create(&handle);
  return {handle, ap::destroy};
}

CustomUniqPtr<re::RenderEngine> createRenderEngine() {
  re::RenderEngine *handle{};
  re::create(&handle);
  return {handle, re::destroy};
}
} // namespace demo
