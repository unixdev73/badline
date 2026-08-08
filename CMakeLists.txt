cmake_minimum_required(VERSION 3.25)
project(badline LANGUAGES CXX)
include(CTest)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(FATAL_ERROR "Detected unsupported system: x64 architecture required")
endif()

if (WIN32)
	message(STATUS "Detected Windows setup")
elseif(UNIX)
	message(STATUS "Detected UNIX setup")
	set(UNIX_DEBUG_CXX_FLAGS -Wall -Wextra -Wpedantic -Werror -O0 -g)
else()
	message(FATAL_ERROR "Detected unsupported system: Only supports UNIX and Windows")
endif()

set(BUILD_TARGET_ARG_PARSER "true" CACHE STRING "Build arg parser module")
set(BUILD_TARGET_RENDER_ENGINE "true" CACHE STRING "Build render engine module")
set(BUILD_TARGET_NETWORKING "true" CACHE STRING "Build networking module")
set(BUILD_TARGET_TESTS "true" CACHE STRING "Build badline tests module")
set(BUILD_TARGET_DEMO "true" CACHE STRING "Build badline demo module")
set(BUILD_TARGET_DOC "true" CACHE STRING "Build badline documentation")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_RPATH ${CMAKE_BUILD_RPATH} /usr/local/lib64)
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} /usr/local/lib64)

set(BADLINE_SHADER_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/badlineShaders)

function(setCompileOptions targetName)
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		if (UNIX)
			target_compile_options(${targetName} PRIVATE ${UNIX_DEBUG_CXX_FLAGS})
		endif()
	else()
		if (UNIX)
			target_compile_options(${targetName} PRIVATE -O3)
		endif()
	endif()
endfunction()

include_directories(/usr/local/include)
include_directories(include)

if (WIN32)
	set(BADLINE_VULKAN_SDK_PATH "" CACHE STRING "Path to Vulkan SDK on Windows")
	set(BADLINE_GLFW_INCLUDE_PATH "" CACHE STRING "Path to GLFW includes on Windows")
	set(BADLINE_GLFW_LIB_PATH "" CACHE STRING "Path to GLFW library on Windows")

	if (NOT BADLINE_VULKAN_SDK_PATH)
		message(FATAL_ERROR "Vulkan SDK path not set; Please set it using -DBADLINE_VULKAN_SDK_PATH")
	endif()

	if (NOT BADLINE_GLFW_INCLUDE_PATH)
		message(FATAL_ERROR "GLFW include path not set; Please set it using -DBADLINE_GLFW_INCLUDE_PATH")
	endif()

	if (NOT BADLINE_GLFW_LIB_PATH)
		message(FATAL_ERROR "GLFW library path not set; Please set it using -DBADLINE_GLFW_LIB_PATH")
	endif()

	include_directories(
		${BADLINE_VULKAN_SDK_PATH}/Include
		${BADLINE_GLFW_INCLUDE_PATH}
	)
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

add_subdirectory(include)
add_subdirectory(src)

if (BUILD_TARGET_DOC)
	add_subdirectory(doc)
endif()

if (BUILD_TARGET_TESTS)
	add_subdirectory(test)
endif()

if (BUILD_TARGET_DEMO)
	add_subdirectory(demo)
endif()
