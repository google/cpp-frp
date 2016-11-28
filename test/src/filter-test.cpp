#include <array_util.h>
#include <frp/static/push/filter.h>
#include <frp/static/push/sink.h>
#include <frp/static/push/source.h>
#include <frp/static/push/transform.h>
#include <gtest/gtest.h>
#include <string>
#include <test_types.h>
#include <vector>

TEST(filter, test1) {
	auto filter(frp::stat::push::filter([](auto i) { return i > 2; },
		frp::stat::push::transform([]() { return make_array(1, 2, 3, 4); })));
	auto sink(frp::stat::push::sink(std::ref(filter)));
	auto reference(*sink);
	auto values(*reference);
	ASSERT_EQ(values.size(), 2);
	ASSERT_EQ(values[0], 3);
	ASSERT_EQ(values[1], 4);
}

TEST(filter, custom_comparator) {
	auto source(frp::stat::push::source(make_array(2, 4, 6, 8)));
	auto filter(frp::stat::push::filter<odd_comparator_type>([](auto i) { return i > 2; },
		std::ref(source)));
	auto sink(frp::stat::push::sink(std::ref(filter)));
	auto reference1(*sink);
	auto values1(*reference1);
	ASSERT_EQ(values1.size(), 3);
	ASSERT_EQ(values1[0], 4);
	ASSERT_EQ(values1[1], 6);
	ASSERT_EQ(values1[2], 8);
	source = make_array(2, 12, 14, 16);
	auto reference2(*sink);
	auto values2(*reference2);
	ASSERT_EQ(values2.size(), 3);
	ASSERT_EQ(values2[0], 4);
	ASSERT_EQ(values2[1], 6);
	ASSERT_EQ(values2[2], 8);
	source = make_array(2, 4, 6, 7);
	auto reference3(*sink);
	auto values3(*reference3);
	ASSERT_EQ(values3.size(), 3);
	ASSERT_EQ(values3[0], 4);
	ASSERT_EQ(values3[1], 6);
	ASSERT_EQ(values3[2], 7);
}
