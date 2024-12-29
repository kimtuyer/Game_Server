#include "pch.h"
#include "CoreTLS.h"

thread_local uint32				LThreadId = 0;
thread_local uint64				LEndTickCount = 0;
thread_local std::stack<int32>	LLockStack;
thread_local SendBufferChunkRef	LSendBufferChunk;
thread_local JobQueue*			LCurrentJobQueue = nullptr;


thread_local uint64				LSecondTickCount = 0;
thread_local uint64				LMinuteTickCount = 0;
thread_local uint64				LPacketCount = 0;
thread_local uint64				LRenderingTickCount = 0;

thread_local string				doc_content;
thread_local CouchbaseClient*	LCouchDBConnect=nullptr;


