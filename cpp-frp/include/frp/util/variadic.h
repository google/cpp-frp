#ifndef _FRP_UTIL_VARIADIC_H_
#define _FRP_UTIL_VARIADIC_H_

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
struct is_not_void : std::bool_constant<!std::is_void<T>::value> {};

template<typename... Ts>
struct all_true_impl {};

template<typename T, typename... Ts>
struct all_true_impl<T, Ts...> {
	static bool any_false(T &ptr, Ts &...ptrs) {
		return !ptr || all_true_impl<Ts...>::any_false(ptrs...);
	}
};

template<>
struct all_true_impl<> {
	static bool any_false() {
		return false;
	}
};

template<typename... Ts>
bool all_true(Ts &...ptrs) {
	return !all_true_impl<Ts...>::any_false(ptrs...);
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_VARIADIC_H_
