#ifndef _FRP_UTIL_LIST_H_
#define _FRP_UTIL_LIST_H_

#include <atomic>
#include <memory>

namespace frp {
namespace util {

template<typename T>
struct single_list_type {
	struct node_type {
		T value;
		std::shared_ptr<node_type> next;
	};

	struct iterator {
		std::shared_ptr<node_type> node;

		T &operator*() const {
			return node->value;
		}

		T *operator->() const {
			return &node->value;
		}
	};

	std::shared_ptr<node_type> head;

	template<typename F>
	void for_each(F &&f) const {
		for (auto node = std::atomic_load(&head); node; node = std::atomic_load(&node->next)) {
			f(node->value);
		}
	}

	auto insert(T &&value) {
		auto node = std::make_shared<node_type>();
		node->value = std::forward<T>(value);
		node->next = std::atomic_load(&head);
		while (!std::atomic_compare_exchange_weak(&head, &node->next, node)) {}
		return iterator{ node };
	}

	auto insert(const T &value) {
		auto node = std::make_shared<node_type>();
		node->value = value;
		node->next = std::atomic_load(&head);
		while (!std::atomic_compare_exchange_weak(&head, &node->next, node)) {}
		return iterator{ node };
	}

	bool erase(const iterator &iterator) {
		{
			auto target(iterator.node);
			if (std::atomic_compare_exchange_strong(&head, &target, target->next)) {
				return true;
			}
		}
		{
			auto target(iterator.node);
			for (auto node = std::atomic_load(&head); node; node = std::atomic_load(&node->next)) {
				if (std::atomic_compare_exchange_strong(&node->next, &target, target->next)) {
					return true;
				}
			}
		}
		return false;
	}
};

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_LIST_H_
