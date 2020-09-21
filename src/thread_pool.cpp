#include <thread_pool.h>

#include <iostream>

ThreadPool::ThreadPool(int count)
{
	pool.reserve(count);
	for (int i = 0; i < count; i++)
	{
		pool.emplace_back(std::thread(&ThreadPool::threadInfiniteCycle, this));
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(jobMutex);
		terminate = true;
	}

	cond.notify_all();
	for (auto &t : pool)
	{
		t.join();
	}
}

void ThreadPool::threadInfiniteCycle()
{
	while(true)
	{
		jobType job;
		{
			std::unique_lock lock(jobMutex);

			cond.wait(lock, [ this ]{ return !jobQueue.empty() || terminate; });
			if (terminate && jobQueue.empty())
			{
				return;
			}
			job = jobQueue.front();
			jobQueue.pop_front();
		}
		try
		{
			job();
		}
		catch(const std::runtime_error &err)
		{
			std::unique_lock lock(outMutex);
			std::cerr << "[" << std::this_thread::get_id() << "] " << err.what() << "\n";
		}
	}
}

void ThreadPool::pushJob(jobType job)
{
	std::unique_lock lock(jobMutex);
	jobQueue.push_back(std::move(job));
	cond.notify_one();
}

void ThreadPool::pushTopJob(jobType job)
{
	std::unique_lock lock(jobMutex);
	jobQueue.push_front(std::move(job));
	cond.notify_one();
}
