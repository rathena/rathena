// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#ifndef WEB_UTILS_HPP
#define WEB_UTILS_HPP

#include <string>
#include <nlohmann/json_fwd.hpp>

bool mergeData(nlohmann::json &orig, const nlohmann::json &patch, bool merge_null);

#endif
