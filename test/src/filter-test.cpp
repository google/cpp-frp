#include <array_util.h>
#include <frp/push/filter.h>
#include <frp/push/sink.h>
#include <frp/push/source.h>
#include <frp/push/transform.h>
#include <gtest/gtest.h>
#include <string>
#include <test_types.h>
#include <vector>

TEST(filter, test1) {
	auto filter(frp::push::filter([](auto i) { return i > 2; },
		frp::push::transform([]() { return make_array(1, 2, 3, 4); })));
	auto sink(frp::push::sink(std::ref(filter)));
	auto values(*sink);
	ASSERT_EQ(values.size(), 2);
	ASSERT_EQ(values[0], 3);
	ASSERT_EQ(values[1], 4);
}

TEST(filter, custom_comparator) {
	auto source(frp::push::source(make_array(2, 4, 6, 8)));
	auto filter(frp::push::filter<odd_comparator_type>([](auto i) { return i > 2; },
		std::ref(source)));
	auto sink(frp::push::sink(std::ref(filter)));
	auto values1(*sink);
	ASSERT_EQ(values1.size(), 3);
	ASSERT_EQ(values1[0], 4);
	ASSERT_EQ(values1[1], 6);
	ASSERT_EQ(values1[2], 8);
	source = make_array(2, 12, 14, 16);
	auto values2(*sink);
	ASSERT_EQ(values2.size(), 3);
	ASSERT_EQ(values2[0], 4);
	ASSERT_EQ(values2[1], 6);
	ASSERT_EQ(values2[2], 8);
	source = make_array(2, 4, 6, 7);
	auto values3(*sink);
	ASSERT_EQ(values3.size(), 3);
	ASSERT_EQ(values3[0], 4);
	ASSERT_EQ(values3[1], 6);
	ASSERT_EQ(values3[2], 7);
}
