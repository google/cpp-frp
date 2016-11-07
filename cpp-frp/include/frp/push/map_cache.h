#ifndef _FRP_PUSH_MAP_CACHE_H_
#define _FRP_PUSH_MAP_CACHE_H_

#include <frp/push/repository.h>
#include <frp/util/collector.h>
#include <frp/vector_view.h>
#include <unordered_map>
#include <vector>

namespace frp {
namespace push {
namespace details {

template<typename K, typename V, typename Container, typename Hash>
struct map_cache_commit_storage_type : util::commit_storage_type<Container, 1> {

	typedef std::unordered_map<K, std::reference_wrapper<const V>, Hash> cache_type;
	cache_type cache;

	map_cache_commit_storage_type(Container &&value, util::revision_type revision,
		const revisions_type &revisions) : util::commit_storage_type<Container, 1>(
			std::forward<Container>(value), revision, revisions) {}
};

} // namespace details

template<typename Comparator, typename Hash, typename Function, typename Dependency>
auto map_cache(Function &&function, Dependency dependency) {
	typedef typename util::unwrap_t<Dependency>::value_type argument_container_type;
	typedef typename argument_container_type::value_type argument_type;
	typedef util::map_return_type<Function, Dependency> value_type;
	static_assert(!std::is_void<argument_type>::value, "Dependency must not be void type.");
	static_assert(std::is_copy_constructible<argument_type>::value,
		"Dependency type must be copy constructible");
	static_assert(std::is_move_constructible<value_type>::value,
		"T must be move constructible");
	static_assert(!std::is_void<value_type>::value, "T must not be void type.");

	typedef vector_view_type<value_type, Comparator> collector_view_type;
	typedef details::map_cache_commit_storage_type<argument_type, value_type, collector_view_type,
		Hash> commit_storage_type;
	return details::make_repository<collector_view_type, commit_storage_type,
		std::equal_to<collector_view_type>>([
			function = std::move(internal::get_function(function)),
			executor = std::move(internal::get_executor(function))](
				auto &&callback, const auto &previous, const auto &storage) {
			typedef util::fixed_size_collector_type<value_type, Comparator> collector_type;
			typedef vector_view_type<value_type, Comparator> collector_view_type;
			typedef std::array<util::revision_type, 1> revisions_type;

			if (storage->value.empty()) {
				callback(std::make_shared<commit_storage_type>(
					collector_view_type(collector_type(0)),
					util::default_revision, revisions_type{ storage->revision }));
			}
			else {
				auto collector(std::make_shared<collector_type>(storage->value.size()));
				std::size_t counter(0);
				for (const auto &value : storage->value) {
					std::size_t index(counter++);
					executor([function, storage, collector, index, &value, callback, previous]() {
						typename commit_storage_type::cache_type::iterator it;
						if (previous && (it = previous->cache.find(value)) != previous->cache.end()
							? collector->construct(index, it->second)
							: collector->construct(index, function(value))) {
							auto commit(std::make_shared<commit_storage_type>(
								collector_view_type(std::move(*collector)),
								util::default_revision, revisions_type{ storage->revision }));
							auto storage_it(std::begin(storage->value));
							auto commit_it(std::begin(commit->value));
							for (;
									storage_it != std::end(storage->value)
									&& commit_it != std::end(commit->value);
									++storage_it, ++commit_it) {
								commit->cache.emplace(*storage_it, std::ref(*commit_it));
							}
							callback(commit);
						}
					});
				}
			}
		}, std::forward<Dependency>(dependency));
}

template<typename Hash, typename Function, typename Dependency>
auto map_cache(Function &&function, Dependency dependency) {
	typedef util::map_return_type<Function, Dependency> value_type;
	static_assert(util::is_equality_comparable<value_type>::value,
		"T must implement equality comparator");
	return map_cache<std::equal_to<value_type>, Hash>(std::forward<Function>(function),
		std::forward<Dependency>(dependency));
}

template<typename Function, typename Dependency>
auto map_cache(Function &&function, Dependency dependency) {
	typedef typename util::unwrap_t<Dependency>::value_type::value_type argument_type;
	return map_cache<std::hash<argument_type>>(std::forward<Function>(function),
		std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_MAP_CACHE_H_
