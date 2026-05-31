#include <curl/curl.h>

#include <functional>
#include <mutex>
#include <string>

#include "prometheus/detail/http_method.h"

namespace prometheus {
namespace detail {

class CurlWrapper {
 public:
  CurlWrapper(std::function<void(CURL*)> presetupCurl);

  CurlWrapper(const CurlWrapper&) = delete;
  CurlWrapper(CurlWrapper&&) = delete;
  CurlWrapper& operator=(const CurlWrapper&) = delete;
  CurlWrapper& operator=(CurlWrapper&&) = delete;

  ~CurlWrapper();

  int performHttpRequest(HttpMethod method, const std::string& uri,
                         const std::string& body = {});
  bool addHttpHeader(const std::string& header);

 private:
  CURL* curl_;
  std::mutex mutex_;
  curl_slist* optHttpHeader_;
  std::function<void(CURL*)> presetupCurl_;
};

}  // namespace detail
}  // namespace prometheus
