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
 *  - create an enum for the data (with int, unsigned int and void *)        *
 *  - create a custom database allocator                                     *
 *  - see what functions need or should be added to the database interface   *
 *                                                                           *
 *  HISTORY:                                                                 *
 *    2.1 (Athena build #???#) - Portability fix                             *
 *      - Fixed the portability of casting to union and added the functions  *
 *        {@link DB#ensure(DB,DBKey,DBCreateData,...)} and                   *
 *        {@link DB#clear(DB,DBApply,...)}.                                  *
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

#include <stdarg.h>

/*****************************************************************************\
 *  (1) Section with public typedefs, enums, unions, structures and defines. *
 *  DB_MANUAL_CAST_TO_UNION - Define when the compiler doesn't allow casting *
 *           to unions.                                                      *
 *  DBRelease    - Enumeration of release options.                           *
 *  DBType       - Enumeration of database types.                            *
 *  DBOptions    - Bitfield enumeration of database options.                 *
 *  DBKey        - Union of used key types.                                  *
 *  DBApply      - Format of functions applyed to the databases.             *
 *  DBMatcher    - Format of matchers used in DB::getall.                    *
 *  DBComparator - Format of the comparators used by the databases.          *
 *  DBHasher     - Format of the hashers used by the databases.              *
 *  DBReleaser   - Format of the releasers used by the databases.            *
 *  DB           - Database interface.                                       *
\*****************************************************************************/

/**
 * Define this to enable the functions that cast to unions.
 * Required when the compiler doesn't support casting to unions.
 * NOTE: It is recommened that the conditional tests to determine if this 
 * should be defined be located in a makefile or a header file specific for 
 * of compatibility and portability issues.
 * @public
 * @see #db_i2key(int)
 * @see #db_ui2key(unsigned int)
 * @see #db_str2key(unsigned char *)
 */
//#define DB_MANUAL_CAST_TO_UNION

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
 *          WARNING: for funtions that return the data (like DB::remove),
 *          a dangling pointer will be returned.
 * @param DB_OPT_RELEASE_BOTH Releases both key and data.
 * @param DB_OPT_ALLOW_NULL_KEY Allow NULL keys in the database.
 * @param DB_OPT_ALLOW_NULL_DATA Allow NULL data in the database.
 * @public
 * @see #db_fix_options(DBType,DBOptions)
 * @see #db_default_release(DBType,DBOptions)
 * @see #db_alloc(const char *,int,DBType,DBOptions,unsigned short)
 */
typedef enum db_opt {
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
 * @see DB#get
 * @see DB#put
 * @see DB#remove
 */
typedef union dbkey {
	int i;
	unsigned int ui;
	const char *str;
} DBKey;

/**
 * Format of funtions that create the data for the key when the entry doesn't 
 * exist in the database yet.
 * @param key Key of the database entry
 * @param args Extra arguments of the funtion
 * @return Data identified by the key to be put in the database
 * @public
 * @see DB#vensure
 * @see DB#ensure
 */
typedef void *(*DBCreateData)(DBKey key, va_list args);

/**
 * Format of functions to be applyed to an unspecified quantity of entries of 
 * a database.
 * Any function that applyes this function to the database will return the sum 
 * of values returned by this function.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param args Extra arguments of the funtion
 * @return Value to be added up by the funtion that is applying this
 * @public
 * @see DB#vforeach
 * @see DB#foreach
 * @see DB#vdestroy
 * @see DB#destroy
 */
typedef int (*DBApply)(DBKey key, void *data, va_list args);

/**
 * Format of functions that match database entries.
 * The purpose of the match depends on the function that is calling the matcher.
 * Returns 0 if it is a match, another number otherwise.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param args Extra arguments of the function
 * @return 0 if a match, another number otherwise
 * @public
 * @see DB#getall
 */
typedef int (*DBMatcher)(DBKey key, void *data, va_list args);

/**
 * Format of the comparators used internally by the database system.
 * Compares key1 to key2.
 * <code>maxlen</code> is the maximum number of character used in DB_STRING and 
 * DB_ISTRING databases. If 0, the maximum number of maxlen is used (64K).
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
 * <code>maxlen</code> is the maximum number of character used in DB_STRING and 
 * DB_ISTRING databases. If 0, the maximum number of maxlen is used (64K).
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
typedef void (*DBReleaser)(DBKey key, void *data, DBRelease which);

/**
 * Public interface of a database. Only contains funtions.
 * All the functions take the interface as the first argument.
 * @public
 * @see DB#get(DB,DBKey)
 * @see DB#getall(DB,void **,unsigned int,DBMatch,...)
 * @see DB#vgetall(DB,void **,unsigned int,DBMatch,va_list)
 * @see DB#put(DB,DBKey,void *)
 * @see DB#remove(DB,DBKey)
 * @see DB#foreach(DB,DBApply,...)
 * @see DB#vforeach(DB,DBApply,va_list)
 * @see DB#destroy(DB,DBApply,...)
 * @see DB#destroy(DB,DBApply,va_list)
 * @see DB#size(DB)
 * @see DB#type(DB)
 * @see DB#options(DB)
 * @see #db_alloc(const char *,int,DBType,DBOptions,unsigned short)
 */
typedef struct dbt *DB;
struct dbt {

	/**
	 * Get the data of the entry identifid by the key.
	 * @param dbi Interface of the database
	 * @param key Key that identifies the entry
	 * @return Data of the entry or NULL if not found
	 * @protected
	 * @see #db_get(DB,DBKey)
	 */
	void *(*get)(DB self, DBKey key);

	/**
	 * Just calls {@link DB#vgetall(DB,void **,unsigned int,DBMatch,va_list)}.
	 * Get the data of the entries matched by <code>match</code>.
	 * It puts a maximum of <code>max</code> entries into <code>buf</code>.
	 * If <code>buf</code> is NULL, it only counts the matches.
	 * Returns the number of entries that matched.
	 * NOTE: if the value returned is greater than <code>max</code>, only the 
	 * first <code>max</code> entries found are put into the buffer.
	 * @param dbi Interface of the database
	 * @param buf Buffer to put the data of the matched entries
	 * @param max Maximum number of data entries to be put into buf
	 * @param match Function that matches the database entries
	 * @param ... Extra arguments for match
	 * @return The number of entries that matched
	 * @protected
	 * @see DB#vgetall
	 * @see #db_getall(DB,void **,unsigned int,DBMatch,...)
	 */
	unsigned int (*getall)(DB self, void **buf, unsigned int max, DBMatcher match, ...);

	/**
	 * Get the data of the entries matched by <code>match</code>.
	 * It puts a maximum of <code>max</code> entries into <code>buf</code>.
	 * If <code>buf</code> is NULL, it only counts the matches.
	 * Returns the number of entries that matched.
	 * NOTE: if the value returned is greater than <code>max</code>, only the 
	 * first <code>max</code> entries found are put into the buffer.
	 * @param dbi Interface of the database
	 * @param buf Buffer to put the data of the matched entries
	 * @param max Maximum number of data entries to be put into buf
	 * @param match Function that matches the database entries
	 * @param ... Extra arguments for match
	 * @return The number of entries that matched
	 * @protected
	 * @see DB#getall
	 * @see #db_vgetall(DB,void **,unsigned int,DBMatch,va_list)
	 */
	unsigned int (*vgetall)(DB self, void **buf, unsigned int max, DBMatcher match, va_list args);

	/**
	 * Just calls {@link common\db.h\DB#vensure(DB,DBKey,DBCreateData,va_list)}.
	 * Get the data of the entry identified by the key.
	 * If the entry does not exist, an entry is added with the data returned by 
	 * <code>create</code>.
	 * @param dbi Interface of the database
	 * @param key Key that identifies the entry
	 * @param create Function used to create the data if the entry doesn't exist
	 * @param ... Extra arguments for create
	 * @return Data of the entry
	 * @protected
	 * @see DB#vensure(DB,DBKey,DBCreateData,va_list)
	 * @see #db_ensure(DB,DBKey,DBCreateData,...)
	 */
	void *(*ensure)(DB self, DBKey key, DBCreateData create, ...);

	/**
	 * Get the data of the entry identified by the key.
	 * If the entry does not exist, an entry is added with the data returned by 
	 * <code>create</code>.
	 * @param dbi Interface of the database
	 * @param key Key that identifies the entry
	 * @param create Function used to create the data if the entry doesn't exist
	 * @param args Extra arguments for create
	 * @return Data of the entry
	 * @protected
	 * @see DB#ensure(DB,DBKey,DBCreateData,...)
	 * @see #db_vensure(DB,DBKey,DBCreateData,va_list)
	 */
	void *(*vensure)(DB self, DBKey key, DBCreateData create, va_list args);

	/**
	 * Put the data identified by the key in the database.
	 * Returns the previous data if the entry exists or NULL.
	 * NOTE: Uses the new key, the old one is released.
	 * @param dbi Interface of the database
	 * @param key Key that identifies the data
	 * @param data Data to be put in the database
	 * @return The previous data if the entry exists or NULL
	 * @protected
	 * @see #db_put(DB,DBKey,void *)
	 */
	void *(*put)(DB self, DBKey key, void *data);

	/**
	 * Remove an entry from the database.
	 * Returns the data of the entry.
	 * NOTE: The key (of the database) is released.
	 * @param dbi Interface of the database
	 * @param key Key that identifies the entry
	 * @return The data of the entry or NULL if not found
	 * @protected
	 * @see #db_remove(DB,DBKey)
	 */
	void *(*remove)(DB self, DBKey key);

	/**
	 * Just calls {@link DB#vforeach(DB,DBApply,va_list)}.
	 * Apply <code>func</code> to every entry in the database.
	 * Returns the sum of values returned by func.
	 * @param dbi Interface of the database
	 * @param func Function to be applyed
	 * @param ... Extra arguments for func
	 * @return Sum of the values returned by func
	 * @protected
	 * @see DB#vforeach
	 * @see #db_foreach(DB,DBApply,...)
	 */
	int (*foreach)(DB self, DBApply func, ...);

	/**
	 * Apply <code>func</code> to every entry in the database.
	 * Returns the sum of values returned by func.
	 * @param dbi Interface of the database
	 * @param func Function to be applyed
	 * @param args Extra arguments for func
	 * @return Sum of the values returned by func
	 * @protected
	 * @see DB#foreach
	 * @see #db_vforeach(DB,DBApply,va_list)
	 */
	int (*vforeach)(DB self, DBApply func, va_list args);

	/**
	 * Just calls {@link DB#vclear(DB,DBApply,va_list)}.
	 * Removes all entries from the database.
	 * Before deleting an entry, func is applyed to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * @param dbi Interface of the database
	 * @param func Function to be applyed to every entry before deleting
	 * @param ... Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see DB#vclear
	 * @see #db_clear(DB,DBApply,...)
	 */
	int (*clear)(DB self, DBApply func, ...);

	/**
	 * Removes all entries from the database.
	 * Before deleting an entry, func is applyed to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * @param dbi Interface of the database
	 * @param func Function to be applyed to every entry before deleting
	 * @param args Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see DB#clear
	 * @see #vclear(DB,DBApply,va_list)
	 */
	int (*vclear)(DB self, DBApply func, va_list args);

	/**
	 * Just calls {@link DB#vdestroy(DB,DBApply,va_list)}.
	 * Finalize the database, feeing all the memory it uses.
	 * Before deleting an entry, func is applyed to it.
	 * Releases the key and the data.
	 * Returns the sum of values returned by func, if it exists.
	 * NOTE: This locks the database globally. Any attempt to insert or remove 
	 * a database entry will give an error and be aborted (except for clearing).
	 * @param dbi Interface of the database
	 * @param func Function to be applyed to every entry before deleting
	 * @param ... Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see DB#vdestroy
	 * @see #db_destroy(DB,DBApply,...)
	 */
	int (*destroy)(DB self, DBApply func, ...);

	/**
	 * Finalize the database, feeing all the memory it uses.
	 * Before deleting an entry, func is applyed to it.
	 * Returns the sum of values returned by func, if it exists.
	 * NOTE: This locks the database globally. Any attempt to insert or remove 
	 * a database entry will give an error and be aborted (except for clearing).
	 * @param dbi Interface of the database
	 * @param func Function to be applyed to every entry before deleting
	 * @param args Extra arguments for func
	 * @return Sum of values returned by func
	 * @protected
	 * @see DB#destroy
	 * @see #db_vdestroy(DB,DBApply,va_list)
	 */
	int (*vdestroy)(DB self, DBApply func, va_list args);

	/**
	 * Return the size of the database (number of items in the database).
	 * @param dbi Interface of the database
	 * @return Size of the database
	 * @protected
	 * @see #db_size(DB)
	 */
	unsigned int (*size)(DB self);

	/**
	 * Return the type of the database.
	 * @param dbi Interface of the database
	 * @return Type of the database
	 * @protected
	 * @see #db_type(DB)
	 */
	DBType (*type)(DB self);

	/**
	 * Return the options of the database.
	 * @param dbi Interface of the database
	 * @return Options of the database
	 * @protected
	 * @see #db_options(DB)
	 */
	DBOptions (*options)(DB self);

};

//For easy access to the common functions.
#ifdef DB_MANUAL_CAST_TO_UNION
#	define i2key   db_i2key
#	define ui2key  db_ui2key
#	define str2key db_str2key
#else /* not DB_MANUAL_CAST_TO_UNION */
#	define i2key(k)   ((DBKey)(int)(k))
#	define ui2key(k)  ((DBKey)(unsigned int)(k))
#	define str2key(k) ((DBKey)(const char *)(k))
#endif /* not DB_MANUAL_CAST_TO_UNION */

#define db_get(db,k)    (db)->get((db),(k))
#define idb_get(db,k)   (db)->get((db),i2key(k))
#define uidb_get(db,k)  (db)->get((db),ui2key(k))
#define strdb_get(db,k) (db)->get((db),str2key(k))

#define db_put(db,k,d)    (db)->put((db),(k),(d))
#define idb_put(db,k,d)   (db)->put((db),i2key(k),(d))
#define uidb_put(db,k,d)  (db)->put((db),ui2key(k),(d))
#define strdb_put(db,k,d) (db)->put((db),str2key(k),(d))

#define db_remove(db,k)    (db)->remove((db),(k))
#define idb_remove(db,k)   (db)->remove((db),i2key(k))
#define uidb_remove(db,k)  (db)->remove((db),ui2key(k))
#define strdb_remove(db,k) (db)->remove((db),str2key(k))

//These are discarding the possible vargs you could send to the function, so those
//that require vargs must not use these defines.
#define db_ensure(db,k,f)    (db)->ensure((db),(k),f)
#define idb_ensure(db,k,f)   (db)->ensure((db),i2key(k),f)
#define uidb_ensure(db,k,f)  (db)->ensure((db),ui2key(k),f)
#define strdb_ensure(db,k,f) (db)->ensure((db),str2key(k),f)

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
 *  db_init            - Initialise the database system.                     *
 *  db_final           - Finalise the database system.                       *
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
 * @see common\db.c#db_fix_options(DBType,DBOptions)
 */
DBOptions db_fix_options(DBType type, DBOptions options);

/**
 * Returns the default comparator for the type of database.
 * @param type Type of database
 * @return Comparator for the type of database or NULL if unknown database
 * @public
 * @see #DBType
 * @see #DBComparator
 * @see common\db.c#db_default_cmp(DBType)
 */
DBComparator db_default_cmp(DBType type);

/**
 * Returns the default hasher for the specified type of database.
 * @param type Type of database
 * @return Hasher of the type of database or NULL if unknown database
 * @public
 * @see #DBType
 * @see #DBHasher
 * @see common\db.c#db_default_hash(DBType)
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
 * @see common\db.c#db_default_release(DBType,DBOptions)
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
 * @see common\db.c#db_custom_release(DBRelease)
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
 *          databases
 * @return The interface of the database
 * @public
 * @see #DBType
 * @see #DB
 * @see #db_default_cmp(DBType)
 * @see #db_default_hash(DBType)
 * @see #db_default_release(DBType,DBOptions)
 * @see #db_fix_options(DBType,DBOptions)
 * @see common\db.c#db_alloc(const char *,int,DBType,DBOptions,unsigned short)
 */
DB db_alloc(const char *file, int line, DBType type, DBOptions options, unsigned short maxlen);

#ifdef DB_MANUAL_CAST_TO_UNION
/**
 * Manual cast from 'int' to the union DBKey.
 * Created for compilers that don't support casting to unions.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 * @see #DB_MANUAL_CAST_TO_UNION
 */
DBKey db_i2key(int key);

/**
 * Manual cast from 'unsigned int' to the union DBKey.
 * Created for compilers that don't support casting to unions.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 * @see #DB_MANUAL_CAST_TO_UNION
 */
DBKey db_ui2key(unsigned int key);

/**
 * Manual cast from 'unsigned char *' to the union DBKey.
 * Created for compilers that don't support casting to unions.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 * @see #DB_MANUAL_CAST_TO_UNION
 */
DBKey db_str2key(const char *key);
#endif /* DB_MANUAL_CAST_TO_UNION */

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

void  linkdb_insert ( struct linkdb_node** head, void *key, void* data); // 重複を考慮しない
void  linkdb_replace( struct linkdb_node** head, void *key, void* data); // 重複を考慮する
void* linkdb_search ( struct linkdb_node** head, void *key);
void* linkdb_erase  ( struct linkdb_node** head, void *key);
void  linkdb_final  ( struct linkdb_node** head );

#endif /* _DB_H_ */
