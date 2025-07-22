#include "pch.h"
#include "GlobalQueue.h"

/*----------------
	GlobalQueue
-----------------*/

GlobalQueue::GlobalQueue()
{

}

GlobalQueue::~GlobalQueue()
{

}

void GlobalQueue::Push(JobQueueRef jobQueue)
{
	_jobQueues.Push(jobQueue);
}

JobQueueRef GlobalQueue::Pop()
{
	return _jobQueues.Pop();
}

//GLockFreeQueue::GLockFreeQueue()
//{
//}
//
//GLockFreeQueue::~GLockFreeQueue()
//{
//}
//
//void GLockFreeQueue::Push(JobQueueRef jobQueue)
//{
//	_freejobQueues.LockFree_Push(jobQueue);
//}
//
//JobQueueRef GLockFreeQueue::Pop()
//{
//	return _freejobQueues.LockFree_Pop();
//}
