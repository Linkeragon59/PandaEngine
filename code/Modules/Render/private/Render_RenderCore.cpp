#include "Render_RenderCore.h"

#include "Render_Debug.h"
#include "Render_Device.h"
#include "Render_RenderTarget.h"
#include "Render_SwapChain.h"
#include "Render_Resource.h"
#include "Render_Gui.h"

#include "Render_Renderer.h"
#include "Render_SampleTriangleRenderer.h"
#include "Render_SampleTexturedRectangleRenderer.h"
#include "Render_GuiRenderer.h"
#include "Render_DeferredRenderer.h"

#include "Core_EntityModule.h"
#include "Core_EntityCameraComponent.h"
#include "Core_EntityTransformComponent.h"
#include "Render_EntityRenderComponent.h"

#include "GLFW/glfw3.h"

#include "Core_WindowModule.h"
#include "imgui.h"
#include "stb_image.h"

namespace Render
{
	namespace
	{
		uint locVulkanApiVersion = VK_API_VERSION_1_0;
	}

	RenderCore::RenderCore()
	{
		// TODO : Avoid crashing if Vulkan is not supported or we can't create surfaces properly.

#if DEBUG_BUILD
		myRenderDebug = Core::Facade::GetCommandLine()->IsSet("renderdebug");
#endif

		CreateVkInstance();

		if (myRenderDebug)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo;
			Debug::FillDebugMessengerCreateInfo(createInfo);
			if (Debug::CreateDebugMessenger(myVkInstance, &createInfo, nullptr, &myDebugMessenger) != VK_SUCCESS)
			{
				Log("Couldn't create a debug messenger!");
			}
		}

		CreateDevice();

