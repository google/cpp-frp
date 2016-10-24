#include <frp/push/filter.h>
#include <frp/push/map.h>
#include <frp/push/sink.h>
#include <frp/push/source.h>
#include <frp/push/transform.h>
#include <gtest/gtest.h>

using namespace frp::push;

TEST(example, example1) {
	auto base = source(5);
	auto exponent = source(2);

	auto squared = transform(
		[](auto base, auto exponent) { return pow(base, exponent); },
		std::ref(base), std::ref(exponent));

	auto random_sequence = transform([](auto i) {
		auto v = std::vector<int>(std::size_t(i));
		std::generate(v.begin(), v.end(), std::rand);
		return v;
	}, std::ref(squared));

	auto filtered = filter([](auto i) { return i % 2; }, std::ref(random_sequence));

	auto strings = map([](auto i) { return std::to_string(i); }, std::ref(filtered));

	auto print = sink(std::ref(strings));

	// read the content of print
	for (auto value : *print) {
		std::cout << value << " ";
	}
	std::cout << std::endl;

	// Update base and exponent repositories with new values.
	// changes will propagate through the graph where necessary.
	base = 6;
	exponent = 3;
}