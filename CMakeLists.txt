cmake_minimum_required(VERSION 3.20)

project(windows_injection_pocs VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_subdirectory(tools/pe-inspect)

if(WIN32)
    add_subdirectory(targets/basic-target)
    add_subdirectory(targets/multithreaded-target)
    add_subdirectory(targets/dll-host)
    add_subdirectory(samples/marker-dll)
else()
    message(STATUS "Windows-specific sample targets are disabled on this host")
endif()
