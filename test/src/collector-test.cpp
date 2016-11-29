#include <array>
#include <frp/util/collector.h>
#include <frp/vector_view.h>
#include <gtest/gtest.h>

struct movable_type {
	movable_type(int value = 0) : value(value) {}
	movable_type(const movable_type &) = delete;
	movable_type(movable_type &&) = default;
	movable_type &operator=(const movable_type &) = delete;
	movable_type &operator=(movable_type &&) = delete;

	bool operator==(const movable_type &movable) const {
		return value == movable.value;
	}

	int value;
};

TEST(fixed_size_collector, test1) {
	frp::util::fixed_size_collector_type<movable_type> collector(2);
	ASSERT_EQ(collector.size(), 0);
	ASSERT_FALSE(collector.construct(1, movable_type(1)));
	ASSERT_EQ(collector.size(), 1);
	ASSERT_TRUE(collector.construct(0, movable_type(0)));
	ASSERT_EQ(collector.size(), 2);

	frp::vector_view_type<movable_type> vector_view(std::move(collector));
	ASSERT_FALSE(vector_view.empty());
	ASSERT_EQ(vector_view.size(), 2);
	ASSERT_EQ(vector_view[0], 0);
	ASSERT_EQ(vector_view[1], 1);
	ASSERT_EQ(vector_view.begin(), vector_view.end() - vector_view.size());
	const std::array<movable_type, 2> movables{ movable_type(0), movable_type(1) };
	ASSERT_TRUE(std::equal(vector_view.begin(), vector_view.end(), movables.begin(),
		movables.end()));
	ASSERT_EQ(*vector_view.rbegin(), 1);
	ASSERT_EQ(*++vector_view.rbegin(), 0);
	ASSERT_EQ(*--vector_view.rend(), 0);
	ASSERT_EQ(vector_view.rend() - vector_view.rbegin(), 2);
	ASSERT_NE(vector_view.rbegin(), vector_view.rend());
	ASSERT_EQ(vector_view.size(), 2);
	ASSERT_EQ(vector_view.rbegin() + 2, vector_view.rend());
	ASSERT_TRUE(std::equal(vector_view.rbegin(), vector_view.rend(), movables.rbegin(),
		movables.rend()));
}

TEST(vector_view, empty_collector) {
	frp::vector_view_type<movable_type> vector_view(
		frp::util::fixed_size_collector_type<movable_type>(0));
	ASSERT_TRUE(vector_view.empty());
	ASSERT_EQ(vector_view.begin(), vector_view.end());
}

TEST(append_collector, test1) {
	frp::util::append_collector_type<movable_type> collector(2);
	ASSERT_EQ(collector.size(), 0);
	ASSERT_FALSE(collector.construct(movable_type(0)));
	ASSERT_EQ(collector.size(), 1);
	ASSERT_TRUE(collector.construct(movable_type(1)));
	ASSERT_EQ(collector.size(), 2);

	frp::vector_view_type<movable_type> vector_view(std::move(collector));
	ASSERT_FALSE(vector_view.empty());
	ASSERT_EQ(vector_view.size(), 2);
	ASSERT_EQ(vector_view[0], 0);
	ASSERT_EQ(vector_view[1], 1);
	ASSERT_EQ(vector_view.begin(), vector_view.end() - vector_view.size());
	const std::array<movable_type, 2> movables{ movable_type(0), movable_type(1) };
	ASSERT_TRUE(std::equal(vector_view.begin(), vector_view.end(), movables.begin(),
		movables.end()));
	ASSERT_EQ(*vector_view.rbegin(), 1);
	ASSERT_EQ(*++vector_view.rbegin(), 0);
	ASSERT_EQ(*--vector_view.rend(), 0);
	ASSERT_EQ(vector_view.rend() - vector_view.rbegin(), 2);
	ASSERT_NE(vector_view.rbegin(), vector_view.rend());
	ASSERT_EQ(vector_view.size(), 2);
	ASSERT_EQ(vector_view.rbegin() + 2, vector_view.rend());
	ASSERT_TRUE(std::equal(vector_view.rbegin(), vector_view.rend(), movables.rbegin(),
		movables.rend()));
}

TEST(append_collector, test2) {
	frp::util::append_collector_type<movable_type> collector(3);
	ASSERT_EQ(collector.size(), 0);
	ASSERT_FALSE(collector.construct(movable_type(0)));
	ASSERT_EQ(collector.size(), 1);
	ASSERT_FALSE(collector.construct(movable_type(1)));
	ASSERT_EQ(collector.size(), 2);
	ASSERT_TRUE(collector.skip());
	ASSERT_EQ(collector.size(), 2);

	frp::vector_view_type<movable_type> vector_view(std::move(collector));
	ASSERT_FALSE(vector_view.empty());
	ASSERT_EQ(vector_view.size(), 2);
	ASSERT_EQ(vector_view[0], 0);
	ASSERT_EQ(vector_view[1], 1);
	ASSERT_EQ(vector_view.begin(), vector_view.end() - vector_view.size());
	const std::array<movable_type, 2> movables{ movable_type(0), movable_type(1) };
	ASSERT_TRUE(std::equal(vector_view.begin(), vector_view.end(), movables.begin(),
		movables.end()));
	ASSERT_EQ(*vector_view.rbegin(), 1);
	ASSERT_EQ(*++vector_view.rbegin(), 0);
	ASSERT_EQ(*--vector_view.rend(), 0);
	ASSERT_EQ(vector_view.rend() - vector_view.rbegin(), 2);
	ASSERT_NE(vector_view.rbegin(), vector_view.rend());
	ASSERT_EQ(vector_view.size(), 2);
	ASSERT_EQ(vector_view.rbegin() + 2, vector_view.rend());
	ASSERT_TRUE(std::equal(vector_view.rbegin(), vector_view.rend(), movables.rbegin(),
		movables.rend()));
}
