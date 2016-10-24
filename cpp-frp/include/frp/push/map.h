#ifndef _FRP_PUSH_MAP_H_
#define _FRP_PUSH_MAP_H_

#include <frp/push/repository.h>
#include <frp/util/collector.h>
#include <vector>

namespace frp {
namespace push {
namespace implementation {

template<typename T, typename F, typename Executor, typename Input,
	typename Container = std::vector<T>>
struct map_generator_type {
	typedef Container value_type;
	typedef util::commit_storage_type<value_type, 1> commit_storage_type;
	typedef typename commit_storage_type::revisions_type revisions_type;

	map_generator_type(F &&function, Executor &&executor)
		: function(std::forward<F>(function)), executor(std::forward<Executor>(executor)) {}

	template<typename CallbackT>
	void operator()(CallbackT &&callback, revisions_type &revisions,
		const std::shared_ptr<util::storage_type<Input>> & storage) const {
		if (storage->value.empty()) {
			callback(std::make_shared<commit_storage_type>(value_type(),
				util::default_revision, revisions));
		} else {
			auto collector(util::make_collector_type<T>(
				[callback = std::forward<CallbackT>(callback), revisions](value_type &&value) {
					callback(std::make_shared<commit_storage_type>(std::forward<value_type>(value),
						util::default_revision, revisions));
				},
				storage->value.size()));
			for (const auto &value : storage->value) {
				// explicit capture of storage to keep shared_ptr alive.
				executor([this, storage, &value, collector]() {
					collector->append_and_call_if_ready(function(value));
				});
			}
		}
	}

	F function;
	Executor executor;
};

template<typename Function, typename Dependency>
using map_return_type =
	decltype((*(Function *)0)(*(typename util::unwrap_t<Dependency>::value_type::value_type *)0));

}  // namespace implementation

template<typename Function, typename Dependency>
auto map(Function function, Dependency dependency) {
	typedef implementation::map_return_type<Function, Dependency> return_type;
	typedef implementation::map_generator_type<return_type,
		internal::get_function_t<Function>, internal::get_executor_t<Function>,
		typename util::unwrap_t<Dependency>::value_type> generator_type;
	typedef typename generator_type::value_type value_type;
	return repository_type<value_type>::make(generator_type(
		std::move(internal::get_function(function)), std::move(internal::get_executor(function))),
		std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_MAP_H_