		if (myRenderDebug)
			Debug::SetupDebugMarkers(myDevice->myLogicalDevice);
	}

	RenderCore::~RenderCore()
	{
		delete myDevice;

		if (myRenderDebug && myDebugMessenger != VK_NULL_HANDLE)
			Debug::DestroyDebugMessenger(myVkInstance, myDebugMessenger, nullptr);

		vkDestroyInstance(myVkInstance, nullptr);
	}

	void RenderCore::Initialize()
	{
		RenderResource::EnableDeleteQueue(true);

		// TODO : Forced double buffering for now, expose the option
		myInFlightFramesCount = 2;
		myCurrentFrameIndex = 0;

		SetupCommandBuffers();
		SetupSyncObjects();
		SetupDefaultData();
	}

	void RenderCore::Finalize()
	{
		Assert(mySwapChains.empty(), "Rendering is being finalized while swapchains are still alive!");
		Assert(myDisabledSwapChains.empty(), "Rendering is being finalized while disabled swapchains are still alive!");
		Assert(myUserRenderTargets.empty(), "Rendering is being finalized while user render targets are still alive!");
		Assert(myActiveRenderTargets.empty(), "Rendering is being finalized while active render targets are still alive!");

		DestroyDefaultData();
		DestroySyncObjects();
		DestroyCommandBuffers();

		RenderResource::EnableDeleteQueue(false);
	}

	void RenderCore::StartFrame()
	{
		VkFence waitFence = mySyncObjects[myCurrentFrameIndex].myRenderFence;
		vkWaitForFences(GetDevice(), 1, &waitFence, VK_TRUE, UINT64_MAX);
		vkResetFences(GetDevice(), 1, &waitFence);

		for (auto it = myDisabledSwapChains.begin(); it != myDisabledSwapChains.end();)
		{
			SwapChain* disabledSwapChain = *it;

			Assert(!disabledSwapChain->IsSetup());
			if (disabledSwapChain->TrySetup())
			{
				mySwapChains.insert(disabledSwapChain);
				EnableSwapChain(disabledSwapChain);
				it = myDisabledSwapChains.erase(it);
				continue;
			}
			++it;
		}

		for (SwapChain* swapChain : mySwapChains)
		{
			VkSemaphore signalSemaphore = mySyncObjects[myCurrentFrameIndex].mySwapChainSemaphores[swapChain].myAcquireSemaphore;
			swapChain->AcquireNext(signalSemaphore);
		}

		for (UserRenderTarget* renderTarget : myUserRenderTargets)
		{
			renderTarget->Progress();
		}

		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(myCommandBuffers[myCurrentFrameIndex], &cmdBufferBeginInfo), "Failed to begin a command buffer");

		Assert(myActiveRenderTargets.size() == mySwapChains.size() + myUserRenderTargets.size());
		for (RenderTarget* renderTarget : myActiveRenderTargets)
		{
			Assert(renderTarget->GetRenderer());
			renderTarget->GetRenderer()->StartFrame();
		}
	}

	void RenderCore::Update()
	{
		{
			Core::ComponentContainer<EntitySimpleGeometryModelComponent>* container = Core::EntityModule::GetInstance()->GetComponentContainer<EntitySimpleGeometryModelComponent>();
			Core::ComponentContainer<Core::Entity3DTransformComponent>* transformContainer = Core::EntityModule::GetInstance()->GetComponentContainer<Core::Entity3DTransformComponent>();
			for (auto iter = container->begin(), end = container->end(); iter != end; ++iter)
			{
				if (Core::Entity3DTransformComponent* transform = transformContainer->GetComponent(iter.GetEntityId()))
					iter.GetComponent()->Update(transform->GetMatrix());
			}
		}

		{
			Core::ComponentContainer<EntityglTFModelComponent>* container = Core::EntityModule::GetInstance()->GetComponentContainer<EntityglTFModelComponent>();
			Core::ComponentContainer<Core::Entity3DTransformComponent>* transformContainer = Core::EntityModule::GetInstance()->GetComponentContainer<Core::Entity3DTransformComponent>();
			for (auto iter = container->begin(), end = container->end(); iter != end; ++iter)
			{
				if (Core::Entity3DTransformComponent* transform = transformContainer->GetComponent(iter.GetEntityId()))
					iter.GetComponent()->Update(transform->GetMatrix());
			}
		}

		for (RenderTarget* renderTarget : myActiveRenderTargets)
		{
			Assert(renderTarget->GetRenderer());
			Renderer* renderer = renderTarget->GetRenderer();

			if (renderer->ShouldDrawGui())
			{
				{
					Core::ComponentContainer<EntityGuiComponent>* container = Core::EntityModule::GetInstance()->GetComponentContainer<EntityGuiComponent>();
					for (EntityGuiComponent* component : *container)
					{
						if (component->myWindow == renderer->GetRenderTarget()->GetAssociatedWindow())
						{
							renderer->DrawGui(component->myGui);
						}
					}
				}
			}

			if (renderer->ShouldDrawWorld())
			{
				{
					Core::ComponentContainer<Core::EntityCameraComponent>* container = Core::EntityModule::GetInstance()->GetComponentContainer<Core::EntityCameraComponent>();
					for (Core::EntityCameraComponent* component : *container)
					{
						if (component->GetWindow() == renderer->GetRenderTarget()->GetAssociatedWindow())
						{
							renderer->SetViewProj(component->GetViewMatrix(), component->GetPerspectiveMatrix());
						}
					}
				}

				{
					Core::ComponentContainer<EntitySimpleGeometryModelComponent>* container = Core::EntityModule::GetInstance()->GetComponentContainer<EntitySimpleGeometryModelComponent>();
					for (EntitySimpleGeometryModelComponent* component : *container)
					{
						renderer->DrawModel(component->myModel);
					}
				}

				{
					Core::ComponentContainer<EntityglTFModelComponent>* container = Core::EntityModule::GetInstance()->GetComponentContainer<EntityglTFModelComponent>();
					for (EntityglTFModelComponent* component : *container)
					{
						renderer->DrawModel(component->myModel);
					}
				}
			}
		}
	}

	void RenderCore::EndFrame()
	{
		for (UserRenderTarget* renderTarget : myUserRenderTargets)
		{
			Assert(renderTarget->GetRenderer());
			renderTarget->GetRenderer()->EndFrame();
		}

		for (SwapChain* swapChain : mySwapChains)
		{
			Assert(swapChain->GetRenderer());
			swapChain->GetRenderer()->EndFrame();
		}

		VK_CHECK_RESULT(vkEndCommandBuffer(myCommandBuffers[myCurrentFrameIndex]), "Failed to end a command buffer");

		std::vector<VkPipelineStageFlags> waitStages;
		std::vector<VkSemaphore> waitSemaphores;
		std::vector<VkSemaphore> signalSemaphores;
		for (const auto& it : mySyncObjects[myCurrentFrameIndex].mySwapChainSemaphores)
		{
			waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			waitSemaphores.push_back(it.second.myAcquireSemaphore);
			signalSemaphores.push_back(it.second.myPresentSemaphore);
		}
		VkFence signalFence = mySyncObjects[myCurrentFrameIndex].myRenderFence;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = static_cast<uint>(waitSemaphores.size());
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &myCommandBuffers[myCurrentFrameIndex];
		submitInfo.signalSemaphoreCount = static_cast<uint>(signalSemaphores.size());;
		submitInfo.pSignalSemaphores = signalSemaphores.data();

		VK_CHECK_RESULT(vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, signalFence), "Failed to submit a command buffer");

		for (auto it = mySwapChains.begin(); it != mySwapChains.end();)
		{
			SwapChain* swapChain = *it;

			VkSemaphore waitSemaphore = mySyncObjects[myCurrentFrameIndex].mySwapChainSemaphores[swapChain].myPresentSemaphore;
			swapChain->Present(waitSemaphore);

			if (!swapChain->IsSetup())
			{
				myDisabledSwapChains.insert(swapChain);
				DisableSwapChain(swapChain);
				it = mySwapChains.erase(it);
				continue;
			}
			++it;
		}

		myCurrentFrameIndex = (myCurrentFrameIndex + 1) % myInFlightFramesCount;
	}

	RenderTarget* RenderCore::RegisterWindow(GLFWwindow* aWindow)
	{
		// TODO : Avoid this from happening between StartFrame and EndFrame

		SwapChain* swapChain = new SwapChain(aWindow);
		if (swapChain->IsSetup())
		{
			mySwapChains.insert(swapChain);
			EnableSwapChain(swapChain);
		}
		else
		{
			myDisabledSwapChains.insert(swapChain);
		}
		return swapChain;
	}

	void RenderCore::UnregisterWindow(GLFWwindow* aWindow)
	{
		// TODO : Handle this better
		vkDeviceWaitIdle(GetDevice());

		auto it = std::find_if(myDisabledSwapChains.begin(), myDisabledSwapChains.end(), [aWindow](SwapChain* aSwapChain) { return aSwapChain->GetWindowHandle() == aWindow; });
		if (it != myDisabledSwapChains.end())
		{
			SwapChain* swapChain = *it;

			myDisabledSwapChains.erase(swapChain);
			delete swapChain;
		}

		it = std::find_if(mySwapChains.begin(), mySwapChains.end(), [aWindow](SwapChain* aSwapChain) { return aSwapChain->GetWindowHandle() == aWindow; });
		if (it != mySwapChains.end())
		{
			SwapChain* swapChain = *it;

			DisableSwapChain(swapChain);
			mySwapChains.erase(swapChain);
			delete swapChain;
		}
	}

	RenderTarget* RenderCore::CreateRenderTarget(uint aWidth, uint aHeight)
	{
		// TODO : Not sure we need several images in the User Render Targets
		// currently this may only be necessary as we recreate frame buffers when starting a frame
		UserRenderTarget* renderTarget = new UserRenderTarget(aWidth, aHeight, myInFlightFramesCount);
		myUserRenderTargets.insert(renderTarget);
		myActiveRenderTargets.insert(renderTarget);
		return renderTarget;
	}

	void RenderCore::DestroyRenderTarget(RenderTarget* aRenderTarget)
	{
		// TODO : Handle this better
		vkDeviceWaitIdle(GetDevice());

		auto it = std::find_if(myUserRenderTargets.begin(), myUserRenderTargets.end(), [aRenderTarget](UserRenderTarget* anOtherRenderTarget) { return anOtherRenderTarget == aRenderTarget; });
		if (it != myUserRenderTargets.end())
		{
			UserRenderTarget* renderTarget = *it;
			myActiveRenderTargets.erase(renderTarget);
			myUserRenderTargets.erase(renderTarget);
			delete renderTarget;
		}
	}

	const VkDescriptorImageInfo* RenderCore::GetUserRenderTargetTexture(RenderTarget* aRenderTarget) const
	{
		for (UserRenderTarget* renderTarget : myUserRenderTargets)
		{
			if (!renderTarget || renderTarget != aRenderTarget)
				continue;

			return renderTarget->GetCurrentDescriptor();
		}
		return GetMissingTextureDescriptorInfo();
	}

	void RenderCore::LoadUserTexture(const char* aTexturePath, const VkDescriptorImageInfo*& anOutDescriptor, uint& anOutWidth, uint& anOutHeight)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(aTexturePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		Assert(pixels, "Failed to load an image!");

		VkDeviceSize textureSize = static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight) * 4;
		Buffer textureStaging;

		textureStaging.Create(textureSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		textureStaging.Map();
		memcpy(textureStaging.myMappedData, pixels, static_cast<size_t>(textureSize));
		textureStaging.Unmap();

		stbi_image_free(pixels);

		ImagePtr userTexture = new Image(texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		userTexture->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			GetGraphicsQueue());

		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand();
		{
			VkBufferImageCopy imageCopyRegion{};
			imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.imageSubresource.mipLevel = 0;
			imageCopyRegion.imageSubresource.baseArrayLayer = 0;
			imageCopyRegion.imageSubresource.layerCount = 1;
			imageCopyRegion.imageExtent = { static_cast<uint>(texWidth), static_cast<uint>(texHeight), 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, userTexture->myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		Helpers::EndOneTimeCommand(commandBuffer, GetGraphicsQueue());

		textureStaging.Destroy();

		userTexture->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			GetGraphicsQueue());

		userTexture->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		userTexture->CreateImageSampler();
		userTexture->SetupDescriptor();

		myUserTextures.insert(userTexture);
		anOutDescriptor = &userTexture->myDescriptor;
		anOutWidth = texWidth;
		anOutHeight = texHeight;
	}

	void RenderCore::UnloadUserTexture(const VkDescriptorImageInfo* aDescriptor)
	{
		for (auto it = myUserTextures.begin(); it != myUserTextures.end();)
		{
			ImagePtr userTexture = *it;
			if (&userTexture->myDescriptor == aDescriptor)
			{
				it = myUserTextures.erase(it);
				continue;
			}
			++it;
		}
	}

	void RenderCore::AttachRenderer(RendererType aType, RenderTarget* aRenderTarget)
	{
		Renderer* renderer = nullptr;
		switch (aType)
		{
		case RendererType::SampleTriangle:
			renderer = new SampleTriangleRenderer(*aRenderTarget);
			break;
		case RendererType::SampleTexturedRectangle:
			renderer = new SampleTexturedRectangleRenderer(*aRenderTarget);
			break;
		case RendererType::GuiOnly:
			renderer = new GuiRenderer(*aRenderTarget);
			break;
		case RendererType::Deferred:
			renderer = new DeferredRenderer(*aRenderTarget);
			break;
		default:
			Assert(false, "Unsupported Renderer Type");
			break;
		}
		aRenderTarget->AttachRenderer(renderer);
	}

	VkPhysicalDevice RenderCore::GetPhysicalDevice() const
	{
		return myDevice->myPhysicalDevice;
	}

	VkDevice RenderCore::GetDevice() const
	{
		return myDevice->myLogicalDevice;
	}

	VmaAllocator RenderCore::GetAllocator() const
	{
		return myDevice->myVmaAllocator;
	}

	VkQueue RenderCore::GetGraphicsQueue() const
	{
		return myDevice->myGraphicsQueue;
	}

	VkCommandPool RenderCore::GetGraphicsCommandPool() const
	{
		return myDevice->myGraphicsCommandPool;
	}

	void RenderCore::CreateVkInstance()
	{
		std::vector<const char*> layers;
		if (myRenderDebug)
			Debug::PopulateValidationLayers(layers);
		Helpers::CheckInstanceLayersSupport(layers);

		uint glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (myRenderDebug)
			Debug::PopulateDebugExtensions(extensions);
		Helpers::CheckInstanceExtensionsSupport(extensions);

		// TODO: Have more parameters when creating the RenderModule
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = locVulkanApiVersion;

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		
		if (myRenderDebug)
		{
			Debug::FillDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = &debugCreateInfo;
		}

		createInfo.enabledLayerCount = static_cast<uint>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
		createInfo.enabledExtensionCount = static_cast<uint>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &myVkInstance), "Failed to create Vulkan instance!");
	}

	void RenderCore::CreateDevice()
	{
		uint deviceCount = 0;
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, nullptr);
		Assert(deviceCount > 0, "No physical device supporting Vulkan was found!");

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(myVkInstance, &deviceCount, devices.data());

		// TODO: Select the physical device based on requirements.
		// For now, just take the first one.
		myDevice = new VulkanDevice(devices[0]);

		// Choose Device features to enable
		VkPhysicalDeviceFeatures enabledFeatures{};
		if (myDevice->myFeatures.samplerAnisotropy == VK_TRUE)
		{
			enabledFeatures.samplerAnisotropy = VK_TRUE;
		}
		if (myDevice->myFeatures.fillModeNonSolid)
		{
			enabledFeatures.fillModeNonSolid = VK_TRUE;
			if (myDevice->myFeatures.wideLines)
			{
				enabledFeatures.wideLines = VK_TRUE;
			}
		};

		std::vector<const char*> layers;
		if (myRenderDebug)
			Debug::PopulateValidationLayers(layers);
		Helpers::CheckInstanceLayersSupport(layers);

		std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		if (myRenderDebug)
			if (myDevice->SupportsExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
				extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

		myDevice->SetupLogicalDevice(
			enabledFeatures,
			layers,
			extensions,
			VK_QUEUE_GRAPHICS_BIT);
		myDevice->SetupVmaAllocator(myVkInstance, locVulkanApiVersion);
	}

	void RenderCore::SetupCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = GetGraphicsCommandPool();
		allocInfo.commandBufferCount = myInFlightFramesCount;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		myCommandBuffers.resize(myInFlightFramesCount);
		VK_CHECK_RESULT(vkAllocateCommandBuffers(GetDevice(), &allocInfo, myCommandBuffers.data()), "Failed to create the command buffers!");
	}

	void RenderCore::DestroyCommandBuffers()
	{
		myCommandBuffers.clear();
	}

	void RenderCore::SetupSyncObjects()
	{
		mySyncObjects.resize(myInFlightFramesCount);

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (SyncObjects& syncObj : mySyncObjects)
		{
			VK_CHECK_RESULT(vkCreateFence(GetDevice(), &fenceInfo, nullptr, &syncObj.myRenderFence), "Failed to create a fence");
		}
	}

	void RenderCore::DestroySyncObjects()
	{
		for (SyncObjects& syncObj : mySyncObjects)
		{
			vkDestroyFence(GetDevice(), syncObj.myRenderFence, nullptr);
		}

		mySyncObjects.clear();
	}

	void RenderCore::SyncObjects::AddSwapchain(SwapChain* aSwapChain)
	{
		Assert(!mySwapChainSemaphores.contains(aSwapChain));
		Semaphores& semaphores = mySwapChainSemaphores[aSwapChain];

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkDevice device = RenderCore::GetInstance()->GetDevice();
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores.myAcquireSemaphore), "Failed to create a semaphore");
		VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores.myPresentSemaphore), "Failed to create a semaphore");
	}

	void RenderCore::SyncObjects::RemoveSwapchain(SwapChain* aSwapChain)
	{
		Assert(mySwapChainSemaphores.contains(aSwapChain));
		Semaphores& semaphores = mySwapChainSemaphores[aSwapChain];

		VkDevice device = RenderCore::GetInstance()->GetDevice();
		vkDestroySemaphore(device, semaphores.myAcquireSemaphore, nullptr);
		vkDestroySemaphore(device, semaphores.myPresentSemaphore, nullptr);

		mySwapChainSemaphores.erase(aSwapChain);
	}

	void RenderCore::SetupDefaultData()
	{
		uint8 white[4] = { 0xff, 0xff, 0xff, 0xff };
		uint8 black[4] = { 0x00, 0x00, 0x00, 0xff };
		uint8 missing[4] = { 0xff, 0x14, 0x93, 0xff };
		SetupTexture(myWhiteTexture, white);
		SetupTexture(myBlackTexture, black);
		SetupTexture(myMissingTexture, missing);

		ShaderHelpers::MaterialData materialData;
		myDefaultMaterial.Create(sizeof(ShaderHelpers::MaterialData),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myDefaultMaterial.SetupDescriptor();
		myDefaultMaterial.Map();
		memcpy(myDefaultMaterial.myMappedData, &materialData, sizeof(ShaderHelpers::MaterialData));
		myDefaultMaterial.Unmap();

		ShaderHelpers::JointMatrixData jointsData;
		myDefaultJointsMatrix.Create(sizeof(ShaderHelpers::JointMatrixData),
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		myDefaultJointsMatrix.SetupDescriptor();
		myDefaultJointsMatrix.Map();
		memcpy(myDefaultJointsMatrix.myMappedData, &jointsData, sizeof(ShaderHelpers::JointMatrixData));
		myDefaultJointsMatrix.Unmap();
	}

	void RenderCore::DestroyDefaultData()
	{
		myWhiteTexture.Destroy();
		myBlackTexture.Destroy();
		myMissingTexture.Destroy();

		myDefaultMaterial.Destroy();
		myDefaultJointsMatrix.Destroy();
	}

	void RenderCore::SetupTexture(Image& anImage, uint8* aColor)
	{
		anImage.Create(1, 1,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		anImage.TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			GetGraphicsQueue());

		Buffer textureStaging;
		textureStaging.Create(4,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		textureStaging.Map();
		{
			memcpy(textureStaging.myMappedData, aColor, 4);
		}
		textureStaging.Unmap();

		VkCommandBuffer commandBuffer = Helpers::BeginOneTimeCommand();
		{
			VkBufferImageCopy imageCopyRegion{};
			imageCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.imageSubresource.mipLevel = 0;
			imageCopyRegion.imageSubresource.baseArrayLayer = 0;
			imageCopyRegion.imageSubresource.layerCount = 1;
			imageCopyRegion.imageExtent = { 1, 1, 1 };
			vkCmdCopyBufferToImage(commandBuffer, textureStaging.myBuffer, anImage.myImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopyRegion);
		}
		Helpers::EndOneTimeCommand(commandBuffer, GetGraphicsQueue());

		textureStaging.Destroy();

		anImage.TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			GetGraphicsQueue());
		anImage.CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		anImage.CreateImageSampler();
		anImage.SetupDescriptor();
	}

	void RenderCore::EnableSwapChain(SwapChain* aSwapChain)
	{
		myActiveRenderTargets.insert(aSwapChain);
		for (SyncObjects& syncObj : mySyncObjects)
		{
			syncObj.AddSwapchain(aSwapChain);
		}
	}

	void RenderCore::DisableSwapChain(SwapChain* aSwapChain)
	{
		for (SyncObjects& syncObj : mySyncObjects)
		{
			syncObj.RemoveSwapchain(aSwapChain);
		}
		myActiveRenderTargets.erase(aSwapChain);
	}
}
