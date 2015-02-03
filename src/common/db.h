/*****************************************************************************\
 *  Copyright (c) Athena Dev Teams - Licensed under GNU GPL                  *
 *  For more information, see LICENCE in the main folder                     *
 *                                                                           *
 *  This file is separated in two sections:                                  *
 *  (1) public typedefs, enums, unions, structures and defines               *
 *  (2) public functions                                                     *
 *                                                                           *
 *  <B>Notes on the release system:</B>                                      *
 *  Whenever an entry is removed from the database both the key and the      *
 *  data are requested to be released.                                       *
 *  At least one entry is removed when replacing an entry, removing an       *
 *  entry, clearing the database or destroying the database.                 *
 *  What is actually released is defined by the release function, the        *
 *  functions of the database only ask for the key and/or data to be         *
 *  released.                                                                *
 *                                                                           *
 *  TODO:                                                                    *
 *  - create a custom database allocator                                     *
 *  - see what functions need or should be added to the database interface   *
 *                                                                           *
 *  HISTORY:                                                                 *
 *    2012/03/09 - Added enum for data types (int, uint, void*)              *
 *    2007/11/09 - Added an iterator to the database.                        *
 *    2.1 (Athena build #???#) - Portability fix                             *
 *      - Fixed the portability of casting to union and added the functions  *
 *        {@link DBMap#ensure(DBMap,DBKey,DBCreateData,...)} and             *
 *        {@link DBMap#clear(DBMap,DBApply,...)}.                            *
 *    2.0 (Athena build 4859) - Transition version                           *
 *      - Almost everything recoded with a strategy similar to objects,      *
 *        database structure is maintained.                                  *
 *    1.0 (up to Athena build 4706)                                          *
 *      - Previous database system.                                          *
 *                                                                           *
 * @version 2.1 (Athena build #???#) - Portability fix                       *
 * @author (Athena build 4859) Flavio @ Amazon Project                       *
 * @author (up to Athena build 4706) Athena Dev Teams                        *
 * @encoding US-ASCII                                                        *
 * @see common#db.c                                                          *
\*****************************************************************************/
#ifndef _DB_H_
#define _DB_H_

#include "../common/cbasetypes.h"
#include <stdarg.h>

/*****************************************************************************\
 *  (1) Section with public typedefs, enums, unions, structures and defines. *
 *  DBRelease    - Enumeration of release options.                           *
 *  DBType       - Enumeration of database types.                            *
 *  DBOptions    - Bitfield enumeration of database options.                 *
 *  DBKey        - Union of used key types.                                  *
 *  DBDataType   - Enumeration of data types.                                *
 *  DBData       - Struct for used data types.                               *
 *  DBApply      - Format of functions applied to the databases.             *
 *  DBMatcher    - Format of matchers used in DBMap::getall.                 *
 *  DBComparator - Format of the comparators used by the databases.          *
 *  DBHasher     - Format of the hashers used by the databases.              *
 *  DBReleaser   - Format of the releasers used by the databases.            *
 *  DBIterator   - Database iterator.                                        *
 *  DBMap        - Database interface.                                       *
\*****************************************************************************/

/**
 * Bitfield with what should be released by the releaser function (if the
 * function supports it).
 * @public
 * @see #DBReleaser
 * @see #db_custom_release(DBRelease)
 */
typedef enum DBRelease {
	DB_RELEASE_NOTHING = 0,
	DB_RELEASE_KEY     = 1,
	DB_RELEASE_DATA    = 2,
	DB_RELEASE_BOTH    = 3
} DBRelease;

/**
 * Supported types of database.
 * See {@link #db_fix_options(DBType,DBOptions)} for restrictions of the 
 * types of databases.
 * @param DB_INT Uses int's for keys
 * @param DB_UINT Uses unsigned int's for keys
 * @param DB_STRING Uses strings for keys.
 * @param DB_ISTRING Uses case insensitive strings for keys.
 * @public
 * @see #DBOptions
 * @see #DBKey
 * @see #db_fix_options(DBType,DBOptions)
 * @see #db_default_cmp(DBType)
 * @see #db_default_hash(DBType)
 * @see #db_default_release(DBType,DBOptions)
 * @see #db_alloc(const char *,int,DBType,DBOptions,unsigned short)
 */
typedef enum DBType {
	DB_INT,
	DB_UINT,
	DB_STRING,
	DB_ISTRING
} DBType;

/**
 * Bitfield of options that define the behaviour of the database.
 * See {@link #db_fix_options(DBType,DBOptions)} for restrictions of the 
 * types of databases.
 * @param DB_OPT_BASE Base options: does not duplicate keys, releases nothing
 *          and does not allow NULL keys or NULL data.
 * @param DB_OPT_DUP_KEY Duplicates the keys internally. If DB_OPT_RELEASE_KEY 
 *          is defined, the real key is freed as soon as the entry is added.
 * @param DB_OPT_RELEASE_KEY Releases the key.
 * @param DB_OPT_RELEASE_DATA Releases the data whenever an entry is removed 
 *          from the database.
 *          WARNING: for funtions that return the data (like DBMap::remove),
 *          a dangling pointer will be returned.
 * @param DB_OPT_RELEASE_BOTH Releases both key and data.
 * @param DB_OPT_ALLOW_NULL_KEY Allow NULL keys in the database.
 * @param DB_OPT_ALLOW_NULL_DATA Allow NULL data in the database.
 * @public
 * @see #db_fix_options(DBType,DBOptions)
 * @see #db_default_release(DBType,DBOptions)
 * @see #db_alloc(const char *,int,DBType,DBOptions,unsigned short)
 */
typedef enum DBOptions {
	DB_OPT_BASE            = 0,
	DB_OPT_DUP_KEY         = 1,
	DB_OPT_RELEASE_KEY     = 2,
	DB_OPT_RELEASE_DATA    = 4,
	DB_OPT_RELEASE_BOTH    = 6,
	DB_OPT_ALLOW_NULL_KEY  = 8,
	DB_OPT_ALLOW_NULL_DATA = 16,
} DBOptions;

/**
 * Union of key types used by the database.
 * @param i Type of key for DB_INT databases
 * @param ui Type of key for DB_UINT databases
 * @param str Type of key for DB_STRING and DB_ISTRING databases
 * @public
 * @see #DBType
 * @see DBMap#get
 * @see DBMap#put
 * @see DBMap#remove
 */
typedef union DBKey {
	int i;
	unsigned int ui;
	const char *str;
} DBKey;

/**
 * Supported types of database data.
 * @param DB_DATA_INT Uses ints for data.
 * @param DB_DATA_UINT Uses unsigned ints for data.
 * @param DB_DATA_PTR Uses void pointers for data.
 * @public
 * @see #DBData
 */
typedef enum DBDataType {
	DB_DATA_INT,
	DB_DATA_UINT,
	DB_DATA_PTR
} DBDataType;

/**
 * Struct for data types used by the database.
 * @param type Type of data
 * @param u Union of available data types
 * @param u.i Data of int type
 * @param u.ui Data of unsigned int type
 * @param u.ptr Data of void* type
 * @public
 */
typedef struct DBData {
	DBDataType type;
	union {
		int i;
		unsigned int ui;
		void *ptr;
	} u;
} DBData;

