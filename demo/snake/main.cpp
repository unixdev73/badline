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
#include <badline/vertexData.hpp>
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
#include <list>

namespace demo {
template <typename T>
using CustomUniqPtr = std::unique_ptr<T, std::function<void(T *const)>>;

enum class Direction { Left, Right, Up, Down };

struct AppData {
  // core objects
  std::mt19937 rng{std::random_device{}()};
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
  re::VulkanWindow *window{};
  GLFWwindow *glfwWin{};
  std::array<unsigned short, 100> frameTimes{};
  std::size_t frameIdx{};

  // game common properties
  std::size_t cubeSide{32};

  // game camera objects
  glm::mat4 projection{1.f};
  glm::mat4 view{1.f};

  // food objects
  re::Texture *schumer{};
  re::Object *cube2{};
  std::pair<std::size_t, std::size_t> foodPos{};
  bool makeNewFood{true};

  // player objects
  bool snakeOutOfBounds{}, extendSnake{}, snakeBitItself{};
  glm::mat4 cubeModel{1.f};
  re::Texture *trump{};
  re::Object *cube{};
  std::chrono::time_point<std::chrono::steady_clock> lastMove{};
  std::vector<std::pair<std::size_t, std::size_t>> snakeBody{};
  std::size_t score{};
  bool updateScore{};
  std::vector<Direction> directions{};
  std::list<std::pair<std::pair<std::size_t, std::size_t>, Direction>>
      dirChangePoints{};

  // user interface
  re::Object *text1{};
  glm::mat4 textModel{};
};

bool initialize(AppData *const);
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
Direction getOppositeDirection(Direction const d) {
  Direction dir;
  switch (d) {
  case Direction::Down:
    dir = Direction::Up;
    break;

  case Direction::Up:
    dir = Direction::Down;
    break;

  case Direction::Left:
    dir = Direction::Right;
    break;

  case Direction::Right:
    dir = Direction::Left;
    break;
  }
  return dir;
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

bool initializeUI(AppData *const a) {
  if (!re::createObject(a->backend, 0, &a->text1)) {
    std::cerr << "ERROR: Failed to create text1 object" << std::endl;
    return false;
  }

  a->textModel = glm::translate(glm::mat4{1}, glm::vec3(50.f, -10.f, 0.f)) *
                 glm::scale(glm::mat4{1}, glm::vec3(5.f, 5.f, 0.f));

  if (!setFontlessText(a->text1, "SCORE = 0")) {
    std::cerr << "ERROR: Failed to set fontless text" << std::endl;
    return false;
  }

  return true;
}

bool initializeScene(AppData *const a) {
  a->projection = glm::orthoRH_ZO(0.f,
                                  (float)a->windowWidth,
                                  0.f,
                                  (float)a->windowHeight,
                                  -2 * float(a->cubeSide),
                                  2 * float(a->cubeSide));

  a->view = glm::lookAt(glm::vec3{0, 0, -float(a->cubeSide) / 2.f},
                        glm::vec3{0, 0, 0},
                        glm::vec3{0, -1.f, 0});

  if (!createCube(a, a->trump, &a->cube))
    return false;
  if (!createCube(a, a->schumer, &a->cube2))
    return false;

  float const cubeSc = a->cubeSide;
  a->cubeModel = glm::scale(glm::mat4{1.f}, glm::vec3{cubeSc, cubeSc, cubeSc});
  a->snakeBody.push_back({a->windowWidth / 2, a->windowHeight / 2});
  a->directions.push_back(Direction::Right);

  re::setCameraProjection(a->backend, a->projection);
  re::setCameraView(a->backend, a->view);
  re::setClearColor(a->backend, 0.4f, 0.0f, 0.6f);
  glfwSetWindowPos(a->glfwWin, 0, 0);

  a->lastMove = std::chrono::steady_clock::now();

  if (!initializeUI(a)) {
    std::cerr << "ERROR: Failed to initialize user interface" << std::endl;
    return false;
  }

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

    Direction newDirection{a->directions.back()},
        oldDirection{a->directions.back()};

    if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_RIGHT)) {
      newDirection = Direction::Right;
    }

