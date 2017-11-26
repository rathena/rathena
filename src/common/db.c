/*****************************************************************************\
 *  Copyright (c) Athena Dev Teams - Licensed under GNU GPL
 *  For more information, see LICENCE in the main folder
 *
 *  This file is separated in five sections:
 *  (1) Private typedefs, enums, structures, defines and global variables
 *  (2) Private functions
 *  (3) Protected functions used internally
 *  (4) Protected functions used in the interface of the database
 *  (5) Public functions
 *
 *  The databases are structured as a hashtable of RED-BLACK trees.
 *
 *  <B>Properties of the RED-BLACK trees being used:</B>
 *  1. The value of any node is greater than the value of its left child and
 *     less than the value of its right child.
 *  2. Every node is colored either RED or BLACK.
 *  3. Every red node that is not a leaf has only black children.
 *  4. Every path from the root to a leaf contains the same number of black
 *     nodes.
 *  5. The root node is black.
 *  An <code>n</code> node in a RED-BLACK tree has the property that its
 *  height is <code>O(lg(n))</code>.
 *  Another important property is that after adding a node to a RED-BLACK
 *  tree, the tree can be readjusted in <code>O(lg(n))</code> time.
 *  Similarly, after deleting a node from a RED-BLACK tree, the tree can be
 *  readjusted in <code>O(lg(n))</code> time.
 *  {@link http://www.cs.mcgill.ca/~cs251/OldCourses/1997/topic18/}
 *
 *  <B>How to add new database types:</B>
 *  1. Add the identifier of the new database type to the enum DBType
 *  2. If not already there, add the data type of the key to the union DBKey
 *  3. If the key can be considered NULL, update the function db_is_key_null
 *  4. If the key can be duplicated, update the functions db_dup_key and
 *     db_dup_key_free
 *  5. Create a comparator and update the function db_default_cmp
 *  6. Create a hasher and update the function db_default_hash
 *  7. If the new database type requires or does not support some options,
 *     update the function db_fix_options
 *
 *  TODO:
 *  - create test cases to test the database system thoroughly
 *  - finish this header describing the database system
 *  - create custom database allocator
 *  - make the system thread friendly
 *  - change the structure of the database to T-Trees
 *  - create a db that organizes itself by splaying
 *
 *  HISTORY:
 *    2013/08/25 - Added int64/uint64 support for keys [Ind/Hercules]
 *    2013/04/27 - Added ERS to speed up iterator memory allocation [Ind/Hercules]
 *    2012/03/09 - Added enum for data types (int, uint, void*)
 *    2008/02/19 - Fixed db_obj_get not handling deleted entries correctly.
 *    2007/11/09 - Added an iterator to the database.
 *    2006/12/21 - Added 1-node cache to the database.
 *    2.1 (Athena build #???#) - Portability fix
 *      - Fixed the portability of casting to union and added the functions
 *        ensure and clear to the database.
 *    2.0 (Athena build 4859) - Transition version
 *      - Almost everything recoded with a strategy similar to objects,
 *        database structure is maintained.
 *    1.0 (up to Athena build 4706)
 *      - Previous database system.
 *
 * @version 2006/12/21
 * @author Athena Dev team
 * @encoding US-ASCII
 * @see #db.h
\*****************************************************************************/

#include "db.h"

#include "ers.h"
#include "malloc.h"
#include "mmo.h"
#include "showmsg.h"
#include "strlib.h"

#include <stdio.h>
#include <stdlib.h>

/*****************************************************************************\
 *  (1) Private typedefs, enums, structures, defines and global variables of *
 *  the database system.                                                     *
 *  DB_ENABLE_STATS - Define to enable database statistics.                  *
 *  HASH_SIZE       - Define with the size of the hashtable.                 *
 *  DBNColor        - Enumeration of colors of the nodes.                    *
 *  DBNode          - Structure of a node in RED-BLACK trees.                *
 *  struct db_free  - Structure that holds a deleted node to be freed.       *
 *  DBMap_impl      - Structure of the database.                             *
 *  stats           - Statistics about the database system.                  *
\*****************************************************************************/

/**
 * If defined statistics about database nodes, database creating/destruction
 * and function usage are kept and displayed when finalizing the database
 * system.
 * WARNING: This adds overhead to every database operation (not sure how much).
 * @private
 * @see #DBStats
 * @see #stats
 * @see #db_final(void)
 */
//#define DB_ENABLE_STATS

/**
 * Size of the hashtable in the database.
 * @private
 * @see DBMap_impl#ht
 */
#define HASH_SIZE (256+27)

/**
 * The color of individual nodes.
 * @private
 * @see struct dbn
 */
typedef enum node_color {
	RED,
	BLACK
} node_color;

/**
 * A node in a RED-BLACK tree of the database.
 * @param parent Parent node
 * @param left Left child node
 * @param right Right child node
 * @param key Key of this database entry
 * @param data Data of this database entry
 * @param deleted If the node is deleted
 * @param color Color of the node
 * @private
 * @see DBMap_impl#ht
 */
typedef struct dbn {
	// Tree structure
	struct dbn *parent;
	struct dbn *left;
	struct dbn *right;
	// Node data
	DBKey key;
	DBData data;
	// Other
	node_color color;
	unsigned deleted : 1;
} DBNode;

/**
 * Structure that holds a deleted node.
 * @param node Deleted node
 * @param root Address to the root of the tree
 * @private
 * @see DBMap_impl#free_list
 */
struct db_free {
	DBNode *node;
	DBNode **root;
};

/**
 * Complete database structure.
 * @param vtable Interface of the database
 * @param alloc_file File where the database was allocated
 * @param alloc_line Line in the file where the database was allocated
 * @param free_list Array of deleted nodes to be freed
 * @param free_count Number of deleted nodes in free_list
 * @param free_max Current maximum capacity of free_list
 * @param free_lock Lock for freeing the nodes
 * @param nodes Manager of reusable tree nodes
 * @param cmp Comparator of the database
 * @param hash Hasher of the database
 * @param release Releaser of the database
 * @param ht Hashtable of RED-BLACK trees
 * @param type Type of the database
 * @param options Options of the database
 * @param item_count Number of items in the database
 * @param maxlen Maximum length of strings in DB_STRING and DB_ISTRING databases
 * @param global_lock Global lock of the database
 * @private
 * @see #db_alloc(const char*,int,DBType,DBOptions,unsigned short)
 */
typedef struct DBMap_impl {
	// Database interface
	struct DBMap vtable;
	// File and line of allocation
	const char *alloc_file;
	int alloc_line;
	// Lock system
	struct db_free *free_list;
	unsigned int free_count;
	unsigned int free_max;
	unsigned int free_lock;
	// Other
	ERS *nodes;
	DBComparator cmp;
	DBHasher hash;
	DBReleaser release;
	DBNode *ht[HASH_SIZE];
	DBNode *cache;
	DBType type;
	DBOptions options;
	uint32 item_count;
	unsigned short maxlen;
	unsigned global_lock : 1;
} DBMap_impl;

/**
 * Complete iterator structure.
 * @param vtable Interface of the iterator
 * @param db Parent database
 * @param ht_index Current index of the hashtable
 * @param node Current node
 * @private
 * @see #DBIterator
 * @see #DBMap_impl
 * @see #DBNode
 */
typedef struct DBIterator_impl {
	// Iterator interface
	struct DBIterator vtable;
	DBMap_impl* db;
	int ht_index;
	DBNode *node;
} DBIterator_impl;

#if defined(DB_ENABLE_STATS)
/**
 * Structure with what is counted when the database statistics are enabled.
 * @private
 * @see #DB_ENABLE_STATS
 * @see #stats
 */
static struct db_stats {
	// Node alloc/free
	uint32 db_node_alloc;
	uint32 db_node_free;
	// Database creating/destruction counters
	uint32 db_int_alloc;
	uint32 db_uint_alloc;
	uint32 db_string_alloc;
	uint32 db_istring_alloc;
	uint32 db_int64_alloc;
	uint32 db_uint64_alloc;
	uint32 db_int_destroy;
	uint32 db_uint_destroy;
	uint32 db_string_destroy;
	uint32 db_istring_destroy;
	uint32 db_int64_destroy;
	uint32 db_uint64_destroy;
	// Function usage counters
	uint32 db_rotate_left;
	uint32 db_rotate_right;
	uint32 db_rebalance;
	uint32 db_rebalance_erase;
	uint32 db_is_key_null;
	uint32 db_dup_key;
	uint32 db_dup_key_free;
	uint32 db_free_add;
	uint32 db_free_remove;
	uint32 db_free_lock;
	uint32 db_free_unlock;
	uint32 db_int_cmp;
	uint32 db_uint_cmp;
	uint32 db_string_cmp;
	uint32 db_istring_cmp;
	uint32 db_int64_cmp;
	uint32 db_uint64_cmp;
	uint32 db_int_hash;
	uint32 db_uint_hash;
	uint32 db_string_hash;
	uint32 db_istring_hash;
	uint32 db_int64_hash;
	uint32 db_uint64_hash;
	uint32 db_release_nothing;
	uint32 db_release_key;
	uint32 db_release_data;
	uint32 db_release_both;
	uint32 dbit_first;
	uint32 dbit_last;
	uint32 dbit_next;
	uint32 dbit_prev;
	uint32 dbit_exists;
	uint32 dbit_remove;
	uint32 dbit_destroy;
	uint32 db_iterator;
	uint32 db_exists;
	uint32 db_get;
	uint32 db_getall;
	uint32 db_vgetall;
	uint32 db_ensure;
	uint32 db_vensure;
	uint32 db_put;
	uint32 db_remove;
	uint32 db_foreach;
	uint32 db_vforeach;
	uint32 db_clear;
	uint32 db_vclear;
	uint32 db_destroy;
	uint32 db_vdestroy;
	uint32 db_size;
	uint32 db_type;
	uint32 db_options;
	uint32 db_fix_options;
	uint32 db_default_cmp;
	uint32 db_default_hash;
	uint32 db_default_release;
	uint32 db_custom_release;
	uint32 db_alloc;
	uint32 db_i2key;
	uint32 db_ui2key;
	uint32 db_str2key;
	uint32 db_i642key;
	uint32 db_ui642key;
	uint32 db_i2data;
	uint32 db_ui2data;
	uint32 db_ptr2data;
	uint32 db_data2i;
	uint32 db_data2ui;
	uint32 db_data2ptr;
	uint32 db_init;
	uint32 db_final;
} stats = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0
};
#define DB_COUNTSTAT(token) do { if ((stats.token) != UINT32_MAX) ++(stats.token); } while(0)
#else /* !defined(DB_ENABLE_STATS) */
#define DB_COUNTSTAT(token)
#endif /* !defined(DB_ENABLE_STATS) */

/* [Ind/Hercules] */
struct eri *db_iterator_ers;
struct eri *db_alloc_ers;

/*****************************************************************************\
 *  (2) Section of private functions used by the database system.            *
 *  db_rotate_left     - Rotate a tree node to the left.                     *
 *  db_rotate_right    - Rotate a tree node to the right.                    *
 *  db_rebalance       - Rebalance the tree.                                 *
 *  db_rebalance_erase - Rebalance the tree after a BLACK node was erased.   *
 *  db_is_key_null     - Returns not 0 if the key is considered NULL.        *
 *  db_dup_key         - Duplicate a key for internal use.                   *
 *  db_dup_key_free    - Free the duplicated key.                            *
 *  db_free_add        - Add a node to the free_list of a database.          *
 *  db_free_remove     - Remove a node from the free_list of a database.     *
 *  db_free_lock       - Increment the free_lock of a database.              *
 *  db_free_unlock     - Decrement the free_lock of a database.              *
 *         If it was the last lock, frees the nodes in free_list.            *
 *         NOTE: Keeps the database trees balanced.                          *
\*****************************************************************************/

/**
 * Rotate a node to the left.
 * @param node Node to be rotated
 * @param root Pointer to the root of the tree
 * @private
 * @see #db_rebalance(DBNode *,DBNode **)
 * @see #db_rebalance_erase(DBNode *,DBNode **)
 */
