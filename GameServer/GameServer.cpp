#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include <tchar.h>
#include "Protocol.pb.h"
#include "Job.h"
#include "Room.h"
#include "CMonster.h"
#include "CZone_Manager.h"
#include "ConsoleMapViewer.h"
#include "nlohmann/json.hpp"
#include "cJSON.h"
#include <fstream>
#include "CouchbaseClient.h"
#include "CZone.h"
//#include "tao/json.hpp"
//atomic<int>	g_nPacketCount = 0;

//class CZone_Manager;
array<shared_ptr<ZoneQueue>, Zone::g_nZoneCount + 1> zoneQueues = {};
map<int, bool>	threadRebalance;
void DoMainJob()
{
	while (true)
	{
		//GZoneManager->DoTimer(Tick::AI_TICK, &CZone_Manager::Update);
		//if (LSecondTickCount < GetTickCount64())
		//{
		//	LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;
		//
		//	GConsoleViewer->gotoxy(0, 0);
		//	cout << "전체 초당 패킷 처리량:" << g_nPacketCount << endl;
		//
		//	g_nPacketCount.store(0);
		//}

		//LEndTickCount = ::GetTickCount64() + Tick::WORKER_TICK;
	}
}
void DoWorkerJob(ServerServiceRef& service)
{
	bool bFlag = false;
	while (true)

	{
		if (LSecondTickCount < GetTickCount64())
		{
			LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;
			bFlag = true;
			g_nPacketCount.fetch_add(LPacketCount);

			//cout << LThreadId <<" : 워커스레드의  초당 패킷 처리량:" << LPacketCount << endl;

			LPacketCount = 0;
		}
		LEndTickCount = ::GetTickCount64() + Tick::WORKER_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직까지 (패킷 핸들러에 의해)
		if (service->GetIocpCore()->Dispatch(10) == true)
			LPacketCount++;

		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐
		ThreadManager::DoGlobalQueueWork();
	}
}

void DoIOCPJob(ServerServiceRef& service)
{
	bool bFlag = false;
	while (true)
	{
		if (LSecondTickCount < GetTickCount64())
		{
			LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;
			g_nPacketCount.fetch_add(LPacketCount);

			LPacketCount = 0;
		}
		LEndTickCount = ::GetTickCount64() + Tick::WORKER_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직까지 (패킷 핸들러에 의해)
		if (service->GetIocpCore()->Dispatch(10) == true)
			LPacketCount++;
	}
}

