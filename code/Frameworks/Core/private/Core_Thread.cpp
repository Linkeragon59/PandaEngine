#include "Core_Thread.h"

#include <windows.h>

namespace Thread
{
	void JobData::Wait()
	{
		// Avoid locking the mutex if we know we are done
		if (myDone)
			return;

		std::unique_lock<std::mutex> lock(myDoneMutex);
		myDoneCondition.wait(lock, [this] { return myDone; });
	}

	void JobData::OnDone()
	{
		{
			std::lock_guard<std::mutex> lock(myDoneMutex);
			myDone = true;
		}

		// In case some threads are waiting for us, tell them to re-evaluate the done state
		myDoneCondition.notify_all();
	}

	WorkerPool::Worker::Worker(WorkerPool* aPool)
		: myPool(aPool)
	{
		myWorkerThread = std::thread(&Worker::RunJobs, this);

		int priority = THREAD_PRIORITY_NORMAL;
		switch (myPool->myWorkersPriority)
		{
		case WorkerPriority::High:
			priority = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case WorkerPriority::Low:
			priority = THREAD_PRIORITY_BELOW_NORMAL;
			break;
		default:
			break;
		}
		SetThreadPriority(myWorkerThread.native_handle(), priority);

#if DEBUG_BUILD
		if (!myPool->myWorkersBaseName.empty())
		{
			std::wstring workerName = std::wstring(myPool->myWorkersBaseName.begin(), myPool->myWorkersBaseName.end());
			workerName += L" ";
			workerName += std::to_wstring(myPool->myWorkers.size());
			SetThreadDescription(myWorkerThread.native_handle(), workerName.c_str());
		}
#endif
	}

	WorkerPool::Worker::~Worker()
	{
		if (myWorkerThread.joinable())
		{
			WaitJobs();

			{
				std::lock_guard<std::mutex> lock(myJobQueueMutex);
				Assert(myJobQueue.empty(), "Jobs remaining in the queue!");
				myStopping = true;
			}

			// Tell the worker it should run (so it can end)
			myWorkToDoCondition.notify_one();

			myWorkerThread.join();
		}
	}

	void WorkerPool::Worker::AssignJob(JobHandle aJob)
	{
		{
			std::lock_guard<std::mutex> lock(myJobQueueMutex);
			myJobQueue.push(aJob);
		}

		// Tell the worker it has work to do
		myWorkToDoCondition.notify_one();
	}

	void WorkerPool::Worker::NotifyWaitingJobs()
	{
		// Tell the worker there is work waiting
		myWorkToDoCondition.notify_one();
	}

	void WorkerPool::Worker::WaitJobs()
	{
		std::unique_lock<std::mutex> lock(myJobQueueMutex);
		myWaitForJobsCondition.wait(lock, [this] { return !EvaluateWorkToDo(); });
	}

	void WorkerPool::Worker::RunJobs()
	{
		while (true)
		{
			JobHandle nextJob;

			{
				std::unique_lock<std::mutex> lock(myJobQueueMutex);
				myWorkToDoCondition.wait(lock, [this] { return EvaluateWorkToDo(); });
				if (myStopping)
				{
					Assert(myJobQueue.empty(), "Jobs remaining in the queue!");
					break;
				}
				nextJob = myJobQueue.front();
			}

			nextJob->myFunction();
			nextJob->OnDone();

			{
				std::lock_guard<std::mutex> lock(myJobQueueMutex);
				myJobQueue.pop();
			}

			// In case some threads are waiting for our jobs, tell them to re-evaluate the state of the queue
			myWaitForJobsCondition.notify_all();
		}
	}

	bool WorkerPool::Worker::EvaluateWorkToDo()
	{
		if (myStopping)
			return true;

		if (!myJobQueue.empty())
			return true;

		if (myPool->AssignJobTo(this))
			return true;

		return false;
	}

	WorkerPool::WorkerPool(WorkerPriority aPriority /*= WorkerPriority::High*/)
		: myWorkersPriority(aPriority)
	{
	}

	void WorkerPool::SetWorkersCount(uint aCount /*= UINT_MAX*/)
	{
		// Releasing the workers will cause to wait
		myWorkers.clear();

		aCount = (std::min)(aCount, std::thread::hardware_concurrency());
		myWorkers.reserve(aCount);
		for (uint i = 0; i < aCount; ++i)
		{
			myWorkers.push_back(std::make_unique<Worker>(this));
		}
	}

	JobHandle WorkerPool::RequestJob(std::function<void()> aJob, uint aWorkIndex /*= UINT_MAX*/)
	{
		JobHandle jobHandle = std::make_shared<JobData>();
		jobHandle->myFunction = std::move(aJob);
		
		if (aWorkIndex < myWorkers.size())
		{
			myWorkers[aWorkIndex]->AssignJob(jobHandle);
			return jobHandle;
		}

		// Put the job in a waiting queue, and the first worker that is done with its jobs will pick it.
		{
			std::lock_guard<std::mutex> lock(myWaitingJobQueueMutex);
			myWaitingJobQueue.push(jobHandle);
		}

		// Notify the workers there is work waiting.
		for (uint i = 0; i < myWorkers.size(); ++i)
		{
			myWorkers[i]->NotifyWaitingJobs();
		}

		return jobHandle;
	}

	void WorkerPool::WaitForJob(JobHandle aJobHandle)
	{
		aJobHandle->Wait();
	}

	void WorkerPool::WaitIdle()
	{
		for (uint i = 0; i < myWorkers.size(); ++i)
		{
			myWorkers[i]->WaitJobs();
		}
	}

	bool WorkerPool::AssignJobTo(Worker* aWorker)
	{
		std::lock_guard<std::mutex> lock(myWaitingJobQueueMutex);

		if (myWaitingJobQueue.empty())
			return false;

		// This is called by the worker thread, and the worker mutex is already locked at this point
		aWorker->myJobQueue.push(myWaitingJobQueue.front());

		myWaitingJobQueue.pop();
		return true;
	}

	void WorkerThread::Start(std::function<void()> aFunction, WorkerPriority aPriority, uint aSleepIntervalMs /*= 16*/)
	{
		StopAndWait();

		myFunction = std::move(aFunction);
		mySleepIntervalMs = aSleepIntervalMs;
		myThread = std::thread(&WorkerThread::Run, this);

		int priority = THREAD_PRIORITY_NORMAL;
		switch (aPriority)
		{
		case WorkerPriority::High:
			priority = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case WorkerPriority::Low:
			priority = THREAD_PRIORITY_BELOW_NORMAL;
			break;
		default:
			break;
		}
		SetThreadPriority(myThread.native_handle(), priority);

#if DEBUG_BUILD
		if (!myThreadName.empty())
		{
			std::wstring name = std::wstring(myThreadName.begin(), myThreadName.end());
			SetThreadDescription(myThread.native_handle(), name.c_str());
		}
#endif
	}

	void WorkerThread::StopAndWait()
	{
		myStopRequested = true;

		if (myThread.joinable())
			myThread.join();

		myStopRequested = false;
	}

	void WorkerThread::Run()
	{
		while (!myStopRequested)
		{
			myFunction();
			std::this_thread::sleep_for(std::chrono::milliseconds(mySleepIntervalMs));
		}
	}
}
