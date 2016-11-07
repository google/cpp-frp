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
struct sink_type {

	template<typename Dependency_>
	friend auto sink(Dependency_ &&dependency);

	typedef T value_type;

	sink_type() = default;

	struct reference {
		template<typename U>
		friend struct sink_type;

		typedef T value_type;

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

	private:
		explicit reference(std::shared_ptr<util::storage_type<T>> &&value)
			: value(std::forward<std::shared_ptr<util::storage_type<T>>>(value)) {}
		std::shared_ptr<util::storage_type<T>> value;
	};

	reference operator*() const {
		return reference(provider());
	}

private:
	template<typename Dependency>
	struct template_storage_type {

		explicit template_storage_type(Dependency &&dependency)
			: dependency(std::forward<Dependency>(dependency)) {}

		std::shared_ptr<util::storage_type<T>> get() const {
			return std::atomic_load(&value);
		}

		void evaluate() {
			std::atomic_store(&value, details::get_storage(util::unwrap_reference(dependency)));
		}

		std::shared_ptr<util::storage_type<T>> value; // Use atomics!
		Dependency dependency;
	};

	template<typename Dependency>
	static auto make(Dependency &&dependency) {
		return sink_type(std::make_shared<template_storage_type<Dependency>>(
			std::forward<Dependency>(dependency)));
	}

	template<typename Storage>
	explicit sink_type(const std::shared_ptr<Storage> &storage)
		: provider([=]() { return storage->get(); })
		, callback(util::add_callback(util::unwrap_reference(storage->dependency),
			[weak_storage = std::weak_ptr<Storage>(storage)]() {
				auto s(weak_storage.lock());
				if (s) {
					s->evaluate();
				}
			})) {
		storage->evaluate();
	}

	std::function<std::shared_ptr<util::storage_type<T>>()> provider;
	util::observable_type::reference_type callback;
};

template<typename Dependency>
auto sink(Dependency &&dependency) {
	typedef typename util::unwrap_t<Dependency>::value_type value_type;
	static_assert(!std::is_void<value_type>::value, "T must not be void type.");
	static_assert(std::is_move_constructible<value_type>::value, "T must be move constructible.");
	return sink_type<value_type>::make(std::forward<Dependency>(dependency));
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_SINK_H_
