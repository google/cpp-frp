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
#include <frp/util/vector.h>
#include <gtest/gtest.h>

TEST(vector, vector_from_array1) {
	std::vector<int> vector(frp::util::vector_from_array(std::array<int, 5>{ 1, 2, 3, 4, 5 }));
	ASSERT_EQ(vector[0], 1);
	ASSERT_EQ(vector[1], 2);
	ASSERT_EQ(vector[2], 3);
	ASSERT_EQ(vector[3], 4);
	ASSERT_EQ(vector[4], 5);
}

struct movable_type {
	movable_type() = default;
	movable_type(const movable_type &) = delete;
	movable_type(movable_type &&) = default;
	movable_type &operator=(const movable_type &) = delete;
	movable_type &operator=(movable_type &&) = delete;
};

TEST(vector, vector_from_array2) {
	std::vector<movable_type> vector(frp::util::vector_from_array(
		std::array<movable_type, 2>{ movable_type(), movable_type() }));
	ASSERT_EQ(vector.size(), 2);
}