static void db_rotate_left(DBNode *node, DBNode **root)
{
	DBNode *y = node->right;

	DB_COUNTSTAT(db_rotate_left);
	// put the left of y at the right of node
	node->right = y->left;
	if (y->left)
		y->left->parent = node;
	y->parent = node->parent;
	// link y and node's parent
	if (node == *root) {
		*root = y; // node was root
	} else if (node == node->parent->left) {
		node->parent->left = y; // node was at the left
	} else {
		node->parent->right = y; // node was at the right
	}
	// put node at the left of y
	y->left = node;
	node->parent = y;
}

/**
 * Rotate a node to the right
 * @param node Node to be rotated
 * @param root Pointer to the root of the tree
 * @private
 * @see #db_rebalance(DBNode *,DBNode **)
 * @see #db_rebalance_erase(DBNode *,DBNode **)
 */
static void db_rotate_right(DBNode *node, DBNode **root)
{
	DBNode *y = node->left;

	DB_COUNTSTAT(db_rotate_right);
	// put the right of y at the left of node
	node->left = y->right;
	if (y->right != 0)
		y->right->parent = node;
	y->parent = node->parent;
	// link y and node's parent
	if (node == *root) {
		*root = y; // node was root
	} else if (node == node->parent->right) {
		node->parent->right = y; // node was at the right
	} else {
		node->parent->left = y; // node was at the left
	}
	// put node at the right of y
	y->right = node;
	node->parent = y;
}

/**
 * Rebalance the RED-BLACK tree.
 * Called when the node and it's parent are both RED.
 * @param node Node to be rebalanced
 * @param root Pointer to the root of the tree
 * @private
 * @see #db_rotate_left(DBNode *,DBNode **)
 * @see #db_rotate_right(DBNode *,DBNode **)
 * @see #db_obj_put(DBMap*,DBKey,DBData)
 */
static void db_rebalance(DBNode *node, DBNode **root)
{
	DBNode *y;

	DB_COUNTSTAT(db_rebalance);
	// Restore the RED-BLACK properties
	node->color = RED;
	while (node != *root && node->parent->color == RED) {
		if (node->parent == node->parent->parent->left) {
			// If node's parent is a left, y is node's right 'uncle'
			y = node->parent->parent->right;
			if (y && y->color == RED) { // case 1
				// change the colors and move up the tree
				node->parent->color = BLACK;
				y->color = BLACK;
				node->parent->parent->color = RED;
				node = node->parent->parent;
			} else {
				if (node == node->parent->right) { // case 2
					// move up and rotate
					node = node->parent;
					db_rotate_left(node, root);
				}
				// case 3
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				db_rotate_right(node->parent->parent, root);
			}
		} else {
			// If node's parent is a right, y is node's left 'uncle'
			y = node->parent->parent->left;
			if (y && y->color == RED) { // case 1
				// change the colors and move up the tree
				node->parent->color = BLACK;
				y->color = BLACK;
				node->parent->parent->color = RED;
				node = node->parent->parent;
			} else {
				if (node == node->parent->left) { // case 2
					// move up and rotate
					node = node->parent;
					db_rotate_right(node, root);
				}
				// case 3
				node->parent->color = BLACK;
				node->parent->parent->color = RED;
				db_rotate_left(node->parent->parent, root);
			}
		}
	}
	(*root)->color = BLACK; // the root can and should always be black
}

/**
 * Erase a node from the RED-BLACK tree, keeping the tree balanced.
 * @param node Node to be erased from the tree
 * @param root Root of the tree
 * @private
 * @see #db_rotate_left(DBNode *,DBNode **)
 * @see #db_rotate_right(DBNode *,DBNode **)
 * @see #db_free_unlock(DBMap_impl*)
 */
static void db_rebalance_erase(DBNode *node, DBNode **root)
{
	DBNode *y = node;
	DBNode *x = NULL;
	DBNode *x_parent = NULL;

	DB_COUNTSTAT(db_rebalance_erase);
	// Select where to change the tree
	if (y->left == NULL) { // no left
		x = y->right;
	} else if (y->right == NULL) { // no right
		x = y->left;
	} else { // both exist, go to the leftmost node of the right sub-tree
		y = y->right;
		while (y->left != NULL)
			y = y->left;
		x = y->right;
	}

	// Remove the node from the tree
	if (y != node) { // both child existed
		// put the left of 'node' in the left of 'y'
		node->left->parent = y;
		y->left = node->left;

		// 'y' is not the direct child of 'node'
		if (y != node->right) {
			// put 'x' in the old position of 'y'
			x_parent = y->parent;
			if (x) x->parent = y->parent;
			y->parent->left = x;
			// put the right of 'node' in 'y'
			y->right = node->right;
			node->right->parent = y;
			// 'y' is a direct child of 'node'
		} else {
			x_parent = y;
		}

		// link 'y' and the parent of 'node'
		if (*root == node) {
			*root = y; // 'node' was the root
		} else if (node->parent->left == node) {
			node->parent->left = y; // 'node' was at the left
		} else {
			node->parent->right = y; // 'node' was at the right
		}
		y->parent = node->parent;
		// switch colors
		{
			node_color tmp = y->color;
			y->color = node->color;
			node->color = tmp;
		}
		y = node;
	} else { // one child did not exist
		// put x in node's position
		x_parent = y->parent;
		if (x) x->parent = y->parent;
		// link x and node's parent
		if (*root == node) {
			*root = x; // node was the root
		} else if (node->parent->left == node) {
			node->parent->left = x; // node was at the left
		} else {
			node->parent->right = x;  // node was at the right
		}
	}

	// Restore the RED-BLACK properties
	if (y->color != RED) {
		while (x != *root && (x == NULL || x->color == BLACK)) {
			DBNode *w;
			if (x == x_parent->left) {
				w = x_parent->right;
				if (w->color == RED) {
					w->color = BLACK;
					x_parent->color = RED;
					db_rotate_left(x_parent, root);
					w = x_parent->right;
				}
				if ((w->left == NULL || w->left->color == BLACK) &&
					(w->right == NULL || w->right->color == BLACK)) {
					w->color = RED;
					x = x_parent;
					x_parent = x_parent->parent;
				} else {
					if (w->right == NULL || w->right->color == BLACK) {
						if (w->left) w->left->color = BLACK;
						w->color = RED;
						db_rotate_right(w, root);
						w = x_parent->right;
					}
					w->color = x_parent->color;
					x_parent->color = BLACK;
					if (w->right) w->right->color = BLACK;
					db_rotate_left(x_parent, root);
					break;
				}
			} else {
				w = x_parent->left;
				if (w->color == RED) {
					w->color = BLACK;
					x_parent->color = RED;
					db_rotate_right(x_parent, root);
					w = x_parent->left;
				}
				if ((w->right == NULL || w->right->color == BLACK) &&
					(w->left == NULL || w->left->color == BLACK)) {
					w->color = RED;
					x = x_parent;
					x_parent = x_parent->parent;
				} else {
					if (w->left == NULL || w->left->color == BLACK) {
						if (w->right) w->right->color = BLACK;
						w->color = RED;
						db_rotate_left(w, root);
						w = x_parent->left;
					}
					w->color = x_parent->color;
					x_parent->color = BLACK;
					if (w->left) w->left->color = BLACK;
					db_rotate_right(x_parent, root);
					break;
				}
			}
		}
		if (x) x->color = BLACK;
	}
}

/**
 * Returns not 0 if the key is considered to be NULL.
 * @param type Type of database
 * @param key Key being tested
 * @return not 0 if considered NULL, 0 otherwise
 * @private
 * @see #db_obj_get(DBMap*,DBKey)
 * @see #db_obj_put(DBMap*,DBKey,DBData)
 * @see #db_obj_remove(DBMap*,DBKey)
 */
static int db_is_key_null(DBType type, DBKey key)
{
	DB_COUNTSTAT(db_is_key_null);
	switch (type) {
		case DB_STRING:
		case DB_ISTRING:
			return (key.str == NULL);

		default: // Not a pointer
			return 0;
	}
}

/**
 * Duplicate the key used in the database.
 * @param db Database the key is being used in
 * @param key Key to be duplicated
 * @param Duplicated key
 * @private
 * @see #db_free_add(DBMap_impl*,DBNode *,DBNode **)
 * @see #db_free_remove(DBMap_impl*,DBNode *)
 * @see #db_obj_put(DBMap*,DBKey,void *)
 * @see #db_dup_key_free(DBMap_impl*,DBKey)
 */
static DBKey db_dup_key(DBMap_impl* db, DBKey key)
{
	char *str;
	size_t len;

	DB_COUNTSTAT(db_dup_key);
	switch (db->type) {
		case DB_STRING:
		case DB_ISTRING:
			len = strnlen(key.str, db->maxlen);
			str = (char*)aMalloc(len + 1);
			memcpy(str, key.str, len);
			str[len] = '\0';
			key.str = str;
			return key;

		default:
			return key;
	}
}

/**
 * Free a key duplicated by db_dup_key.
 * @param db Database the key is being used in
 * @param key Key to be freed
 * @private
 * @see #db_dup_key(DBMap_impl*,DBKey)
 */
static void db_dup_key_free(DBMap_impl* db, DBKey key)
{
	DB_COUNTSTAT(db_dup_key_free);
	switch (db->type) {
		case DB_STRING:
		case DB_ISTRING:
			aFree((char*)key.str);
			return;

		default:
			return;
	}
}

/**
 * Add a node to the free_list of the database.
 * Marks the node as deleted.
 * If the key isn't duplicated, the key is duplicated and released.
 * @param db Target database
 * @param root Root of the tree from the node
 * @param node Target node
 * @private
 * @see #struct db_free
 * @see DBMap_impl#free_list
 * @see DBMap_impl#free_count
 * @see DBMap_impl#free_max
 * @see #db_obj_remove(DBMap*,DBKey)
 * @see #db_free_remove(DBMap_impl*,DBNode *)
 */
static void db_free_add(DBMap_impl* db, DBNode *node, DBNode **root)
{
	DBKey old_key;

	DB_COUNTSTAT(db_free_add);
	if (db->free_lock == (unsigned int)~0) {
		ShowFatalError("db_free_add: free_lock overflow\n"
				"Database allocated at %s:%d\n",
				db->alloc_file, db->alloc_line);
		exit(EXIT_FAILURE);
	}
	if (!(db->options&DB_OPT_DUP_KEY)) { // Make sure we have a key until the node is freed
		old_key = node->key;
		node->key = db_dup_key(db, node->key);
		db->release(old_key, node->data, DB_RELEASE_KEY);
	}
	if (db->free_count == db->free_max) { // No more space, expand free_list
		db->free_max = (db->free_max<<2) +3; // = db->free_max*4 +3
		if (db->free_max <= db->free_count) {
			if (db->free_count == (unsigned int)~0) {
				ShowFatalError("db_free_add: free_count overflow\n"
						"Database allocated at %s:%d\n",
						db->alloc_file, db->alloc_line);
				exit(EXIT_FAILURE);
			}
			db->free_max = (unsigned int)~0;
		}
		RECREATE(db->free_list, struct db_free, db->free_max);
	}
	node->deleted = 1;
	db->free_list[db->free_count].node = node;
	db->free_list[db->free_count].root = root;
	db->free_count++;
	db->item_count--;
}

/**
 * Remove a node from the free_list of the database.
 * Marks the node as not deleted.
 * NOTE: Frees the duplicated key of the node.
 * @param db Target database
 * @param node Node being removed from free_list
 * @private
 * @see #struct db_free
 * @see DBMap_impl#free_list
 * @see DBMap_impl#free_count
 * @see #db_obj_put(DBMap*,DBKey,DBData)
 * @see #db_free_add(DBMap_impl*,DBNode**,DBNode*)
 */
