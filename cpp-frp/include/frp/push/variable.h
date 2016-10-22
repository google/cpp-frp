#ifndef _FRP_PUSH_VARIABLE_H_
#define _FRP_PUSH_VARIABLE_H_

#include <frp/util/observable.h>
#include <frp/util/storage.h>
#include <memory>

namespace frp {
namespace push {

template<typename T>
struct mutable_repository_type {

	typedef T value_type;

	explicit mutable_repository_type(T &&value)
		: storage(std::make_unique<storage_type>(std::forward<T>(value))) {}

	struct storage_type : util::observable_type {
		explicit storage_type(T &&value)
			: value(std::make_shared<util::storage_type<T>>(std::forward<T>(value),
				util::default_revision)) {}

		std::shared_ptr<util::storage_type<T>> value; // Use atomics!
	};

	auto operator=(T &&value) const {
		auto replacement = std::make_shared<util::storage_type<T>>(std::forward<T>(value));

		auto current = get();
		do {
			replacement->revision = current->revision + 1;
		} while ((!current || !current->compare_value(replacement))
			&& !std::atomic_compare_exchange_weak(&storage->value, &current, replacement));
		storage->update();
	}

	auto get() const {
		return std::atomic_load(&storage->value);
	}

	template<typename F>
	auto add_callback(F &&f) const {
		return storage->add_callback(std::forward<F>(f));
	}

	std::unique_ptr<storage_type> storage;
};

template<typename T>
mutable_repository_type<T> variable(T &&value) {
	return mutable_repository_type<T>(std::forward<T>(value));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_VARIABLE_H_
