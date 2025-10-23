find_package(Vulkan 1.4 COMPONENTS glslangValidator REQUIRED)
find_library(GLFW glfw NAMES glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(vulkanMemoryAllocator vulkanMemoryAllocator.cpp)
target_compile_options(vulkanMemoryAllocator PRIVATE -w)

add_library(renderEngine renderEngine.cpp)

target_link_libraries(renderEngine
	${Vulkan_LIBRARIES}
	${GLFW}
	vulkanMemoryAllocator
)
