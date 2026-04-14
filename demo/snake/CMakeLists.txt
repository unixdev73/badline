add_executable(snake
	main.cpp
)
setCompileOptions(snake)

target_link_libraries(snake
	argParser
	renderEngine
)

if (UNIX)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/trump.jpg
	${CMAKE_CURRENT_BINARY_DIR}/assets/trump.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/schumer.jpg
	${CMAKE_CURRENT_BINARY_DIR}/assets/schumer.jpg
 COPYONLY
)
elseif(MSVC)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/trump.jpg
	${CMAKE_CURRENT_BINARY_DIR}/Release/assets/trump.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/schumer.jpg
	${CMAKE_CURRENT_BINARY_DIR}/Release/assets/schumer.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/trump.jpg
	${CMAKE_CURRENT_BINARY_DIR}/Debug/assets/trump.jpg
 COPYONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/schumer.jpg
	${CMAKE_CURRENT_BINARY_DIR}/Debug/assets/schumer.jpg
 COPYONLY
)
endif()