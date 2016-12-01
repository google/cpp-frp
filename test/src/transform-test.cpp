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

TEST(transform, test_one_parent) {
	auto squared(frp::stat::push::transform([](int i) { return i * i; },
		frp::stat::push::transform([]() { return -1; })));
	auto sink(frp::stat::push::sink(std::ref(squared)));
	auto reference(*sink);
	ASSERT_EQ(*reference, 1);
}

TEST(transform, test_two_parents) {
	auto left(frp::stat::push::transform([]() { return short(-1); }));
	auto right(frp::stat::push::transform([]() { return 2; }));

	auto multiplier(frp::stat::push::transform([](auto i, auto j) { return i * j; }, std::ref(left),
		std::ref(right)));
	auto sink(frp::stat::push::sink(std::ref(multiplier)));
	auto reference(*sink);
	ASSERT_EQ(*reference, -2);
}

template<typename... Args> inline void pass(Args&&...) {}

TEST(transform, variadic_arguments) {
	auto result(fsp::sink(fsp::transform(
		[](auto argument, auto... arguments) { pass(argument += arguments...); return argument; },
		fsp::source(5), fsp::source(3), fsp::source(4), fsp::source(8))));
	ASSERT_EQ(**result, 5 + 3 + 4 + 8);
}

TEST(transform, test_two_children) {
	auto top(frp::stat::push::transform([]() { return 2; }));
	auto left(frp::stat::push::sink(
		frp::stat::push::transform([](auto i) { return -1 * i; }, std::ref(top))));
	auto right(frp::stat::push::sink(
		frp::stat::push::transform([](auto i) { return 2 * i; }, std::ref(top))));
	auto left_reference(*left);
	auto right_reference(*right);
	ASSERT_EQ(*left_reference, -2);
	ASSERT_EQ(*right_reference, 4);
}

TEST(transform, test_diamond) {
	auto top(frp::stat::push::transform([]() { return 2; }));
	auto bottom(frp::stat::push::transform([](auto i, auto j) { return i * j; },
		frp::stat::push::transform([](auto i) { return 1 + i; }, std::ref(top)),
		frp::stat::push::transform([](auto i) { return short(2 + i); }, std::ref(top))));
	auto sink(frp::stat::push::sink(std::ref(bottom)));
	auto reference(*sink);
	ASSERT_EQ(*reference, 12);
}

TEST(transform, void_transform) {
	auto source(frp::stat::push::source(5));
	auto transform(frp::stat::push::transform([](auto i) {},
		frp::stat::push::transform([](auto input) { return 3 * input; }, std::ref(source))));
	source = 6;
}

TEST(transform, shared_ptr) {
	auto source(std::make_shared<frp::stat::push::source_type<int>>(frp::stat::push::source(5)));

	auto merge(frp::stat::push::transform([](auto i, auto j) {},
		frp::stat::push::transform([](auto i) { return i; }, source),
		frp::stat::push::transform([](auto i) { return i; }, source)));
}

TEST(transform, unique_ptr) {
	auto sink(std::make_unique<frp::stat::push::source_type<int>>(frp::stat::push::source(5)));
}

struct A {
	A(int i) : i(i) {}
	A(const A &) = delete;
	A(A &&) = default;
	A &operator=(const A &) = delete;
	A &operator=(A &&) = default;
	auto operator==(const A &a) const {
		return i == a.i;
	}
	int i;
};

TEST(transform, movable) {
	auto source(frp::stat::push::source<A>());
	auto transform(frp::stat::push::transform([](auto &v) {}, std::ref(source)));
	source = A(5);
}

int foo(short a, char b) {
	return int(a) * b;
}

TEST(transform, function_ptr) {
	auto sink(frp::stat::push::sink(
		frp::stat::push::transform(&foo, frp::stat::push::source(short(13)),
			frp::stat::push::source(char(42)))));
	auto reference(*sink);
	ASSERT_EQ(*reference, 42 * 13);
}

TEST(transform, bind) {
	auto source(frp::stat::push::source(char(42)));
	auto transform(frp::stat::push::transform(std::bind(&foo, short(5), std::placeholders::_1),
		std::ref(source)));
	auto sink(frp::stat::push::sink(std::ref(transform)));
	auto reference(*sink);
	ASSERT_EQ(*reference, 42 * 5);
}

struct C {
	int a, b;

	auto operator==(const C &c) const {
		return a == c.a && b == c.b;
	}
};

struct C_comparator {
	auto operator()(const C &lhs, const C &rhs) const {
		return lhs.a == rhs.a;
	}
};

TEST(transform, custom_comparator) {
	auto source(frp::stat::push::source(C{ 0, 0, }));
	auto sink(frp::stat::push::sink(frp::stat::push::transform<C_comparator>(
		[](auto c) { return c; }, std::ref(source))));
	auto reference1(*sink);
	auto value1(*reference1);
	ASSERT_EQ(value1.a, 0);
	ASSERT_EQ(value1.b, 0);
	source = { 0, 1 };
	auto reference2(*sink);
	auto value2(*reference2);
	ASSERT_EQ(value2.a, 0);
	ASSERT_EQ(value2.b, 0);
	source = { 1, 1 };
	auto reference3(*sink);
	auto value3(*reference3);
	ASSERT_EQ(value3.a, 1);
	ASSERT_EQ(value3.b, 1);
}

TEST(transform, assignment) {
	frp::stat::push::repository_type<int> repository;
	repository = frp::stat::push::transform([]() { return 0; });
}

TEST(transform, thread_pool1) {
	thread_pool pool(4);

	frp::stat::push::transform(frp::execute_on(std::ref(pool), []() { return rand(); }));
	frp::stat::push::transform(frp::execute_on(std::ref(pool), []() { return rand(); }));
}

TEST(transform, thread_pool2) {
	thread_pool pool(4);

	std::atomic_int counter(0);
	auto source(frp::stat::push::source(0));
	frp::stat::push::transform(frp::execute_on(thread_pool(1),
		[&](auto source) {
			auto previous(counter.exchange(source));
			assert(previous < source);
		}),
		frp::stat::push::transform(
			frp::execute_on(std::ref(pool), [](auto source) {
				return source + 1;
				std::this_thread::yield();
			}),
			std::ref(source)));
	for (int i = 0; i < 10000; ++i) {
		source = i;
	}
}

TEST(transform, references) {
	auto f([](auto source) {
		return source * source;
	});
	auto source(frp::stat::push::transform([]() { return -2; }));
	frp::stat::push::transform(std::cref(f), std::cref(source));
}
