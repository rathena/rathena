#pragma once
#include <exception>
#include <string>

class CoordinatorException : public std::exception {
public:
    explicit CoordinatorException(const std::string& msg) : msg_(msg) {}
    const char* what() const noexcept override { return msg_.c_str(); }
private:
    std::string msg_;
};

namespace ErrorHandling {
    void throwIf(bool condition, const std::string& msg);
}