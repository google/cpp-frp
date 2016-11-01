#include <array_util.h>
#include <frp/push/map_cache.h>
#include <frp/push/sink.h>
#include <frp/push/source.h>
#include <frp/push/transform.h>
#include <future>
#include <gtest/gtest.h>
#include <string>
#include <test_types.h>
#include <vector>

TEST(map_cache, test1) {
	auto source(frp::push::source(make_array(1, 2, 3, 4)));
	auto map(frp::push::map_cache([](auto i) { return std::to_string(i); }, std::ref(source)));
	auto sink(frp::push::sink(std::ref(map)));
	auto reference(*sink);
	auto values(*reference);
	ASSERT_EQ(values[0], "1");
	ASSERT_EQ(values[1], "2");
	ASSERT_EQ(values[2], "3");
	ASSERT_EQ(values[3], "4");
}

TEST(map_cache, test_caching) {
	auto source(frp::push::source(make_array(1, 2, 3, 4)));
	std::unordered_map<int, std::size_t> counter;
	auto map(frp::push::map_cache([&](auto i) {
			++counter[i];
			return std::to_string(i);
		},
		std::ref(source)));
	source = make_array(3, 4, 5, 6);
	auto sink(frp::push::sink(std::ref(map)));
	auto reference(*sink);
	auto values(*reference);
	ASSERT_EQ(values[0], "3");
	ASSERT_EQ(values[1], "4");
	ASSERT_EQ(values[2], "5");
	ASSERT_EQ(values[3], "6");
	ASSERT_EQ(counter[1], 1);
	ASSERT_EQ(counter[2], 1);
	ASSERT_EQ(counter[3], 1);
	ASSERT_EQ(counter[4], 1);
	ASSERT_EQ(counter[5], 1);
	ASSERT_EQ(counter[6], 1);
}

TEST(map_cache, custom_comparator) {
	auto source(frp::push::source(make_array(1, 3, 5)));
	auto map(frp::push::map_cache<odd_comparator_type, std::hash<int>>([](auto c) { return c; },
		std::ref(source)));
	auto sink(frp::push::sink(std::ref(map)));
	auto reference1(*sink);
	auto value1(*reference1);
	ASSERT_EQ(value1[0], 1);
	ASSERT_EQ(value1[1], 3);
	ASSERT_EQ(value1[2], 5);
	source = { 5, 7, 9 };
	auto reference2(*sink);
	auto value2(*reference2);
	ASSERT_EQ(value2[0], 1);
	ASSERT_EQ(value2[1], 3);
	ASSERT_EQ(value2[2], 5);
	source = { 1, 2, 3 };
	auto reference3(*sink);
	auto value3(*reference3);
	ASSERT_EQ(value3[0], 1);
	ASSERT_EQ(value3[1], 2);
	ASSERT_EQ(value3[2], 3);
}
