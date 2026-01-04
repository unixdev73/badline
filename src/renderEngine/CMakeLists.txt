find_package(Vulkan 1.3 COMPONENTS glslangValidator REQUIRED)
find_library(GLFW glfw NAMES glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(renderEngine
	vulkanBackend.cpp
	renderEngine.cpp
	logs.cpp
)
setCompileOptions(renderEngine)
target_compile_options(renderEngine PRIVATE
	-Wno-nullability-extension
	-Wno-nullability-completeness
)

add_library(vulkanAllocator vulkanAllocator.cpp)
target_compile_options(vulkanAllocator PRIVATE
	-Wno-nullability-extension
	-Wno-nullability-completeness
)

set(renderEngineLibList
	${Vulkan_LIBRARIES}
	vulkanAllocator
	-pthread
	${GLFW}
)

target_link_libraries(renderEngine ${renderEngineLibList})
add_subdirectory(shaders)
