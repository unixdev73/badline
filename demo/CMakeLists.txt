add_executable(demo
	parseCLI.cpp
	main.cpp
)

target_sources(demo PRIVATE FILE_SET demoParseCLIModule
	TYPE CXX_MODULES
	FILES parseCLI.ixx
)

foreach(module IN LISTS PROD_TARGETS)
	target_link_libraries(demo ${module})
endforeach()
