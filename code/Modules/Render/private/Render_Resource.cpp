#include "Render_Resource.h"

#include "Core_Thread.h"
#include "Core_TimeModule.h"

namespace Render
{
	struct RenderResourceDeleteQueue
	{
		RenderResourceDeleteQueue()
		{
#if DEBUG_BUILD
			myThread.SetName("RenderResourceDeleteQueue");
#endif
		}

		void Enable(bool aEnable)
		{
			if (!myIsEnabled && aEnable)
			{
				myThread.Start([this]() {
					if (myResourcesToDelete.empty())
						return;

					std::lock_guard<std::mutex> lock(myMutex);
					ResourceToDelete* resource = &myResourcesToDelete.front();
					while (resource && Core::TimeModule::GetInstance()->GetFrameCounter() >= resource->myFrameToRelease)
					{
						delete resource->myResource;
						myResourcesToDelete.pop();
						resource = myResourcesToDelete.empty() ? nullptr : &myResourcesToDelete.front();
					}
				}, Thread::WorkerPriority::Low, 100);
			}
			else if (myIsEnabled && !aEnable)
			{
				myThread.StopAndWait();

				std::lock_guard<std::mutex> lock(myMutex);
				ResourceToDelete* resource = myResourcesToDelete.size() > 0 ? &myResourcesToDelete.front() : nullptr;
				while (resource)
				{
					delete resource->myResource;
					myResourcesToDelete.pop();
					resource = myResourcesToDelete.empty() ? nullptr : &myResourcesToDelete.front();
				}
			}
			myIsEnabled = aEnable;
		}

		void AddToDelete(RenderResource* aResource)
		{
			// TODO : Not sure why we need this +1, without it we sometimes releasing too early and get validation errors
			ResourceToDelete resource = { aResource, Core::TimeModule::GetInstance()->GetFrameCounter() + RenderCore::GetInstance()->GetInFlightFramesCount() + 1 };
			
			std::lock_guard<std::mutex> lock(myMutex);
			myResourcesToDelete.push(resource);
		}

		Thread::WorkerThread myThread;
		bool myIsEnabled = false;

		struct ResourceToDelete 
		{
			RenderResource* myResource;
			uint myFrameToRelease;
		};
		std::mutex myMutex;
		std::queue<ResourceToDelete> myResourcesToDelete;
	};

	static RenderResourceDeleteQueue theDeleteQueue;

	void RenderResource::Release()
	{
		if (myRefCount.fetch_sub(1) == 1)
		{
			theDeleteQueue.AddToDelete(this);
		}
	}

	void RenderResource::EnableDeleteQueue(bool aEnable)
	{
		theDeleteQueue.Enable(aEnable);
	}
}
