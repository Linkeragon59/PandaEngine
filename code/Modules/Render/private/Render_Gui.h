#pragma once

#include "Render_Image.h"
#include "Render_Buffer.h"
#include "Render_ShaderHelpers.h"
#include "Render_Fonts.h"

#include <queue>

struct ImGuiContext;
struct ImPlotContext;

namespace Render
{
	class Gui
	{
	public:
		Gui(GLFWwindow* aWindow, bool aMenuBar);
		~Gui();

		void Update(const std::function<void()>& aCallback);
		void Draw(VkCommandBuffer aCommandBuffer, VkPipelineLayout aPipelineLayout, uint aDescriptorSetIndex, const std::function<VkDescriptorSet(const ShaderHelpers::DescriptorInfo&)>& myDescriptorSetGetter);

		ImGuiContext* GetGuiContext() const { return myGuiContext; }
		ImPlotContext* GetPlotContext() const { return myPlotContext; }
		GLFWwindow* GetWindow() const { return myWindow; }

		ImFont* GetFont(FontType aFontType) const { return myFontMap.GetFont(aFontType); }

	private:
		ImGuiContext* myGuiContext = nullptr;
		ImPlotContext* myPlotContext = nullptr;
		GLFWwindow* myWindow = nullptr;

		bool myHasMenuBar = false;

		uint myWindowResizeCallbackId = UINT_MAX;
		int myWindowWidth = -1;
		int myWindowHeight = -1;

		uint myWindowContentScaleCallbackId = UINT_MAX;
		float myContentScaleX = 1.f;
		float myContentScaleY = 1.f;
		bool myReloadFont = false;

		uint myMouseCallbackId;
		struct MouseData
		{
			int myButton = 0;
			bool myDown = false;
		};
		std::queue<MouseData> myMouseData;

		uint myCursorPosCallbackId = UINT_MAX;
		bool myCursorMoved = false;
		double myCursorXPos = 0.0;
		double myCursorYPos = 0.0;

		uint myTouchCallbackId = UINT_MAX;
		struct TouchData
		{
			bool myDown = false;
			glm::vec2 myPos = { 0.f, 0.f };
		};
		std::map<uint64, glm::vec2> myActiveTouches;
		std::optional<uint64> myLastFingerId;
		std::queue<TouchData> myTouchData;

		uint myScrollCallbackId = UINT_MAX;
		bool myScrollChanged = false;
		double myXScroll = 0.0;
		double myYScroll = 0.0;

		struct KeyData
		{
			int myKey = 0;
			bool myDown = false;
		};
		uint myKeyCallbackId;
		std::queue<KeyData> myKeyData;

		uint myCharacterCallbackId = UINT_MAX;
		std::queue<uint> myTextInput;

		void PrepareFont();
		ImagePtr myFontTexture;
		FontMap myFontMap;

		ShaderHelpers::GuiPushConstBlock myPushConstBlock;

		BufferPtr myVertexBuffer;
		int myVertexCount = 0;
		BufferPtr myIndexBuffer;
		int myIndexCount = 0;

		void InitStyle();
		void InitIO();
	};
}
