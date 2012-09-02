/*****************************************************************************\
 *  Copyright (c) Athena Dev Teams - Licensed under GNU GPL                  *
 *  For more information, see LICENCE in the main folder                     *
 *                                                                           *
 *  <H1>Entry Reusage System</H1>                                            *
 *                                                                           *
 *  There are several root entry managers, each with a different entry size. *
 *  Each manager will keep track of how many instances have been 'created'.  *
 *  They will only automatically destroy themselves after the last instance  *
 *  is destroyed.                                                            *
 *                                                                           *
 *  Entries can be allocated from the managers.                              *
 *  If it has reusable entries (freed entry), it uses one.                   *
 *  So no assumption should be made about the data of the entry.             *
 *  Entries should be freed in the manager they where allocated from.        *
 *  Failure to do so can lead to unexpected behaviours.                      *
 *                                                                           *
 *  <H2>Advantages:</H2>                                                     *
 *  - The same manager is used for entries of the same size.                 *
 *    So entries freed in one instance of the manager can be used by other   *
 *    instances of the manager.                                              *
 *  - Much less memory allocation/deallocation - program will be faster.     *
 *  - Avoids memory fragmentaion - program will run better for longer.       *
 *                                                                           *
 *  <H2>Disavantages:</H2>                                                   *
 *  - Unused entries are almost inevitable - memory being wasted.            *
 *  - A  manager will only auto-destroy when all of its instances are        *
 *    destroyed so memory will usually only be recovered near the end.       *
 *  - Always wastes space for entries smaller than a pointer.                *
 *                                                                           *
 *  WARNING: The system is not thread-safe at the moment.                    *
 *                                                                           *
 *  HISTORY:                                                                 *
 *    0.1 - Initial version                                                  *
 *                                                                           *
 * @version 0.1 - Initial version                                            *
 * @author Flavio @ Amazon Project                                           *
 * @encoding US-ASCII                                                        *
\*****************************************************************************/
#ifndef _ERS_H_
#define _ERS_H_

#include "../common/cbasetypes.h"

/*****************************************************************************\
 *  (1) All public parts of the Entry Reusage System.                        *
 *  DISABLE_ERS           - Define to disable this system.                   *
 *  ERS_ALIGNED           - Alignment of the entries in the blocks.          *
 *  ERS                   - Entry manager.                                   *
 *  ers_new               - Allocate an instance of an entry manager.        *
 *  ers_report            - Print a report about the current state.          *
 *  ers_force_destroy_all - Force the destruction of all the managers.       *
\*****************************************************************************/

/**
 * Define this to disable the Entry Reusage System.
 * All code except the typedef of ERInterface will be disabled.
 * To allow a smooth transition, 
 */
//#define DISABLE_ERS

/**
 * Entries are aligned to ERS_ALIGNED bytes in the blocks of entries.
 * By default it aligns to one byte, using the "natural order" of the entries.
 * This should NEVER be set to zero or less.
 * If greater than one, some memory can be wasted. This should never be needed 
 * but is here just in case some aligment issues arise.
 */
#ifndef ERS_ALIGNED
#	define ERS_ALIGNED 1
#endif /* not ERS_ALIGN_ENTRY */

enum ERSOptions {
	ERS_OPT_NONE           = 0,
	ERS_OPT_CLEAR          = 1,/* silently clears any entries left in the manager upon destruction */
};

/**
 * Public interface of the entry manager.
 * @param alloc Allocate an entry from this manager
 * @param free Free an entry allocated from this manager
 * @param entry_size Return the size of the entries of this manager
 * @param destroy Destroy this instance of the manager
 */
typedef struct eri {

	/**
	 * Allocate an entry from this entry manager.
	 * If there are reusable entries available, it reuses one instead.
	 * @param self Interface of the entry manager
	 * @return An entry
	 */
	void *(*alloc)(struct eri *self);

	/**
	 * Free an entry allocated from this manager.
	 * WARNING: Does not check if the entry was allocated by this manager.
	 * Freeing such an entry can lead to unexpected behaviour.
	 * @param self Interface of the entry manager
	 * @param entry Entry to be freed
	 */
	void (*free)(struct eri *self, void *entry);

	/**
	 * Return the size of the entries allocated from this manager.
	 * @param self Interface of the entry manager
	 * @return Size of the entries of this manager in bytes
	 */
	size_t (*entry_size)(struct eri *self);

	/**
	 * Destroy this instance of the manager.
	 * The manager is actually only destroyed when all the instances are destroyed.
	 * When destroying the manager a warning is shown if the manager has 
	 * missing/extra entries.
	 * @param self Interface of the entry manager
	 */
	void (*destroy)(struct eri *self);

} *ERS;

#ifdef DISABLE_ERS
// Use memory manager to allocate/free and disable other interface functions
#	define ers_alloc(obj,type) (type *)aMalloc(sizeof(type))
#	define ers_free(obj,entry) aFree(entry)
#	define ers_entry_size(obj) (size_t)0
#	define ers_destroy(obj)
// Disable the public functions
#	define ers_new(size,name,options) NULL
#	define ers_report()
#	define ers_force_destroy_all()
#else /* not DISABLE_ERS */
// These defines should be used to allow the code to keep working whenever 
// the system is disabled
#	define ers_alloc(obj,type) (type *)(obj)->alloc(obj)
#	define ers_free(obj,entry) (obj)->free((obj),(entry))
#	define ers_entry_size(obj) (obj)->entry_size(obj)
#	define ers_destroy(obj)    (obj)->destroy(obj)

/**
 * Get a new instance of the manager that handles the specified entry size.
 * Size has to greater than 0.
 * If the specified size is smaller than a pointer, the size of a pointer is 
 * used instead.
 * It's also aligned to ERS_ALIGNED bytes, so the smallest multiple of 
 * ERS_ALIGNED that is greater or equal to size is what's actually used.
 * @param The requested size of the entry in bytes
 * @return Interface of the object
 */
ERS ers_new(uint32 size, char *name, enum ERSOptions options);

/**
 * Print a report about the current state of the Entry Reusage System.
 * Shows information about the global system and each entry manager.
 * The number of entries are checked and a warning is shown if extra reusable 
 * entries are found.
 * The extra entries are included in the count of reusable entries.
 */
void ers_report(void);

/**
 * Forcibly destroy all the entry managers, checking for nothing.
 * The system is left as if no instances or entries had ever been allocated.
 * All previous entries and instances of the managers become invalid.
 * The use of this is NOT recommended.
 * It should only be used in extreme situations to make shure all the memory 
 * allocated by this system is released.
 */
void ers_force_destroy_all(void);
#endif /* DISABLE_ERS / not DISABLE_ERS */

#endif /* _ERS_H_ */
