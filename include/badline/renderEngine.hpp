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

#pragma once

#include <memory>

namespace re::Result {
constexpr int Success = 0;
constexpr int ErrorNullptrParameter = 1;
constexpr int ErrorFailedToInitGLFW = 2;
constexpr int ErrorMemoryAllocationFailure = 3;
constexpr int ErrorVulkanInstanceCreationFailure = 4;
constexpr int ErrorNoVulkanDevicesAvailable = 5;
constexpr int ErrorVulkanDeviceCreationFailure = 6;
constexpr int ErrorVulkanWindowCreationFailure = 7;
} // namespace re::Result

namespace re {
struct RenderEngineT;
using RenderEngine = RenderEngineT *;

using UniqueRenderEngine =
    std::unique_ptr<RenderEngineT, void (*)(RenderEngine const)>;

int createRenderEngine(RenderEngine *const, std::string const &appName,
                       bool debug);
void destroyRenderEngine(RenderEngine const);
UniqueRenderEngine createRenderEngine(std::string const &appName, bool debug);

int createWindow(RenderEngine const, uint32_t width, uint32_t height);

int run(RenderEngine const);
} // namespace re
