if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_compile_options(-Wall -Wextra -Wpedantic -O0 -g)
endif()

if ("${BUILD_TARGETS}" STREQUAL "all")
	set(BUILD_PROD_TARGETS 1)
	set(BUILD_TEST_TARGETS 1)
endif()

if (BUILD_PROD_TARGETS)
	foreach(module IN LISTS PROD_TARGETS)
		add_subdirectory(${module})
	endforeach()
endif()

if (BUILD_TEST_TARGETS)
	foreach(module IN LISTS PROD_TARGETS)
		add_subdirectory(${module}/test)
	endforeach()
endif()

add_executable(run main.cpp)

foreach(module IN LISTS PROD_TARGETS)
	target_link_libraries(run ${module})
endforeach()
