// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CONF_HPP_
#define _CONF_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include "cbasetypes.h"
#include "../../3rdparty/libconfig/libconfig.h"

int conf_read_file(config_t *config, const char *config_filename);
int config_setting_copy(config_setting_t *parent, const config_setting_t *src);

#ifdef __cplusplus
}
#endif

#endif // _CONF_HPP_
