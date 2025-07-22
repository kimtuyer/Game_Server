#pragma once
#include "Job.h"
#include "LockQueue.h"
#include "LockFreeQueue.h"
#include "JobTimer.h"

/*--------------
	JobQueue
---------------*/

class JobQueue : public enable_shared_from_this<JobQueue>
{
public:
	void DoAsync(CallbackType&& callback)
	{
		Push(ObjectPool<Job>::MakeShared(std::move(callback)),true);
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsync(Ret(T::*memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		Push(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

	void DoAsyncDB(CallbackType&& callback)
	{
		PushDB(ObjectPool<Job>::MakeShared(std::move(callback)));
	}

	template<typename T, typename Ret, typename... Args>
	void DoAsyncDB(Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		PushDB(ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

	template<typename T, typename Ret, typename... Args>
	void DoLogicJob(int nZone,Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		PushLogicJob(nZone,ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...));
	}

	void DoTimer(uint64 tickAfter, CallbackType&& callback)
	{
		JobRef job = ObjectPool<Job>::MakeShared(std::move(callback));
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	template<typename T, typename Ret, typename... Args>
	void DoTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	template<typename T, typename Ret, typename... Args>
	void DoDBJobTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GDBJobTimer->Reserve(tickAfter, shared_from_this(), job);
	}

	template<typename T, typename Ret, typename... Args>
	void DoZoneJobTimer(uint64 tickAfter, int nZone, Ret(T::* memFunc)(Args...), Args... args)
	{
		shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
		JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
		GZoneJobTimer[nZone]->Reserve(tickAfter, shared_from_this(), job);
	}

	//template<typename T, typename Ret, typename... Args>
	//void DoBroadTimer(uint64 tickAfter, Ret(T::* memFunc)(Args...), Args... args)
	//{
	//	shared_ptr<T> owner = static_pointer_cast<T>(shared_from_this());
	//	JobRef job = ObjectPool<Job>::MakeShared(owner, memFunc, std::forward<Args>(args)...);
	//	GBroadCastTimer->Reserve(tickAfter, shared_from_this(), job);
	//}
	//void					BroadJobPush(JobRef job, bool pushOnly = false);
	//void					BroadCast_Execute();

	void					ClearJobs() { _jobs.Clear(); }

public:
	void					Push(JobRef job, bool pushOnly = false);

	void					PushDB(JobRef job);

	void					PushLogicJob(int ZoneID,JobRef job);



	void					Execute(int nZoneID,int jobType);




protected:
	LockQueue<JobRef>		_jobs;
	Atomic<int32>			_jobCount = 0;

	LockFreeQueue<JobRef>	_freeJobs;
};

