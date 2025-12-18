add_executable(run
	main.cpp
)
setCompileOptions(run)

target_link_libraries(run
	argParser
	renderEngine
)
