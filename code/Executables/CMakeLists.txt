cmake_minimum_required(VERSION 3.16)

add_subdirectory(NeatPoleBalancing)
set_target_properties(NeatPoleBalancing PROPERTIES FOLDER "Executables")

add_subdirectory(NeatPoleBalancing2)
set_target_properties(NeatPoleBalancing2 PROPERTIES FOLDER "Executables")

add_subdirectory(NeatXOR)
set_target_properties(NeatXOR PROPERTIES FOLDER "Executables")
