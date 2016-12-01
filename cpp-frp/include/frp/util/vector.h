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
#ifndef _FRP_UTIL_VECTOR_H_
#define _FRP_UTIL_VECTOR_H_

#include <array>
#include <atomic>
#include <memory>
#include <vector>

namespace frp {
namespace util {

template<typename T, std::size_t N>
auto vector_from_array(std::array<T, N> &&array) {
	return std::vector<T>(std::make_move_iterator(array.begin()),
		std::make_move_iterator(array.end()));
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_VECTOR_H_
