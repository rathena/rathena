// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CONF_HPP_
#define _CONF_HPP_

#include "../../3rdparty/libconfig/libconfig.h"

#include "cbasetypes.hpp"

int conf_read_file(config_t *config, const char *config_filename);
int config_setting_copy(config_setting_t *parent, const config_setting_t *src);

#endif // _CONF_HPP_
