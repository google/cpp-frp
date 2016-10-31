#ifndef _FRP_UTIL_VECTOR_H_
#define _FRP_UTIL_VECTOR_H_

#include <array>
#include <atomic>
#include <memory>
#include <vector>

namespace frp {
namespace util {

template<typename T, std::size_t N>
auto vector_from_array(std::array<T, N> &&array) {
	return std::vector<T>(std::make_move_iterator(array.begin()),
		std::make_move_iterator(array.end()));
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_VECTOR_H_
