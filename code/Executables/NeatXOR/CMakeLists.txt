cmake_minimum_required(VERSION 3.16)

add_executable(NeatXOR)

target_sources(NeatXOR
	PRIVATE
		Precompile.h
		main.cpp
)

target_precompile_headers(NeatXOR PRIVATE Precompile.h)
target_compile_features(NeatXOR PRIVATE cxx_std_23)

target_include_directories(NeatXOR PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatXOR PRIVATE Core)
target_link_libraries(NeatXOR PRIVATE NEAT)

set_property(TARGET NeatXOR PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
