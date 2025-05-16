// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MSG_CONF_HPP
#define	MSG_CONF_HPP

#include <config/core.hpp>

enum lang_types {
	LANG_RUS = 0x01,
	LANG_SPN = 0x02,
	LANG_GRM = 0x04,
	LANG_CHN = 0x08,
	LANG_MAL = 0x10,
	LANG_IDN = 0x20,
	LANG_FRN = 0x40,
	LANG_POR = 0x80,
	LANG_THA = 0x100,
	LANG_MAX
};

#ifndef LANG_ENABLE
	// Multilanguage System.
	// Define which languages to enable (bitmask).
	// 0xFFF will enable all, while 0x000 will enable English only.
	#define LANG_ENABLE 0x000
#endif

//read msg in table
const char* _msg_txt(int32 msg_number,int32 size, char ** msg_table);
//store msg from txtfile into msg_table
int32 _msg_config_read(const char* cfgName,int32 size, char ** msg_table);
//clear msg_table
void _do_final_msg(int32 size, char ** msg_table);
//Lookups
int32 msg_langstr2langtype(char * langtype);
const char* msg_langtype2langstr(int32 langtype);
// Verify that the choosen langtype is enabled.
int32 msg_checklangtype(int32 lang, bool display);

#endif	/* MSG_CONF_HPP */
