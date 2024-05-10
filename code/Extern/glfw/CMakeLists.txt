cmake_minimum_required(VERSION 3.16)

add_library(GLFW)
target_sources(GLFW
	PRIVATE
		3.3.9/include/GLFW/glfw3.h
		3.3.9/include/GLFW/glfw3native.h
		3.3.9/src/internal.h
		3.3.9/src/mappings.h
		3.3.9/src/win32_platform.h
		3.3.9/src/win32_joystick.h
		3.3.9/src/wgl_context.h
		3.3.9/src/egl_context.h
		3.3.9/src/osmesa_context.h
		3.3.9/src/context.c
		3.3.9/src/init.c
		3.3.9/src/input.c
		3.3.9/src/monitor.c
		3.3.9/src/vulkan.c
		3.3.9/src/window.c
		3.3.9/src/win32_init.c
		3.3.9/src/win32_joystick.c
		3.3.9/src/win32_monitor.c
		3.3.9/src/win32_time.c
		3.3.9/src/win32_thread.c
		3.3.9/src/win32_window.c
		3.3.9/src/wgl_context.c
		3.3.9/src/egl_context.c
		3.3.9/src/osmesa_context.c
)

target_compile_features(GLFW PRIVATE cxx_std_23)
target_compile_definitions(GLFW PRIVATE _GLFW_WIN32)
target_compile_options(GLFW PRIVATE -wd4100 -wd4152 -wd4201 -wd4244 -wd4706 -wd4996)

target_include_directories(GLFW PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/3.3.9/include)
target_compile_definitions(GLFW PUBLIC GLFW_PANDA)
