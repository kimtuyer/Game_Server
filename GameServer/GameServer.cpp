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
atomic<int>	g_nPacketCount = 0;
//class CZone_Manager;
void DoMainJob(ServerServiceRef& service)
{
	while (true)
	{
		if (LSecondTickCount < GetTickCount64())
		{
			LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;


			cout << "전체 초당 패킷 처리량:" << g_nPacketCount << endl;

			g_nPacketCount.store(0);
		}

		LEndTickCount = ::GetTickCount64() + Tick::WORKER_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직까지 (패킷 핸들러에 의해)
		service->GetIocpCore()->Dispatch(10);
		
		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐
		ThreadManager::DoGlobalQueueWork();
	}
}

void DoWorkerJob(ServerServiceRef& service,bool bMain=false)
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
		else
		{
			bFlag = false;
			//LPacketCount++;

		}

		LEndTickCount = ::GetTickCount64() + Tick::WORKER_TICK;

		// 네트워크 입출력 처리 -> 인게임 로직까지 (패킷 핸들러에 의해)
		if(service->GetIocpCore()->Dispatch(10)==true )
			LPacketCount++;


		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐
		ThreadManager::DoGlobalQueueWork();
	}
}

int main()
{
	//GRoom->DoTimer(1000, [] { cout << "Hello 1000" << endl; });
	//GRoom->DoTimer(2000, [] { cout << "Hello 2000" << endl; });

	ClientPacketHandler::Init();
	GZoneManager->Init();
	//ZoneManager()->Init();

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>, // TODO : SessionManager 등
		2200);

	ASSERT_CRASH(service->Start());

	unsigned int core_count = std::thread::hardware_concurrency();
	int nThreadCnt = 10;//core_count * 2 + 1;
	for (int32 i = 0; i < nThreadCnt; i++)
	{
		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}

	// Main Thread
	DoMainJob(service);

	GThreadManager->Join();
}