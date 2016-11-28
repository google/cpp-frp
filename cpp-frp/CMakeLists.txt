include_directories(include)

set(FRP_INCLUDES
  "include/frp/internal/operator.h"
  "include/frp/push/filter.h"
  "include/frp/push/map.h"
  "include/frp/push/map_cache.h"
  "include/frp/push/repository.h"
  "include/frp/push/sink.h"
  "include/frp/push/source.h"
  "include/frp/push/transform.h"
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
