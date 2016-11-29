#include <frp/util/list.h>
#include <gtest/gtest.h>

TEST(single_linked_list, test1) {
	frp::util::single_list_type<int> list;
	auto it1(list.insert(3));
	auto it2(list.insert(2));
	auto it3(list.insert(1));
	ASSERT_TRUE(list.erase(it2));
	list.for_each([i = 0](auto value) mutable {
		ASSERT_EQ(value, 1 + (i++ * 2));
	});
}