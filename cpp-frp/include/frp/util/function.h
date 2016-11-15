#ifndef _FRP_UTIL_FUNCTION_H_
#define _FRP_UTIL_FUNCTION_H_

#include <frp/util/reference.h>
#include <tuple>
#include <utility>

namespace frp {
namespace util {
namespace details {

template<typename F, typename Tuple, std::size_t... I>
auto invoke(F &&f, Tuple &&tuple, std::index_sequence<I...>) {
	return f(std::move(std::get<I>(unwrap_reference(std::forward<Tuple>(tuple))))...);
}

} // namespace details

template<typename F, typename Tuple>
auto invoke(F &&f, Tuple &&tuple) {
	return details::invoke(std::forward<F>(f), std::forward<Tuple>(tuple),
		std::make_index_sequence<std::tuple_size<unwrap_reference_t<Tuple>>::value>{});
}

template<typename F, typename... Ds>
using transform_return_type = decltype(std::declval<internal::get_function_t<F>>()(
	std::declval<const typename util::unwrap_container_t<Ds>::value_type &>()...));

template<typename F, typename... Ds>
using map_return_type = decltype(std::declval<internal::get_function_t<F>>()(
	std::declval<const typename util::unwrap_container_t<Ds>::value_type::value_type &>()...));

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_FUNCTION_H_
