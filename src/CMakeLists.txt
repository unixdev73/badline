if (BUILD_TARGET_ARG_PARSER)
	add_subdirectory(argParser)
endif()

if (BUILD_TARGET_RENDER_ENGINE)
	add_subdirectory(renderEngine)
endif()

if (BUILD_TARGET_NETWORKING)
	add_subdirectory(networking)
endif()
