cmake_minimum_required(VERSION 3.20)

project(windows_injection_pocs VERSION 0.2.0 LANGUAGES CXX)

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
add_subdirectory(tools/pe-inspect)

add_test(NAME pe-inspect-rejects-text COMMAND pe-inspect "${CMAKE_SOURCE_DIR}/README.md")
set_tests_properties(pe-inspect-rejects-text PROPERTIES WILL_FAIL TRUE)

if(WIN32)
    find_package(Threads REQUIRED)

    add_subdirectory(targets/basic-target)
    add_subdirectory(targets/multithreaded-target)
    add_subdirectory(targets/dll-host)
    add_subdirectory(samples/marker-dll)
    add_subdirectory(techniques/01-dll-loading)
    add_subdirectory(techniques/02-process-memory)

    target_link_libraries(multithreaded-target PRIVATE Threads::Threads)

    add_test(NAME pe-inspect-valid-exe COMMAND pe-inspect $<TARGET_FILE:basic-target>)
    add_test(NAME pe-inspect-valid-dll COMMAND pe-inspect $<TARGET_FILE:marker-dll>)
    add_test(NAME basic-target-smoke COMMAND basic-target --interval-ms 100 --duration-sec 1)
    add_test(NAME multithreaded-target-smoke COMMAND multithreaded-target --duration-sec 1)
    add_test(NAME memory-layout-smoke COMMAND memory-layout)
    add_test(NAME dll-loading-smoke COMMAND dll-load-demo $<TARGET_FILE:marker-dll>)
    add_test(NAME dll-host-smoke COMMAND dll-host $<TARGET_FILE:marker-dll> --hold-ms 50)
else()
    message(STATUS "Windows-specific sample targets are disabled on this host")
endif()
