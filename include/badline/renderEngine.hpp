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

namespace re {
struct VulkanBackend;
struct TransformMatrix;
struct Vertices;
struct Indices;
struct Instances;
struct RenderEngine;
struct Window;

/* Note: Only the render engine has to be destroyed manually.
 * Everything created by the engine will be automatically taken care of :)
 * (Recommendation: wrap engine pointer into unique pointer with destroy
 * function as custom deleter).
 */
void create(RenderEngine **const handle);

void destroy(RenderEngine *const handle);

/* For example, the VulkanBackend will be destroyed automatically
 * by the engine. Use this handle to configure Vulkan related details
 * prior to initialization which does the actual heavy lifting.
 */
bool createBackend(RenderEngine *const handle, VulkanBackend **const p);

bool initialize(RenderEngine *const handle);

bool createWindow(RenderEngine *const handle, Window **const p);

bool createProjection(RenderEngine *const handle, TransformMatrix **const p);

bool setProjection(RenderEngine *const handle, TransformMatrix const *const p);

bool createView(RenderEngine *const handle, TransformMatrix **const p);

bool setView(RenderEngine *const handle, TransformMatrix const *const p);

bool createVertices(RenderEngine *const handle, Vertices **const p);

bool uploadVertices(RenderEngine *const handle);

bool createIndices(RenderEngine *const handle, Indices **const p);

bool uploadIndices(RenderEngine *const handle);

bool createInstances(RenderEngine *const handle, Instances **const p);

bool uploadInstances(RenderEngine *const handle);

bool render(RenderEngine *const handle);

void printErrors(RenderEngine const *const handle);
} // namespace re