/**
 * Format of functions that create the data for the key when the entry doesn't 
 * exist in the database yet.
 * @param key Key of the database entry
 * @param args Extra arguments of the function
 * @return Data identified by the key to be put in the database
 * @public
 * @see DBMap#vensure
 * @see DBMap#ensure
 */
typedef DBData (*DBCreateData)(DBKey key, va_list args);

/**
 * Format of functions to be applied to an unspecified quantity of entries of 
 * a database.
 * Any function that applies this function to the database will return the sum 
 * of values returned by this function.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param args Extra arguments of the function
 * @return Value to be added up by the function that is applying this
 * @public
 * @see DBMap#vforeach
 * @see DBMap#foreach
 * @see DBMap#vdestroy
 * @see DBMap#destroy
 */
typedef int (*DBApply)(DBKey key, DBData *data, va_list args);

/**
 * Format of functions that match database entries.
 * The purpose of the match depends on the function that is calling the matcher.
 * Returns 0 if it is a match, another number otherwise.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param args Extra arguments of the function
 * @return 0 if a match, another number otherwise
 * @public
 * @see DBMap#getall
 */
typedef int (*DBMatcher)(DBKey key, DBData data, va_list args);

/**
 * Format of the comparators used internally by the database system.
 * Compares key1 to key2.
 * Returns 0 is equal, negative if lower and positive is higher.
 * @param key1 Key being compared
 * @param key2 Key we are comparing to
 * @param maxlen Maximum number of characters used in DB_STRING and DB_ISTRING
 *          databases.
 * @return 0 if equal, negative if lower and positive if higher
 * @public
 * @see #db_default_cmp(DBType)
 */
typedef int (*DBComparator)(DBKey key1, DBKey key2, unsigned short maxlen);

/**
 * Format of the hashers used internally by the database system.
 * Creates the hash of the key.
 * @param key Key being hashed
 * @param maxlen Maximum number of characters used in DB_STRING and DB_ISTRING
 *          databases.
 * @return Hash of the key
 * @public
 * @see #db_default_hash(DBType)
 */
typedef unsigned int (*DBHasher)(DBKey key, unsigned short maxlen);

/**
 * Format of the releaser used by the database system.
 * Releases nothing, the key, the data or both.
 * All standard releasers use aFree to release.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param which What is being requested to be released
 * @public
 * @see #DBRelease
 * @see #db_default_releaser(DBType,DBOptions)
 * @see #db_custom_release(DBRelease)
 */
typedef void (*DBReleaser)(DBKey key, DBData data, DBRelease which);



typedef struct DBIterator DBIterator;
typedef struct DBMap DBMap;



/**
 * Database iterator.
 * Supports forward iteration, backward iteration and removing entries from the database.
 * The iterator is initially positioned before the first entry of the database.
 * While the iterator exists the database is locked internally, so invoke 
 * {@link DBIterator#destroy} as soon as possible.
 * @public
 * @see #DBMap
 */
struct DBIterator
{

	/**
	 * Fetches the first entry in the database.
	 * Returns the data of the entry.
	 * Puts the key in out_key, if out_key is not NULL.
	 * @param self Iterator
	 * @param out_key Key of the entry
	 * @return Data of the entry
	 * @protected
	 */
	DBData* (*first)(DBIterator* self, DBKey* out_key);

	/**
	 * Fetches the last entry in the database.
	 * Returns the data of the entry.
	 * Puts the key in out_key, if out_key is not NULL.
	 * @param self Iterator
	 * @param out_key Key of the entry
	 * @return Data of the entry
	 * @protected
	 */
	DBData* (*last)(DBIterator* self, DBKey* out_key);

	/**
	 * Fetches the next entry in the database.
	 * Returns the data of the entry.
	 * Puts the key in out_key, if out_key is not NULL.
	 * @param self Iterator
	 * @param out_key Key of the entry
	 * @return Data of the entry
	 * @protected
	 */
	DBData* (*next)(DBIterator* self, DBKey* out_key);

	/**
	 * Fetches the previous entry in the database.
	 * Returns the data of the entry.
	 * Puts the key in out_key, if out_key is not NULL.
	 * @param self Iterator
	 * @param out_key Key of the entry
	 * @return Data of the entry
	 * @protected
	 */
	DBData* (*prev)(DBIterator* self, DBKey* out_key);

	/**
	 * Returns true if the fetched entry exists.
	 * The databases entries might have NULL data, so use this to to test if 
	 * the iterator is done.
	 * @param self Iterator
	 * @return true is the entry exists
	 * @protected
	 */
	bool (*exists)(DBIterator* self);

	/**
	 * Removes the current entry from the database.
	 * NOTE: {@link DBIterator#exists} will return false until another entry 
	 *       is fetched
	 * Puts data of the removed entry in out_data, if out_data is not NULL.
	 * @param self Iterator
	 * @param out_data Data of the removed entry.
	 * @return 1 if entry was removed, 0 otherwise
	 * @protected
	 * @see DBMap#remove
	 */
	int (*remove)(DBIterator* self, DBData *out_data);

	/**
	 * Destroys this iterator and unlocks the database.
	 * @param self Iterator
	 * @protected
	 */
	void (*destroy)(DBIterator* self);

};

/**
 * Public interface of a database. Only contains funtions.
 * All the functions take the interface as the first argument.
 * @public
 * @see #db_alloc(const char*,int,DBType,DBOptions,unsigned short)
 */
struct DBMap {

	/**
	 * Returns a new iterator for this database.
	 * The iterator keeps the database locked until it is destroyed.
	 * The database will keep functioning normally but will only free internal 
	 * memory when unlocked, so destroy the iterator as soon as possible.
	 * @param self Database
	 * @return New iterator
	 * @protected
	 */
	DBIterator* (*iterator)(DBMap* self);

	/**
	 * Returns true if the entry exists.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @return true is the entry exists
	 * @protected
	 */
	bool (*exists)(DBMap* self, DBKey key);

	/**
	 * Get the data of the entry identified by the key.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @return Data of the entry or NULL if not found
	 * @protected
	 */
	DBData* (*get)(DBMap* self, DBKey key);

	/**
	 * Just calls {@link DBMap#vgetall}.
	 * Get the data of the entries matched by <code>match</code>.
	 * It puts a maximum of <code>max</code> entries into <code>buf</code>.
	 * If <code>buf</code> is NULL, it only counts the matches.
	 * Returns the number of entries that matched.
	 * NOTE: if the value returned is greater than <code>max</code>, only the 
	 * first <code>max</code> entries found are put into the buffer.
	 * @param self Database
	 * @param buf Buffer to put the data of the matched entries
	 * @param max Maximum number of data entries to be put into buf
	 * @param match Function that matches the database entries
	 * @param ... Extra arguments for match
	 * @return The number of entries that matched
	 * @protected
	 * @see DBMap#vgetall(DBMap*,void **,unsigned int,DBMatcher,va_list)
	 */
	unsigned int (*getall)(DBMap* self, DBData** buf, unsigned int max, DBMatcher match, ...);

