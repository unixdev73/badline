add_executable(demo
	parseCLI.cpp
	main.cpp
)

foreach(module IN LISTS PROD_TARGETS)
	target_link_libraries(demo ${module})
endforeach()
