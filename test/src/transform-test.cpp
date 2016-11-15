#include <frp/push/sink.h>
#include <frp/push/source.h>
#include <frp/push/transform.h>
#include <future>
#include <gtest/gtest.h>
#include <thread_pool.h>

TEST(repo, test_one_parent) {
	auto squared(frp::push::transform([](int i) { return i * i; },
		frp::push::transform([]() { return -1; })));
	auto sink(frp::push::sink(std::ref(squared)));
	auto reference(*sink);
	ASSERT_EQ(*reference, 1);
}

TEST(repo, test_two_parents) {
	auto left(frp::push::transform([]() { return short(-1); }));
	auto right(frp::push::transform([]() { return 2; }));

	auto multiplier(frp::push::transform([](auto i, auto j) { return i * j; }, std::ref(left),
		std::ref(right)));
	auto sink(frp::push::sink(std::ref(multiplier)));
	auto reference(*sink);
	ASSERT_EQ(*reference, -2);
}

TEST(repo, test_two_children) {
	auto top(frp::push::transform([]() { return 2; }));
	auto left(frp::push::sink(frp::push::transform([](auto i) { return -1 * i; }, std::ref(top))));
	auto right(frp::push::sink(frp::push::transform([](auto i) { return 2 * i; }, std::ref(top))));
	auto left_reference(*left);
	auto right_reference(*right);
	ASSERT_EQ(*left_reference, -2);
	ASSERT_EQ(*right_reference, 4);
}

TEST(repo, test_diamond) {
	auto top(frp::push::transform([]() { return 2; }));
	auto bottom(frp::push::transform([](auto i, auto j) { return i * j; },
		frp::push::transform([](auto i) { return 1 + i; }, std::ref(top)),
		frp::push::transform([](auto i) { return short(2 + i); }, std::ref(top))));
	auto sink(frp::push::sink(std::ref(bottom)));
	auto reference(*sink);
	ASSERT_EQ(*reference, 12);
}

TEST(repo, void_repository) {
	frp::push::transform([](auto i) {}, frp::push::transform([]() { return -1; }));
}

TEST(repo, shared_ptr) {
	auto source(std::make_shared<frp::push::source_type<int>>(frp::push::source(5)));

	auto merge(frp::push::transform([](auto i, auto j) {},
		frp::push::transform([](auto i) { return i; }, source),
		frp::push::transform([](auto i) { return i; }, source)));
}

TEST(repo, unique_ptr) {
	auto sink(std::make_unique<frp::push::source_type<int>>(frp::push::source(5)));
}

TEST(repo, mutable_repository) {
	auto top(frp::push::source(5));
	auto left(frp::push::transform([](auto top) { return 3 * top; }, std::ref(top)));
	auto right(frp::push::transform([](auto top) { return 2 * top; }, std::ref(top)));
	auto sink(frp::push::sink(frp::push::transform(
		[](auto a, auto b) { return std::make_pair(a, b); }, std::ref(left), std::ref(right))));
	auto reference(*sink);
	auto value(*reference);
	ASSERT_EQ(value.first, 15);
	ASSERT_EQ(value.second, 10);
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
	auto source(frp::push::source<A>());
	auto transform(frp::push::transform([](auto &v) {}, std::ref(source)));
	source = A(5);
}

int foo(short a, char b) {
	return int(a) * b;
}

TEST(transform, function_ptr) {
	auto sink(frp::push::sink(
		frp::push::transform(&foo, frp::push::source(short(13)),
			frp::push::source(char(42)))));
	auto reference(*sink);
	ASSERT_EQ(*reference, 42 * 13);
}

TEST(transform, bind) {
	auto source(frp::push::source(char(42)));
	auto transform(frp::push::transform(std::bind(&foo, short(5), std::placeholders::_1),
		std::ref(source)));
	auto sink(frp::push::sink(std::ref(transform)));
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
	auto source(frp::push::source(C{ 0, 0, }));
	auto sink(frp::push::sink(frp::push::transform<C_comparator>(
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
	frp::push::repository_type<int> repository;
	repository = frp::push::transform([]() { return 0; });
}

TEST(transform, thread_pool1) {
	thread_pool pool(4);

	frp::push::transform(frp::execute_on(std::ref(pool), []() { return rand(); }));
	frp::push::transform(frp::execute_on(std::ref(pool), []() { return rand(); }));
}

TEST(transform, thread_pool2) {
	thread_pool pool(4);

	std::atomic_int counter(0);
	auto source(frp::push::source(0));
	frp::push::transform(frp::execute_on(thread_pool(1),
		[&](auto source) {
			auto previous(counter.exchange(source));
			assert(previous < source);
		}),
		frp::push::transform(
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
	auto source(frp::push::transform([]() { return -2; }));
	frp::push::transform(std::cref(f), std::cref(source));
}