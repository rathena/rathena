#pragma once

#include <locale>

class RAIILocale {
 public:
  explicit RAIILocale(const char* name) {
    std::locale::global(std::locale(name));
  }

  ~RAIILocale() { std::locale::global(savedLocale_); }

  RAIILocale(const RAIILocale&) = delete;
  RAIILocale(RAIILocale&&) = delete;
  RAIILocale& operator=(const RAIILocale&) = delete;
  RAIILocale& operator=(RAIILocale&&) = delete;

 private:
  const std::locale savedLocale_ = std::locale{};
};
