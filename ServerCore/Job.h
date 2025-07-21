#pragma once
#include <functional>

/*---------
	Job
----------*/

using CallbackType = std::function<void()>;

class Job
{
public:
	Job(CallbackType&& callback) : _callback(std::move(callback))
	{
	}

	template<typename T, typename Ret, typename... Args>
	Job(shared_ptr<T> owner, Ret(T::* memFunc)(Args...), Args&&... args)
	{
		/*_callback = [owner, memFunc, args...]()
		{
			(owner.get()->*memFunc)(args...);
		};*/

		// weak_ptr로 순환 참조를 방지합니다.
		std::weak_ptr<T> weakOwner = owner;
		_callback = [weakOwner, memFunc, args...]()
			{
				// Job 실행 시점에 owner가 유효한지 확인하고 사용합니다.
				if (std::shared_ptr<T> owner = weakOwner.lock())
				{
					(owner.get()->*memFunc)(args...);
				}
			};
	}

	void Execute()
	{
		_callback();
	}

private:
	CallbackType _callback;
};

