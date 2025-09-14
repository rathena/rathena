// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef AUTH_HPP
#define AUTH_HPP

#include "http.hpp"

bool isAuthorized(const Request &request, bool checkGuildLeader=false);

#endif
