#ifndef _FRP_UTIL_COLLECTOR_H_
#define _FRP_UTIL_COLLECTOR_H_

#include <atomic>
#include <frp/util/list.h>
#include <vector>

namespace frp {
namespace util {

template<typename T, typename F>
struct collector_type {
	F f;
	single_list_type<T> list;
	std::atomic_size_t counter;
	const std::size_t num_elements;

	collector_type(F &&f, std::size_t num_elements)
		: f(std::forward<F>(f)), counter(num_elements), num_elements(num_elements) {}

	void append_and_call_if_ready(T &&value) {
		list.insert(std::forward<T>(value));
		decrease();
	}

	void append_and_call_if_ready(const T &value) {
		list.insert(value);
		decrease();
	}

	void decrease() {
		if (--counter == 0) {
			std::vector<T> vector;
			vector.reserve(num_elements);
			list.for_each([&](auto &value) {
				vector.push_back(std::move(value));
			});
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
