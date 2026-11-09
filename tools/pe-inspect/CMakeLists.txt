add_executable(pe-inspect main.cpp)

if(MSVC)
    target_compile_options(pe-inspect PRIVATE /W4 /permissive-)
else()
    target_compile_options(pe-inspect PRIVATE -Wall -Wextra -Wpedantic)
endif()