	/**
	 * Get the data of the entries matched by <code>match</code>.
	 * It puts a maximum of <code>max</code> entries into <code>buf</code>.
	 * If <code>buf</code> is NULL, it only counts the matches.
	 * Returns the number of entries that matched.
	 * NOTE: if the value returned is greater than <code>max</code>, only the 
	 * first <code>max</code> entries found are put into the buffer.
	 * @param self Database
	 * @param buf Buffer to put the data of the matched entries
	 * @param max Maximum number of data entries to be put into buf
	 * @param match Function that matches the database entries
	 * @param ... Extra arguments for match
	 * @return The number of entries that matched
	 * @protected
	 * @see DBMap#getall(DBMap*,void **,unsigned int,DBMatcher,...)
	 */
	unsigned int (*vgetall)(DBMap* self, DBData** buf, unsigned int max, DBMatcher match, va_list args);

	/**
	 * Just calls {@link DBMap#vensure}.
	 * Get the data of the entry identified by the key.
	 * If the entry does not exist, an entry is added with the data returned by 
	 * <code>create</code>.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @param create Function used to create the data if the entry doesn't exist
	 * @param ... Extra arguments for create
	 * @return Data of the entry
	 * @protected
	 * @see DBMap#vensure(DBMap*,DBKey,DBCreateData,va_list)
	 */
	DBData* (*ensure)(DBMap* self, DBKey key, DBCreateData create, ...);

	/**
	 * Get the data of the entry identified by the key.
	 * If the entry does not exist, an entry is added with the data returned by 
	 * <code>create</code>.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @param create Function used to create the data if the entry doesn't exist
	 * @param args Extra arguments for create
	 * @return Data of the entry
	 * @protected
	 * @see DBMap#ensure(DBMap*,DBKey,DBCreateData,...)
	 */
	DBData* (*vensure)(DBMap* self, DBKey key, DBCreateData create, va_list args);

	/**
	 * Put the data identified by the key in the database.
	 * Puts the previous data in out_data, if out_data is not NULL.
	 * NOTE: Uses the new key, the old one is released.
	 * @param self Database
	 * @param key Key that identifies the data
	 * @param data Data to be put in the database
	 * @param out_data Previous data if the entry exists
	 * @return 1 if if the entry already exists, 0 otherwise
	 * @protected
	 */
	int (*put)(DBMap* self, DBKey key, DBData data, DBData *out_data);

	/**
	 * Remove an entry from the database.
	 * Puts the previous data in out_data, if out_data is not NULL.
	 * NOTE: The key (of the database) is released.
	 * @param self Database
	 * @param key Key that identifies the entry
	 * @param out_data Previous data if the entry exists
	 * @return 1 if if the entry already exists, 0 otherwise
	 * @protected
	 */
	int (*remove)(DBMap* self, DBKey key, DBData *out_data);

	/**
	 * Just calls {@link DBMap#vforeach}.
	 * Apply <code>func</code> to every entry in the database.
	 * Returns the sum of values returned by func.
	 * @param self Database
	 * @param func Function to be applied
	 * @param ... Extra arguments for func
	 * @return Sum of the values returned by func
	 * @protected
	 * @see DBMap#vforeach(DBMap*,DBApply,va_list)
	 */
	int (*foreach)(DBMap* self, DBApply func, ...);

	/**
	 * Apply <code>func</code> to every entry in the database.
	 * Returns the sum of values returned by func.
	 * @param self Database
	 * @param func Function to be applied
	 * @param args Extra arguments for func
	 * @return Sum of the values returned by func
	 * @protected
	 * @see DBMap#foreach(DBMap*,DBApply,...)
	 */
	int (*vforeach)(DBMap* self, DBApply func, va_list args);

	/**
	 * Just calls {@link DBMap#vclear}.
	 * Removes all entries from the database.
	 * Before deleting an entry, func is applied to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * @param self Database
	 * @param func Function to be applied to every entry before deleting
	 * @param ... Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see DBMap#vclear(DBMap*,DBApply,va_list)
	 */
	int (*clear)(DBMap* self, DBApply func, ...);

	/**
	 * Removes all entries from the database.
	 * Before deleting an entry, func is applied to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * @param self Database
	 * @param func Function to be applied to every entry before deleting
	 * @param args Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see DBMap#clear(DBMap*,DBApply,...)
	 */
	int (*vclear)(DBMap* self, DBApply func, va_list args);

	/**
	 * Just calls {@link DBMap#vdestroy}.
	 * Finalize the database, feeing all the memory it uses.
	 * Before deleting an entry, func is applied to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * NOTE: This locks the database globally. Any attempt to insert or remove 
	 * a database entry will give an error and be aborted (except for clearing).
	 * @param self Database
	 * @param func Function to be applied to every entry before deleting
	 * @param ... Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see DBMap#vdestroy(DBMap*,DBApply,va_list)
	 */
	int (*destroy)(DBMap* self, DBApply func, ...);

	/**
	 * Finalize the database, feeing all the memory it uses.
	 * Before deleting an entry, func is applied to it.
	 * Returns the sum of values returned by func, if it exists.
	 * NOTE: This locks the database globally. Any attempt to insert or remove 
	 * a database entry will give an error and be aborted (except for clearing).
	 * @param self Database
	 * @param func Function to be applied to every entry before deleting
	 * @param args Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see DBMap#destroy(DBMap*,DBApply,...)
	 */
	int (*vdestroy)(DBMap* self, DBApply func, va_list args);

	/**
	 * Return the size of the database (number of items in the database).
	 * @param self Database
	 * @return Size of the database
	 * @protected
	 */
	unsigned int (*size)(DBMap* self);

	/**
	 * Return the type of the database.
	 * @param self Database
	 * @return Type of the database
	 * @protected
	 */
	DBType (*type)(DBMap* self);

	/**
	 * Return the options of the database.
	 * @param self Database
	 * @return Options of the database
	 * @protected
	 */
	DBOptions (*options)(DBMap* self);

};

// For easy access to the common functions.

#define db_exists(db,k)    ( (db)->exists((db),(k)) )
#define idb_exists(db,k)   ( (db)->exists((db),db_i2key(k)) )
#define uidb_exists(db,k)  ( (db)->exists((db),db_ui2key(k)) )
#define strdb_exists(db,k) ( (db)->exists((db),db_str2key(k)) )

// Get pointer-type data from DBMaps of various key types
#define db_get(db,k)    ( db_data2ptr((db)->get((db),(k))) )
#define idb_get(db,k)   ( db_data2ptr((db)->get((db),db_i2key(k))) )
#define uidb_get(db,k)  ( db_data2ptr((db)->get((db),db_ui2key(k))) )
#define strdb_get(db,k) ( db_data2ptr((db)->get((db),db_str2key(k))) )

// Get int-type data from DBMaps of various key types
#define db_iget(db,k)    ( db_data2i((db)->get((db),(k))) )
#define idb_iget(db,k)   ( db_data2i((db)->get((db),db_i2key(k))) )
#define uidb_iget(db,k)  ( db_data2i((db)->get((db),db_ui2key(k))) )
#define strdb_iget(db,k) ( db_data2i((db)->get((db),db_str2key(k))) )

// Get uint-type data from DBMaps of various key types
#define db_uiget(db,k)    ( db_data2ui((db)->get((db),(k))) )
#define idb_uiget(db,k)   ( db_data2ui((db)->get((db),db_i2key(k))) )
#define uidb_uiget(db,k)  ( db_data2ui((db)->get((db),db_ui2key(k))) )
#define strdb_uiget(db,k) ( db_data2ui((db)->get((db),db_str2key(k))) )

