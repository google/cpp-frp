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
#ifndef _FRP_UTIL_OBSERVABLE_H_
#define _FRP_UTIL_OBSERVABLE_H_

#include <frp/util/list.h>
#include <functional>
#include <memory>

namespace frp {
namespace util {

struct observable_type {

	typedef std::function<void()> callback_type;
	typedef single_list_type<callback_type> callback_container_type;

	struct reference_type {

		struct storage_type {
			callback_container_type::iterator iterator;
			observable_type &observable;

			storage_type(callback_container_type::iterator iterator, observable_type &observable)
				: iterator(iterator), observable(observable) {}

			~storage_type() {
				observable.callbacks.erase(std::move(iterator));
			}
		};

		std::unique_ptr<storage_type> storage;
	};

	template<typename F>
	reference_type add_callback(F &&f) {
		return { std::make_unique<reference_type::storage_type>(
			callbacks.insert(std::forward<F>(f)), *this) };
	}

	void update() const {
		callbacks.for_each([](const auto &callback) {
			callback();
		});
	}

private:
	callback_container_type callbacks;
};

template<typename O, typename F>
auto add_callback(O &observable, F &&f)->decltype(observable.add_callback(std::forward<F>(f))) {
	return observable.add_callback(std::forward<F>(f));
}

} // namespace util
} // namespace frp

#endif // _FRP_UTIL_OBSERVABLE_H_
