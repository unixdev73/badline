add_library(badlineArgParser interface.cpp internals.cpp logs.cpp)
setCompileOptions(badlineArgParser)

install(TARGETS badlineArgParser DESTINATION lib)
