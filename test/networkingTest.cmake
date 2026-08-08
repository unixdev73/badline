if (UNIX)
  add_executable(blomClientServerLauncher blomServerIntro.cpp)
  target_link_libraries(blomClientServerLauncher badlineNetworking)

  add_test(NAME networkingTest001 COMMAND blomClientServerLauncher)
endif()
