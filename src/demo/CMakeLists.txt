add_executable(demo
	main.cpp
)
setCompileOptions(demo)

target_link_libraries(demo
	argParser
	renderEngine
)