static void db_free_remove(DBMap_impl* db, DBNode *node)
{
	unsigned int i;

	DB_COUNTSTAT(db_free_remove);
	for (i = 0; i < db->free_count; i++) {
		if (db->free_list[i].node == node) {
			if (i < db->free_count -1) // copy the last item to where the removed one was
				memcpy(&db->free_list[i], &db->free_list[db->free_count -1], sizeof(struct db_free));
			db_dup_key_free(db, node->key);
			break;
		}
	}
	node->deleted = 0;
	if (i == db->free_count) {
		ShowWarning("db_free_remove: node was not found - database allocated at %s:%d\n", db->alloc_file, db->alloc_line);
	} else {
		db->free_count--;
	}
	db->item_count++;
}

/**
 * Increment the free_lock of the database.
 * @param db Target database
 * @private
 * @see DBMap_impl#free_lock
 * @see #db_unlock(DBMap_impl*)
 */
static void db_free_lock(DBMap_impl* db)
{
	DB_COUNTSTAT(db_free_lock);
	if (db->free_lock == (unsigned int)~0) {
		ShowFatalError("db_free_lock: free_lock overflow\n"
				"Database allocated at %s:%d\n",
				db->alloc_file, db->alloc_line);
		exit(EXIT_FAILURE);
	}
	db->free_lock++;
}

/**
 * Decrement the free_lock of the database.
 * If it was the last lock, frees the nodes of the database.
 * Keeps the tree balanced.
 * NOTE: Frees the duplicated keys of the nodes
 * @param db Target database
 * @private
 * @see DBMap_impl#free_lock
 * @see #db_free_dbn(DBNode*)
 * @see #db_lock(DBMap_impl*)
 */
static void db_free_unlock(DBMap_impl* db)
{
	unsigned int i;

	DB_COUNTSTAT(db_free_unlock);
	if (db->free_lock == 0) {
		ShowWarning("db_free_unlock: free_lock was already 0\n"
				"Database allocated at %s:%d\n",
				db->alloc_file, db->alloc_line);
	} else {
		db->free_lock--;
	}
	if (db->free_lock)
		return; // Not last lock

	for (i = 0; i < db->free_count ; i++) {
		db_rebalance_erase(db->free_list[i].node, db->free_list[i].root);
		db_dup_key_free(db, db->free_list[i].node->key);
		DB_COUNTSTAT(db_node_free);
		ers_free(db->nodes, db->free_list[i].node);
	}
	db->free_count = 0;
}

/*****************************************************************************\
 *  (3) Section of protected functions used internally.                      *
 *  NOTE: the protected functions used in the database interface are in the  *
 *           next section.                                                   *
 *  db_int_cmp         - Default comparator for DB_INT databases.            *
 *  db_uint_cmp        - Default comparator for DB_UINT databases.           *
 *  db_string_cmp      - Default comparator for DB_STRING databases.         *
 *  db_istring_cmp     - Default comparator for DB_ISTRING databases.        *
 *  db_int64_cmp       - Default comparator for DB_INT64 databases.          *
 *  db_uint64_cmp      - Default comparator for DB_UINT64 databases.         *
 *  db_int_hash        - Default hasher for DB_INT databases.                *
 *  db_uint_hash       - Default hasher for DB_UINT databases.               *
 *  db_string_hash     - Default hasher for DB_STRING databases.             *
 *  db_istring_hash    - Default hasher for DB_ISTRING databases.            *
 *  db_int64_hash      - Default hasher for DB_INT64 databases.              *
 *  db_uint64_hash     - Default hasher for DB_UINT64 databases.             *
 *  db_release_nothing - Releaser that releases nothing.                     *
 *  db_release_key     - Releaser that only releases the key.                *
 *  db_release_data    - Releaser that only releases the data.               *
 *  db_release_both    - Releaser that releases key and data.                *
\*****************************************************************************/

/**
 * Default comparator for DB_INT databases.
 * Compares key1 to key2.
 * Return 0 if equal, negative if lower and positive if higher.
 * <code>maxlen</code> is ignored.
 * @param key1 Key to be compared
 * @param key2 Key being compared to
 * @param maxlen Maximum length of the key to hash
 * @return 0 if equal, negative if lower and positive if higher
 * @see DBType#DB_INT
 * @see #DBComparator
 * @see #db_default_cmp(DBType)
 */
static int db_int_cmp(DBKey key1, DBKey key2, unsigned short maxlen)
{
	(void)maxlen;//not used
	DB_COUNTSTAT(db_int_cmp);
	if (key1.i < key2.i) return -1;
	if (key1.i > key2.i) return 1;
	return 0;
}

/**
 * Default comparator for DB_UINT databases.
 * Compares key1 to key2.
 * Return 0 if equal, negative if lower and positive if higher.
 * <code>maxlen</code> is ignored.
 * @param key1 Key to be compared
 * @param key2 Key being compared to
 * @param maxlen Maximum length of the key to hash
 * @return 0 if equal, negative if lower and positive if higher
 * @see DBType#DB_UINT
 * @see #DBComparator
 * @see #db_default_cmp(DBType)
 */
static int db_uint_cmp(DBKey key1, DBKey key2, unsigned short maxlen)
{
	(void)maxlen;//not used
	DB_COUNTSTAT(db_uint_cmp);
	if (key1.ui < key2.ui) return -1;
	if (key1.ui > key2.ui) return 1;
	return 0;
}

/**
 * Default comparator for DB_STRING databases.
 * Compares key1 to key2.
 * Return 0 if equal, negative if lower and positive if higher.
 * @param key1 Key to be compared
 * @param key2 Key being compared to
 * @param maxlen Maximum length of the key to hash
 * @return 0 if equal, negative if lower and positive if higher
 * @see DBType#DB_STRING
 * @see #DBComparator
 * @see #db_default_cmp(DBType)
 */
static int db_string_cmp(DBKey key1, DBKey key2, unsigned short maxlen)
{
	DB_COUNTSTAT(db_string_cmp);
	return strncmp((const char *)key1.str, (const char *)key2.str, maxlen);
}

/**
 * Default comparator for DB_ISTRING databases.
 * Compares key1 to key2 case insensitively.
 * Return 0 if equal, negative if lower and positive if higher.
 * @param key1 Key to be compared
 * @param key2 Key being compared to
 * @param maxlen Maximum length of the key to hash
 * @return 0 if equal, negative if lower and positive if higher
 * @see DBType#DB_ISTRING
 * @see #DBComparator
 * @see #db_default_cmp(DBType)
 */
static int db_istring_cmp(DBKey key1, DBKey key2, unsigned short maxlen)
{
	DB_COUNTSTAT(db_istring_cmp);
	return strncasecmp((const char *)key1.str, (const char *)key2.str, maxlen);
}

/**
 * Default comparator for DB_INT64 databases.
 * Compares key1 to key2.
 * Return 0 if equal, negative if lower and positive if higher.
 * <code>maxlen</code> is ignored.
 * @param key1 Key to be compared
 * @param key2 Key being compared to
 * @param maxlen Maximum length of the key to hash
 * @return 0 if equal, negative if lower and positive if higher
 * @see DBType#DB_INT64
 * @see #DBComparator
 * @see #db_default_cmp(DBType)
 */
static int db_int64_cmp(DBKey key1, DBKey key2, unsigned short maxlen)
{
	(void)maxlen;//not used
	DB_COUNTSTAT(db_int64_cmp);
	if (key1.i64 < key2.i64) return -1;
	if (key1.i64 > key2.i64) return 1;
	return 0;
}

/**
 * Default comparator for DB_UINT64 databases.
 * Compares key1 to key2.
 * Return 0 if equal, negative if lower and positive if higher.
 * <code>maxlen</code> is ignored.
 * @param key1 Key to be compared
 * @param key2 Key being compared to
 * @param maxlen Maximum length of the key to hash
 * @return 0 if equal, negative if lower and positive if higher
 * @see DBType#DB_UINT64
 * @see #DBComparator
 * @see #db_default_cmp(DBType)
 */
static int db_uint64_cmp(DBKey key1, DBKey key2, unsigned short maxlen)
{
	(void)maxlen;//not used
	DB_COUNTSTAT(db_uint64_cmp);
	if (key1.ui64 < key2.ui64) return -1;
	if (key1.ui64 > key2.ui64) return 1;
	return 0;
}


/**
 * Default hasher for DB_INT databases.
 * Returns the value of the key as an unsigned int.
 * <code>maxlen</code> is ignored.
 * @param key Key to be hashed
 * @param maxlen Maximum length of the key to hash
 * @return hash of the key
 * @see DBType#DB_INT
 * @see #DBHasher
 * @see #db_default_hash(DBType)
 */
static uint64 db_int_hash(DBKey key, unsigned short maxlen)
{
	(void)maxlen;//not used
	DB_COUNTSTAT(db_int_hash);
	return (uint64)key.i;
}

/**
 * Default hasher for DB_UINT databases.
 * Just returns the value of the key.
 * <code>maxlen</code> is ignored.
 * @param key Key to be hashed
 * @param maxlen Maximum length of the key to hash
 * @return hash of the key
 * @see DBType#DB_UINT
 * @see #DBHasher
 * @see #db_default_hash(DBType)
 */
static uint64 db_uint_hash(DBKey key, unsigned short maxlen)
{
	(void)maxlen;//not used
	DB_COUNTSTAT(db_uint_hash);
	return (uint64)key.ui;
}

/**
 * Default hasher for DB_STRING databases.
 * @param key Key to be hashed
 * @param maxlen Maximum length of the key to hash
 * @return hash of the key
 * @see DBType#DB_STRING
 * @see #DBHasher
 * @see #db_default_hash(DBType)
 */
static uint64 db_string_hash(DBKey key, unsigned short maxlen)
{
	const char *k = key.str;
	unsigned int hash = 0;
	unsigned short i;

	DB_COUNTSTAT(db_string_hash);

	for (i = 0; *k; ++i) {
		hash = (hash*33 + ((unsigned char)*k))^(hash>>24);
		k++;
		if (i == maxlen)
			break;
	}

	return (uint64)hash;
}

/**
 * Default hasher for DB_ISTRING databases.
 * @param key Key to be hashed
 * @param maxlen Maximum length of the key to hash
 * @return hash of the key
 * @see DBType#DB_ISTRING
 * @see #db_default_hash(DBType)
 */
static uint64 db_istring_hash(DBKey key, unsigned short maxlen)
{
	const char *k = key.str;
	unsigned int hash = 0;
	unsigned short i;

	DB_COUNTSTAT(db_istring_hash);

	for (i = 0; *k; i++) {
		hash = (hash*33 + ((unsigned char)TOLOWER(*k)))^(hash>>24);
		k++;
		if (i == maxlen)
			break;
	}

	return (uint64)hash;
}

/**
 * Default hasher for DB_INT64 databases.
 * Returns the value of the key as an unsigned int.
 * <code>maxlen</code> is ignored.
 * @param key Key to be hashed
 * @param maxlen Maximum length of the key to hash
 * @return hash of the key
 * @see DBType#DB_INT64
 * @see #DBHasher
 * @see #db_default_hash(DBType)
 */
static uint64 db_int64_hash(DBKey key, unsigned short maxlen)
{
	(void)maxlen;//not used
	DB_COUNTSTAT(db_int64_hash);
	return (uint64)key.i64;
}

/**
 * Default hasher for DB_UINT64 databases.
 * Just returns the value of the key.
 * <code>maxlen</code> is ignored.
 * @param key Key to be hashed
 * @param maxlen Maximum length of the key to hash
 * @return hash of the key
 * @see DBType#DB_UINT64
 * @see #DBHasher
 * @see #db_default_hash(DBType)
 */
static uint64 db_uint64_hash(DBKey key, unsigned short maxlen)
{
	(void)maxlen;//not used
	DB_COUNTSTAT(db_uint64_hash);
	return key.ui64;
}

/**
 * Releaser that releases nothing.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param which What is being requested to be released
 * @protected
 * @see #DBReleaser
 * @see #db_default_releaser(DBType,DBOptions)
 */
