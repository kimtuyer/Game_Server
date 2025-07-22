#pragma once
class GLockFreeQueue
{
public:
	GLockFreeQueue();
	~GLockFreeQueue();

	void					Push(JobQueueRef jobQueue);
	JobQueueRef				Pop();

private:
	//LockQueue<JobQueueRef> _jobQueues;
	LockFreeQueue<JobQueueRef> _freejobQueues;

};
