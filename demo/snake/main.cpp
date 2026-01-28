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
#include <badline/renderEngine.hpp>
#include <badline/argParser.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <filesystem>
#include <functional>
#include <iostream>
#include <random>
#include <memory>
#include <chrono>
#include <array>
#include <set>

namespace demo {
template <typename T>
using CustomUniqPtr = std::unique_ptr<T, std::function<void(T *const)>>;

enum class Direction { Left, Right, Up, Down };

Direction getOppositeDirection(Direction const d) {
  switch (d) {
  case Direction::Down:
    return Direction::Up;
  case Direction::Up:
    return Direction::Down;
  case Direction::Left:
    return Direction::Right;
  case Direction::Right:
    return Direction::Left;
  }
}

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
  glm::mat4 cubeModel{1.f};
  re::VulkanWindow *window{};
  GLFWwindow *glfwWin{};
  re::Texture *trump{};
  re::Object *cube{};
  re::Texture *schumer{};
  re::Object *cube2{};

  std::array<unsigned short, 100> frameTimes{};
  std::size_t frameIdx{};

  std::chrono::time_point<std::chrono::steady_clock> lastMove{};
  std::vector<std::pair<std::size_t, std::size_t>> snakeBody{};
  Direction direction{Direction::Right};
  std::pair<std::size_t, std::size_t> foodPos{};
  bool makeNewFood{true};

  std::mt19937 rng{std::random_device{}()};
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
bool createCube(AppData *const a, re::Texture *const, re::Object **const);
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
  float const bodySize = 32.f;
  std::size_t const gridWidth = a->windowWidth / bodySize;
  std::size_t const gridHeight = a->windowHeight / bodySize;
  a->projection = glm::orthoRH_ZO(0.f,
                                  (float)a->windowWidth,
                                  0.f,
                                  (float)a->windowHeight,
                                  -2 * bodySize,
                                  2 * bodySize);
  re::setCameraProjection(a->backend, a->projection);

  a->view = glm::lookAt(glm::vec3{0, 0, -bodySize / 2.f},
                        glm::vec3{0, 0, 0},
                        glm::vec3{0, -1.f, 0});
  re::setCameraView(a->backend, a->view);

  if (!createCube(a, a->trump, &a->cube))
    return false;
  if (!createCube(a, a->schumer, &a->cube2))
    return false;
  float const cubeSc = bodySize;
  a->cubeModel = glm::scale(glm::mat4{1.f}, glm::vec3{cubeSc, cubeSc, cubeSc});

  glfwSetWindowPos(a->glfwWin, 0, 0);

  a->snakeBody.push_back({gridWidth / 2, gridHeight / 2});
  re::setClearColor(a->backend, 0.4f, 0.0f, 0.6f);

  namespace ch = std::chrono;
  a->lastMove = ch::steady_clock::now();
  auto const minTime = std::chrono::milliseconds(17); // ~60 FPS
  auto beginFrame = std::chrono::steady_clock::now();

