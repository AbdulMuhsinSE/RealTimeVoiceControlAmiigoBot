#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_J

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue
{
	public:
	
	T pop() 
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		while (queue_.empty())
		{
			cond_.wait(mlock);
		}
		auto item = queue_.front();
		queue_.pop();
		return item;
	}
	
	void pop(T& item)
    {
      std::unique_lock<std::mutex> mlock(mutex_);
      while (queue_.empty())
      {
		cond_.wait(mlock);
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
	
	void push (T&& item)
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		queue_.push(item);
		mlock.unlock();
		cond_.notify_one();
	}
	
	ThreadSafeQueue()=default;
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;            // disable copying
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete; // disable assignment
	
	private:
	
	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;
};

#endif
