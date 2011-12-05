#ifndef _RRCONFIGS_
#define _RRCONFIGS_
/**
 * Ragnarok Resources Configuration File (http://ro-resources.net)
 * The following settings are applied upon compiling the program,
 * therefore any settings you disable will not even be added to the program
 * making these settings the most performance-effiecient possible
 **/

/**
 * @INFO: RREmu Settings Core
 * - For detailed guidance on these check http://trac.ro-resources.net/wiki/CoreConfiguration
 **/

/**
 * No settings past this point
 **/
#include "./Renewal.h"
#include "./Secure.h"
#include "./Skills/General.h"
/**
 * Constants come last; so they process anything that could've been modified in early includes
 **/
#include "./Data/Const.h"
/**
 * End of File
 **/
#endif
