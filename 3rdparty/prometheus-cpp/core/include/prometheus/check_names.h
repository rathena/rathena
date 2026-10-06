#pragma once

#include <string>

#include "prometheus/detail/core_export.h"
#include "prometheus/metric_type.h"

namespace prometheus {

PROMETHEUS_CPP_CORE_EXPORT bool CheckMetricName(const std::string& name);
PROMETHEUS_CPP_CORE_EXPORT bool CheckLabelName(const std::string& name,
                                               MetricType type);
}  // namespace prometheus
