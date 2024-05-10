#include "Render_RenderModule.h"

#include "Render_Renderer.h"

#include "GLFW/glfw3.h"

namespace Render
{
	void RenderModule::OnRegister()
	{
		myRenderCore = new RenderCore;
	}

	void RenderModule::OnUnregister()
	{
		SafeDelete(myRenderCore);
	}

	void RenderModule::OnInitialize()
	{
		myRenderCore->Initialize();
	}

	void RenderModule::OnFinalize()
	{
		myRenderCore->Finalize();
	}

	void RenderModule::OnUpdate(Core::Module::UpdateType aType)
	{
		if (aType == Core::Module::UpdateType::EarlyUpdate)
		{
			myRenderCore->StartFrame();
		}
		else if (aType == Core::Module::UpdateType::MainUpdate)
		{
			myRenderCore->Update();
		}
		else if (aType == Core::Module::UpdateType::LateUpdate)
		{
			myRenderCore->EndFrame();
		}
	}

	void RenderModule::RegisterWindow(GLFWwindow* aWindow, RendererType aType)
	{
		RenderTarget* swapChain = myRenderCore->RegisterWindow(aWindow);
		myRenderCore->AttachRenderer(aType, swapChain);
	}

	void RenderModule::UnregisterWindow(GLFWwindow* aWindow)
	{
		myRenderCore->UnregisterWindow(aWindow);
	}

	RenderTargetHandle RenderModule::CreateRenderTarget(uint aWidth, uint aHeight, RendererType aType)
	{
		RenderTarget* renderTarget = myRenderCore->CreateRenderTarget(aWidth, aHeight);
		myRenderCore->AttachRenderer(aType, renderTarget);
		return RenderTargetHandle(renderTarget);
	}

	void RenderModule::DestroyRenderTarget(RenderTargetHandle aHandle)
	{
		myRenderCore->DestroyRenderTarget(aHandle);
	}

	void* RenderModule::GetUserRenderTargetTexture(RenderTargetHandle aHandle) const
	{
		return (void*)myRenderCore->GetUserRenderTargetTexture(aHandle);
	}

	void RenderModule::LoadUserTexture(const char* aTexturePath, void*& anOutTextureID, uint& anOutWidth, uint& anOutHeight)
	{
		myRenderCore->LoadUserTexture(aTexturePath, (const VkDescriptorImageInfo*&)anOutTextureID, anOutWidth, anOutHeight);
	}

	void RenderModule::UnloadUserTexture(void* aTextureID)
	{
		myRenderCore->UnloadUserTexture((const VkDescriptorImageInfo*)aTextureID);
	}
}
