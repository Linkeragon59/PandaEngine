cmake_minimum_required(VERSION 3.16)

add_executable(NeatDoubleCartPole)

target_sources(NeatDoubleCartPole
	PRIVATE
		Precompile.h
		CartPole.h
		CartPole.cpp
		main.cpp
)

target_precompile_headers(NeatDoubleCartPole PRIVATE Precompile.h)
target_compile_features(NeatDoubleCartPole PRIVATE cxx_std_23)

target_include_directories(NeatDoubleCartPole PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatDoubleCartPole PRIVATE Core)
target_link_libraries(NeatDoubleCartPole PRIVATE Render)
target_link_libraries(NeatDoubleCartPole PRIVATE ImGui)
target_link_libraries(NeatDoubleCartPole PRIVATE NEAT)

set_property(TARGET NeatDoubleCartPole PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
