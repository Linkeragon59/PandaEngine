#pragma once

#if DEBUG_BUILD

#include "Core_Module.h"

namespace Debugger
{
	class DebuggerModule : public Core::Module
	{
		DECLARE_CORE_MODULE(DebuggerModule, "Debugger", { "Render" })

	protected:
		void OnInitialize() override;
		void OnFinalize() override;

	public:
		void ShowWindowsInfoWindow(bool* anOpen) const;
		void ShowTouchInfo(bool* anOpen);
		void DrawTouches(GLFWwindow* aWindow) const;

	private:
		uint myWindowOpenCallback = UINT_MAX;
		struct WindowInfo
		{
			uint myTouchCallbackId = UINT_MAX;
			std::map<uint64, std::pair<float, float>> myTouches;
		};
		std::map<GLFWwindow*, WindowInfo> myWindowsInfo;
		
		std::set<uint64> myFakeTouchIds;
	};
}

#endif
