#pragma once
#include <stack>

extern thread_local uint32				LThreadId;
extern thread_local uint64				LEndTickCount;
extern thread_local	uint64				LSecondTickCount;
extern thread_local  uint64				LMinuteTickCount;
extern thread_local  uint64				LPacketCount;
extern thread_local  uint64				LRenderingTickCount;


extern thread_local std::stack<int32>	LLockStack;
extern thread_local SendBufferChunkRef	LSendBufferChunk;
extern thread_local class JobQueue*		LCurrentJobQueue;