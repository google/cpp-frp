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
include_directories(../cpp-frp/include)
include_directories(${gtest_SOURCE_DIR}/include)

set(INCLUDES
  "include/array_util.h"
  "include/task_queue.h"
  "include/test_types.h"
  "include/thread_pool.h"
)

set(SOURCES
  "src/collector-test.cpp"
  "src/example-test.cpp"
  "src/filter-test.cpp"
  "src/list-test.cpp"
  "src/map_cache-test.cpp"
  "src/map-test.cpp"
  "src/source-sink-test.cpp"
  "src/transform-test.cpp"
  "src/vector-test.cpp"
)

add_executable(frp-test ${INCLUDES} ${SOURCES})
target_link_libraries(frp-test gtest gtest_main)
if (${ENABLE_COVERAGE})
  target_link_libraries(frp-test -fprofile-arcs -ftest-coverage)
endif ()

add_test(UnitTests frp-test)
