add_executable(poc01-injector code/poc01-injector.cpp)
add_library(poc01-payload SHARED code/poc01-payload.cpp)
add_library(poc01-payload-heartbeat SHARED code/poc01-payload-heartbeat.cpp)
add_executable(poc01-selftest code/poc01-selftest.cpp)

target_compile_definitions(poc01-injector PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_definitions(poc01-payload PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_definitions(poc01-payload-heartbeat PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_definitions(poc01-selftest PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

target_link_libraries(poc01-injector PRIVATE shell32)
target_link_libraries(poc01-selftest PRIVATE shell32)
set_target_properties(poc01-payload PROPERTIES PREFIX "")
set_target_properties(poc01-payload-heartbeat PROPERTIES PREFIX "")

add_dependencies(poc01-selftest poc01-injector poc01-payload basic-target)

add_test(NAME dll-injection-smoke
         COMMAND poc01-selftest
                 --target $<TARGET_FILE:basic-target>
                 --injector $<TARGET_FILE:poc01-injector>
                 --payload $<TARGET_FILE:poc01-payload>)
set_tests_properties(dll-injection-smoke PROPERTIES TIMEOUT 90)