static void db_release_nothing(DBKey key, DBData data, DBRelease which)
{
	(void)key;(void)data;(void)which;//not used
	DB_COUNTSTAT(db_release_nothing);
}

/**
 * Releaser that only releases the key.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param which What is being requested to be released
 * @protected
 * @see #DBReleaser
 * @see #db_default_release(DBType,DBOptions)
 */
static void db_release_key(DBKey key, DBData data, DBRelease which)
{
	(void)data;//not used
	DB_COUNTSTAT(db_release_key);
	if (which&DB_RELEASE_KEY) aFree((char*)key.str); // needs to be a pointer
}

/**
 * Releaser that only releases the data.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param which What is being requested to be released
 * @protected
 * @see #DBData
 * @see #DBRelease
 * @see #DBReleaser
 * @see #db_default_release(DBType,DBOptions)
 */
static void db_release_data(DBKey key, DBData data, DBRelease which)
{
	(void)key;//not used
	DB_COUNTSTAT(db_release_data);
	if (which&DB_RELEASE_DATA && data.type == DB_DATA_PTR) {
		aFree(data.u.ptr);
		data.u.ptr = NULL;
	}
}

/**
 * Releaser that releases both key and data.
 * @param key Key of the database entry
 * @param data Data of the database entry
 * @param which What is being requested to be released
 * @protected
 * @see #DBKey
 * @see #DBData
 * @see #DBRelease
 * @see #DBReleaser
 * @see #db_default_release(DBType,DBOptions)
 */
static void db_release_both(DBKey key, DBData data, DBRelease which)
{
	DB_COUNTSTAT(db_release_both);
	if (which&DB_RELEASE_KEY) aFree((char*)key.str); // needs to be a pointer
	if (which&DB_RELEASE_DATA && data.type == DB_DATA_PTR) {
		aFree(data.u.ptr);
		data.u.ptr = NULL;
	}
}

/*****************************************************************************\
 *  (4) Section with protected functions used in the interface of the        *
 *  database and interface of the iterator.                                  *
 *  dbit_obj_first   - Fetches the first entry from the database.            *
 *  dbit_obj_last    - Fetches the last entry from the database.             *
 *  dbit_obj_next    - Fetches the next entry from the database.             *
 *  dbit_obj_prev    - Fetches the previous entry from the database.         *
 *  dbit_obj_exists  - Returns true if the current entry exists.             *
 *  dbit_obj_remove  - Remove the current entry from the database.           *
 *  dbit_obj_destroy - Destroys the iterator, unlocking the database and     *
 *           freeing used memory.                                            *
 *  db_obj_iterator - Return a new database iterator.                         *
 *  db_obj_exists   - Checks if an entry exists.                             *
 *  db_obj_get      - Get the data identified by the key.                    *
 *  db_obj_vgetall  - Get the data of the matched entries.                   *
 *  db_obj_getall   - Get the data of the matched entries.                   *
 *  db_obj_vensure  - Get the data identified by the key, creating if it     *
 *           doesn't exist yet.                                              *
 *  db_obj_ensure   - Get the data identified by the key, creating if it     *
 *           doesn't exist yet.                                              *
 *  db_obj_put      - Put data identified by the key in the database.        *
 *  db_obj_remove   - Remove an entry from the database.                     *
 *  db_obj_vforeach - Apply a function to every entry in the database.       *
 *  db_obj_foreach  - Apply a function to every entry in the database.       *
 *  db_obj_vclear   - Remove all entries from the database.                  *
 *  db_obj_clear    - Remove all entries from the database.                  *
 *  db_obj_vdestroy - Destroy the database, freeing all the used memory.     *
 *  db_obj_destroy  - Destroy the database, freeing all the used memory.     *
 *  db_obj_size     - Return the size of the database.                       *
 *  db_obj_type     - Return the type of the database.                       *
 *  db_obj_options  - Return the options of the database.                    *
\*****************************************************************************/

/**
 * Fetches the first entry in the database.
 * Returns the data of the entry.
 * Puts the key in out_key, if out_key is not NULL.
 * @param self Iterator
 * @param out_key Key of the entry
 * @return Data of the entry
 * @protected
 * @see DBIterator#first
 */
DBData* dbit_obj_first(DBIterator* self, DBKey* out_key)
{
	DBIterator_impl* it = (DBIterator_impl*)self;

	DB_COUNTSTAT(dbit_first);
	// position before the first entry
	it->ht_index = -1;
	it->node = NULL;
	// get next entry
	return self->next(self, out_key);
}

/**
 * Fetches the last entry in the database.
 * Returns the data of the entry.
 * Puts the key in out_key, if out_key is not NULL.
 * @param self Iterator
 * @param out_key Key of the entry
 * @return Data of the entry
 * @protected
 * @see DBIterator#last
 */
DBData* dbit_obj_last(DBIterator* self, DBKey* out_key)
{
	DBIterator_impl* it = (DBIterator_impl*)self;

	DB_COUNTSTAT(dbit_last);
	// position after the last entry
	it->ht_index = HASH_SIZE;
	it->node = NULL;
	// get previous entry
	return self->prev(self, out_key);
}

/**
 * Fetches the next entry in the database.
 * Returns the data of the entry.
 * Puts the key in out_key, if out_key is not NULL.
 * @param self Iterator
 * @param out_key Key of the entry
 * @return Data of the entry
 * @protected
 * @see DBIterator#next
 */
DBData* dbit_obj_next(DBIterator* self, DBKey* out_key)
{
	DBIterator_impl* it = (DBIterator_impl*)self;
	DBNode *node;
	DBNode *parent;
	struct dbn fake;

	DB_COUNTSTAT(dbit_next);
	if( it->ht_index < 0 )
	{// get first node
		it->ht_index = 0;
		it->node = NULL;
	}
	node = it->node;
	memset(&fake, 0, sizeof(fake));
	for( ; it->ht_index < HASH_SIZE; ++(it->ht_index) )
	{
		// Iterate in the order: left tree, current node, right tree
		if( node == NULL )
		{// prepare initial node of this hash
			node = it->db->ht[it->ht_index];
			if( node == NULL )
				continue;// next hash
			fake.right = node;
			node = &fake;
		}

		while( node )
		{// next node
			if( node->right )
			{// continue in the right subtree
				node = node->right;
				while( node->left )
					node = node->left;// get leftmost node
			}
			else
			{// continue to the next parent (recursive)
				parent = node->parent;
				while( parent )
				{
					if( parent->right != node )
						break;
					node = parent;
					parent = node->parent;
				}
				if( parent == NULL )
				{// next hash
					node = NULL;
					break;
				}
				node = parent;
			}

			if( !node->deleted )
			{// found next entry
				it->node = node;
				if( out_key )
					memcpy(out_key, &node->key, sizeof(DBKey));
				return &node->data;
			}
		}
	}
	it->node = NULL;
	return NULL;// not found
}

/**
 * Fetches the previous entry in the database.
 * Returns the data of the entry.
 * Puts the key in out_key, if out_key is not NULL.
 * @param self Iterator
 * @param out_key Key of the entry
 * @return Data of the entry
 * @protected
 * @see DBIterator#prev
 */
DBData* dbit_obj_prev(DBIterator* self, DBKey* out_key)
{
	DBIterator_impl* it = (DBIterator_impl*)self;
	DBNode *node;
	DBNode *parent;
	struct dbn fake;

	DB_COUNTSTAT(dbit_prev);
	if( it->ht_index >= HASH_SIZE )
	{// get last node
		it->ht_index = HASH_SIZE-1;
		it->node = NULL;
	}
	node = it->node;
	memset(&fake, 0, sizeof(fake));
	for( ; it->ht_index >= 0; --(it->ht_index) )
	{
		// Iterate in the order: right tree, current node, left tree
		if( node == NULL )
		{// prepare initial node of this hash
			node = it->db->ht[it->ht_index];
			if( node == NULL )
				continue;// next hash
			fake.left = node;
			node = &fake;
		}

		while( node )
		{// next node
			if( node->left )
			{// continue in the left subtree
				node = node->left;
				while( node->right )
					node = node->right;// get rightmost node
			}
			else
			{// continue to the next parent (recursive)
				parent = node->parent;
				while( parent )
				{
					if( parent->left != node )
						break;
					node = parent;
					parent = node->parent;
				}
				if( parent == NULL )
				{// next hash
					node = NULL;
					break;
				}
				node = parent;
			}

			if( !node->deleted )
			{// found previous entry
				it->node = node;
				if( out_key )
					memcpy(out_key, &node->key, sizeof(DBKey));
				return &node->data;
			}
		}
	}
	it->node = NULL;
	return NULL;// not found
}

/**
 * Returns true if the fetched entry exists.
 * The databases entries might have NULL data, so use this to to test if
 * the iterator is done.
 * @param self Iterator
 * @return true if the entry exists
 * @protected
 * @see DBIterator#exists
 */
bool dbit_obj_exists(DBIterator* self)
{
	DBIterator_impl* it = (DBIterator_impl*)self;

	DB_COUNTSTAT(dbit_exists);
	return (it->node && !it->node->deleted);
}

/**
 * Removes the current entry from the database.
 * NOTE: {@link DBIterator#exists} will return false until another entry
 *       is fetched
 * Puts data of the removed entry in out_data, if out_data is not NULL (unless data has been released)
 * @param self Iterator
 * @param out_data Data of the removed entry.
 * @return 1 if entry was removed, 0 otherwise
 * @protected
 * @see DBMap#remove
 * @see DBIterator#remove
 */
int dbit_obj_remove(DBIterator* self, DBData *out_data)
{
	DBIterator_impl* it = (DBIterator_impl*)self;
	DBNode *node;
	int retval = 0;

	DB_COUNTSTAT(dbit_remove);
	node = it->node;
	if( node && !node->deleted )
	{
		DBMap_impl* db = it->db;
		if( db->cache == node )
			db->cache = NULL;
		db->release(node->key, node->data, DB_RELEASE_DATA);
		if( out_data )
			memcpy(out_data, &node->data, sizeof(DBData));
		retval = 1;
		db_free_add(db, node, &db->ht[it->ht_index]);
	}
	return retval;
}

/**
 * Destroys this iterator and unlocks the database.
 * @param self Iterator
 * @protected
 */
void dbit_obj_destroy(DBIterator* self)
{
	DBIterator_impl* it = (DBIterator_impl*)self;

	DB_COUNTSTAT(dbit_destroy);
	// unlock the database
	db_free_unlock(it->db);
	// free iterator
	ers_free(db_iterator_ers,self);
}

/**
 * Returns a new iterator for this database.
 * The iterator keeps the database locked until it is destroyed.
 * The database will keep functioning normally but will only free internal
 * memory when unlocked, so destroy the iterator as soon as possible.
 * @param self Database
 * @return New iterator
 * @protected
 */
static DBIterator* db_obj_iterator(DBMap* self)
{
	DBMap_impl* db = (DBMap_impl*)self;
	DBIterator_impl* it;

	DB_COUNTSTAT(db_iterator);
	it = ers_alloc(db_iterator_ers, struct DBIterator_impl);
	/* Interface of the iterator **/
	it->vtable.first   = dbit_obj_first;
	it->vtable.last    = dbit_obj_last;
	it->vtable.next    = dbit_obj_next;
	it->vtable.prev    = dbit_obj_prev;
	it->vtable.exists  = dbit_obj_exists;
	it->vtable.remove  = dbit_obj_remove;
	it->vtable.destroy = dbit_obj_destroy;
	/* Initial state (before the first entry) */
	it->db = db;
	it->ht_index = -1;
	it->node = NULL;
	/* Lock the database */
	db_free_lock(db);
	return &it->vtable;
}

/**
 * Returns true if the entry exists.
 * @param self Interface of the database
 * @param key Key that identifies the entry
 * @return true is the entry exists
 * @protected
 * @see DBMap#exists
 */
