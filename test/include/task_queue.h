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
#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

#include <functional>
#include <future>
#include <mutex>
#include <queue>

struct task_queue {
private:
	typedef std::function<void()> task_type;

	template<typename T, typename Function, typename... Args>
	struct execute_type {
		static void execute(const std::shared_ptr<std::promise<T>> &promise,
			Function &function, Args&... args) {
			promise->set_value(function(args...));
		}
	};
	template<typename Function, typename... Args>
	struct execute_type<void, Function, Args...> {
		static void execute(const std::shared_ptr<std::promise<void>> &promise,
			Function &function, Args&... args) {
			function(args...);
			promise->set_value();
		}
	};
public:
	template<typename Function, typename... Args>
	auto operator() (Function function, Args... args) {
		typedef decltype(function(std::forward<Args>(args)...)) result_type;
		typedef std::promise<result_type> promise_type;
		promise_type promise;
		std::future<result_type> future(promise.get_future());
		std::lock_guard<std::mutex> lock(tasks_mutex);
		// TODO(gardell): queue should support move-only objects
		// and not require a shared_ptr here.
		tasks.push(std::bind(
			&task_queue::execute_type<result_type, Function, Args...>::execute,
			std::make_shared<promise_type>(std::move(promise)),
			std::forward<Function>(function), std::forward<Args>(args)...));
		return future;
	}

	void process_one() {
		task_type task;
		{
			std::lock_guard<std::mutex> lock(tasks_mutex);
			if (tasks.empty()) {
				return;
			}
			task = std::move(tasks.front());
			tasks.pop();
		}
		task();
	}

	void process_all() {
		tasks_type tasks;
		{
			std::lock_guard<std::mutex> lock(tasks_mutex);
			tasks = std::move(this->tasks);
		}
		while (!tasks.empty()) {
			tasks.front()();
			tasks.pop();
		}
	}

	bool empty() const {
		std::lock_guard<std::mutex> lock(tasks_mutex);
		return tasks.empty();
	}

private:
	typedef std::queue<task_type> tasks_type;
	tasks_type tasks;
	mutable std::mutex tasks_mutex;
};

#endif // _TASK_QUEUE_H_
