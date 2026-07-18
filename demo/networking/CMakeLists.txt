add_executable(multicast multicast.cpp)

add_executable(interface_query interface_query.cpp)
target_link_libraries(interface_query badline_net)
