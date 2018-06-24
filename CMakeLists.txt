cmake_minimum_required(VERSION 2.8)
project(lz)

add_library(liblz liblz/lz.cpp liblz/lz.h)

add_executable(crunch source/crunch.cpp)
add_executable(decrunch source/decrunch.cpp)

target_link_libraries(crunch liblz)
target_link_libraries(decrunch liblz)