static bool db_obj_exists(DBMap* self, DBKey key)
{
	DBMap_impl* db = (DBMap_impl*)self;
	DBNode *node;
	bool found = false;

	DB_COUNTSTAT(db_exists);
	if (db == NULL) return false; // nullpo candidate
	if (!(db->options&DB_OPT_ALLOW_NULL_KEY) && db_is_key_null(db->type, key)) {
		return false; // nullpo candidate
	}

	if (db->cache && db->cmp(key, db->cache->key, db->maxlen) == 0) {
#if defined(DEBUG)
		if (db->cache->deleted) {
			ShowDebug("db_exists: Cache contains a deleted node. Please report this!!!\n");
			return false;
		}
#endif
		return true; // cache hit
	}

	db_free_lock(db);
	node = db->ht[db->hash(key, db->maxlen)%HASH_SIZE];
	while (node) {
		int c = db->cmp(key, node->key, db->maxlen);
		if (c == 0) {
			if (!(node->deleted)) {
				db->cache = node;
				found = true;
			}
			break;
		}
		if (c < 0)
			node = node->left;
		else
			node = node->right;
	}
	db_free_unlock(db);
	return found;
}

/**
 * Get the data of the entry identified by the key.
 * @param self Interface of the database
 * @param key Key that identifies the entry
 * @return Data of the entry or NULL if not found
 * @protected
 * @see DBMap#get
 */
static DBData* db_obj_get(DBMap* self, DBKey key)
{
	DBMap_impl* db = (DBMap_impl*)self;
	DBNode *node;
	DBData *data = NULL;

	DB_COUNTSTAT(db_get);
	if (db == NULL) return NULL; // nullpo candidate
	if (!(db->options&DB_OPT_ALLOW_NULL_KEY) && db_is_key_null(db->type, key)) {
		ShowError("db_get: Attempted to retrieve non-allowed NULL key for db allocated at %s:%d\n",db->alloc_file, db->alloc_line);
		return NULL; // nullpo candidate
	}

	if (db->cache && db->cmp(key, db->cache->key, db->maxlen) == 0) {
#if defined(DEBUG)
		if (db->cache->deleted) {
			ShowDebug("db_get: Cache contains a deleted node. Please report this!!!\n");
			return NULL;
		}
#endif
		return &db->cache->data; // cache hit
	}

	db_free_lock(db);
	node = db->ht[db->hash(key, db->maxlen)%HASH_SIZE];
	while (node) {
		int c = db->cmp(key, node->key, db->maxlen);
		if (c == 0) {
			if (!(node->deleted)) {
				data = &node->data;
				db->cache = node;
			}
			break;
		}
		if (c < 0)
			node = node->left;
		else
			node = node->right;
	}
	db_free_unlock(db);
	return data;
}

/**
 * Get the data of the entries matched by <code>match</code>.
 * It puts a maximum of <code>max</code> entries into <code>buf</code>.
 * If <code>buf</code> is NULL, it only counts the matches.
 * Returns the number of entries that matched.
 * NOTE: if the value returned is greater than <code>max</code>, only the
 * first <code>max</code> entries found are put into the buffer.
 * @param self Interface of the database
 * @param buf Buffer to put the data of the matched entries
 * @param max Maximum number of data entries to be put into buf
 * @param match Function that matches the database entries
 * @param ... Extra arguments for match
 * @return The number of entries that matched
 * @protected
 * @see DBMap#vgetall
 */
static unsigned int db_obj_vgetall(DBMap* self, DBData **buf, unsigned int max, DBMatcher match, va_list args)
{
	DBMap_impl* db = (DBMap_impl*)self;
	unsigned int i;
	DBNode *node;
	DBNode *parent;
	unsigned int ret = 0;

	DB_COUNTSTAT(db_vgetall);
	if (db == NULL) return 0; // nullpo candidate
	if (match == NULL) return 0; // nullpo candidate

	db_free_lock(db);
	for (i = 0; i < HASH_SIZE; i++) {
		// Match in the order: current node, left tree, right tree
		node = db->ht[i];
		while (node) {

			if (!(node->deleted)) {
				va_list argscopy;
				va_copy(argscopy, args);
				if (match(node->key, node->data, argscopy) == 0) {
					if (buf && ret < max)
						buf[ret] = &node->data;
					ret++;
				}
				va_end(argscopy);
			}

			if (node->left) {
				node = node->left;
				continue;
			}

			if (node->right) {
				node = node->right;
				continue;
			}

			while (node) {
				parent = node->parent;
				if (parent && parent->right && parent->left == node) {
					node = parent->right;
					break;
				}
				node = parent;
			}

		}
	}
	db_free_unlock(db);
	return ret;
}

/**
 * Just calls {@link DBMap#vgetall}.
 * Get the data of the entries matched by <code>match</code>.
 * It puts a maximum of <code>max</code> entries into <code>buf</code>.
 * If <code>buf</code> is NULL, it only counts the matches.
 * Returns the number of entries that matched.
 * NOTE: if the value returned is greater than <code>max</code>, only the
 * first <code>max</code> entries found are put into the buffer.
 * @param self Interface of the database
 * @param buf Buffer to put the data of the matched entries
 * @param max Maximum number of data entries to be put into buf
 * @param match Function that matches the database entries
 * @param ... Extra arguments for match
 * @return The number of entries that matched
 * @protected
 * @see DBMap#vgetall
 * @see DBMap#getall
 */
static unsigned int db_obj_getall(DBMap* self, DBData **buf, unsigned int max, DBMatcher match, ...)
{
	va_list args;
	unsigned int ret;

	DB_COUNTSTAT(db_getall);
	if (self == NULL) return 0; // nullpo candidate

	va_start(args, match);
	ret = self->vgetall(self, buf, max, match, args);
	va_end(args);
	return ret;
}

/**
 * Get the data of the entry identified by the key.
 * If the entry does not exist, an entry is added with the data returned by
 * <code>create</code>.
 * @param self Interface of the database
 * @param key Key that identifies the entry
 * @param create Function used to create the data if the entry doesn't exist
 * @param args Extra arguments for create
 * @return Data of the entry
 * @protected
 * @see DBMap#vensure
 */
static DBData* db_obj_vensure(DBMap* self, DBKey key, DBCreateData create, va_list args)
{
	DBMap_impl* db = (DBMap_impl*)self;
	DBNode *node;
	DBNode *parent = NULL;
	unsigned int hash;
	int c = 0;
	DBData *data = NULL;

	DB_COUNTSTAT(db_vensure);
	if (db == NULL) return NULL; // nullpo candidate
	if (create == NULL) {
		ShowError("db_ensure: Create function is NULL for db allocated at %s:%d\n",db->alloc_file, db->alloc_line);
		return NULL; // nullpo candidate
	}
	if (!(db->options&DB_OPT_ALLOW_NULL_KEY) && db_is_key_null(db->type, key)) {
		ShowError("db_ensure: Attempted to use non-allowed NULL key for db allocated at %s:%d\n",db->alloc_file, db->alloc_line);
		return NULL; // nullpo candidate
	}

	if (db->cache && db->cmp(key, db->cache->key, db->maxlen) == 0)
		return &db->cache->data; // cache hit

	db_free_lock(db);
	hash = db->hash(key, db->maxlen)%HASH_SIZE;
	node = db->ht[hash];
	while (node) {
		c = db->cmp(key, node->key, db->maxlen);
		if (c == 0) {
			break;
		}
		parent = node;
		if (c < 0)
			node = node->left;
		else
			node = node->right;
	}
	// Create node if necessary
	if (node == NULL) {
		va_list argscopy;
		if (db->item_count == UINT32_MAX) {
			ShowError("db_vensure: item_count overflow, aborting item insertion.\n"
					"Database allocated at %s:%d",
					db->alloc_file, db->alloc_line);
				return NULL;
		}
		DB_COUNTSTAT(db_node_alloc);
		node = ers_alloc(db->nodes, struct dbn);
		node->left = NULL;
		node->right = NULL;
		node->deleted = 0;
		db->item_count++;
		if (c == 0) { // hash entry is empty
			node->color = BLACK;
			node->parent = NULL;
			db->ht[hash] = node;
		} else {
			node->color = RED;
			if (c < 0) { // put at the left
				parent->left = node;
				node->parent = parent;
			} else { // put at the right
				parent->right = node;
				node->parent = parent;
			}
			if (parent->color == RED) // two consecutive RED nodes, must rebalance
				db_rebalance(node, &db->ht[hash]);
		}
		// put key and data in the node
		if (db->options&DB_OPT_DUP_KEY) {
			node->key = db_dup_key(db, key);
			if (db->options&DB_OPT_RELEASE_KEY)
				db->release(key, node->data, DB_RELEASE_KEY);
		} else {
			node->key = key;
		}
		va_copy(argscopy, args);
		node->data = create(key, argscopy);
		va_end(argscopy);
	}
	data = &node->data;
	db->cache = node;
	db_free_unlock(db);
	return data;
}

/**
 * Just calls {@link DBMap#vensure}.
 * Get the data of the entry identified by the key.
 * If the entry does not exist, an entry is added with the data returned by
 * <code>create</code>.
 * @param self Interface of the database
 * @param key Key that identifies the entry
 * @param create Function used to create the data if the entry doesn't exist
 * @param ... Extra arguments for create
 * @return Data of the entry
 * @protected
 * @see DBMap#vensure
 * @see DBMap#ensure
 */
static DBData* db_obj_ensure(DBMap* self, DBKey key, DBCreateData create, ...)
{
	va_list args;
	DBData *ret = NULL;

	DB_COUNTSTAT(db_ensure);
	if (self == NULL) return NULL; // nullpo candidate

	va_start(args, create);
	ret = self->vensure(self, key, create, args);
	va_end(args);
	return ret;
}

/**
 * Put the data identified by the key in the database.
 * Puts the previous data in out_data, if out_data is not NULL. (unless data has been released)
 * NOTE: Uses the new key, the old one is released.
 * @param self Interface of the database
 * @param key Key that identifies the data
 * @param data Data to be put in the database
 * @param out_data Previous data if the entry exists
 * @return 1 if if the entry already exists, 0 otherwise
 * @protected
 * @see #db_malloc_dbn(void)
 * @see DBMap#put
 * FIXME: If this method fails shouldn't it return another value?
 *        Other functions rely on this to know if they were able to put something [Panikon]
 */
static int db_obj_put(DBMap* self, DBKey key, DBData data, DBData *out_data)
{
	DBMap_impl* db = (DBMap_impl*)self;
	DBNode *node;
	DBNode *parent = NULL;
	int c = 0, retval = 0;
	unsigned int hash;

	DB_COUNTSTAT(db_put);
	if (db == NULL) return 0; // nullpo candidate
	if (db->global_lock) {
		ShowError("db_put: Database is being destroyed, aborting entry insertion.\n"
				"Database allocated at %s:%d\n",
				db->alloc_file, db->alloc_line);
		return 0; // nullpo candidate
	}
	if (!(db->options&DB_OPT_ALLOW_NULL_KEY) && db_is_key_null(db->type, key)) {
		ShowError("db_put: Attempted to use non-allowed NULL key for db allocated at %s:%d\n",db->alloc_file, db->alloc_line);
		return 0; // nullpo candidate
	}
	if (!(db->options&DB_OPT_ALLOW_NULL_DATA) && (data.type == DB_DATA_PTR && data.u.ptr == NULL)) {
		ShowError("db_put: Attempted to use non-allowed NULL data for db allocated at %s:%d\n",db->alloc_file, db->alloc_line);
		return 0; // nullpo candidate
	}

	if (db->item_count == UINT32_MAX) {
		ShowError("db_put: item_count overflow, aborting item insertion.\n"
				"Database allocated at %s:%d",
				db->alloc_file, db->alloc_line);
		return 0;
	}
	// search for an equal node
	db_free_lock(db);
	hash = db->hash(key, db->maxlen)%HASH_SIZE;
	for (node = db->ht[hash]; node; ) {
		c = db->cmp(key, node->key, db->maxlen);
		if (c == 0) { // equal entry, replace
			if (node->deleted) {
				db_free_remove(db, node);
			} else {
				db->release(node->key, node->data, DB_RELEASE_BOTH);
				if (out_data)
					memcpy(out_data, &node->data, sizeof(*out_data));
				retval = 1;
			}
			break;
		}
		parent = node;
		if (c < 0) {
			node = node->left;
		} else {
			node = node->right;
		}
	}
	// allocate a new node if necessary
	if (node == NULL) {
		DB_COUNTSTAT(db_node_alloc);
		node = ers_alloc(db->nodes, struct dbn);
		node->left = NULL;
		node->right = NULL;
		node->deleted = 0;
		db->item_count++;
		if (c == 0) { // hash entry is empty
			node->color = BLACK;
			node->parent = NULL;
			db->ht[hash] = node;
		} else {
			node->color = RED;
			if (c < 0) { // put at the left
				parent->left = node;
				node->parent = parent;
			} else { // put at the right
				parent->right = node;
				node->parent = parent;
			}
			if (parent->color == RED) // two consecutive RED nodes, must rebalance
				db_rebalance(node, &db->ht[hash]);
		}
	}
	// put key and data in the node
	if (db->options&DB_OPT_DUP_KEY) {
		node->key = db_dup_key(db, key);
		if (db->options&DB_OPT_RELEASE_KEY)
			db->release(key, data, DB_RELEASE_KEY);
	} else {
		node->key = key;
	}
	node->data = data;
	db->cache = node;
	db_free_unlock(db);
	return retval;
}

