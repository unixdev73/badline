add_library(scopedLogger)

target_sources(scopedLogger
	PRIVATE
		private/scopedLogger.cpp
	PUBLIC
		scopedLogger.cpp
	PUBLIC
		FILE_SET scopedLoggerModule
		TYPE CXX_MODULES
		FILES scopedLogger.ixx
)
