#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "Memory.h"
#include "DeadLockProfiler.h"
#include "SocketUtils.h"
#include "SendBuffer.h"
#include "GlobalQueue.h"
#include "JobTimer.h"
#include  "RandomMove.h"
#include "ConsoleMapViewer.h"
#include "GLockFreeQueue.h"
ThreadManager*		GThreadManager = nullptr;
Memory*				GMemory = nullptr;
SendBufferManager*	GSendBufferManager = nullptr;
GlobalQueue*		GGlobalQueue = nullptr;
GlobalQueue*		GDBQueue = nullptr;
#ifdef __LOCKFREE__
vector<GLockFreeQueue*>		GZoneLogicQueue;// = nullptr;
#else
vector<GlobalQueue*>		GZoneLogicQueue;// = nullptr;
#endif // __LOCKFREE__


JobTimer*			GJobTimer = nullptr;
JobTimer*			GDBJobTimer = nullptr;
vector<JobTimer*>		GZoneJobTimer;// = nullptr;

//JobTimer*			GBroadCastTimer = nullptr;
//GlobalQueue*		GBroadQueue = nullptr;
RandomMove* GRandomMove = nullptr;

ConsoleMapViewer* GConsoleViewer = nullptr;
DeadLockProfiler*	GDeadLockProfiler = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GMemory = new Memory();
		GSendBufferManager = new SendBufferManager();
		GGlobalQueue = new GlobalQueue();
		GJobTimer = new JobTimer();
		GDBJobTimer = new JobTimer();
		GDBQueue = new GlobalQueue();
		//GZoneLogicQueue.resize(g_nZoneCount);
		for (int i = 0; i <= g_nZoneCount; i++)
#ifdef __LOCKFREE__
			GZoneLogicQueue.push_back(new GLockFreeQueue);
#else
			GZoneLogicQueue.push_back(new GlobalQueue);
#endif // __LOCKFREE__
		for (int i = 0; i <= g_nZoneCount; i++)
			GZoneJobTimer.push_back(new JobTimer);
		//GZoneJobTimer
		//GBroadQueue = new GlobalQueue();
		//GBroadCastTimer = new JobTimer();

		GDeadLockProfiler = new DeadLockProfiler();
		GRandomMove = new RandomMove();
		GConsoleViewer = new ConsoleMapViewer();
		SocketUtils::Init();
	
	
	}

	~CoreGlobal()
	{
		delete GThreadManager;
		delete GMemory;
		delete GSendBufferManager;
		delete GGlobalQueue;
		delete GJobTimer;
		delete GDeadLockProfiler;
		delete GRandomMove;
		delete GConsoleViewer;
		delete GDBJobTimer;
		delete GDBQueue;

		for (auto i : GZoneLogicQueue)
		{
			delete i;
		}
		for (auto i : GZoneJobTimer)
		{
			delete i;
		}

		
		SocketUtils::Clear();
	}
} GCoreGlobal;