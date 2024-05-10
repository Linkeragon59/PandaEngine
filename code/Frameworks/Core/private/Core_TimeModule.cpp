#include "Core_TimeModule.h"

namespace Core
{
	void TimeModule::OnRegister()
	{
		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
		myStartTime = currentTime;
		myCurrentTime = currentTime;
	}

	void TimeModule::OnUpdate(Module::UpdateType aType)
	{
		if (aType == Module::UpdateType::EarlyUpdate)
		{
			std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

			myTimeNs = currentTime - myStartTime;
			myDeltaTimeNs = currentTime - myCurrentTime;
			myTime = currentTime - myStartTime;
			myDeltaTime = currentTime - myCurrentTime;

			myCurrentTime = currentTime;

			myFrameCounter++;
		}
	}
	uint64 TimeModule::GetCurrentTimeNs() const
	{
		std::chrono::nanoseconds timeNs = std::chrono::high_resolution_clock::now() - myStartTime;
		return timeNs.count();
	}
	uint64 TimeModule::GetCurrentTimeMs() const
	{
		std::chrono::nanoseconds timeNs = std::chrono::high_resolution_clock::now() - myStartTime;
		return std::chrono::duration_cast<std::chrono::milliseconds>(timeNs).count();
	}
	float TimeModule::GetCurrentTimeSec() const
	{
		std::chrono::duration<float> time = std::chrono::high_resolution_clock::now() - myStartTime;
		return time.count();
	}
}
