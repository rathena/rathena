#include <curl/curl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "prometheus/counter.h"
#include "prometheus/detail/future_std.h"
#include "prometheus/exposer.h"
#include "prometheus/family.h"
#include "prometheus/registry.h"

namespace prometheus {
namespace {

using namespace testing;

class IntegrationTest : public testing::Test {
 public:
  void SetUp() override {
    exposer_ = detail::make_unique<Exposer>("127.0.0.1:0");
    auto ports = exposer_->GetListeningPorts();
    base_url_ = std::string("http://127.0.0.1:") + std::to_string(ports.at(0));
  }

  struct Response {
    long code = 0;
    std::string body;
    std::string contentType;
  };

  std::function<void(CURL*)> fetchPrePerform_;

  Response FetchMetrics(const std::string& metrics_path) const {
    auto curl = std::shared_ptr<CURL>(curl_easy_init(), curl_easy_cleanup);
    if (!curl) {
      throw std::runtime_error("failed to initialize libcurl");
    }

    const auto url = base_url_ + metrics_path;
    Response response;

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);

    if (fetchPrePerform_) {
      fetchPrePerform_(curl.get());
    }

    CURLcode curl_error = curl_easy_perform(curl.get());
    if (curl_error != CURLE_OK) {
      throw std::runtime_error("failed to perform HTTP request");
    }

    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.code);

    char* ct = nullptr;
    curl_easy_getinfo(curl.get(), CURLINFO_CONTENT_TYPE, &ct);
    if (ct) {
      response.contentType = ct;
    }

    return response;
  }

  std::shared_ptr<Registry> RegisterSomeCounter(const std::string& name,
                                                const std::string& path) const {
    auto registry = std::make_shared<Registry>();

    BuildCounter().Name(name).Register(*registry).Add({}).Increment();

    exposer_->RegisterCollectable(registry, path);

    return registry;
  };

  std::unique_ptr<Exposer> exposer_;

  std::string base_url_;
  std::string default_metrics_path_ = "/metrics";

 private:
  static std::size_t WriteCallback(void* contents, std::size_t size,
                                   std::size_t nmemb, void* userp) {
    auto response = reinterpret_cast<std::string*>(userp);

    std::size_t realsize = size * nmemb;
    response->append(reinterpret_cast<const char*>(contents), realsize);
    return realsize;
  }
};

TEST_F(IntegrationTest, doesNotExposeAnythingOnDefaultPath) {
  const auto metrics = FetchMetrics(default_metrics_path_);

  EXPECT_GE(metrics.code, 400);
}

TEST_F(IntegrationTest, exposeSingleCounter) {
  const std::string counter_name = "example_total";
  auto registry = RegisterSomeCounter(counter_name, default_metrics_path_);

  const auto metrics = FetchMetrics(default_metrics_path_);

  ASSERT_EQ(metrics.code, 200);
  EXPECT_THAT(metrics.body, HasSubstr(counter_name));
}

TEST_F(IntegrationTest, exposesCountersOnDifferentUrls) {
  const std::string first_metrics_path = "/first";
  const std::string second_metrics_path = "/second";

  const std::string first_counter_name = "first_total";
  const std::string second_counter_name = "second_total";

  const auto first_registry =
      RegisterSomeCounter(first_counter_name, first_metrics_path);
  const auto second_registry =
      RegisterSomeCounter(second_counter_name, second_metrics_path);

  // all set-up

  const auto first_metrics = FetchMetrics(first_metrics_path);
  const auto second_metrics = FetchMetrics(second_metrics_path);

  // check results

  ASSERT_EQ(first_metrics.code, 200);
  ASSERT_EQ(second_metrics.code, 200);

  EXPECT_THAT(first_metrics.body, HasSubstr(first_counter_name));
  EXPECT_THAT(second_metrics.body, HasSubstr(second_counter_name));

  EXPECT_THAT(first_metrics.body, Not(HasSubstr(second_counter_name)));
  EXPECT_THAT(second_metrics.body, Not(HasSubstr(first_counter_name)));
}

TEST_F(IntegrationTest, unexposeRegistry) {
  const std::string counter_name = "some_counter_total";
  const auto registry =
      RegisterSomeCounter(counter_name, default_metrics_path_);

  exposer_->RemoveCollectable(registry, default_metrics_path_);

  const auto metrics = FetchMetrics(default_metrics_path_);
  ASSERT_EQ(metrics.code, 200);
  EXPECT_THAT(metrics.body, Not(HasSubstr(counter_name)));
}