// Put pointer-type data into DBMaps of various key types
#define db_put(db,k,d)    ( (db)->put((db),(k),db_ptr2data(d),NULL) )
#define idb_put(db,k,d)   ( (db)->put((db),db_i2key(k),db_ptr2data(d),NULL) )
#define uidb_put(db,k,d)  ( (db)->put((db),db_ui2key(k),db_ptr2data(d),NULL) )
#define strdb_put(db,k,d) ( (db)->put((db),db_str2key(k),db_ptr2data(d),NULL) )

// Put int-type data into DBMaps of various key types
#define db_iput(db,k,d)    ( (db)->put((db),(k),db_i2data(d),NULL) )
#define idb_iput(db,k,d)   ( (db)->put((db),db_i2key(k),db_i2data(d),NULL) )
#define uidb_iput(db,k,d)  ( (db)->put((db),db_ui2key(k),db_i2data(d),NULL) )
#define strdb_iput(db,k,d) ( (db)->put((db),db_str2key(k),db_i2data(d),NULL) )

// Put uint-type data into DBMaps of various key types
#define db_uiput(db,k,d)    ( (db)->put((db),(k),db_ui2data(d),NULL) )
#define idb_uiput(db,k,d)   ( (db)->put((db),db_i2key(k),db_ui2data(d),NULL) )
#define uidb_uiput(db,k,d)  ( (db)->put((db),db_ui2key(k),db_ui2data(d),NULL) )
#define strdb_uiput(db,k,d) ( (db)->put((db),db_str2key(k),db_ui2data(d),NULL) )

// Remove entry from DBMaps of various key types
#define db_remove(db,k)    ( (db)->remove((db),(k),NULL) )
#define idb_remove(db,k)   ( (db)->remove((db),db_i2key(k),NULL) )
#define uidb_remove(db,k)  ( (db)->remove((db),db_ui2key(k),NULL) )
#define strdb_remove(db,k) ( (db)->remove((db),db_str2key(k),NULL) )

//These are discarding the possible vargs you could send to the function, so those
//that require vargs must not use these defines.
#define db_ensure(db,k,f)    ( db_data2ptr((db)->ensure((db),(k),(f))) )
#define idb_ensure(db,k,f)   ( db_data2ptr((db)->ensure((db),db_i2key(k),(f))) )
#define uidb_ensure(db,k,f)  ( db_data2ptr((db)->ensure((db),db_ui2key(k),(f))) )
#define strdb_ensure(db,k,f) ( db_data2ptr((db)->ensure((db),db_str2key(k),(f))) )

// Database creation and destruction macros
#define idb_alloc(opt)            db_alloc(__FILE__,__LINE__,DB_INT,(opt),sizeof(int))
#define uidb_alloc(opt)           db_alloc(__FILE__,__LINE__,DB_UINT,(opt),sizeof(unsigned int))
#define strdb_alloc(opt,maxlen)   db_alloc(__FILE__,__LINE__,DB_STRING,(opt),(maxlen))
#define stridb_alloc(opt,maxlen)  db_alloc(__FILE__,__LINE__,DB_ISTRING,(opt),(maxlen))
#define db_destroy(db)            ( (db)->destroy((db),NULL) )
// Other macros
#define db_clear(db)        ( (db)->clear(db,NULL) )
#define db_size(db)         ( (db)->size(db) )
#define db_iterator(db)     ( (db)->iterator(db) )
#define dbi_first(dbi)      ( db_data2ptr((dbi)->first(dbi,NULL)) )
#define dbi_last(dbi)       ( db_data2ptr((dbi)->last(dbi,NULL)) )
#define dbi_next(dbi)       ( db_data2ptr((dbi)->next(dbi,NULL)) )
#define dbi_prev(dbi)       ( db_data2ptr((dbi)->prev(dbi,NULL)) )
#define dbi_remove(dbi)     ( (dbi)->remove(dbi,NULL) )
#define dbi_exists(dbi)     ( (dbi)->exists(dbi) )
#define dbi_destroy(dbi)    ( (dbi)->destroy(dbi) )

/*****************************************************************************\
 *  (2) Section with public functions.                                       *
 *  db_fix_options     - Fix the options for a type of database.             *
 *  db_default_cmp     - Get the default comparator for a type of database.  *
 *  db_default_hash    - Get the default hasher for a type of database.      *
 *  db_default_release - Get the default releaser for a type of database     *
 *           with the fixed options.                                         *
 *  db_custom_release  - Get the releaser that behaves as specified.         *
 *  db_alloc           - Allocate a new database.                            *
 *  db_i2key           - Manual cast from 'int' to 'DBKey'.                  *
 *  db_ui2key          - Manual cast from 'unsigned int' to 'DBKey'.         *
 *  db_str2key         - Manual cast from 'unsigned char *' to 'DBKey'.      *
 *  db_i2data          - Manual cast from 'int' to 'DBData'.                 *
 *  db_ui2data         - Manual cast from 'unsigned int' to 'DBData'.        *
 *  db_ptr2data        - Manual cast from 'void*' to 'DBData'.               *
 *  db_data2i          - Gets 'int' value from 'DBData'.                     *
 *  db_data2ui         - Gets 'unsigned int' value from 'DBData'.            *
 *  db_data2ptr        - Gets 'void*' value from 'DBData'.                   *
 *  db_init            - Initializes the database system.                    *
 *  db_final           - Finalizes the database system.                      *
\*****************************************************************************/

/**
 * Returns the fixed options according to the database type.
 * Sets required options and unsets unsupported options.
 * For numeric databases DB_OPT_DUP_KEY and DB_OPT_RELEASE_KEY are unset.
 * @param type Type of the database
 * @param options Original options of the database
 * @return Fixed options of the database
 * @private
 * @see #DBType
 * @see #DBOptions
 * @see #db_default_release(DBType,DBOptions)
 */
DBOptions db_fix_options(DBType type, DBOptions options);

/**
 * Returns the default comparator for the type of database.
 * @param type Type of database
 * @return Comparator for the type of database or NULL if unknown database
 * @public
 * @see #DBType
 * @see #DBComparator
 */
DBComparator db_default_cmp(DBType type);

/**
 * Returns the default hasher for the specified type of database.
 * @param type Type of database
 * @return Hasher of the type of database or NULL if unknown database
 * @public
 * @see #DBType
 * @see #DBHasher
 */
DBHasher db_default_hash(DBType type);

/**
 * Returns the default releaser for the specified type of database with the 
 * specified options.
 * NOTE: the options are fixed by {@link #db_fix_options(DBType,DBOptions)}
 * before choosing the releaser
 * @param type Type of database
 * @param options Options of the database
 * @return Default releaser for the type of database with the fixed options
 * @public
 * @see #DBType
 * @see #DBOptions
 * @see #DBReleaser
 * @see #db_fix_options(DBType,DBOptions)
 * @see #db_custom_release(DBRelease)
 */
DBReleaser db_default_release(DBType type, DBOptions options);

/**
 * Returns the releaser that behaves as <code>which</code> specifies.
 * @param which Defines what the releaser releases
 * @return Releaser for the specified release options
 * @public
 * @see #DBRelease
 * @see #DBReleaser
 * @see #db_default_release(DBType,DBOptions)
 */
DBReleaser db_custom_release(DBRelease which);

