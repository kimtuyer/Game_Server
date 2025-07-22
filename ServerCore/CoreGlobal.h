#pragma once
#include <vector>
extern class ThreadManager*		GThreadManager;
extern class Memory*			GMemory;
extern class SendBufferManager* GSendBufferManager;
extern class GlobalQueue*		GGlobalQueue;
extern class GlobalQueue* GDBQueue;
extern  std::vector<GlobalQueue*> GZoneLogicQueue;
extern class JobTimer*			GJobTimer;
extern class JobTimer*			GDBJobTimer;
extern  std::vector<JobTimer*> GZoneJobTimer;

//extern class JobTimer*			GBroadCastTimer;
//extern class GlobalQueue*		GBroadQueue;

extern class DeadLockProfiler*	GDeadLockProfiler;
extern class RandomMove* GRandomMove;
extern class ConsoleMapViewer*  GConsoleViewer;

