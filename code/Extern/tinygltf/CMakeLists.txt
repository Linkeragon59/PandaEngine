cmake_minimum_required(VERSION 3.16)

add_library(tinyglTF INTERFACE)

target_include_directories(tinyglTF INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/main)
