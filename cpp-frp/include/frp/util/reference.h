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
