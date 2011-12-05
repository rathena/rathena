#ifndef _RRCONFIGS_SKILLS_GENERAL_
#define _RRCONFIGS_SKILLS_GENERAL_
/**
 * Ragnarok Resources Configuration File (http://ro-resources.net)
 * The following settings are applied upon compiling the program,
 * therefore any settings you disable will not even be added to the program
 * making these settings the most performance-effiecient possible
 **/

/**
 * Default Magical Reflection Behavior
 * - When reflecting, reflected damage depends on gears caster is wearing, not target
 * - When disabled damage depends on gears target is wearing, not caster.
 * @values 1 (enabled) or 0 (disabled)
 **/
#define RR_MAGIC_REFLECTION 1

/**
 * No settings past this point
 **/
#include "Mage_Classes.h"
#include "Swordsman_Classes.h"
#endif
