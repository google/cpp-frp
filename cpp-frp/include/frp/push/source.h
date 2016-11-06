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

	template<typename Comparator>
	static auto make(T &&value) {
		return source_repository_type<T>(std::make_unique<template_storage_type<Comparator>>(
			std::forward<T>(value)));
	}

	template<typename Comparator>
	static auto make(const T &value) {
		return source_repository_type<T>(std::make_unique<template_storage_type<Comparator>>(
			value));
	}

	template<typename Comparator>
	static auto make() {
		return source_repository_type<T>(std::make_unique<template_storage_type<Comparator>>());
	}

	template<typename StorageT>
	explicit source_repository_type(std::unique_ptr<StorageT> &&storage)
		: storage(std::forward<std::unique_ptr<StorageT>>(storage)) {}

	struct storage_type : util::storage_supplier_type<T> {
		virtual void accept(std::shared_ptr<util::storage_type<T>> &&) = 0;
	};

	template<typename Comparator>
	struct template_storage_type : storage_type {
		template_storage_type() = default;
		explicit template_storage_type(T &&value) : value(std::make_shared<util::storage_type<T>>(
			std::forward<T>(value), util::default_revision)) {}
		explicit template_storage_type(const T &value)
			: value(std::make_shared<util::storage_type<T>>(value, util::default_revision)) {}

		std::shared_ptr<util::storage_type<T>> get() const override final {
			return std::atomic_load(&value);
		}

		void accept(std::shared_ptr<util::storage_type<T>> &&replacement) override final {
			auto current = get();
			do {
				replacement->revision = (current ? current->revision : util::default_revision) + 1;
			} while ((!current || !current->compare_value(*replacement, comparator))
				&& !std::atomic_compare_exchange_weak(&value, &current, replacement));
			update();
		}

		std::shared_ptr<util::storage_type<T>> value; // Use atomics!
		Comparator comparator;
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

	auto &operator=(T &&value) const {
		storage->accept(std::make_shared<util::storage_type<T>>(std::forward<T>(value)));
		return *this;
	}

	auto &operator=(const T &value) const {
		storage->accept(std::make_shared<util::storage_type<T>>(value));
		return *this;
	}

	reference operator*() const {
		return reference{ get() };
	}

	auto get() const {
		return storage->get();
	}

	template<typename F>
	auto add_callback(F &&f) const {
		return storage->add_callback(std::forward<F>(f));
	}

	std::unique_ptr<storage_type> storage;
};

namespace details {

template<typename T>
struct source_type_requirements_type {

	typedef std::decay_t<T> value_type;

	static_assert(!std::is_void<value_type>::value, "T must not be void type.");
	static_assert(std::is_move_constructible<value_type>::value, "T must be move constructible");
};

template<typename T>
struct source_type_equality_requirements_type : source_type_requirements_type<T> {

	typedef std::decay_t<T> value_type;

	static_assert(util::is_equality_comparable<value_type>::value,
		"T must implement equality comparator");
};

} // namespace details

template<typename Comparator, typename T>
auto source() {
	typedef typename details::source_type_requirements_type<T>::value_type value_type;
	return source_repository_type<value_type>::make<Comparator>();
}

template<typename Comparator, typename T>
auto source(T &&value) {
	typedef typename details::source_type_requirements_type<T>::value_type value_type;
	return source_repository_type<value_type>::make<Comparator>(std::forward<T>(value));
}

template<typename Comparator, typename T>
auto source(const T &value) {
	typedef typename details::source_type_requirements_type<T>::value_type value_type;
	return source_repository_type<value_type>::make<Comparator>(value);
}

template<typename T>
auto source() {
	typedef typename details::source_type_equality_requirements_type<T>::value_type value_type;
	return source<std::equal_to<value_type>, T>();
}

template<typename T>
auto source(T &&value) {
	typedef typename details::source_type_equality_requirements_type<T>::value_type value_type;
	return source<std::equal_to<value_type>, T>(std::forward<T>(value));
}

template<typename T>
auto source(const T &value) {
	typedef typename details::source_type_equality_requirements_type<T>::value_type value_type;
	return source<std::equal_to<value_type>, T>(value);
}

} // namespace push
} // namespace frp

#endif // _FRP_PUSH_SOURCE_H_
