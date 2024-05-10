cmake_minimum_required(VERSION 3.16)

add_executable(NeatPoleBalancing)

target_sources(NeatPoleBalancing
	PRIVATE
		Precompile.h
		main.cpp
)

target_precompile_headers(NeatPoleBalancing PRIVATE Precompile.h)
target_compile_features(NeatPoleBalancing PRIVATE cxx_std_23)

target_include_directories(NeatPoleBalancing PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatPoleBalancing PRIVATE Core)
target_link_libraries(NeatPoleBalancing PRIVATE Render)
target_link_libraries(NeatPoleBalancing PRIVATE ImGui)
target_link_libraries(NeatPoleBalancing PRIVATE NEAT)

set_property(TARGET NeatPoleBalancing PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
