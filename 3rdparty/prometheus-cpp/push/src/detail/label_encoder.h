#pragma once

#include <iosfwd>

#include "prometheus/labels.h"
#include "prometheus/detail/push_export.h"

namespace prometheus {
namespace detail {

PROMETHEUS_CPP_PUSH_EXPORT void encodeLabel(std::ostream& os, const Label& label);

}
}  // namespace prometheus