// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "metrics.hpp"

#include <algorithm>
#include <cstdio>
#include <mutex>
#include <unordered_map>
#include <vector>

#ifdef RA_ENABLE_PROMETHEUS_CPP
#include <sstream>

#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <prometheus/text_serializer.h>
#endif

namespace rathena::metrics {
namespace {
	struct MetricKey {
		std::string name;
		std::string label_key;
		std::string label_value;

		bool operator==( const MetricKey& other ) const {
			return this->name == other.name && this->label_key == other.label_key && this->label_value == other.label_value;
		}
	};

	struct MetricKeyHash {
		size_t operator()( const MetricKey& key ) const {
			size_t hash = std::hash<std::string>{}( key.name );
			hash ^= std::hash<std::string>{}( key.label_key ) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			hash ^= std::hash<std::string>{}( key.label_value ) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		}
	};

	std::string sanitize_metric_name( std::string_view name ){
		std::string out;
		out.reserve( name.size() + 8 );

		for( char ch : name ){
			if( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == ':' ){
				out.push_back( ch );
			}else{
				out.push_back( '_' );
			}
		}

		if( out.empty() || ( out[0] >= '0' && out[0] <= '9' ) ){
			out.insert( 0, "rathena_" );
		}

		return out;
	}

	std::string sanitize_label_key( std::string_view key ){
		if( key.empty() ){
			return {};
		}

		std::string out;
		out.reserve( key.size() + 1 );

		for( char ch : key ){
			if( (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' ){
				out.push_back( ch );
			}else{
				out.push_back( '_' );
			}
		}

		if( out.empty() || ( out[0] >= '0' && out[0] <= '9' ) ){
			out.insert( out.begin(), '_' );
		}

		return out;
	}

	std::string sanitize_label_value( std::string_view value ){
		std::string out;
		out.reserve( value.size() );

		for( unsigned char ch : value ){
			// Keep plain printable ASCII only to avoid invalid UTF-8 in Prometheus text output.
			if( ch >= 0x20 && ch <= 0x7E ){
				out.push_back( static_cast<char>( ch ) );
			}else{
				out.push_back( '_' );
			}
		}

		return out;
	}

	MetricKey make_key( std::string_view name, std::string_view label_key, std::string_view label_value ){
		MetricKey key;
		key.name = sanitize_metric_name( name );
		key.label_key = sanitize_label_key( label_key );
		key.label_value = key.label_key.empty() ? "" : sanitize_label_value( label_value );
		return key;
	}
}

#ifdef RA_ENABLE_PROMETHEUS_CPP

namespace {
	struct MetricFamilyKey {
		std::string name;
		std::string label_key;

		bool operator==( const MetricFamilyKey& other ) const {
			return this->name == other.name && this->label_key == other.label_key;
		}
	};

	struct MetricFamilyKeyHash {
		size_t operator()( const MetricFamilyKey& key ) const {
			size_t hash = std::hash<std::string>{}( key.name );
			hash ^= std::hash<std::string>{}( key.label_key ) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
			return hash;
		}
	};

	std::mutex g_metrics_mutex;
	std::shared_ptr<prometheus::Registry> g_registry = std::make_shared<prometheus::Registry>();

	std::unordered_map<MetricFamilyKey, prometheus::Family<prometheus::Counter>*, MetricFamilyKeyHash> g_counter_families;
	std::unordered_map<MetricKey, prometheus::Counter*, MetricKeyHash> g_counter_metrics;

	std::unordered_map<MetricFamilyKey, prometheus::Family<prometheus::Gauge>*, MetricFamilyKeyHash> g_gauge_families;
	std::unordered_map<MetricKey, prometheus::Gauge*, MetricKeyHash> g_gauge_metrics;

	std::unordered_map<MetricKey, double, MetricKeyHash> g_duration_max_seconds;

	prometheus::Labels labels_for( const MetricKey& key ){
		if( key.label_key.empty() ){
			return {};
		}

		return { { key.label_key, key.label_value } };
	}

