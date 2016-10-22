#ifndef _FRP_PUSH_FILTER_H_
#define _FRP_PUSH_FILTER_H_

#include <frp/push/repository.h>
#include <frp/util/collector.h>
#include <vector>

namespace frp {
namespace push {
namespace implementation {

template<typename T, typename F, typename Executor, typename Input,
	typename Container = std::vector<T>>
struct filter_generator_type {
	typedef Container value_type;
	typedef util::commit_storage_type<value_type, 1> commit_storage_type;
	typedef typename commit_storage_type::revisions_type revisions_type;

	filter_generator_type(F &&function, Executor &&executor)
		: function(std::forward<F>(function)), executor(std::forward<Executor>(executor)) {}

	template<typename CallbackT>
	void operator()(CallbackT &&callback, revisions_type &revisions,
		const std::shared_ptr<util::storage_type<Input>> & storage) const {
		auto collector(util::make_collector_type<T>(
			[callback = std::forward<CallbackT>(callback), revisions](value_type &&value) {
			callback(std::make_shared<commit_storage_type>(std::forward<value_type>(value),
				util::default_revision, revisions));
		},
			storage->value.size()));
		for (const auto &value : storage->value) {
			// explicit capture of storage to keep shared_ptr alive.
			executor([this, storage, &value, collector]() {
				if (function(value)) {
					collector->append_and_call_if_ready(value);
				} else {
					collector->decrease();
				}
			});
		}
	}

	F function;
	Executor executor;
};

}  // namespace implementation

template<typename Function, typename Dependency>
auto filter(Function function, Dependency dependency) {
	typedef util::unwrap_t<Dependency>::value_type::value_type return_type;
	typedef implementation::filter_generator_type<return_type,
		internal::get_function_t<Function>, internal::get_executor_t<Function>,
		typename util::unwrap_t<Dependency>::value_type> generator_type;
	typedef typename generator_type::value_type value_type;
	return repository_type<value_type>::make(generator_type(
		std::move(internal::get_function(function)), std::move(internal::get_executor(function))),
		std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_FILTER_H_
