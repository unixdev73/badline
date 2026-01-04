cmake_minimum_required(VERSION 3.25)
project(badline LANGUAGES CXX)
include(CTest)

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_STANDARD 23)

if (BUILD_ALL_TARGETS)
	set(BUILD_TARGET_ARG_PARSER "true")
	set(BUILD_TARGET_RENDER_ENGINE "true")
	set(BUILD_TARGET_TESTS "true")
	set(BUILD_TARGET_DEMO "true")
endif()

function(setCompileOptions targetName)
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_compile_options(${targetName} PRIVATE
			-Wall -Wextra -Wpedantic -Werror -O0 -g)
	else()
		target_compile_options(${targetName} -O3)
	endif()
endfunction()

include_directories(/usr/local/include)
include_directories(include)
add_subdirectory(src)
add_subdirectory(doc)

if (BUILD_TARGET_TESTS)
	add_subdirectory(test)
endif()

if (BUILD_TARGET_DEMO)
	add_subdirectory(demo)
endif()
