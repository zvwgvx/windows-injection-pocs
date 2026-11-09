add_executable(dll-load-demo code/normal-load.cpp)
target_compile_definitions(dll-load-demo PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

add_executable(poc01-target code/target.cpp)
add_executable(poc01-injector code/injector.cpp)
add_library(poc01-payload SHARED code/payload.cpp)

target_compile_definitions(poc01-target PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_compile_definitions(poc01-injector PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

set_target_properties(poc01-target PROPERTIES OUTPUT_NAME target)
set_target_properties(poc01-injector PROPERTIES OUTPUT_NAME injector)
set_target_properties(poc01-payload PROPERTIES OUTPUT_NAME payload PREFIX "")

foreach(item poc01-target poc01-injector poc01-payload)
    set_target_properties(${item} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/01-dll-loading"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/01-dll-loading"
    )
    foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
        set_target_properties(${item} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/bin/01-dll-loading"
            LIBRARY_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/bin/01-dll-loading"
        )
    endforeach()
endforeach()

add_dependencies(poc01-target poc01-payload)
