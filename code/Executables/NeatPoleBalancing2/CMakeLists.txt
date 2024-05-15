cmake_minimum_required(VERSION 3.16)

add_executable(NeatPoleBalancing2)

target_sources(NeatPoleBalancing2
	PRIVATE
		Precompile.h
		main.cpp
)

target_precompile_headers(NeatPoleBalancing2 PRIVATE Precompile.h)
target_compile_features(NeatPoleBalancing2 PRIVATE cxx_std_23)

target_include_directories(NeatPoleBalancing2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatPoleBalancing2 PRIVATE Core)
target_link_libraries(NeatPoleBalancing2 PRIVATE Render)
target_link_libraries(NeatPoleBalancing2 PRIVATE ImGui)
target_link_libraries(NeatPoleBalancing2 PRIVATE NEAT)

set_property(TARGET NeatPoleBalancing2 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
