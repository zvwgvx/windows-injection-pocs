add_executable(memory-layout code/memory-layout.cpp)
add_executable(memory-regions code/regions.cpp)
target_compile_definitions(memory-layout PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_definitions(memory-regions PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
