#include <frp/static/push/sink.h>
#include <frp/static/push/source.h>
#include <gtest/gtest.h>
#include <test_types.h>

TEST(source, immediate_value) {
	auto source(frp::stat::push::source(5));
	auto sink(frp::stat::push::sink(std::ref(source)));
	ASSERT_TRUE(*source);
	ASSERT_TRUE(*sink);
	ASSERT_EQ(**sink, 5);
}

TEST(source, set_value) {
	auto source(frp::stat::push::source<int>());
	ASSERT_FALSE(*source);
	auto sink(frp::stat::push::sink(std::ref(source)));
	source = 5;
	ASSERT_TRUE(*source);
	ASSERT_TRUE(*sink);
	ASSERT_EQ(**sink, 5);
}

TEST(source, undefined_access) {
	auto source(frp::stat::push::source<int>());
	ASSERT_FALSE(*source);
	ASSERT_THROW(**source, std::domain_error);
	auto sink(frp::stat::push::sink(std::ref(source)));
	ASSERT_FALSE(*sink);
	ASSERT_THROW(**sink, std::domain_error);
}

TEST(source, custom_comparator) {
	auto source(frp::stat::push::source<odd_comparator_type>(2));
	auto sink(frp::stat::push::sink(std::ref(source)));
	ASSERT_EQ(**sink, 2);
	source = 4;
	ASSERT_EQ(**sink, 2);
	source = 5;
	ASSERT_EQ(**sink, 5);
}

TEST(sink, movable_type) {
	auto source(frp::stat::push::source<movable_odd_comparator_type>(movable_type(2)));
	auto sink(frp::stat::push::sink(std::ref(source)));
	auto source_reference(*source);
	auto sink_reference(*sink);
	ASSERT_EQ(source_reference->value, 2);
	ASSERT_EQ(sink_reference->value, 2);
}