#ifndef _FRP_UTIL_VECTOR_H_
#define _FRP_UTIL_VECTOR_H_

#include <array>
#include <vector>

namespace frp {
namespace util {

template<typename T, std::size_t N>
std::vector<T> make_vector(std::array<T, N> &&array) {
	std::vector<T> vector;
	vector.reserve(N);
	vector.assign(std::make_move_iterator(array.begin()), std::make_move_iterator(array.end()));
	return vector;
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_VECTOR_H_
