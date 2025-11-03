cmake_minimum_required(VERSION 4.1.2)
project(badline LANGUAGES CXX)
include(CTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_STANDARD 20)

set(PROD_TARGETS
	"scopedLogger"
	"renderEngine"
	"argParser"
)

include_directories(/usr/local/include)
include_directories(include)
add_subdirectory(src)
add_subdirectory(demo)
