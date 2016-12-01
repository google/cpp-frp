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
	static auto make() {
		return std::array<T, 0>();
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

template<typename T>
auto make_array() {
	return make_array_type<T>::make();
}

#endif // _ARRAY_UTIL_H_
