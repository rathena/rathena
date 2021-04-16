// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CONFIG_RENEWAL_HPP
#define CONFIG_RENEWAL_HPP

//quick option to disable all renewal option, used by ./configure
#define PRERE
#ifndef PRERE
/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/config/
 **/


/**
 * @INFO: This file holds general-purpose renewal settings, for class-specific ones check /src/config/classes folder
 **/

/// Game renewal server mode
/// (disable by commenting the line)
///
/// Leave this line to enable renewal specific support such as renewal formulas
#define RENEWAL

/// Renewal cast time
/// (disable by commenting the line)
///
/// Leave this line to enable renewal casting time algorithms and enable fixed cast bonuses.
/// See also default_fixed_castrate in conf/battle/skill.conf for default fixed cast time (default is 20%).
/// Cast time is altered be 2 portion, Variable Cast Time (VCT) and Fixed Cast Time (FCT).
/// By default FCT is 20% of VCT (some skills aren't)
/// - VCT is decreased by DEX * 2 + INT.
/// - FCT is NOT reduced by stats, reduced by equips or buffs.
/// Example:
///  On a skill whos cast time is 10s, only 8s may be reduced. the other 2s are part of a FCT
#define RENEWAL_CAST

/// Renewal drop rate algorithms
/// (disable by commenting the line)
///
/// Leave this line to enable renewal item drop rate algorithms
/// While enabled a special modified based on the difference between the player and monster level is applied
/// Based on the http://irowiki.org/wiki/Drop_System#Level_Factor table
#define RENEWAL_DROP

/// Renewal exp rate algorithms
/// (disable by commenting the line)
///
/// Leave this line to enable renewal item exp rate algorithms
/// While enabled a special modified based on the difference between the player and monster level is applied
#define RENEWAL_EXP

/// Renewal level modifier on damage
/// (disable by commenting the line)
///
// Leave this line to enable renewal base level modifier on skill damage (selected skills only)
#define RENEWAL_LVDMG

/// Renewal ASPD [malufett]
/// (disable by commenting the line)
///
/// Leave this line to enable renewal ASPD
/// - shield penalty is applied
/// - AGI has a greater factor in ASPD increase
/// - there is a change in how skills/items give ASPD
/// - some skill/item ASPD bonuses won't stack
#define RENEWAL_ASPD

/// Renewal stat calculations
/// (disable by commenting the line)
///
/// Leave this line to enable renewal calculation for increasing status/parameter points
#define RENEWAL_STAT

#endif

#endif /* CONFIG_RENEWAL_HPP */
