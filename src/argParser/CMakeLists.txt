add_library(argParser interface.cpp internals.cpp)
add_library(argParserLogger dummyLoggerImpl.cpp)

if (USE_SCOPED_LOGGER)
	message(STATUS "Using scoped logger for argParser")
	list(APPEND argParserLibList scopedLogger)
else()
	message(STATUS "Using dummy logger for argParser")
	list(APPEND argParserLibList argParserLogger)
endif()

target_link_libraries(argParser ${argParserLibList})
