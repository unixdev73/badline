# What is this?

This is a game engine that uses Vulkan as a backend.

# Prerequisites

The project has the following dependencies:

- Vulkan
- GLFW
- GLM

On FreeBSD you can get it all like so:

<p>
sudo pkg install vulkan-headers vulkan-loader vulkan-tools
vulkan-utility-libraries vulkan-validation-layers vulkan-extension-layer
glfw glm
</p>

# Building

<p> cmake --preset dev </p>
<p> cmake --build ./build --parallel </p>
