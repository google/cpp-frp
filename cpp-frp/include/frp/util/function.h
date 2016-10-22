#ifndef _FRP_UTIL_FUNCTION_H_
#define _FRP_UTIL_FUNCTION_H_

#include <utility>

namespace frp {
namespace util {

template<typename F, typename Tuple, std::size_t... I>
auto invoke_impl(F &&f, Tuple &&tuple, std::index_sequence<I...>) {
	return f(std::move(std::get<I>(tuple))...);
}

template<typename F, typename... Ts>
auto invoke(F &&f, std::tuple<Ts...> &&tuple) {
	return invoke_impl(std::forward<F>(f),
		std::forward<std::tuple<Ts...>>(tuple),
		std::make_index_sequence<sizeof...(Ts)>{});
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_FUNCTION_H_
