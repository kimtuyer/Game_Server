#pragma once
#include <concurrent_queue.h>
template<typename T>
class LockFreeQueue
{
public:

	void LockFree_Push(T item)
	{
		_items.push(item);

	}
	T LockFree_Pop()
	{
		T ret;
		_items.try_pop(ret);
		return ret;
	}
private:
	concurrency::concurrent_queue<T> _items;

};