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
#include <frp/static/push/sink.h>
#include <frp/static/push/source.h>
#include <gtest/gtest.h>
#include <test_types.h>

TEST(source, immediate_value) {
	auto source(frp::stat::push::source(5));
	auto sink(frp::stat::push::sink(std::ref(source)));
	ASSERT_TRUE(*source);
	ASSERT_TRUE(*sink);
	ASSERT_EQ(**sink, 5);
}

TEST(source, set_value) {
	auto source(frp::stat::push::source<int>());
	ASSERT_FALSE(*source);
	auto sink(frp::stat::push::sink(std::ref(source)));
	source = 5;
	ASSERT_TRUE(*source);
	ASSERT_TRUE(*sink);
	ASSERT_EQ(**sink, 5);
}

TEST(source, undefined_access) {
	auto source(frp::stat::push::source<int>());
	ASSERT_FALSE(*source);
	ASSERT_THROW(**source, std::domain_error);
	auto sink(frp::stat::push::sink(std::ref(source)));
	ASSERT_FALSE(*sink);
	ASSERT_THROW(**sink, std::domain_error);
}

TEST(source, custom_comparator) {
	auto source(frp::stat::push::source<odd_comparator_type>(2));
	auto sink(frp::stat::push::sink(std::ref(source)));
	ASSERT_EQ(**sink, 2);
	source = 4;
	ASSERT_EQ(**sink, 2);
	source = 5;
	ASSERT_EQ(**sink, 5);
}

TEST(sink, movable_type) {
	auto source(frp::stat::push::source<movable_odd_comparator_type>(movable_type(2)));
	auto sink(frp::stat::push::sink(std::ref(source)));
	auto source_reference(*source);
	auto sink_reference(*sink);
	ASSERT_EQ(source_reference->value, 2);
	ASSERT_EQ(sink_reference->value, 2);
}
