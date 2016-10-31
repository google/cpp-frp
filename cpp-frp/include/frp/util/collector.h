#ifndef _FRP_UTIL_COLLECTOR_H_
#define _FRP_UTIL_COLLECTOR_H_

#include <atomic>
#include <cassert>
#include <frp/util/list.h>

namespace frp {
namespace util {

template<typename T, typename Container, typename Allocator = std::allocator<T>>
struct array_deleter_type {

	Container &container;

	void operator()(T* ptr) {
		std::for_each(ptr, ptr + container.size(), [this](auto &value) {
			std::allocator_traits<Allocator>::destroy(container.allocator, &value);
		});
		container.allocator.deallocate(ptr, container.capacity);
	}
};

template<typename T, typename Comparator = std::equal_to<T>,
	typename Allocator = std::allocator<T>>
struct fixed_size_collector_type {
	template<typename U, typename Allocator_, typename Comparator_>
	friend struct collector_view_type;

	typedef array_deleter_type<T, fixed_size_collector_type<T, Comparator, Allocator>, Allocator>
		deleter_type;

	explicit fixed_size_collector_type(std::size_t size, Allocator &&allocator = Allocator(),
		const Comparator &comparator = Comparator())
		: storage(allocator.allocate(size), deleter_type{ *this })
		, storage_size(0)
		, capacity(size)
		, comparator(comparator) {}

	fixed_size_collector_type(fixed_size_collector_type &&) = delete;

	template<typename... Args>
	bool construct(std::size_t index, Args&&... args) {
		assert(index < capacity);
		auto size(++storage_size);
		assert(storage_size <= capacity);

		std::allocator_traits<Allocator>::construct(allocator, &storage[index],
			std::forward<Args>(args)...);
		return storage_size == capacity;
	}

	std::size_t size() const {
		return storage_size;
	}

	~fixed_size_collector_type() {
		if (storage) {
			assert(storage_size == capacity);
		}
	}

	typedef std::unique_ptr<T[], deleter_type> storage_type;
	storage_type storage;
	Comparator comparator;
	Allocator allocator;
	std::atomic_size_t storage_size;
	std::size_t capacity;
};

template<typename T, typename Comparator = std::equal_to<T>,
	typename Allocator = std::allocator<T>>
struct append_collector_type {
	template<typename U, typename Allocator_, typename Comparator_>
	friend struct collector_view_type;

	typedef array_deleter_type<T, append_collector_type<T, Comparator, Allocator>, Allocator>
		deleter_type;

	explicit append_collector_type(std::size_t size, Allocator &&allocator = Allocator(),
		const Comparator &comparator = Comparator())
		: storage(allocator.allocate(size), deleter_type{ *this })
		, comparator(comparator)
		, allocator(allocator)
		, storage_size(0)
		, capacity(size)
		, counter(0) {}

	append_collector_type(append_collector_type &&) = delete;

	template<typename... Args>
	bool construct(Args&&... args) {
		std::size_t index(storage_size++);
		assert(index < capacity);

		std::allocator_traits<Allocator>::construct(allocator, &storage[index],
			std::forward<Args>(args)...);
		return ++counter == capacity;
	}

	bool skip() {
		return ++counter == capacity;
	}

	std::size_t size() const {
		return storage_size;
	}

	typedef std::unique_ptr<T[], deleter_type> storage_type;
	storage_type storage;
	Comparator comparator;
	Allocator allocator;
	std::atomic_size_t storage_size;
	std::size_t capacity;
	std::atomic_size_t counter;
};

// TODO(gardell): Rename to vector_view or something generic
template<typename T, typename Comparator = std::equal_to<T>,
	typename Allocator = std::allocator<T>>
struct collector_view_type {

	typedef array_deleter_type<T, collector_view_type<T, Comparator, Allocator>, Allocator>
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
	typedef pointer iterator; // TODO(gardell): Make a proper iterator type inheriting std::iterator
	typedef pointer const_iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator; // TODO(gardell): Implement rbegin/rend/constant versions
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	explicit collector_view_type(fixed_size_collector_type<T, Comparator, Allocator> &&collector)
		: storage(collector.storage.release(), deleter_type{ *this })
		, comparator(std::move(collector.comparator))
		, allocator(std::move(collector.allocator))
		, storage_size(collector.storage_size)
		, capacity(collector.capacity) {
		assert(storage_size == collector.capacity);
	}

	explicit collector_view_type(append_collector_type<T, Comparator, Allocator> &&collector)
		: storage(collector.storage.release(), deleter_type{ *this })
		, comparator(std::move(collector.comparator))
		, allocator(std::move(collector.allocator))
		, storage_size(collector.storage_size)
		, capacity(collector.capacity) {
		assert(capacity == collector.counter);
	}

	collector_view_type(const collector_view_type &collector)
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

	collector_view_type()
		: storage((T *) nullptr, deleter_type{ *this }), storage_size(0), capacity(0) {}

	auto &operator=(const collector_view_type &copy) {
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
		return storage.get();
	}

	const_iterator end() const {
		return storage.get() + storage_size;
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

	bool operator==(const collector_view_type &collector) const {
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

#endif // _FRP_UTIL_COLLECTOR_H_