/**
 * Allocate a new database of the specified type.
 * It uses the default comparator, hasher and releaser of the specified 
 * database type and fixed options.
 * NOTE: the options are fixed by {@link #db_fix_options(DBType,DBOptions)}
 * before creating the database.
 * @param file File where the database is being allocated
 * @param line Line of the file where the database is being allocated
 * @param type Type of database
 * @param options Options of the database
 * @param maxlen Maximum length of the string to be used as key in string 
 *          databases. If 0, the maximum number of maxlen is used (64K).
 * @return The interface of the database
 * @public
 * @see #DBType
 * @see #DBMap
 * @see #db_default_cmp(DBType)
 * @see #db_default_hash(DBType)
 * @see #db_default_release(DBType,DBOptions)
 * @see #db_fix_options(DBType,DBOptions)
 */
DBMap* db_alloc(const char *file, int line, DBType type, DBOptions options, unsigned short maxlen);

/**
 * Manual cast from 'int' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
DBKey db_i2key(int key);

/**
 * Manual cast from 'unsigned int' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
DBKey db_ui2key(unsigned int key);

/**
 * Manual cast from 'unsigned char *' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
DBKey db_str2key(const char *key);

/**
 * Manual cast from 'int' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
DBData db_i2data(int data);

/**
 * Manual cast from 'unsigned int' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
DBData db_ui2data(unsigned int data);

/**
 * Manual cast from 'void *' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
DBData db_ptr2data(void *data);

/**
 * Gets int type data from struct DBData.
 * If data is not int type, returns 0.
 * @param data Data
 * @return Integer value of the data.
 * @public
 */
int db_data2i(DBData *data);

/**
 * Gets unsigned int type data from struct DBData.
 * If data is not unsigned int type, returns 0.
 * @param data Data
 * @return Unsigned int value of the data.
 * @public
 */
unsigned int db_data2ui(DBData *data);

/**
 * Gets void* type data from struct DBData.
 * If data is not void* type, returns NULL.
 * @param data Data
 * @return Void* value of the data.
 * @public
 */
void* db_data2ptr(DBData *data);

/**
 * Initialize the database system.
 * @public
 * @see #db_final(void)
 */
void db_init(void);

/**
 * Finalize the database system.
 * Frees the memory used by the block reusage system.
 * @public
 * @see #db_init(void)
 */
void db_final(void);

// Link DB System - From jAthena
struct linkdb_node {
	struct linkdb_node *next;
	struct linkdb_node *prev;
	void               *key;
	void               *data;
};

typedef int (*LinkDBFunc)(void* key, void* data, va_list args);

void  linkdb_insert ( struct linkdb_node** head, void *key, void* data); // 重複を考慮しない
void  linkdb_replace( struct linkdb_node** head, void *key, void* data); // 重複を考慮する
void* linkdb_search ( struct linkdb_node** head, void *key);
void* linkdb_erase  ( struct linkdb_node** head, void *key);
void  linkdb_final  ( struct linkdb_node** head );
int   linkdb_vforeach(struct linkdb_node** head, LinkDBFunc func, va_list ap);
int   linkdb_foreach( struct linkdb_node** head, LinkDBFunc func, ...  );



/// Finds an entry in an array.
/// ex: ARR_FIND(0, size, i, list[i] == target);
///
/// @param __start   Starting index (ex: 0)
/// @param __end     End index (ex: size of the array)
/// @param __var     Index variable
/// @param __cmp     Expression that returns true when the target entry is found
#define ARR_FIND(__start, __end, __var, __cmp) \
	do{ \
		for( (__var) = (__start); (__var) < (__end); ++(__var) ) \
			if( __cmp ) \
				break; \
	}while(0)



/// Moves an entry of the array.
/// Use ARR_MOVERIGHT/ARR_MOVELEFT if __from and __to are direct numbers.
/// ex: ARR_MOVE(i, 0, list, int);// move index i to index 0
///
///
/// @param __from   Initial index of the entry
/// @param __to     Target index of the entry
/// @param __arr    Array
/// @param __type   Type of entry
#define ARR_MOVE(__from, __to, __arr, __type) \
	do{ \
		if( (__from) != (__to) ) \
		{ \
			__type __backup__; \
			memmove(&__backup__, (__arr)+(__from), sizeof(__type)); \
			if( (__from) < (__to) ) \
				memmove((__arr)+(__from), (__arr)+(__from)+1, ((__to)-(__from))*sizeof(__type)); \
			else if( (__from) > (__to) ) \
				memmove((__arr)+(__to)+1, (__arr)+(__to), ((__from)-(__to))*sizeof(__type)); \
			memmove((__arr)+(__to), &__backup__, sizeof(__type)); \
		} \
	}while(0)



/// Moves an entry of the array to the right.
/// ex: ARR_MOVERIGHT(1, 4, list, int);// move index 1 to index 4
///
/// @param __from   Initial index of the entry
/// @param __to     Target index of the entry
/// @param __arr    Array
/// @param __type   Type of entry
#define ARR_MOVERIGHT(__from, __to, __arr, __type) \
	do{ \
		__type __backup__; \
		memmove(&__backup__, (__arr)+(__from), sizeof(__type)); \
		memmove((__arr)+(__from), (__arr)+(__from)+1, ((__to)-(__from))*sizeof(__type)); \
		memmove((__arr)+(__to), &__backup__, sizeof(__type)); \
	}while(0)



/// Moves an entry of the array to the left.
/// ex: ARR_MOVELEFT(3, 0, list, int);// move index 3 to index 0
///
/// @param __from   Initial index of the entry
/// @param __end    Target index of the entry
/// @param __arr    Array
/// @param __type   Type of entry
#define ARR_MOVELEFT(__from, __to, __arr, __type) \
	do{ \
		__type __backup__; \
		memmove(&__backup__, (__arr)+(__from), sizeof(__type)); \
		memmove((__arr)+(__to)+1, (__arr)+(__to), ((__from)-(__to))*sizeof(__type)); \
		memmove((__arr)+(__to), &__backup__, sizeof(__type)); \
	}while(0)



/////////////////////////////////////////////////////////////////////
// Vector library based on defines. (dynamic array)
// uses aMalloc, aRealloc, aFree



/// Declares an anonymous vector struct.
///
/// @param __type Type of data
#define VECTOR_DECL(__type) \
	struct { \
		size_t _max_; \
		size_t _len_; \
		__type* _data_; \
	}



/// Declares a named vector struct.
///
/// @param __name Structure name
/// @param __type Type of data
#define VECTOR_STRUCT_DECL(__name,__type) \
	struct __name { \
		size_t _max_; \
		size_t _len_; \
		__type* _data_; \
	}



/// Declares and initializes an anonymous vector variable.
///
/// @param __type Type of data
/// @param __var Variable name
#define VECTOR_VAR(__type,__var) \
	VECTOR_DECL(__type) __var = {0,0,NULL}



/// Declares and initializes a named vector variable.
///
/// @param __name Structure name
/// @param __var Variable name
#define VECTOR_STRUCT_VAR(__name,__var) \
	struct __name __var = {0,0,NULL}



/// Initializes a vector.
///
/// @param __vec Vector
#define VECTOR_INIT(__vec) \
	memset(&(__vec), 0, sizeof(__vec))



