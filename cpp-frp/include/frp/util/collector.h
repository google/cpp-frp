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
#ifndef _FRP_UTIL_COLLECTOR_H_
#define _FRP_UTIL_COLLECTOR_H_

#include <algorithm>
#include <atomic>
#include <cassert>
#include <frp/util/list.h>
#include <functional>

namespace frp {

template<typename T, typename Allocator, typename Comparator>
struct vector_view_type;

namespace util {

template<typename T, typename Container, typename Allocator>
struct array_deleter_type {

	Container &container;

	void operator()(T* ptr) {
		std::for_each(ptr, ptr + container.storage_size, [this](auto &value) {
			std::allocator_traits<Allocator>::destroy(container.allocator, &value);
		});
		container.allocator.deallocate(ptr, container.capacity);
	}
};

template<typename T, typename Comparator = std::equal_to<T>,
	typename Allocator = std::allocator<T>>
struct fixed_size_collector_type {
	template<typename U, typename Allocator_, typename Comparator_>
	friend struct frp::vector_view_type;
	template<typename U, typename Container_, typename Allocator_>
	friend struct array_deleter_type;

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
		return size == capacity;
	}

	std::size_t size() const {
		return storage_size;
	}

	~fixed_size_collector_type() {
		if (storage) {
			assert(storage_size == capacity);
		}
	}

private:
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
	friend struct frp::vector_view_type;
	template<typename U, typename Container_, typename Allocator_>
	friend struct array_deleter_type;

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

private:
	typedef std::unique_ptr<T[], deleter_type> storage_type;
	storage_type storage;
	Comparator comparator;
	Allocator allocator;
	std::atomic_size_t storage_size;
	std::size_t capacity;
	std::atomic_size_t counter;
};

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_COLLECTOR_H_
