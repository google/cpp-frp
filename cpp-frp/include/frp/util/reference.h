#ifndef _FRP_UTIL_REFERENCE_H_
#define _FRP_UTIL_REFERENCE_H_

#include <functional>
#include <memory>

namespace frp {
namespace util {

template<typename T>
struct unwrap_type {
	typedef T type;
};

template<typename T>
struct unwrap_type<std::reference_wrapper<T>> : unwrap_type<T> {};

template<typename T>
struct unwrap_type<std::shared_ptr<T>> : unwrap_type<T> {};

template<typename T>
struct unwrap_type<std::unique_ptr<T>> : unwrap_type<T> {};

template<typename T>
using unwrap_t = typename unwrap_type<T>::type;

template<typename T>
struct unwrap_reference_type {
	static T &get(T &value) {
		return value;
	}
};

template<typename T>
struct unwrap_reference_type<std::reference_wrapper<T>> {
	static T &get(const std::reference_wrapper<T> &value) {
		return value;
	}
};

template<typename T>
struct unwrap_reference_type<std::shared_ptr<T>> {
	static T &get(const std::shared_ptr<T> &value) {
		return *value;
	}
};

template<typename T>
struct unwrap_reference_type<std::unique_ptr<T>> {
	static T &get(const std::unique_ptr<T> &value) {
		return *value;
	}
};

template<typename T>
auto &unwrap_reference(T &value) {
	return unwrap_reference_type<T>::get(value);
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_REFERENCE_H_
