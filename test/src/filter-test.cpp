#include <frp/push/filter.h>
#include <frp/push/transform.h>
#include <future>
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(filter, test1) {
	auto map(frp::push::filter([](auto i) { return i > 2; },
		frp::push::transform([]() { return std::array<int, 4>{ 1, 2, 3, 4 }; })));

	std::promise<std::vector<int>> promise;
	auto future = promise.get_future();
	auto result(frp::push::transform([&](const auto &values) { promise.set_value(values); },
		std::ref(map)));
	auto values(future.get());
	ASSERT_EQ(values.size(), 2);
	ASSERT_NE(std::find(values.begin(), values.end(), 3), values.end());
	ASSERT_NE(std::find(values.begin(), values.end(), 4), values.end());
}
