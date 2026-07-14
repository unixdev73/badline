add_subdirectory(objectsAndTextures)
add_subdirectory(snake)
add_subdirectory(multicast)

if (MSVC)
	add_custom_target(copyShaders ALL COMMAND
		${CMAKE_COMMAND} -E copy_directory
		${CMAKE_BINARY_DIR}/src/renderEngine/shaders
		${CMAKE_BINARY_DIR}/demo/objectsAndTextures/Release/shaders
		COMMAND
		${CMAKE_COMMAND} -E copy_directory
		${CMAKE_BINARY_DIR}/src/renderEngine/shaders
		${CMAKE_BINARY_DIR}/demo/objectsAndTextures/Debug/shaders
		COMMAND
		${CMAKE_COMMAND} -E copy_directory
		${CMAKE_BINARY_DIR}/src/renderEngine/shaders
		${CMAKE_BINARY_DIR}/demo/snake/Release/shaders
		COMMAND
		${CMAKE_COMMAND} -E copy_directory
		${CMAKE_BINARY_DIR}/src/renderEngine/shaders
		${CMAKE_BINARY_DIR}/demo/snake/Debug/shaders
	)
else()
	add_custom_target(copyShaders ALL COMMAND
		${CMAKE_COMMAND} -E copy_directory
		${CMAKE_BINARY_DIR}/src/renderEngine/shaders
		${CMAKE_BINARY_DIR}/demo/objectsAndTextures/shaders
		COMMAND
		${CMAKE_COMMAND} -E copy_directory
		${CMAKE_BINARY_DIR}/src/renderEngine/shaders
		${CMAKE_BINARY_DIR}/demo/snake/shaders
	)
endif()

add_dependencies(copyShaders
	vertexShader
	fragmentShader
	fragNoSamplerShader
)
