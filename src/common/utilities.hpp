// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef UTILILITIES_HPP
#define UTILILITIES_HPP

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "cbasetypes.hpp"
#include "random.hpp"

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

		/**
		 * Find a key-value pair and return the key value as a reference
		 * @param map: Map to search through
		 * @param key: Key wanted
		 * @return Key value on success or nullptr on failure
		 */
		template <typename K, typename V> V* map_find( std::map<K,V>& map, K key ){
			auto it = map.find( key );

			if( it != map.end() ){
				return &it->second;
			}else{
				return nullptr;
			}
		}

		/**
		 * Find a key-value pair and return the key value as a reference
		 * @param map: Map to search through
		 * @param key: Key wanted
		 * @return Key value on success or nullptr on failure
		 */
		template <typename K, typename V> std::shared_ptr<V> map_find( std::map<K,std::shared_ptr<V>>& map, K key ){
			auto it = map.find( key );

			if( it != map.end() ){
				return it->second;
			}else{
				return nullptr;
			}
		}

		/**
		 * Get a key-value pair and return the key value
		 * @param map: Map to search through
		 * @param key: Key wanted
		 * @param defaultValue: Value returned if key doesn't exist
		 * @return Key value on success or defaultValue on failure
		 */
		template <typename K, typename V> V map_get(std::map<K, V>& map, K key, V defaultValue) {
			auto it = map.find(key);

			if (it != map.end())
				return it->second;
			else
				return defaultValue;
		}

		/**
		 * Find a key-value pair and return the key value as a reference
		 * @param map: Unordered Map to search through
		 * @param key: Key wanted
		 * @return Key value on success or nullptr on failure
		 */
		template <typename K, typename V> V* umap_find(std::unordered_map<K, V>& map, K key) {
			auto it = map.find(key);

			if (it != map.end())
				return &it->second;
			else
				return nullptr;
		}

		/**
		 * Find a key-value pair and return the key value as a reference
		 * @param map: Unordered Map to search through
		 * @param key: Key wanted
		 * @return Key value on success or nullptr on failure
		 */
		template <typename K, typename V> std::shared_ptr<V> umap_find(std::unordered_map<K, std::shared_ptr<V>>& map, K key) {
			auto it = map.find(key);

			if (it != map.end())
				return it->second;
			else
				return nullptr;
		}

		/**
		 * Get a key-value pair and return the key value
		 * @param map: Unordered Map to search through
		 * @param key: Key wanted
		 * @param defaultValue: Value returned if key doesn't exist
		 * @return Key value on success or defaultValue on failure
		 */
		template <typename K, typename V> V umap_get(std::unordered_map<K, V>& map, K key, V defaultValue) {
			auto it = map.find(key);

			if (it != map.end())
				return it->second;
			else
				return defaultValue;
		}

		/**
		 * Get a random value from the given map
		 * @param map: Unordered Map to search through
		 * @return A random value by reference
		*/
		template <typename K, typename V> V& umap_random( std::unordered_map<K, V>& map ){
			auto it = map.begin();

			std::advance( it, rnd_value( 0, map.size() - 1 ) );

			return it->second;
		}

		bool safe_addition( int64 a, int64 b, int64& result );
		bool safe_substraction( int64 a, int64 b, int64& result );
		bool safe_multiplication( int64 a, int64 b, int64& result );
	}
}

#endif /* UTILILITIES_HPP */
