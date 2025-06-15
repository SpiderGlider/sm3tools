add_executable(sm3tools sm3tools.cpp pcssb.cpp)
target_compile_features(sm3tools PUBLIC cxx_std_17)
set_target_properties(sm3tools PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
  target_compile_options(sm3tools PRIVATE /W4)
else()
  target_compile_options(sm3tools PRIVATE -Wall -Wextra -pedantic)
endif()


add_library(myIO STATIC myIO.cpp)
target_compile_features(myIO PRIVATE cxx_std_17)
set_target_properties(myIO PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
  target_compile_options(myIO PRIVATE /W4)
else()
  target_compile_options(myIO PRIVATE -Wall -Wextra -pedantic)
endif()


target_link_libraries(sm3tools PRIVATE myIO)