add_executable(sm3tools sm3tools.cpp pcssb.cpp)
target_link_libraries(sm3tools PUBLIC compiler_flags)

add_library(myIO STATIC myIO.cpp)
target_link_libraries(myIO PUBLIC compiler_flags)

target_link_libraries(sm3tools PRIVATE myIO)