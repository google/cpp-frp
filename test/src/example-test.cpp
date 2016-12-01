/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cmath>
#include <frp/static/push/filter.h>
#include <frp/static/push/map.h>
#include <frp/static/push/sink.h>
#include <frp/static/push/source.h>
#include <frp/static/push/transform.h>
#include <gtest/gtest.h>

using namespace frp::stat::push;

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
	auto values(*print);
	for (auto value : *values) {
		std::cout << value << " ";
	}
	std::cout << std::endl;

	// Update base and exponent repositories with new values.
	// changes will propagate through the graph where necessary.
	base = 6;
	exponent = 3;
}
