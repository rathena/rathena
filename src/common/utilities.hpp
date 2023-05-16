// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef UTILILITIES_HPP
#define UTILILITIES_HPP

#include <algorithm>
#include <locale>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "cbasetypes.hpp"
#include "random.hpp"

#ifndef __has_builtin
	#define __has_builtin(x) 0
#endif

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
		 * Resize a map.
		 * @param map: Map to resize
		 * @param size: Size to set map to
		 */
		template <typename K, typename V, typename S> void map_resize(std::map<K, V> &map, S size) {
			auto it = map.begin();

			std::advance(it, size);

			map.erase(it, map.end());
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
		 * Resize an unordered map.
		 * @param map: Unordered map to resize
		 * @param size: Size to set unordered map to
		 */
		template <typename K, typename V, typename S> void umap_resize(std::unordered_map<K, V> &map, S size) {
			map.erase(std::advance(map.begin(), map.min(size, map.size())), map.end());
		}

		/**
		 * Get a random value from the given unordered map
		 * @param map: Unordered Map to search through
		 * @return A random value by reference
		*/
		template <typename K, typename V> V& umap_random( std::unordered_map<K, V>& map ){
			auto it = map.begin();

			std::advance( it, rnd_value( 0, map.size() - 1 ) );

			return it->second;
		}

		/**
		 * Get a random value from the given vector
		 * @param vec: Vector to search through
		 * @return A random value by reference
		*/
		template <typename K> K &vector_random(std::vector<K> &vec) {
			auto it = vec.begin();

			std::advance(it, rnd_value(0, vec.size() - 1));

			return *it;
		}

		/**
		 * Get an iterator element
		 * @param vec: Vector to search through
		 * @param value: Value wanted
		 * @return Key value iterator on success or vector end iterator on failure
		 */
		template <typename K, typename V> typename std::vector<K>::iterator vector_get(std::vector<K> &vec, V key) {
			return std::find(vec.begin(), vec.end(), key);
		}

		/**
		 * Determine if a value exists in the vector
		 * @param vec: Vector to search through
		 * @param value: Value wanted
		 * @return True on success or false on failure
		 */
		template <typename K, typename V> bool vector_exists(const std::vector<K> &vec, V value) {
			auto it = std::find(vec.begin(), vec.end(), value);

			if (it != vec.end())
				return true;
			else
				return false;
		}

		/**
		 * Erase an index value from a vector
		 * @param vector: Vector to erase value from
		 * @param index: Index value to remove
		 */
		template <typename K> void erase_at(std::vector<K>& vector, size_t index) {
			if (vector.size() == 1) {
				vector.clear();
				vector.shrink_to_fit();
			} else
				vector.erase(vector.begin() + index);
		}

		/**
		 * Determine if a value exists in the vector and then erase it
		 * This will only erase the first occurrence of the value
		 * @param vector: Vector to erase value from
		 * @param value: Value to remove
		 */
		template <typename K, typename V> void vector_erase_if_exists(std::vector<K> &vector, V value) {
			auto it = std::find(vector.begin(), vector.end(), value);

			if (it != vector.end()) {
				if (vector.size() == 1) {
					vector.clear();
					vector.shrink_to_fit();
				} else
					vector.erase(it);
			}
		}

#if __has_builtin( __builtin_add_overflow ) || ( defined( __GNUC__ ) && !defined( __clang__ ) && defined( GCC_VERSION  ) && GCC_VERSION >= 50100 )
		template <typename T> bool safe_addition(T a, T b, T &result) {
			return __builtin_add_overflow(a, b, &result);
		}
#else
		template <typename T> bool safe_addition( T a, T b, T& result ){
			bool overflow = false;

			if( std::numeric_limits<T>::is_signed ){
				if( b < 0 ){
					if( a < ( (std::numeric_limits<T>::min)() - b ) ){
						overflow = true;
					}
				}else{
					if( a > ( (std::numeric_limits<T>::max)() - b ) ){
						overflow = true;
					}
				}
			}else{
				if( a > ( (std::numeric_limits<T>::max)() - b ) ){
					overflow = true;
				}
			}

			result = a + b;

			return overflow;
		}
#endif

		bool safe_substraction( int64 a, int64 b, int64& result );
		bool safe_multiplication( int64 a, int64 b, int64& result );

		/**
		 * Safely add values without overflowing.
		 * @param a: Holder of value to increment
		 * @param b: Increment by
		 * @param cap: Cap value
		 * @return Result of a + b
		 */
		template <typename T> T safe_addition_cap( T a, T b, T cap ){
			T result;

			if( rathena::util::safe_addition( a, b, result ) ){
				return cap;
			}else{
				return result;
			}
		}

		template <typename T> void tolower( T& string ){
			std::transform( string.begin(), string.end(), string.begin(), ::tolower );
		}

		/**
		* Pad string with arbitrary character in-place
		* @param str: String to pad
		* @param padding: Padding character
		* @param num: Maximum length of padding
		*/
		void string_left_pad_inplace(std::string& str, char padding, size_t num);

		/**
		* Pad string with arbitrary character
		* @param original: String to pad
		* @param padding: Padding character
		* @param num: Maximum length of padding
		*
		* @return A copy of original string with padding added
		*/
		std::string string_left_pad(const std::string& original, char padding, size_t num);

		/**
		* Encode base10 number to base62. Originally by lututui
		* @param val: Base10 Number
		* @return Base62 string
		**/
		std::string base62_encode( uint32 val );
	}
}

#endif /* UTILILITIES_HPP */