TEST_F(IntegrationTest, acceptOptionalCompression) {
  const std::string counter_name = "example_total";
  auto registry = RegisterSomeCounter(counter_name, default_metrics_path_);

  fetchPrePerform_ = [](CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
  };
  const auto metrics = FetchMetrics(default_metrics_path_);

  ASSERT_EQ(metrics.code, 200);
  EXPECT_THAT(metrics.body, HasSubstr(counter_name));
}

TEST_F(IntegrationTest, shouldDealWithExpiredCollectables) {
  const std::string first_counter_name = "first_total";
  const std::string second_counter_name = "second_total";

  const auto registry =
      RegisterSomeCounter(first_counter_name, default_metrics_path_);
  auto disappearing_registry =
      RegisterSomeCounter(second_counter_name, default_metrics_path_);

  disappearing_registry.reset();

  // all set-up

  const auto metrics = FetchMetrics(default_metrics_path_);

  // check results

  ASSERT_EQ(metrics.code, 200);

  EXPECT_THAT(metrics.body, HasSubstr(first_counter_name));
  EXPECT_THAT(metrics.body, Not(HasSubstr(second_counter_name)));
}

TEST_F(IntegrationTest, shouldSendBodyAsUtf8) {
  const std::string counter_name = "example_total";
  auto registry = RegisterSomeCounter(counter_name, default_metrics_path_);

  const auto metrics = FetchMetrics(default_metrics_path_);

  // check content type

  ASSERT_EQ(metrics.code, 200);
  EXPECT_THAT(metrics.contentType, HasSubstr("utf-8"));
}

class BasicAuthIntegrationTest : public IntegrationTest {
 public:
  void SetUp() override {
    IntegrationTest::SetUp();

    registry_ = RegisterSomeCounter(counter_name_, default_metrics_path_);

    exposer_->RegisterAuth(
        [&](const std::string& user, const std::string& password) {
          return user == username_ && password == password_;
        },
        "Some Auth Realm", default_metrics_path_);
  }

 protected:
  const std::string counter_name_ = "example_total";
  const std::string username_ = "test_user";
  const std::string password_ = "test_password";

  std::shared_ptr<Registry> registry_;
};

TEST_F(BasicAuthIntegrationTest, shouldPerformProperAuthentication) {
  fetchPrePerform_ = [&](CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username_.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password_.c_str());
  };

  const auto metrics = FetchMetrics(default_metrics_path_);
  ASSERT_EQ(metrics.code, 200);
  EXPECT_THAT(metrics.body, HasSubstr(counter_name_));
}

TEST_F(BasicAuthIntegrationTest, shouldRejectRequestWithoutAuthorization) {
  const auto metrics = FetchMetrics(default_metrics_path_);
  ASSERT_EQ(metrics.code, 401);
}

TEST_F(BasicAuthIntegrationTest, shouldRejectRequestWithWrongUsername) {
  fetchPrePerform_ = [&](CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERNAME, "dummy");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password_.c_str());
  };

  const auto metrics = FetchMetrics(default_metrics_path_);
  ASSERT_EQ(metrics.code, 401);
}

TEST_F(BasicAuthIntegrationTest, shouldRejectRequestWithWrongPassword) {
  fetchPrePerform_ = [&](CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username_.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "dummy");
  };

  const auto metrics = FetchMetrics(default_metrics_path_);
  ASSERT_EQ(metrics.code, 401);
}

#if defined(CURLAUTH_BEARER) && defined(CURLOPT_XOAUTH2_BEARER)
TEST_F(BasicAuthIntegrationTest, shouldRejectWrongAuthorizationMethod) {
  fetchPrePerform_ = [](CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BEARER);
    curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, "test_token");
  };

  const auto metrics = FetchMetrics(default_metrics_path_);
  ASSERT_EQ(metrics.code, 401);
}
#endif

TEST_F(BasicAuthIntegrationTest, shouldRejectMalformedBase64) {
  std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> header(
      curl_slist_append(nullptr, "Authorization: Basic $"),
      curl_slist_free_all);

  fetchPrePerform_ = [&header](CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header.get());
  };

  const auto metrics = FetchMetrics(default_metrics_path_);
  ASSERT_EQ(metrics.code, 401);
}

TEST_F(BasicAuthIntegrationTest, shouldRejectMalformedBasicAuth) {
  std::unique_ptr<curl_slist, decltype(&curl_slist_free_all)> header(
      curl_slist_append(nullptr, "Authorization: Basic YWJj"),
      curl_slist_free_all);

  fetchPrePerform_ = [&header](CURL* curl) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header.get());
  };

  const auto metrics = FetchMetrics(default_metrics_path_);
  ASSERT_EQ(metrics.code, 401);
}

}  // namespace
}  // namespace prometheus
