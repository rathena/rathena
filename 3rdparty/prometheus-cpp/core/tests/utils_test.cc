#include "prometheus/detail/utils.h"

#include <gtest/gtest.h>

namespace prometheus {

namespace {

class UtilsTest : public testing::Test {
 public:
  detail::LabelHasher hasher;
};

TEST_F(UtilsTest, hash_labels_1) {
  Labels labels{{"key1", "value1"}, {"key2", "value2"}};
  EXPECT_EQ(hasher(labels), hasher(labels));
}

TEST_F(UtilsTest, hash_labels_2) {
  Labels labels1{{"aa", "bb"}};
  Labels labels2{{"a", "abb"}};
  EXPECT_NE(hasher(labels1), hasher(labels2));
}

TEST_F(UtilsTest, hash_label_3) {
  Labels labels1{{"a", "a"}};
  Labels labels2{{"aa", ""}};
  EXPECT_NE(hasher(labels1), hasher(labels2));
}

}  // namespace

}  // namespace prometheus
