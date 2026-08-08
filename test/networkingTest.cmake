if (UNIX)
  add_executable(blomClientServerLauncher blomServerIntro.cpp)
  target_link_libraries(blomClientServerLauncher badline_net)

  add_test(NAME networkingTest001 COMMAND blomClientServerLauncher)
endif()
