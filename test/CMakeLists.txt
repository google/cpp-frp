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
