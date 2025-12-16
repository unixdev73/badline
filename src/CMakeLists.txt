macro(addBuildTarget targetName)
	message(STATUS "Adding module to target list: ${targetName}")
	list(APPEND BUILD_TARGET_LIST ${targetName})
endmacro()

if (NOT BUILD_TARGET_LIST)
	if (BUILD_TARGET_ARG_PARSER)
		addBuildTarget("argParser")
	endif()

	if (BUILD_TARGET_RENDER_ENGINE)
		addBuildTarget("renderEngine")
	endif()
endif()

foreach(module IN LISTS BUILD_TARGET_LIST)
	add_subdirectory(${module})
endforeach()
