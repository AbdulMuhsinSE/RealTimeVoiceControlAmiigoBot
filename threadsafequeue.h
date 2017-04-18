#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue
{
	public:
	T pop();
	void pop(T& item);
	void push(const T& item);
	void push (T&& item);
	
	private:
	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;
};
