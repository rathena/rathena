// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef _CONFIG_CORE_H_
#define _CONFIG_CORE_H_

/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/map/config/
 **/

/**
 * Max number of items on @autolootid list
 **/
#define AUTOLOOTITEM_SIZE 10

//Uncomment to enable the Cell Stack Limit mod.
//It's only config is the battle_config cell_stack_limit.
//Only chars affected are those defined in BL_CHAR (mobs and players currently)
//#define CELL_NOSTACK

//Uncomment to enable circular area checks.
//By default, all range checks in Aegis are of Square shapes, so a weapon range
//  of 10 allows you to attack from anywhere within a 21x21 area.
//Enabling this changes such checks to circular checks, which is more realistic,
//  but is not the official behaviour.
//#define CIRCULAR_AREA

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

#endif // _CONFIG_CORE_H_
