cmake_minimum_required(VERSION 3.15...4.0)

project(sm3tools VERSION 0.1.0
        DESCRIPTION "Spider-Man 3 File Archive Tools"
        LANGUAGES CXX)

#compiler options consolidated into compiler_flags interface library
add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE cxx_std_17)

#don't think this would work in this case
#set_target_properties(compiler_flags PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(compiler_flags INTERFACE /W4)
else()
    target_compile_options(compiler_flags INTERFACE -Wall -Wextra -pedantic)
endif()

add_subdirectory("src" "bin")