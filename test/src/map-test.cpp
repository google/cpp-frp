#include <array_util.h>
#include <frp/push/map.h>
#include <frp/push/sink.h>
#include <frp/push/source.h>
#include <frp/push/transform.h>
#include <future>
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(map, test1) {
	auto map(frp::push::map([](auto i) { return std::to_string(i); },
		frp::push::transform([]() { return make_array(1, 2, 3, 4); })));

	std::promise<std::vector<std::string>> promise;
	auto future = promise.get_future();
	auto result(frp::push::transform([&](auto values) { promise.set_value(values); },
		std::ref(map)));
	auto values(future.get());
	ASSERT_NE(std::find(values.begin(), values.end(), "1"), values.end());
	ASSERT_NE(std::find(values.begin(), values.end(), "2"), values.end());
	ASSERT_NE(std::find(values.begin(), values.end(), "3"), values.end());
}

struct vector_comparator {
	auto operator()(const std::vector<int> &lhs, const std::vector<int> &rhs) {
		return std::find(lhs.begin(), lhs.end(), 1) != lhs.end()
			&& std::find(rhs.begin(), rhs.end(), 1) != rhs.end();
	}
};

TEST(map, custom_comparator) {
	auto source(frp::push::source(std::vector<int>{ 1, 2, 3 }));
	auto map(frp::push::map<std::vector<int>, vector_comparator>(
		[](auto c) { return c; }, std::ref(source)));
	auto sink(frp::push::sink(std::ref(map)));
	auto value1(*sink);
	ASSERT_NE(std::find(value1.begin(), value1.end(), 1), value1.end());
	ASSERT_NE(std::find(value1.begin(), value1.end(), 2), value1.end());
	ASSERT_NE(std::find(value1.begin(), value1.end(), 3), value1.end());
	source = { 1, 4, 5 };
	auto value2(*sink);
	ASSERT_NE(std::find(value2.begin(), value2.end(), 1), value2.end());
	ASSERT_NE(std::find(value2.begin(), value2.end(), 2), value2.end());
	ASSERT_NE(std::find(value2.begin(), value2.end(), 3), value2.end());
	source = { 2, 3, 4 };
	auto value3(*sink);
	ASSERT_NE(std::find(value3.begin(), value3.end(), 2), value3.end());
	ASSERT_NE(std::find(value3.begin(), value3.end(), 3), value3.end());
	ASSERT_NE(std::find(value3.begin(), value3.end(), 4), value3.end());
}