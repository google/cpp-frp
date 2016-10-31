#include <frp/util/collector.h>
#include <gtest/gtest.h>

// TODO(gardell): Test the vector_view_type and accompanying classes.

struct movable_type {
	movable_type() = default;
	movable_type(const movable_type &) = delete;
	movable_type(movable_type &&) = default;
	movable_type &operator=(const movable_type &) = delete;
	movable_type &operator=(movable_type &&) = delete;
};
/*
TEST(vector, collector1) {
	frp::util::collector_type<int> collector(2);
	collector.construct(0, 1);
	collector.construct(1, 2);
	frp::util::collector_view_type<int> view(std::move(collector));
	ASSERT_EQ(view[0], 1);
	ASSERT_EQ(view[1], 2);
	ASSERT_EQ(*view.begin(), 1);
}

TEST(vector, collector2) {
	frp::util::collector_type<movable_type> collector(2);
	collector.construct(0, movable_type());
	collector.construct(1, movable_type());
	frp::util::collector_view_type<movable_type> view(std::move(collector));
}*/