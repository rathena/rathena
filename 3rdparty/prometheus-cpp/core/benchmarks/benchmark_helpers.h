#pragma once

#include <cstddef>
#include <string>

#include "prometheus/labels.h"

std::string GenerateRandomString(std::size_t length);
prometheus::Labels GenerateRandomLabels(std::size_t number_of_labels);