    if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_DOWN)) {
      newDirection = Direction::Down;
    }

    if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_LEFT)) {
      newDirection = Direction::Left;
    }

    if (GLFW_PRESS == glfwGetKey(win, GLFW_KEY_UP)) {
      newDirection = Direction::Up;
    }

    if (newDirection != getOppositeDirection(oldDirection))
      if (newDirection != oldDirection) {
        a->directions.back() = newDirection;
        if (a->snakeBody.size() > 1)
          a->dirChangePoints.push_back({a->snakeBody.back(), newDirection});
      }
  }

  return true;
}

void onGameExit(AppData *const a) {
  if (a->snakeOutOfBounds)
    std::cout << "snake out of bounds, ";
  else if (a->snakeBitItself)
    std::cout << "snake bit itself, ";
  std::cout << "score: " << a->snakeBody.size() << std::endl;

  float sum = 0.f;
  for (auto const &e : a->frameTimes)
    sum += static_cast<float>(e);
  auto const avgFrameTime = sum / a->frameTimes.size();
  std::cout << "AVG: " << avgFrameTime << " MS/F = ";
  std::cout << (1.0 / avgFrameTime) * 1000.0 << " FPS" << std::endl;
}

void updateFood(AppData *const a) {
  if (!a->makeNewFood)
    return;

  std::size_t x{}, y{};
  bool generate = true;

  while (generate) {
    x = a->rng() % (a->windowWidth - a->cubeSide + 1) + a->cubeSide / 2;
    y = a->rng() % (a->windowHeight - a->cubeSide + 1) + a->cubeSide / 2;
    generate = false;

    for (auto const &[sX, sY] : a->snakeBody)
      if (x == sX && y == sY) {
        generate = true;
        break;
      }
  }

  a->foodPos.first = x;
  a->foodPos.second = y;
  a->makeNewFood = false;
}

bool intersects(std::pair<std::pair<std::size_t, std::size_t>,
                          std::pair<std::size_t, std::size_t>> const &a,
                std::pair<std::pair<std::size_t, std::size_t>,
                          std::pair<std::size_t, std::size_t>> const &b) {
  auto const [a_pos, a_size] = a;
  auto const [b_pos, b_size] = b;

  auto const [ax, ay] = a_pos;
  auto const [aw, ah] = a_size;

  auto const [bx, by] = b_pos;
  auto const [bw, bh] = b_size;

  return !(ax + aw <= bx || ax >= bx + bw || ay + ah <= by || ay >= by + bh);
}

void updateSnake(AppData *const a) {
  std::size_t const increment = 2;
  std::size_t *x{};
  std::size_t *y{};

  std::unordered_map<Direction, std::pair<bool, std::size_t **>> shiftMap{
      {Direction::Right, {1, &x}},
      {Direction::Left, {0, &x}},
      {Direction::Up, {0, &y}},
      {Direction::Down, {1, &y}}};

  {
    std::pair const snakeSz(a->cubeSide, a->cubeSide);
    auto const snakePos = a->snakeBody.back();
    for (auto it = a->snakeBody.rbegin() + 2; it < a->snakeBody.rend(); ++it) {
      if (intersects({snakePos, snakeSz}, {*it, snakeSz})) {
        a->snakeBitItself = true;
        return;
      }
    }
  }

  if (a->extendSnake) {
    auto const head = a->snakeBody.back();
    auto const hdir = a->directions.back();
    if (hdir == Direction::Right)
      a->snakeBody.push_back({head.first + a->cubeSide, head.second});
    else if (hdir == Direction::Left)
      a->snakeBody.push_back({head.first - a->cubeSide, head.second});
    else if (hdir == Direction::Up)
      a->snakeBody.push_back({head.first, head.second - a->cubeSide});
    else
      a->snakeBody.push_back({head.first, head.second + a->cubeSide});
    a->directions.push_back(hdir);
    a->extendSnake = false;
    ++a->score;
    a->updateScore = true;
  }

  {
    auto const [headX, headY] = a->snakeBody.back();
    auto const direction = a->directions.back();
    if (direction == Direction::Right &&
        headX + increment + a->cubeSide / 2 > std::size_t(a->windowWidth)) {
      a->snakeOutOfBounds = true;
      return;
    } else if (direction == Direction::Left &&
               increment > headX - a->cubeSide / 2) {
      a->snakeOutOfBounds = true;
      return;
    } else if (direction == Direction::Up &&
               headY - increment - a->cubeSide / 2 == 0) {
      a->snakeOutOfBounds = true;
      return;
    } else if (direction == Direction::Down &&
               increment + headY + a->cubeSide / 2 >
                   std::size_t(a->windowHeight)) {
      a->snakeOutOfBounds = true;
      return;
    }
  }

  bool remove = false;
  for (std::size_t i = 0; i < a->snakeBody.size(); ++i) {
    auto const [op, axis] = shiftMap.at(a->directions[i]);
    x = &a->snakeBody[i].first;
    y = &a->snakeBody[i].second;
    **axis = op ? **axis + increment : **axis - increment;
    for (auto const &[pos, dir] : a->dirChangePoints) {
      if (std::pair(*x, *y) == pos) {
        a->directions[i] = dir;
        if (i == 0)
          remove = true;
        break;
      }
    }
  }

  if (remove)
    a->dirChangePoints.pop_front();
}

