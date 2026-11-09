add_library(marker-dll SHARED marker.cpp)
target_compile_definitions(marker-dll PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
set_target_properties(marker-dll PROPERTIES PREFIX "")
