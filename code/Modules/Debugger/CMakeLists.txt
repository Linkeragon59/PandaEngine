cmake_minimum_required(VERSION 3.16)

add_library(Debugger)
target_sources(Debugger
	PRIVATE
		public/Debugger_Module.h

		private/Debugger_Precompile.h
		private/Debugger_Module.cpp
)

target_precompile_headers(Debugger PRIVATE private/Debugger_Precompile.h)
target_compile_features(Debugger PRIVATE cxx_std_23)

target_include_directories(Debugger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(Debugger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(Debugger PRIVATE Core)
target_link_libraries(Debugger PRIVATE Render)
target_link_libraries(Debugger PRIVATE GLFW)
target_link_libraries(Debugger PRIVATE ImGui)
