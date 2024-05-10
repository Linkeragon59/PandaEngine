cmake_minimum_required(VERSION 3.16)

add_library(Render)
target_sources(Render
	PRIVATE
		public/Render_EntityRenderComponent.h
		public/Render_Fonts.h
		public/Render_RenderModule.h

		private/Render_Precompile.h
		private/Render_Precompile.cpp
		private/Render_Buffer.h
		private/Render_Buffer.cpp
		private/Render_Camera.h
		private/Render_Camera.cpp
		private/Render_Debug.h
		private/Render_Debug.cpp
		private/Render_Device.h
		private/Render_Device.cpp
		private/Render_DeferredRenderer.h
		private/Render_DeferredRenderer.cpp
		private/Render_EntityRenderComponent.cpp
		private/Render_glTFModel.h
		private/Render_glTFModel.cpp
		private/Render_Gui.h
		private/Render_Gui.cpp
		private/Render_GuiRenderer.h
		private/Render_GuiRenderer.cpp
		private/Render_Helpers.h
		private/Render_Helpers.cpp
		private/Render_Image.h
		private/Render_Image.cpp
		private/Render_ImGuiHelper.h
		private/Render_ImGuiHelper.cpp
		private/Render_Model.h
		private/Render_Model.cpp
		private/Render_PointLightsSet.h
		private/Render_PointLightsSet.cpp
		private/Render_RenderCore.h
		private/Render_RenderCore.cpp
		private/Render_Renderer.h
		private/Render_Renderer.cpp
		private/Render_RenderModule.cpp
		private/Render_RenderTarget.h
		private/Render_RenderTarget.cpp
		private/Render_Resource.h
		private/Render_Resource.cpp
		private/Render_SampleTexturedRectangleRenderer.h
		private/Render_SampleTexturedRectangleRenderer.cpp
		private/Render_SampleTriangleRenderer.h
		private/Render_SampleTriangleRenderer.cpp
		private/Render_ShaderHelpers.h
		private/Render_ShaderHelpers.cpp
		private/Render_SwapChain.h
		private/Render_SwapChain.cpp
)

target_precompile_headers(Render PRIVATE private/Render_Precompile.h)
target_compile_features(Render PRIVATE cxx_std_23)

target_include_directories(Render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/public)
target_include_directories(Render PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private)

target_link_libraries(Render PUBLIC VulkanLib)

target_link_libraries(Render PRIVATE Core)
target_link_libraries(Render PRIVATE VulkanMemoryAllocator)
target_link_libraries(Render PRIVATE VulkanInclude)
target_link_libraries(Render PRIVATE GLFW)
target_link_libraries(Render PRIVATE ImGui)
target_link_libraries(Render PRIVATE tinyglTF)
