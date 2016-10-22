#ifndef _ARRAY_UTIL_H_
#define _ARRAY_UTIL_H_

#include <array>

template<typename... Ts>
struct make_array_type;

template<typename T>
struct make_array_type<T> {
	static auto make(T &&value) {
		return std::array<T, 1>{ std::forward<T>(value) };
	}
};

template<typename T, typename... Ts>
struct make_array_type<T, Ts...> {
	static auto make(T &&value, Ts &&... values) {
		return std::array<T, 1 + sizeof...(Ts)>{
			std::forward<T>(value), std::forward<Ts>(values)... };
	}
};

template<typename... Ts>
auto make_array(Ts &&... values) {
	return make_array_type<Ts...>::make(std::forward<Ts>(values)...);
}

#endif // _ARRAY_UTIL_H_
