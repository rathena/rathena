
#include "prometheus/gateway.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>

#include "detail/curl_wrapper.h"
#include "detail/label_encoder.h"
#include "prometheus/detail/future_std.h"
#include "prometheus/metric_family.h"  // IWYU pragma: keep
#include "prometheus/text_serializer.h"

// IWYU pragma: no_include <system_error>
// IWYU pragma: no_include <cxxabi.h>

namespace prometheus {

namespace {
class SetupAdapter {
 public:
  SetupAdapter(const std::string& username, const std::string& password,
               std::chrono::seconds timeout)
      : timeout_(timeout) {
    if (!username.empty()) {
      auth_ = username + ":" + password;
    }
  }

  void operator()(CURL* curl) {
    if (!auth_.empty()) {
      curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
      curl_easy_setopt(curl, CURLOPT_USERPWD, auth_.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_);
  }

 private:
  std::string auth_;
  std::chrono::seconds timeout_;
};

std::string concatenate(const std::string& host, const std::string& port) {
  std::stringstream ss;
  ss << host << ":" << port;
  return ss.str();
}
}  // namespace

Gateway::Gateway(const std::string& host, const std::string& port,
                 const std::string& jobname, const Labels& labels,
                 const std::string& username, const std::string& password,
                 std::chrono::seconds timeout)
    : Gateway(host, port, SetupAdapter{username, password, timeout}, jobname,
              labels) {}

Gateway::Gateway(const std::string& host, const std::string& port,
                 std::function<void(CURL*)> presetupCurl,
                 const std::string& jobname, const Labels& labels)
    : Gateway(concatenate(host, port), presetupCurl, jobname, labels) {}

Gateway::Gateway(const std::string& url,
                 std::function<void(CURL*)> presetupCurl,
                 const std::string& jobname, const Labels& labels) {
  curlWrapper_ = detail::make_unique<detail::CurlWrapper>(presetupCurl);

  std::stringstream jobUriStream;
  jobUriStream << url;
  if (!url.empty() && url.back() != '/') {
    jobUriStream << "/";
  }
  jobUriStream << "metrics";
  detail::encodeLabel(jobUriStream, {"job", jobname});
  jobUri_ = jobUriStream.str();

  std::stringstream labelStream;
  for (auto& label : labels) {
    detail::encodeLabel(labelStream, label);
  }
  labels_ = labelStream.str();
}

Gateway::~Gateway() = default;

Labels Gateway::GetInstanceLabel(std::string hostname) {
  if (hostname.empty()) {
    return Labels{};
  }
  return Labels{{"instance", hostname}};
}

void Gateway::RegisterCollectable(const std::weak_ptr<Collectable>& collectable,
                                  const Labels* labels) {
  std::stringstream ss;

  if (labels) {
    for (auto& label : *labels) {
      detail::encodeLabel(ss, label);
    }
  }

  std::lock_guard<std::mutex> lock{mutex_};
  CleanupStalePointers(collectables_);
  collectables_.emplace_back(collectable, ss.str());
}

std::string Gateway::getUri(const CollectableEntry& collectable) const {
  std::stringstream uri;
  uri << jobUri_ << labels_ << collectable.second;

  return uri.str();
}

int Gateway::Push() { return push(detail::HttpMethod::Post); }

int Gateway::PushAdd() { return push(detail::HttpMethod::Put); }

int Gateway::push(detail::HttpMethod method) {
  const auto serializer = TextSerializer{};

  std::lock_guard<std::mutex> lock{mutex_};
  for (auto& wcollectable : collectables_) {
    auto collectable = wcollectable.first.lock();
    if (!collectable) {
      continue;
    }

    auto metrics = collectable->Collect();
    auto body = serializer.Serialize(metrics);
    auto uri = getUri(wcollectable);
    auto status_code = curlWrapper_->performHttpRequest(method, uri, body);

    if (status_code < 100 || status_code >= 400) {
      return status_code;
    }
  }

  return 200;
}

std::future<int> Gateway::AsyncPush() {
  return async_push(detail::HttpMethod::Post);
}

std::future<int> Gateway::AsyncPushAdd() {
  return async_push(detail::HttpMethod::Put);
}

std::future<int> Gateway::async_push(detail::HttpMethod method) {
  const auto serializer = TextSerializer{};
  std::vector<std::future<int>> futures;

  std::lock_guard<std::mutex> lock{mutex_};
  for (auto& wcollectable : collectables_) {
    auto collectable = wcollectable.first.lock();
    if (!collectable) {
      continue;
    }

    auto metrics = collectable->Collect();
    auto body = std::make_shared<std::string>(serializer.Serialize(metrics));
    auto uri = getUri(wcollectable);

    futures.push_back(std::async(std::launch::async, [method, uri, body, this] {
      return curlWrapper_->performHttpRequest(method, uri, *body);
    }));
  }

  const auto reduceFutures = [](std::vector<std::future<int>> lfutures) {
    auto final_status_code = 200;

    for (auto& future : lfutures) {
      auto status_code = future.get();

      if (status_code < 100 || status_code >= 400) {
        final_status_code = status_code;
      }
    }

    return final_status_code;
  };

  return std::async(std::launch::async, reduceFutures, std::move(futures));
}

int Gateway::Delete() {
  return curlWrapper_->performHttpRequest(detail::HttpMethod::Delete, jobUri_);
}

std::future<int> Gateway::AsyncDelete() {
  return std::async(std::launch::async, [&] { return Delete(); });
}

int Gateway::DeleteForInstance() {
  return curlWrapper_->performHttpRequest(detail::HttpMethod::Delete,
                                          jobUri_ + labels_);
}

std::future<int> Gateway::AsyncDeleteForInstance() {
  return std::async(std::launch::async, [&] { return DeleteForInstance(); });
}

void Gateway::CleanupStalePointers(
    std::vector<CollectableEntry>& collectables) {
  collectables.erase(
      std::remove_if(std::begin(collectables), std::end(collectables),
                     [](const CollectableEntry& candidate) {
                       return candidate.first.expired();
                     }),
      std::end(collectables));
}

bool Gateway::AddHttpHeader(const std::string& header) {
  return curlWrapper_->addHttpHeader(header);
}

}  // namespace prometheus
