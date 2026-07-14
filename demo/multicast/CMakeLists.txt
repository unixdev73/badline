add_executable(multicast main.cpp server.cpp client.cpp)
target_link_libraries(multicast badline_net)
