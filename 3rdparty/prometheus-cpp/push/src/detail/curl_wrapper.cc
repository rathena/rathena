#include "curl_wrapper.h"

#include <stdexcept>

namespace prometheus {
namespace detail {

static const char CONTENT_TYPE[] =
    "Content-Type: text/plain; version=0.0.4; charset=utf-8";

CurlWrapper::CurlWrapper(std::function<void(CURL*)> presetupCurl)
    : presetupCurl_(presetupCurl) {
  /* In windows, this will init the winsock stuff */
  auto error = curl_global_init(CURL_GLOBAL_ALL);
  if (error) {
    throw std::runtime_error("Cannot initialize global curl!");
  }

  curl_ = curl_easy_init();
  if (!curl_) {
    curl_global_cleanup();
    throw std::runtime_error("Cannot initialize easy curl!");
  }

  if (presetupCurl_) {
    presetupCurl_(curl_);  
  }

  optHttpHeader_ = curl_slist_append(nullptr, CONTENT_TYPE);
  if (!optHttpHeader_) {
    throw std::runtime_error("Cannot append the header of the content type");
  }
}

CurlWrapper::~CurlWrapper() {
  curl_slist_free_all(optHttpHeader_);
  curl_easy_cleanup(curl_);
  curl_global_cleanup();
}

int CurlWrapper::performHttpRequest(HttpMethod method, const std::string& uri,
                                    const std::string& body) {
  std::lock_guard<std::mutex> l(mutex_);

  curl_easy_reset(curl_);
  curl_easy_setopt(curl_, CURLOPT_URL, uri.c_str());

  curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, optHttpHeader_);

  if (presetupCurl_) {
    presetupCurl_(curl_);  
  }

  if (!body.empty()) {
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.size());
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.data());
  } else {
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, 0L);
  }

  switch (method) {
    case HttpMethod::Post:
      curl_easy_setopt(curl_, CURLOPT_POST, 1L);
      break;

    case HttpMethod::Put:
      curl_easy_setopt(curl_, CURLOPT_NOBODY, 0L);
      curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
      break;

    case HttpMethod::Delete:
      curl_easy_setopt(curl_, CURLOPT_HTTPGET, 0L);
      curl_easy_setopt(curl_, CURLOPT_NOBODY, 0L);
      curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
      break;
  }

  auto curl_error = curl_easy_perform(curl_);

  long response_code;
  curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);

  if (curl_error != CURLE_OK) {
    return -curl_error;
  }

  return response_code;
}

bool CurlWrapper::addHttpHeader(const std::string& header) {
  std::lock_guard<std::mutex> lock{mutex_};
  auto updated_header = curl_slist_append(optHttpHeader_, header.c_str());
  if (!updated_header) {
    return false;
  }

  optHttpHeader_ = updated_header;
  return true;
}

}  // namespace detail
}  // namespace prometheus
