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

/*==========================================
 * @runetablet [close|info [<char name>]] - Opens or closes the native Rune
 * Tablet window, or reports equipped Rune Sets and their applied effects.
 * Restricted to group level 99; players use the Rune Stone NPCs.
 * "activate" is kept as a compatibility alias for opening.
 *------------------------------------------*/
ACMD_FUNC(runetablet) {
	nullpo_retr(-1, sd);

	if (pc_get_group_level(sd) < 99)
		return -1;

	if (message && strcmp(message, "close") == 0) {
		rune_tablet_close(*sd);
		return 0;
	}

	if (message && strncmp(message, "info", 4) == 0 && (message[4] == '\0' || message[4] == ' ')) {
		const char* name = message + 4;
		map_session_data* target = sd;

		while (*name == ' ')
			++name;

		if (*name != '\0' && (target = map_nick2sd(name, false)) == nullptr) {
			clif_displaymessage(fd, msg_txt(sd, 3)); // Character not found.
			return -1;
		}

		rune_tablet_show_effects(*target, fd);
		return 0;
	}

	if (message && *message && strcmp(message, "activate") != 0) {
		clif_displaymessage(fd, "Usage: @runetablet [close|info [<char name>]]");
		return -1;
	}

	return rune_tablet_open(*sd) ? 0 : -1;
}

/*==========================================
 * @reloadrunetabletdb - Reload the Rune Tablet set and decomposition databases
 *------------------------------------------*/
ACMD_FUNC(reloadrunetabletdb) {
	nullpo_retr(-1, sd);

	rune_tablet_reload();
	clif_displaymessage(fd, "Rune Tablet databases have been reloaded.");

	return 0;
}
