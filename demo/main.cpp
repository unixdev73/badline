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

#include <badline/renderEngine.hpp>
#include <badline/scopedLogger.hpp>
#include "parseCLI.hpp"
#include <iostream>

using sl::inf;

int main(int const argc, char const *const *const argv) {
  try {
    auto logger = sl::createLogger(__func__);
    demo::AppCLI data{};

    inf(logger.get(), "Parsing CLI");
    if (auto error = demo::parseCLI(&data, argv, 1, argc, logger.get()); error)
      return 1;

    inf(logger.get(), "Creating render engine");
    auto engine = re::createRenderEngine("Demo", data.debug);
    if (!engine)
      return 2;

    inf(logger.get(), "Creating window");
    if (auto err = re::createWindow(engine.get(), data.width, data.height); err)
      return 3;

    inf(logger.get(), "Running main loop");
    return re::run(engine.get());
  } catch (std::exception const &e) {
    std::cerr << "Error: Caught exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Error: Caught unknown exception." << std::endl;
  }
}
