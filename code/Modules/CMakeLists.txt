cmake_minimum_required(VERSION 3.16)

add_subdirectory(Render)
set_target_properties(Render PROPERTIES FOLDER "Modules")

add_subdirectory(Debugger)
set_target_properties(Debugger PROPERTIES FOLDER "Modules")
