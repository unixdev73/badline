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

namespace re {
/* This communicates with the GPU, manages the window, executes draws, etc... */
struct VulkanBackend;

/* This is the main aggregator structure */
struct RenderEngine;

/* Note: Only the render engine has to be destroyed manually.
 * Everything created by the engine will be automatically taken care of :)
 */
void create(RenderEngine **const handle);

/* Recommendation: wrap engine pointer into unique pointer with the destroy
 * function as custom deleter.
 */
void destroy(RenderEngine *const handle);

/* This will create an instance of the vulkan backend.
 * You should only create it once.
 * It controls all low-level gpu-related things, but it also
 * manages the window. But, it does not initialize glfw for you,
 * so make sure to do that yourself!
 */
bool createBackend(RenderEngine *const handle, VulkanBackend **const p);

void enableInfoLogs(RenderEngine *const handle);

void enableWarnings(RenderEngine *const handle);

/* The render engine module stores all errors at all levels,
 * so when a function deep within the module fails,
 * the full trace is included alongside the messages.
 * You're welcome :)
 */
void printLogs(RenderEngine const *const handle);
} // namespace re
