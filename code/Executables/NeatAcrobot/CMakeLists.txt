cmake_minimum_required(VERSION 3.16)

add_executable(NeatAcrobot)

target_sources(NeatAcrobot
	PRIVATE
		Precompile.h
		Acrobot.h
		Acrobot.cpp
		main.cpp
)

target_precompile_headers(NeatAcrobot PRIVATE Precompile.h)
target_compile_features(NeatAcrobot PRIVATE cxx_std_23)

target_include_directories(NeatAcrobot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NeatAcrobot PRIVATE Core)
target_link_libraries(NeatAcrobot PRIVATE Render)
target_link_libraries(NeatAcrobot PRIVATE ImGui)
target_link_libraries(NeatAcrobot PRIVATE NEAT)

set_property(TARGET NeatAcrobot PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
