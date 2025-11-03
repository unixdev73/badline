find_package(Vulkan 1.3 COMPONENTS glslangValidator REQUIRED)
find_library(GLFW glfw NAMES glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(vulkanMemoryAllocator vulkanMemoryAllocator.cpp)
target_compile_options(vulkanMemoryAllocator PRIVATE -w)

add_library(renderEngineLogger dummyLoggerImpl.cpp)

add_library(renderEngine
	interface.cpp
)

set(renderEngineLibList
	vulkanMemoryAllocator
	${Vulkan_LIBRARIES}
	${GLFW}
)

if (USE_SCOPED_LOGGER)
	message(STATUS "Using scoped logger for renderEngine")
	list(APPEND renderEngineLibList scopedLogger)
else()
	message(STATUS "Using dummy logger for renderEngine")
	list(APPEND renderEngineLibList renderEngineLogger)
endif()

target_link_libraries(renderEngine ${renderEngineLibList})
