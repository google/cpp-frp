#
# Copyright 2016 Google Inc. All Rights Reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
include_directories(include)

set(FRP_INCLUDES
  "include/frp/internal/operator.h"
  "include/frp/static/push/filter.h"
  "include/frp/static/push/map.h"
  "include/frp/static/push/map_cache.h"
  "include/frp/static/push/repository.h"
  "include/frp/static/push/sink.h"
  "include/frp/static/push/source.h"
  "include/frp/static/push/transform.h"
  "include/frp/util/collector.h"
  "include/frp/util/function.h"
  "include/frp/util/list.h"
  "include/frp/util/observable.h"
  "include/frp/util/observe_all.h"
  "include/frp/util/reference.h"
  "include/frp/util/storage.h"
  "include/frp/util/variadic.h"
  "include/frp/util/vector.h"
  "include/frp/execute_on.h"
  "include/frp/vector_view.h"
)

add_library(cpp-frp ${FRP_INCLUDES} "src/dummy.cpp")
