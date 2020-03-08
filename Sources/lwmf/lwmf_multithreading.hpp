/*
***************************************************************
*                                                             *
* lwmf_multithreading - lightweight media framework           *
*                                                             *
* (C) 2019 - present by Stefan Kubsch                         *
*                                                             *
***************************************************************
*/

#pragma once

#include <vector>
#include <thread>
#include <future>
#include <memory>
#include <utility>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace lwmf
{


	class Multithreading final
	{
	public:
		Multithreading();
		~Multithreading();

		template<class F, class... Args>void AddThread(F&& f, Args&& ... args);
		void WaitForThreads();

	private:
		std::vector<std::thread> Workers{};
		std::vector<std::future<void>> Results{};
		std::queue<std::packaged_task<void()>> Tasks{};
		std::mutex QueueMutex{};
		std::condition_variable Condition{};
		bool Stop{};
	};

	inline Multithreading::Multithreading()
	{
		for (std::size_t i{}; i < std::thread::hardware_concurrency(); ++i)
		{
			Workers.emplace_back([this]
			{
				while (true)
				{
					std::packaged_task<void()> Task;
					{
						std::unique_lock<std::mutex> lock(this->QueueMutex);
						this->Condition.wait(lock, [this] { return this->Stop || !this->Tasks.empty(); });

						if (this->Stop && this->Tasks.empty())
						{
							return;
						}

						Task = std::move(Tasks.front());
						Tasks.pop();
					}

					Task();
				}
			});
		}
	}

	inline Multithreading::~Multithreading()
	{
		{
			std::unique_lock<std::mutex> lock(QueueMutex);
			Stop = true;
		}

		Condition.notify_all();

		for (auto&& Worker : Workers)
		{
			Worker.join();
		}
	}

	template<class F, class... Args>void Multithreading::AddThread(F&& f, Args&& ... args)
	{
		std::packaged_task<std::invoke_result_t<F, Args...>()> Task(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		Results.emplace_back(Task.get_future());

		{
			std::unique_lock<std::mutex> lock(QueueMutex);
			Tasks.emplace(std::move(Task));
		}

		Condition.notify_one();
	}

	inline void Multithreading::WaitForThreads()
	{
		for (const auto& Result : Results)
		{
			Result.wait();
		}

		Results.clear();
		Results.shrink_to_fit();
	}


} // namespace lwmf