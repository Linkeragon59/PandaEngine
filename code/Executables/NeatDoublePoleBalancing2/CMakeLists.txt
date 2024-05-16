cmake_minimum_required(VERSION 3.16)

add_executable(NeatDoublePoleBalancing2)

target_sources(NeatDoublePoleBalancing2
	PRIVATE
		Precompile.h
		CartPole.h
		CartPole.cpp
		main.cpp
)

target_precompile_headers(NeatDoublePoleBalancing2 PRIVATE Precompile.h)
target_compile_features(NeatDoublePoleBalancing2 PRIVATE cxx_std_23)

target_include_directories(NeatDoublePoleBalancing2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatDoublePoleBalancing2 PRIVATE Core)
target_link_libraries(NeatDoublePoleBalancing2 PRIVATE Render)
target_link_libraries(NeatDoublePoleBalancing2 PRIVATE ImGui)
target_link_libraries(NeatDoublePoleBalancing2 PRIVATE NEAT)

set_property(TARGET NeatDoublePoleBalancing2 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
