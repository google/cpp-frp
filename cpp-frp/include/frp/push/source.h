#ifndef _FRP_PUSH_SOURCE_H_
#define _FRP_PUSH_SOURCE_H_

#include <frp/util/observable.h>
#include <frp/util/storage.h>
#include <memory>
#include <stdexcept>

namespace frp {
namespace push {

template<typename T>
struct source_repository_type {

	typedef T value_type;

	explicit source_repository_type(T &&value)
		: storage(std::make_unique<storage_type>(std::forward<T>(value))) {}
	source_repository_type() : storage(std::make_unique<storage_type>()) {}

	struct storage_type : util::observable_type {
		storage_type() = default;
		explicit storage_type(T &&value) : value(std::make_shared<util::storage_type<T>>(
			std::forward<T>(value), util::default_revision)) {}

		std::shared_ptr<util::storage_type<T>> value; // Use atomics!
	};

	auto &operator=(T &&value) const {
		accept((std::make_shared<util::storage_type<T>>(std::forward<T>(value))));
		return *this;
	}

	auto &operator=(const T &value) const {
		accept(std::make_shared<util::storage_type<T>>(value));
		return *this;
	}

	operator bool() const {
		return !!get();
	}

	auto operator*() const {
		auto storage(get());
		if (!storage) {
			throw std::domain_error("value not available");
		} else {
			return storage->value;
		}
	}

	void accept(std::shared_ptr<util::storage_type<T>> &&replacement) const {
		auto current = get();
		do {
			replacement->revision = (current ? current->revision : util::default_revision) + 1;
		} while ((!current || !current->compare_value(*replacement))
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
auto source() {
	return source_repository_type<T>();
}

template<typename T>
auto source(T &&value) {
	return source_repository_type<T>(std::forward<T>(value));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_SOURCE_H_
