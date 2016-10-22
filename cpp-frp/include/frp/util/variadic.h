#ifndef _FRP_UTIL_VARIADIC_H_
#define _FRP_UTIL_VARIADIC_H_

namespace frp {
namespace util {

template<typename... Ts>
struct all_true_type {};

template<typename T, typename... Ts>
struct all_true_type<T, Ts...> {
	static bool any_false(T &ptr, Ts &...ptrs) {
		return !ptr || all_true_type<Ts...>::any_false(ptrs...);
	}
};

template<>
struct all_true_type<> {
	static bool any_false() {
		return false;
	}
};

template<typename... Ts>
bool all_true(Ts &...ptrs) {
	return !all_true_type<Ts...>::any_false(ptrs...);
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_VARIADIC_H_
