add_executable(objectsAndTextures
	main.cpp
)
setCompileOptions(objectsAndTextures)

target_link_libraries(objectsAndTextures
	argParser
	renderEngine
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/trump.jpg
	${CMAKE_CURRENT_BINARY_DIR}/assets/trump.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/viking_room.png
	${CMAKE_CURRENT_BINARY_DIR}/assets/viking_room.png
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/viking_room.obj
	${CMAKE_CURRENT_BINARY_DIR}/assets/viking_room.obj
 COPYONLY
)
