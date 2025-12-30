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
int createTriangle(App *const a) {
  std::vector<re::Vertex> vertices = {
      {{-0.75f, 0.75f, 0.f, 1.f}, {1.f, 0.f, 0.f, 1.f}},
      {{0.f, -0.75f, 0.f, 1.f}, {0.f, 1.f, 0.f, 1.f}},
      {{0.75f, 0.75f, 0.f, 1.f}, {0.f, 0.f, 1.f, 1.f}},
  };

  if (auto r = re::setVertices(a->engine.get(), &vertices);
      r != re::Result::Success)
    return 1;

  return 0;
}

int run(App *const a) {
  if (createTriangle(a))
    return 1;

  return !(re::run(a->engine.get()) == re::Result::Success);
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

  return 0;
}
} // namespace demo
