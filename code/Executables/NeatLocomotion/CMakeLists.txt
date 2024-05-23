cmake_minimum_required(VERSION 3.16)

add_executable(NeatLocomotion)

target_sources(NeatLocomotion
	PRIVATE
		Precompile.h
		Character.h
		Character.cpp
		main.cpp
)

target_precompile_headers(NeatLocomotion PRIVATE Precompile.h)
target_compile_features(NeatLocomotion PRIVATE cxx_std_23)

target_include_directories(NeatLocomotion PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatLocomotion PRIVATE Core)
target_link_libraries(NeatLocomotion PRIVATE Render)
target_link_libraries(NeatLocomotion PRIVATE ImGui)
target_link_libraries(NeatLocomotion PRIVATE NEAT)

set_property(TARGET NeatLocomotion PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
