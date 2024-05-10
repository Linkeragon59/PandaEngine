cmake_minimum_required(VERSION 3.16)

add_library(RapidJSON INTERFACE)

target_include_directories(RapidJSON INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/master/include)
