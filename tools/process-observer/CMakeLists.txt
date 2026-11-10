add_executable(process-observer main.cpp)
target_compile_definitions(process-observer PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
