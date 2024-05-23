cmake_minimum_required(VERSION 3.16)

add_subdirectory(NeatAcrobot)
set_target_properties(NeatAcrobot PROPERTIES FOLDER "Executables")

add_subdirectory(NeatCartPole)
set_target_properties(NeatCartPole PROPERTIES FOLDER "Executables")

add_subdirectory(NeatDoubleCartPole)
set_target_properties(NeatDoubleCartPole PROPERTIES FOLDER "Executables")

add_subdirectory(NeatLocomotion)
set_target_properties(NeatLocomotion PROPERTIES FOLDER "Executables")

add_subdirectory(NeatXOR)
set_target_properties(NeatXOR PROPERTIES FOLDER "Executables")
