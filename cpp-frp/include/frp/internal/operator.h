#ifndef _FRP_INTERNAL_OPERATOR_H_
#define _FRP_INTERNAL_OPERATOR_H_

namespace frp {
namespace internal {

template<typename T>
auto get_storage(T &value)->decltype(value.get_storage()) {
	return value.get_storage();
}

} // namespace details
} // namespace frp

#endif // _FRP_INTERNAL_OPERATOR_H_
