#include "pch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "BufferReader.h"
#include "ServerPacketHandler.h"
#include "ClientSession.h"
char sendData[] = "Hello World";

void DoWorkerJob(ClientServiceRef& service, bool bMain = false)
{
	bool bFlag = false;
	while (true)
	{
		if (LSecondTickCount < GetTickCount64())
		{
			LSecondTickCount = GetTickCount64() + Tick::SECOND_TICK;
			bFlag = true;
			//g_nPacketCount.fetch_add(LPacketCount);

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
		if (service->GetIocpCore()->Dispatch(10) == true)
			LPacketCount++;


		// 예약된 일감 처리
		ThreadManager::DistributeReservedJobs();

		// 글로벌 큐
		ThreadManager::DoGlobalQueueWork();
	}
}

int main()
{
	ServerPacketHandler::Init();

	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<ClientSession>, // TODO : SessionManager 등
		g_nClientMaxCount); //1000일땐 500메가, 2000일땐 900메가 
	
	ASSERT_CRASH(service->Start());

	unsigned int core_count = std::thread::hardware_concurrency();
	int nThreadCnt = 10; //core_count * 2 + 1;
	for (int32 i = 0; i < nThreadCnt; i++)
	{
		GThreadManager->Launch([&service]()
			{
				//while (true)
				{
					DoWorkerJob(service);
					//service->GetIocpCore()->Dispatch();
				}
			});
	}

	Protocol::C_LOGIN chatPkt;

	//string strTemp;
	//cin >> strTemp;
	//
	//chatPkt.set_msg(strTemp);

	//chatPkt.set_msg(u8"Hello World !");
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(chatPkt);
	service->Broadcast(sendBuffer);

	//while (true)  
	//{
	//	service->Broadcast(sendBuffer);
	//	this_thread::sleep_for(1s);
	//}

	GThreadManager->Join();
}