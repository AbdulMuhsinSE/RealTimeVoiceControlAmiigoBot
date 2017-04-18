#include "threadsafequeue.h"

T pop()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while
	{
		cond.wait(mlock);
	}
	auto item = queue_.front();
	queue_.pop();
	return item;
}

void pop(T& item)
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while
	{
		cond.wait(mlock);
	}
	item = queue_.front();
	queue_.pop();
}

void push(const T& item)
{
	std::unique_lock<std::mutex> mlock(mutex_);
	queue_.push(item);
	mlock.unlock();
	cond_.notify_one();
}

void push(T&& item)
{
	std::unique_lock<std::mutex> mlock(mutex_);
	queue_.push(item);
	mlock.unlock();
	cond_.notify_one();
}
