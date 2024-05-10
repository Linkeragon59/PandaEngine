#include "Core_WindowModule.h"
#include "Core_InputModule.h"
#include "Core_TimeModule.h"

#define FULL_SCREEN_WORKAROUND DEBUG_BUILD && 0

#if FULL_SCREEN_WORKAROUND
#include <wtypes.h>
#include <WinUser.h>
#undef max
#undef min
#endif

#include "GLFW/glfw3.h"

namespace Core
{
	namespace WindowModule_Priv
	{
		void GetMonitorPhysicalSize(GLFWmonitor* aMonitor, float* aWidth, float* aHeight)
		{
			int width = 0, height = 0;
			glfwGetMonitorPhysicalSize(aMonitor, &width, &height);
			*aWidth = width / 10.f;
			*aHeight = height / 10.f;
		}

		void GetMonitorSize(GLFWmonitor* aMonitor, int* aWidth, int* aHeight)
		{
			if (const GLFWvidmode* mode = glfwGetVideoMode(aMonitor))
			{
				*aWidth = mode->width;
				*aHeight = mode->height;
			}
		}
	}

	void WindowModule::OnRegister()
	{
		glfwInit();
		glfwSetMonitorCallback(WindowModule::OnMonitorSetupChanged);
		int monitorCount = 0;
		if (GLFWmonitor** monitors = glfwGetMonitors(&monitorCount))
		{
			for (int i = 0; i < monitorCount; ++i)
				AddMonitorInfo(monitors[i]);
		}
		myMainMonitor = glfwGetPrimaryMonitor();
	}

	void WindowModule::OnUnregister()
	{
		glfwSetMonitorCallback(nullptr);
		glfwTerminate();
	}

	const WindowModule::WindowProperties* WindowModule::GetWindowProperties(GLFWwindow* aWindow) const
	{
		auto it = myWindows.find(aWindow);
		return (it != myWindows.end()) ? &it->second : nullptr;
	}

	GLFWwindow* WindowModule::OpenWindow(const WindowParams& someParams)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
		glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

		if (someParams.mySupportsTouch)
			glfwWindowHint(GLFW_SUPPORTS_TOUCH, GLFW_TRUE);

		int width = someParams.myWidth, height = someParams.myHeight;
		GLFWmonitor* monitor = nullptr;

		if (someParams.myMode == WindowMode::Fullscreen && someParams.myMonitor)
		{
			WindowModule_Priv::GetMonitorSize(someParams.myMonitor, &width, &height);
#if FULL_SCREEN_WORKAROUND
			if (const GLFWvidmode* mode = glfwGetVideoMode(someParams.myMonitor))
			{
				glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
				glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
				height += 1;
			}
#else
			monitor = someParams.myMonitor;
#endif
		}

		GLFWwindow* window = glfwCreateWindow(width, height,
			someParams.myTitle.c_str(), monitor, nullptr);

		if (someParams.myMode == WindowMode::Windowed && someParams.myMonitor)
		{
			int monitorPosX = 0, monitorPosY = 0;
			glfwGetMonitorPos(someParams.myMonitor, &monitorPosX, &monitorPosY);
			int frameLeft = 0, frameTop = 0, frameRight = 0, frameBottom = 0;
			glfwGetWindowFrameSize(window, &frameLeft, &frameTop, &frameRight, &frameBottom);
			glfwSetWindowPos(window, monitorPosX + frameLeft, monitorPosY + frameTop);
		}
#if FULL_SCREEN_WORKAROUND
		else if (someParams.myMode == WindowMode::Fullscreen && someParams.myMonitor)
		{
			int monitorX, monitorY;
			glfwGetMonitorPos(someParams.myMonitor, &monitorX, &monitorY);
			glfwSetWindowPos(window, monitorX, monitorY - 1);
			static HWND hShellWnd = ::FindWindow("Shell_TrayWnd", NULL);
			ShowWindow(hShellWnd, SW_HIDE);
		}
#endif

