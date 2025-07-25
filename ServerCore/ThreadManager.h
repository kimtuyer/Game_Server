#pragma once

#include <thread>
#include <functional>
//#include "Enum.h"
/*------------------
	ThreadManager
-------------------*/


class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void	Launch(function<void(void)> callback);
	void	Join();

	static void InitTLS();
	static void DestroyTLS();

	static bool DoGlobalQueueWork();
	static void DistributeReservedJobs();


	static bool DoDBQueueWork();
	static void DistributeDBJobs();


	static bool DoZoneQueueWork(int nZoneID);
	static void DistributeZoneJobs(int nZoneID);


	//array<unique_ptr<ZoneQueue>, Zone::g_nZoneCount+1> zoneQueues;
private:
	Mutex			_lock;
	vector<thread>	_threads;

};

