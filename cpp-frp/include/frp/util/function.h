#ifndef _FRP_UTIL_FUNCTION_H_
#define _FRP_UTIL_FUNCTION_H_

#include <frp/util/reference.h>
#include <tuple>
#include <utility>

namespace frp {
namespace util {

template<typename F, typename Tuple, std::size_t... I>
auto invoke_impl(F &&f, Tuple tuple, std::index_sequence<I...>) {
	return f(std::move(std::get<I>(unwrap_reference(std::forward<Tuple>(tuple))))...);
}

template<typename F, typename Tuple>
auto invoke(F &&f, Tuple tuple) {
	return invoke_impl(std::forward<F>(f), std::forward<Tuple>(tuple),
		std::make_index_sequence<std::tuple_size<unwrap_t<Tuple>>::value>{});
}

template<typename F, typename... Ds>
using transform_return_type = decltype(std::declval<F>()(
	std::declval<const typename util::unwrap_t<Ds>::value_type &>()...));

template<typename F, typename... Ds>
using map_return_type = decltype(std::declval<F>()(
	std::declval<const typename util::unwrap_t<Ds>::value_type::value_type &>()...));

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_FUNCTION_H_
