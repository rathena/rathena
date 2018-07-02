// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CONFIG_SECURE_HPP_
#define _CONFIG_SECURE_HPP_

/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/config/
 **/

/**
 * @INFO: This file holds optional security settings
 **/

/**
 * Optional NPC Dialog Timer
 * When enabled all npcs dialog will 'timeout' if user is on idle for longer than the amount of seconds allowed
 * - On 'timeout' the npc dialog window changes its next/menu to a 'close' button
 * Comment to disable the timer.
 **/
#define SECURE_NPCTIMEOUT

/**
+ * Number of seconds after an 'input' field is displayed before invoking an idle timeout.
+ * Default: 180
 **/
#define NPC_SECURE_TIMEOUT_INPUT 180

/**
+ * Number of seconds after a 'menu' is displayed before invoking an idle timeout.
+ * Default: 60
 **/
#define NPC_SECURE_TIMEOUT_MENU 60

/**
+ * Number of seconds after a 'next' button is displayed before invoking an idle timeout.
+ * Default: 60
 **/
#define NPC_SECURE_TIMEOUT_NEXT 60

/**
 * (Secure) Optional NPC Dialog Timer
 * @requirement : SECURE_NPCTIMEOUT must be enabled
 * Minimum Interval Between timeout checks in seconds
 * Default: 1s
 **/
#define SECURE_NPCTIMEOUT_INTERVAL 1

#endif // _CONFIG_SECURE_HPP_
