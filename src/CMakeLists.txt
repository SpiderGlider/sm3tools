add_executable(pcssb pcssb.cpp)
target_compile_features(pcssb PUBLIC cxx_std_17)
set_target_properties(pcssb PROPERTIES CXX_EXTENSIONS OFF)

add_library(myIO STATIC myIO.cpp)
target_compile_features(myIO PRIVATE cxx_std_17)
set_target_properties(myIO PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(pcssb myIO)