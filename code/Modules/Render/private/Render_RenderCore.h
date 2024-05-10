#pragma once

#include "Render_RenderModule.h"

#include "Render_Buffer.h"
#include "Render_Image.h"
#include "Render_ShaderHelpers.h"

namespace Render
{
	struct VulkanDevice;

	class Renderer;
	class UserRenderTarget;
	class SwapChain;

	class RenderCore
	{
	public:
		static RenderCore* GetInstance() { return RenderModule::GetInstance()->GetRenderCore(); }
		RenderCore();
		~RenderCore();

		void Initialize();
		void Finalize();

		void StartFrame();
		void Update();
		void EndFrame();

		RenderTarget* RegisterWindow(GLFWwindow* aWindow);
		void UnregisterWindow(GLFWwindow* aWindow);

		RenderTarget* CreateRenderTarget(uint aWidth, uint aHeight);
		void DestroyRenderTarget(RenderTarget* aRenderTarget);
		const VkDescriptorImageInfo* GetUserRenderTargetTexture(RenderTarget* aRenderTarget) const;

		void LoadUserTexture(const char* aTexturePath, const VkDescriptorImageInfo*& anOutDescriptor, uint& anOutWidth, uint& anOutHeight);
		void UnloadUserTexture(const VkDescriptorImageInfo* aDescriptor);

		void AttachRenderer(RendererType aType, RenderTarget* aRenderTarget);

		VkInstance GetVkInstance() const { return myVkInstance; }

		VulkanDevice* GetVulkanDevice() const { return myDevice; }
		VkPhysicalDevice GetPhysicalDevice() const;
		VkDevice GetDevice() const;
		VmaAllocator GetAllocator() const;
		VkQueue GetGraphicsQueue() const;
		VkCommandPool GetGraphicsCommandPool() const;

		uint GetInFlightFramesCount() const { return myInFlightFramesCount; }
		VkCommandBuffer GetCurrentCommandBuffer() const { return myCommandBuffers[myCurrentFrameIndex]; }

		const VkDescriptorImageInfo* GetWhiteTextureDescriptorInfo() const { return &myWhiteTexture.myDescriptor; }
		const VkDescriptorImageInfo* GetBlackTextureDescriptorInfo() const { return &myBlackTexture.myDescriptor; }
		const VkDescriptorImageInfo* GetMissingTextureDescriptorInfo() const { return &myMissingTexture.myDescriptor; }
		const VkDescriptorBufferInfo* GetDefaultMaterialDescriptorInfo() const { return &myDefaultMaterial.myDescriptor; }
		const VkDescriptorBufferInfo* GetDefaultJointsMatrixDescriptorInfo() const { return &myDefaultJointsMatrix.myDescriptor; }

	private:
		void CreateVkInstance();
		VkInstance myVkInstance = VK_NULL_HANDLE;
		bool myRenderDebug = false;
		VkDebugUtilsMessengerEXT myDebugMessenger = VK_NULL_HANDLE;
		void CreateDevice();
		VulkanDevice* myDevice = nullptr;

		uint myInFlightFramesCount = 1;
		uint myCurrentFrameIndex = 0;

		void SetupCommandBuffers();
		void DestroyCommandBuffers();
		std::vector<VkCommandBuffer> myCommandBuffers;

		void SetupSyncObjects();
		void DestroySyncObjects();
		struct SyncObjects
		{
			VkFence myRenderFence = VK_NULL_HANDLE;
			struct Semaphores
			{
				VkSemaphore myAcquireSemaphore = VK_NULL_HANDLE;
				VkSemaphore myPresentSemaphore = VK_NULL_HANDLE;
			};
			std::map<SwapChain*, Semaphores> mySwapChainSemaphores;
			void AddSwapchain(SwapChain* aSwapChain);
			void RemoveSwapchain(SwapChain* aSwapChain);
		};
		std::vector<SyncObjects> mySyncObjects;

		void SetupDefaultData();
		void DestroyDefaultData();
		void SetupTexture(Image& anImage, uint8* aColor);
		Image myWhiteTexture;
		Image myBlackTexture;
		Image myMissingTexture;
		Buffer myDefaultMaterial;
		Buffer myDefaultJointsMatrix;

		void EnableSwapChain(SwapChain* aSwapChain);
		void DisableSwapChain(SwapChain* aSwapChain);

		std::set<SwapChain*> mySwapChains;
		std::set<SwapChain*> myDisabledSwapChains; // waiting to setup
		std::set<UserRenderTarget*> myUserRenderTargets;
		std::set<RenderTarget*> myActiveRenderTargets;

		std::set<ImagePtr> myUserTextures;
	};
}