bool updateLogic(AppData *const a) {
  updateFood(a);
  updateSnake(a);

  if (a->snakeOutOfBounds)
    return true;

  auto const tileSz = std::pair(a->cubeSide, a->cubeSide);
  auto const head = a->snakeBody.back();

  if (intersects(std::pair(a->foodPos, tileSz), std::pair(head, tileSz))) {
    a->makeNewFood = true;
    a->extendSnake = true;
  }

  return true;
}

bool stageScene(AppData *const a) {
  if (!re::stage(a->backend, a->text1, a->textModel)) {
    std::cerr << "Failed to stage text1" << std::endl;
    return false;
  }

  for (auto const &[x, y] : a->snakeBody) {
    if (!re::stage(a->backend,
                   a->cube,
                   glm::translate(glm::mat4{1}, glm::vec3(x, -1.f * y, 0.f)) *
                       a->cubeModel)) {
      std::cerr << "Failed to stage snake" << std::endl;
      return false;
    }
  }

  if (!re::stage(
          a->backend,
          a->cube2,
          glm::translate(
              glm::mat4{1},
              glm::vec3(a->foodPos.first, -1.f * a->foodPos.second, 0.f)) *
              a->cubeModel)) {
    std::cerr << "Failed to stage food" << std::endl;
    return false;
  }

  return true;
}

auto updateFrame(AppData *const a, auto const &beginFrame) {
  namespace ch = std::chrono;
  auto const diff =
      ch::duration_cast<ch::milliseconds>(ch::steady_clock::now() - beginFrame);

  a->frameTimes[a->frameIdx] = diff.count();
  a->frameIdx = (a->frameIdx + 1) % a->frameTimes.size();
  return diff;
}

bool run(AppData *const a) {
  auto const minTime = std::chrono::milliseconds(17); // ~60 FPS
  auto beginFrame = std::chrono::steady_clock::now();

  while (!glfwWindowShouldClose(a->glfwWin)) {
    bool quit = false;
    glfwPollEvents();

    if (auto diff = updateFrame(a, beginFrame); diff >= minTime) {
      if (!handleInput(a, &quit))
        return false;
      if (quit)
        break;

      if (!updateLogic(a))
        return false;
      if (a->snakeOutOfBounds || a->snakeBitItself)
        break;

      if (a->updateScore) {
        if (!setFontlessText(
                a->text1, "SCORE = " + std::to_string(a->score))) {
          std::cerr << "ERROR: Failed to set fontless text" << std::endl;
          return false;
        }
        a->updateScore = false;
      }

      if (!stageScene(a))
        return false;

      if (!re::render(a->backend, a->window))
        return false;

      beginFrame = std::chrono::steady_clock::now();
    }
  }

  onGameExit(a);
  return true;
}

void printHelpMsg() {
  std::cout << "This is a badline demo. Usage: [--width] [--height] [--help]";
  std::cout << "\nThe width and height options define the window resolution";
  std::cout << std::endl;
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

bool isFlag(ap::ArgParser *const handle, std::string const &flag) {
  unsigned count{};
  if (!ap::getFlagCount(handle, flag.c_str(), &count))
    return false;
  if (!count)
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

bool initialize(AppData *const data) {
  updateCWD(data);

  if (!initializeArgParser(data))
    return false;

  if (data->printHelp) {
    demo::printHelpMsg();
    return true;
  }

  if (!initializeGLFW(data))
    return false;

  if (!initializeEngine(data))
    return false;

  if (!loadAssets(data))
    return false;

  if (!initializeScene(data)) {
    std::cerr << "Failed to initialize scene" << std::endl;
    return false;
  }
  return true;
}
} // namespace demo
