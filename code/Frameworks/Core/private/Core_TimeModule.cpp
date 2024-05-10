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
}
