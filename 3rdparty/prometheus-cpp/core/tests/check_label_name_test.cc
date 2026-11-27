#include <gtest/gtest.h>

#include <string>

#include "prometheus/check_names.h"
#include "prometheus/metric_type.h"

namespace prometheus {
namespace {

class CheckLabelNameTest : public testing::TestWithParam<MetricType> {
 protected:
  bool CheckLabelName(const std::string& name) {
    return ::prometheus::CheckLabelName(name, GetParam());
  }
};

TEST_P(CheckLabelNameTest, empty_label_name) {
  EXPECT_FALSE(CheckLabelName(""));
}
TEST_P(CheckLabelNameTest, invalid_label_name) {
  EXPECT_FALSE(CheckLabelName("log-level"));
}
TEST_P(CheckLabelNameTest, leading_invalid_label_name) {
  EXPECT_FALSE(CheckLabelName("-abcd"));
}
TEST_P(CheckLabelNameTest, trailing_invalid_label_name) {
  EXPECT_FALSE(CheckLabelName("abcd-"));
}
TEST_P(CheckLabelNameTest, good_label_name) {
  EXPECT_TRUE(CheckLabelName("type"));
}
TEST_P(CheckLabelNameTest, numeric_label_name) {
  EXPECT_FALSE(CheckLabelName("2unlimited"));
}
TEST_P(CheckLabelNameTest, reserved_label_name) {
  EXPECT_FALSE(CheckLabelName("__some_reserved_label"));
}
TEST_P(CheckLabelNameTest, reject_le_for_histogram) {
  EXPECT_EQ(GetParam() != MetricType::Histogram, CheckLabelName("le"));
}
TEST_P(CheckLabelNameTest, reject_quantile_for_histogram) {
  EXPECT_EQ(GetParam() != MetricType::Summary, CheckLabelName("quantile"));
}

INSTANTIATE_TEST_SUITE_P(AllMetricTypes, CheckLabelNameTest,
                         testing::Values(MetricType::Counter, MetricType::Gauge,
                                         MetricType::Histogram,
                                         MetricType::Info, MetricType::Summary,
                                         MetricType::Untyped));

}  // namespace
}  // namespace prometheus
