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
#include <frp/util/list.h>
#include <gtest/gtest.h>

TEST(single_linked_list, test1) {
	frp::util::single_list_type<int> list;
	auto it1(list.insert(3));
	auto it2(list.insert(2));
	auto it3(list.insert(1));
	ASSERT_TRUE(list.erase(it2));
	list.for_each([i = 0](auto value) mutable {
		ASSERT_EQ(value, 1 + (i++ * 2));
	});
}
