// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef MMO_ACCOUNT_HPP
#define MMO_ACCOUNT_HPP

#include <memory>
#include <string>

#include "../common/cbasetypes.hpp"
#include "../common/mmo.hpp" // ACCOUNT_REG2_NUM
#include "../config/core.hpp"

#ifndef WEB_AUTH_TOKEN_LENGTH
#define WEB_AUTH_TOKEN_LENGTH 16+1
#endif

struct mmo_account {
	uint32 account_id;
	char userid[NAME_LENGTH];
	char pass[32+1];        // 23+1 for plaintext, 32+1 for md5-ed passwords
	char sex;               // gender (M/F/S)
	char email[40];         // e-mail (by default: a@a.com)
	unsigned int group_id;  // player group id
	uint8 char_slots;       // this accounts maximum character slots (maximum is limited to MAX_CHARS define in char server)
	unsigned int state;     // packet 0x006a value + 1 (0: compte OK)
	time_t unban_time;      // (timestamp): ban time limit of the account (0 = no ban)
	time_t expiration_time; // (timestamp): validity limit of the account (0 = unlimited)
	unsigned int logincount;// number of successful auth attempts
	char lastlogin[24];     // date+time of last successful login
	char last_ip[16];       // save of last IP of connection
	char birthdate[10+1];   // assigned birth date (format: YYYY-MM-DD)
	char pincode[PINCODE_LENGTH+1];		// pincode system
	time_t pincode_change;	// (timestamp): last time of pincode change
	char web_auth_token[WEB_AUTH_TOKEN_LENGTH]; // web authentication token (randomized on each login)
	int old_group;
	time_t vip_time;
};

#endif /* MMO_ACCOUNT_HPP */
