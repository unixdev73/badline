add_executable(run
	main.cpp
)
setCompileOptions(run)

target_link_libraries(run
	argParser
	renderEngine
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/trump.jpg
 ${CMAKE_BINARY_DIR}/demo/assets/trump.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/viking_room.png
 ${CMAKE_BINARY_DIR}/demo/assets/viking_room.png
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/viking_room.obj
 ${CMAKE_BINARY_DIR}/demo/assets/viking_room.obj
 COPYONLY
)
