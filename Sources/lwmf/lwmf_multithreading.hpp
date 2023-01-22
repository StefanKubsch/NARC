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
#include <tuple>
#include <utility>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace lwmf
{


	class Multithreading final
	{
	public:
		Multithreading();
		Multithreading(const Multithreading&) = delete;
		Multithreading(Multithreading&&) = delete;
		Multithreading& operator = (const Multithreading&) = delete;
		Multithreading& operator = (Multithreading&&) = delete;
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
		const std::size_t NumberOfThreads{ static_cast<std::size_t>(std::thread::hardware_concurrency()) };
		Workers.reserve(NumberOfThreads);
		LWMFSystemLog.AddEntry(LogLevel::Trace, __FILENAME__, __LINE__, "lwmf::Multithreading() (variable name:NumberOfThreads, value: " + std::to_string(NumberOfThreads) + ")");

		for (std::size_t i{}; i < NumberOfThreads; ++i)
		{
			Workers.emplace_back([this]
			{
				while (true)
				{
					std::packaged_task<void()> Task;
					{
						std::unique_lock<std::mutex> lock(QueueMutex);
						Condition.wait(lock, [this] { return Stop || !Tasks.empty(); });

						if (Stop && Tasks.empty())
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
		std::packaged_task<std::invoke_result_t<F, Args...>()> Task([func = std::forward<F>(f),	args = std::make_tuple(std::forward<Args>(args)...)]()
		{
			return std::apply(func, std::move(args));
		});

		Results.emplace_back(Task.get_future());
		std::unique_lock<std::mutex> lock(QueueMutex);
		Tasks.emplace(std::move(Task));
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