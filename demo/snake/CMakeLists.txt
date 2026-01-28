add_executable(snake
	main.cpp
)
setCompileOptions(snake)

target_link_libraries(snake
	argParser
	renderEngine
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/trump.jpg
	${CMAKE_CURRENT_BINARY_DIR}/assets/trump.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/schumer.jpg
	${CMAKE_CURRENT_BINARY_DIR}/assets/schumer.jpg
 COPYONLY
)
