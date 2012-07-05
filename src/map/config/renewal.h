// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _CONFIG_RENEWAL_H_
#define _CONFIG_RENEWAL_H_

/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/map/config/
 **/

/**
 * @INFO: This file holds general-purpose renewal settings, for class-specific ones check /src/map/config/Skills folder
 **/

/// game renewal server mode
/// (disable by commenting the line)
///
/// leave this line to enable renewal specific support such as renewal formulas
#define RENEWAL

/// renewal cast time
/// (disable by commenting the line)
///
/// leave this line to enable renewal casting time algorithms
/// cast time is decreased by DEX * 2 + INT while 20% of the cast time is not reduced by stats.
/// example:
///  on a skill whos cast time is 10s, only 8s may be reduced. the other 2s are part of a
///  "fixed cast time" which can only be reduced by specialist items and skills
#define RENEWAL_CAST

/// renewal drop rate algorithms
/// (disable by commenting the line)
///
/// leave this line to enable renewal item drop rate algorithms
/// while enabled a special modified based on the difference between the player and monster level is applied
/// based on the http://irowiki.org/wiki/Drop_System#Level_Factor table
#define RENEWAL_DROP

/// renewal exp rate algorithms
/// (disable by commenting the line)
///
/// leave this line to enable renewal item exp rate algorithms
/// while enabled a special modified based on the difference between the player and monster level is applied
#define RENEWAL_EXP

/// renewal level modifier on damage
/// (disable by commenting the line)
/// 
// leave this line to enable renewal base level modifier on skill damage (selected skills only)
#define RENEWAL_LVDMG

/// renewal cast time variable cast requirement
///
/// this is the value required for no variable cast-time with stats.
/// formula: (DEX * 2) + INT
/// default: 530
#define RENEWAL_CAST_VMIN 530

/// renewal enchant deadly poison algorithm
///
/// leave this line to enable the renewed EDP algorithm
/// under renewal mode:
///  - damage is NOT increased by 400%
///  - it does NOT affect grimtooth
///  - weapon and status ATK are increased
#define RENEWAL_EDP

/// renewal ASPD [malufett]
/// (disable by commenting the line)
/// 
/// leave this line to enable renewal ASPD
/// - shield penalty is applied
/// - agi has a greater factor to your ASPD increase
/// - there is a change in how skill/items give ASPD
/// - some skills/items ASPD bonus won't stack
#define RENEWAL_ASPD

#endif // _CONFIG_RENEWAL_H_
