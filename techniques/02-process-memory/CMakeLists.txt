add_executable(memory-layout main.cpp)
add_executable(memory-regions regions.cpp)
target_compile_definitions(memory-layout PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_definitions(memory-regions PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
