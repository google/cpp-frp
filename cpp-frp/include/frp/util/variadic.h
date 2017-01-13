/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
	static bool any_false(const T &ptr, const Ts&... ptrs) {
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
bool all_true(const Ts &...ptrs) {
	return !details::all_true_type<Ts...>::any_false(ptrs...);
}

namespace details {

template<std::size_t E, std::size_t N, std::size_t I>
struct tuple_le_except_index_impl_type {

	template<typename T1, typename T2>
	static constexpr bool le(T1 value1, T2 value2) {
		return std::get<I>(value1) <= std::get<I>(value2)
			? tuple_le_except_index_impl_type<E, N, I + 1>::le(
				std::forward<T1>(value1), std::forward<T2>(value2)) : false;
	}
};

template<std::size_t N, std::size_t I>
struct tuple_le_except_index_impl_type<I, N, I> {

	template<typename T1, typename T2>
	static constexpr bool le(T1 value1, T2 value2) {
		return tuple_le_except_index_impl_type<I, N, I + 1>::le(
			std::forward<T1>(value1), std::forward<T2>(value2));
	}
};

template<std::size_t E, std::size_t N>
struct tuple_le_except_index_impl_type<E, N, N> {
	template<typename T1, typename T2>
	static constexpr bool le(T1 value1, T2 value2) {
		return true;
	}
};

} // namespace details

template<std::size_t I, typename T1, typename T2>
constexpr bool tuple_le_except_index(T1 value1, T2 value2) {
	static_assert(std::tuple_size<T1>::value == std::tuple_size<T2>::value,
		"tuples must be of equal size");
	return details::tuple_le_except_index_impl_type<I, std::tuple_size<T1>::value, 0>
		::le(std::forward<T1>(value1), std::forward<T2>(value2));
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_VARIADIC_H_
