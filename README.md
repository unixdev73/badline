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

# TODO

## Increase test coverage

## Refactor logger interface

Make the log record comprise of multiple fragments:

A B C D E

A - date
B - time
C - log level
D - function name
E - message

Make all the fragments configurable:

addDateFieldToLogTemplate()
setDateComponentSeparator()

addTimeFieldToLogTemplate()
setTimeComponentSeparator()
addTimeHourComponent()
addTimeMinuteComponent()
addTimeSecondComponent()
addTimeMillisecondComponent()

addLogLevelFieldToLogTemplate()
addFunctionNameFieldToLogTemplate()
addMessageFieldToLogTemplate()

## Refactor arg parser

Add strict / lenient parsing mode
Refactor parsing algorithm to use state tracking

## Expand render engine

Add vulkan memory allocator
Create command pools
Allocate command buffers
Create image views
Write shaders
Refactor optimal GPU selection