void DoZoneJob(ServerServiceRef& service, int ZoneID)
{
	auto Zone = GZoneManager->GetZone(ZoneID);
	if (Zone == nullptr)
		return;
#ifdef __ZONE_THREAD_VER1__
	zoneQueues[ZoneID] = MakeShared<ZoneQueue>();
	auto& ZoneQueue = zoneQueues[ZoneID];
	//queue<PacketInfo> localqueue;
#endif	
	uint64 lastUpdatetime = 0;
	while (true)
	{
		uint64 nowtime = GetTickCount64();
		uint64 elapsedtime = nowtime - lastUpdatetime;

		if (elapsedtime >= Tick::AI_TICK) {
			Zone->Update();
			lastUpdatetime = nowtime;
		}

		uint64 nextUpdateTime = lastUpdatetime + Tick::AI_TICK;
		uint64 timeUntilNextUpdate = nextUpdateTime - GetTickCount64();


		if (timeUntilNextUpdate > 0) {
			uint64 endTime = timeUntilNextUpdate + GetTickCount64();

			while (GetTickCount64() < endTime)
			{
#ifdef __ZONE_THREAD_VER1__
				PacketInfo job;
				if (ZoneQueue->jobs.try_pop(job))
					ClientPacketHandler::HandlePacket(job.m_session, job.m_buffer, job.m_len);
				else
				{
					ThreadManager::DistributeReservedJobs();

					ThreadManager::DoGlobalQueueWork();

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
#endif // __ZONE_THREAD_VER1__
#ifdef __ZONE_THREAD_VER2__
				if (service->GetIocpCore()->Dispatch(10) == true)
					LPacketCount++;
				else
				{
					ThreadManager::DistributeReservedJobs();

					ThreadManager::DoGlobalQueueWork();

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
#endif
				
			}
		}

		
	}
}


void DoZoneJob3(ServerServiceRef& service, int ZoneID)
{
	threadRebalance.insert({LThreadId, false});
	vector<CZoneRef> Zones;
	vector<pair<int, int>> Zonelist;
	uint64 lastUpdatetime = 0;
	int nbeginSecID = 0;
	int nendSecID = 0;

	if (ZoneID <= g_nZoneCount)
	{
		GZoneManager->PushThreadToZoneList(LThreadId, pair(ZoneID, 0));
		auto Zone = GZoneManager->GetZone(ZoneID);
		if (Zone == nullptr)
		return;
		Zones.push_back(Zone);
	}

	auto getZone = [&]()
		{

			for (auto [zoneid, AssignNumber] : Zonelist)
			{
				if (AssignNumber == 0) // 해당 존은 하나의 스레드 만 할당
				{
					Zones.push_back(GZoneManager->GetZone(zoneid));
					nbeginSecID = 0;
					nendSecID = 0;
				}

				else if (AssignNumber == 1)
				{
					//해당 존 1번째 분할 스레드,  0~ n/2-1 번째까지 섹터만 담당
					Zones.push_back(GZoneManager->GetZone(zoneid));
					nbeginSecID = 1;
					nendSecID = (Sector_Count / 2);
				}

				else
				{
					//해당 존 2번째 분할 스레드,  n/2+1 ~ n-1 번째까지 섹터만 담당
					Zones.push_back(GZoneManager->GetZone(zoneid));
					nbeginSecID = (Sector_Count / 2) + 1;
					nendSecID = (Sector_Count);

				}


			}
		};

	while (true)
	{
		if (threadRebalance[LThreadId] == true)
		{
			Zones.clear();
			Zonelist.clear();

			//새로운 존 정보 재할당 받음.
			if (GZoneManager->ReassignThreadtoZone(LThreadId, Zonelist) == false)
				break;

			getZone();

			threadRebalance[LThreadId] = false;

		}

		if (Zones.empty())
		{
			if (GZoneManager->ReassignThreadtoZone(LThreadId, Zonelist) == false)
				break;
			if(Zonelist.empty()==false)
				getZone();

		}

		uint64 nowtime = GetTickCount64();
		uint64 elapsedtime = nowtime - lastUpdatetime;
		if (elapsedtime >= Tick::AI_TICK) {
			for (auto Zone : Zones)
			{
				int id = Zone->ZoneID();

				if(nbeginSecID==0 && nendSecID==0)
					Zone->Update();
				else
					Zone->Update_Partial(nbeginSecID,nendSecID);

			}
			lastUpdatetime = nowtime;
		}

		uint64 nextUpdateTime = lastUpdatetime + Tick::AI_TICK;
		uint64 timeUntilNextUpdate = nextUpdateTime - GetTickCount64();


		if (timeUntilNextUpdate > 0) {
			uint64 endTime = timeUntilNextUpdate + GetTickCount64();

			while (GetTickCount64() < endTime)
			{

//#ifdef __ZONE_THREAD_VER2__
				if (service->GetIocpCore()->Dispatch(10) == true)
					LPacketCount++;
				else
				{
					ThreadManager::DistributeReservedJobs();

					ThreadManager::DoGlobalQueueWork();

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
//#endif

			}
		}

		LEndTickCount = ::GetTickCount64()- nowtime; //+ Tick::WORKER_TICK;
		GConsoleViewer->update_threadLatency(LThreadId, LEndTickCount,Zonelist);

	}




}
void DoRenderingJob()
{
	while (true)
	{
		if (LSecondTickCount < GetTickCount64())
		{
			LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;

			//GConsoleViewer->refreshCurrentDisplay();
		}

		if (LRenderingTickCount < GetTickCount64())
		{
			LRenderingTickCount = GetTickCount64() + Tick::RENDERING_TICK;

			GConsoleViewer->renderFrame();

			//락 프리 기법 렌더링
			//GConsoleViewer->processUpdates();
		}

		//LEndTickCount = ::GetTickCount64() + Tick::WORKER_TICK;
	}
}

void DoBroadJob(ServerServiceRef& service, bool bMain = false)
{
	//bool bFlag = false;
	while (true)
	{
		//if (LSecondTickCount < GetTickCount64())
		//{
		//	LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;
		//	bFlag = true;
		//	g_nPacketCount.fetch_add(LPacketCount);
		//
		//	//cout << LThreadId <<" : 워커스레드의  초당 패킷 처리량:" << LPacketCount << endl;
		//
		//	LPacketCount = 0;
		//}
		//else
		//{
		//	bFlag = false;
		//	//LPacketCount++;
		//
		//}

		//LEndTickCount = ::GetTickCount64() + Tick::WORKER_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직까지 (패킷 핸들러에 의해)
		//if (service->GetIocpCore()->Dispatch(10) == true)
		//	LPacketCount++;

		ThreadManager::DistributeBroadJobs();

		ThreadManager::DoBroadQueueWork();
		// 예약된 일감 처리

		// 글로벌 큐
		//ThreadManager::DoGlobalQueueWork();
	}
}
using json = nlohmann::json;
json read_json_from_file(const std::string& filename) {
	std::ifstream f(filename);
	json data = json::parse(f);
	return data;
}
int main()
{
	ClientPacketHandler::Init();
	GZoneManager->Init(g_nZoneCount, g_nZoneUserMax);


	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>,
		g_nServerMaxUser);

	ASSERT_CRASH(service->Start());

	unsigned int core_count = std::thread::hardware_concurrency();
	int nThreadCnt = core_count * 2;
	g_nThreadCnt = nThreadCnt;
#ifdef __COUCHBASE_DB__
	g_CouchbaseManager->Init(nThreadCnt + 1);
#endif // __COUCHBASE_DB__

#//ifdef __CONSOLE_UI__
	//#endif
	int zoneID = 1;
#ifdef __ZONE_THREAD_VER3__
	for (int32 i = 0; i < Thread::IOCP_THREADS; i++)
	{
			GThreadManager->Launch([&service]()
				{
					DoIOCPJob(service);
				});

	}

	for (int32 i = 0; i < nThreadCnt-Thread::IOCP_THREADS; i++)
	{
		
		GThreadManager->Launch([&service,&zoneID]()
			{

				DoZoneJob3(service, zoneID++);
			});

		Thread::ZONE_THREADS++;
	}



#else
	for (int32 i = 0; i < nThreadCnt; i++)
	{
		GThreadManager->Launch([&service, i, &zoneID]()
			{
#ifdef __ZONE_THREAD__
				if (i < Thread::IOCP_THREADS)
					DoIOCPJob(service);
				else if (i < g_nZoneCount + Thread::IOCP_THREADS)
				{

					DoZoneJob(service, zoneID++);
				}
#else
				DoWorkerJob(service);
#endif // __ZONE_THREAD__
			});
		Thread::ZONE_THREADS++;

	}

#endif // __ZONE_THREAD_VER3__



	GThreadManager->Launch([]()
		{
			DoRenderingJob();
		});

	//GThreadManager->Launch([]()
	//	{
	//		DoMainJob();
	//	});
	//
	// Main Thread
	GThreadManager->Join();
}