// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef SCRIPT_HTTP_HPP
#define SCRIPT_HTTP_HPP

#include <string>
#include "../map/script.hpp"

// HTTP utility functions for AI service communication
std::string http_post(const std::string& url, const std::string& data);
std::string http_get(const std::string& url);

// Script command function declarations
BUILDIN_FUNC(httppost);
BUILDIN_FUNC(httpget);
BUILDIN_FUNC(npcwalk);
BUILDIN_FUNC(npcwalkid);

#endif // SCRIPT_HTTP_HPP