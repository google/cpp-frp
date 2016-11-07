#ifndef _FRP_PUSH_FILTER_H_
#define _FRP_PUSH_FILTER_H_

#include <frp/push/repository.h>
#include <frp/util/collector.h>
#include <frp/vector_view.h>
#include <vector>

namespace frp {
namespace push {
namespace implementation {

template<typename T, typename F, typename Executor, typename Input, typename Comparator>
struct filter_generator_type {
	typedef util::append_collector_type<T, Comparator> collector_type;
	typedef vector_view_type<T, Comparator> collector_view_type;
	typedef util::commit_storage_type<collector_view_type, 1> commit_storage_type;
	typedef typename commit_storage_type::revisions_type revisions_type;

	filter_generator_type(F &&function, Executor &&executor)
		: function(std::forward<F>(function)), executor(std::forward<Executor>(executor)) {}

	template<typename Callback>
	void operator()(Callback &&callback, const std::shared_ptr<commit_storage_type> &previous,
			const std::shared_ptr<util::storage_type<Input>> & storage) const {
		const revisions_type revisions{ storage->revision };
		if (storage->value.empty()) {
			callback(std::make_shared<commit_storage_type>(collector_view_type(collector_type(0)),
				util::default_revision, revisions));
		} else {
			auto collector(std::make_shared<collector_type>(storage->value.size()));
			std::size_t counter(0);
			for (const auto &value : storage->value) {
				std::size_t index(counter++);
				executor([this, storage, collector, index, &value, callback, revisions]() {
					if (function(value)
						? collector->construct(std::ref(value)) : collector->skip()) {
						callback(std::make_shared<commit_storage_type>(
							collector_view_type(std::move(*collector)),
							util::default_revision, revisions));
					}
				});
			}
		}
	}

	F function;
	Executor executor;
};

}  // namespace implementation

template<typename Comparator, typename Function, typename Dependency>
auto filter(Function function, Dependency dependency) {
	typedef util::unwrap_t<Dependency>::value_type::value_type value_type;
	static_assert(!std::is_void<value_type>::value, "T must not be void type.");
	static_assert(std::is_copy_constructible<value_type>::value, "T must be copy constructible.");
	typedef implementation::filter_generator_type<value_type,
		internal::get_function_t<Function>, internal::get_executor_t<Function>,
		typename util::unwrap_t<Dependency>::value_type, Comparator> generator_type;
	typedef typename generator_type::collector_view_type collector_view_type;
	return details::make_repository<collector_view_type, typename generator_type::commit_storage_type,
		std::equal_to<collector_view_type>>(generator_type(
			std::move(internal::get_function(function)),
			std::move(internal::get_executor(function))),
			std::forward<Dependency>(dependency));
}

template<typename Function, typename Dependency>
auto filter(Function function, Dependency dependency) {
	typedef util::unwrap_t<Dependency>::value_type::value_type value_type;
	static_assert(util::is_equality_comparable<value_type>::value,
		"T must implement equality comparator");
	return filter<std::equal_to<value_type>>(std::forward<Function>(function),
		std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_FILTER_H_
