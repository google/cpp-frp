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
#ifndef _FRP_UTIL_OBSERVE_ALL_H_
#define _FRP_UTIL_OBSERVE_ALL_H_

#include <array>

namespace frp {
namespace util {

template<typename F>
struct observe_all_type {

	template<typename... Observables>
	auto operator() (Observables&... observables) const {
		typedef std::array<observable_type::reference_type, sizeof...(Observables)> array_type;
		return array_type{ add_callback(util::unwrap_container(observables), function)... };
	}

	F function;
};

template<typename F>
static auto observe_all(F &&function) {
	return observe_all_type<F>{ std::forward<F>(function) };
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_OBSERVE_ALL_H_
