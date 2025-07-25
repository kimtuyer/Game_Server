#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"
#include "GLockFreeQueue.h"

/*--------------
	JobQueue
---------------*/

void JobQueue::Push(JobRef job, bool pushOnly)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job); // WRITE_LOCK

	// 첫번째 Job을 넣은 쓰레드가 실행까지 담당
	if (prevCount == 0)
	{
		// 이미 실행중인 JobQueue가 없으면 실행
		if (LCurrentJobQueue == nullptr && pushOnly == false)
		{
			Execute(1,JobType::GLOBAL_JOB);
		}
		else
		{
			// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다
			GGlobalQueue->Push(shared_from_this());
		}
	}
}

void JobQueue::PushDB(JobRef job)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job); // WRITE_LOCK
	if (prevCount == 0)
	{
		{
			GDBQueue->Push(shared_from_this());
		}
	}
}

void JobQueue::PushLogicJob(int ZoneID,JobRef job)
{
	const int32 prevCount = _jobCount.fetch_add(1);
#ifdef __ZONEQUEUE_LOCKFREE__
	_freeJobs.LockFree_Push(job);
#else
	_jobs.Push(job); // WRITE_LOCK
#endif // __ZONEQUEUE_LOCKFREE__

	if (prevCount == 0)
	{
		{
			GZoneLogicQueue[ZoneID]->Push(shared_from_this());
		}
	}
}

//void JobQueue::BroadJobPush(JobRef job, bool pushOnly)
//{
//	const int32 prevCount = _jobCount.fetch_add(1);
//	_jobs.Push(job); // WRITE_LOCK
//
//	// 첫번째 Job을 넣은 쓰레드가 실행까지 담당
//	if (prevCount == 0)
//	{
//		// 이미 실행중인 JobQueue가 없으면 실행
//		if (LCurrentJobQueue == nullptr && pushOnly == false)
//		{
//			BroadCast_Execute();
//		}
//		else
//		{
//			// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다
//			GBroadQueue->Push(shared_from_this());
//		}
//	}
//
//}
//
//void JobQueue::BroadCast_Execute()
//{
//	LCurrentJobQueue = this;
//
//	while (true)
//	{
//		Vector<JobRef> jobs;
//		_jobs.PopAll(OUT jobs);
//
//		const int32 jobCount = static_cast<int32>(jobs.size());
//		for (int32 i = 0; i < jobCount; i++)
//			jobs[i]->Execute();
//
//		// 남은 일감이 0개라면 종료
//		if (_jobCount.fetch_sub(jobCount) == jobCount)
//		{
//			LCurrentJobQueue = nullptr;
//			return;
//		}
//
//		//const uint64 now = ::GetTickCount64();
//		//if (now >= LEndTickCount)
//		//{
//		//	LCurrentJobQueue = nullptr;
//		//	// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다
//		//	GGlobalQueue->Push(shared_from_this());
//		//	break;
//		//}
//	}
//
//}

// 1) 일감이 너~무 몰리면?
void JobQueue::Execute(int nZoneID,int JobType)
{
	LCurrentJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
			jobs[i]->Execute();

		// 남은 일감이 0개라면 종료
		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			LCurrentJobQueue = nullptr;
			return;
		}

		if (JobType == JobType::Zone_Job)
		{
			const uint64 now = ::GetTickCount64();
			if (now >= LEndTickCount)
			{
				LCurrentJobQueue = nullptr;
			
				if (JobType == JobType::Zone_Job)
				{
					GZoneLogicQueue[nZoneID]->Push(shared_from_this());
				}

				break;
			}
		}
	}
}