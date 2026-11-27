#include "label_encoder.h"

#include <algorithm>
#include <ostream>

#include "prometheus/detail/base64.h"

namespace prometheus {
namespace detail {

namespace {
// Does this character need encoding like in RFC 3986 section 2.3?
bool needsEncoding(char c) {
  if (c >= 'a' && c <= 'z') {
    return false;
  }
  if (c >= 'A' && c <= 'Z') {
    return false;
  }
  if (c >= '0' && c <= '9') {
    return false;
  }
  if (c == '-' || c == '.' || c == '_' || c == '~') {
    return false;
  }
  return true;
}
}  // namespace

void encodeLabel(std::ostream& os, const Label& label) {
  if (label.second.empty()) {
    os << "/" << label.first << "@base64/=";
  } else if (std::any_of(label.second.begin(), label.second.end(),
                         needsEncoding)) {
    os << "/" << label.first << "@base64/"
       << detail::base64url_encode(label.second);
  } else {
    os << "/" << label.first << "/" << label.second;
  }
}
}  // namespace detail
}  // namespace prometheus
