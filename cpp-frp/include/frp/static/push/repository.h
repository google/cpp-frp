/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _FRP_STATIC_PUSH_REPOSITORY_H_
#define _FRP_STATIC_PUSH_REPOSITORY_H_

#include <frp/execute_on.h>
#include <frp/internal/namespace_alias.h>
#include <frp/internal/operator.h>
#include <frp/util/function.h>
#include <frp/util/observable.h>
#include <frp/util/observe_all.h>
#include <frp/util/reference.h>
#include <frp/util/storage.h>
#include <frp/util/variadic.h>
#include <frp/util/vector.h>

namespace frp {
namespace stat {
namespace push {

template<typename T>
struct repository_type;

namespace details {

template<typename Commit, typename Comparator, typename Revisions>
void submit_commit(const std::shared_ptr<std::shared_ptr<Commit>> &previous,
		const std::shared_ptr<util::observable_type> &observable,
		const Comparator &comparator, const Revisions &revisions,
		const std::shared_ptr<Commit> &commit) {
	auto value(std::atomic_load(&*previous));
	bool exchanged(false);
	bool equals;
	do {
		commit->revision = (value ? value->revision : util::default_revision) + 1;
		equals = value && commit->compare_value(*value, comparator);
	} while ((!value || value->is_newer(revisions))
		&& !(exchanged = std::atomic_compare_exchange_strong(&*previous, &value, commit)));
	if (exchanged && !equals) {
		observable->update();
	}
}

template<typename Storage, typename Generator, typename Comparator, typename... Ts>
void generate_attempt_commit(const std::shared_ptr<std::shared_ptr<Storage>> &storage,
	const std::shared_ptr<util::observable_type> &observable, Generator &generator,
	Comparator &comparator, const std::shared_ptr<Ts> &... dependencies) {
	if (util::all_true(dependencies...)) {
		typedef std::array<util::revision_type, sizeof...(Ts)> revisions_type;
		revisions_type revisions{ dependencies->revision... };
		auto value(std::atomic_load(&*storage));
		if (!value || value->is_newer(revisions)) {
			generator(std::bind(&submit_commit<Storage, Comparator, revisions_type>, storage,
				observable, comparator, revisions, std::placeholders::_1), value, dependencies...);
		}
	}
}

template<typename Storage, typename Generator, typename Comparator, typename... Dependencies>
void attempt_commit_callback(const std::weak_ptr<std::shared_ptr<Storage>> &weak_storage,
		const std::shared_ptr<Generator> &generator, Comparator &comparator,
		const std::shared_ptr<util::observable_type> &observable,
		const std::shared_ptr<std::tuple<Dependencies...>> &dependencies) {
	auto storage(weak_storage.lock());
	if (storage) {
		util::invoke([&](const Dependencies&... dependencies) {
			generate_attempt_commit(storage, observable, *generator, comparator,
				internal::get_storage(util::unwrap_container(dependencies))...);
		}, std::ref(*dependencies));
	}
}

template<typename T, typename Storage, typename Comparator, typename Generator,
	typename... Dependencies>
repository_type<T> make_repository(Generator &&generator, Dependencies &&... dependencies);

} // namespace details

template<typename T>
struct repository_type {

	template<typename U, typename Storage, typename Comparator, typename Generator,
		typename... Dependencies>
	friend repository_type<U> details::make_repository(Generator &&generator,
		Dependencies &&... dependencies);
	template<typename O, typename F>
	friend auto util::add_callback(O &observable, F &&f)
		->decltype(observable.add_callback(std::forward<F>(f)));
	template<typename U>
	friend auto internal::get_storage(U &value)->decltype(value.get_storage());

	typedef T value_type;

	repository_type() = default;

private:
	template<typename Update, typename Provider, typename... Dependencies>
	repository_type(const std::shared_ptr<util::observable_type> &observable, const Update &update,
		Provider &&provider, const std::shared_ptr<std::tuple<Dependencies...>> &dependencies)
		: observable(observable)
		, callbacks(util::vector_from_array(util::invoke(
			util::observe_all(update), std::ref(*dependencies))))
		, provider(std::forward<Provider>(provider)) {}

	auto get_storage() const {
		return provider();
	}

	template<typename F>
	auto add_callback(F &&f) const {
		return observable->add_callback(std::forward<F>(f));
	}

	std::function<std::shared_ptr<util::storage_type<T>>()> provider;
	std::shared_ptr<util::observable_type> observable;
	std::vector<util::observable_type::reference_type> callbacks;
};

namespace details {

template<typename T, typename Storage, typename Comparator, typename Generator,
	typename... Dependencies>
repository_type<T> make_repository(Generator &&generator, Dependencies &&... dependencies) {
	// TODO(gardell): Group storage together, there's an awful lot of shared_ptr instances!
	// Note that storage should be kept separate, since its highly volatile.
	auto storage(std::make_shared<std::shared_ptr<Storage>>());
	auto observable(std::make_shared<util::observable_type>());
	auto shared_dependencies(std::make_shared<std::tuple<Dependencies...>>(
		std::forward<Dependencies>(dependencies)...));
	auto callback(std::bind(
		&attempt_commit_callback<Storage, Generator, Comparator, Dependencies...>,
		std::weak_ptr<std::shared_ptr<Storage>>(storage),
		std::make_shared<Generator>(std::forward<Generator>(generator)), Comparator(), observable,
		shared_dependencies));
	auto provider([=]() { return std::atomic_load(&*storage); });
	repository_type<T> repository(observable, callback, provider, shared_dependencies);
	callback();
	return repository;
}

} // namespace details

} // namespace push
} // namespace stat
} // namespace frp

#endif  // _FRP_STATIC_PUSH_REPOSITORY_H_
