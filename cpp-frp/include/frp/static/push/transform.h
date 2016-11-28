#ifndef _FRP_STATIC_PUSH_TRANSFORM_H_
#define _FRP_STATIC_PUSH_TRANSFORM_H_

#include <frp/internal/namespace_alias.h>
#include <frp/static/push/repository.h>

namespace frp {
namespace stat {
namespace push {

template<typename Comparator, typename Function, typename... Dependencies>
auto transform(Function &&function, Dependencies... dependencies) {
	static_assert(util::all_true_type<typename util::is_not_void<
		typename util::unwrap_container_t<Dependencies>::value_type>::type...>::value,
		"Dependencies can not be void type.");

	typedef util::transform_return_type<Function, Dependencies...> value_type;
	typedef util::commit_storage_type<value_type, sizeof...(Dependencies)> commit_storage_type;

	return details::make_repository<value_type, commit_storage_type, Comparator>(
		[function = internal::get_function(util::unwrap_reference(std::forward<Function>(function))),
		 executor = internal::get_executor(util::unwrap_reference(std::forward<Function>(function)))](
			auto &&callback, const auto &, const auto &... storage) {
		executor([=, callback = std::move(callback)]() {
			callback(commit_storage_type::make(std::bind(std::ref(function),
				std::cref(storage->value)...), { storage->revision... }));
		});
	}, std::forward<Dependencies>(dependencies)...);
}

template<typename Function, typename... Dependencies>
auto transform(Function &&function, Dependencies... dependencies) {
	typedef util::transform_return_type<Function, Dependencies...> value_type;
	return transform<std::equal_to<value_type>, Function, Dependencies...>(
		std::forward<Function>(function), std::forward<Dependencies>(dependencies)...);
}

} // namespace push
} // namespace stat
} // namespace frp

#endif // _FRP_STATIC_PUSH_TRANSFORM_H_
