#include "Debugger_Module.h"

#if DEBUG_BUILD

#include "Core_InputModule.h"
#include "Core_WindowModule.h"

#include <format>

namespace Debugger
{
	void DebuggerModule::OnInitialize()
	{
		Core::InputModule* inputModule = Core::InputModule::GetInstance();
		myWindowOpenCallback = Core::WindowModule::GetInstance()->AddWindowOpenCallback([this, inputModule](GLFWwindow* aWindow, bool anOpen) {
			if (anOpen)
			{
				WindowInfo& info = myWindowsInfo[aWindow];
				info.myTouchCallbackId = inputModule->AddTouchCallback([&info](uint64 aFingerId, double aXPos, double aYPos, bool anUp) {
					if (anUp)
					{
						info.myTouches.erase(aFingerId);
						return;
					}
					auto& touch = info.myTouches[aFingerId];
					touch.first = (float)aXPos;
					touch.second = (float)aYPos;
				}, aWindow);
			}
			else
			{
				Assert(myWindowsInfo.find(aWindow) != myWindowsInfo.end());
				WindowInfo& info = myWindowsInfo[aWindow];
				inputModule->RemoveTouchCallback(info.myTouchCallbackId);
				myWindowsInfo.erase(aWindow);
			}
		});
	}

	void DebuggerModule::OnFinalize()
	{
		Core::InputModule* inputModule = Core::InputModule::GetInstance();
		for (auto& info : myWindowsInfo)
			inputModule->RemoveTouchCallback(info.second.myTouchCallbackId);

		Core::WindowModule::GetInstance()->RemoveWindowOpenCallback(myWindowOpenCallback);
		myWindowOpenCallback = UINT_MAX;
	}

