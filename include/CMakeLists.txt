configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configure/resources.hpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/badline/resources.hpp @ONLY)
install(DIRECTORY badline DESTINATION include)
