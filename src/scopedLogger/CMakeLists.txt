add_library(scopedLogger scopedLogger.cpp)

target_sources(scopedLogger PUBLIC FILE_SET scopedLoggerModule
	TYPE CXX_MODULES
	FILES scopedLogger.ixx
)
