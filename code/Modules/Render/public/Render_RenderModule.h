#pragma once

#include "Core_Module.h"

struct GLFWwindow;

namespace Render
{
	enum class RendererType
	{
		SampleTriangle,
		SampleTexturedRectangle,
		GuiOnly,
		Deferred,
		Count
	};

	class RenderTarget;
	typedef RenderTarget* RenderTargetHandle;

	class RenderCore;

	class RenderModule : public Core::Module
	{
	DECLARE_CORE_MODULE(RenderModule, "Render")

	protected:
		void OnRegister() override;
		void OnUnregister() override;
		
		void OnInitialize() override;
		void OnFinalize() override;

		void OnUpdate(Core::Module::UpdateType aType) override;

	public:
		RenderCore* GetRenderCore() const { return myRenderCore; }

		void RegisterWindow(GLFWwindow* aWindow, RendererType aType);
		void UnregisterWindow(GLFWwindow* aWindow);

		RenderTargetHandle CreateRenderTarget(uint aWidth, uint aHeight, RendererType aType);
		void DestroyRenderTarget(RenderTargetHandle aHandle);
		void* GetUserRenderTargetTexture(RenderTargetHandle aHandle) const;

		void LoadUserTexture(const char* aTexturePath, void*& anOutTextureID, uint& anOutWidth, uint& anOutHeight);
		void UnloadUserTexture(void* aTextureID);

	private:
		RenderCore* myRenderCore = nullptr;
	};
}
