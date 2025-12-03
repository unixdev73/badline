message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

macro(addBuildTarget targetName)
	message(STATUS "Adding module to target list: ${targetName}")
	list(APPEND BUILD_TARGET_LIST ${targetName})
endmacro()

function(setCompileOptions targetName)
	if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
		target_compile_options(${targetName} PRIVATE
			-Wall -Wextra -Wpedantic -Werror -O0 -g)
	else()
		target_compile_options(${targetName} -O3)
	endif()
endfunction()

if (BUILD_ALL_TARGETS)
	set(BUILD_TARGET_ARG_PARSER "true")
	set(BUILD_TARGET_RENDER_ENGINE "true")
	set(BUILD_TARGET_DEMO "true")
endif()

if (NOT BUILD_TARGET_LIST)
	if (BUILD_TARGET_ARG_PARSER)
		addBuildTarget("argParser")
	endif()

	if (BUILD_TARGET_RENDER_ENGINE)
		addBuildTarget("renderEngine")
	endif()

	if (BUILD_TARGET_DEMO)
		addBuildTarget("demo")
	endif()
endif()

foreach(module IN LISTS BUILD_TARGET_LIST)
	add_subdirectory(${module})
endforeach()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(test)
