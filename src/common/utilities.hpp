#pragma once
#include <memory>
#include <string>
#include <map>

// Class used to perform time measurement
class cScopeTimer {
	struct sPimpl; //this is to avoid long compilation time
	std::unique_ptr<sPimpl> aPimpl;
    
	cScopeTimer();
};

int levenshtein( const std::string &s1, const std::string &s2 );

template <typename K, typename V> V* map_find( std::map<K,V>& map, K key ){
	auto it = map.find( key );

	if( it != map.end() ){
		return &it->second;
	}else{
		return nullptr;
	}
}
