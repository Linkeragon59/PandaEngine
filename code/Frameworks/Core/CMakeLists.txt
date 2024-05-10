cmake_minimum_required(VERSION 3.16)

add_library(Core)
target_sources(Core
	PRIVATE
		public/Core_Assert.h
		public/Core_CommandLine.h
		public/Core_Defines.h
		public/Core_Entity.h
		public/Core_EntityCameraComponent.h
		public/Core_EntityModule.h
		public/Core_EntityTransformComponent.h
		public/Core_Facade.h
		public/Core_File.h
		public/Core_glm.h
		public/Core_InputModule.h
		public/Core_Log.h
		public/Core_Module.h
		public/Core_SlotArray.h
		public/Core_SharedPtr.h
		public/Core_Thread.h
		public/Core_TimeModule.h
		public/Core_Utils.h
		public/Core_WindowModule.h
		public/glm.natvis

		private/Core_Precompile.h
		private/Core_Assert.cpp
		private/Core_CommandLine.cpp
		private/Core_EntityCameraComponent.cpp
		private/Core_EntityModule.cpp
		private/Core_Facade.cpp
		private/Core_File.cpp
		private/Core_InputModule.cpp
		private/Core_Log.cpp
		private/Core_Module.cpp
		private/Core_ModuleManager.h
		private/Core_ModuleManager.cpp
		private/Core_Thread.cpp
		private/Core_TimeModule.cpp
		private/Core_Utils.cpp
		private/Core_WindowModule.cpp
)

target_precompile_headers(Core PRIVATE private/Core_Precompile.h)
target_compile_features(Core PRIVATE cxx_std_23)

target_include_directories(Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(Core PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(Core PUBLIC GLM)

target_link_libraries(Core PRIVATE GLFW)
