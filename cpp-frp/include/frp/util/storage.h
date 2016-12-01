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
#ifndef _FRP_UTIL_STORAGE_H_
#define _FRP_UTIL_STORAGE_H_

#include <array>
#include <cstdint>
#include <frp/util/observable.h>
#include <memory>

namespace frp {
namespace util {

template<typename T, typename = bool>
struct is_equality_comparable : std::false_type {};

template<typename T>
struct is_equality_comparable<T, decltype(std::declval<const T&>() == std::declval<const T&>())>
	: std::true_type {};

typedef uint64_t revision_type;

constexpr revision_type default_revision = 0;

template<typename T>
struct storage_type {
	T value;
	revision_type revision;

	storage_type(T &&value, revision_type revision = default_revision)
		: value(std::forward<T>(value)), revision(revision) {}

	storage_type(const T &value, revision_type revision = default_revision)
		: value(value), revision(revision) {}

	auto compare_value(storage_type &storage) const {
		return value == storage.value;
	}

	template<typename Comparator>
	auto compare_value(storage_type &storage, const Comparator &comparator) const {
		return comparator(value, storage.value);
	}
};

template<>
struct storage_type<void> {
	revision_type revision;

	storage_type(revision_type revision) : revision(revision) {}

	auto compare_value(storage_type &storage) const {
		return false;
	}

	template<typename Comparator>
	auto compare_value(storage_type &storage, Comparator &comparator) const {
		return false;
	}
};

template<typename T, std::size_t DependenciesN>
struct commit_storage_type : storage_type<T> {
	constexpr static std::size_t dependents_size = DependenciesN;
	typedef std::array<revision_type, dependents_size> revisions_type;
	const revisions_type revisions;

	template<typename F>
	static auto make(F &&function, const revisions_type &revisions) {
		return std::make_shared<commit_storage_type<T, DependenciesN>>(function(),
			default_revision, revisions);
	}

	commit_storage_type(T &&value, revision_type revision, const revisions_type &revisions)
		: storage_type<T>(std::forward<T>(value), revision), revisions(revisions) {}

	bool is_newer(const revisions_type &revisions) const {
		return !(revisions < this->revisions);
	}
};

template<std::size_t DependenciesN>
struct commit_storage_type<void, DependenciesN> : storage_type<void> {
	constexpr static std::size_t dependents_size = DependenciesN;
	typedef std::array<revision_type, dependents_size> revisions_type;
	const revisions_type revisions;

	template<typename F>
	static auto make(F &&function, const revisions_type &revisions) {
		function();
		return std::make_shared<commit_storage_type<void, DependenciesN>>(default_revision,
			revisions);
	}

	commit_storage_type(revision_type revision,
		const std::array<revision_type, dependents_size> &revisions)
		: storage_type<void>(revision), revisions(revisions) {}

	bool is_newer(const revisions_type &revisions) const {
		return !(revisions < this->revisions);
	}
};

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_STORAGE_H_
