#ifndef _CACHE_H_
#define _CACHE_H_

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

template<typename K, typename V>
struct cache_type {
	typedef std::mutex mutex_type;
	typedef std::unordered_map<K, std::weak_ptr<V>> container_type;
	typedef std::function<V(const K &)> function_type;
	container_type container;
	mutex_type container_mutex;
	function_type function;

	template<typename F>
	explicit cache_type(F &&f) : function(std::forward<F>(f)) {}

	auto operator()(const K &key) {
		std::lock_guard<mutex_type> lock(container_mutex);
		auto it(container.find(key));
		std::shared_ptr<V> value;
		if (it == container.end() || !(value = it->second.lock())) {
			value = std::make_shared<V>(function(key));
			container.emplace(key, value);
		}
		return value;
	}
};

#endif // _CACHE_H_
