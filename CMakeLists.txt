cmake_minimum_required(VERSION 3.25)
project(badline LANGUAGES CXX)
include(CTest)

set(BUILD_TARGET_ARG_PARSER "false" CACHE STRING "Build arg parser module")
set(BUILD_TARGET_RENDER_ENGINE "false" CACHE STRING "Build render engine module")
set(BUILD_TARGET_TESTS "false" CACHE STRING "Build badline tests module")
set(BUILD_TARGET_DEMO "false" CACHE STRING "Build badline demo module")
set(BUILD_TARGET_DOC "false" CACHE STRING "Build badline documentation")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_RPATH ${CMAKE_BUILD_RPATH} /usr/local/lib64)
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_RPATH} /usr/local/lib64)

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

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

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
