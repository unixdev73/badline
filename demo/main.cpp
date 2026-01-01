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

#include "app.hpp"
#include <GLFW/glfw3.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>

int main(int const argc, char const *const *const argv) {
  try {
    demo::App a{.argc = std::size_t(argc), .argv = argv};

    if (demo::initialize(&a)) {
      std::cerr << "Failed to initialize demo" << std::endl;
      return 1;
    }

    return demo::run(&a);

  } catch (std::exception const &e) {
    std::cerr << "Error: Caught exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Error: Caught unknown exception." << std::endl;
  }
}

namespace demo {
int createScene(App *const a) {
  std::vector<re::Vertex> vertices = {
      {{-1.000000, 1.000000, 1.500000}, {1.f, 0.f, 0.f, 1.f}},
      {{-1.000000, -1.000000, 1.500000}, {1.f, 0.f, 0.f, 1.f}},
      {{1.000000, -1.000000, 1.500000}, {1.f, 0.f, 0.f, 1.f}},
      {{1.000000, 1.000000, 1.500000}, {1.f, 0.f, 0.f, 1.f}},
      {{-1.000000, 1.000000, -0.500000}, {1.f, 0.f, 0.f, 1.f}},
      {{-1.000000, -1.000000, -0.500000}, {1.f, 0.f, 0.f, 1.f}},
      {{1.000000, -1.000000, -0.500000}, {1.f, 0.f, 0.f, 1.f}},
      {{1.000000, 1.000000, -0.500000}, {1.f, 0.f, 0.f, 1.f}},
  };

  std::vector<uint32_t> indices = {
      0, 1, 2, 2, 3, 0, // Front face
      4, 5, 6, 6, 7, 4, // Back face
      0, 3, 7, 7, 4, 0, // Left face
      1, 2, 6, 6, 5, 1, // Right face
      3, 2, 6, 6, 7, 3, // Top face
      0, 1, 5, 5, 4, 0  // Bottom face
  };

  if (auto r = re::setVertices(a->engine.get(), &vertices);
      r != re::Result::Success) {
    std::cerr << "Failed to set vertices. " << std::endl;
    return 1;
  }

  if (auto r = re::setIndices(a->engine.get(), &indices);
      r != re::Result::Success) {
    std::cerr << "Failed to set indices. " << std::endl;
    return 1;
  }

  auto &instances = a->instances;
  std::size_t instCnt = 3;
  instances.resize(instCnt);

  for (std::size_t i = 0; i < instances.size(); ++i) {
    instances[i] =
        glm::translate(glm::mat4(1), glm::vec3(-5.f + i * 5.f, -1.f, 0.f));
  }

  a->view = glm::translate(glm::mat4(1), glm::vec3(0, 0, -15));

  if (auto r = re::setInstances(a->engine.get(), &instances, instances.size());
      r != re::Result::Success) {
    std::cerr << "Failed to set instances. " << std::endl;
    return 1;
  }
  return 0;
}

int update(App *const a) {
  auto &instances = a->instances;
  auto &proj = a->proj;
  auto &view = a->view;

  static const glm::mat4 rotX =
      glm::rotate(glm::mat4(1), 0.001f, glm::vec3(1, 0, 0));
  static const glm::mat4 rotY =
      glm::rotate(glm::mat4(1), 0.001f, glm::vec3(0, 1, 0));
  static const glm::mat4 rotZ =
      glm::rotate(glm::mat4(1), 0.001f, glm::vec3(0, 0, 1));

  for (std::size_t i = 0; i < instances.size(); ++i) {
    switch (i % 3) {
    case 0:
      instances[i] = instances[i] * rotX;
      break;
    case 1:
      // instances[i] = instances[i] * rotY;
      break;
    case 2:
      instances[i] = instances[i] * rotZ;
      break;
    }
  }

  if (auto r = re::setInstances(a->engine.get(), &instances);
      r != re::Result::Success) {
    std::cerr << "Failed to set instances. " << std::endl;
    return 1;
  }

  auto const aspect = float(a->windowWidth) / float(a->windowHeight);
  proj = glm::perspective(glm::radians(45.f), aspect, 0.1f, 100.f);
  view = view * rotY;
  re::setProjection(a->engine.get(), proj);
  re::setView(a->engine.get(), view);
  return 0;
}

int run(App *const a) {
  if (createScene(a)) {
    std::cerr << "Creating scene failed" << std::endl;
    return 1;
  }

  auto const engine = a->engine.get();
  bool windowOpen = true;
  re::isWindowOpen(engine, &windowOpen);

  while (windowOpen) {
    glfwPollEvents();

    if (update(a)) {
      std::cerr << "Update failed" << std::endl;
      return 1;
    }

    auto r = re::render(engine);
    if (r != re::Result::Success) {
      std::cerr << "Rendering failed" << std::endl;
      return 1;
    }

    bool keyPressed = false;
    re::isKeyPressed(engine, GLFW_KEY_ESCAPE, &keyPressed);
    if (keyPressed)
      re::closeWindow(engine);

    re::isWindowOpen(engine, &windowOpen);
  }

  return 0;
}

int initialize(App *const a) {
  namespace fs = std::filesystem;
  fs::current_path(fs::canonical(fs::path{a->argv[0]}.parent_path()));

  if (a->argc > 1 && initializeArgParser(a))
    return 1;

  a->engine = re::createRenderEngine("Demo", true);
  if (!a->engine) {
    std::cerr << "Failed to create render engine" << std::endl;
    return 1;
  }

  if (openWindow(a)) {
    std::cerr << "Failed to open window" << std::endl;
    return 1;
  }

  return 0;
}

int initializeArgParser(App *const a) {
  a->parser = ap::createArgParser();
  if (!a->parser) {
    std::cerr << "Failed to create arg parser" << std::endl;
    return 1;
  }

  ap::addOption(a->parser.get(), "width", 'w');
  ap::addOption(a->parser.get(), "height", 'h');

  auto result = ap::parse(a->parser.get(), a->argv, 1, a->argc);
  if (result != ap::Result::Success) {
    std::size_t pos{};
    ap::getErrorPosition(a->parser.get(), &pos);
    std::string err = a->argv[pos + 1];
    std::string errType{};
    ap::toString(result, &errType);
    std::cerr << "Argument parsing failed with error code: " << errType
              << "\nProblematic token";
    std::cerr << " at position " << pos << ": '" << err << "'" << std::endl;
    return 1;
  }

  return 0;
}

int convertToNumber(std::string const &input, long long *const output) {
  if (input.size()) {
    try {
      *output = std::stoll(input);
    } catch (...) {
      std::cerr << "Converting: '" << input << "' to a number failed\n";
      return 1;
    }
  } else
    *output = 0;
  return 0;
}

int convertWindowArgs(std::vector<std::string> *const vals,
                      uint32_t *const w,
                      uint32_t *const h) {
  auto getSize = [&vals](std::size_t const i, uint32_t *const output) {
    long long out{};
    if (convertToNumber(vals->at(i), &out))
      return 1;
    if (out)
      *output = out;
    return 0;
  };

  if (getSize(0, w)) {
    std::cerr << "Failed to set window width" << std::endl;
    return 1;
  }

  if (getSize(1, h)) {
    std::cerr << "Failed to set window height" << std::endl;
    return 1;
  }

  return 0;
}

int extractWindowArgs(App *const a, uint32_t *const w, uint32_t *const h) {
  std::vector<std::string> const opts = {"width", "height"};
  std::vector<std::string> vals(opts.size(), "");
  auto const handle = a->parser.get();

  for (std::size_t i = 0; i < opts.size(); ++i) {
    std::size_t count{};

    auto result = ap::getOptionCount(handle, opts[i], &count);
    if (result != ap::Result::Success) {
      std::cerr << "Failed to query option count: '" << opts[i];
      std::cerr << "'" << std::endl;
      return 1;
    }

    if (count)
      ap::getOptionInstanceValue(handle, opts[i], 0, &vals[i]);
  }

  return convertWindowArgs(&vals, w, h);
}

int openWindow(App *const a) {
  uint32_t width{640}, height{480};

  if (a->argc > 1 && extractWindowArgs(a, &width, &height)) {
    std::cerr << "Failed to extract window args." << std::endl;
    return 1;
  }

  auto result = re::createWindow(a->engine.get(), width, height);
  if (result != re::Result::Success) {
    std::string err{};
    re::toString(result, &err);
    std::cerr << "Window creation failure: " << err << std::endl;
    return 1;
  }

  a->windowWidth = width;
  a->windowHeight = height;
  return 0;
}
} // namespace demo
