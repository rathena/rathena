#include "prometheus/histogram.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace prometheus {

namespace {

template <class ForwardIterator>
bool is_strict_sorted(ForwardIterator first, ForwardIterator last) {
  return std::adjacent_find(first, last,
                            std::greater_equal<typename std::iterator_traits<
                                ForwardIterator>::value_type>()) == last;
}

}  // namespace

Histogram::Histogram(const BucketBoundaries& buckets)
    : bucket_boundaries_{buckets}, bucket_counts_{buckets.size() + 1} {
  if (!is_strict_sorted(begin(bucket_boundaries_), end(bucket_boundaries_))) {
    throw std::invalid_argument("Bucket Boundaries must be strictly sorted");
  }
}

Histogram::Histogram(BucketBoundaries&& buckets)
    : bucket_boundaries_{std::move(buckets)},
      bucket_counts_{bucket_boundaries_.size() + 1} {
  if (!is_strict_sorted(begin(bucket_boundaries_), end(bucket_boundaries_))) {
    throw std::invalid_argument("Bucket Boundaries must be strictly sorted");
  }
}

void Histogram::Observe(const double value) {
  const auto bucket_index = static_cast<std::size_t>(
      std::distance(bucket_boundaries_.begin(),
                    std::lower_bound(bucket_boundaries_.begin(),
                                     bucket_boundaries_.end(), value)));

  std::lock_guard<std::mutex> lock(mutex_);
  sum_.Increment(value);
  bucket_counts_[bucket_index].Increment();
}

void Histogram::ObserveMultiple(const std::vector<double>& bucket_increments,
                                const double sum_of_values) {
  if (bucket_increments.size() != bucket_counts_.size()) {
    throw std::length_error(
        "The size of bucket_increments was not equal to"
        "the number of buckets in the histogram.");
  }

  std::lock_guard<std::mutex> lock(mutex_);
  sum_.Increment(sum_of_values);

  for (std::size_t i{0}; i < bucket_counts_.size(); ++i) {
    bucket_counts_[i].Increment(bucket_increments[i]);
  }
}

void Histogram::Reset() {
  std::lock_guard<std::mutex> lock(mutex_);
  for (std::size_t i = 0; i < bucket_counts_.size(); ++i) {
    bucket_counts_[i].Reset();
  }
  sum_.Set(0);
}

ClientMetric Histogram::Collect() const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto metric = ClientMetric{};

  auto cumulative_count = 0ULL;
  metric.histogram.bucket.reserve(bucket_counts_.size());
  for (std::size_t i{0}; i < bucket_counts_.size(); ++i) {
    cumulative_count += bucket_counts_[i].Value();
    auto bucket = ClientMetric::Bucket{};
    bucket.cumulative_count = cumulative_count;
    bucket.upper_bound = (i == bucket_boundaries_.size()
                              ? std::numeric_limits<double>::infinity()
                              : bucket_boundaries_[i]);
    metric.histogram.bucket.push_back(std::move(bucket));
  }
  metric.histogram.sample_count = cumulative_count;
  metric.histogram.sample_sum = sum_.Value();

  return metric;
}

}  // namespace prometheus
