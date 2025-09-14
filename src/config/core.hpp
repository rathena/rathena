// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CONFIG_CORE_HPP
#define CONFIG_CORE_HPP

/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/config/
 **/

#include <custom/defines_pre.hpp>

/// Max number of items on @autolootid list
#define AUTOLOOTITEM_SIZE 10

/// The maximum number of atcommand and @warp suggestions
#define MAX_SUGGESTIONS 10

/// Comment to disable the official walk path
/// The official walkpath disables ranged units from taking non-clear walk paths to attack a target,
/// e.g. if they need to get around an obstacle to attack, players will have to click to walk around it
/// before attacking and monsters will just drop target once they get in attack range and can't attack.
/// If disabled, the server automatically makes sure units find a position to attack from by moving closer.
/// Disabling this also stops skills from failing when the target has walked behind an obstacle during cast.
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

/// Comment to disable the job base HP/SP/AP table (job_basepoints.yml)
#define HP_SP_TABLES

/// Uncomment to enable VIP system.
//#define VIP_ENABLE

/// Enable VIP script changes? (requires VIP_ENABLE)
/// The primary effects of this are restrictions on non-VIP players, such as requiring
/// a Reset Stone to change into third classes, paying more for equipment upgrades, and
/// so forth. Note that the changes are based on euRO, not iRO.
#define VIP_SCRIPT 0

#ifdef VIP_ENABLE
	#ifndef MIN_STORAGE
		#define MIN_STORAGE 300 // Default number of storage slots.
	#endif
	#ifndef MAX_CHAR_VIP
		#define MAX_CHAR_VIP 6 // This must be less than MAX_CHARS
	#endif
#else
	#ifndef MIN_STORAGE
		#define MIN_STORAGE MAX_STORAGE // If the VIP system is disabled the min = max.
	#endif
	#ifndef MAX_CHAR_VIP
		#define MAX_CHAR_VIP 0
	#endif
#endif

#ifndef MAX_CHAR_BILLING
	#define MAX_CHAR_BILLING 0 // This must be less than MAX_CHARS
#endif

/// Comment to disable warnings for deprecated script commands
#define SCRIPT_COMMAND_DEPRECATION

/// Comment to disable warnings for deprecated script constants
#define SCRIPT_CONSTANT_DEPRECATION

// Uncomment to enable deprecated support for Windows XP and lower
// Note:
// Windows XP still has 32bit ticks. This means you need to restart your operating system before time
// overflows, which is approximately every ~49 days.
//#define DEPRECATED_WINDOWS_SUPPORT

// Uncomment to enable compilation for unsupported compilers
// Note:
// Compilation might work on these compilers, but they might not fully follow newer C++ rules and
// cause unexpected behavior.
// Do NOT create any issues or ask for help with these compilers.
//#define DEPRECATED_COMPILER_SUPPORT

/// Uncomment for use with Nemo patch ExtendCashShopPreview
//#define ENABLE_CASHSHOP_PREVIEW_PATCH

/// Uncomment for use with Nemo patch ExtendOldCashShopPreview
//#define ENABLE_OLD_CASHSHOP_PREVIEW_PATCH

#if defined(_DEBUG) || defined(DEBUG)
	#define DETAILED_LOADING_OUTPUT
#endif

/// Uncomment to forcibly disable the detailed loading output.
/// This will noticeably decrease the boot time of the map server by not having to print so many status messages.
//#undef DETAILED_LOADING_OUTPUT

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

#include <custom/defines_post.hpp>

#endif /* CONFIG_CORE_HPP */