  while (!glfwWindowShouldClose(a->glfwWin)) {
    glfwPollEvents();
    bool quit = false;

    auto const diff = ch::duration_cast<ch::milliseconds>(
        ch::steady_clock::now() - beginFrame);

    a->frameTimes[a->frameIdx] = diff.count();
    a->frameIdx = (a->frameIdx + 1) % a->frameTimes.size();

    if (diff >= minTime) {
      if (!handleInput(a, &quit))
        return false;
      if (quit)
        break;

      if (auto now = ch::steady_clock::now();
          now - a->lastMove >= ch::milliseconds(250)) {
        auto oldTail = a->snakeBody.back();

        for (std::size_t i = a->snakeBody.size() - 1; i > 0; --i) {
          a->snakeBody[i] = a->snakeBody[i - 1];
        }

        if (a->snakeBody.at(0) == a->foodPos) {
          a->makeNewFood = true;
          a->snakeBody.push_back(oldTail);
        }

        switch (a->direction) {
        case Direction::Right:
          ++a->snakeBody.at(0).first;
          break;
        case Direction::Down:
          ++a->snakeBody.at(0).second;
          break;
        case Direction::Left:
          --a->snakeBody.at(0).first;
          break;
        case Direction::Up:
          --a->snakeBody.at(0).second;
          break;
        }
        a->lastMove = now;
      }

      if (a->snakeBody.front().first > gridWidth ||
          a->snakeBody.front().second > gridHeight) {
        std::cout << "Game over: snake out of bounds" << std::endl;
        break;
      }

      std::set<std::pair<std::size_t, std::size_t>> chunks;
      bool bitten = false;
      for (auto const piece : a->snakeBody) {
        if (chunks.contains(piece)) {
          std::cout << "Game over: snake bit itself" << std::endl;
          bitten = true;
          break;
        }
        chunks.emplace(piece);
        auto x = (piece.first * bodySize + bodySize / 2.f);
        auto y = -(piece.second * bodySize + bodySize / 2.f);
        auto const posMat =
            glm::translate(glm::mat4{1.f}, glm::vec3{x, y, 0.f}) * a->cubeModel;

        if (!re::stage(a->backend, a->cube, posMat)) {
          re::printLogs(a->engine);
          return false;
        }
      }
      if (bitten)
        break;

      if (a->makeNewFood) {
        std::size_t randomX{}, randomY{};
        bool regen{};
        do {
          randomX = a->rng() % gridWidth;
          randomY = a->rng() % gridHeight;
          regen = false;
          for (auto const &pos : a->snakeBody) {
            if (randomX == pos.first && randomY == pos.second) {
              regen = true;
              break;
            }
          }
        } while (regen);

        a->foodPos = {randomX, randomY};
        a->makeNewFood = false;
      }

      auto x = (a->foodPos.first * bodySize + bodySize / 2.f);
      auto y = -(a->foodPos.second * bodySize + bodySize / 2.f);
      auto const posMat =
          glm::translate(glm::mat4{1.f}, glm::vec3{x, y, 0.f}) * a->cubeModel;

      if (!re::stage(a->backend, a->cube2, posMat)) {
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

  std::cout << "score: " << a->snakeBody.size() << std::endl;

  float sum = 0.f;
  for (auto const &e : a->frameTimes)
    sum += static_cast<float>(e);
  auto const avgFrameTime = sum / a->frameTimes.size();
  std::cout << "AVG: " << avgFrameTime << " MS/F = ";
  std::cout << (1.0 / avgFrameTime) * 1000.0 << " FPS" << std::endl;
  return true;
}

bool createCube(AppData *const a,
                re::Texture *const tex,
                re::Object **const cube) {
  if (!re::createObject(a->backend, tex, cube)) {
    std::cerr << "Failed to create cube" << std::endl;
    return false;
  }

  float const z{0.5f};
  re::addVertex(*cube, {-0.5f, -0.5f, z}, {0.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {0.5f, -0.5f, z}, {1.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {0.5f, 0.5f, z}, {1.0f, 1.0f}, {}, {});
  re::addVertex(*cube, {-0.5f, 0.5f, z}, {0.0f, 1.0f}, {}, {});

  re::addVertex(*cube, {0.5f, -0.5f, -z}, {0.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {-0.5f, -0.5f, -z}, {1.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {-0.5f, 0.5f, -z}, {1.0f, 1.0f}, {}, {});
  re::addVertex(*cube, {0.5f, 0.5f, -z}, {0.0f, 1.0f}, {}, {});

  re::addVertex(*cube, {-0.5f, -0.5f, -z}, {0.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {-0.5f, -0.5f, z}, {1.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {-0.5f, 0.5f, z}, {1.0f, 1.0f}, {}, {});
  re::addVertex(*cube, {-0.5f, 0.5f, -z}, {0.0f, 1.0f}, {}, {});

  re::addVertex(*cube, {0.5f, -0.5f, z}, {0.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {0.5f, -0.5f, -z}, {1.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {0.5f, 0.5f, -z}, {1.0f, 1.0f}, {}, {});
  re::addVertex(*cube, {0.5f, 0.5f, z}, {0.0f, 1.0f}, {}, {});

  re::addVertex(*cube, {-0.5f, 0.5f, z}, {0.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {0.5f, 0.5f, z}, {1.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {0.5f, 0.5f, -z}, {1.0f, 1.0f}, {}, {});
  re::addVertex(*cube, {-0.5f, 0.5f, -z}, {0.0f, 1.0f}, {}, {});

  re::addVertex(*cube, {-0.5f, -0.5f, -z}, {0.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {0.5f, -0.5f, -z}, {1.0f, 0.0f}, {}, {});
  re::addVertex(*cube, {0.5f, -0.5f, z}, {1.0f, 1.0f}, {}, {});
  re::addVertex(*cube, {-0.5f, -0.5f, z}, {0.0f, 1.0f}, {}, {});

  std::vector<uint32_t> cubeIndices = {0,  1,  2,  2,  3,  0,

                                       4,  5,  6,  6,  7,  4,

                                       8,  9,  10, 10, 11, 8,

                                       12, 13, 14, 14, 15, 12,

                                       16, 17, 18, 18, 19, 16,

                                       20, 21, 22, 22, 23, 20};

  re::setIndices(*cube, std::move(cubeIndices));
  re::uploadObjectDataToGPU(*cube);
  return true;
}

bool handleInput(AppData *const a, bool *const quit) {
  if (!a->glfwWin) {
    std::cerr << "Window handle = nullptr" << std::endl;
    return false;
  }

  std::vector<GLFWwindow *> windows = {a->glfwWin};
  for (auto const win : windows) {
    if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_ESCAPE)) {
      *quit = true;
      return true;
    }

    Direction newDirection{a->direction};

    if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_RIGHT)) {
      newDirection = Direction::Right;
    }

    else if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_DOWN)) {
      newDirection = Direction::Down;
    }

    else if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_LEFT)) {
      newDirection = Direction::Left;
    }

    else if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_UP)) {
      newDirection = Direction::Up;
    }

    if (newDirection != getOppositeDirection(a->direction))
      a->direction = newDirection;
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

  if (!re::createTexture(data->backend, &data->schumer))
    return false;

  if (!re::loadFromFile(data->schumer, "./assets/schumer.jpg"))
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
