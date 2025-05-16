// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef HTTP_HPP
#define HTTP_HPP

#ifdef WIN32
#include <common/winapi.hpp>
#endif

#include <httplib.h>


typedef httplib::Request Request;
typedef httplib::Response Response;

#define HANDLER_FUNC(x) void x (const Request &req, Response &res)
typedef HANDLER_FUNC((*handler_func));


#endif
