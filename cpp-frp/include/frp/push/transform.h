#ifndef _FRP_PUSH_TRANSFORM_H_
#define _FRP_PUSH_TRANSFORM_H_

#include <frp/push/repository.h>

namespace frp {
namespace push {

template<typename Comparator, typename Function, typename... Dependencies>
auto transform(Function &&function, Dependencies... dependencies) {
	typedef util::transform_return_type<Function, Dependencies...> value_type;
	typedef util::commit_storage_type<value_type, sizeof...(Dependencies)> commit_storage_type;

	return impl::make_repository<value_type, commit_storage_type, Comparator>(
		[function = std::forward<Function>(function)](
			auto &&callback, const auto &, const auto &... storage) {
		internal::get_executor(function)([=, callback = std::move(callback)]() {
			callback(commit_storage_type::make(
				std::bind(internal::get_function(function), std::cref(storage->value)...),
				{ storage->revision... }));
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
} // namespace frp

#endif // _FRP_PUSH_TRANSFORM_H_
