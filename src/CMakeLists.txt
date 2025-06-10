add_executable(pcssb pcssb.cpp)
target_compile_features(pcssb PUBLIC cxx_std_17)
set_target_properties(pcssb PROPERTIES CXX_EXTENSIONS OFF)

if (MSVC)
    target_compile_options(pcssb PRIVATE /W4)
else()
    # for gcc / clang
    target_compile_options(pcssb PRIVATE -Wall -Wextra -pedantic)
endif()

add_library(myIO STATIC myIO.cpp)
target_compile_features(myIO PRIVATE cxx_std_17)
set_target_properties(myIO PROPERTIES CXX_EXTENSIONS OFF)

if (MSVC)
    target_compile_options(myIO PRIVATE /W4)
else()
    # for gcc / clang
    target_compile_options(myIO PRIVATE -Wall -Wextra -pedantic)
endif()

target_link_libraries(pcssb myIO)