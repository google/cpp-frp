#include <frp/push/sink.h>
#include <frp/push/source.h>
#include <frp/push/transform.h>
#include <future>
#include <gtest/gtest.h>

TEST(repo, test_one_parent) {
	auto repository1(frp::push::transform([]() { return -1; }));
	std::promise<int> promise;
	auto future = promise.get_future();
	auto generator2([&](int i) {
		auto temp = i * i;
		promise.set_value(temp);
		return temp;
	});

	auto repository2(frp::push::transform(generator2, std::ref(repository1)));

	ASSERT_EQ(future.get(), 1);
}

TEST(repo, test_two_parents) {
	auto repository_a(frp::push::transform([]() { return short(-1); }));
	auto repository_b(frp::push::transform([]() { return 2; }));

	auto promise = std::make_shared<std::promise<int>>();
	auto future = promise->get_future();
	auto generator = [promise](auto i, auto j) {
		auto temp = i * j;
		promise->set_value(temp);
		return temp;
	};

	auto repository(frp::push::transform(generator, std::ref(repository_a),
		std::ref(repository_b)));
	ASSERT_EQ(future.get(), -2);
}

TEST(repo, test_two_children) {
	auto promise_a = std::make_shared<std::promise<int>>();
	auto future_a = promise_a->get_future();
	auto generator_a = [promise_a](auto i) {
		auto temp = -1 * i;
		promise_a->set_value(temp);
		return temp;
	};

	auto promise_b = std::make_shared<std::promise<int>>();
	auto future_b = promise_b->get_future();
	auto generator_b = [promise_b](auto i) {
		auto temp = 2 * i;
		promise_b->set_value(temp);
		return temp;
	};

	auto repository_top(frp::push::transform([]() { return 2; }));
	auto repository_a(frp::push::transform(generator_a, std::ref(repository_top)));
	auto repository_b(frp::push::transform(generator_b, std::ref(repository_top)));

	ASSERT_EQ(future_a.get(), -2);
	ASSERT_EQ(future_b.get(), 4);
}

TEST(repo, test_diamond) {
	auto repository_top(frp::push::transform([]() { return 2; }));
	auto repository_a(frp::push::transform([](auto i) { return 1 + i; },
		std::ref(repository_top)));
	auto repository_b(frp::push::transform([](auto i) { return short(2 + i); },
		std::ref(repository_top)));

	auto promise = std::make_shared<std::promise<int>>();
	auto future_bottom = promise->get_future();
	auto generator_bottom = [promise](auto i, auto j) {
		promise->set_value(i * j);
	};

	auto repository_bottom(frp::push::transform(generator_bottom,
		std::ref(repository_a), std::ref(repository_b)));
	ASSERT_EQ(future_bottom.get(), 12);
}

TEST(repo, void_repository) {
	frp::push::transform([](auto i) {}, frp::push::transform([]() { return -1; }));
}

TEST(repo, shared_ptr) {
	auto source(std::make_shared<frp::push::source_repository_type<int>>(frp::push::source(5)));

	auto merge(frp::push::transform([](auto i, auto j) {},
		frp::push::transform([](auto i) { return i; }, source),
		frp::push::transform([](auto i) { return i; }, source)));
}

TEST(repo, unique_ptr) {
	auto sink(std::make_unique<frp::push::source_repository_type<int>>(frp::push::source(5)));
}

TEST(repo, mutable_repository) {
	auto repository_top(frp::push::source(5));
	auto repository_a(frp::push::transform([](auto top) { return 3 * top; },
		std::ref(repository_top)));
	auto repository_b(frp::push::transform([](auto top) { return 2 * top; },
		std::ref(repository_top)));

	auto promise_a = std::make_shared<std::promise<int>>();
	auto future_a = promise_a->get_future();
	auto promise_b = std::make_shared<std::promise<int>>();
	auto future_b = promise_b->get_future();
	auto repository_bottom(frp::push::transform(
		[promise_a, promise_b](auto a, auto b) { promise_a->set_value(a); promise_b->set_value(b); },
		std::ref(repository_a), std::ref(repository_b)));

	ASSERT_EQ(future_a.get(), 15);
	ASSERT_EQ(future_b.get(), 10);
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
	auto source1(frp::push::source(short(13)));
	auto source2(frp::push::source(char(42)));
	auto result(frp::push::transform(&foo, std::ref(source1), std::ref(source2)));
	auto sink(frp::push::sink(std::ref(result)));
	ASSERT_EQ(*sink, 42 * 13);
}

TEST(transform, bind) {
	auto source(frp::push::source(char(42)));
	auto result(frp::push::transform(std::bind(&foo, short(5), std::placeholders::_1),
		std::ref(source)));
	auto sink(frp::push::sink(std::ref(result)));
	ASSERT_EQ(*sink, 42 * 5);
}

struct C {
	int a, b;

	auto operator==(const C &c) const {
		return a == c.a && b == c.b;
	}
};

struct C_comparator {
	auto operator()(const C &lhs, const C &rhs) {
		return lhs.a == rhs.a;
	}
};

TEST(transform, custom_comparator) {
	auto source(frp::push::source(C{ 0, 0, }));
	auto transform(frp::push::transform<C_comparator>([](auto c) { return c; }, std::ref(source)));
	auto sink(frp::push::sink(std::ref(transform)));
	auto value1(*sink);
	ASSERT_EQ(value1.a, 0);
	ASSERT_EQ(value1.b, 0);
	source = { 0, 1 };
	auto value2(*sink);
	ASSERT_EQ(value2.a, 0);
	ASSERT_EQ(value2.b, 0);
	source = { 1, 1 };
	auto value3(*sink);
	ASSERT_EQ(value3.a, 1);
	ASSERT_EQ(value3.b, 1);
}