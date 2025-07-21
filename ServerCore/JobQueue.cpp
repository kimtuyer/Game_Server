#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"

/*--------------
	JobQueue
---------------*/

void JobQueue::Push(JobRef job, bool pushOnly)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job); // WRITE_LOCK

	// ù��° Job�� ���� �����尡 ������� ���
	if (prevCount == 0)
	{
		// �̹� �������� JobQueue�� ������ ����
		if (LCurrentJobQueue == nullptr && pushOnly == false)
		{
			Execute(1,JobType::GLOBAL_JOB);
		}
		else
		{
			// ���� �ִ� �ٸ� �����尡 �����ϵ��� GlobalQueue�� �ѱ��
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
	_jobs.Push(job); // WRITE_LOCK
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
//	// ù��° Job�� ���� �����尡 ������� ���
//	if (prevCount == 0)
//	{
//		// �̹� �������� JobQueue�� ������ ����
//		if (LCurrentJobQueue == nullptr && pushOnly == false)
//		{
//			BroadCast_Execute();
//		}
//		else
//		{
//			// ���� �ִ� �ٸ� �����尡 �����ϵ��� GlobalQueue�� �ѱ��
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
//		// ���� �ϰ��� 0����� ����
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
//		//	// ���� �ִ� �ٸ� �����尡 �����ϵ��� GlobalQueue�� �ѱ��
//		//	GGlobalQueue->Push(shared_from_this());
//		//	break;
//		//}
//	}
//
//}

// 1) �ϰ��� ��~�� ������?
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

		// ���� �ϰ��� 0����� ����
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

				//if (JobType == JobType::GLOBAL_JOB)
				//{
				//	// ���� �ִ� �ٸ� �����尡 �����ϵ��� GlobalQueue�� �ѱ��
				//	GGlobalQueue->Push(shared_from_this());
				//}
				//else if (JobType == JobType::DB_JOB)
				//{
				//	GDBQueue->Push(shared_from_this());
				//}
				if (JobType == JobType::Zone_Job)
				{
					GZoneLogicQueue[nZoneID]->Push(shared_from_this());
				}

				break;
			}
		}
	}
}