/// Returns the internal array of values.
///
/// @param __vec Vector
/// @return Array of values
#define VECTOR_DATA(__vec) \
	( (__vec)._data_ )



/// Returns the length of the vector.
///
/// @param __vec Vector
/// @return Length
#define VECTOR_LENGTH(__vec) \
	( (__vec)._len_ )



/// Returns the capacity of the vector.
///
/// @param __vec Vector
/// @return Capacity
#define VECTOR_CAPACITY(__vec) \
	( (__vec)._max_ )



/// Returns the value at the target index.
/// Assumes the index exists.
///
/// @param __vec Vector
/// @param __idx Index
/// @return Value
#define VECTOR_INDEX(__vec,__idx) \
	( VECTOR_DATA(__vec)[__idx] )



/// Returns the first value of the vector.
/// Assumes the array is not empty.
///
/// @param __vec Vector
/// @return First value
#define VECTOR_FIRST(__vec) \
	( VECTOR_INDEX(__vec,0) )



/// Returns the last value of the vector.
/// Assumes the array is not empty.
///
/// @param __vec Vector
/// @return Last value
#define VECTOR_LAST(__vec) \
	( VECTOR_INDEX(__vec,VECTOR_LENGTH(__vec)-1) )



/// Resizes the vector.
/// Excess values are discarded, new positions are zeroed.
///
/// @param __vec Vector
/// @param __n Size
#define VECTOR_RESIZE(__vec,__n) \
	do{ \
		if( (__n) > VECTOR_CAPACITY(__vec) ) \
		{ /* increase size */ \
			if( VECTOR_CAPACITY(__vec) == 0 ) SET_POINTER(VECTOR_DATA(__vec), aMalloc((__n)*sizeof(VECTOR_FIRST(__vec)))); /* allocate new */ \
			else SET_POINTER(VECTOR_DATA(__vec), aRealloc(VECTOR_DATA(__vec),(__n)*sizeof(VECTOR_FIRST(__vec)))); /* reallocate */ \
			memset(VECTOR_DATA(__vec)+VECTOR_LENGTH(__vec), 0, (VECTOR_CAPACITY(__vec)-VECTOR_LENGTH(__vec))*sizeof(VECTOR_FIRST(__vec))); /* clear new data */ \
			VECTOR_CAPACITY(__vec) = (__n); /* update capacity */ \
		} \
		else if( (__n) == 0 && VECTOR_CAPACITY(__vec) ) \
		{ /* clear vector */ \
			aFree(VECTOR_DATA(__vec)); VECTOR_DATA(__vec) = NULL; /* free data */ \
			VECTOR_CAPACITY(__vec) = 0; /* clear capacity */ \
			VECTOR_LENGTH(__vec) = 0; /* clear length */ \
		} \
		else if( (__n) < VECTOR_CAPACITY(__vec) ) \
		{ /* reduce size */ \
			SET_POINTER(VECTOR_DATA(__vec), aRealloc(VECTOR_DATA(__vec),(__n)*sizeof(VECTOR_FIRST(__vec)))); /* reallocate */ \
			VECTOR_CAPACITY(__vec) = (__n); /* update capacity */ \
			if( VECTOR_LENGTH(__vec) > (__n) ) VECTOR_LENGTH(__vec) = (__n); /* update length */ \
		} \
	}while(0)



/// Ensures that the array has the target number of empty positions.
/// Increases the capacity in multiples of __step.
///
/// @param __vec Vector
/// @param __n Empty positions
/// @param __step Increase
#define VECTOR_ENSURE(__vec,__n,__step) \
	do{ \
		size_t _empty_ = VECTOR_CAPACITY(__vec)-VECTOR_LENGTH(__vec); \
		while( (__n) > _empty_ ) _empty_ += (__step); \
		if( _empty_ != VECTOR_CAPACITY(__vec)-VECTOR_LENGTH(__vec) ) VECTOR_RESIZE(__vec,_empty_+VECTOR_LENGTH(__vec)); \
	}while(0)



/// Inserts a zeroed value in the target index.
/// Assumes the index is valid and there is enough capacity.
///
/// @param __vec Vector
/// @param __idx Index
#define VECTOR_INSERTZEROED(__vec,__idx) \
	do{ \
		if( (__idx) < VECTOR_LENGTH(__vec) ) /* move data */ \
			memmove(&VECTOR_INDEX(__vec,(__idx)+1),&VECTOR_INDEX(__vec,__idx),(VECTOR_LENGTH(__vec)-(__idx))*sizeof(VECTOR_FIRST(__vec))); \
		memset(&VECTOR_INDEX(__vec,__idx), 0, sizeof(VECTOR_INDEX(__vec,__idx))); /* set zeroed value */ \
		++VECTOR_LENGTH(__vec); /* increase length */ \
	}while(0)



/// Inserts a value in the target index. (using the '=' operator)
/// Assumes the index is valid and there is enough capacity.
///
/// @param __vec Vector
/// @param __idx Index
/// @param __val Value
#define VECTOR_INSERT(__vec,__idx,__val) \
	do{ \
		if( (__idx) < VECTOR_LENGTH(__vec) ) /* move data */ \
			memmove(&VECTOR_INDEX(__vec,(__idx)+1),&VECTOR_INDEX(__vec,__idx),(VECTOR_LENGTH(__vec)-(__idx))*sizeof(VECTOR_FIRST(__vec))); \
		VECTOR_INDEX(__vec,__idx) = (__val); /* set value */ \
		++VECTOR_LENGTH(__vec); /* increase length */ \
	}while(0)



/// Inserts a value in the target index. (using memcpy)
/// Assumes the index is valid and there is enough capacity.
///
/// @param __vec Vector
/// @param __idx Index
/// @param __val Value
#define VECTOR_INSERTCOPY(__vec,__idx,__val) \
	VECTOR_INSERTARRAY(__vec,__idx,&(__val),1)



/// Inserts the values of the array in the target index. (using memcpy)
/// Assumes the index is valid and there is enough capacity.
///
/// @param __vec Vector
/// @param __idx Index
/// @param __pval Array of values
/// @param __n Number of values
#define VECTOR_INSERTARRAY(__vec,__idx,__pval,__n) \
	do{ \
		if( (__idx) < VECTOR_LENGTH(__vec) ) /* move data */ \
			memmove(&VECTOR_INDEX(__vec,(__idx)+(__n)),&VECTOR_INDEX(__vec,__idx),(VECTOR_LENGTH(__vec)-(__idx))*sizeof(VECTOR_FIRST(__vec))); \
		memcpy(&VECTOR_INDEX(__vec,__idx), (__pval), (__n)*sizeof(VECTOR_FIRST(__vec))); /* set values */ \
		VECTOR_LENGTH(__vec) += (__n); /* increase length */ \
	}while(0)



/// Inserts a zeroed value in the end of the vector.
/// Assumes there is enough capacity.
///
/// @param __vec Vector
#define VECTOR_PUSHZEROED(__vec) \
	do{ \
		memset(&VECTOR_INDEX(__vec,VECTOR_LENGTH(__vec)), 0, sizeof(VECTOR_INDEX(__vec,VECTOR_LENGTH(__vec)))); /* set zeroed value */ \
		++VECTOR_LENGTH(__vec); /* increase length */ \
	}while(0)


