#include "pch.h"
#include "GLockFreeQueue.h"

GLockFreeQueue::GLockFreeQueue()
{
}

GLockFreeQueue::~GLockFreeQueue()
{
}

void GLockFreeQueue::Push(JobQueueRef jobQueue)
{
	_freejobQueues.LockFree_Push(jobQueue);
}

JobQueueRef GLockFreeQueue::Pop()
{
	return _freejobQueues.LockFree_Pop();
}