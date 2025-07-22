#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "GlobalQueue.h"

/*------------------
	ThreadManager
-------------------*/

ThreadManager::ThreadManager()
{
	// Main Thread
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard guard(_lock);

	_threads.push_back(thread([=]()
		{
			InitTLS();
			callback();
			DestroyTLS();
		}));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if (t.joinable())
			t.join();
	}
	_threads.clear();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{

}

bool ThreadManager::DoGlobalQueueWork()
{
	while (true)
	{
		/*uint64 now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;*/

		JobQueueRef jobQueue = GGlobalQueue->Pop();
		if (jobQueue == nullptr)
		{
			return false;
			break;
		}

		jobQueue->Execute(1,JobType::GLOBAL_JOB);
		return true;
	}
}

void ThreadManager::DistributeReservedJobs()
{
	const uint64 now = ::GetTickCount64();

	GJobTimer->Distribute(now);
}
bool ThreadManager::DoDBQueueWork()
{
	while (true)
	{
		/*uint64 now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;*/

		JobQueueRef jobQueue = GDBQueue->Pop();
		if (jobQueue == nullptr)
		{
			return false;
			break;
		}

		jobQueue->Execute(1,JobType::DB_JOB);
		return true;
	}
}
void ThreadManager::DistributeDBJobs()
{
	const uint64 now = ::GetTickCount64();
	
	GDBJobTimer->Distribute(now);
}
bool ThreadManager::DoZoneQueueWork(int nZoneID)
{
	while (true)
	{
		uint64 now = ::GetTickCount64();
		if (now > LEndTickCount)
			break;

		JobQueueRef jobQueue = GZoneLogicQueue[nZoneID]->Pop();
		if (jobQueue == nullptr)
		{
			return false;
			break;
		}

		jobQueue->Execute( nZoneID,JobType::Zone_Job);
		return true;
	}
}
void ThreadManager::DistributeZoneJobs(int nZoneID)
{
	const uint64 now = ::GetTickCount64();

	GZoneJobTimer[nZoneID]->ZoneJob_Distribute(nZoneID,now);
}
/*												*/
