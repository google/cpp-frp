#ifndef _FRP_PUSH_TRANSFORM_H_
#define _FRP_PUSH_TRANSFORM_H_

#include <frp/push/repository.h>

namespace frp {
namespace push {
namespace implementation {

template<typename T, typename F, typename Executor, typename... Ts>
struct transform_generator_type {
	typedef T value_type;
	constexpr static std::size_t dependencies_size = sizeof...(Ts);
	typedef util::commit_storage_type<T, dependencies_size> commit_storage_type;
	typedef typename commit_storage_type::revisions_type revisions_type;

	transform_generator_type(F &&function, Executor &&executor)
		: function(std::forward<F>(function)), executor(std::forward<Executor>(executor)) {}

	template<typename CallbackT>
	void operator()(CallbackT &&callback, revisions_type &revisions,
		const std::shared_ptr<util::storage_type<Ts>> &... storage) const {
		executor([=, callback = std::move(callback)]() {
			callback(commit_storage_type::make(std::bind(function, storage->value...), revisions));
		});
	}

	F function;
	Executor executor;
};

}  // namespace implementation

template<typename Function, typename... Dependencies>
auto transform(Function function, Dependencies... dependencies) {
	typedef decltype(internal::get_function(function)(util::unwrap_reference(dependencies).get()->value...))
		value_type;
	typedef implementation::transform_generator_type<value_type,
		internal::get_function_t<Function>, internal::get_executor_t<Function>,
		typename util::unwrap_t<Dependencies>::value_type...> generator_type;
	return repository_type<value_type>::make(
		generator_type(std::move(internal::get_function(function)),
			std::move(internal::get_executor(function))),
		std::forward<Dependencies>(dependencies)...);
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_TRANSFORM_H_
