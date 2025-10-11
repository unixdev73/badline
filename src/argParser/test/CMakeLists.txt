add_executable(testSplitter splitter.cpp)
target_link_libraries(testSplitter argParser)

add_test(NAME splitTest0001 COMMAND testSplitter "--username=test" "--username" "test")
add_test(NAME splitTest0002 COMMAND testSplitter "=" "" "")
add_test(NAME splitTest0003 COMMAND testSplitter "l=" "l" "")
add_test(NAME splitTest0004 COMMAND testSplitter "=r" "" "r")
