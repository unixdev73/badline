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

Every module has a separate directory in src.
Every module is independent, and can be build alone.
To build a specific module simply specify the appropriate variable
on the command line. Available variables can be queried
by taking a look into the CMakePresets.json file.
The 'dev' preset builds everything.

<p> cmake --preset dev -S ./badline </p>
<p> cmake --build ./badline\_build --parallel </p>

# Documentation

There is a saying that good code is self-documenting code.
(I laughed).
Well, while that may be the case, I fear it will be more likely
to find a unicorn than easy to understand self-documenting code.

The doc directory contains a work-in-progress documentation.
It is not exactly "ready" yet, as the project is still evolving heavily.
But, the most important guidelines are already present
in the architecture file. 

The doc module provides a cmake target called **umlDoc**
This target is not built by default.
It generates dependency diagrams of each module
to understand the project more easily.

<p> cmake --build ./badline\_build --target umlDoc </p>

This target requires plantuml and clang-uml.
If the command succeeds, a directory 'diagrams' should appear in
the build dir. Inside there will be diagrams with the svg extension,
which can be viewed with inkscape, for example.
