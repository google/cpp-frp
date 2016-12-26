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
#ifndef _FRP_UTIL_FUNCTION_H_
#define _FRP_UTIL_FUNCTION_H_

#include <frp/util/reference.h>
#include <tuple>
#include <utility>

namespace frp {
namespace util {
namespace details {

template<typename F, typename Tuple, std::size_t... I>
auto invoke(F &&f, Tuple &&tuple, std::index_sequence<I...>) {
	return f(std::get<I>(unwrap_reference(std::forward<Tuple>(tuple)))...);
}

} // namespace details

template<typename F, typename Tuple>
auto invoke(F &&f, Tuple &&tuple) {
	return details::invoke(std::forward<F>(f), std::forward<Tuple>(tuple),
		std::make_index_sequence<std::tuple_size<unwrap_reference_t<Tuple>>::value>{});
}

template<typename F, typename... Ds>
using transform_return_type = decltype(std::declval<const internal::get_function_t<F>>()(
	std::declval<const typename util::unwrap_container_t<Ds>::value_type &>()...));

template<typename F, typename... Ds>
using map_return_type = decltype(std::declval<const internal::get_function_t<F>>()(
	std::declval<const typename util::unwrap_container_t<Ds>::value_type::value_type &>()...));

namespace details {

template<std::size_t E, std::size_t N, std::size_t I, typename F, typename T, typename... Outputs>
struct unwrap_container_for_index
	: unwrap_container_for_index<E, N, I + 1, F, T, Outputs..., std::tuple_element_t<I, T>> {};

template<std::size_t N, std::size_t I, typename F, typename T, typename... Outputs>
struct unwrap_container_for_index<I, N, I, F, T, Outputs...>
	: unwrap_container_for_index<I, N, I + 1, F, T, Outputs..., typename std::tuple_element_t<I, T>::value_type> {};

template<std::size_t E, std::size_t N, typename F, typename T, typename... Outputs>
struct unwrap_container_for_index<E, N, N, F, T, Outputs...> {
	typedef decltype(std::declval<F>()(std::declval<const typename Outputs &>()...)) type;
};

} // namespace details

template<std::size_t I, typename F, typename... Ds>
struct indexed_map_return_type : details::unwrap_container_for_index<I, sizeof...(Ds), 0,
	const internal::get_function_t<F>,
	std::tuple<typename util::unwrap_container_t<Ds>::value_type...>> {};

template<std::size_t I, typename F, typename... Ds>
using indexed_map_return_t = typename indexed_map_return_type<I, F, Ds...>::type;

namespace details {

template<std::size_t E, std::size_t I>
struct replace_if_index_type {
	template<typename T, typename R>
	static auto &&unwrap(T &&value, R &&replacement) {
		return std::forward<T>(value);
	}
};

template<std::size_t I>
struct replace_if_index_type<I, I> {
	template<typename T, typename R>
	static auto &&unwrap(T &&value, R &&replacement) {
		return std::forward<R>(replacement);
	}
};

template<std::size_t E, typename F, typename T, typename Tuple, std::size_t... I>
auto indexed_invoke_with_replacement(F &&f, T &&replacement, Tuple &&tuple,
		std::index_sequence<I...>) {
	return f(replace_if_index_type<E, I>::unwrap(
		std::get<I>(unwrap_reference(std::forward<Tuple>(tuple))),
		unwrap_reference(std::forward<T>(replacement)))...);
}

} // namespace details

template<std::size_t E, typename F, typename T, typename Tuple>
auto indexed_invoke_with_replacement(F &&f, T &&replacement, Tuple &&tuple) {
	return details::indexed_invoke_with_replacement<E>(std::forward<F>(f),
		std::forward<T>(replacement), std::forward<Tuple>(tuple),
		std::make_index_sequence<std::tuple_size<unwrap_reference_t<Tuple>>::value>{});
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_FUNCTION_H_
