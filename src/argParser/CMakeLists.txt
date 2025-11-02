add_library(argParserInternals internals.cpp)

add_library(argParser interface.cpp)
target_link_libraries(argParser argParserInternals)
