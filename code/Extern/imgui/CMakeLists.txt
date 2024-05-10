cmake_minimum_required(VERSION 3.16)

add_library(ImGui)
target_sources(ImGui
	PRIVATE
		1.90.4/imconfig.h
		1.90.4/imgui.h
		1.90.4/imgui_helpers.h
		1.90.4/imgui_internal.h
		1.90.4/imstb_rectpack.h
		1.90.4/imstb_textedit.h
		1.90.4/imstb_truetype.h
		1.90.4/imgui.cpp
		1.90.4/imgui_demo.cpp
		1.90.4/imgui_draw.cpp
		1.90.4/imgui_tables.cpp
		1.90.4/imgui_widgets.cpp
		implot-0.16/implot.h
		implot-0.16/implot_internal.h
		implot-0.16/implot.cpp
		implot-0.16/implot_demo.cpp
		implot-0.16/implot_items.cpp
)

target_compile_features(ImGui PRIVATE cxx_std_23)

target_include_directories(ImGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/1.90.4)
target_include_directories(ImGui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/implot-0.16)
target_compile_definitions(ImGui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
target_compile_definitions(ImGui PUBLIC IMGUI_PANDA)
