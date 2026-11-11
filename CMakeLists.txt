cmake_minimum_required(VERSION 3.20)

project(windows_injection_pocs VERSION 0.4.1 LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "This project supports Windows only.")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project requires Windows x64.")
endif()

if(MINGW)
    # Keep cross-compiled binaries self-contained on a plain Windows x64 host.
    add_link_options(-static -static-libgcc -static-libstdc++)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config} "${CMAKE_BINARY_DIR}/bin")
endforeach()

enable_testing()
find_package(Threads REQUIRED)

add_subdirectory(tools/pe-inspect)
add_subdirectory(tools/process-observer)
add_subdirectory(targets/basic-target)
add_subdirectory(targets/multithreaded-target)
add_subdirectory(tools/dll-host)
add_subdirectory(tools/dll-load-demo)
add_subdirectory(samples/marker-dll)
add_subdirectory(pocs/01-classic-dll-injection)
add_subdirectory(tools/process-memory)

target_link_libraries(multithreaded-target PRIVATE Threads::Threads)

add_test(NAME process-observer-self COMMAND process-observer --self --samples 1)

add_test(NAME pe-inspect-rejects-text COMMAND pe-inspect "${CMAKE_SOURCE_DIR}/README.md")
set_tests_properties(pe-inspect-rejects-text PROPERTIES WILL_FAIL TRUE)
add_test(NAME pe-inspect-valid-exe COMMAND pe-inspect $<TARGET_FILE:basic-target>)
add_test(NAME pe-inspect-valid-dll COMMAND pe-inspect $<TARGET_FILE:marker-dll>)
add_test(NAME basic-target-smoke COMMAND basic-target --interval-ms 100 --duration-sec 1)
add_test(NAME multithreaded-target-smoke COMMAND multithreaded-target --duration-sec 1)
add_test(NAME memory-layout-smoke COMMAND memory-layout)
add_test(NAME dll-loading-smoke COMMAND dll-load-demo $<TARGET_FILE:marker-dll>)
add_test(NAME dll-host-smoke COMMAND dll-host $<TARGET_FILE:marker-dll> --hold-ms 50)

# Shared staging directory for the technique 01 run bundle: the independent
# target, the injector, the payload and the self-test. The files are generated,
# not sources, and are ignored by the repository's binary rules.
set(POC_STAGE_DIR "${CMAKE_SOURCE_DIR}/tools/bin")
file(MAKE_DIRECTORY "${POC_STAGE_DIR}")
add_custom_target(stage-poc01 ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:basic-target>"
            "${POC_STAGE_DIR}/$<TARGET_FILE_NAME:basic-target>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:poc01-injector>"
            "${POC_STAGE_DIR}/$<TARGET_FILE_NAME:poc01-injector>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:poc01-payload>"
            "${POC_STAGE_DIR}/$<TARGET_FILE_NAME:poc01-payload>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:poc01-payload-heartbeat>"
            "${POC_STAGE_DIR}/$<TARGET_FILE_NAME:poc01-payload-heartbeat>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:poc01-selftest>"
            "${POC_STAGE_DIR}/$<TARGET_FILE_NAME:poc01-selftest>"
    DEPENDS basic-target poc01-injector poc01-payload poc01-payload-heartbeat poc01-selftest
    COMMENT "Staging the technique 01 run bundle in tools/bin"
    VERBATIM)
