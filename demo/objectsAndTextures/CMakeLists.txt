add_executable(objectsAndTextures
	main.cpp
)
setCompileOptions(objectsAndTextures)

target_link_libraries(objectsAndTextures
	argParser
	renderEngine
)

if (UNIX)
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
elseif(MSVC)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/trump.jpg
	${CMAKE_CURRENT_BINARY_DIR}/Release/assets/trump.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/viking_room.png
	${CMAKE_CURRENT_BINARY_DIR}/Release/assets/viking_room.png
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/viking_room.obj
	${CMAKE_CURRENT_BINARY_DIR}/Release/assets/viking_room.obj
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/trump.jpg
	${CMAKE_CURRENT_BINARY_DIR}/Debug/assets/trump.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/viking_room.png
	${CMAKE_CURRENT_BINARY_DIR}/Debug/assets/viking_room.png
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/viking_room.obj
	${CMAKE_CURRENT_BINARY_DIR}/Debug/assets/viking_room.obj
 COPYONLY
)
endif()