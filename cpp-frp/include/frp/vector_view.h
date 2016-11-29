#ifndef _FRP_VECTOR_VIEW_H_
#define _FRP_VECTOR_VIEW_H_

#include <atomic>
#include <cassert>
#include <frp/util/collector.h>

namespace frp {
namespace internal {

template<typename T, typename Comparator, typename Allocator, bool CopyConstructible>
struct vector_view_type_impl;

template<typename T, typename Comparator, typename Allocator>
struct vector_view_type_impl<T, Comparator, Allocator, false> {

	template<typename U, typename Container_, typename Allocator_>
	friend struct util::array_deleter_type;

protected:
	typedef util::array_deleter_type<T, vector_view_type_impl<T, Comparator, Allocator, false>,
		Allocator> deleter_type;
	typedef std::unique_ptr<T[], deleter_type> storage_type;
	typedef std::size_t size_type;
	typedef T *pointer;

	vector_view_type_impl()
		: storage((pointer) nullptr, deleter_type{ *this }), storage_size(0), capacity(0) {}

	vector_view_type_impl(vector_view_type_impl &&copy)
		: storage(copy.storage.release(), deleter_type{ *this })
		, comparator(std::move(copy.comparator))
		, allocator(std::move(copy.allocator))
		, storage_size(copy.storage_size), capacity(copy.capacity) {}

	vector_view_type_impl &operator=(vector_view_type_impl &&copy) {
		storage = storage_type(copy.storage.release(), deleter_type{ *this });
		comparator = std::move(copy.comparator);
		allocator = std::move(copy.allocator);
		storage_size = copy.storage_size;
		capacity = copy.capacity;
		return *this;
	}

	template<typename Deleter>
	vector_view_type_impl(std::unique_ptr<T[], Deleter> &&storage, Comparator &&comparator,
		Allocator &&allocator, size_type storage_size, size_type capacity)
		: storage(storage.release(), deleter_type{ *this })
		, comparator(std::forward<Comparator>(comparator))
		, allocator(std::forward<Allocator>(allocator))
		, storage_size(storage_size)
		, capacity(capacity) {}

	storage_type storage;
	Comparator comparator;
	Allocator allocator;
	size_type storage_size;
	size_type capacity;
};

template<typename T, typename Comparator, typename Allocator>
struct vector_view_type_impl<T, Comparator, Allocator, true> {

	template<typename U, typename Container_, typename Allocator_>
	friend struct util::array_deleter_type;

protected:
	typedef util::array_deleter_type<T, vector_view_type_impl<T, Comparator, Allocator, true>,
		Allocator> deleter_type;
	typedef std::unique_ptr<T[], deleter_type> storage_type;
	typedef std::size_t size_type;
	typedef T *pointer;

	vector_view_type_impl()
		: storage((pointer) nullptr, deleter_type{ *this }), storage_size(0), capacity(0) {}

	vector_view_type_impl(vector_view_type_impl &&copy)
		: storage(copy.storage.release(), deleter_type{ *this })
		, comparator(std::move(copy.comparator))
		, allocator(std::move(copy.allocator))
		, storage_size(copy.storage_size), capacity(copy.capacity) {}

	vector_view_type_impl(const vector_view_type_impl &copy)
		: storage(allocator.allocate(copy.storage_size), deleter_type{ *this })
		, comparator(copy.comparator)
		, allocator(copy.allocator)
		, storage_size(copy.storage_size)
		, capacity(copy.storage_size) {
		for (size_type index = 0; index < storage_size; ++index) {
			std::allocator_traits<Allocator>::construct(allocator, &storage[index],
				std::ref(copy.storage[index]));
		}
	}

	vector_view_type_impl &operator=(vector_view_type_impl &&copy) {
		storage = storage_type(copy.storage.release(), deleter_type{ *this });
		comparator = std::move(copy.comparator);
		allocator = std::move(copy.allocator);
		storage_size = copy.storage_size;
		capacity = copy.capacity;
		return *this;
	}

	vector_view_type_impl &operator=(const vector_view_type_impl &copy) {
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

	template<typename Deleter>
	vector_view_type_impl(std::unique_ptr<T[], Deleter> &&storage, Comparator &&comparator,
		Allocator &&allocator, size_type storage_size, size_type capacity)
		: storage(storage.release(), deleter_type{ *this })
		, comparator(std::forward<Comparator>(comparator))
		, allocator(std::forward<Allocator>(allocator))
		, storage_size(storage_size)
		, capacity(capacity) {}

	storage_type storage;
	Comparator comparator;
	Allocator allocator;
	size_type storage_size;
	size_type capacity;
};

} // namespace internal

template<typename T, typename Comparator = std::equal_to<T>,
	typename Allocator = std::allocator<T>>
struct vector_view_type : internal::vector_view_type_impl<T, Comparator, Allocator,
		std::is_copy_constructible<T>::value> {

	typedef internal::vector_view_type_impl<T, Comparator, Allocator,
		std::is_copy_constructible<T>::value> parent_type;

	typedef T value_type;
	typedef Allocator allocator_type;
	typedef Comparator comparator_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef typename std::allocator_traits<Allocator>::pointer pointer;
	typedef typename std::allocator_traits<Allocator>::const_pointer const_pointer;

	struct iterator : std::iterator<std::random_access_iterator_tag, const T> {
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
			return iterator(p - difference);
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

	explicit vector_view_type(util::fixed_size_collector_type<T, Comparator, Allocator> &&collector)
		: parent_type(std::move(collector.storage), std::move(collector.comparator),
			std::move(collector.allocator), collector.storage_size, collector.capacity) {
		assert(parent_type::storage_size == collector.capacity);
	}

	explicit vector_view_type(util::append_collector_type<T, Comparator, Allocator> &&collector)
		: parent_type(std::move(collector.storage), std::move(collector.comparator),
			std::move(collector.allocator), collector.storage_size, collector.capacity) {
		assert(parent_type::capacity == collector.counter);
	}

	reference operator[](size_type index) const {
		return parent_type::storage[index];
	}

	const_iterator begin() const {
		return const_iterator(parent_type::storage.get());
	}

	const_iterator end() const {
		return const_iterator(parent_type::storage.get() + size());
	}

	auto rbegin() const {
		return const_reverse_iterator(end());
	}

	auto rend() const {
		return const_reverse_iterator(begin());
	}

	auto size() const {
		return parent_type::storage_size;
	}

	bool empty() const {
		return size() == 0;
	}

	bool operator==(const vector_view_type &collector) const {
		return size() == collector.size()
			&& std::equal(begin(), end(), collector.begin(), collector.end(),
				parent_type::comparator);
	}
};

} // namespace frp

#endif // _FRP_VECTOR_VIEW_H_
