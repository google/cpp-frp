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
	auto sink(frp::push::sink(frp::push::transform(
		[&](const auto &values) { return values; }, std::ref(map))));
	auto values(*sink);
	ASSERT_EQ(values[0], "1");
	ASSERT_EQ(values[1], "2");
	ASSERT_EQ(values[2], "3");
}

struct even_comparator {

	auto operator()(int lhs, int rhs) {
		return lhs % 2 == rhs % 2;
	}
};

TEST(map, custom_comparator) {
	auto source(frp::push::source(std::vector<int>{ 1, 3, 5 }));
	auto map(frp::push::map<even_comparator>([](auto c) { return c; },
		std::ref(source)));
	auto sink(frp::push::sink(std::ref(map)));
	auto value1(*sink);
	ASSERT_EQ(value1[0], 1);
	ASSERT_EQ(value1[1], 3);
	ASSERT_EQ(value1[2], 5);
	source = { 5, 7, 9 };
	auto value2(*sink);
	ASSERT_EQ(value2[0], 1);
	ASSERT_EQ(value2[1], 3);
	ASSERT_EQ(value2[2], 5);
	source = { 1, 2, 3 };
	auto value3(*sink);
	ASSERT_EQ(value3[0], 1);
	ASSERT_EQ(value3[1], 2);
	ASSERT_EQ(value3[2], 3);
}