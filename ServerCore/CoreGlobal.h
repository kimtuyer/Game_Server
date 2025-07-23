#pragma once
#include "Dev_define.h"
#include <vector>
#ifdef __ZONEQUEUE_LOCKFREE__
extern class GLockFreeQueue;
#endif
extern class ThreadManager*		GThreadManager;
extern class Memory*			GMemory;
extern class SendBufferManager* GSendBufferManager;
extern class GlobalQueue*		GGlobalQueue;
extern class GlobalQueue* GDBQueue;

#ifdef __ZONEQUEUE_LOCKFREE__
extern  std::vector<GLockFreeQueue*> GZoneLogicQueue;
#else
extern  std::vector<GlobalQueue*> GZoneLogicQueue;
#endif // __ZONEQUEUE_LOCKFREE__

extern class JobTimer*			GJobTimer;
extern class JobTimer*			GDBJobTimer;
extern  std::vector<JobTimer*> GZoneJobTimer;

//extern class JobTimer*			GBroadCastTimer;
//extern class GlobalQueue*		GBroadQueue;

extern class DeadLockProfiler*	GDeadLockProfiler;
extern class RandomMove* GRandomMove;
extern class ConsoleMapViewer*  GConsoleViewer;

