#ifndef _RRCONFIGS_CONST_
#define _RRCONFIGS_CONST_
/**
 * rAthena configuration file (http://rathena.org)
 * For detailed guidance on these check http://rathena.org/wiki/SRC/map/config/
 **/

/**
 * @INFO: This file holds constants that aims at making code smoother and more efficient
 */

/**
 * "Constants"
 **/
#if RECASTING
	#define CONST_CASTRATE_SCALE RECASTING_VMIN
	/**
	 * Cast Rate Formula: (DEX x 2)+INT
	 **/
	#define CONST_CASTRATE_CALC ((status_get_dex(bl)*2)+status_get_int(bl))
#else
	#define CONST_CASTRATE_SCALE battle_config.castrate_dex_scale
	/**
	 * Cast Rate Formula: (DEX)
	 **/
	#define CONST_CASTRATE_CALC (status_get_dex(bl))
#endif


/**
 * "Sane Checks" to save you from compiling with cool bugs 
 **/
#if SECURE_NPCTIMEOUT_INTERVAL <= 0
	#error SECURE_NPCTIMEOUT_INTERVAL should be at least 1 (1s)
#endif
#if SECURE_NPCTIMEOUT < 0
	#error SECURE_NPCTIMEOUT cannot be lower than 0
#endif

/**
 * End of File
 **/
#endif
