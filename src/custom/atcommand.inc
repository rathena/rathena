// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

#include "../map/map.hpp"
#include "../map/pc.hpp"
#include "../map/clif.hpp"
#include "nofx.hpp"
#include "../common/strlib.hpp"
#include "../common/msg_conf.hpp"
#include <algorithm> 
#include <cctype>    
#include <string>    

ACMD_FUNC(nofx) 
{

	uint16 skill_id = 0;
	char arg1_str[256]; 
	char arg2_str[256]; 
	char temp_str[256]; 

	int args_read = sscanf(message, "%255s %255s", arg1_str, arg2_str);

	if (args_read < 1) { 
		clif_displaymessage(sd->fd, "Usage: @nofx <Skill ID> {on|off|clear}");
		clif_displaymessage(sd->fd, "  'on' hide, 'off' show, 'clear' show all.");
		return 0;
	}

	safestrncpy(temp_str, arg1_str, sizeof(temp_str));
	std::transform(temp_str, temp_str + strlen(temp_str), temp_str, ::tolower);

	if (strcmp(temp_str, "clear") == 0) {
		if (args_read > 1) { 
			clif_displaymessage(sd->fd, "Usage: @nofx clear (does not take additional arguments).");
			
			return 0;
		}
		
		custom_effects_data* ce_data = CustomEffectManager::get_instance()->get_effects(sd->status.char_id);
		if (!ce_data) { 
			clif_displaymessage(sd->fd, "[Erreur] Custom effects data not found. Contact an administrator.");
			ShowError("ATCOMMAND: @nofx clear - Failed to get custom_effects_data for char_id %u.\n", sd->status.char_id);
			return 0;
		}

		ce_data->clear_all_hidden_skills();
		clif_displaymessage(sd->fd, "[Effets] All skill effects are now visible.");
		
		return 0; 
	}

	if (args_read < 2) { 
		clif_displaymessage(sd->fd, "Usage: @nofx <Skill ID> {on|off|clear}");
		clif_displaymessage(sd->fd, "  'on' hide, 'off' show, 'clear' show all.");
		
		return 0;
	}

	
	if (sscanf(arg1_str, "%hu", &skill_id) != 1) {
		clif_displaymessage(sd->fd, "Invalid skill ID. Must be a number.");
		
		return 0;
	}

	custom_effects_data* ce_data = CustomEffectManager::get_instance()->get_effects(sd->status.char_id);
	if (!ce_data) {
		clif_displaymessage(sd->fd, "[Erreur] Custom effects data not found. Contact an administrator.");
		ShowError("ATCOMMAND: @nofx - Failed to get custom_effects_data for char_id %u.\n", sd->status.char_id);
		return 0;
	}

	safestrncpy(temp_str, arg2_str, sizeof(temp_str));
	std::transform(temp_str, temp_str + strlen(temp_str), temp_str, ::tolower);

	if (strcmp(temp_str, "on") == 0) {
		ce_data->set_skill_effect_hidden(skill_id, true);
		char output_msg[256];
		snprintf(output_msg, sizeof(output_msg), "[Effets] Skill ID %hu is now hidden.", skill_id);
		clif_displaymessage(sd->fd, output_msg);
		
	} else if (strcmp(temp_str, "off") == 0) {
		ce_data->set_skill_effect_hidden(skill_id, false);
		char output_msg[256];
		snprintf(output_msg, sizeof(output_msg), "[Effets] Skill ID %hu is now visible.", skill_id);
		clif_displaymessage(sd->fd, output_msg);
		
	} else {
		clif_displaymessage(sd->fd, "Invalid action. Use 'on', 'off' or 'clear'.");
		
	}

	return 0;
}


