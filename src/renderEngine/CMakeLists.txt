if (UNIX)
	find_package(Vulkan 1.3 COMPONENTS glslangValidator REQUIRED)
	find_package(glfw3 3.4 REQUIRED)
	find_package(glm REQUIRED)
	set(THREAD_LIBRARY "-pthread")
elseif(WIN32)
	set(Vulkan_DIR ${BADLINE_VULKAN_SDK_PATH})
	find_package(Vulkan 1.3 COMPONENTS glslangValidator REQUIRED)
	add_library(glfw SHARED IMPORTED)
	set_target_properties(glfw PROPERTIES
		IMPORTED_LOCATION ${BADLINE_GLFW_LIB_PATH}
		IMPORTED_IMPLIB ${BADLINE_GLFW_LIB_PATH}
		INTERFACE_INCLUDE_DIRECTORIES ${BADLINE_GLFW_INCLUDE_PATH}
	)
endif()

add_library(badlineRenderEngine
	vulkanBackend.cpp
	renderEngine.cpp
	fontlessText.cpp
	logs.cpp
)

setCompileOptions(badlineRenderEngine)

add_library(vulkanAllocator vulkanAllocator.cpp)

if (UNIX)
	target_compile_options(badlineRenderEngine PRIVATE
		-Wno-nullability-extension
		-Wno-nullability-completeness
	)

	target_compile_options(vulkanAllocator PRIVATE
		-Wno-nullability-extension
		-Wno-nullability-completeness
	)
endif()

set(renderEngineLibList
	${Vulkan_LIBRARIES}
	vulkanAllocator
	${THREAD_LIBRARY}
	glfw
)

target_link_libraries(badlineRenderEngine ${renderEngineLibList})
add_subdirectory(shaders)

install(TARGETS badlineRenderEngine DESTINATION lib)
