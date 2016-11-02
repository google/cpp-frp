#ifndef _FRP_PUSH_SINK_H_
#define _FRP_PUSH_SINK_H_

#include <frp/util/observable.h>
#include <frp/util/reference.h>
#include <frp/util/storage.h>
#include <memory>
#include <stdexcept>

namespace frp {
namespace push {

template<typename T>
struct sink_repository_type {

	typedef T value_type;

	template<typename Dependency>
	struct template_storage_type : util::storage_supplier_type<T> {

		explicit template_storage_type(Dependency &&dependency)
			: dependency(std::forward<Dependency>(dependency)) {}

		std::shared_ptr<util::storage_type<T>> get() const final override {
			return std::atomic_load(&value);
		}

		void evaluate() {
			std::atomic_store(&value, util::unwrap_reference(dependency).get());
		}

		std::shared_ptr<util::storage_type<T>> value; // Use atomics!
		Dependency dependency;
	};

	struct reference {
		std::shared_ptr<util::storage_type<T>> value;

		operator bool() const {
			return !!value;
		}

		const auto &operator*() const {
			if (!value) {
				throw std::domain_error("value not available");
			}
			else {
				return value->value;
			}
		}

		const auto operator->() const {
			return &operator*();
		}

		operator const T &() const {
			return operator*();
		}
	};

	reference operator*() const {
		return reference{ get() };
	}

	auto get() const {
		return storage->get();
	}

	template<typename Dependency>
	static auto make(Dependency &&dependency) {
		return sink_repository_type(std::make_shared<template_storage_type<Dependency>>(
			std::forward<Dependency>(dependency)));
	}

	template<typename Storage>
	explicit sink_repository_type(const std::shared_ptr<Storage> &storage)
		: storage(storage)
		, callback(util::add_callback(util::unwrap_reference(storage->dependency),
			[storage, weak_storage = std::weak_ptr<Storage>(storage)]() {
				auto storage(weak_storage.lock());
				if (storage) {
					storage->evaluate();
				}
			})) {
		storage->evaluate();
	}

	sink_repository_type() = default;
	sink_repository_type(const sink_repository_type &) = delete;
	sink_repository_type(sink_repository_type &&) = default;
	sink_repository_type &operator=(const sink_repository_type &) = delete;
	sink_repository_type &operator=(sink_repository_type &&) = default;

	std::shared_ptr<util::storage_supplier_type<T>> storage;
	util::observable_type::reference_type callback;
};

template<typename Dependency>
auto sink(Dependency &&dependency) {
	typedef typename util::unwrap_t<Dependency>::value_type value_type;
	static_assert(std::is_move_constructible<value_type>::value, "T must be move constructible");
	return sink_repository_type<value_type>::make(std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_SINK_H_
