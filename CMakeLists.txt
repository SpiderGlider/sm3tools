cmake_minimum_required(VERSION 3.15...4.0)

project(sm3tools VERSION 1.0
        DESCRIPTION "Spider-Man 3 File Archive Tools"
        LANGUAGES CXX)

add_executable(pcssb src/pcssb.cpp src/myIO.cpp)

target_compile_features(pcssb PUBLIC cxx_std_17)
set_target_properties(pcssb PROPERTIES CXX_EXTENSIONS OFF)
