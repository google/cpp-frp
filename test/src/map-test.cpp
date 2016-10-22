#include <frp/push/map.h>
#include <frp/push/transform.h>
#include <future>
#include <gtest/gtest.h>
#include <string>
#include <vector>

TEST(map, test1) {
	auto map(frp::push::map([](auto i) { return std::to_string(i); },
		frp::push::transform([]() { return std::array<int, 3>{ 1, 2, 3 }; })));

	std::promise<std::vector<std::string>> promise;
	auto future = promise.get_future();
	auto result(frp::push::transform([&](auto values) { promise.set_value(values); },
		std::ref(map)));
	auto values(future.get());
	ASSERT_NE(std::find(values.begin(), values.end(), "1"), values.end());
	ASSERT_NE(std::find(values.begin(), values.end(), "2"), values.end());
	ASSERT_NE(std::find(values.begin(), values.end(), "3"), values.end());
}