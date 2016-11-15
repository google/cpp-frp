#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <task_queue.h>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

struct thread_pool {
private:
	struct instance_type {
	public:
		explicit instance_type(std::size_t num_threads)
			: running(true), threads(num_threads) {
			for (std::thread &thread : threads) {
				thread = std::thread([this]() {
					for (;;) {
						std::unique_lock<std::mutex> lock(mutex);
						cv.wait(lock, [this]() { return !running || !tasks.empty(); });
						if (!running) {
							break;
						}
						tasks.process_one();
					}
				});
			}
		}

		~instance_type() {
			{
				std::unique_lock<std::mutex> lock(mutex);
				running = false;
			}
			cv.notify_all();
			for (std::thread &thread : threads) {
				thread.join();
			}
		}

		template<typename Function, typename... Args>
		auto operator() (Function function, Args... args) {
			auto future(tasks(std::forward<Function>(function),
				std::forward<Args>(args)...));
			cv.notify_one();
			return future;
		}

		std::size_t num_threads() const {
			return threads.size();
		}

	private:
		bool running;
		std::condition_variable cv;
		std::mutex mutex;
		task_queue tasks;
		std::vector<std::thread> threads;
	};
public:
	explicit thread_pool(std::size_t num_threads)
		: instance(new instance_type(num_threads)) {}

	thread_pool() = default;
	thread_pool(thread_pool &&) = default;
	thread_pool(const thread_pool &) = delete;
	thread_pool &operator=(thread_pool &&) = default;
	thread_pool &operator=(const thread_pool &) = default;

	template<typename Function, typename... Args>
	auto operator() (Function function, Args... args) const {
		return instance->operator()(std::forward<Function>(function),
			std::forward<Args>(args)...);
	}

	std::size_t num_threads() const {
		return instance->num_threads();
	}

private:
	std::unique_ptr<instance_type> instance;
};

#endif // _THREAD_POOL_H_
