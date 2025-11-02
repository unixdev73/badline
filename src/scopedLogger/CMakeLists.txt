add_library(scopedLoggerInternals internals.cpp)

add_library(scopedLogger interface.cpp)
target_link_libraries(scopedLogger scopedLoggerInternals)