/// Inserts a value in the end of the vector. (using the '=' operator)
/// Assumes there is enough capacity.
///
/// @param __vec Vector
/// @param __val Value
#define VECTOR_PUSH(__vec,__val) \
	do{ \
		VECTOR_INDEX(__vec,VECTOR_LENGTH(__vec)) = (__val); /* set value */ \
		++VECTOR_LENGTH(__vec); /* increase length */ \
	}while(0)



/// Inserts a value in the end of the vector. (using memcpy)
/// Assumes there is enough capacity.
///
/// @param __vec Vector
/// @param __val Value
#define VECTOR_PUSHCOPY(__vec,__val) \
	VECTOR_PUSHARRAY(__vec,&(__val),1)



/// Inserts the values of the array in the end of the vector. (using memcpy)
/// Assumes there is enough capacity.
///
/// @param __vec Vector
/// @param __pval Array of values
/// @param __n Number of values
#define VECTOR_PUSHARRAY(__vec,__pval,__n) \
	do{ \
		memcpy(&VECTOR_INDEX(__vec,VECTOR_LENGTH(__vec)), (__pval), (__n)*sizeof(VECTOR_FIRST(__vec))); /* set values */ \
		VECTOR_LENGTH(__vec) += (__n); /* increase length */ \
	}while(0)



/// Removes and returns the last value of the vector.
/// Assumes the array is not empty.
///
/// @param __vec Vector
/// @return Removed value
#define VECTOR_POP(__vec) \
	( VECTOR_INDEX(__vec,--VECTOR_LENGTH(__vec)) )



/// Removes the last N values of the vector and returns the value of the last pop.
/// Assumes there are enough values.
///
/// @param __vec Vector
/// @param __n Number of pops
/// @return Last removed value
#define VECTOR_POPN(__vec,__n) \
	( VECTOR_INDEX(__vec,(VECTOR_LENGTH(__vec)-=(__n))) )



/// Removes the target index from the vector.
/// Assumes the index is valid and there are enough values.
///
/// @param __vec Vector
/// @param __idx Index
#define VECTOR_ERASE(__vec,__idx) \
	VECTOR_ERASEN(__vec,__idx,1)



/// Removes N values from the target index of the vector.
/// Assumes the index is valid and there are enough values.
///
/// @param __vec Vector
/// @param __idx Index
/// @param __n Number of values
#define VECTOR_ERASEN(__vec,__idx,__n) \
	do{ \
		if( (__idx) < VECTOR_LENGTH(__vec)-(__n) ) /* move data */ \
			memmove(&VECTOR_INDEX(__vec,__idx),&VECTOR_INDEX(__vec,(__idx)+(__n)),(VECTOR_LENGTH(__vec)-((__idx)+(__n)))*sizeof(VECTOR_FIRST(__vec))); \
		VECTOR_LENGTH(__vec) -= (__n); /* decrease length */ \
	}while(0)



/// Clears the vector, freeing allocated data.
///
/// @param __vec Vector
#define VECTOR_CLEAR(__vec) \
	do{ \
		if( VECTOR_CAPACITY(__vec) ) \
		{ \
			aFree(VECTOR_DATA(__vec)); VECTOR_DATA(__vec) = NULL; /* clear allocated array */ \
			VECTOR_CAPACITY(__vec) = 0; /* clear capacity */ \
			VECTOR_LENGTH(__vec) = 0; /* clear length */ \
		} \
	}while(0)



/////////////////////////////////////////////////////////////////////
// Binary heap library based on defines. (uses the vector defines above)
// uses aMalloc, aRealloc, aFree
// WARNING: BHEAP implementation details affect behaviour of A* pathfinding



/// Declares an anonymous binary heap struct.
///
/// @param __type Type of data
#define BHEAP_DECL(__type) VECTOR_DECL(__type)



/// Declares a named binary heap struct.
///
/// @param __name Structure name
/// @param __type Type of data
#define BHEAP_STRUCT_DECL(__name,__type) VECTOR_STRUCT_DECL(__name,__type)



/// Declares and initializes an anonymous binary heap variable.
///
/// @param __type Type of data
/// @param __var Variable name
#define BHEAP_VAR(__type,__var) VECTOR_VAR(__type,__var)



/// Declares and initializes a named binary heap variable.
///
/// @param __name Structure name
/// @param __var Variable name
#define BHEAP_STRUCT_VAR(__name,__var) VECTOR_STRUCT_VAR(__name,__var)



/// Initializes a heap.
///
/// @param __heap Binary heap
#define BHEAP_INIT(__heap) VECTOR_INIT(__heap)



/// Returns the internal array of values.
///
/// @param __heap Binary heap
/// @return Array of values
#define BHEAP_DATA(__heap) VECTOR_DATA(__heap)



/// Returns the length of the heap.
///
/// @param __heap Binary heap
/// @return Length
#define BHEAP_LENGTH(__heap) VECTOR_LENGTH(__heap)



/// Returns the capacity of the heap.
///
/// @param __heap Binary heap
/// @return Capacity
#define BHEAP_CAPACITY(__heap) VECTOR_CAPACITY(__heap)



/// Ensures that the heap has the target number of empty positions.
/// Increases the capacity in multiples of __step.
///
/// @param __heap Binary heap
/// @param __n Empty positions
/// @param __step Increase
#define BHEAP_ENSURE(__heap,__n,__step) VECTOR_ENSURE(__heap,__n,__step)



/// Returns the top value of the heap.
/// Assumes the heap is not empty.
///
/// @param __heap Binary heap
/// @return Value at the top
#define BHEAP_PEEK(__heap) VECTOR_INDEX(__heap,0)



/// Inserts a value in the heap. (using the '=' operator)
/// Assumes there is enough capacity.
///
/// The comparator takes two values as arguments, returns:
/// - negative if the first value is on the top
/// - positive if the second value is on the top
/// - 0 if they are equal
///
/// @param __heap Binary heap
/// @param __val Value
/// @param __topcmp Comparator
/// @param __swp Swapper
#define BHEAP_PUSH(__heap,__val,__topcmp,__swp) \
	do{ \
		size_t _i_ = VECTOR_LENGTH(__heap); \
		VECTOR_PUSH(__heap,__val); /* insert at end */ \
		while( _i_ ) \
		{ /* restore heap property in parents */ \
			size_t _parent_ = (_i_-1)/2; \
			if( __topcmp(VECTOR_INDEX(__heap,_parent_),VECTOR_INDEX(__heap,_i_)) < 0 ) \
				break; /* done */ \
			__swp(VECTOR_INDEX(__heap,_parent_),VECTOR_INDEX(__heap,_i_)); \
			_i_ = _parent_; \
		} \
	}while(0)



/// See BHEAP_PUSH. Version used by A* implementation, matching client bheap.
///
/// @param __heap Binary heap
/// @param __val Value
/// @param __topcmp Comparator
/// @param __swp Swapper
#define BHEAP_PUSH2(__heap,__val,__topcmp,__swp) \
	do{ \
		size_t _i_ = VECTOR_LENGTH(__heap); \
		VECTOR_PUSH(__heap,__val); /* insert at end */ \
		BHEAP_SIFTDOWN(__heap,0,_i_,__topcmp,__swp); \
	}while(0)



