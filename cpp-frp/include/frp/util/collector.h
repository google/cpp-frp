#ifndef _FRP_UTIL_COLLECTOR_H_
#define _FRP_UTIL_COLLECTOR_H_

#include <atomic>
#include <mutex>
#include <vector>

namespace frp {
namespace util {

template<typename T, typename F>
struct collector_type {
	F f;
	std::vector<T> vector;
	std::atomic_size_t counter;
	std::mutex mutex;

	collector_type(F &&f, std::size_t num_elements)
		: f(std::forward<F>(f)), counter(num_elements) {
		vector.reserve(num_elements);
	}

	void append_and_call_if_ready(T &&value) {
		{
			std::lock_guard<std::mutex> lock(mutex);
			vector.push_back(std::forward<T>(value));
		}
		decrease();
	}

	void append_and_call_if_ready(const T &value) {
		{
			std::lock_guard<std::mutex> lock(mutex);
			vector.push_back(value);
		}
		decrease();
	}

	void decrease() {
		if (--counter == 0) {
			f(std::move(vector));
		}
	}
};

template<typename T, typename F>
auto make_collector_type(F &&f, std::size_t num_elements) {
	return std::make_shared<collector_type<T, F>>(std::forward<F>(f), num_elements);
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_COLLECTOR_H_
