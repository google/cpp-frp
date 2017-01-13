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
#ifndef _FRP_STATIC_PUSH_MAP_CACHE_H_
#define _FRP_STATIC_PUSH_MAP_CACHE_H_

#include <frp/internal/namespace_alias.h>
#include <frp/static/push/repository.h>
#include <frp/util/collector.h>
#include <frp/vector_view.h>
#include <unordered_map>
#include <vector>

namespace frp {
namespace stat {
namespace push {
namespace details {

template<typename K, typename V, typename Container, typename Hash, std::size_t DependenciesN>
struct map_cache_commit_storage_type : util::commit_storage_type<Container, DependenciesN> {

	typedef std::unordered_map<K, std::reference_wrapper<const V>, Hash> cache_type;
	typedef frp::util::commit_storage_type<Container, DependenciesN> parent_type;
	typedef typename parent_type::revisions_type revisions_type;
	cache_type cache;

	map_cache_commit_storage_type(Container &&value, util::revision_type revision,
		const revisions_type &revisions) : util::commit_storage_type<Container, DependenciesN>(
			std::forward<Container>(value), revision, revisions) {}
};

} // namespace details

template<std::size_t I, typename Comparator, typename Hash, typename Function,
	typename... Dependencies>
auto map_cache(Function &&function, Dependencies... dependencies) {
	static_assert(I < sizeof...(Dependencies),
		"expanded index must be in the range of [0, arity) where arity = number of dependencies.");
	typedef typename util::unwrap_reference_t<std::tuple_element_t<I, std::tuple<Dependencies...>>>
		::value_type argument_container_type;
	typedef typename argument_container_type::value_type argument_type;
	typedef util::map_return_t<I, Function, Dependencies...> value_type;
	static_assert(!std::is_void<argument_type>::value, "Dependency must not be void type.");
	static_assert(std::is_copy_constructible<argument_type>::value,
		"Dependency type must be copy constructible");
	static_assert(std::is_move_constructible<value_type>::value,
		"T must be move constructible");
	static_assert(!std::is_void<value_type>::value, "T must not be void type.");

	typedef vector_view_type<value_type, Comparator> collector_view_type;
	typedef details::map_cache_commit_storage_type<argument_type, value_type, collector_view_type,
		Hash, sizeof...(Dependencies)> commit_storage_type;
	typedef std::array<util::revision_type, sizeof...(Dependencies)> revisions_type;
	return details::make_repository<collector_view_type, commit_storage_type,
			std::equal_to<collector_view_type>>([
				function = std::move(internal::get_function(function)),
				executor = std::move(internal::get_executor(function))](
				auto &&callback, const auto &previous_storage, const auto &dependencies) {
		typedef util::fixed_size_collector_type<value_type, Comparator> collector_type;
		typedef vector_view_type<value_type, Comparator> collector_view_type;

		auto previous(std::atomic_load(&*previous_storage));
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
			bool cache_usable(previous && frp::util::tuple_le_except_index<I>(
				revisions, previous->revisions));
			for (const auto &value : collection) {
				std::size_t index(counter++);
				executor([function, collector, index, &value, callback, previous, revisions,
					cache_usable, values]() {
					auto &collection(std::get<I>(values)->value);
					typename commit_storage_type::cache_type::iterator it;
					if (cache_usable && (it = previous->cache.find(value))
						!= previous->cache.end() ? collector->construct(index, it->second)
						: collector->construct(index,
							util::indexed_invoke_with_replacement<I>(std::move(function),
								std::cref(value), util::invoke([&](const auto&... storage) {
									return std::tie(storage->value...);
								}, values)))) {
						auto commit(std::make_shared<commit_storage_type>(
							collector_view_type(std::move(*collector)),
							util::default_revision, revisions));
						std::transform(std::begin(collection), std::end(collection),
							std::begin(commit->value),
							std::inserter(commit->cache, std::end(commit->cache)),
							[](auto &key, auto &value) {
								return std::make_pair(key, std::ref(value));
							});
						callback(commit);
					}
				});
			}
		}
	}, std::forward<Dependencies>(dependencies)...);
}

template<std::size_t I, typename Hash, typename Function, typename... Dependencies>
auto map_cache(Function &&function, Dependencies... dependencies) {
	typedef util::map_return_t<I, Function, Dependencies...> value_type;
	static_assert(util::is_equality_comparable<value_type>::value,
		"T must implement equality comparator");
	return map_cache<I, std::equal_to<value_type>, Hash>(std::forward<Function>(function),
		std::forward<Dependencies>(dependencies)...);
}

template<std::size_t I, typename Function, typename... Dependencies>
auto map_cache(Function &&function, Dependencies... dependencies) {
	typedef typename util::unwrap_reference_t<std::tuple_element_t<I, std::tuple<Dependencies...>>>
		::value_type argument_container_type;
	typedef typename argument_container_type::value_type argument_type;
	return map_cache<I, std::hash<argument_type>>(std::forward<Function>(function),
		std::forward<Dependencies>(dependencies)...);
}

template<typename Comparator, typename Hash, typename Function, typename Dependency>
auto map_cache(Function &&function, Dependency dependency) {
	return map_cache<0, Comparator, Hash>(std::forward<Function>(function),
		std::forward<Dependency>(dependency));
}

template<typename Hash, typename Function, typename Dependency>
auto map_cache(Function &&function, Dependency dependency) {
	typedef util::map_return_t<0, Function, Dependency> value_type;
	static_assert(util::is_equality_comparable<value_type>::value,
		"T must implement equality comparator");
	return map_cache<std::equal_to<value_type>, Hash>(std::forward<Function>(function),
		std::forward<Dependency>(dependency));
}

template<typename Function, typename Dependency>
auto map_cache(Function &&function, Dependency dependency) {
	return map_cache<0>(std::forward<Function>(function), std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace stat
} // namespace frp

#endif // _FRP_STATIC_PUSH_MAP_CACHE_H_
