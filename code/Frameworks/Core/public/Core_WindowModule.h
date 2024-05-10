#pragma once
#include "Core_Module.h"
#include "Core_SlotArray.h"

#include <functional>

struct GLFWmonitor;
struct GLFWwindow;

namespace Core
{
	class WindowModule : public Module
	{
	DECLARE_CORE_MODULE(WindowModule, "Window")

	protected:
		void OnRegister() override;
		void OnUnregister() override;

	public:
		struct MonitorProperties
		{
			float myPhysicalWidth = 100.f; // cm
			float myPhysicalHeight = 100.f; // cm

			int myWidth = 1920;
			int myHeight = 1920;
			int myPosX = 0;
			int myPosY = 0;

			int myWorkAreaX = 0;
			int myWorkAreaY = 0;
			int myWorkAreaWidth = 1920;
			int myWorkAreaHeight = 1080;

			float myContentScaleX = 1.f;
			float myContentScaleY = 1.f;

			std::string myName;
		};
		using MonitorsInfo = std::map<GLFWmonitor*, MonitorProperties>;
		const MonitorsInfo& GetMonitorsProperties() const { return myMonitors; }
		GLFWmonitor* GetMainMonitor() const { return myMainMonitor; }

		enum class WindowMode
		{
			Fullscreen,
			Windowed,
		};
		static const char* ToString(WindowMode aMode)
		{
			switch (aMode)
			{
			case Core::WindowModule::WindowMode::Fullscreen: return "Fullscreen";
			case Core::WindowModule::WindowMode::Windowed: return "Windowed";
			default: return "Unknown";
			}
		}

		struct WindowParams
		{
			WindowMode myMode = WindowMode::Fullscreen;
			GLFWmonitor* myMonitor = nullptr;
			int myWidth = 1280;
			int myHeight = 720;
			std::string myTitle;
			bool mySupportsTouch = false;
		};

		struct WindowProperties
		{
			WindowMode myMode = WindowMode::Fullscreen;
			GLFWmonitor* myMonitor = nullptr;

			int myWidth = 1280;
			int myHeight = 720;
			int myPosX = 0;
			int myPosY = 0;

			// in pixels
			int myFrameBufferWidth = 1280;
			int myFrameBufferHeight = 720;

			// in cm
			float myPhysicalWidth = 1.f;
			float myPhysicalHeight = 1.f;

			float myContentScaleX = 1.f;
			float myContentScaleY = 1.f;

			std::string myTitle;

			// for switching back to windowed
			int myInitialWidth = 1280;
			int myInitialHeight = 720;

			bool myTouchRequested = false;
		};
		using WindowsInfo = std::map<GLFWwindow*, WindowProperties>;
		const WindowProperties* GetWindowProperties(GLFWwindow* aWindow) const;
		const WindowsInfo& GetWindowsProperties() const { return myWindows; }

		GLFWwindow* OpenWindow(const WindowParams& someParams);
		void CloseWindow(GLFWwindow* aWindow);
		void MakeWindowFullscreen(GLFWwindow* aWindow, GLFWmonitor* aMonitor = nullptr);
		void MakeWindowWindowed(GLFWwindow* aWindow);

		typedef std::function<void(GLFWwindow*, bool)> WindowOpenCallback;
		typedef std::function<void(uint)> MonitorsCountCallback;
		typedef std::function<void(int, int)> SizeCallback;
		typedef std::function<void(float, float)> ContentScaleCallback;
		typedef std::function<void()> CloseCallback;

		uint AddWindowOpenCallback(WindowOpenCallback aCallback);
		void RemoveWindowOpenCallback(uint aCallbackId);

		uint AddMonitorsCountCallback(MonitorsCountCallback aCallback);
		void RemoveMonitorsCountCallback(uint aCallbackId);

		uint AddWindowSizeCallback(SizeCallback aCallback, GLFWwindow* aWindow);
		void RemoveWindowSizeCallback(uint aCallbakId);

		uint AddFramebufferSizeCallback(SizeCallback aCallback, GLFWwindow* aWindow);
		void RemoveFramebufferSizeCallback(uint aCallbakId);

		uint AddContentScaleCallback(ContentScaleCallback aCallback, GLFWwindow* aWindow);
		void RemoveContentScaleCallback(uint aCallbakId);

		uint AddCloseCallback(CloseCallback aCallback, GLFWwindow* aWindow);
		void RemoveCloseCallback(uint aCallbakId);

	protected:
		static void OnMonitorSetupChanged(GLFWmonitor* aMonitor, int anEvent);
		static void OnWindowSizeChanged(GLFWwindow* aWindow, int aWidth, int aHeight);
		static void OnWindowPosChanged(GLFWwindow* aWindow, int aPosX, int aPosY);
		static void OnWindowFramebufferSizeChanged(GLFWwindow* aWindow, int aWidth, int aHeight);
		static void OnWindowContentScaleChanged(GLFWwindow* aWindow, float aContentScaleX, float aContentScaleY);
		static void OnWindowClosed(GLFWwindow* aWindow);

	private:
		void AddMonitorInfo(GLFWmonitor* aMonitor);
		void AddWindowInfo(GLFWwindow* aWindow, const WindowParams& someParams);
		void UpdateMonitorForWindow(GLFWwindow* aWindow, WindowProperties& someOutProperties);
		GLFWmonitor* FindWindowMonitor(GLFWwindow* aWindow) const;

		void RegisterCallbacks(GLFWwindow* aWindow);
		void UnregisterCallbacks(GLFWwindow* aWindow);

		struct WindowOpenCallbackEntry
		{
			void Clear() { myCallback = nullptr; }
			bool IsSet() const { return myCallback != nullptr; }
			WindowOpenCallback myCallback = nullptr;
		};
		SlotArray<WindowOpenCallbackEntry> myWindowOpenCallbacks;
		
		struct MonitorsCountCallbackEntry
		{
			void Clear() { myCallback = nullptr; }
			bool IsSet() const { return myCallback != nullptr; }
			MonitorsCountCallback myCallback = nullptr;
		};
		SlotArray<MonitorsCountCallbackEntry> myMonitorsCountCallbacks;

		struct SizeCallbackEntry
		{
			void Clear() { myWindow = nullptr; myCallback = nullptr; }
			bool IsSet() const { return myCallback != nullptr; }
			GLFWwindow* myWindow = nullptr;
			SizeCallback myCallback = nullptr;
		};
		SlotArray<SizeCallbackEntry> myWindowSizeCallbacks;
		SlotArray<SizeCallbackEntry> myFramebufferSizeCallbacks;

		struct ContentScaleCallbackEntry
		{
			void Clear() { myWindow = nullptr; myCallback = nullptr; }
			bool IsSet() const { return myCallback != nullptr; }
			GLFWwindow* myWindow = nullptr;
			ContentScaleCallback myCallback = nullptr;
		};
		SlotArray<ContentScaleCallbackEntry> myContentScaleCallbacks;

		struct CloseCallbackEntry
		{
			void Clear() { myWindow = nullptr; myCallback = nullptr; }
			bool IsSet() const { return myCallback != nullptr; }
			GLFWwindow* myWindow = nullptr;
			CloseCallback myCallback = nullptr;
		};
		SlotArray<CloseCallbackEntry> myCloseCallbacks;

		GLFWmonitor* myMainMonitor = nullptr;
		MonitorsInfo myMonitors;
		WindowsInfo myWindows;
	};
}
