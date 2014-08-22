// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _CONFIG_CORE_H_
#define _CONFIG_CORE_H_

/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/config/
 **/

/// Max number of items on @autolootid list
#define AUTOLOOTITEM_SIZE 10

/// The maximum number of atcommand and @warp suggestions
#define MAX_SUGGESTIONS 10

/// Comment to disable the official walk path
/// The official walkpath disables users from taking non-clear walk paths,
/// e.g. if they want to get around an obstacle they have to walk around it,
/// while with OFFICIAL_WALKPATH disabled if they click to walk around a obstacle the server will do it automatically
#define OFFICIAL_WALKPATH

/// leave this line uncommented to enable callfunc checks when processing scripts.
/// while allowed, the script engine will attempt to match user-defined functions
/// in scripts allowing direct function callback (without the use of callfunc.)
/// this CAN affect performance, so if you find scripts running slower or find
/// your map-server using more resources while this is active, comment the line
#define SCRIPT_CALLFUNC_CHECK

/// Uncomment to disable rAthena's anonymous stat report
/// We kindly ask you to consider keeping it enabled, it helps us improve rAthena.
//#define STATS_OPT_OUT

/// uncomment to enable query_sql script command and mysql logs to function on it's own thread
/// be aware this feature is under tests and you should use at your own risk, we however
/// welcome any feedback you may have regarding this feature, please send us all bug reports.
//#define BETA_THREAD_TEST

/// Uncomment to enable the Cell Stack Limit mod.
/// It's only config is the battle_config cell_stack_limit.
/// Only chars affected are those defined in BL_CHAR (mobs and players currently)
//#define CELL_NOSTACK

/// Uncomment to enable circular area checks.
/// By default, all range checks in Aegis are of Square shapes, so a weapon range
/// - of 10 allows you to attack from anywhere within a 21x21 area.
/// Enabling this changes such checks to circular checks, which is more realistic,
/// - but is not the official behaviour.
//#define CIRCULAR_AREA

/// Comment to disable Guild/Party Bound item system
/// By default, we recover/remove Guild/Party Bound items automatically
#define BOUND_ITEMS

/// Uncomment to enable real-time server stats (in and out data and ram usage).
//#define SHOW_SERVER_STATS

/// Uncomment to enable skills damage adjustments
/// By enabling this, db/skill_damage.txt and the skill_damage mapflag will adjust the
/// damage rate of specified skills.
#define ADJUST_SKILL_DAMAGE

/// Uncomment to enable the job base HP/SP table (job_basehpsp_db.txt)
//#define HP_SP_TABLES

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
#else
	#define MIN_STORAGE MAX_STORAGE // If the VIP system is disabled the min = max.
	#define MIN_CHARS MAX_CHARS // Default number of characters per account.
	#define MAX_CHAR_BILLING 0
	#define MAX_CHAR_VIP 0
#endif

/**
 * No settings past this point
 **/
#include "./renewal.h"
#include "./secure.h"
#include "./classes/general.h"

/**
 * Constants come last; so they process anything that could've been modified in early includes
 **/
#include "./const.h"

#endif // _CONFIG_CORE_H_