/**
 * Remove an entry from the database.
 * Puts the previous data in out_data, if out_data is not NULL. (unless data has been released)
 * NOTE: The key (of the database) is released in {@link #db_free_add(DBMap_impl*,DBNode*,DBNode **)}.
 * @param self Interface of the database
 * @param key Key that identifies the entry
 * @param out_data Previous data if the entry exists
 * @return 1 if if the entry already exists, 0 otherwise
 * @protected
 * @see #db_free_add(DBMap_impl*,DBNode*,DBNode **)
 * @see DBMap#remove
 */
static int db_obj_remove(DBMap* self, DBKey key, DBData *out_data)
{
	DBMap_impl* db = (DBMap_impl*)self;
	DBNode *node;
	unsigned int hash;
	int retval = 0;

	DB_COUNTSTAT(db_remove);
	if (db == NULL) return 0; // nullpo candidate
	if (db->global_lock) {
		ShowError("db_remove: Database is being destroyed. Aborting entry deletion.\n"
				"Database allocated at %s:%d\n",
				db->alloc_file, db->alloc_line);
		return 0; // nullpo candidate
	}
	if (!(db->options&DB_OPT_ALLOW_NULL_KEY) && db_is_key_null(db->type, key)) {
		ShowError("db_remove: Attempted to use non-allowed NULL key for db allocated at %s:%d\n",db->alloc_file, db->alloc_line);
		return 0; // nullpo candidate
	}

	db_free_lock(db);
	hash = db->hash(key, db->maxlen)%HASH_SIZE;
	for(node = db->ht[hash]; node; ){
		int c = db->cmp(key, node->key, db->maxlen);
		if (c == 0) {
			if (!(node->deleted)) {
				if (db->cache == node)
					db->cache = NULL;
				db->release(node->key, node->data, DB_RELEASE_DATA);
				if (out_data)
					memcpy(out_data, &node->data, sizeof(*out_data));
				retval = 1;
				db_free_add(db, node, &db->ht[hash]);
			}
			break;
		}
		if (c < 0)
			node = node->left;
		else
			node = node->right;
	}
	db_free_unlock(db);
	return retval;
}

/**
 * Apply <code>func</code> to every entry in the database.
 * Returns the sum of values returned by func.
 * @param self Interface of the database
 * @param func Function to be applied
 * @param args Extra arguments for func
 * @return Sum of the values returned by func
 * @protected
 * @see DBMap#vforeach
 */
static int db_obj_vforeach(DBMap* self, DBApply func, va_list args)
{
	DBMap_impl* db = (DBMap_impl*)self;
	unsigned int i;
	int sum = 0;
	DBNode *node;
	DBNode *parent;

	DB_COUNTSTAT(db_vforeach);
	if (db == NULL) return 0; // nullpo candidate
	if (func == NULL) {
		ShowError("db_foreach: Passed function is NULL for db allocated at %s:%d\n",db->alloc_file, db->alloc_line);
		return 0; // nullpo candidate
	}

	db_free_lock(db);
	for (i = 0; i < HASH_SIZE; i++) {
		// Apply func in the order: current node, left node, right node
		node = db->ht[i];
		while (node) {
			if (!(node->deleted)) {
				va_list argscopy;
				va_copy(argscopy, args);
				sum += func(node->key, &node->data, argscopy);
				va_end(argscopy);
			}
			if (node->left) {
				node = node->left;
				continue;
			}
			if (node->right) {
				node = node->right;
				continue;
			}
			while (node) {
				parent = node->parent;
				if (parent && parent->right && parent->left == node) {
					node = parent->right;
					break;
				}
				node = parent;
			}
		}
	}
	db_free_unlock(db);
	return sum;
}

/**
 * Just calls {@link DBMap#vforeach}.
 * Apply <code>func</code> to every entry in the database.
 * Returns the sum of values returned by func.
 * @param self Interface of the database
 * @param func Function to be applied
 * @param ... Extra arguments for func
 * @return Sum of the values returned by func
 * @protected
 * @see DBMap#vforeach
 * @see DBMap#foreach
 */
static int db_obj_foreach(DBMap* self, DBApply func, ...)
{
	va_list args;
	int ret;

	DB_COUNTSTAT(db_foreach);
	if (self == NULL) return 0; // nullpo candidate

	va_start(args, func);
	ret = self->vforeach(self, func, args);
	va_end(args);
	return ret;
}

/**
 * Removes all entries from the database.
 * Before deleting an entry, func is applied to it.
 * Releases the key and the data.
 * Returns the sum of values returned by func, if it exists.
 * @param self Interface of the database
 * @param func Function to be applied to every entry before deleting
 * @param args Extra arguments for func
 * @return Sum of values returned by func
 * @protected
 * @see DBMap#vclear
 */
static int db_obj_vclear(DBMap* self, DBApply func, va_list args)
{
	DBMap_impl* db = (DBMap_impl*)self;
	int sum = 0;
	unsigned int i;
	DBNode *node;
	DBNode *parent;

	DB_COUNTSTAT(db_vclear);
	if (db == NULL) return 0; // nullpo candidate

	db_free_lock(db);
	db->cache = NULL;
	for (i = 0; i < HASH_SIZE; i++) {
		// Apply the func and delete in the order: left tree, right tree, current node
		node = db->ht[i];
		db->ht[i] = NULL;
		while (node) {
			parent = node->parent;
			if (node->left) {
				node = node->left;
				continue;
			}
			if (node->right) {
				node = node->right;
				continue;
			}
			if (node->deleted) {
				db_dup_key_free(db, node->key);
			} else {
				if (func)
				{
					va_list argscopy;
					va_copy(argscopy, args);
					sum += func(node->key, &node->data, argscopy);
					va_end(argscopy);
				}
				db->release(node->key, node->data, DB_RELEASE_BOTH);
				node->deleted = 1;
			}
			DB_COUNTSTAT(db_node_free);
			if (parent) {
				if (parent->left == node)
					parent->left = NULL;
				else
					parent->right = NULL;
			}
			ers_free(db->nodes, node);
			node = parent;
		}
		db->ht[i] = NULL;
	}
	db->free_count = 0;
	db->item_count = 0;
	db_free_unlock(db);
	return sum;
}

/**
 * Just calls {@link DBMap#vclear}.
 * Removes all entries from the database.
 * Before deleting an entry, func is applied to it.
 * Releases the key and the data.
 * Returns the sum of values returned by func, if it exists.
 * NOTE: This locks the database globally. Any attempt to insert or remove
 * a database entry will give an error and be aborted (except for clearing).
 * @param self Interface of the database
 * @param func Function to be applied to every entry before deleting
 * @param ... Extra arguments for func
 * @return Sum of values returned by func
 * @protected
 * @see DBMap#vclear
 * @see DBMap#clear
 */
static int db_obj_clear(DBMap* self, DBApply func, ...)
{
	va_list args;
	int ret;

	DB_COUNTSTAT(db_clear);
	if (self == NULL) return 0; // nullpo candidate

	va_start(args, func);
	ret = self->vclear(self, func, args);
	va_end(args);
	return ret;
}

/**
 * Finalize the database, feeing all the memory it uses.
 * Before deleting an entry, func is applied to it.
 * Returns the sum of values returned by func, if it exists.
 * NOTE: This locks the database globally. Any attempt to insert or remove
 * a database entry will give an error and be aborted (except for clearing).
 * @param self Interface of the database
 * @param func Function to be applied to every entry before deleting
 * @param args Extra arguments for func
 * @return Sum of values returned by func
 * @protected
 * @see DBMap#vdestroy
 */
static int db_obj_vdestroy(DBMap* self, DBApply func, va_list args)
{
	DBMap_impl* db = (DBMap_impl*)self;
	int sum;

	DB_COUNTSTAT(db_vdestroy);
	if (db == NULL) return 0; // nullpo candidate
	if (db->global_lock) {
		ShowError("db_vdestroy: Database is already locked for destruction. Aborting second database destruction.\n"
				"Database allocated at %s:%d\n",
				db->alloc_file, db->alloc_line);
		return 0;
	}
	if (db->free_lock)
		ShowWarning("db_vdestroy: Database is still in use, %u lock(s) left. Continuing database destruction.\n"
				"Database allocated at %s:%d\n",
				db->free_lock, db->alloc_file, db->alloc_line);

#ifdef DB_ENABLE_STATS
	switch (db->type) {
		case DB_INT: DB_COUNTSTAT(db_int_destroy); break;
		case DB_UINT: DB_COUNTSTAT(db_uint_destroy); break;
		case DB_STRING: DB_COUNTSTAT(db_string_destroy); break;
		case DB_ISTRING: DB_COUNTSTAT(db_istring_destroy); break;
		case DB_INT64: DB_COUNTSTAT(db_int64_destroy); break;
		case DB_UINT64: DB_COUNTSTAT(db_uint64_destroy); break;
	}
#endif /* DB_ENABLE_STATS */
	db_free_lock(db);
	db->global_lock = 1;
	sum = self->vclear(self, func, args);
	aFree(db->free_list);
	db->free_list = NULL;
	db->free_max = 0;
	ers_destroy(db->nodes);
	db_free_unlock(db);
	ers_free(db_alloc_ers, db);
	return sum;
}

/**
 * Just calls {@link DBMap#db_vdestroy}.
 * Finalize the database, feeing all the memory it uses.
 * Before deleting an entry, func is applied to it.
 * Releases the key and the data.
 * Returns the sum of values returned by func, if it exists.
 * NOTE: This locks the database globally. Any attempt to insert or remove
 * a database entry will give an error and be aborted.
 * @param self Database
 * @param func Function to be applied to every entry before deleting
 * @param ... Extra arguments for func
 * @return Sum of values returned by func
 * @protected
 * @see DBMap#vdestroy
 * @see DBMap#destroy
 */
static int db_obj_destroy(DBMap* self, DBApply func, ...)
{
	va_list args;
	int ret;

	DB_COUNTSTAT(db_destroy);
	if (self == NULL) return 0; // nullpo candidate

	va_start(args, func);
	ret = self->vdestroy(self, func, args);
	va_end(args);
	return ret;
}

/**
 * Return the size of the database (number of items in the database).
 * @param self Interface of the database
 * @return Size of the database
 * @protected
 * @see DBMap_impl#item_count
 * @see DBMap#size
 */
static unsigned int db_obj_size(DBMap* self)
{
	DBMap_impl* db = (DBMap_impl*)self;
	unsigned int item_count;

	DB_COUNTSTAT(db_size);
	if (db == NULL) return 0; // nullpo candidate

	db_free_lock(db);
	item_count = db->item_count;
	db_free_unlock(db);

	return item_count;
}

