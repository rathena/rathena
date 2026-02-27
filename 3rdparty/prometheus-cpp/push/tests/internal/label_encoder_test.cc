#include "detail/label_encoder.h"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

namespace prometheus {
namespace {

class LabelEncoderTest : public testing::Test {
 protected:
  std::string Encode(const Label& label) {
    std::stringstream ss;
    detail::encodeLabel(ss, label);
    return ss.str();
  }
};

// test cases taken from https://github.com/prometheus/pushgateway#url

TEST_F(LabelEncoderTest, regular) {
  EXPECT_EQ("/foo/bar", Encode(Label{"foo", "bar"}));
}

TEST_F(LabelEncoderTest, empty) {
  EXPECT_EQ("/first_label@base64/=", Encode(Label{"first_label", ""}));
}

TEST_F(LabelEncoderTest, path) {
  EXPECT_EQ("/path@base64/L3Zhci90bXA=", Encode(Label{"path", "/var/tmp"}));
}

TEST_F(LabelEncoderTest, unicode) {
  const char unicodeText[] =
      "\xce\xa0\xcf\x81\xce\xbf\xce\xbc\xce\xb7\xce\xb8\xce\xb5\xcf\x8d\xcf"
      "\x82";  // Προμηθεύς
  EXPECT_EQ("/name@base64/zqDPgc6_zrzOt864zrXPjc-C",
            Encode(Label{"name", unicodeText}));
}

}  // namespace
}  // namespace prometheus
