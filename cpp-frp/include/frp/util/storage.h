#ifndef _FRP_UTIL_STORAGE_H_
#define _FRP_UTIL_STORAGE_H_

#include <array>
#include <cstdint>
#include <frp/util/observable.h>
#include <memory>
#include <unordered_map>

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

	static_assert(std::is_move_constructible<T>::value, "T must be move constructible");
	static_assert(is_equality_comparable<T>::value, "T must implement equality comparator");

	storage_type(T &&value, revision_type revision = default_revision)
		: value(std::forward<T>(value)), revision(revision) {}

	storage_type(const T &value, revision_type revision = default_revision)
		: value(value), revision(revision) {}

	auto compare_value(storage_type &storage) const {
		return value == storage.value;
	}

	template<typename Comparator>
	auto compare_value(storage_type &storage, Comparator &comparator) const {
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

template<typename T>
struct storage_supplier_type : observable_type {
	virtual std::shared_ptr<storage_type<T>> get() const = 0;
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

// TODO(gardell): ADD HASH TYPE
// TODO(gardell): Key should be reference_wrapper
// Wrap reference_wrapper hash function around K hash type
template<typename K, typename V, typename Container, typename Hash, std::size_t DependenciesN>
struct map_cache_commit_storage_type : commit_storage_type<Container, DependenciesN> {

	typedef std::unordered_map<K, std::reference_wrapper<V>, Hash> cache_type;
	cache_type cache;

	map_cache_commit_storage_type(Container &&value, revision_type revision,
		const revisions_type &revisions)
		: commit_storage_type<Container, DependenciesN>(
			std::forward<Container>(value), revision, revisions) {}
};

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_STORAGE_H_
