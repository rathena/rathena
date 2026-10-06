#include "prometheus/detail/utils.h"

#include <cstddef>
#include <map>
#include <utility>

#include "hash.h"

namespace prometheus {

namespace detail {

std::size_t LabelHasher::operator()(const Labels& labels) const {
  std::size_t seed = 0;
  for (auto& label : labels) {
    hash_combine(&seed, label.first, label.second);
  }

  return seed;
}

}  // namespace detail

}  // namespace prometheus
