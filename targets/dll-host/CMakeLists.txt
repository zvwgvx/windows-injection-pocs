add_executable(dll-host main.cpp)
target_compile_definitions(dll-host PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
