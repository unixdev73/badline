add_executable(testArgParser testArgParser.cpp)
target_link_libraries(testArgParser PRIVATE argParser)

# Unknown option
add_test(NAME argParserTest01 COMMAND testArgParser "--hello")
set_tests_properties(argParserTest01 PROPERTIES WILL_FAIL true)

# Free value count
add_test(NAME argParserTest02 COMMAND testArgParser "freeValue")
set_tests_properties(argParserTest02 PROPERTIES WILL_FAIL true)

# Flag list
add_test(NAME argParserTest03 COMMAND testArgParser "-abcdef")

# Arg list
add_test(NAME argParserTest04 COMMAND testArgParser "-abcdefg")
set_tests_properties(argParserTest04 PROPERTIES WILL_FAIL true)

# Arg list
add_test(NAME argParserTest05 COMMAND testArgParser "-abcdefg" "555")
add_test(NAME argParserTest06 COMMAND testArgParser "-abcdefg=555")

# Long option
add_test(NAME argParserTest07 COMMAND testArgParser "--optionG=val")
add_test(NAME argParserTest08 COMMAND testArgParser "--optionG" "val")
