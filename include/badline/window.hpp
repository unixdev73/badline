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
struct Window;

bool setResolution(Window *const handle, unsigned const w, unsigned const h);

bool setTitle(Window *const handle, char const *const p);

bool open(Window *const handle, VulkanBackend *const p);

bool resize(Window *const handle);

bool getWidth(Window const *const handle);

bool getHeight(Window const *const handle);

bool getTitle(Window const *const handle);

bool isOpen(Window const *const handle, bool *const open);

bool isKeyPressed(Window const *const handle,
                  int const key,
                  bool *const isPressed);

bool close(Window const *const handle);
} // namespace re
