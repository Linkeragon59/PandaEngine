#pragma once
#include "Core_Module.h"

#include <atomic>
#include <chrono>

namespace Core
{
	class TimeModule : public Module
	{
	DECLARE_CORE_MODULE(TimeModule, "Time")

	protected:
		void OnRegister() override;
		void OnUpdate(Module::UpdateType aType) override;

	public:
		// Time since startup - Updated once each frame
		uint64 GetTimeNs() const { return myTimeNs.count(); }
		uint64 GetTimeMs() const { return std::chrono::duration_cast<std::chrono::milliseconds>(myTimeNs).count(); }
		float GetTimeSec() const { return myTime.count(); }

		// Time since startup - Immediate result
		uint64 GetCurrentTimeNs() const;
		uint64 GetCurrentTimeMs() const;
		float GetCurrentTimeSec() const;

		// Duration of last frame - Updated once each frame
		uint64 GetDeltaTimeNs() const { return myDeltaTimeNs.count(); }
		uint64 GetDeltaTimeMs() const { return std::chrono::duration_cast<std::chrono::milliseconds>(myDeltaTimeNs).count(); }
		float GetDeltaTimeSec() const { return myDeltaTime.count(); }

		uint GetFrameCounter() const { return myFrameCounter; }

	private:
		std::chrono::nanoseconds myTimeNs;
		std::chrono::nanoseconds myDeltaTimeNs;
		std::chrono::duration<float> myTime;
		std::chrono::duration<float> myDeltaTime;

		std::chrono::high_resolution_clock::time_point myStartTime;
		std::chrono::high_resolution_clock::time_point myCurrentTime;

		std::atomic<uint> myFrameCounter = 0;
	};
}
