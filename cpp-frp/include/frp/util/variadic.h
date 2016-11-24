#ifndef _FRP_UTIL_VARIADIC_H_
#define _FRP_UTIL_VARIADIC_H_

#include <type_traits>

namespace frp {
namespace util {

template<typename... T>
struct all_true_type;

template<>
struct all_true_type<> : std::true_type {};

template<>
struct all_true_type<std::false_type> : std::false_type {};

template<typename... T>
struct all_true_type<std::false_type, T...> : std::false_type {};

template<typename... T>
struct all_true_type<std::true_type, T...> : all_true_type<T...> {};

template<typename T>
struct is_not_void : std::integral_constant<bool, !std::is_void<T>::value> {};

namespace details {

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

} // namespace details

template<typename... Ts>
bool all_true(Ts &...ptrs) {
	return !details::all_true_type<Ts...>::any_false(ptrs...);
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_VARIADIC_H_
