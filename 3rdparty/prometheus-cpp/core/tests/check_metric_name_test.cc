#include <gtest/gtest.h>

#include "prometheus/check_names.h"

namespace prometheus {
namespace {

TEST(CheckMetricNameTest, empty_metric_name) {
  EXPECT_FALSE(CheckMetricName(""));
}
TEST(CheckMetricNameTest, good_metric_name) {
  EXPECT_TRUE(CheckMetricName("prometheus_notifications_total"));
}
TEST(CheckMetricNameTest, numeric_metric_name) {
  EXPECT_FALSE(CheckMetricName("2unlimited"));
}
TEST(CheckMetricNameTest, reserved_metric_name) {
  EXPECT_FALSE(CheckMetricName("__some_reserved_metric"));
}
TEST(CheckMetricNameTest, malformed_metric_name) {
  EXPECT_FALSE(CheckMetricName("fa mi ly with space in name or |"));
}

}  // namespace
}  // namespace prometheus
