#include <array_util.h>
#include <frp/push/filter.h>
#include <frp/push/sink.h>
#include <frp/push/transform.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(filter, test1) {
	auto map(frp::push::filter([](auto i) { return i > 2; },
		frp::push::transform([]() { return make_array(1, 2, 3, 4); })));
	auto sink(frp::push::sink(std::ref(map)));
	auto values(*sink);
	ASSERT_EQ(values.size(), 2);
	ASSERT_EQ(values[0], 3);
	ASSERT_EQ(values[1], 4);
}
