// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../../src/map/async_file_log.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
using Microseconds = std::chrono::duration<double, std::micro>;
constexpr int records = 10000;

void require( bool result, const char* message ) {
	if( !result )
		throw std::runtime_error( message );
}

void run( const std::filesystem::path& path, bool async ) {
	AsyncFileLog log;
	const auto filename = path.string();
	require( !log.append( "unexpected\n" ), "stopped writer accepted a record" );
	if( async )
		log.start( filename.c_str() );

	std::vector<double> latency;
	latency.reserve( records );
	const auto start = Clock::now();
	for( int i = 0; i < records; ++i ) {
		char line[128];
		std::snprintf( line, sizeof( line ), "2026-11-12 - %d\tM\t501,1,0,0,0,0,0,prontera,'0',0,0\n", i );
		const auto submitted = Clock::now();
		if( !log.append( line ) ) {
			FILE* file = std::fopen( filename.c_str(), "a" );
			require( file != nullptr, "cannot open sync log" );
			const bool written = std::fputs( line, file ) != EOF;
			const bool closed = std::fclose( file ) == 0;
			require( written && closed, "cannot append to sync log" );
		}
		latency.push_back( Microseconds( Clock::now() - submitted ).count() );
	}
	require( log.stop() == 0, "async writer failed" );
	const double total_ms = Microseconds( Clock::now() - start ).count() / 1000;

	std::ifstream file( path );
	std::string line;
	for( int i = 0; i < records; ++i ) {
		require( static_cast<bool>( std::getline( file, line ) ), "missing record" );
		require( line == "2026-11-12 - " + std::to_string( i ) + "\tM\t501,1,0,0,0,0,0,prontera,'0',0,0", "changed or reordered record" );
	}
	require( !std::getline( file, line ), "extra record" );
	file.close();
	std::filesystem::remove( path );

	std::sort( latency.begin(), latency.end() );
	std::cout << ( async ? "async" : "sync" ) << " count=" << records << " producer_us_p50=" << latency[records / 2]
		<< " producer_us_p99=" << latency[records * 99 / 100] << " total_ms=" << total_ms << '\n';
}
}

int main( int argc, char** argv ) {
	if( argc != 2 ) {
		std::cerr << "Usage: async_file_log <new scratch directory>\n";
		return 2;
	}

	try {
		const std::filesystem::path dir( argv[1] );
		require( std::filesystem::create_directory( dir ), "scratch directory must be new" );
		AsyncFileLog log;
		log.start( ( dir / "missing" / "item.log" ).string().c_str() );
		require( log.append( "test\n" ), "writer refused a record" );
		require( log.stop() == 1, "open failure was not counted" );

		for( bool async : { false, true, true, false } )
			run( dir / "item.log", async );
		std::filesystem::remove( dir );
	} catch( const std::exception& e ) {
		std::cerr << e.what() << '\n';
		return 1;
	}
}