/// Removes the top value of the heap. (using the '=' operator)
/// Assumes the heap is not empty.
///
/// The comparator takes two values as arguments, returns:
/// - negative if the first value is on the top
/// - positive if the second value is on the top
/// - 0 if they are equal
///
/// @param __heap Binary heap
/// @param __topcmp Comparator
/// @param __swp Swapper
#define BHEAP_POP(__heap,__topcmp,__swp) BHEAP_POPINDEX(__heap,0,__topcmp,__swp)



/// See BHEAP_POP. Version used by A* implementation, matching client bheap.
///
/// @param __heap Binary heap
/// @param __topcmp Comparator
/// @param __swp Swapper
#define BHEAP_POP2(__heap,__topcmp,__swp) \
	do{ \
		VECTOR_INDEX(__heap,0) = VECTOR_POP(__heap); /* put last at index */ \
		if( !VECTOR_LENGTH(__heap) ) /* removed last, nothing to do */ \
			break; \
		BHEAP_SIFTUP(__heap,0,__topcmp,__swp);	\
	}while(0)



/// Removes the target value of the heap. (using the '=' operator)
/// Assumes the index exists.
///
/// The comparator takes two values as arguments, returns:
/// - negative if the first value is on the top
/// - positive if the second value is on the top
/// - 0 if they are equal
///
/// @param __heap Binary heap
/// @param __idx Index
/// @param __topcmp Comparator
/// @param __swp Swapper
#define BHEAP_POPINDEX(__heap,__idx,__topcmp,__swp) \
	do{ \
		size_t _i_ = __idx; \
		VECTOR_INDEX(__heap,__idx) = VECTOR_POP(__heap); /* put last at index */ \
		if( _i_ >= VECTOR_LENGTH(__heap)) /* removed last, nothing to do */ \
			break; \
		while( _i_ ) \
		{ /* restore heap property in parents */ \
			size_t _parent_ = (_i_-1)/2; \
			if( __topcmp(VECTOR_INDEX(__heap,_parent_),VECTOR_INDEX(__heap,_i_)) < 0 ) \
				break; /* done */ \
			__swp(VECTOR_INDEX(__heap,_parent_),VECTOR_INDEX(__heap,_i_)); \
			_i_ = _parent_; \
		} \
		while( _i_ < VECTOR_LENGTH(__heap) ) \
		{ /* restore heap property in childs */ \
			size_t _lchild_ = _i_*2 + 1; \
			size_t _rchild_ = _i_*2 + 2; \
			if( (_lchild_ >= VECTOR_LENGTH(__heap) || __topcmp(VECTOR_INDEX(__heap,_i_),VECTOR_INDEX(__heap,_lchild_)) <= 0) && \
				(_rchild_ >= VECTOR_LENGTH(__heap) || __topcmp(VECTOR_INDEX(__heap,_i_),VECTOR_INDEX(__heap,_rchild_)) <= 0) ) \
				break; /* done */ \
			else if( _rchild_ >= VECTOR_LENGTH(__heap) || __topcmp(VECTOR_INDEX(__heap,_lchild_),VECTOR_INDEX(__heap,_rchild_)) <= 0 ) \
			{ /* left child */ \
				__swp(VECTOR_INDEX(__heap,_i_),VECTOR_INDEX(__heap,_lchild_)); \
				_i_ = _lchild_; \
			} \
			else \
			{ /* right child */ \
				__swp(VECTOR_INDEX(__heap,_i_),VECTOR_INDEX(__heap,_rchild_)); \
				_i_ = _rchild_; \
			} \
		} \
	}while(0)



/// Follow path up towards (but not all the way to) the root, swapping nodes until finding
/// a place where the new item that was placed at __idx fits.
/// Only goes as high as __startidx (usually 0).
///
/// @param __heap Binary heap
/// @param __startidx Index of an ancestor of __idx
/// @param __idx Index of an inserted element
/// @param __topcmp Comparator
/// @param __swp Swapper
#define BHEAP_SIFTDOWN(__heap,__startidx,__idx,__topcmp,__swp)	\
	do{ \
		size_t _i2_ = __idx; \
		while( _i2_ > __startidx ) \
		{ /* restore heap property in parents */ \
			size_t _parent_ = (_i2_-1)/2; \
			if( __topcmp(VECTOR_INDEX(__heap,_parent_),VECTOR_INDEX(__heap,_i2_)) <= 0 ) \
				break; /* done */ \
			__swp(VECTOR_INDEX(__heap,_parent_),VECTOR_INDEX(__heap,_i2_)); \
			_i2_ = _parent_; \
		} \
	}while(0)



/// Repeatedly swap the smaller child with parent, after placing a new item at __idx.
///
/// @param __heap Binary heap
/// @param __idx Index of an inserted element
/// @param __topcmp Comparator
/// @param __swp Swapper
#define BHEAP_SIFTUP(__heap,__idx,__topcmp,__swp) \
	do{ \
		size_t _i_ = __idx; \
		size_t _lchild_ = _i_*2 + 1; \
		while( _lchild_ < VECTOR_LENGTH(__heap) ) \
		{ /* restore heap property in childs */ \
			size_t _rchild_ = _i_*2 + 2; \
			if( _rchild_ >= VECTOR_LENGTH(__heap) || __topcmp(VECTOR_INDEX(__heap,_lchild_),VECTOR_INDEX(__heap,_rchild_)) < 0 ) \
			{ /* left child */ \
				__swp(VECTOR_INDEX(__heap,_i_),VECTOR_INDEX(__heap,_lchild_)); \
				_i_ = _lchild_; \
			} \
			else \
			{ /* right child */ \
				__swp(VECTOR_INDEX(__heap,_i_),VECTOR_INDEX(__heap,_rchild_)); \
				_i_ = _rchild_; \
			} \
			_lchild_ = _i_*2 + 1; \
		} \
		BHEAP_SIFTDOWN(__heap,__idx,_i_,__topcmp,__swp); \
	}while(0)



/// Call this after modifying the item at __idx__ to restore the heap
///
/// @param __heap Binary heap
/// @param __idx Index
/// @param __topcmp Comparator
/// @param __swp Swapper
#define BHEAP_UPDATE(__heap,__idx,__topcmp,__swp) \
	do{ \
		BHEAP_SIFTDOWN(__heap,0,__idx,__topcmp,__swp); \
		BHEAP_SIFTUP(__heap,__idx,__topcmp,__swp); \
	}while(0)



/// Clears the binary heap, freeing allocated data.
///
/// @param __heap Binary heap
#define BHEAP_CLEAR(__heap) VECTOR_CLEAR(__heap)



/// Generic comparator for a min-heap. (minimum value at top)
/// Returns -1 if v1 is smaller, 1 if v2 is smaller, 0 if equal.
///
/// @param v1 First value
/// @param v2 Second value
/// @return negative if v1 is top, positive if v2 is top, 0 if equal
#define BHEAP_MINTOPCMP(v1,v2) ( v1 == v2 ? 0 : v1 < v2 ? -1 : 1 )



/// Generic comparator for a max-heap. (maximum value at top)
/// Returns -1 if v1 is bigger, 1 if v2 is bigger, 0 if equal.
///
/// @param v1 First value
/// @param v2 Second value
/// @return negative if v1 is top, positive if v2 is top, 0 if equal
#define BHEAP_MAXTOPCMP(v1,v2) ( v1 == v2 ? 0 : v1 > v2 ? -1 : 1 )



#endif /* _DB_H_ */
