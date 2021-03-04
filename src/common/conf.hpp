// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CONF_HPP
#define CONF_HPP

#include "../../3rdparty/libconfig/libconfig.h"

#include "cbasetypes.hpp"

int conf_read_file(config_t *config, const char *config_filename);
int config_setting_copy(config_setting_t *parent, const config_setting_t *src);

#endif /* CONF_HPP */
