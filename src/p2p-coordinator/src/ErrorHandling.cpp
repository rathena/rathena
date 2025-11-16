#include "ErrorHandling.h"
#include <stdexcept>

void ErrorHandling::throwIf(bool condition, const std::string& msg) {
    if (condition) {
        throw CoordinatorException(msg);
    }
}