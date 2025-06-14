add_executable(sm3tools sm3tools.cpp pcssb.cpp)
target_compile_features(sm3tools PUBLIC cxx_std_17)
set_target_properties(sm3tools PROPERTIES CXX_EXTENSIONS OFF)

add_library(myIO STATIC myIO.cpp)
target_compile_features(myIO PRIVATE cxx_std_17)
set_target_properties(myIO PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(sm3tools myIO)