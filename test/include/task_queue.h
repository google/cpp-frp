#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

#include <functional>
#include <future>
#include <mutex>
#include <queue>

namespace frp {
namespace util {

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

} // namespace util
} // namespace frp

#endif // _TASK_QUEUE_H_