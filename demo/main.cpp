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

#include "app.hpp"
#include <badline/vertices.hpp>
#include <badline/vertex.hpp>
#include <badline/indices.hpp>
#include <badline/instances.hpp>
#include <badline/transformMatrix.hpp>
#include <badline/vulkanBackend.hpp>
#include <badline/window.hpp>
#include <functional>
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

int main(int const argc, char const *const *const argv) {
  try {
    demo::raiiGLFW glfwGuard{};
    demo::App a{.argc = std::size_t(argc), .argv = argv};

    if (demo::initialize(&a))
      if (demo::run(&a))
        return 0;

    re::printErrors(a.engine.get());
    return 1;

  } catch (std::exception const &e) {
    std::cerr << "Error: Caught exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Error: Caught unknown exception." << std::endl;
  }
}

namespace demo {
void printErrorMessage(ap::ArgParser const *const parser) {
  char const *errorString{};
  ap::getErrorMessage(parser, &errorString);
  if (!errorString)
    return;
  std::cerr << "ERROR: " << errorString << std::endl;
}

bool createScene(App *const a) {
  re::Vertices *vertices{};
  if (!re::createVertices(a->engine.get(), &vertices)) {
    std::cerr << "createScene: Failed to create vertices" << std::endl;
    return false;
  }

  struct VertData {
    glm::vec3 pos{};
    glm::vec4 col{};
  };
  std::vector<VertData> vertData = {
      {{0.000000, 2.000000, 2.000000}, {1.f, 0.f, 0.f, 1.f}},
      {{0.000000, 0.000000, 2.000000}, {1.f, 0.f, 0.f, 1.f}},
      {{2.000000, 0.000000, 2.000000}, {1.f, 0.f, 0.f, 1.f}},
      {{2.000000, 2.000000, 2.000000}, {1.f, 0.f, 0.f, 1.f}},
      {{0.000000, 2.000000, 0.000000}, {1.f, 0.f, 0.f, 1.f}},
      {{0.000000, 0.000000, 0.000000}, {1.f, 0.f, 0.f, 1.f}},
      {{2.000000, 0.000000, 0.000000}, {1.f, 0.f, 0.f, 1.f}},
      {{2.000000, 2.000000, 0.000000}, {1.f, 0.f, 0.f, 1.f}},
  };

  for (auto &v : vertData) {
    re::Vertex *vert{};
    re::addVertex(vertices, &vert);
    re::setPosition(vert, &v.pos);
    re::setColor(vert, &v.col);
  }

  if (!re::uploadVertices(a->engine.get())) {
    std::cerr << "createScene: Failed to upload vertices. " << std::endl;
    return false;
  }

  std::vector<uint32_t> indexSeq = {
      0, 1, 2, 2, 3, 0, // Front face
      4, 5, 6, 6, 7, 4, // Back face
      0, 3, 7, 7, 4, 0, // Left face
      1, 2, 6, 6, 5, 1, // Right face
      3, 2, 6, 6, 7, 3, // Top face
      0, 1, 5, 5, 4, 0  // Bottom face
  };

  re::Indices *indices{};
  if (!re::createIndices(a->engine.get(), &indices)) {
    std::cerr << "createScene: Failed to create indices. " << std::endl;
    return false;
  }

  for (auto index : indexSeq)
    re::addIndex(indices, index);

  if (!re::uploadIndices(a->engine.get())) {
    std::cerr << "createScene: Failed to upload indices. " << std::endl;
    return false;
  }

  re::Instances *instances{};
  if (!re::createInstances(a->engine.get(), &instances)) {
    std::cerr << "createScene: Failed to create instances. " << std::endl;
    return false;
  }

  a->instanceRotations.resize(3, glm::mat4(1));
  a->instanceTranslations.resize(a->instanceRotations.size(), glm::mat4(1));

  for (auto const &m : a->instanceRotations) {
    unsigned long id = 0;
    if (!re::addInstance(instances, &id)) {
      std::cerr << "createScene: Failed to add instance. " << std::endl;
      return false;
    }
    re::setTransform(instances, id, &m);
  }

  if (!re::uploadInstances(a->engine.get())) {
    std::cerr << "createScene: Failed to upload instances. " << std::endl;
    return false;
  }

  a->view = glm::translate(glm::mat4(1), glm::vec3(0, 0, -15));
  re::TransformMatrix *viewTr, *projTr{};

  if (!re::createProjection(a->engine.get(), &projTr)) {
    std::cerr << "createScene: Failed to create projection. " << std::endl;
    return false;
  }

  auto const aspect = float(a->windowWidth) / float(a->windowHeight);
  a->proj = glm::perspective(glm::radians(45.f), aspect, 0.1f, 100.f);

  re::setMatrix(projTr, &a->proj);
  if (!re::setProjection(a->engine.get(), projTr)) {
    std::cerr << "createScene: Failed to set projection. " << std::endl;
    return false;
  }

  if (!re::createView(a->engine.get(), &viewTr)) {
    std::cerr << "createScene: Failed to create view. " << std::endl;
    return false;
  }

  re::setMatrix(viewTr, &a->view);
  if (!re::setView(a->engine.get(), viewTr)) {
    std::cerr << "createScene: Failed to set view. " << std::endl;
    return false;
  }

  return true;
}

bool update(App *const a) {
  static const glm::mat4 rotX =
      glm::rotate(glm::mat4(1), 0.05f, glm::vec3(1, 0, 0));
  static const glm::mat4 rotY =
      glm::rotate(glm::mat4(1), 0.05f, glm::vec3(0, 1, 0));
  static const glm::mat4 rotZ =
      glm::rotate(glm::mat4(1), 0.05f, glm::vec3(0, 0, 1));

  for (std::size_t i = 0; i < a->instanceRotations.size(); ++i) {
    switch (i % 3) {
    case 0:
      a->instanceRotations[i] = a->instanceRotations[i] * rotX;
      break;
    case 1:
      break;
    case 2:
      a->instanceRotations[i] = a->instanceRotations[i] * rotZ;
      break;
    }
  }

  re::Instances *instances{};
  if (!re::createInstances(a->engine.get(), &instances)) {
    std::cerr << "update: Failed to create instances. " << std::endl;
    return false;
  }

  for (std::size_t i = 0; i < a->instanceTranslations.size(); ++i) {
    auto initTransl = glm::translate(glm::mat4(1), glm::vec3(-1.f, -1.f, -1.f));
    a->instanceTranslations[i] =
        glm::translate(glm::mat4(1), glm::vec3(-4.f + i * 4.f, 0.f, 0.f));
    auto const m =
        a->instanceTranslations[i] * a->instanceRotations[i] * initTransl;

    unsigned long tr{};
    if (!re::addInstance(instances, &tr)) {
      std::cerr << "update: Failed to add instance" << std::endl;
      return false;
    }

    if (!re::setTransform(instances, tr, &m)) {
      std::cerr << "update: Failed to set transform" << std::endl;
      return false;
    }
  }

  if (!re::uploadInstances(a->engine.get())) {
    std::cerr << "update: Failed to upload instances. " << std::endl;
    return false;
  }

  re::TransformMatrix *viewTr{};
  a->view = a->view * rotY;
  if (!re::createView(a->engine.get(), &viewTr)) {
    std::cerr << "createScene: Failed to create view. " << std::endl;
    return false;
  }

  re::setMatrix(viewTr, &a->view);
  if (!re::setView(a->engine.get(), viewTr)) {
    std::cerr << "createScene: Failed to set view. " << std::endl;
    return false;
  }

  return true;
}

bool run(App *const a) {
  if (!createScene(a)) {
    std::cerr << "run: Creating scene failed" << std::endl;
    return false;
  }

  auto const engine = a->engine.get();
  bool windowOpen = true;
  re::isOpen(a->win, &windowOpen);

  constexpr auto frameMinTime = std::chrono::milliseconds(17);
  auto timeStamp = std::chrono::steady_clock::now();

  while (windowOpen) {
    glfwPollEvents();

    if (auto now = std::chrono::steady_clock::now();
        now - timeStamp >= frameMinTime) {
      if (!update(a)) {
        std::cerr << "run: Update failed" << std::endl;
        return false;
      }

      if (!re::render(engine)) {
        std::cerr << "run: Rendering failed" << std::endl;
        return false;
      }

      timeStamp = now;
    }

    bool keyPressed = false;
    re::isKeyPressed(a->win, GLFW_KEY_ESCAPE, &keyPressed);
    if (keyPressed)
      re::close(a->win);

    re::isOpen(a->win, &windowOpen);
  }

  return true;
}

std::unique_ptr<re::RenderEngine, std::function<void(re::RenderEngine *const)>>
createSmartRenderEngine() {
  re::RenderEngine *handle{};
  re::create(&handle);
  return {handle, re::destroy};
}

bool initialize(App *const a) {
  namespace fs = std::filesystem;
  fs::current_path(fs::canonical(fs::path{a->argv[0]}.parent_path()));

  if (a->argc > 1 && !initializeArgParser(a))
    return false;

  a->engine = createSmartRenderEngine();
  if (!a->engine) {
    std::cerr << "initialize: Failed to create render engine" << std::endl;
    return false;
  }

  re::VulkanBackend *vk{};
  if (!re::createBackend(a->engine.get(), &vk)) {
    std::cerr << "initialize: Failed to create render backend" << std::endl;
    return false;
  }

  re::setApplicationName(vk, "Demo");
  re::setValidationLayersOn(vk);
  re::initialize(a->engine.get());

  if (!openWindow(a, vk)) {
    std::cerr << "initialize: Failed to open window" << std::endl;
    return false;
  }

  return true;
}

SmartArgParser createSmartArgParser() {
  ap::ArgParser *parser{};
  ap::create(&parser);
  return SmartArgParser{parser, ap::destroy};
}

bool initializeArgParser(App *const a) {
  a->parser = createSmartArgParser();
  auto const p = a->parser.get();

  if (!p) {
    std::cerr << "Failed to create arg parser" << std::endl;
    return false;
  }

  if (!ap::addOption(p, "width", 'w')) {
    printErrorMessage(p);
    return false;
  }

  if (!ap::addOption(p, "height", 'h')) {
    printErrorMessage(p);
    return false;
  }

  if (!ap::parse(p, a->argv, 1, a->argc)) {
    printErrorMessage(p);
    return false;
  }

  return true;
}

bool convertToNumber(std::string const &input, long long *const output) {
  if (input.size()) {
    try {
      *output = std::stoll(input);
    } catch (...) {
      std::cerr << "Converting: '" << input << "' to a number failed\n";
      return false;
    }
  } else
    *output = 0;
  return true;
}

bool convertWindowArgs(std::vector<std::string> *const vals,
                       uint32_t *const w,
                       uint32_t *const h) {
  auto getSize = [&vals](std::size_t const i, uint32_t *const output) {
    long long out{};
    if (!convertToNumber(vals->at(i), &out))
      return false;
    if (out)
      *output = out;
    return true;
  };

  if (!getSize(0, w)) {
    std::cerr << "Failed to set window width" << std::endl;
    return false;
  }

  if (!getSize(1, h)) {
    std::cerr << "Failed to set window height" << std::endl;
    return false;
  }

  return true;
}

bool extractWindowArgs(App *const a, uint32_t *const w, uint32_t *const h) {
  std::vector<std::string> const opts = {"width", "height"};
  std::vector<std::string> vals(opts.size(), "");
  auto const handle = a->parser.get();

  for (std::size_t i = 0; i < opts.size(); ++i) {
    unsigned count{};

    if (!ap::getOptionCount(handle, opts[i].c_str(), &count)) {
      printErrorMessage(handle);
      return false;
    }

    if (count) {
      char const *value{};
      if (!ap::getOptionValue(handle, opts[i].c_str(), 0, &value)) {
        printErrorMessage(handle);
        return false;
      }
      vals[i] = value;
    }
  }

  return convertWindowArgs(&vals, w, h);
}

bool openWindow(App *const a, re::VulkanBackend *const vk) {
  uint32_t width{640}, height{480};

  if (a->argc > 1 && !extractWindowArgs(a, &width, &height)) {
    std::cerr << "Failed to extract window args." << std::endl;
    return false;
  }

  re::Window *win{};
  if (!re::createWindow(a->engine.get(), &win)) {
    std::cerr << "openWindow: Window creation failure: " << std::endl;
    return false;
  }

  if (!re::setResolution(win, width, height)) {
    std::cerr << "openWindow: Failed to set resolution: " << std::endl;
    return false;
  }

  if (!re::open(win, vk)) {
    std::cerr << "openWindow: Failed to open window: " << std::endl;
    return false;
  }

  a->win = win;
  a->windowWidth = width;
  a->windowHeight = height;
  return true;
}
} // namespace demo
