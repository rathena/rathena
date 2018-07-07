// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _UTILILITIES_HPP_
#define _UTILILITIES_HPP_

#include <memory>
#include <string>
#include <map>

#include "cbasetypes.hpp"

// Class used to perform time measurement
class cScopeTimer {
	struct sPimpl; //this is to avoid long compilation time
	std::unique_ptr<sPimpl> aPimpl;
    
	cScopeTimer();
};

int levenshtein( const std::string &s1, const std::string &s2 );

namespace rathena {
	namespace util {
		template <typename K, typename V> bool map_exists( std::map<K,V>& map, K key ){
			return map.find( key ) != map.end();
		}

		template <typename K, typename V> V* map_find( std::map<K,V>& map, K key ){
			auto it = map.find( key );

			if( it != map.end() ){
				return &it->second;
			}else{
				return nullptr;
			}
		}

		inline uint32 invert_rgb_color( uint32 color ){
			return ( ( color & 0xFF ) << 16) | ( color & 0xFF00 ) | ( ( color & 0xFF0000 ) >> 16 );
		}
	}
}

#endif /* _UTILILITIES_HPP_ */
