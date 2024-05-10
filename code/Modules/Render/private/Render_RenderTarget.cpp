#include "Render_RenderTarget.h"

#include "Render_Renderer.h"

namespace Render
{
	RenderTarget::RenderTarget()
	{
		myDevice = RenderCore::GetInstance()->GetDevice();
	}

	RenderTarget::~RenderTarget()
	{
		if (myRenderer)
		{
			Assert(!myRenderer->IsSetup());
			SafeDelete(myRenderer);
		}
	}

	UserRenderTarget::UserRenderTarget(uint aWidth, uint aHeight, uint anImagesCount)
		: RenderTarget()
	{
		myExtent.width = aWidth;
		myExtent.height = aHeight;

		myColorFormat = VK_FORMAT_R8G8B8A8_UNORM;
		myFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		myImages.resize(anImagesCount);
		for (ImagePtr& image : myImages)
		{
			image = new Image();
			image->Create(myExtent.width, myExtent.height, myColorFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			image->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
			image->CreateImageSampler();
			image->SetupDescriptor();
		}
	}

	UserRenderTarget::~UserRenderTarget()
	{
		if (myRenderer && myRenderer->IsSetup())
		{
			myRenderer->Cleanup();
		}
	}

	void UserRenderTarget::AttachRenderer(Renderer* aRenderer)
	{
		Assert(!myRenderer);
		myRenderer = aRenderer;
		myRenderer->Setup();
	}

	const VkDescriptorImageInfo* UserRenderTarget::GetCurrentDescriptor() const
	{
		return &myImages[myCurrentImageIndex]->myDescriptor;
	}
}