/**
 * Return the type of database.
 * @param self Interface of the database
 * @return Type of the database
 * @protected
 * @see DBMap_impl#type
 * @see DBMap#type
 */
static DBType db_obj_type(DBMap* self)
{
	DBMap_impl* db = (DBMap_impl*)self;
	DBType type;

	DB_COUNTSTAT(db_type);
	if (db == NULL) return (DBType)-1; // nullpo candidate - TODO what should this return?

	db_free_lock(db);
	type = db->type;
	db_free_unlock(db);

	return type;
}

/**
 * Return the options of the database.
 * @param self Interface of the database
 * @return Options of the database
 * @protected
 * @see DBMap_impl#options
 * @see DBMap#options
 */
static DBOptions db_obj_options(DBMap* self)
{
	DBMap_impl* db = (DBMap_impl*)self;
	DBOptions options;

	DB_COUNTSTAT(db_options);
	if (db == NULL) return DB_OPT_BASE; // nullpo candidate - TODO what should this return?

	db_free_lock(db);
	options = db->options;
	db_free_unlock(db);

	return options;
}

/*****************************************************************************\
 *  (5) Section with public functions.
 *  db_fix_options     - Apply database type restrictions to the options.
 *  db_default_cmp     - Get the default comparator for a type of database.
 *  db_default_hash    - Get the default hasher for a type of database.
 *  db_default_release - Get the default releaser for a type of database with the specified options.
 *  db_custom_release  - Get a releaser that behaves a certain way.
 *  db_alloc           - Allocate a new database.
 *  db_i2key           - Manual cast from 'int' to 'DBKey'.
 *  db_ui2key          - Manual cast from 'unsigned int' to 'DBKey'.
 *  db_str2key         - Manual cast from 'unsigned char *' to 'DBKey'.
 *  db_i642key         - Manual cast from 'int64' to 'DBKey'.
 *  db_ui642key        - Manual cast from 'uin64' to 'DBKey'.
 *  db_i2data          - Manual cast from 'int' to 'DBData'.
 *  db_ui2data         - Manual cast from 'unsigned int' to 'DBData'.
 *  db_ptr2data        - Manual cast from 'void*' to 'DBData'.
 *  db_data2i          - Gets 'int' value from 'DBData'.
 *  db_data2ui         - Gets 'unsigned int' value from 'DBData'.
 *  db_data2ptr        - Gets 'void*' value from 'DBData'.
 *  db_init            - Initializes the database system.
 *  db_final           - Finalizes the database system.
\*****************************************************************************/

/**
 * Returns the fixed options according to the database type.
 * Sets required options and unsets unsupported options.
 * For numeric databases DB_OPT_DUP_KEY and DB_OPT_RELEASE_KEY are unset.
 * @param type Type of the database
 * @param options Original options of the database
 * @return Fixed options of the database
 * @private
 * @see #db_default_release(DBType,DBOptions)
 * @see #db_alloc(const char *,int,DBType,DBOptions,unsigned short)
 */
DBOptions db_fix_options(DBType type, DBOptions options)
{
	DB_COUNTSTAT(db_fix_options);
	switch (type) {
		case DB_INT:
		case DB_UINT:
		case DB_INT64:
		case DB_UINT64: // Numeric database, do nothing with the keys
			return (DBOptions)(options&~(DB_OPT_DUP_KEY|DB_OPT_RELEASE_KEY));

		default:
			ShowError("db_fix_options: Unknown database type %u with options %x\n", type, options);
		case DB_STRING:
		case DB_ISTRING: // String databases, no fix required
			return options;
	}
}

/**
 * Returns the default comparator for the specified type of database.
 * @param type Type of database
 * @return Comparator for the type of database or NULL if unknown database
 * @public
 * @see #db_int_cmp(DBKey,DBKey,unsigned short)
 * @see #db_uint_cmp(DBKey,DBKey,unsigned short)
 * @see #db_string_cmp(DBKey,DBKey,unsigned short)
 * @see #db_istring_cmp(DBKey,DBKey,unsigned short)
 * @see #db_int64_cmp(DBKey,DBKey,unsigned short)
 * @see #db_uint64_cmp(DBKey,DBKey,unsigned short)
 */
DBComparator db_default_cmp(DBType type)
{
	DB_COUNTSTAT(db_default_cmp);
	switch (type) {
		case DB_INT:     return &db_int_cmp;
		case DB_UINT:    return &db_uint_cmp;
		case DB_STRING:  return &db_string_cmp;
		case DB_ISTRING: return &db_istring_cmp;
		case DB_INT64:   return &db_int64_cmp;
		case DB_UINT64:  return &db_uint64_cmp;
		default:
			ShowError("db_default_cmp: Unknown database type %u\n", type);
			return NULL;
	}
}

/**
 * Returns the default hasher for the specified type of database.
 * @param type Type of database
 * @return Hasher of the type of database or NULL if unknown database
 * @public
 * @see #db_int_hash(DBKey,unsigned short)
 * @see #db_uint_hash(DBKey,unsigned short)
 * @see #db_string_hash(DBKey,unsigned short)
 * @see #db_istring_hash(DBKey,unsigned short)
 * @see #db_int64_hash(DBKey,unsigned short)
 * @see #db_uint64_hash(DBKey,unsigned short)
 */
DBHasher db_default_hash(DBType type)
{
	DB_COUNTSTAT(db_default_hash);
	switch (type) {
		case DB_INT:     return &db_int_hash;
		case DB_UINT:    return &db_uint_hash;
		case DB_STRING:  return &db_string_hash;
		case DB_ISTRING: return &db_istring_hash;
		case DB_INT64:   return &db_int64_hash;
		case DB_UINT64:  return &db_uint64_hash;
		default:
			ShowError("db_default_hash: Unknown database type %u\n", type);
			return NULL;
	}
}

/**
 * Returns the default releaser for the specified type of database with the
 * specified options.
 * NOTE: the options are fixed with {@link #db_fix_options(DBType,DBOptions)}
 * before choosing the releaser.
 * @param type Type of database
 * @param options Options of the database
 * @return Default releaser for the type of database with the specified options
 * @public
 * @see #db_release_nothing(DBKey,DBData,DBRelease)
 * @see #db_release_key(DBKey,DBData,DBRelease)
 * @see #db_release_data(DBKey,DBData,DBRelease)
 * @see #db_release_both(DBKey,DBData,DBRelease)
 * @see #db_custom_release(DBRelease)
 */
DBReleaser db_default_release(DBType type, DBOptions options)
{
	DB_COUNTSTAT(db_default_release);
	options = db_fix_options(type, options);
	if (options&DB_OPT_RELEASE_DATA) { // Release data, what about the key?
		if (options&(DB_OPT_DUP_KEY|DB_OPT_RELEASE_KEY))
			return &db_release_both; // Release both key and data
		return &db_release_data; // Only release data
	}
	if (options&(DB_OPT_DUP_KEY|DB_OPT_RELEASE_KEY))
		return &db_release_key; // Only release key
	return &db_release_nothing; // Release nothing
}

/**
 * Returns the releaser that releases the specified release options.
 * @param which Options that specified what the releaser releases
 * @return Releaser for the specified release options
 * @public
 * @see #db_release_nothing(DBKey,DBData,DBRelease)
 * @see #db_release_key(DBKey,DBData,DBRelease)
 * @see #db_release_data(DBKey,DBData,DBRelease)
 * @see #db_release_both(DBKey,DBData,DBRelease)
 * @see #db_default_release(DBType,DBOptions)
 */
DBReleaser db_custom_release(DBRelease which)
{
	DB_COUNTSTAT(db_custom_release);
	switch (which) {
		case DB_RELEASE_NOTHING: return &db_release_nothing;
		case DB_RELEASE_KEY:     return &db_release_key;
		case DB_RELEASE_DATA:    return &db_release_data;
		case DB_RELEASE_BOTH:    return &db_release_both;
		default:
			ShowError("db_custom_release: Unknown release options %u\n", which);
			return NULL;
	}
}

/**
 * Allocate a new database of the specified type.
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
 * @see #DBMap_impl
 * @see #db_fix_options(DBType,DBOptions)
 */
DBMap* db_alloc(const char *file, const char *func, int line, DBType type, DBOptions options, unsigned short maxlen) {
	DBMap_impl* db;
	unsigned int i;
	char ers_name[50];

#ifdef DB_ENABLE_STATS
	DB_COUNTSTAT(db_alloc);
	switch (type) {
		case DB_INT: DB_COUNTSTAT(db_int_alloc); break;
		case DB_UINT: DB_COUNTSTAT(db_uint_alloc); break;
		case DB_STRING: DB_COUNTSTAT(db_string_alloc); break;
		case DB_ISTRING: DB_COUNTSTAT(db_istring_alloc); break;
		case DB_INT64: DB_COUNTSTAT(db_int64_alloc); break;
		case DB_UINT64: DB_COUNTSTAT(db_uint64_alloc); break;
	}
#endif /* DB_ENABLE_STATS */
	db = ers_alloc(db_alloc_ers, struct DBMap_impl);

	options = db_fix_options(type, options);
	/* Interface of the database */
	db->vtable.iterator = db_obj_iterator;
	db->vtable.exists   = db_obj_exists;
	db->vtable.get      = db_obj_get;
	db->vtable.getall   = db_obj_getall;
	db->vtable.vgetall  = db_obj_vgetall;
	db->vtable.ensure   = db_obj_ensure;
	db->vtable.vensure  = db_obj_vensure;
	db->vtable.put      = db_obj_put;
	db->vtable.remove   = db_obj_remove;
	db->vtable.foreach  = db_obj_foreach;
	db->vtable.vforeach = db_obj_vforeach;
	db->vtable.clear    = db_obj_clear;
	db->vtable.vclear   = db_obj_vclear;
	db->vtable.destroy  = db_obj_destroy;
	db->vtable.vdestroy = db_obj_vdestroy;
	db->vtable.size     = db_obj_size;
	db->vtable.type     = db_obj_type;
	db->vtable.options  = db_obj_options;
	/* File and line of allocation */
	db->alloc_file = file;
	db->alloc_line = line;
	/* Lock system */
	db->free_list = NULL;
	db->free_count = 0;
	db->free_max = 0;
	db->free_lock = 0;
	/* Other */
	snprintf(ers_name, 50, "db_alloc:nodes:%s:%s:%d",func,file,line);
	db->nodes = ers_new(sizeof(struct dbn),ers_name,ERS_OPT_WAIT|ERS_OPT_FREE_NAME|ERS_OPT_CLEAN);
	db->cmp = db_default_cmp(type);
	db->hash = db_default_hash(type);
	db->release = db_default_release(type, options);
	for (i = 0; i < HASH_SIZE; i++)
		db->ht[i] = NULL;
	db->cache = NULL;
	db->type = type;
	db->options = options;
	db->item_count = 0;
	db->maxlen = maxlen;
	db->global_lock = 0;

	if( db->maxlen == 0 && (type == DB_STRING || type == DB_ISTRING) )
		db->maxlen = UINT16_MAX;

	return &db->vtable;
}

/**
 * Manual cast from 'int' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
DBKey db_i2key(int key)
{
	DBKey ret;

	DB_COUNTSTAT(db_i2key);
	ret.i = key;
	return ret;
}

/**
 * Manual cast from 'unsigned int' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
DBKey db_ui2key(unsigned int key)
{
	DBKey ret;

	DB_COUNTSTAT(db_ui2key);
	ret.ui = key;
	return ret;
}

/**
 * Manual cast from 'const char *' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
DBKey db_str2key(const char *key)
{
	DBKey ret;

	DB_COUNTSTAT(db_str2key);
	ret.str = key;
	return ret;
}

/**
 * Manual cast from 'int64' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
DBKey db_i642key(int64 key)
{
	DBKey ret;

	DB_COUNTSTAT(db_i642key);
	ret.i64 = key;
	return ret;
}

/**
 * Manual cast from 'uin64' to the union DBKey.
 * @param key Key to be casted
 * @return The key as a DBKey union
 * @public
 */