	prometheus::Counter& get_or_create_counter( const MetricKey& key ){
		auto metric_it = g_counter_metrics.find( key );
		if( metric_it != g_counter_metrics.end() ){
			return *metric_it->second;
		}

		MetricFamilyKey family_key{ key.name, key.label_key };
		auto family_it = g_counter_families.find( family_key );
		prometheus::Family<prometheus::Counter>* family = nullptr;
		if( family_it == g_counter_families.end() ){
			family = &prometheus::BuildCounter()
				.Name( key.name )
				.Help( "rAthena counter metric" )
				.Register( *g_registry );
			g_counter_families.emplace( std::move( family_key ), family );
		}else{
			family = family_it->second;
		}

		auto& metric = family->Add( labels_for( key ) );
		g_counter_metrics.emplace( key, &metric );
		return metric;
	}

	prometheus::Gauge& get_or_create_gauge( const MetricKey& key ){
		auto metric_it = g_gauge_metrics.find( key );
		if( metric_it != g_gauge_metrics.end() ){
			return *metric_it->second;
		}

		MetricFamilyKey family_key{ key.name, key.label_key };
		auto family_it = g_gauge_families.find( family_key );
		prometheus::Family<prometheus::Gauge>* family = nullptr;
		if( family_it == g_gauge_families.end() ){
			family = &prometheus::BuildGauge()
				.Name( key.name )
				.Help( "rAthena gauge metric" )
				.Register( *g_registry );
			g_gauge_families.emplace( std::move( family_key ), family );
		}else{
			family = family_it->second;
		}

		auto& metric = family->Add( labels_for( key ) );
		g_gauge_metrics.emplace( key, &metric );
		return metric;
	}
}

void counter_inc( std::string_view name, uint64_t amount, std::string_view label_key, std::string_view label_value ){
	MetricKey key = make_key( name, label_key, label_value );

	std::lock_guard<std::mutex> lock( g_metrics_mutex );
	get_or_create_counter( key ).Increment( static_cast<double>( amount ) );
}

void gauge_set( std::string_view name, double value, std::string_view label_key, std::string_view label_value ){
	MetricKey key = make_key( name, label_key, label_value );

	std::lock_guard<std::mutex> lock( g_metrics_mutex );
	get_or_create_gauge( key ).Set( value );
}

void observe_duration_ns( std::string_view name, uint64_t duration_ns, std::string_view label_key, std::string_view label_value ){
	MetricKey base_key = make_key( name, label_key, label_value );
	const double seconds = static_cast<double>( duration_ns ) / 1000000000.0;

	std::lock_guard<std::mutex> lock( g_metrics_mutex );

	MetricKey count_key = base_key;
	count_key.name += "_seconds_count";
	get_or_create_counter( count_key ).Increment();

	MetricKey sum_key = base_key;
	sum_key.name += "_seconds_sum";
	get_or_create_counter( sum_key ).Increment( seconds );

	MetricKey max_key = base_key;
	max_key.name += "_seconds_max";
	double& current_max = g_duration_max_seconds[max_key];
	if( seconds > current_max ){
		current_max = seconds;
		get_or_create_gauge( max_key ).Set( current_max );
	}
}

std::string export_prometheus(){
	std::lock_guard<std::mutex> lock( g_metrics_mutex );

	prometheus::TextSerializer serializer;
	std::ostringstream output;
	serializer.Serialize( output, g_registry->Collect() );
	return output.str();
}

bool write_prometheus_to_file( const char* path ){
	if( path == nullptr || path[0] == '\0' ){
		return false;
	}

	std::string text = export_prometheus();

	FILE* fp = fopen( path, "w" );
	if( fp == nullptr ){
		return false;
	}

	const bool ok = fwrite( text.data(), 1, text.size(), fp ) == text.size();
	fclose( fp );
	return ok;
}

#else

namespace {
	struct DurationStats {
		uint64_t count = 0;
		double sum_seconds = 0;
		double max_seconds = 0;
	};

	std::mutex g_metrics_mutex;
	std::unordered_map<MetricKey, uint64_t, MetricKeyHash> g_counters;
	std::unordered_map<MetricKey, double, MetricKeyHash> g_gauges;
	std::unordered_map<MetricKey, DurationStats, MetricKeyHash> g_durations;

	std::string escape_label_value( std::string_view value ){
		std::string out;
		out.reserve( value.size() + 8 );

		for( char ch : value ){
			switch( ch ){
				case '\\':
					out += "\\\\";
					break;
				case '"':
					out += "\\\"";
					break;
				case '\n':
					out += "\\n";
					break;
				default:
					out.push_back( ch );
					break;
			}
		}

		return out;
	}