	void DebuggerModule::ShowWindowsInfoWindow(bool* anOpen) const
	{
		if (ImGui::Begin("Windows Debug", anOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Core::WindowModule* windowModule = Core::WindowModule::GetInstance();
			const Core::WindowModule::MonitorsInfo& monitors = windowModule->GetMonitorsProperties();
			const GLFWmonitor* mainMonitor = windowModule->GetMainMonitor();
			const Core::WindowModule::WindowsInfo& windows = windowModule->GetWindowsProperties();

			auto displayWindowProperties = [windowModule](GLFWwindow* aWindow, const Core::WindowModule::WindowProperties& someProperties)
			{
				std::string windowName = std::format("Window {} : {}", (void*)aWindow, someProperties.myTitle.c_str());
				if (ImGui::TreeNodeEx(windowName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Text("Mode : %s", Core::WindowModule::ToString(someProperties.myMode));
					ImGui::SameLine();
					ImGui::Spacing();
					ImGui::SameLine();
					if (ImGui::Button("Toggle Fullscreen"))
					{
						if (someProperties.myMode == Core::WindowModule::WindowMode::Fullscreen)
							windowModule->MakeWindowWindowed(aWindow);
						else
							windowModule->MakeWindowFullscreen(aWindow, someProperties.myMonitor);
					}
					ImGui::Text("Size : %d, %d", someProperties.myWidth, someProperties.myHeight);
					ImGui::Text("Position : %d, %d", someProperties.myPosX, someProperties.myPosY);
					ImGui::Text("Framebuffer : %d, %d", someProperties.myFrameBufferWidth, someProperties.myFrameBufferHeight);
					ImGui::Text("PhysicalSize : %f, %f", someProperties.myPhysicalWidth, someProperties.myPhysicalHeight);
					ImGui::Text("Content Scale : %f, %f", someProperties.myContentScaleX, someProperties.myContentScaleY);
					ImGui::TreePop();
				}
			};

			for (const auto& itMonitor : monitors)
			{
				std::string monitorName = std::format("Monitor {}{} : {}", (void*)itMonitor.first,
					itMonitor.first == mainMonitor ? " (MAIN)" : "", itMonitor.second.myName.c_str());
				if (ImGui::TreeNodeEx(monitorName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Text("PhysicalSize : %f, %f", itMonitor.second.myPhysicalWidth, itMonitor.second.myPhysicalHeight);
					ImGui::Text("Size : %d, %d", itMonitor.second.myWidth, itMonitor.second.myHeight);
					ImGui::Text("Position : %d, %d", itMonitor.second.myPosX, itMonitor.second.myPosY);
					ImGui::Text("Work Area : %d, %d, %d, %d",
						itMonitor.second.myWorkAreaX, itMonitor.second.myWorkAreaY,
						itMonitor.second.myWorkAreaWidth, itMonitor.second.myWorkAreaHeight);
					ImGui::Text("Content Scale : %f, %f", itMonitor.second.myContentScaleX, itMonitor.second.myContentScaleY);
					for (const auto& itWindow : windows)
					{
						if (itWindow.second.myMonitor != itMonitor.first)
							continue;
						displayWindowProperties(itWindow.first, itWindow.second);
					}
					ImGui::TreePop();
				}
			}

			for (const auto& itWindow : windows)
			{
				if (itWindow.second.myMonitor)
					continue;
				displayWindowProperties(itWindow.first, itWindow.second);
			}
		}
		ImGui::End();
	}

	void DebuggerModule::ShowTouchInfo(bool* anOpen)
	{
		if (ImGui::Begin("Touch Debug", anOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Core::WindowModule* windowModule = Core::WindowModule::GetInstance();
			Core::InputModule* inputModule = Core::InputModule::GetInstance();
			const Core::WindowModule::WindowsInfo& windows = windowModule->GetWindowsProperties();

			for (const auto& itWindow : windows)
			{
				GLFWwindow* window = itWindow.first;
				const Core::WindowModule::WindowProperties& props = itWindow.second;

				if (!itWindow.second.myTouchRequested)
					continue;

				const auto& info = myWindowsInfo.find(window);
				if (info == myWindowsInfo.end())
					continue;
				
				std::string windowName = std::format("Window {} : {}", (void*)window, props.myTitle.c_str());
				if (ImGui::TreeNodeEx(windowName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					uint touchesCount = static_cast<uint>(info->second.myTouches.size());

					// Fake touches
					if (ImGui::Button("Add Fake Touch"))
					{
						myFakeTouchIds.insert(inputModule->FakeTouchBegin(window, props.myWidth / 2.0, props.myHeight / 2.0));
					}

					std::set<uint64> fakeTouchesToRemove;

					ImGui::Text("Current Touches Count : %d", touchesCount);
					for (const auto& touch : info->second.myTouches)
					{
						bool isFakeTouch = myFakeTouchIds.find(touch.first) != myFakeTouchIds.end();
						std::string format = std::format("\tID:{}{}, x:{:.0f}, y:{:.0f}", isFakeTouch ? "FAKE_" : "", isFakeTouch ? UINT64_MAX - touch.first : touch.first, touch.second.first, touch.second.second);
						ImGui::Text(format.c_str());
						if (isFakeTouch)
						{
							ImGui::PushID(static_cast<int>(UINT64_MAX - touch.first));
							int x0 = (int)touch.second.first, y0 = (int)touch.second.second, x = x0, y = y0;

							ImGui::Indent();
							ImGui::SliderInt("X", &x, 0, props.myWidth);
							ImGui::SameLine();
							ImGui::SliderInt("Y", &y, 0, props.myHeight);
							ImGui::SameLine();
							if (ImGui::Button("Remove"))
							{
								fakeTouchesToRemove.insert(touch.first);
							}
							ImGui::Unindent();

							if (x != x0 || y != y0)
							{
								inputModule->FakeTouchMove(window, touch.first, x, y);
							}

							ImGui::PopID();
						}
					}

					for (uint64 fakeTouchId : fakeTouchesToRemove)
					{
						inputModule->FakeTouchEnd(window, fakeTouchId);
						myFakeTouchIds.erase(fakeTouchId);
					}

					ImGui::TreePop();
				}
			}
		}
		ImGui::End();
	}

	void DebuggerModule::DrawTouches(GLFWwindow* aWindow) const
	{
		const auto& info = myWindowsInfo.find(aWindow);
		if (info == myWindowsInfo.end())
			return;

		for (auto& touch : info->second.myTouches)
		{
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			static ImU32 colors[10] = { 0xFF0000FF, 0x7FFF0000, 0x7F00FF00, 0x7FFF00FF, 0x7F00FFFF, 0x7FFFFF00, 0x7F7F00FF, 0x7F007FFF, 0x7F00FF7F, 0x7FFF007F };
			draw_list->AddCircleFilled(ImVec2((float)touch.second.first, (float)touch.second.second), 25.f, colors[touch.first % 10]);
		}
	}
}

#endif
