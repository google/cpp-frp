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
#include <frp/static/push/map.h>
#include <frp/static/push/sink.h>
#include <frp/static/push/source.h>
#include <frp/static/push/transform.h>
#include <gtest/gtest.h>
#include <numeric>
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
		this->value = value;
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
		require_incrementing_types<int, int>{ {-1, -1} }), std::ref(left), std::ref(right)));
	auto tail(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_type<std::tuple<int, int>>{ { -1, -1 } }), std::ref(bottom)));

	for (int i = 0; i < 1000; ++i) {
		source = i;
		std::this_thread::yield();
	}
	pool.wait_idle();
}

TEST(transform, multi_thread) {
	auto source(fsp::source(0));
	thread_pool pool(8);
	auto top(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_type<int>{-1}), std::ref(source)));
	auto middle(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_type<int>{-1}), std::ref(top)));
	auto bottom(fsp::transform(frp::execute_on(std::ref(pool),
		require_incrementing_type<int>{-1}), std::ref(middle)));

	for (int i = 0; i < 1000; ++i) {
		source = i;
		std::this_thread::yield();
	}
	pool.wait_idle();
}

TEST(map, multi_thread) {
	auto source(fsp::source<std::vector<int>>());
	thread_pool pool(8);
	auto mapped(fsp::map(frp::execute_on(std::ref(pool), [](auto source) {
		return source * 2;
	}), std::ref(source)));
	auto transformed(fsp::transform(frp::execute_on(thread_pool(4), [](const auto &source) {
		auto it(std::begin(source));
		auto first(*it);
		for (int i = 0; it != std::end(source); ++it, ++i) {
			assert(*it == first + 2 * i);
		}
	}), std::ref(mapped)));
	for (int i = 0; i < 1000; ++i) {
		std::vector<int> range(5);
		std::iota(std::begin(range), std::end(range), i);
		source = std::move(range);
		std::this_thread::yield();
	}
	pool.wait_idle();
}
