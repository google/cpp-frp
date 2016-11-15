#ifndef _FRP_PUSH_MAP_H_
#define _FRP_PUSH_MAP_H_

#include <frp/push/repository.h>
#include <frp/util/collector.h>
#include <frp/vector_view.h>
#include <vector>

namespace frp {
namespace push {

template<typename Comparator, typename Function, typename Dependency>
auto map(Function &&function, Dependency dependency) {
	typedef typename util::unwrap_reference_t<Dependency>::value_type::value_type argument_type;
	typedef util::map_return_type<Function, Dependency> value_type;
	static_assert(!std::is_void<argument_type>::value, "Dependency must not be void type.");
	static_assert(std::is_move_constructible<value_type>::value,
		"T must be move constructible");
	static_assert(!std::is_void<value_type>::value, "T must not be void type.");

	typedef vector_view_type<value_type, Comparator> collector_view_type;
	typedef util::commit_storage_type<collector_view_type, 1> commit_storage_type;
	return details::make_repository<collector_view_type, commit_storage_type,
			std::equal_to<collector_view_type>>([
				function = std::move(internal::get_function(function)),
				executor = std::move(internal::get_executor(function))](
				auto &&callback, const auto &previous, const auto & storage) {
		typedef util::fixed_size_collector_type<value_type, Comparator> collector_type;
		typedef vector_view_type<value_type, Comparator> collector_view_type;
		typedef std::array<util::revision_type, 1> revisions_type;

		if (storage->value.empty()) {
			callback(std::make_shared<commit_storage_type>(collector_view_type(collector_type(0)),
				util::default_revision, revisions_type{ storage->revision }));
		} else {
			auto collector(std::make_shared<collector_type>(storage->value.size()));
			std::size_t counter(0);
			for (const auto &value : storage->value) {
				std::size_t index(counter++);
				executor([function, storage, collector, index, &value, callback]() {
					if (collector->construct(index, function(value))) {
						callback(std::make_shared<commit_storage_type>(
							collector_view_type(std::move(*collector)),
							util::default_revision, revisions_type{ storage->revision }));
					}
				});
			}
		}
	}, std::forward<Dependency>(dependency));
}

template<typename Function, typename Dependency>
auto map(Function &&function, Dependency dependency) {
	typedef util::map_return_type<Function, Dependency> value_type;
	static_assert(util::is_equality_comparable<value_type>::value,
		"T must implement equality comparator");
	return map<std::equal_to<value_type>>(std::forward<Function>(function),
		std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_MAP_H_
