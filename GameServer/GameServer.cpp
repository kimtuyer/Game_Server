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
//#include "tao/json.hpp"
//atomic<int>	g_nPacketCount = 0;

//class CZone_Manager;
void DoMainJob(ServerServiceRef& service)
{
	while (true)
	{
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
	
		//GRoom->DoTimer(1000, [] { cout << "Hello 1000" << endl; });
	//GRoom->DoTimer(2000, [] { cout << "Hello 2000" << endl; });
	//CouchbaseClient* g_pCouchbaseClient=nullptr;
	//try {
	//	 g_pCouchbaseClient = new CouchbaseClient("couchbase://localhost",
	//		"default",//"default",
	//		"admin",
	//		"552123");
	//
	//	//	
	//	//
	//}
	//catch (const std::exception& e) {
	//	delete g_pCouchbaseClient;
	//std::cerr << "Error: " << e.what() << std::endl;
	//	return 1;
	//}
	//string query;
	////json document1 = { {"name", "John Doe"}, {"age", 30} };
	//document cb;
	//cb.key = "User5";
	//cb.cas = 0;
	//cb.threadID = 1;
	//g_pCouchbaseClient->get(cb.key, cb);

	
	//document1.insert()

	//json document1 = { {"name", "John Doe"}, {"age", 30} };
//	string doculment_id = "User";
	//doculment_id += "5";
	//string json_string = document.dump();

	//cb.key = doculment_id;
	//cb.value = document1.dump();
	//cb.cas = 0; //최초 저장 문서시

	//g_pCouchbaseClient->upsert(cb);


	//// 데이터 조회
	//GetCallback cb;
	//cb.threadid = 1;
	//auto value = g_pCouchbaseClient->get("user:1", LCB_WAIT_DEFAULT,cb);
	//
	//GetCallback cb2;
	//cb2.threadid = 2;
	// value = g_pCouchbaseClient->get("user:2", LCB_WAIT_DEFAULT, cb2);

	//if (doc_content.value.length() > 0)
	//	std::cout << "Retrieved value: " << doc_content.value << std::endl;
	

	//printf("Libcouchbase version: %s\n", lcb_get_version(NULL));


	ClientPacketHandler::Init();
	GZoneManager->Init(g_nZoneCount, g_nZoneUserMax);
	
	//ZoneManager()->Init();

	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>, // TODO : SessionManager 등
		g_nServerMaxUser);

	ASSERT_CRASH(service->Start());

	unsigned int core_count = std::thread::hardware_concurrency();
	int nThreadCnt = core_count * 2;//+ 1;
#ifdef __COUCHBASE_DB__
	g_CouchbaseManager->Init(nThreadCnt + 1);
#endif // __COUCHBASE_DB__

#//ifdef __CONSOLE_UI__
	GThreadManager->Launch([]()
		{
			DoRenderingJob();
		});
//#endif	
	for (int32 i = 0; i < nThreadCnt; i++)
	{
		

		GThreadManager->Launch([&service]()
			{
				DoWorkerJob(service);
			});
	}
	

	// Main Thread
	//DoMainJob(service);
	GThreadManager->Join();
}