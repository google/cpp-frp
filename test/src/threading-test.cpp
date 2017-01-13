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
#include <frp/static/push/transform.h>
#include <future>
#include <gtest/gtest.h>
#include <thread_pool.h>

template<typename T>
struct require_incrementing_type {

	T operator()(T value) const {
		assert(value > this->value);
		this->value = value;
		return value;
	}

	mutable T value;
};

template<typename... T>
struct require_incrementing_types {

	auto operator()(T... value) const {
		auto new_value(std::make_tuple(value...));
		assert(new_value > this->value);
		this->value = new_value;
		return new_value;
	}

	mutable std::tuple<T...> value;
};

TEST(transform, single_thread_diamond) {
	auto source(fsp::source(0));
	thread_pool pool(1);
	auto top(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_type<int>{-1}), std::ref(source)));
	auto left(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_type<int>{-1}), std::ref(top)));
	auto right(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_type<int>{ -1 }), std::ref(top)));
	auto bottom(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_types<int, int>{ std::make_tuple(-1, -1) }), std::ref(left),
		std::ref(right)));
	auto tail(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_type<std::tuple<int, int>>{ std::make_tuple(-1, -1) }), std::ref(bottom)));

	for (int i = 0; i < 1000; ++i) {
		source = i;
		std::this_thread::yield();
	}
	pool.wait_idle();
}
