add_executable(pcssb pcssb.cpp myIO.cpp)

target_compile_features(pcssb PUBLIC cxx_std_17)
set_target_properties(pcssb PROPERTIES CXX_EXTENSIONS OFF)