#ifndef _FRP_UTIL_OBSERVABLE_H_
#define _FRP_UTIL_OBSERVABLE_H_

#include <frp/util/list.h>
#include <functional>
#include <memory>

namespace frp {
namespace util {

struct observable_type {

	typedef std::function<void()> callback_type;
	typedef single_list_type<callback_type> callback_container_type;

	struct reference_type {

		struct storage_type {
			callback_container_type::iterator iterator;
			observable_type &observable;

			storage_type(callback_container_type::iterator iterator, observable_type &observable)
				: iterator(iterator), observable(observable) {}

			~storage_type() {
				observable.callbacks.erase(std::move(iterator));
			}
		};

		std::unique_ptr<storage_type> storage;
	};

	template<typename F>
	reference_type add_callback(F &&f) {
		return { std::make_unique<reference_type::storage_type>(
			callbacks.insert(std::forward<F>(f)), *this) };
	}

	void update() const {
		callbacks.for_each([](const auto &callback) {
			callback();
		});
	}

private:
	callback_container_type callbacks;
};

template<typename O, typename F>
auto add_callback(O &observable, F &&f) {
	return observable.add_callback(std::forward<F>(f));
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_OBSERVABLE_H_