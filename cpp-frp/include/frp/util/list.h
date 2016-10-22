#ifndef _FRP_UTIL_LIST_H_
#define _FRP_UTIL_LIST_H_

#include <list>
#include <mutex>

namespace frp {
namespace util {

// TODO(gardell): Use/create a lock-free list with the following interface
template<typename T>
struct list_type {
	typedef std::list<T> internal_list_type;
	typedef typename internal_list_type::iterator iterator;
	internal_list_type list;
	mutable std::mutex mutex;

	auto append(T &&value) {
		std::lock_guard<std::mutex> lock(mutex);
		return list.insert(list.end(), std::forward<T>(value));
	}

	void erase(const iterator &iterator) {
		std::lock_guard<std::mutex> lock(mutex);
		list.erase(iterator);
	}

	template<typename F>
	void for_each(F &&f) const {
		std::lock_guard<std::mutex> lock(mutex);
		for (const auto &value : list) {
			f(value);
		}
	}
};

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_LIST_H_
