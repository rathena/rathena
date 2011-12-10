#ifndef _RRCONFIGS_CONST_
#define _RRCONFIGS_CONST_
/**
 * Ragnarok Resources Configuration File (http://ro-resources.net)
 * The following settings are applied upon compiling the program,
 * therefore any settings you disable will not even be added to the program
 * making these settings the most performance-effiecient possible
 **/

/**
 * @INFO: This file holds constants that aims at making code smoother and more efficient
 */

/**
 * "Constants"
 **/
#define CONST_CASTRATE_SCALE ( RECASTING ? RECASTING_VMIN : battle_config.castrate_dex_scale )
#define CONST_CASTRATE_CALC ( RECASTING ? ((status_get_dex(bl)*2)+status_get_int(bl)) : status_get_dex(bl) )

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
