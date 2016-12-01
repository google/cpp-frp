/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

	thread_pool() = delete;
	thread_pool(thread_pool &&copy) = default;
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
