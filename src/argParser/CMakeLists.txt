add_library(argParser argParser.cpp)

target_sources(argParser PUBLIC FILE_SET argParserModule
	TYPE CXX_MODULES
	FILES argParser.ixx
)

include(testSplitter.cmake)
