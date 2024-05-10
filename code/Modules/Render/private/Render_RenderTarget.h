#pragma once

#include "Render_Image.h"

namespace Render
{
	class Renderer;

	class RenderTarget
	{
	public:
		RenderTarget();
		virtual ~RenderTarget();
		virtual void AttachRenderer(Renderer* aRenderer) = 0;
		virtual GLFWwindow* GetAssociatedWindow() const { return nullptr; }

		uint GetImagesCount() const { return (uint)myImages.size(); };
		VkExtent2D GetExtent() const { return myExtent; }
		VkFormat GetColorFormat() const { return myColorFormat; }
		VkImageLayout GetFinalLayout() const { return myFinalLayout; }

		uint GetCurrentImageIndex() const { return myCurrentImageIndex; }
		VkImageView GetCurrentRenderTarget() const { return myImages[myCurrentImageIndex]->myImageView; }

		Renderer* GetRenderer() const { return myRenderer; }

	protected:
		VkDevice myDevice = VK_NULL_HANDLE;

		std::vector<ImagePtr> myImages;
		uint myCurrentImageIndex = 0;
		VkExtent2D myExtent = {};
		VkFormat myColorFormat = VK_FORMAT_UNDEFINED;
		VkImageLayout myFinalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		Renderer* myRenderer = nullptr;
	};

	class UserRenderTarget : public RenderTarget
	{
	public:
		UserRenderTarget(uint aWidth, uint aHeight, uint anImagesCount);
		~UserRenderTarget() override;
		void AttachRenderer(Renderer* aRenderer) override;

		void Progress() { myCurrentImageIndex = (myCurrentImageIndex + 1) % myImages.size(); }
		const VkDescriptorImageInfo* GetCurrentDescriptor() const;
	};
}
