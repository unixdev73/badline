find_package(Vulkan 1.3 COMPONENTS glslangValidator REQUIRED)
find_library(GLFW glfw NAMES glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(vulkanMemoryAllocator allocator.cpp)
target_compile_options(vulkanMemoryAllocator PRIVATE -w)

add_library(renderEngine
	interface.cpp
	vkresult.cpp
	instance.cpp
	engine.cpp
	device.cpp
	shader.cpp
	window.cpp
)
setCompileOptions(renderEngine)

set(renderEngineLibList
	vulkanMemoryAllocator
	${Vulkan_LIBRARIES}
	${GLFW}
)

target_link_libraries(renderEngine ${renderEngineLibList})

set(vertIn ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.vert)
set(fragIn ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.frag)
set(shaderOutDir ${CMAKE_BINARY_DIR}/demo/shaders)
set(vertOut ${shaderOutDir}/vertex.spv)
set(fragOut ${shaderOutDir}/fragment.spv)

file(MAKE_DIRECTORY ${shaderOutDir})

add_custom_command(OUTPUT ${vertOut} ${fragOut}
	COMMAND glslangValidator -V -S vert ${vertIn} -o ${vertOut}
	COMMAND glslangValidator -V -S frag ${fragIn} -o ${fragOut}
	DEPENDS ${vertIn} ${fragIn}
	COMMENT "Compiling shaders"
)

add_custom_target(shaders ALL DEPENDS ${vertOut} ${fragOut})