		AddWindowInfo(window, someParams);
		RegisterCallbacks(window);
		return window;
	}

	void WindowModule::CloseWindow(GLFWwindow* aWindow)
	{
		UnregisterCallbacks(aWindow);
		myWindows.erase(aWindow);
		glfwDestroyWindow(aWindow);
	}

	void WindowModule::MakeWindowFullscreen(GLFWwindow* aWindow, GLFWmonitor* aMonitor)
	{
		auto it = ourInstance->myWindows.find(aWindow);
		if (it == ourInstance->myWindows.end())
			return;

		if (!aMonitor)
			if (const WindowProperties* props = GetWindowProperties(aWindow))
				aMonitor = props->myMonitor;

		if (!aMonitor)
			return;

#if FULL_SCREEN_WORKAROUND
		if (const GLFWvidmode* mode = glfwGetVideoMode(aMonitor))
		{
			glfwSetWindowAttrib(aWindow, GLFW_DECORATED, GLFW_FALSE);
			glfwSetWindowAttrib(aWindow, GLFW_MAXIMIZED, GLFW_TRUE);
			int monitorX, monitorY;
			glfwGetMonitorPos(aMonitor, &monitorX, &monitorY);
			glfwSetWindowPos(aWindow, monitorX, monitorY - 1);
			glfwSetWindowSize(aWindow, mode->width, mode->height + 1);
			static HWND hShellWnd = ::FindWindow("Shell_TrayWnd", NULL);
			ShowWindow(hShellWnd, SW_HIDE);
		}
#else
		int width, height;
		WindowModule_Priv::GetMonitorSize(aMonitor, &width, &height);
		glfwSetWindowMonitor(aWindow, aMonitor,
			it->second.myPosX, it->second.myPosY, width, height, GLFW_DONT_CARE);
#endif

		it->second.myMode = WindowMode::Fullscreen;
	}

	void WindowModule::MakeWindowWindowed(GLFWwindow* aWindow)
	{
		auto it = ourInstance->myWindows.find(aWindow);
		if (it == ourInstance->myWindows.end())
			return;

#if FULL_SCREEN_WORKAROUND
		glfwSetWindowAttrib(aWindow, GLFW_DECORATED, GLFW_TRUE);
		glfwSetWindowAttrib(aWindow, GLFW_MAXIMIZED, GLFW_FALSE);
		int frameLeft = 0, frameTop = 0, frameRight = 0, frameBottom = 0;
		glfwGetWindowFrameSize(aWindow, &frameLeft, &frameTop, &frameRight, &frameBottom);
		glfwSetWindowPos(aWindow, it->second.myPosX + frameLeft, it->second.myPosY + frameTop);
		glfwSetWindowSize(aWindow,
			static_cast<int>(it->second.myInitialWidth * it->second.myContentScaleX),
			static_cast<int>(it->second.myInitialHeight * it->second.myContentScaleY));
		static HWND hShellWnd = ::FindWindow("Shell_TrayWnd", NULL);
		ShowWindow(hShellWnd, SW_SHOW);
#else
		glfwSetWindowMonitor(aWindow, nullptr,
			it->second.myPosX, it->second.myPosY,
			static_cast<int>(it->second.myInitialWidth * it->second.myContentScaleX),
			static_cast<int>(it->second.myInitialHeight * it->second.myContentScaleY),
			GLFW_DONT_CARE);
		
		int frameLeft = 0, frameTop = 0, frameRight = 0, frameBottom = 0;
		glfwGetWindowFrameSize(aWindow, &frameLeft, &frameTop, &frameRight, &frameBottom);
		glfwSetWindowPos(aWindow, it->second.myPosX + frameLeft, it->second.myPosY + frameTop);
#endif
		
		it->second.myMode = WindowMode::Windowed;
	}

	uint WindowModule::AddWindowOpenCallback(WindowOpenCallback aCallback)
	{
		WindowOpenCallbackEntry entry;
		entry.myCallback = aCallback;
		return myWindowOpenCallbacks.Add(entry);
	}

	void WindowModule::RemoveWindowOpenCallback(uint aCallbackId)
	{
		myWindowOpenCallbacks.Remove(aCallbackId);
	}

	uint WindowModule::AddMonitorsCountCallback(MonitorsCountCallback aCallback)
	{
		MonitorsCountCallbackEntry entry;
		entry.myCallback = aCallback;
		return myMonitorsCountCallbacks.Add(entry);
	}

	void WindowModule::RemoveMonitorsCountCallback(uint aCallbackId)
	{
		myMonitorsCountCallbacks.Remove(aCallbackId);
	}

	uint WindowModule::AddWindowSizeCallback(SizeCallback aCallback, GLFWwindow* aWindow)
	{
		SizeCallbackEntry entry;
		entry.myWindow = aWindow;
		entry.myCallback = aCallback;
		return myWindowSizeCallbacks.Add(entry);
	}

	void WindowModule::RemoveWindowSizeCallback(uint aCallbakId)
	{
		myWindowSizeCallbacks.Remove(aCallbakId);
	}

	uint WindowModule::AddFramebufferSizeCallback(SizeCallback aCallback, GLFWwindow* aWindow)
	{
		SizeCallbackEntry entry;
		entry.myWindow = aWindow;
		entry.myCallback = aCallback;
		return myFramebufferSizeCallbacks.Add(entry);
	}

	void WindowModule::RemoveFramebufferSizeCallback(uint aCallbakId)
	{
		myFramebufferSizeCallbacks.Remove(aCallbakId);
	}

	uint WindowModule::AddContentScaleCallback(ContentScaleCallback aCallback, GLFWwindow* aWindow)
	{
		ContentScaleCallbackEntry entry;
		entry.myWindow = aWindow;
		entry.myCallback = aCallback;
		return myContentScaleCallbacks.Add(entry);
	}

	void WindowModule::RemoveContentScaleCallback(uint aCallbakId)
	{
		myContentScaleCallbacks.Remove(aCallbakId);
	}

	uint WindowModule::AddCloseCallback(CloseCallback aCallback, GLFWwindow* aWindow)
	{
		CloseCallbackEntry entry;
		entry.myWindow = aWindow;
		entry.myCallback = aCallback;
		return myCloseCallbacks.Add(entry);
	}

	void WindowModule::RemoveCloseCallback(uint aCallbakId)
	{
		myCloseCallbacks.Remove(aCallbakId);
	}

	void WindowModule::OnMonitorSetupChanged(GLFWmonitor* aMonitor, int anEvent)
	{
		if (anEvent == GLFW_DISCONNECTED)
			ourInstance->myMonitors.erase(aMonitor);
		else
			ourInstance->AddMonitorInfo(aMonitor);
		ourInstance->myMainMonitor = glfwGetPrimaryMonitor();

		for (auto& it : ourInstance->myWindows)
			ourInstance->UpdateMonitorForWindow(it.first, it.second);

		for (const MonitorsCountCallbackEntry& entry : ourInstance->myMonitorsCountCallbacks.myEntries)
		{
			if (entry.IsSet())
			{
				entry.myCallback(static_cast<uint>(ourInstance->myMonitors.size()));
			}
		}
	}

	void WindowModule::OnWindowSizeChanged(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		auto it = ourInstance->myWindows.find(aWindow);
		if (it == ourInstance->myWindows.end())
			return;

		it->second.myWidth = aWidth;
		it->second.myHeight = aHeight;
		ourInstance->UpdateMonitorForWindow(aWindow, it->second);

		for (const SizeCallbackEntry& entry : ourInstance->myWindowSizeCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(aWidth, aHeight);
			}
		}
	}

	void WindowModule::OnWindowPosChanged(GLFWwindow* aWindow, int aPosX, int aPosY)
	{
		auto it = ourInstance->myWindows.find(aWindow);
		if (it == ourInstance->myWindows.end())
			return;

		it->second.myPosX = aPosX;
		it->second.myPosY = aPosY;
		ourInstance->UpdateMonitorForWindow(aWindow, it->second);
	}

	void WindowModule::OnWindowFramebufferSizeChanged(GLFWwindow* aWindow, int aWidth, int aHeight)
	{
		auto it = ourInstance->myWindows.find(aWindow);
		if (it == ourInstance->myWindows.end())
			return;

		it->second.myFrameBufferWidth = aWidth;
		it->second.myFrameBufferHeight = aHeight;

		for (const SizeCallbackEntry& entry : ourInstance->myFramebufferSizeCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(aWidth, aHeight);
			}
		}
	}

	void WindowModule::OnWindowContentScaleChanged(GLFWwindow* aWindow, float aContentScaleX, float aContentScaleY)
	{
		auto it = ourInstance->myWindows.find(aWindow);
		if (it == ourInstance->myWindows.end())
			return;

		it->second.myContentScaleX = aContentScaleX;
		it->second.myContentScaleY = aContentScaleY;

		for (const ContentScaleCallbackEntry& entry : ourInstance->myContentScaleCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(aContentScaleX, aContentScaleY);
			}
		}
	}

	void WindowModule::OnWindowClosed(GLFWwindow* aWindow)
	{
		auto it = ourInstance->myWindows.find(aWindow);
		if (it == ourInstance->myWindows.end())
			return;

		for (const CloseCallbackEntry& entry : ourInstance->myCloseCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback();
			}
		}
	}

	void WindowModule::AddMonitorInfo(GLFWmonitor* aMonitor)
	{
		if (!aMonitor)
			return;

		MonitorProperties props;

		WindowModule_Priv::GetMonitorPhysicalSize(aMonitor, &props.myPhysicalWidth, &props.myPhysicalHeight);
		WindowModule_Priv::GetMonitorSize(aMonitor, &props.myWidth, &props.myHeight);
		glfwGetMonitorPos(aMonitor, &props.myPosX, &props.myPosY);
		glfwGetMonitorWorkarea(aMonitor, &props.myWorkAreaX, &props.myWorkAreaY, &props.myWorkAreaWidth, &props.myWorkAreaHeight);
		glfwGetMonitorContentScale(aMonitor, &props.myContentScaleX, &props.myContentScaleY);	
		props.myName = glfwGetMonitorName(aMonitor);

		myMonitors[aMonitor] = props;
	}

	void WindowModule::AddWindowInfo(GLFWwindow* aWindow, const WindowParams& someParams)
	{
		WindowProperties props;

		props.myMode = someParams.myMode;
		glfwGetWindowSize(aWindow, &props.myWidth, &props.myHeight);
		glfwGetWindowPos(aWindow, &props.myPosX, &props.myPosY);
		glfwGetFramebufferSize(aWindow, &props.myFrameBufferWidth, &props.myFrameBufferHeight);
		glfwGetWindowContentScale(aWindow, &props.myContentScaleX, &props.myContentScaleY);
		UpdateMonitorForWindow(aWindow, props);
		props.myTitle = someParams.myTitle;
		props.myInitialWidth = someParams.myWidth;
		props.myInitialHeight = someParams.myHeight;
		props.myTouchRequested = someParams.mySupportsTouch;

		myWindows[aWindow] = props;
	}

	void WindowModule::UpdateMonitorForWindow(GLFWwindow* aWindow, WindowProperties& someOutProperties)
	{
		someOutProperties.myMonitor = FindWindowMonitor(aWindow);
		if (someOutProperties.myMonitor)
		{
			float monitorPhysicalWidth = 1.f, monitorPhysicalHeight = 1.f;
			WindowModule_Priv::GetMonitorPhysicalSize(someOutProperties.myMonitor, &monitorPhysicalWidth, &monitorPhysicalHeight);
			int monitorWidth = someOutProperties.myWidth, monitorHeight = someOutProperties.myHeight;
			WindowModule_Priv::GetMonitorSize(someOutProperties.myMonitor, &monitorWidth, &monitorHeight);
			someOutProperties.myPhysicalWidth = monitorPhysicalWidth * someOutProperties.myWidth / monitorWidth;
			someOutProperties.myPhysicalHeight = monitorPhysicalHeight * someOutProperties.myHeight / monitorHeight;
		}
	}

	GLFWmonitor* WindowModule::FindWindowMonitor(GLFWwindow* aWindow) const
	{
		// If fullscreen, we know the monitor right away
		GLFWmonitor* monitor = glfwGetWindowMonitor(aWindow);
		if (monitor)
			return monitor;

		// If not, we can evaluate the overlap between the window and the different monitors
		int monitorCount = 0;
		int maxOverlap = 0;
		if (GLFWmonitor** monitors = glfwGetMonitors(&monitorCount))
		{
			int windowL, windowT, windowR, windowB;
			glfwGetWindowPos(aWindow, &windowL, &windowT);
			glfwGetWindowSize(aWindow, &windowR, &windowB);
			windowR += windowL;
			windowB += windowT;

			for (int i = 0; i < monitorCount; ++i)
			{
				int monitorL, monitorT, monitorR, monitorB;
				glfwGetMonitorPos(monitors[i], &monitorL, &monitorT);
				WindowModule_Priv::GetMonitorSize(monitors[i], &monitorR, &monitorB);
				monitorR += monitorL;
				monitorB += monitorT;
				
				int overlap = std::max(0, std::min(windowR, monitorR) - std::max(windowL, monitorL))
					* std::max(0, std::min(windowB, monitorB) - std::max(windowT, monitorT));
				if (overlap > maxOverlap)
				{
					maxOverlap = overlap;
					monitor = monitors[i];
				}
			}
		}
		if (monitor)
			return monitor;

		// Otherwise just assume it's the main monitor
		return myMainMonitor;
	}

	void WindowModule::RegisterCallbacks(GLFWwindow* aWindow)
	{
		glfwSetWindowSizeCallback(aWindow, WindowModule::OnWindowSizeChanged);
		glfwSetWindowPosCallback(aWindow, WindowModule::OnWindowPosChanged);
		glfwSetFramebufferSizeCallback(aWindow, WindowModule::OnWindowFramebufferSizeChanged);
		glfwSetWindowContentScaleCallback(aWindow, WindowModule::OnWindowContentScaleChanged);
		glfwSetWindowCloseCallback(aWindow, WindowModule::OnWindowClosed);

		glfwSetMouseButtonCallback(aWindow, InputModule::OnMouseCallback);
		glfwSetCursorPosCallback(aWindow, InputModule::OnCursorCallback);
		glfwSetTouchCallback(aWindow, InputModule::OnTouchCallback);
		glfwSetKeyCallback(aWindow, InputModule::OnKeyCallback);
		glfwSetScrollCallback(aWindow, InputModule::OnScrollCallback);
		glfwSetCharCallback(aWindow, InputModule::OnCharacterCallback);

		for (const WindowOpenCallbackEntry& entry : myWindowOpenCallbacks.myEntries)
		{
			if (entry.IsSet())
			{
				entry.myCallback(aWindow, true);
			}
		}
	}

	void WindowModule::UnregisterCallbacks(GLFWwindow* aWindow)
	{
		for (const WindowOpenCallbackEntry& entry : myWindowOpenCallbacks.myEntries)
		{
			if (entry.IsSet())
			{
				entry.myCallback(aWindow, false);
			}
		}

		glfwSetWindowSizeCallback(aWindow, nullptr);
		glfwSetWindowPosCallback(aWindow, nullptr);
		glfwSetFramebufferSizeCallback(aWindow, nullptr);
		glfwSetWindowContentScaleCallback(aWindow, nullptr);
		glfwSetWindowCloseCallback(aWindow, nullptr);

		glfwSetMouseButtonCallback(aWindow, nullptr);
		glfwSetCursorPosCallback(aWindow, nullptr);
		glfwSetTouchCallback(aWindow, nullptr);
		glfwSetKeyCallback(aWindow, nullptr);
		glfwSetScrollCallback(aWindow, nullptr);
		glfwSetCharCallback(aWindow, nullptr);
	}
}
