// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef METRICS_HPP
#define METRICS_HPP

#include <cstdint>
#include <string>
#include <string_view>

namespace rathena::metrics {

void counter_inc( std::string_view name, uint64_t amount = 1, std::string_view label_key = {}, std::string_view label_value = {} );
void gauge_set( std::string_view name, double value, std::string_view label_key = {}, std::string_view label_value = {} );
void observe_duration_ns( std::string_view name, uint64_t duration_ns, std::string_view label_key = {}, std::string_view label_value = {} );

std::string export_prometheus();
bool write_prometheus_to_file( const char* path );

}

#endif
