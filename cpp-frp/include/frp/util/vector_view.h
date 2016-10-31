#ifndef _FRP_UTIL_VECTOR_VIEW_H_
#define _FRP_UTIL_VECTOR_VIEW_H_

#include <atomic>
#include <cassert>
#include <frp/util/collector.h>

namespace frp {
namespace util {

template<typename T, typename Comparator = std::equal_to<T>,
	typename Allocator = std::allocator<T>>
struct vector_view_type {

	typedef array_deleter_type<T, vector_view_type<T, Comparator, Allocator>, Allocator>
		deleter_type;

	typedef T value_type;
	typedef Allocator allocator_type;
	typedef Comparator comparator_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef typename std::allocator_traits<Allocator>::pointer pointer;
	typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;

	struct iterator : std::iterator<std::random_access_iterator_tag, T> {
		friend struct vector_view_type<T, Comparator, Allocator>;

		auto operator+=(difference_type difference) {
			p += difference;
			return *this;
		}

		auto operator-=(difference_type difference) {
			p -= difference;
			return *this;
		}

		auto operator+(difference_type difference) const {
			return iterator(p + difference);
		}

		auto operator-(difference_type difference) const {
			return iterator(p + difference);
		}

		difference_type operator-(const iterator &it) const {
			return p - it.p;
		}

		const_reference operator[](size_type index) const {
			return p[index];
		}

		bool operator<(const iterator &it) const {
			return p < it.p;
		}

		bool operator>(const iterator &it) const {
			return p > it.p;
		}

		bool operator<=(const iterator &it) const {
			return p <= it.p;
		}

		bool operator>=(const iterator &it) const {
			return p >= it.p;
		}

		bool operator==(const iterator &it) const {
			return p == it.p;
		}

		bool operator!=(const iterator &it) const {
			return p != it.p;
		}

		iterator &operator++() {
			++p;
			return *this;
		}

		iterator operator++(int) const {
			return iterator(p + 1);
		}

		iterator &operator--() {
			--p;
			return *this;
		}

		iterator operator--(int) const {
			return iterator(p - 1);
		}

		const_reference operator*() const {
			return *p;
		}

		pointer operator->() const {
			return p;
		}

	private:
		explicit iterator(const_pointer p) : p(p) {}

		const_pointer p;
	};
	typedef iterator const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	explicit vector_view_type(fixed_size_collector_type<T, Comparator, Allocator> &&collector)
		: storage(collector.storage.release(), deleter_type{ *this })
		, comparator(std::move(collector.comparator))
		, allocator(std::move(collector.allocator))
		, storage_size(collector.storage_size)
		, capacity(collector.capacity) {
		assert(storage_size == collector.capacity);
	}

	explicit vector_view_type(append_collector_type<T, Comparator, Allocator> &&collector)
		: storage(collector.storage.release(), deleter_type{ *this })
		, comparator(std::move(collector.comparator))
		, allocator(std::move(collector.allocator))
		, storage_size(collector.storage_size)
		, capacity(collector.capacity) {
		assert(capacity == collector.counter);
	}

	vector_view_type(const vector_view_type &collector)
		: storage(allocator.allocate(collector.storage_size), deleter_type{ *this })
		, comparator(std::move(collector.comparator))
		, allocator(std::move(collector.allocator))
		, storage_size(collector.storage_size)
		, capacity(collector.storage_size) {
		for (size_type index = 0; index < storage_size; ++index) {
			std::allocator_traits<Allocator>::construct(allocator, &storage[index],
				std::ref(collector[index]));
		}
	}

	vector_view_type()
		: storage((pointer) nullptr, deleter_type{ *this }), storage_size(0), capacity(0) {}

	auto &operator=(const vector_view_type &copy) {
		comparator = copy.comparator;
		allocator = copy.allocator;
		storage_size = copy.storage_size;
		capacity = copy.capacity;
		storage.reset(allocator.allocate(storage_size));
		for (size_type index = 0; index < storage_size; ++index) {
			std::allocator_traits<Allocator>::construct(allocator, &storage[index],
				std::ref(copy[index]));
		}
		return *this;
	}

	reference operator[](size_type index) const {
		return storage[index];
	}

	const_iterator begin() const {
		return const_iterator(storage.get());
	}

	const_iterator end() const {
		return const_iterator(storage.get() + storage_size);
	}

	auto rbegin() const {
		return const_reverse_iterator(end() - 1);
	}

	auto rend() const {
		return const_reverse_iterator(begin() - 1);
	}

	auto size() const {
		return storage_size;
	}

	bool empty() const {
		return storage_size == 0;
	}

	bool operator==(const vector_view_type &collector) const {
		return storage_size == collector.size()
			&& std::equal(begin(), end(), collector.begin(), collector.end(), comparator);
	}

	typedef std::unique_ptr<T[], deleter_type> storage_type;
	storage_type storage;
	Comparator comparator;
	Allocator allocator;
	size_type storage_size;
	size_type capacity;
};

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_VECTOR_VIEW_H_
