#ifndef _FRP_UTIL_OBSERVE_ALL_H_
#define _FRP_UTIL_OBSERVE_ALL_H_

#include <array>

namespace frp {
namespace util {

template<typename F>
struct observe_all_type {

	template<typename... Observables>
	auto operator() (Observables&... observables) const {
		typedef std::array<observable_type::reference_type, sizeof...(Observables)> array_type;
		return array_type{ add_callback(util::unwrap_container(observables), function)... };
	}

	F function;
};

template<typename F>
static auto observe_all(F &&function) {
	return observe_all_type<F>{ std::forward<F>(function) };
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_OBSERVE_ALL_H_
