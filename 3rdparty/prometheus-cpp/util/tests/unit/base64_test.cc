#include "prometheus/detail/base64.h"

#include <gtest/gtest.h>

#include <string>

namespace prometheus {
namespace {

struct TestVector {
  const std::string decoded;
  const std::string encoded;
};

const TestVector testVector[] = {
    // RFC 3548 examples
    {"\x14\xfb\x9c\x03\xd9\x7e", "FPucA9l+"},
    {"\x14\xfb\x9c\x03\xd9", "FPucA9k="},
    {"\x14\xfb\x9c\x03", "FPucAw=="},

    // RFC 4648 examples
    {"", ""},
    {"f", "Zg=="},
    {"fo", "Zm8="},
    {"foo", "Zm9v"},
    {"foob", "Zm9vYg=="},
    {"fooba", "Zm9vYmE="},
    {"foobar", "Zm9vYmFy"},

    // Wikipedia examples
    {"sure.", "c3VyZS4="},
    {"sure", "c3VyZQ=="},
    {"sur", "c3Vy"},
    {"su", "c3U="},
    {"leasure.", "bGVhc3VyZS4="},
    {"easure.", "ZWFzdXJlLg=="},
    {"asure.", "YXN1cmUu"},
    {"sure.", "c3VyZS4="},
};

using namespace testing;

TEST(Base64Test, encodeTest) {
  for (const auto& test_case : testVector) {
    std::string encoded = detail::base64_encode(test_case.decoded);
    EXPECT_EQ(test_case.encoded, encoded);
  }
}

TEST(Base64Test, encodeUrlTest) {
  const char unicodeText[] =
      "\xce\xa0\xcf\x81\xce\xbf\xce\xbc\xce\xb7\xce\xb8\xce\xb5\xcf\x8d\xcf"
      "\x82";  // Προμηθεύς
  std::string encoded = detail::base64url_encode(unicodeText);
  EXPECT_EQ("zqDPgc6_zrzOt864zrXPjc-C", encoded);
}

TEST(Base64Test, decodeTest) {
  for (const auto& test_case : testVector) {
    std::string decoded = detail::base64_decode(test_case.encoded);
    EXPECT_EQ(test_case.decoded, decoded);
  }
}

TEST(Base64Test, rejectInvalidSymbols) {
  EXPECT_ANY_THROW(detail::base64_decode("...."));
}

TEST(Base64Test, rejectInvalidInputSize) {
  EXPECT_ANY_THROW(detail::base64_decode("ABC"));
}

TEST(Base64Test, rejectInvalidPadding) {
  EXPECT_ANY_THROW(detail::base64_decode("A==="));
}

}  // namespace
}  // namespace prometheus
