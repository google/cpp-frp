#ifndef _FRP_PUSH_MAP_CACHE_H_
#define _FRP_PUSH_MAP_CACHE_H_

#include <frp/push/repository.h>
#include <frp/util/collector.h>
#include <frp/util/vector_view.h>
#include <vector>

namespace frp {
namespace push {
namespace implementation {

template<typename T, typename F, typename Executor, typename Input, typename Comparator, typename Hash>
struct map_cache_generator_type {
	typedef util::fixed_size_collector_type<T, Comparator> collector_type;
	typedef util::vector_view_type<T, Comparator> collector_view_type;
	typedef util::map_cache_commit_storage_type<typename Input::value_type, T, collector_view_type,
		Hash, 1> commit_storage_type;
	typedef typename commit_storage_type::revisions_type revisions_type;

	map_cache_generator_type(F &&function, Executor &&executor)
		: function(std::forward<F>(function)), executor(std::forward<Executor>(executor)) {}

	template<typename CallbackT>
	void operator()(CallbackT &&callback, revisions_type &revisions,
		const std::shared_ptr<commit_storage_type> &previous,
		const std::shared_ptr<util::storage_type<Input>> & storage) const {
		if (storage->value.empty()) {
			callback(std::make_shared<commit_storage_type>(collector_view_type(collector_type(0)),
				util::default_revision, revisions));
		}
		else {
			auto collector(std::make_shared<collector_type>(storage->value.size()));
			std::size_t counter(0);
			for (const auto &value : storage->value) {
				std::size_t index(counter++);
				executor([this, storage, collector, index, &value, callback, revisions, previous]() {
					typename commit_storage_type::cache_type::iterator it;
					if (previous && (it = previous->cache.find(value)) != previous->cache.end()
						? collector->construct(index, it->second)
						: collector->construct(index, function(value))) {
						auto commit(std::make_shared<commit_storage_type>(
							collector_view_type(std::move(*collector)),
							util::default_revision, revisions));
						for (std::size_t i = 0; i < storage->value.size(); ++i) {
							commit->cache.emplace(storage->value[i], std::ref(commit->value[i]));
						}
						callback(commit);
					}
				});
			}
		}
	}

	F function;
	Executor executor;
};

// TODO(gardell): Reuse
template<typename F, typename D>
using map_return_type = decltype(std::declval<F>()(
	std::declval<const typename util::unwrap_t<D>::value_type::value_type &>()));

}  // namespace implementation

template<typename Comparator, typename Hash, typename Function, typename Dependency>
auto map_cache(Function function, Dependency dependency) {
	typedef implementation::map_return_type<Function, Dependency> value_type;
	typedef implementation::map_cache_generator_type<value_type,
		internal::get_function_t<Function>, internal::get_executor_t<Function>,
		typename util::unwrap_t<Dependency>::value_type, Comparator, Hash> generator_type;
	typedef typename generator_type::collector_view_type collector_view_type;
	return repository_type<collector_view_type>::make<std::equal_to<collector_view_type>>(generator_type(
		std::move(internal::get_function(function)), std::move(internal::get_executor(function))),
		std::forward<Dependency>(dependency));
}

template<typename Hash, typename Function, typename Dependency>
auto map_cache(Function function, Dependency dependency) {
	typedef implementation::map_return_type<Function, Dependency> value_type;
	return map_cache<std::equal_to<value_type>, Hash>(std::forward<Function>(function),
		std::forward<Dependency>(dependency));
}

template<typename Function, typename Dependency>
auto map_cache(Function function, Dependency dependency) {
	typedef typename util::unwrap_t<Dependency>::value_type::value_type argument_type;
	return map_cache<std::hash<argument_type>>(std::forward<Function>(function),
		std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_MAP_CACHE_H_