DBKey db_ui642key(uint64 key)
{
	DBKey ret;

	DB_COUNTSTAT(db_ui642key);
	ret.ui64 = key;
	return ret;
}

/**
 * Manual cast from 'int' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
DBData db_i2data(int data)
{
	DBData ret;

	DB_COUNTSTAT(db_i2data);
	ret.type = DB_DATA_INT;
	ret.u.i = data;
	return ret;
}

/**
 * Manual cast from 'unsigned int' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
DBData db_ui2data(unsigned int data)
{
	DBData ret;

	DB_COUNTSTAT(db_ui2data);
	ret.type = DB_DATA_UINT;
	ret.u.ui = data;
	return ret;
}

/**
 * Manual cast from 'void *' to the struct DBData.
 * @param data Data to be casted
 * @return The data as a DBData struct
 * @public
 */
DBData db_ptr2data(void *data)
{
	DBData ret;

	DB_COUNTSTAT(db_ptr2data);
	ret.type = DB_DATA_PTR;
	ret.u.ptr = data;
	return ret;
}

/**
 * Gets int type data from struct DBData.
 * If data is not int type, returns 0.
 * @param data Data
 * @return Integer value of the data.
 * @public
 */
int db_data2i(DBData *data)
{
	DB_COUNTSTAT(db_data2i);
	if (data && DB_DATA_INT == data->type)
		return data->u.i;
	return 0;
}

/**
 * Gets unsigned int type data from struct DBData.
 * If data is not unsigned int type, returns 0.
 * @param data Data
 * @return Unsigned int value of the data.
 * @public
 */
unsigned int db_data2ui(DBData *data)
{
	DB_COUNTSTAT(db_data2ui);
	if (data && DB_DATA_UINT == data->type)
		return data->u.ui;
	return 0;
}

/**
 * Gets void* type data from struct DBData.
 * If data is not void* type, returns NULL.
 * @param data Data
 * @return Void* value of the data.
 * @public
 */
void* db_data2ptr(DBData *data)
{
	DB_COUNTSTAT(db_data2ptr);
	if (data && DB_DATA_PTR == data->type)
		return data->u.ptr;
	return NULL;
}

/**
 * Initializes the database system.
 * @public
 * @see #db_final(void)
 */
void db_init(void) {
	db_iterator_ers = ers_new(sizeof(struct DBIterator_impl),"db.c::db_iterator_ers",ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK);
	db_alloc_ers = ers_new(sizeof(struct DBMap_impl),"db.c::db_alloc_ers",ERS_OPT_CLEAN|ERS_OPT_FLEX_CHUNK);
	ers_chunk_size(db_alloc_ers, 50);
	ers_chunk_size(db_iterator_ers, 10);
	DB_COUNTSTAT(db_init);
}

/**
 * Finalizes the database system.
 * @public
 * @see #db_init(void)
 */
void db_final(void)
{
#ifdef DB_ENABLE_STATS
	DB_COUNTSTAT(db_final);
	ShowInfo(CL_WHITE"Database nodes"CL_RESET":\n"
			"allocated %u, freed %u\n",
			stats.db_node_alloc, stats.db_node_free);
	ShowInfo(CL_WHITE"Database types"CL_RESET":\n"
			"DB_INT     : allocated %10u, destroyed %10u\n"
			"DB_UINT    : allocated %10u, destroyed %10u\n"
			"DB_STRING  : allocated %10u, destroyed %10u\n"
			"DB_ISTRING : allocated %10u, destroyed %10u\n"
			"DB_INT64   : allocated %10u, destroyed %10u\n"
			"DB_UINT64  : allocated %10u, destroyed %10u\n",
			stats.db_int_alloc,     stats.db_int_destroy,
			stats.db_uint_alloc,    stats.db_uint_destroy,
			stats.db_string_alloc,  stats.db_string_destroy,
			stats.db_istring_alloc, stats.db_istring_destroy,
			stats.db_int64_alloc,   stats.db_int64_destroy,
			stats.db_uint64_alloc,  stats.db_uint64_destroy);
	ShowInfo(CL_WHITE"Database function counters"CL_RESET":\n"
			"db_rotate_left     %10u, db_rotate_right    %10u,\n"
			"db_rebalance       %10u, db_rebalance_erase %10u,\n"
			"db_is_key_null     %10u,\n"
			"db_dup_key         %10u, db_dup_key_free    %10u,\n"
			"db_free_add        %10u, db_free_remove     %10u,\n"
			"db_free_lock       %10u, db_free_unlock     %10u,\n"
			"db_int_cmp         %10u, db_uint_cmp        %10u,\n"
			"db_string_cmp      %10u, db_istring_cmp     %10u,\n"
			"db_int64_cmp       %10u, db_uint64_cmp      %10u,\n"
			"db_int_hash        %10u, db_uint_hash       %10u,\n"
			"db_string_hash     %10u, db_istring_hash    %10u,\n"
			"db_int64_hash      %10u, db_uint64_hash     %10u,\n"
			"db_release_nothing %10u, db_release_key     %10u,\n"
			"db_release_data    %10u, db_release_both    %10u,\n"
			"dbit_first         %10u, dbit_last          %10u,\n"
			"dbit_next          %10u, dbit_prev          %10u,\n"
			"dbit_exists        %10u, dbit_remove        %10u,\n"
			"dbit_destroy       %10u, db_iterator        %10u,\n"
			"db_exits           %10u, db_get             %10u,\n"
			"db_getall          %10u, db_vgetall         %10u,\n"
			"db_ensure          %10u, db_vensure         %10u,\n"
			"db_put             %10u, db_remove          %10u,\n"
			"db_foreach         %10u, db_vforeach        %10u,\n"
			"db_clear           %10u, db_vclear          %10u,\n"
			"db_destroy         %10u, db_vdestroy        %10u,\n"
			"db_size            %10u, db_type            %10u,\n"
			"db_options         %10u, db_fix_options     %10u,\n"
			"db_default_cmp     %10u, db_default_hash    %10u,\n"
			"db_default_release %10u, db_custom_release  %10u,\n"
			"db_alloc           %10u, db_i2key           %10u,\n"
			"db_ui2key          %10u, db_str2key         %10u,\n"
			"db_i642key         %10u, db_ui642key        %10u,\n"
			"db_i2data          %10u, db_ui2data         %10u,\n"
			"db_ptr2data        %10u, db_data2i          %10u,\n"
			"db_data2ui         %10u, db_data2ptr        %10u,\n"
			"db_init            %10u, db_final           %10u\n",
			stats.db_rotate_left,     stats.db_rotate_right,
			stats.db_rebalance,       stats.db_rebalance_erase,
			stats.db_is_key_null,
			stats.db_dup_key,         stats.db_dup_key_free,
			stats.db_free_add,        stats.db_free_remove,
			stats.db_free_lock,       stats.db_free_unlock,
			stats.db_int_cmp,         stats.db_uint_cmp,
			stats.db_string_cmp,      stats.db_istring_cmp,
			stats.db_int64_cmp,       stats.db_uint64_cmp,
			stats.db_int_hash,        stats.db_uint_hash,
			stats.db_string_hash,     stats.db_istring_hash,
			stats.db_int64_hash,      stats.db_uint64_hash,
			stats.db_release_nothing, stats.db_release_key,
			stats.db_release_data,    stats.db_release_both,
			stats.dbit_first,         stats.dbit_last,
			stats.dbit_next,          stats.dbit_prev,
			stats.dbit_exists,        stats.dbit_remove,
			stats.dbit_destroy,       stats.db_iterator,
			stats.db_exists,          stats.db_get,
			stats.db_getall,          stats.db_vgetall,
			stats.db_ensure,          stats.db_vensure,
			stats.db_put,             stats.db_remove,
			stats.db_foreach,         stats.db_vforeach,
			stats.db_clear,           stats.db_vclear,
			stats.db_destroy,         stats.db_vdestroy,
			stats.db_size,            stats.db_type,
			stats.db_options,         stats.db_fix_options,
			stats.db_default_cmp,     stats.db_default_hash,
			stats.db_default_release, stats.db_custom_release,
			stats.db_alloc,           stats.db_i2key,
			stats.db_ui2key,          stats.db_str2key,
			stats.db_i642key,         stats.db_ui642key,
			stats.db_i2data,          stats.db_ui2data,
			stats.db_ptr2data,        stats.db_data2i,
			stats.db_data2ui,         stats.db_data2ptr,
			stats.db_init,            stats.db_final);
#endif /* DB_ENABLE_STATS */
	ers_destroy(db_iterator_ers);
	ers_destroy(db_alloc_ers);
}

// Link DB System - jAthena
void linkdb_insert( struct linkdb_node** head, void *key, void* data)
{
	struct linkdb_node *node;
	if( head == NULL ) return ;
	node = (struct linkdb_node*)aMalloc( sizeof(struct linkdb_node) );
	if( *head == NULL ) {
		// first node
		*head      = node;
		node->prev = NULL;
		node->next = NULL;
	} else {
		// link nodes
		node->next    = *head;
		node->prev    = (*head)->prev;
		(*head)->prev = node;
		(*head)       = node;
	}
	node->key  = key;
	node->data = data;
}

void linkdb_vforeach( struct linkdb_node** head, LinkDBFunc func, va_list ap) {
	struct linkdb_node *node;
	if( head == NULL ) return;
	node = *head;
	while ( node ) {
		va_list argscopy;
		va_copy(argscopy, ap);
		func(node->key, node->data, argscopy);
		va_end(argscopy);
		node = node->next;
	}
}

void linkdb_foreach( struct linkdb_node** head, LinkDBFunc func, ...) {
	va_list ap;
	va_start(ap, func);
	linkdb_vforeach(head, func, ap);
	va_end(ap);
}

void* linkdb_search( struct linkdb_node** head, void *key)
{
	int n = 0;
	struct linkdb_node *node;
	if( head == NULL ) return NULL;
	node = *head;
	while( node ) {
		if( node->key == key ) {
			if( node->prev && n > 5 ) {
				//Moving the head in order to improve processing efficiency
				node->prev->next = node->next;
				if(node->next) node->next->prev = node->prev;
				node->next = *head;
				node->prev = (*head)->prev;
				(*head)->prev = node;
				(*head)       = node;
			}
			return node->data;
		}
		node = node->next;
		n++;
	}
	return NULL;
}

void* linkdb_erase( struct linkdb_node** head, void *key)
{
	struct linkdb_node *node;
	if( head == NULL ) return NULL;
	node = *head;
	while( node ) {
		if( node->key == key ) {
			void *data = node->data;
			if( node->prev == NULL )
				*head = node->next;
			else
				node->prev->next = node->next;
			if( node->next )
				node->next->prev = node->prev;
			aFree( node );
			return data;
		}
		node = node->next;
	}
	return NULL;
}

void linkdb_replace( struct linkdb_node** head, void *key, void *data )
{
	int n = 0;
	struct linkdb_node *node;
	if( head == NULL ) return ;
	node = *head;
	while( node ) {
		if( node->key == key ) {
			if( node->prev && n > 5 ) {
				//Moving the head in order to improve processing efficiency
				if(node->prev) node->prev->next = node->next;
				if(node->next) node->next->prev = node->prev;
				node->next = *head;
				node->prev = (*head)->prev;
				(*head)->prev = node;
				(*head)       = node;
			}
			node->data = data;
			return ;
		}
		node = node->next;
		n++;
	}
	//Insert because it can not find
	linkdb_insert( head, key, data );
}

void linkdb_final( struct linkdb_node** head )
{
	struct linkdb_node *node, *node2;
	if( head == NULL ) return ;
	node = *head;
	while( node ) {
		node2 = node->next;
		aFree( node );
		node = node2;
	}
	*head = NULL;
}