	std::string format_labels( const MetricKey& key ){
		if( key.label_key.empty() ){
			return {};
		}

		return "{" + key.label_key + "=\"" + escape_label_value( key.label_value ) + "\"}";
	}
}

void counter_inc( std::string_view name, uint64_t amount, std::string_view label_key, std::string_view label_value ){
	MetricKey key = make_key( name, label_key, label_value );

	std::lock_guard<std::mutex> lock( g_metrics_mutex );
	g_counters[key] += amount;
}

void gauge_set( std::string_view name, double value, std::string_view label_key, std::string_view label_value ){
	MetricKey key = make_key( name, label_key, label_value );

	std::lock_guard<std::mutex> lock( g_metrics_mutex );
	g_gauges[key] = value;
}

void observe_duration_ns( std::string_view name, uint64_t duration_ns, std::string_view label_key, std::string_view label_value ){
	MetricKey key = make_key( name, label_key, label_value );

	std::lock_guard<std::mutex> lock( g_metrics_mutex );
	DurationStats& stats = g_durations[key];
	const double seconds = static_cast<double>(duration_ns) / 1000000000.0;
	stats.count++;
	stats.sum_seconds += seconds;
	stats.max_seconds = std::max( stats.max_seconds, seconds );
}

std::string export_prometheus(){
	std::vector<std::pair<MetricKey, uint64_t>> counters;
	std::vector<std::pair<MetricKey, double>> gauges;
	std::vector<std::pair<MetricKey, DurationStats>> durations;

	{
		std::lock_guard<std::mutex> lock( g_metrics_mutex );
		counters.reserve( g_counters.size() );
		for( const auto& it : g_counters ){
			counters.push_back( it );
		}

		gauges.reserve( g_gauges.size() );
		for( const auto& it : g_gauges ){
			gauges.push_back( it );
		}

		durations.reserve( g_durations.size() );
		for( const auto& it : g_durations ){
			durations.push_back( it );
		}
	}

	auto less_metric = []( const auto& a, const auto& b ){
		if( a.first.name != b.first.name ){
			return a.first.name < b.first.name;
		}
		if( a.first.label_key != b.first.label_key ){
			return a.first.label_key < b.first.label_key;
		}
		return a.first.label_value < b.first.label_value;
	};

	std::sort( counters.begin(), counters.end(), less_metric );
	std::sort( gauges.begin(), gauges.end(), less_metric );
	std::sort( durations.begin(), durations.end(), less_metric );

	std::string out;
	out.reserve( 4096 );

	std::string previous_metric;
	for( const auto& it : counters ){
		const std::string metric_name = it.first.name + "_total";
		if( previous_metric != metric_name ){
			out += "# TYPE " + metric_name + " counter\n";
			previous_metric = metric_name;
		}

		out += metric_name + format_labels( it.first ) + " " + std::to_string( it.second ) + "\n";
	}

	previous_metric.clear();
	for( const auto& it : gauges ){
		if( previous_metric != it.first.name ){
			out += "# TYPE " + it.first.name + " gauge\n";
			previous_metric = it.first.name;
		}

		out += it.first.name + format_labels( it.first ) + " " + std::to_string( it.second ) + "\n";
	}

	previous_metric.clear();
	for( const auto& it : durations ){
		const std::string metric_name = it.first.name + "_seconds";
		if( previous_metric != metric_name ){
			out += "# TYPE " + metric_name + " summary\n";
			previous_metric = metric_name;
		}

		const std::string labels = format_labels( it.first );
		out += metric_name + "_count" + labels + " " + std::to_string( it.second.count ) + "\n";
		out += metric_name + "_sum" + labels + " " + std::to_string( it.second.sum_seconds ) + "\n";
		out += metric_name + "_max" + labels + " " + std::to_string( it.second.max_seconds ) + "\n";
	}

	return out;
}

bool write_prometheus_to_file( const char* path ){
	if( path == nullptr || path[0] == '\0' ){
		return false;
	}

	std::string text = export_prometheus();

	FILE* fp = fopen( path, "w" );
	if( fp == nullptr ){
		return false;
	}

	const bool ok = fwrite( text.data(), 1, text.size(), fp ) == text.size();
	fclose( fp );

	return ok;
}

#endif
}
