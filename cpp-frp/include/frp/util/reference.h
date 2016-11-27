#ifndef _FRP_UTIL_REFERENCE_H_
#define _FRP_UTIL_REFERENCE_H_

#include <functional>
#include <memory>

namespace frp {
namespace util {

template<typename T>
struct unwrap_reference_type {
	typedef T value_type;

	static decltype(auto) get(T &value) {
		return value;
	}

	static decltype(auto) get(T &&value) {
		return std::forward<T>(value);
	}
};

template<typename T>
struct unwrap_reference_type<std::reference_wrapper<T>> {
	typedef T value_type;

	static T &get(const std::reference_wrapper<T> &value) {
		return value;
	}
};

template<typename T>
using unwrap_reference_t = typename unwrap_reference_type<std::decay_t<T>>::value_type;

template<typename T>
decltype(auto) unwrap_reference(T &value) {
	return unwrap_reference_type<T>::get(value);
}

template<typename T>
decltype(auto) unwrap_reference(T &&value) {
	return unwrap_reference_type<T>::get(std::forward<T>(value));
}

template<typename T>
struct unwrap_container_type {
	typedef T value_type;

	static decltype(auto) get(T &value) {
		return value;
	}

	static decltype(auto) get(const T &value) {
		return value;
	}

	static decltype(auto) get(T &&value) {
		return std::forward<T>(value);
	}
};

template<typename T>
struct unwrap_container_type<std::reference_wrapper<T>> {
	typedef T value_type;

	static T &get(const std::reference_wrapper<T> &value) {
		return value;
	}
};

template<typename T>
struct unwrap_container_type<std::shared_ptr<T>> {
	typedef T value_type;

	static T &get(const std::shared_ptr<T> &value) {
		return *value;
	}
};

template<typename T>
struct unwrap_container_type<std::unique_ptr<T>> {
	typedef T value_type;

	static T &get(const std::unique_ptr<T> &value) {
		return *value;
	}
};

template<typename T>
decltype(auto) unwrap_container(T &value) {
	return unwrap_container_type<std::decay_t<T>>::get(value);
}

template<typename T>
decltype(auto) unwrap_container(T &&value) {
	return unwrap_container_type<std::decay_t<T>>::get(std::forward<T>(value));
}

template<typename T>
using unwrap_container_t = typename unwrap_container_type<std::decay_t<T>>::value_type;

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_REFERENCE_H_
