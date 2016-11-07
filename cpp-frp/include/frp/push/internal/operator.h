#ifndef _FRP_PUSH_INTERNAL_OPERATOR_H_
#define _FRP_PUSH_INTERNAL_OPERATOR_H_

namespace frp {
namespace push {
namespace details {

template<typename T>
auto get_storage(T &value) {
	return value.get_storage();
}

} // namespace details
} // namespace push
} // namespace frp

#endif // _FRP_PUSH_INTERNAL_OPERATOR_H_
