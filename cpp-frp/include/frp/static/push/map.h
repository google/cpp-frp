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
#ifndef _FRP_STATIC_PUSH_MAP_H_
#define _FRP_STATIC_PUSH_MAP_H_

#include <frp/internal/namespace_alias.h>
#include <frp/static/push/repository.h>
#include <frp/util/collector.h>
#include <frp/vector_view.h>
#include <vector>

namespace frp {
namespace stat {
namespace push {

template<std::size_t I, typename Comparator, typename Function, typename... Dependencies>
auto map(Function &&function, Dependencies... dependencies) {
	static_assert(I < sizeof...(Dependencies),
		"expanded index must be in the range of [0, arity) where arity = number of dependencies.");
	static_assert(util::all_true_type<typename util::is_not_void<
		typename util::unwrap_container_t<Dependencies>::value_type>::type...>::value,
		"Dependencies can not be void type.");

	typedef util::map_return_t<I, Function, Dependencies...> value_type;
	static_assert(!std::is_void<value_type>::value, "T must not be void type.");
	static_assert(std::is_move_constructible<value_type>::value, "T must be move constructible");

	typedef vector_view_type<value_type, Comparator> collector_view_type;
	typedef util::commit_storage_type<collector_view_type, sizeof...(Dependencies)>
		commit_storage_type;
	typedef std::array<util::revision_type, sizeof...(Dependencies)> revisions_type;

	return details::make_repository<collector_view_type, commit_storage_type,
			std::equal_to<collector_view_type>>([
				function = internal::get_function(util::unwrap_reference(std::forward<Function>(function))),
				executor = internal::get_executor(util::unwrap_reference(std::forward<Function>(function)))](
				auto &&callback, const auto &previous, const auto &dependencies) {
		typedef util::fixed_size_collector_type<value_type, Comparator> collector_type;
		typedef vector_view_type<value_type, Comparator> collector_view_type;

		auto values(util::invoke([&](const auto&... dependency) {
			return std::make_tuple(internal::get_storage(util::unwrap_container(dependency))...);
		}, *dependencies));
		auto revisions(util::invoke([&](const auto&... storage) {
			return revisions_type{ storage->revision... };
		}, values));
		auto &collection(std::get<I>(values)->value);
		if (collection.empty()) {
			callback(std::make_shared<commit_storage_type>(
				collector_view_type(collector_type(0)), util::default_revision,
				revisions));
		} else {
			auto collector(std::make_shared<collector_type>(collection.size()));
			std::size_t counter(0);
			for (const auto &value : collection) {
				std::size_t index(counter++);
				executor([function, collector, index, &value, callback, values, revisions]() {
					auto complete(collector->construct(index,
						util::indexed_invoke_with_replacement<I>(std::move(function),
							std::cref(value),
							util::invoke([&](const auto&... values) {
								return std::tie(values->value...);
							}, values))));
					if (complete) {
						callback(std::make_shared<commit_storage_type>(
							collector_view_type(std::move(*collector)),
							util::default_revision, revisions));
					}
				});
			}
		}
	}, std::forward<Dependencies>(dependencies)...);
}

template<std::size_t I, typename Function, typename... Dependencies>
auto map(Function &&function, Dependencies... dependencies) {
	typedef util::map_return_t<I, Function, Dependencies...> value_type;
	static_assert(util::is_equality_comparable<value_type>::value,
		"T must implement equality comparator");
	return map<I, std::equal_to<value_type>>(std::forward<Function>(function),
		std::forward<Dependencies>(dependencies)...);
}

template<typename Comparator, typename Function, typename... Dependencies>
auto map(Function &&function, Dependencies... dependencies) {
	return map<0, Comparator>(std::forward<Function>(function),
		std::forward<Dependencies>(dependencies)...);
}

template<typename Function, typename... Dependencies>
auto map(Function &&function, Dependencies... dependencies) {
	return map<0>(std::forward<Function>(function), std::forward<Dependencies>(dependencies)...);
}

} // namespace push
} // namespace stat
} // namespace frp

#endif // _FRP_STATIC_PUSH_MAP_H_
