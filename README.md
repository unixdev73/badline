# What is this?

This is a game engine that uses Vulkan as a backend.

# Prerequisites

The project has the following dependencies:

- Vulkan
- GLFW
- GLM
- plantuml, clang-uml (optional)

On FreeBSD you can get it all like so:

<p>
sudo pkg install vulkan-headers vulkan-loader vulkan-tools
vulkan-utility-libraries vulkan-validation-layers vulkan-extension-layer
glfw glm
</p>

# Building

Every module has a separate directory in src.
Every module is independent, and can be build alone.
To not build a specific module simply set the appropriate variable
on the command line to false.

The following options are available:
- BUILD\_TARGET\_ARG\_PARSER
- BUILD\_TARGET\_RENDER\_ENGINE
- BUILD\_TARGET\_TESTS
- BUILD\_TARGET\_DEMO
- BUILD\_TARGET\_DOC

For windows builds the following additional variables must be specified:
- BADLINE\_VULKAN\_SDK\_PATH
- BADLINE\_GLFW\_INCLUDE\_PATH
- BADLINE\_GLFW\_LIB\_PATH

An example configure command on windows might look like this:

```
cmake -B ./badline_build -S ./badline -G"Visual Studio 18 2026" -DBADLINE_VULKAN_SDK_PATH="C:/VulkanSDK/1.4.335.0" -DBADLINE_GLFW_INCLUDE_PATH="C:/Users/c/Downloads/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/include" -DBADLINE_GLFW_LIB_PATH="C:/Users/c/Downloads/glfw-3.4.bin.WIN64/glfw-3.4.bin.WIN64/lib-vc2017/glfw3.lib"
```

Then, to build:

```
cmake --build ./badline_build --config Release
```

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

```
cmake --build ./badline_build --target umlDoc
```

This target requires plantuml and clang-uml.
If the command succeeds, a directory 'diagrams' should appear in
the build dir. Inside there will be diagrams with the svg extension,
which can be viewed with inkscape, for example.
