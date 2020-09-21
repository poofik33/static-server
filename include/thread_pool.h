#ifndef STATICSERVER_THREAD_POOL_H
#define STATICSERVER_THREAD_POOL_H

#include <thread>
#include <vector>
#include <queue>

using jobType = std::function<void()>;

class ThreadPool
{
public:
	ThreadPool(int count);
	~ThreadPool();

	ThreadPool(const ThreadPool &) = delete;
	ThreadPool & operator= (const ThreadPool &) = delete;

	ThreadPool(ThreadPool &&p) : pool(std::move(p.pool)) { }
	ThreadPool & operator= (ThreadPool &&p) noexcept { pool = std::move(p.pool); return *this; };

	void threadInfiniteCycle();
	void pushJob(jobType job);
	void pushTopJob(jobType job);
private:
	std::deque<jobType> jobQueue;
	std::vector<std::thread> pool;

	std::mutex jobMutex;
	std::condition_variable cond;
	bool terminate = false;
};

#endif //STATICSERVER_THREAD_POOL_H
