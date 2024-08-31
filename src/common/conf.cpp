// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "conf.hpp"

#include "showmsg.hpp" // ShowError

int conf_read_file(config_t *config, const char *config_filename)
{
	config_init(config);
	if (!config_read_file(config, config_filename)) {
		ShowError("%s:%d - %s\n", config_error_file(config),
		          config_error_line(config), config_error_text(config));
		config_destroy(config);
		return 1;
	}
	return 0;
}

//
// Functions to copy settings from libconfig/contrib
//
static void config_setting_copy_simple(config_setting_t *parent, const config_setting_t *src);
static void config_setting_copy_elem(config_setting_t *parent, const config_setting_t *src);
static void config_setting_copy_aggregate(config_setting_t *parent, const config_setting_t *src);
int config_setting_copy(config_setting_t *parent, const config_setting_t *src);

void config_setting_copy_simple(config_setting_t *parent, const config_setting_t *src)
{
	if (config_setting_is_aggregate(src)) {
		config_setting_copy_aggregate(parent, src);
	}
	else {
		config_setting_t *set = config_setting_add(parent, config_setting_name(src), config_setting_type(src));

		if (set == nullptr)
			return;

		if (CONFIG_TYPE_INT == config_setting_type(src)) {
			config_setting_set_int(set, config_setting_get_int(src));
			config_setting_set_format(set, src->format);
		} else if (CONFIG_TYPE_INT64 == config_setting_type(src)) {
			config_setting_set_int64(set, config_setting_get_int64(src));
			config_setting_set_format(set, src->format);
		} else if (CONFIG_TYPE_FLOAT == config_setting_type(src)) {
			config_setting_set_float(set, config_setting_get_float(src));
		} else if (CONFIG_TYPE_STRING == config_setting_type(src)) {
			config_setting_set_string(set, config_setting_get_string(src));
		} else if (CONFIG_TYPE_BOOL == config_setting_type(src)) {
			config_setting_set_bool(set, config_setting_get_bool(src));
		}
	}
}

void config_setting_copy_elem(config_setting_t *parent, const config_setting_t *src)
{
	config_setting_t *set = nullptr;

	if (config_setting_is_aggregate(src))
		config_setting_copy_aggregate(parent, src);
	else if (CONFIG_TYPE_INT == config_setting_type(src)) {
		set = config_setting_set_int_elem(parent, -1, config_setting_get_int(src));
		config_setting_set_format(set, src->format);
	} else if (CONFIG_TYPE_INT64 == config_setting_type(src)) {
		set = config_setting_set_int64_elem(parent, -1, config_setting_get_int64(src));
		config_setting_set_format(set, src->format);   
	} else if (CONFIG_TYPE_FLOAT == config_setting_type(src)) {
		config_setting_set_float_elem(parent, -1, config_setting_get_float(src));
	} else if (CONFIG_TYPE_STRING == config_setting_type(src)) {
		config_setting_set_string_elem(parent, -1, config_setting_get_string(src));
	} else if (CONFIG_TYPE_BOOL == config_setting_type(src)) {
		config_setting_set_bool_elem(parent, -1, config_setting_get_bool(src));
	}
}

void config_setting_copy_aggregate(config_setting_t *parent, const config_setting_t *src)
{
	config_setting_t *newAgg;
	int i, n;

	newAgg = config_setting_add(parent, config_setting_name(src), config_setting_type(src));

	if (newAgg == nullptr)
		return;

	n = config_setting_length(src);
	
	for (i = 0; i < n; i++) {
		if (config_setting_is_group(src)) {
			config_setting_copy_simple(newAgg, config_setting_get_elem(src, i));            
		} else {
			config_setting_copy_elem(newAgg, config_setting_get_elem(src, i));
		}
	}
}

int config_setting_copy(config_setting_t *parent, const config_setting_t *src)
{
	if (!config_setting_is_group(parent) && !config_setting_is_list(parent))
		return CONFIG_FALSE;

	if (config_setting_is_aggregate(src)) {
		config_setting_copy_aggregate(parent, src);
	} else {
		config_setting_copy_simple(parent, src);
	}
	return CONFIG_TRUE;
}
