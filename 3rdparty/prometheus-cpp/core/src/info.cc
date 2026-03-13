#include "prometheus/info.h"

namespace prometheus {

ClientMetric Info::Collect() const {
  ClientMetric metric;
  metric.info.value = 1;
  return metric;
}

}  // namespace prometheus
