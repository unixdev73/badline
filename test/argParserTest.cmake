add_executable(addArgParserFlag addArgParserFlag.cpp)
setCompileOptions(addArgParserFlag)
target_link_libraries(addArgParserFlag PRIVATE argParser)

add_executable(addArgParserOpt addArgParserOpt.cpp)
setCompileOptions(addArgParserOpt)
target_link_libraries(addArgParserOpt PRIVATE argParser)

add_executable(testArgParser testArgParser.cpp)
setCompileOptions(testArgParser)
target_link_libraries(testArgParser PRIVATE argParser)

# Adding new flag "longForm:shortForm"
add_test(NAME addArgParserFlagTest1 COMMAND addArgParserFlag "abc:a")

# Adding the same flag twice
add_test(NAME addArgParserFlagTest2 COMMAND addArgParserFlag "abc:a" "abc:a")
set_tests_properties(addArgParserFlagTest2 PROPERTIES WILL_FAIL true)

# Adding new option "longForm:shortForm"
add_test(NAME addArgParserOptTest1 COMMAND addArgParserOpt "abc:a")

# Adding the same option twice
add_test(NAME addArgParserOptTest2 COMMAND addArgParserOpt "abc:a" "abc:a")
set_tests_properties(addArgParserOptTest2 PROPERTIES WILL_FAIL true)

# Parse unsupported arg
add_test(NAME argParserTest01 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"--hello"
)
set_tests_properties(argParserTest01 PROPERTIES WILL_FAIL true)

add_test(NAME argParserTest02 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-h"
)
set_tests_properties(argParserTest02 PROPERTIES WILL_FAIL true)

# Parse supported flag
add_test(NAME argParserTest03 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"--alpha"
)
add_test(NAME argParserTest04 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-a"
)

# Parse supported flag list
add_test(NAME argParserTest05 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-abc"
)

# Parse supported option
add_test(NAME argParserTest06 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"--file" "/path/to/file.txt"
)
add_test(NAME argParserTest07 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-f" "/path/to/file.txt"
)
add_test(NAME argParserTest08 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"--file=/path/to/my_file_file.txt"
)
add_test(NAME argParserTest09 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-f=/path/to/my-custom_file-file.txt"
)

# Parse supported mixed arg list
add_test(NAME argParserTest10 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-abcf" "/path/to/file.txt"
)
add_test(NAME argParserTest11 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-abcf=/path/to/file.txt"
)

# Parse unsupported mixed arg list
add_test(NAME argParserTest12 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-abcfv=/path/to/file.txt"
)
set_tests_properties(argParserTest12 PROPERTIES WILL_FAIL true)

add_test(NAME argParserTest13 COMMAND testArgParser
	${CMAKE_CURRENT_SOURCE_DIR}/testParserFlags.txt
	${CMAKE_CURRENT_SOURCE_DIR}/testParserOptions.txt
	"-abcfv" "/path/to/file.txt"
)
set_tests_properties(argParserTest13 PROPERTIES WILL_FAIL true)
