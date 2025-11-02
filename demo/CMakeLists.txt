add_executable(demo)

target_sources(demo
	PRIVATE
		parseCLI.cpp
		main.cpp
	PRIVATE
		FILE_SET demoParseCLIModule
		TYPE CXX_MODULES
		FILES parseCLI.ixx
)

foreach(module IN LISTS PROD_TARGETS)
	target_link_libraries(demo ${module})
endforeach()
