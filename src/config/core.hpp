// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CONFIG_CORE_HPP_
#define _CONFIG_CORE_HPP_

/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/config/
 **/

#include "../custom/defines_pre.hpp"

/// Max number of items on @autolootid list
#define AUTOLOOTITEM_SIZE 10

/// The maximum number of atcommand and @warp suggestions
#define MAX_SUGGESTIONS 10

/// Comment to disable the official walk path
/// The official walkpath disables users from taking non-clear walk paths,
/// e.g. if they want to get around an obstacle they have to walk around it,
/// while with OFFICIAL_WALKPATH disabled if they click to walk around a obstacle the server will do it automatically
#define OFFICIAL_WALKPATH

/// Uncomment to enable the Cell Stack Limit mod.
/// It's only config is the battle_config custom_cell_stack_limit.
/// Only chars affected are those defined in BL_CHAR
//#define CELL_NOSTACK

/// Uncomment to enable circular area checks.
/// By default, most server-sided range checks in Aegis are of square shapes, so a monster
/// with a range of 4 can attack anything within a 9x9 area.
/// Client-sided range checks are, however, are always circular.
/// Enabling this changes all checks to circular checks, which is more realistic,
/// - but is not the official behaviour.
//#define CIRCULAR_AREA

/// Comment to disable Guild/Party Bound item system
/// By default, we recover/remove Guild/Party Bound items automatically
#define BOUND_ITEMS

/// Uncomment to enable real-time server stats (in and out data and ram usage).
//#define SHOW_SERVER_STATS

/// Uncomment to enable the job base HP/SP table (job_basehpsp_db.txt)
#define HP_SP_TABLES

/// Uncomment to enable VIP system.
//#define VIP_ENABLE

/// Enable VIP script changes? (requires VIP_ENABLE)
/// The primary effects of this are restrictions on non-VIP players, such as requiring
/// a Reset Stone to change into third classes, paying more for equipment upgrades, and
/// so forth. Note that the changes are based on euRO, not iRO.
#define VIP_SCRIPT 0

#ifdef VIP_ENABLE
	#define MIN_STORAGE 300 // Default number of storage slots.
	#define MIN_CHARS 3 // Default number of characters per account.
	#define MAX_CHAR_VIP 6 // This must be less than MAX_CHARS
	#define MAX_CHAR_BILLING 0 // This must be less than MAX_CHARS
#endif

/// Comment to disable warnings for deprecated script commands
#define SCRIPT_COMMAND_DEPRECATION

/// Comment to disable warnings for deprecated script constants
#define SCRIPT_CONSTANT_DEPRECATION

/**
 * No settings past this point
 **/
#include "./packets.hpp"
#include "./renewal.hpp"
#include "./secure.hpp"
#include "./classes/general.hpp"

/**
 * Constants come last; so they process anything that could've been modified in early includes
 **/
#include "./const.hpp"

#include "../custom/defines_post.hpp"

#endif // _CONFIG_CORE_HPP_
