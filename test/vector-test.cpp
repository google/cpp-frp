#include <frp/util/vector.h>
#include <gtest/gtest.h>

TEST(vector, vector_from_array1) {
	std::vector<int> vector(frp::util::vector_from_array(std::array<int, 5>{ 1, 2, 3, 4, 5 }));
	ASSERT_EQ(vector[0], 1);
	ASSERT_EQ(vector[1], 2);
	ASSERT_EQ(vector[2], 3);
	ASSERT_EQ(vector[3], 4);
	ASSERT_EQ(vector[4], 5);
}

struct movable_type {
	movable_type() = default;
	movable_type(const movable_type &) = delete;
	movable_type(movable_type &&) = default;
	movable_type &operator=(const movable_type &) = delete;
	movable_type &operator=(movable_type &&) = delete;
};

TEST(vector, vector_from_array2) {
	std::vector<movable_type> vector(frp::util::vector_from_array(
		std::array<movable_type, 2>{ movable_type(), movable_type() }));
	ASSERT_EQ(vector.size(), 2);
}
