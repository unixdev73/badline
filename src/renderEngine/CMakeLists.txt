find_package(Vulkan 1.3 COMPONENTS glslangValidator REQUIRED)
find_library(GLFW glfw NAMES glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(vulkanMemoryAllocator vulkanMemoryAllocator.cpp)
target_compile_options(vulkanMemoryAllocator PRIVATE -w)

add_library(renderEngine
	interface.cpp
)

set(renderEngineLibList
	vulkanMemoryAllocator
	${Vulkan_LIBRARIES}
	${GLFW}
)

target_link_libraries(renderEngine ${renderEngineLibList})
