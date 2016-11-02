#ifndef _FRP_PUSH_REPOSITORY_H_
#define _FRP_PUSH_REPOSITORY_H_

#include <frp/execute_on.h>
#include <frp/util/function.h>
#include <frp/util/observable.h>
#include <frp/util/observe_all.h>
#include <frp/util/reference.h>
#include <frp/util/storage.h>
#include <frp/util/variadic.h>
#include <frp/util/vector.h>

namespace frp {
namespace push {

template<typename T>
struct repository_type {

	typedef T value_type;

	template<typename Storage, typename Generator, typename Comparator, typename... Dependencies>
	struct template_storage_type
		: util::storage_supplier_type<T>
		, std::enable_shared_from_this<template_storage_type<
			Storage, Generator, Comparator, Dependencies...>> {

		constexpr static std::size_t dependencies_size = sizeof...(Dependencies);

		std::shared_ptr<util::storage_type<T>> get() const final override {
			return std::atomic_load(&value);
		}

		void evaluate() {
			util::invoke([this](Dependencies&... dependencies) {
				evaluate_impl(util::unwrap_reference(dependencies).get()...);
			}, std::ref(dependencies));
		}

		template<typename... Ts>
		void evaluate_impl(const std::shared_ptr<Ts> &... values) {
			if (util::all_true(values...)) {
				std::array<util::revision_type, dependencies_size> revisions{
					values->revision... };
				auto value(std::atomic_load(&value));
				if (!value || value->is_newer(revisions)) {
					generator([revisions, storage = shared_from_this()](
						const std::shared_ptr<Storage> &commit) {
						auto value(std::atomic_load(&storage->value));
						do {
							commit->revision = (value ? value->revision : util::default_revision) + 1;
							if (value && (!value->is_newer(revisions)
								|| commit->compare_value(*value, storage->comparator))) {
								return;
							}
						} while (!std::atomic_compare_exchange_strong(&storage->value, &value,
							commit));
						storage->update();
					}, value, values...);
				}
			}
		}

		template_storage_type(Generator &&generator, Dependencies &&... dependencies)
			: dependencies(std::forward<Dependencies>(dependencies)...)
			, generator(std::forward<Generator>(generator)) {}

		std::shared_ptr<Storage> value; // Use atomics!
		std::tuple<Dependencies...> dependencies;
		Generator generator;
		Comparator comparator;
	};

	template<typename Storage, typename Comparator, typename Generator, typename... Dependencies>
	static auto make(Generator &&generator, Dependencies &&... dependencies) {
		return repository_type(std::make_shared<template_storage_type<
			Storage, Generator, Comparator, Dependencies...>>(
				std::forward<Generator>(generator),
				std::forward<Dependencies>(dependencies)...));
	}

	template<typename Storage>
	explicit repository_type(const std::shared_ptr<Storage> &storage)
		: storage(storage)
		, callbacks(util::vector_from_array(util::invoke(util::observe_all(
			[storage, weak_storage = std::weak_ptr<Storage>(storage)]() {
		auto storage(weak_storage.lock());
		if (storage) {
			storage->evaluate();
		}
	}), std::ref(storage->dependencies)))) {
		storage->evaluate();
	}

	auto get() const {
		return storage->get();
	}

	template<typename F>
	auto add_callback(F &&f) const {
		return storage->add_callback(std::forward<F>(f));
	}

	repository_type() = default;
	repository_type(const repository_type &) = delete;
	repository_type(repository_type &&) = default;
	repository_type &operator=(const repository_type &) = delete;
	repository_type &operator=(repository_type &&) = default;

	std::shared_ptr<util::storage_supplier_type<T>> storage;
	std::vector<util::observable_type::reference_type> callbacks;
};

} // namespace push
} // namespace frp

#endif  // _FRP_PUSH_REPOSITORY_H_
