cmake_minimum_required(VERSION 3.16)

add_executable(NeatCartPole)

target_sources(NeatCartPole
	PRIVATE
		Precompile.h
		CartPole.h
		CartPole.cpp
		main.cpp
)

target_precompile_headers(NeatCartPole PRIVATE Precompile.h)
target_compile_features(NeatCartPole PRIVATE cxx_std_23)

target_include_directories(NeatCartPole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatCartPole PRIVATE Core)
target_link_libraries(NeatCartPole PRIVATE Render)
target_link_libraries(NeatCartPole PRIVATE ImGui)
target_link_libraries(NeatCartPole PRIVATE NEAT)

set_property(TARGET NeatCartPole PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
