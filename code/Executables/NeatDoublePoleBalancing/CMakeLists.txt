cmake_minimum_required(VERSION 3.16)

add_executable(NeatDoublePoleBalancing)

target_sources(NeatDoublePoleBalancing
	PRIVATE
		Precompile.h
		CartPole.h
		CartPole.cpp
		main.cpp
)

target_precompile_headers(NeatDoublePoleBalancing PRIVATE Precompile.h)
target_compile_features(NeatDoublePoleBalancing PRIVATE cxx_std_23)

target_include_directories(NeatDoublePoleBalancing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatDoublePoleBalancing PRIVATE Core)
target_link_libraries(NeatDoublePoleBalancing PRIVATE Render)
target_link_libraries(NeatDoublePoleBalancing PRIVATE ImGui)
target_link_libraries(NeatDoublePoleBalancing PRIVATE NEAT)

set_property(TARGET NeatDoublePoleBalancing PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
