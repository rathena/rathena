// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "inter.hpp"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h> // for stat/lstat/fstat - [Dekamaster/Ultimate GM Tool]
#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>
#include <stdlib.h>

#include "../common/cbasetypes.h"
#include "../common/malloc.h"
#include "../common/strlib.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/timer.h"

#include "char.hpp"
#include "char_logif.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"
#include "int_party.hpp"
#include "int_guild.hpp"
#include "int_storage.hpp"
#include "int_pet.hpp"
#include "int_homun.hpp"
#include "int_mercenary.hpp"
#include "int_mail.hpp"
#include "int_auction.hpp"
#include "int_quest.hpp"
#include "int_elemental.hpp"
#include "int_clan.hpp"
#include "int_achievement.hpp"

#define WISDATA_TTL (60*1000)	//Wis data Time To Live (60 seconds)
#define WISDELLIST_MAX 256		// Number of elements in the list Delete data Wis


Sql* sql_handle = NULL;	///Link to mysql db, connection FD

int char_server_port = 3306;
char char_server_ip[32] = "127.0.0.1";
char char_server_id[32] = "ragnarok";
char char_server_pw[32] = ""; // Allow user to send empty password (bugreport:7787)
char char_server_db[32] = "ragnarok";
char default_codepage[32] = ""; //Feature by irmin.
struct Inter_Config interserv_config;
unsigned int party_share_level = 10;

/// Received packet Lengths from map-server
int inter_recv_packet_length[] = {
	-1,-1, 7,-1, -1,13,36, (2+4+4+4+1+NAME_LENGTH),  0,-1, 0, 0,  0, 0,  0, 0,	// 3000-
	 6,-1, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0,	// 3010-
	-1,10,-1,14, 15+NAME_LENGTH,19, 6,-1, 14,14, 6, 0,  0, 0,  0, 0,	// 3020- Party
	-1, 6,-1,-1, 55,19, 6,-1, 14,-1,-1,-1, 18,19,186,-1,	// 3030-
	-1, 9, 0, 0,  0, 0, 0, 0,  8, 6,11,10, 10,-1,6+NAME_LENGTH, 0,	// 3040-
	-1,-1,10,10,  0,-1,12, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3050-  Auction System [Zephyrus]
	 6,-1, 6,-1, 16+NAME_LENGTH+ACHIEVEMENT_NAME_LENGTH, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3060-  Quest system [Kevin] [Inkfish] / Achievements [Aleos]
	-1,10, 6,-1,  0, 0, 0, 0,  0, 0, 0, 0, -1,10,  6,-1,	// 3070-  Mercenary packets [Zephyrus], Elemental packets [pakpil]
	48,14,-1, 6,  0, 0, 0, 0,  0, 0,13,-1,  0, 0,  0, 0,	// 3080-  Pet System, Storage
	-1,10,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 3090-  Homunculus packets [albator]
	 2,-1, 6, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,	// 30A0-  Clan packets
};

struct WisData {
	int id, fd, count, len, gmlvl;
	unsigned long tick;
	char src[NAME_LENGTH], dst[NAME_LENGTH], msg[512];
};
static DBMap* wis_db = NULL; // int wis_id -> struct WisData*
static int wis_dellist[WISDELLIST_MAX], wis_delnum;

/* from pc.c due to @accinfo. any ideas to replace this crap are more than welcome. */
const char* job_name(int class_) {
	switch (class_) {
		case JOB_NOVICE:
		case JOB_SWORDMAN:
		case JOB_MAGE:
		case JOB_ARCHER:
		case JOB_ACOLYTE:
		case JOB_MERCHANT:
		case JOB_THIEF:
			return msg_txt(JOB_NOVICE+class_);

		case JOB_KNIGHT:
		case JOB_PRIEST:
		case JOB_WIZARD:
		case JOB_BLACKSMITH:
		case JOB_HUNTER:
		case JOB_ASSASSIN:
			return msg_txt(7 - JOB_KNIGHT+class_);

		case JOB_KNIGHT2:
			return msg_txt(7);

		case JOB_CRUSADER:
		case JOB_MONK:
		case JOB_SAGE:
		case JOB_ROGUE:
		case JOB_ALCHEMIST:
		case JOB_BARD:
		case JOB_DANCER:
			return msg_txt(13 - JOB_CRUSADER+class_);

		case JOB_CRUSADER2:
			return msg_txt(13);

		case JOB_WEDDING:
		case JOB_SUPER_NOVICE:
		case JOB_GUNSLINGER:
		case JOB_NINJA:
		case JOB_XMAS:
			return msg_txt(20 - JOB_WEDDING+class_);

		case JOB_SUMMER:
			return msg_txt(71);

		case JOB_HANBOK:
		case JOB_OKTOBERFEST:
			return msg_txt(105 - JOB_HANBOK+class_);

		case JOB_NOVICE_HIGH:
		case JOB_SWORDMAN_HIGH:
		case JOB_MAGE_HIGH:
		case JOB_ARCHER_HIGH:
		case JOB_ACOLYTE_HIGH:
		case JOB_MERCHANT_HIGH:
		case JOB_THIEF_HIGH:
			return msg_txt(25 - JOB_NOVICE_HIGH+class_);

		case JOB_LORD_KNIGHT:
		case JOB_HIGH_PRIEST:
		case JOB_HIGH_WIZARD:
		case JOB_WHITESMITH:
		case JOB_SNIPER:
		case JOB_ASSASSIN_CROSS:
			return msg_txt(32 - JOB_LORD_KNIGHT+class_);

		case JOB_LORD_KNIGHT2:
			return msg_txt(32);

		case JOB_PALADIN:
		case JOB_CHAMPION:
		case JOB_PROFESSOR:
		case JOB_STALKER:
		case JOB_CREATOR:
		case JOB_CLOWN:
		case JOB_GYPSY:
			return msg_txt(38 - JOB_PALADIN + class_);

		case JOB_PALADIN2:
			return msg_txt(38);

		case JOB_BABY:
		case JOB_BABY_SWORDMAN:
		case JOB_BABY_MAGE:
		case JOB_BABY_ARCHER:
		case JOB_BABY_ACOLYTE:
		case JOB_BABY_MERCHANT:
		case JOB_BABY_THIEF:
			return msg_txt(45 - JOB_BABY + class_);

		case JOB_BABY_KNIGHT:
		case JOB_BABY_PRIEST:
		case JOB_BABY_WIZARD:
		case JOB_BABY_BLACKSMITH:
		case JOB_BABY_HUNTER:
		case JOB_BABY_ASSASSIN:
			return msg_txt(52 - JOB_BABY_KNIGHT + class_);

		case JOB_BABY_KNIGHT2:
			return msg_txt(52);

		case JOB_BABY_CRUSADER:
		case JOB_BABY_MONK:
		case JOB_BABY_SAGE:
		case JOB_BABY_ROGUE:
		case JOB_BABY_ALCHEMIST:
		case JOB_BABY_BARD:
		case JOB_BABY_DANCER:
			return msg_txt(58 - JOB_BABY_CRUSADER + class_);

		case JOB_BABY_CRUSADER2:
			return msg_txt(58);

		case JOB_SUPER_BABY:
			return msg_txt(65);

		case JOB_TAEKWON:
			return msg_txt(66);
		case JOB_STAR_GLADIATOR:
		case JOB_STAR_GLADIATOR2:
			return msg_txt(67);
		case JOB_SOUL_LINKER:
			return msg_txt(68);

		case JOB_GANGSI:
		case JOB_DEATH_KNIGHT:
		case JOB_DARK_COLLECTOR:
			return msg_txt(72 - JOB_GANGSI+class_);

		case JOB_RUNE_KNIGHT:
		case JOB_WARLOCK:
		case JOB_RANGER:
		case JOB_ARCH_BISHOP:
		case JOB_MECHANIC:
		case JOB_GUILLOTINE_CROSS:
			return msg_txt(75 - JOB_RUNE_KNIGHT+class_);

		case JOB_RUNE_KNIGHT_T:
		case JOB_WARLOCK_T:
		case JOB_RANGER_T:
		case JOB_ARCH_BISHOP_T:
		case JOB_MECHANIC_T:
		case JOB_GUILLOTINE_CROSS_T:
			return msg_txt(75 - JOB_RUNE_KNIGHT_T+class_);

		case JOB_ROYAL_GUARD:
		case JOB_SORCERER:
		case JOB_MINSTREL:
		case JOB_WANDERER:
		case JOB_SURA:
		case JOB_GENETIC:
		case JOB_SHADOW_CHASER:
			return msg_txt(81 - JOB_ROYAL_GUARD+class_);

		case JOB_ROYAL_GUARD_T:
		case JOB_SORCERER_T:
		case JOB_MINSTREL_T:
		case JOB_WANDERER_T:
		case JOB_SURA_T:
		case JOB_GENETIC_T:
		case JOB_SHADOW_CHASER_T:
			return msg_txt(81 - JOB_ROYAL_GUARD_T+class_);

		case JOB_RUNE_KNIGHT2:
		case JOB_RUNE_KNIGHT_T2:
			return msg_txt(75);

		case JOB_ROYAL_GUARD2:
		case JOB_ROYAL_GUARD_T2:
			return msg_txt(81);

		case JOB_RANGER2:
		case JOB_RANGER_T2:
			return msg_txt(77);

		case JOB_MECHANIC2:
		case JOB_MECHANIC_T2:
			return msg_txt(79);

		case JOB_BABY_RUNE:
		case JOB_BABY_WARLOCK:
		case JOB_BABY_RANGER:
		case JOB_BABY_BISHOP:
		case JOB_BABY_MECHANIC:
		case JOB_BABY_CROSS:
		case JOB_BABY_GUARD:
		case JOB_BABY_SORCERER:
		case JOB_BABY_MINSTREL:
		case JOB_BABY_WANDERER:
		case JOB_BABY_SURA:
		case JOB_BABY_GENETIC:
		case JOB_BABY_CHASER:
			return msg_txt(88 - JOB_BABY_RUNE+class_);

		case JOB_BABY_RUNE2:
			return msg_txt(88);

		case JOB_BABY_GUARD2:
			return msg_txt(94);

		case JOB_BABY_RANGER2:
			return msg_txt(90);

		case JOB_BABY_MECHANIC2:
			return msg_txt(92);

		case JOB_SUPER_NOVICE_E:
		case JOB_SUPER_BABY_E:
			return msg_txt(101 - JOB_SUPER_NOVICE_E+class_);

		case JOB_KAGEROU:
		case JOB_OBORO:
			return msg_txt(103 - JOB_KAGEROU+class_);

		case JOB_REBELLION:
			return msg_txt(106);

		case JOB_SUMMONER:
			return msg_txt(108);

		case JOB_BABY_SUMMONER:
			return msg_txt(109);

		case JOB_BABY_NINJA:
		case JOB_BABY_KAGEROU:
		case JOB_BABY_OBORO:
		case JOB_BABY_TAEKWON:
		case JOB_BABY_STAR_GLADIATOR:
		case JOB_BABY_SOUL_LINKER:
		case JOB_BABY_GUNSLINGER:
		case JOB_BABY_REBELLION:
			return msg_txt(110 - JOB_BABY_NINJA+class_);

		case JOB_BABY_STAR_GLADIATOR2:
			return msg_txt(114);

		default:
			return msg_txt(118);
	}
}

/**
  * [Dekamaster/Nightroad]
  **/
const char * geoip_countryname[253] = {"Unknown","Asia/Pacific Region","Europe","Andorra","United Arab Emirates","Afghanistan","Antigua and Barbuda","Anguilla","Albania","Armenia","Netherlands Antilles",
		"Angola","Antarctica","Argentina","American Samoa","Austria","Australia","Aruba","Azerbaijan","Bosnia and Herzegovina","Barbados",
		"Bangladesh","Belgium","Burkina Faso","Bulgaria","Bahrain","Burundi","Benin","Bermuda","Brunei Darussalam","Bolivia",
		"Brazil","Bahamas","Bhutan","Bouvet Island","Botswana","Belarus","Belize","Canada","Cocos (Keeling) Islands","Congo, The Democratic Republic of the",
		"Central African Republic","Congo","Switzerland","Cote D'Ivoire","Cook Islands","Chile","Cameroon","China","Colombia","Costa Rica",
		"Cuba","Cape Verde","Christmas Island","Cyprus","Czech Republic","Germany","Djibouti","Denmark","Dominica","Dominican Republic",
		"Algeria","Ecuador","Estonia","Egypt","Western Sahara","Eritrea","Spain","Ethiopia","Finland","Fiji",
		"Falkland Islands (Malvinas)","Micronesia, Federated States of","Faroe Islands","France","France, Metropolitan","Gabon","United Kingdom","Grenada","Georgia","French Guiana",
		"Ghana","Gibraltar","Greenland","Gambia","Guinea","Guadeloupe","Equatorial Guinea","Greece","South Georgia and the South Sandwich Islands","Guatemala",
		"Guam","Guinea-Bissau","Guyana","Hong Kong","Heard Island and McDonald Islands","Honduras","Croatia","Haiti","Hungary","Indonesia",
		"Ireland","Israel","India","British Indian Ocean Territory","Iraq","Iran, Islamic Republic of","Iceland","Italy","Jamaica","Jordan",
		"Japan","Kenya","Kyrgyzstan","Cambodia","Kiribati","Comoros","Saint Kitts and Nevis","Korea, Democratic People's Republic of","Korea, Republic of","Kuwait",
		"Cayman Islands","Kazakhstan","Lao People's Democratic Republic","Lebanon","Saint Lucia","Liechtenstein","Sri Lanka","Liberia","Lesotho","Lithuania",
		"Luxembourg","Latvia","Libyan Arab Jamahiriya","Morocco","Monaco","Moldova, Republic of","Madagascar","Marshall Islands","Macedonia","Mali",
		"Myanmar","Mongolia","Macau","Northern Mariana Islands","Martinique","Mauritania","Montserrat","Malta","Mauritius","Maldives",
		"Malawi","Mexico","Malaysia","Mozambique","Namibia","New Caledonia","Niger","Norfolk Island","Nigeria","Nicaragua",
		"Netherlands","Norway","Nepal","Nauru","Niue","New Zealand","Oman","Panama","Peru","French Polynesia",
		"Papua New Guinea","Philippines","Pakistan","Poland","Saint Pierre and Miquelon","Pitcairn Islands","Puerto Rico","Palestinian Territory","Portugal","Palau",
		"Paraguay","Qatar","Reunion","Romania","Russian Federation","Rwanda","Saudi Arabia","Solomon Islands","Seychelles","Sudan",
		"Sweden","Singapore","Saint Helena","Slovenia","Svalbard and Jan Mayen","Slovakia","Sierra Leone","San Marino","Senegal","Somalia","Suriname",
		"Sao Tome and Principe","El Salvador","Syrian Arab Republic","Swaziland","Turks and Caicos Islands","Chad","French Southern Territories","Togo","Thailand",
		"Tajikistan","Tokelau","Turkmenistan","Tunisia","Tonga","Timor-Leste","Turkey","Trinidad and Tobago","Tuvalu","Taiwan",
		"Tanzania, United Republic of","Ukraine","Uganda","United States Minor Outlying Islands","United States","Uruguay","Uzbekistan","Holy See (Vatican City State)","Saint Vincent and the Grenadines","Venezuela",
		"Virgin Islands, British","Virgin Islands, U.S.","Vietnam","Vanuatu","Wallis and Futuna","Samoa","Yemen","Mayotte","Serbia","South Africa",
		"Zambia","Montenegro","Zimbabwe","Anonymous Proxy","Satellite Provider","Other","Aland Islands","Guernsey","Isle of Man","Jersey",
		"Saint Barthelemy","Saint Martin"};
unsigned char *geoip_cache;
void geoip_readdb(void){
	struct stat bufa;
	FILE *db=fopen("./db/GeoIP.dat","rb");
	fstat(fileno(db), &bufa);
	geoip_cache = (unsigned char *) aMalloc(sizeof(unsigned char) * bufa.st_size);
	if(fread(geoip_cache, sizeof(unsigned char), bufa.st_size, db) != bufa.st_size) { ShowError("geoip_cache reading didn't read all elements \n"); }
	fclose(db);
	ShowStatus("Finished Reading " CL_GREEN "GeoIP" CL_RESET " Database.\n");
}
/* [Dekamaster/Nightroad] */
/* WHY NOT A DBMAP: There are millions of entries in GeoIP and it has its own algorithm to go quickly through them, a DBMap wouldn't be efficient */
const char* geoip_getcountry(uint32 ipnum){
	int depth;
	unsigned int x;
	unsigned int offset = 0;

	for (depth = 31; depth >= 0; depth--) {
		const unsigned char *buf;
		buf = geoip_cache + (long)6 *offset;
		if (ipnum & (1 << depth)) {
			/* Take the right-hand branch */
			x =   (buf[3*1 + 0] << (0*8))
				+ (buf[3*1 + 1] << (1*8))
				+ (buf[3*1 + 2] << (2*8));
		} else {
			/* Take the left-hand branch */
			x =   (buf[3*0 + 0] << (0*8))
				+ (buf[3*0 + 1] << (1*8))
				+ (buf[3*0 + 2] << (2*8));
		}
		if (x >= 16776960) {
			x=x-16776960;
			return geoip_countryname[x];
		}
		offset = x;
	}
	return geoip_countryname[0];
}
/* sends a mesasge to map server (fd) to a user (u_fd) although we use fd we keep aid for safe-check */
/* extremely handy I believe it will serve other uses in the near future */
void inter_to_fd(int fd, int u_fd, int aid, char* msg, ...) {
	char msg_out[512];
	va_list ap;
	int len = 1;/* yes we start at 1 */

	va_start(ap,msg);
		len += vsnprintf(msg_out, 512, msg, ap);
	va_end(ap);

	WFIFOHEAD(fd,12 + len);

	WFIFOW(fd,0) = 0x3807;
	WFIFOW(fd,2) = 12 + (unsigned short)len;
	WFIFOL(fd,4) = u_fd;
	WFIFOL(fd,8) = aid;
	safestrncpy(WFIFOCP(fd,12), msg_out, len);

	WFIFOSET(fd,12 + len);

	return;
}

/**
 * Transmit the result of a account_information request from map-serv, with type 1
 * @param fd : map-serv link
 * @param u_fd : player fd to send info to
 * @param acc_id : id of player found
 * @param acc_name : name of player found
 */
void mapif_acc_info_ack(int fd, int u_fd, int acc_id, const char* acc_name){
	WFIFOHEAD(fd,10 + NAME_LENGTH);
	WFIFOW(fd,0) = 0x3808;
	WFIFOL(fd,2) = u_fd;
	WFIFOL(fd,6) = acc_id;
	safestrncpy(WFIFOCP(fd,10),acc_name,NAME_LENGTH);
	WFIFOSET(fd,10 + NAME_LENGTH);
}

/**
 * Receive a account_info request from map-serv
 * @author : [Dekamaster/Nightroad]
 * @param fd : map-serv link
 */
void mapif_parse_accinfo(int fd) {
	int u_fd = RFIFOL(fd,2), u_aid = RFIFOL(fd,6), u_group = RFIFOL(fd,10);
	char type= RFIFOB(fd,14);
	char query[NAME_LENGTH], query_esq[NAME_LENGTH*2+1];
	uint32 account_id = 0;
	char *data;

	safestrncpy(query, RFIFOCP(fd,15), NAME_LENGTH);
	Sql_EscapeString(sql_handle, query_esq, query);

	account_id = atoi(query);

	if (account_id < START_ACCOUNT_NUM) {	// is string
		if ( SQL_ERROR == Sql_Query(sql_handle, "SELECT `account_id`,`name`,`class`,`base_level`,`job_level`,`online` FROM `%s` WHERE `name` LIKE '%s' LIMIT 10", schema_config.char_db, query_esq)
				|| Sql_NumRows(sql_handle) == 0 ) {
			if( Sql_NumRows(sql_handle) == 0 ) {
				inter_to_fd(fd, u_fd, u_aid, (char *)msg_txt(212) ,query);
			} else {
				Sql_ShowDebug(sql_handle);
				inter_to_fd(fd, u_fd, u_aid, (char *)msg_txt(213));
			}
			Sql_FreeResult(sql_handle);
			return;
		} else {
			if( Sql_NumRows(sql_handle) == 1 ) {//we found a perfect match
				Sql_NextRow(sql_handle);
				Sql_GetData(sql_handle, 0, &data, NULL); account_id = atoi(data);
				Sql_FreeResult(sql_handle);
			} else {// more than one, listing... [Dekamaster/Nightroad]
				inter_to_fd(fd, u_fd, u_aid, (char *)msg_txt(214),(int)Sql_NumRows(sql_handle));
				while ( SQL_SUCCESS == Sql_NextRow(sql_handle) ) {
					int class_;
					short base_level, job_level, online;
					char name[NAME_LENGTH];

					Sql_GetData(sql_handle, 0, &data, NULL); account_id = atoi(data);
					Sql_GetData(sql_handle, 1, &data, NULL); safestrncpy(name, data, sizeof(name));
					Sql_GetData(sql_handle, 2, &data, NULL); class_ = atoi(data);
					Sql_GetData(sql_handle, 3, &data, NULL); base_level = atoi(data);
					Sql_GetData(sql_handle, 4, &data, NULL); job_level = atoi(data);
					Sql_GetData(sql_handle, 5, &data, NULL); online = atoi(data);

					inter_to_fd(fd, u_fd, u_aid, (char *)msg_txt(215), account_id, name, job_name(class_), base_level, job_level, online?"Online":"Offline");
				}
				Sql_FreeResult(sql_handle);
				return;
			}
		}
	}

	/* it will only get here if we have a single match then ask login-server to fetch the `login` record */
	if (!account_id || chlogif_req_accinfo(fd, u_fd, u_aid, account_id, type) != 1) {
		inter_to_fd(fd, u_fd, u_aid, (char *)msg_txt(213));
	}
	return;
}

/**
 * Show account info from login-server to user
 */
void mapif_accinfo_ack(bool success, int map_fd, int u_fd, int u_aid, int account_id, int8 type,
	int group_id, int logincount, int state, const char *email, const char *last_ip, const char *lastlogin,
	const char *birthdate, const char *userid)
{
	
	if (map_fd <= 0 || !session_isActive(map_fd))
		return; // check if we have a valid fd

	if (!success) {
		inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(216), account_id);
		return;
	}

	if (type == 1) { //type 1 we don't want all the info [lighta] @CHECKME
		mapif_acc_info_ack(map_fd, u_fd, account_id, userid);
		return;
	}

	inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(217), account_id);
	inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(218), userid, group_id, state);
	inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(221), email, birthdate);
	inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(222), last_ip, geoip_getcountry(str2ip(last_ip)));
	inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(223), logincount, lastlogin);
	inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(224));

	if ( SQL_ERROR == Sql_Query(sql_handle, "SELECT `char_id`, `name`, `char_num`, `class`, `base_level`, `job_level`, `online` FROM `%s` WHERE `account_id` = '%d' ORDER BY `char_num` LIMIT %d", schema_config.char_db, account_id, MAX_CHARS)
		|| Sql_NumRows(sql_handle) == 0 )
	{
		if( Sql_NumRows(sql_handle) == 0 )
			inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(226));
		else {
			inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(213));
			Sql_ShowDebug(sql_handle);
		}
	} else {
		while ( SQL_SUCCESS == Sql_NextRow(sql_handle) ) {
			uint32 char_id, class_;
			short char_num, base_level, job_level, online;
			char name[NAME_LENGTH];
			char *data;

			Sql_GetData(sql_handle, 0, &data, NULL); char_id = atoi(data);
			Sql_GetData(sql_handle, 1, &data, NULL); safestrncpy(name, data, sizeof(name));
			Sql_GetData(sql_handle, 2, &data, NULL); char_num = atoi(data);
			Sql_GetData(sql_handle, 3, &data, NULL); class_ = atoi(data);
			Sql_GetData(sql_handle, 4, &data, NULL); base_level = atoi(data);
			Sql_GetData(sql_handle, 5, &data, NULL); job_level = atoi(data);
			Sql_GetData(sql_handle, 6, &data, NULL); online = atoi(data);

			inter_to_fd(map_fd, u_fd, u_aid, (char *)msg_txt(225), char_num, char_id, name, job_name(class_), base_level, job_level, online?"Online":"Offline");
		}
	}
	Sql_FreeResult(sql_handle);
}

/**
 * Handles save reg data from map server and distributes accordingly.
 *
 * @param val either str or int, depending on type
 * @param type false when int, true otherwise
 **/
void inter_savereg(uint32 account_id, uint32 char_id, const char *key, unsigned int index, intptr_t val, bool is_string)
{
	char esc_val[254*2+1];
	char esc_key[32*2+1];

	Sql_EscapeString(sql_handle, esc_key, key);
	if( is_string && val ) {
		Sql_EscapeString(sql_handle, esc_val, (char*)val);
	}
	if( key[0] == '#' && key[1] == '#' ) { // global account reg
		if( session_isValid(login_fd) )
			chlogif_send_global_accreg(key,index,val,is_string);
		else {
			ShowError("Login server unavailable, can't perform update on '%s' variable for AID:%d CID:%d\n",key,account_id,char_id);
		}
	} else if ( key[0] == '#' ) { // local account reg
		if( is_string ) {
			if( val ) {
				if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%s')", schema_config.acc_reg_str_table, account_id, esc_key, index, esc_val) )
					Sql_ShowDebug(sql_handle);
			} else {
				if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", schema_config.acc_reg_str_table, account_id, esc_key, index) )
					Sql_ShowDebug(sql_handle);
			}
		} else {
			if( val ) {
				if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%d')", schema_config.acc_reg_num_table, account_id, esc_key, index, (int)val) )
					Sql_ShowDebug(sql_handle);
			} else {
				if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", schema_config.acc_reg_num_table, account_id, esc_key, index) )
					Sql_ShowDebug(sql_handle);
			}
		}
	} else { /* char reg */
		if( is_string ) {
			if( val ) {
				if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`char_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%s')", schema_config.char_reg_str_table, char_id, esc_key, index, esc_val) )
					Sql_ShowDebug(sql_handle);
			} else {
				if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", schema_config.char_reg_str_table, char_id, esc_key, index) )
					Sql_ShowDebug(sql_handle);
			}
		} else {
			if( val ) {
				if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`char_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%d')", schema_config.char_reg_num_table, char_id, esc_key, index, (int)val) )
					Sql_ShowDebug(sql_handle);
			} else {
				if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", schema_config.char_reg_num_table, char_id, esc_key, index) )
					Sql_ShowDebug(sql_handle);
			}
		}
	}
}

// Load account_reg from sql (type=2)
int inter_accreg_fromsql(uint32 account_id, uint32 char_id, int fd, int type)
{
	char* data;
	size_t len;
	unsigned int plen = 0;

	switch( type ) {
		case 3: //char reg
			if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `char_id`='%d'", schema_config.char_reg_str_table, char_id) )
				Sql_ShowDebug(sql_handle);
			break;
		case 2: //account reg
			if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%d'", schema_config.acc_reg_str_table, account_id) )
				Sql_ShowDebug(sql_handle);
			break;
		case 1: //account2 reg
			ShowError("inter_accreg_fromsql: Char server shouldn't handle type 1 registry values (##). That is the login server's job!\n");
			return 0;
		default:
			ShowError("inter_accreg_fromsql: Invalid type %d\n", type);
			return 0;
	}

	WFIFOHEAD(fd, 60000 + 300);
	WFIFOW(fd, 0) = 0x3804;
	// 0x2 = length, set prior to being sent
	WFIFOL(fd, 4) = account_id;
	WFIFOL(fd, 8) = char_id;
	WFIFOB(fd, 12) = 0; // var type (only set when all vars have been sent, regardless of type)
	WFIFOB(fd, 13) = 1; // is string type
	WFIFOW(fd, 14) = 0; // count
	plen = 16;

	/**
	 * Vessel!
	 *
	 * str type
	 * { keyLength(B), key(<keyLength>), index(L), valLength(B), val(<valLength>) }
	 **/
	while ( SQL_SUCCESS == Sql_NextRow(sql_handle) ) {
		Sql_GetData(sql_handle, 0, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len; // won't be higher; the column size is 32
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += len;

		Sql_GetData(sql_handle, 1, &data, NULL);

		WFIFOL(fd, plen) = (unsigned int)atol(data);
		plen += 4;

		Sql_GetData(sql_handle, 2, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len; // won't be higher; the column size is 254
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += len;

		WFIFOW(fd, 14) += 1;

		if( plen > 60000 ) {
			WFIFOW(fd, 2) = plen;
			WFIFOSET(fd, plen);

			// prepare follow up
			WFIFOHEAD(fd, 60000 + 300);
			WFIFOW(fd, 0) = 0x3804;
			// 0x2 = length, set prior to being sent
			WFIFOL(fd, 4) = account_id;
			WFIFOL(fd, 8) = char_id;
			WFIFOB(fd, 12) = 0; // var type (only set when all vars have been sent, regardless of type)
			WFIFOB(fd, 13) = 1; // is string type
			WFIFOW(fd, 14) = 0; // count
			plen = 16;
		}
	}

	WFIFOW(fd, 2) = plen;
	WFIFOSET(fd, plen);

	Sql_FreeResult(sql_handle);

	switch( type ) {
		case 3: //char reg
			if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `char_id`='%d'", schema_config.char_reg_num_table, char_id))
				Sql_ShowDebug(sql_handle);
			break;
		case 2: //account reg
			if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%d'", schema_config.acc_reg_num_table, account_id))
				Sql_ShowDebug(sql_handle);
			break;
#if 0 // This is already checked above.
		case 1: //account2 reg
			ShowError("inter_accreg_fromsql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
			return 0;
#endif // 0
	}

	WFIFOHEAD(fd, 60000 + 300);
	WFIFOW(fd, 0) = 0x3804;
	// 0x2 = length, set prior to being sent
	WFIFOL(fd, 4) = account_id;
	WFIFOL(fd, 8) = char_id;
	WFIFOB(fd, 12) = 0; // var type (only set when all vars have been sent, regardless of type)
	WFIFOB(fd, 13) = 0; // is int type
	WFIFOW(fd, 14) = 0; // count
	plen = 16;

	/**
	 * Vessel!
	 *
	 * int type
	 * { keyLength(B), key(<keyLength>), index(L), value(L) }
	 **/
	while ( SQL_SUCCESS == Sql_NextRow(sql_handle) ) {
		Sql_GetData(sql_handle, 0, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len;/* won't be higher; the column size is 32 */
		plen += 1;

		safestrncpy(WFIFOCP(fd,plen), data, len);
		plen += len;

		Sql_GetData(sql_handle, 1, &data, NULL);

		WFIFOL(fd, plen) = (unsigned int)atol(data);
		plen += 4;

		Sql_GetData(sql_handle, 2, &data, NULL);

		WFIFOL(fd, plen) = atoi(data);
		plen += 4;

		WFIFOW(fd, 14) += 1;

		if( plen > 60000 ) {
			WFIFOW(fd, 2) = plen;
			WFIFOSET(fd, plen);

			/* prepare follow up */
			WFIFOHEAD(fd, 60000 + 300);
			WFIFOW(fd, 0) = 0x3804;
			/* 0x2 = length, set prior to being sent */
			WFIFOL(fd, 4) = account_id;
			WFIFOL(fd, 8) = char_id;
			WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
			WFIFOB(fd, 13) = 0;/* is int type */
			WFIFOW(fd, 14) = 0;/* count */
			plen = 16;
		}
	}

	WFIFOB(fd, 12) = type;
	WFIFOW(fd, 2) = plen;
	WFIFOSET(fd, plen);

	Sql_FreeResult(sql_handle);
	return 1;
}

/*==========================================
 * read config file
 *------------------------------------------*/
int inter_config_read(const char* cfgName)
{
	char line[1024];
	FILE* fp;

	fp = fopen(cfgName, "r");
	if(fp == NULL) {
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}

	while(fgets(line, sizeof(line), fp)) {
		char w1[24], w2[1024];

		if (line[0] == '/' && line[1] == '/')
			continue;

		if (sscanf(line, "%23[^:]: %1023[^\r\n]", w1, w2) != 2)
			continue;

		if(!strcmpi(w1,"char_server_ip"))
			strcpy(char_server_ip,w2);
		else if(!strcmpi(w1,"char_server_port"))
			char_server_port = atoi(w2);
		else if(!strcmpi(w1,"char_server_id"))
			strcpy(char_server_id,w2);
		else if(!strcmpi(w1,"char_server_pw"))
			strcpy(char_server_pw,w2);
		else if(!strcmpi(w1,"char_server_db"))
			strcpy(char_server_db,w2);
		else if(!strcmpi(w1,"default_codepage"))
			strcpy(default_codepage,w2);
		else if(!strcmpi(w1,"party_share_level"))
			party_share_level = (unsigned int)atof(w2);
		else if(!strcmpi(w1,"log_inter"))
			charserv_config.log_inter = atoi(w2);
		else if(!strcmpi(w1,"inter_server_conf"))
			interserv_config.cfgFile = w2;
		else if(!strcmpi(w1,"import"))
			inter_config_read(w2);
	}
	fclose(fp);

	ShowInfo ("Done reading %s.\n", cfgName);

	return 0;
}

// Save interlog into sql
int inter_log(const char* fmt, ...)
{
	char str[255];
	char esc_str[sizeof(str)*2+1];// escaped str
	va_list ap;

	va_start(ap,fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	Sql_EscapeStringLen(sql_handle, esc_str, str, strnlen(str, sizeof(str)));
	if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s` (`time`, `log`) VALUES (NOW(),  '%s')", schema_config.interlog_db, esc_str) )
		Sql_ShowDebug(sql_handle);

	return 0;
}

void yaml_invalid_warning(const char* fmt, YAML::Node &node, std::string &file) {
	YAML::Emitter out;
	out << node;
	ShowWarning(fmt, file.c_str());
	ShowMessage("%s\n", out.c_str());
}

/**
 * Read inter config file
 **/
void inter_config_readConf(void) {
	std::vector<std::string> directories = { "conf/", "conf/import/" };
	static const std::string file_name(interserv_config.cfgFile);

	for (auto directory : directories) {
		std::string current_file = directory + file_name;
		YAML::Node config;
		int count = 0;

		try {
			config = YAML::LoadFile(current_file);
		}
		catch (std::exception &e) {
			ShowError("Cannot read storage definition file '" CL_WHITE "%s" CL_RESET "' (Caused by : " CL_WHITE "%s" CL_RESET ").\n", current_file.c_str(), e.what());
			return;
		}

		if (config["Storages"]) {
			for (auto node : config["Storages"]) {
				unsigned int id;

				if (!node["ID"]) {
					yaml_invalid_warning("inter_config_readConf: Storage definition with no ID field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, current_file);
					continue;
				}

				try {
					id = node["ID"].as<unsigned int>();
				}
				catch (std::exception) {
					yaml_invalid_warning("inter_config_readConf: Storage definition with invalid ID field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, current_file);
					continue;
				}

				if( id > UINT8_MAX ){
					yaml_invalid_warning("inter_config_readConf: Storage definition with invalid ID field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, current_file);
					continue;
				}

				bool existing = inter_premiumStorage_exists(id);
				auto storage_table = existing ? interserv_config.storages[id] : std::make_shared<s_storage_table>();

				if (!existing && (!node["Name"] || !node["Table"])) {
					yaml_invalid_warning("inter_config_readConf: Invalid storage definition in '" CL_WHITE "%s" CL_RESET "'.\n", node, current_file);
					continue;
				}
				
				if (node["Name"])
					safestrncpy(storage_table->name, node["Name"].as<std::string>().c_str(), NAME_LENGTH);
				if(node["Table"])
					safestrncpy(storage_table->table, node["Table"].as<std::string>().c_str(), DB_NAME_LEN);
				if (node["Max"]) {
					try {
						storage_table->max_num = node["Max"].as<uint16>();
					}
					catch (std::exception) {
						yaml_invalid_warning("inter_config_readConf: Storage definition with invalid Max field in '" CL_WHITE "%s" CL_RESET "', skipping.\n", node, current_file);
						continue;
					}
				}
				else if (!existing)
					storage_table->max_num = MAX_STORAGE;

				if (!existing) {
					storage_table->id = (uint8)id;
					interserv_config.storages[id] = storage_table;
				}

				count++;
			}
		}
		ShowStatus("Done reading '" CL_WHITE "%d" CL_RESET "' storage information in '" CL_WHITE "%s" CL_RESET "'\n", count, current_file.c_str());
	}
}

void inter_config_finalConf(void) {

}

void inter_config_defaults(void) {
	interserv_config.cfgFile = "inter_server.yml";
}

// initialize
int inter_init_sql(const char *file)
{
	inter_config_defaults();
	inter_config_read(file);

	//DB connection initialized
	sql_handle = Sql_Malloc();
	ShowInfo("Connect Character DB server.... (Character Server)\n");
	if( SQL_ERROR == Sql_Connect(sql_handle, char_server_id, char_server_pw, char_server_ip, (uint16)char_server_port, char_server_db) )
	{
		ShowError("Couldn't connect with username = '%s', password = '%s', host = '%s', port = '%d', database = '%s'\n",
			char_server_id, char_server_pw, char_server_ip, char_server_port, char_server_db);
		Sql_ShowDebug(sql_handle);
		Sql_Free(sql_handle);
		exit(EXIT_FAILURE);
	}

	if( *default_codepage ) {
		if( SQL_ERROR == Sql_SetEncoding(sql_handle, default_codepage) )
			Sql_ShowDebug(sql_handle);
	}

	wis_db = idb_alloc(DB_OPT_RELEASE_DATA);
	inter_config_readConf();
	inter_guild_sql_init();
	inter_storage_sql_init();
	inter_party_sql_init();
	inter_pet_sql_init();
	inter_homunculus_sql_init();
	inter_mercenary_sql_init();
	inter_elemental_sql_init();
	inter_mail_sql_init();
	inter_auction_sql_init();
	inter_clan_init();

	geoip_readdb();
	return 0;
}

// finalize
void inter_final(void)
{
	wis_db->destroy(wis_db, NULL);

	inter_config_finalConf();
	inter_guild_sql_final();
	inter_storage_sql_final();
	inter_party_sql_final();
	inter_pet_sql_final();
	inter_homunculus_sql_final();
	inter_mercenary_sql_final();
	inter_elemental_sql_final();
	inter_mail_sql_final();
	inter_auction_sql_final();
	inter_clan_final();

	if(geoip_cache) aFree(geoip_cache);
	
	return;
}

/**
 * IZ 0x388c <len>.W { <storage_table>.? }*?
 * Sends storage information to map-server
 * @param fd
 **/
void inter_Storage_sendInfo(int fd) {
	int size = sizeof(struct s_storage_table), len = 4 + interserv_config.storages.size() * size, offset;
	// Send storage table information
	WFIFOHEAD(fd, len);
	WFIFOW(fd, 0) = 0x388c;
	WFIFOW(fd, 2) = len;
	offset = 4;
	for (auto storage : interserv_config.storages) {
		memcpy(WFIFOP(fd, offset), storage.second.get(), size);
		offset += size;
	}
	WFIFOSET(fd, len);
}

int inter_mapif_init(int fd)
{
	inter_Storage_sendInfo(fd);
	return 0;
}


//--------------------------------------------------------

// broadcast sending
int mapif_broadcast(unsigned char *mes, int len, unsigned long fontColor, short fontType, short fontSize, short fontAlign, short fontY, int sfd)
{
	unsigned char *buf = (unsigned char*)aMalloc((len)*sizeof(unsigned char));

	WBUFW(buf,0) = 0x3800;
	WBUFW(buf,2) = len;
	WBUFL(buf,4) = fontColor;
	WBUFW(buf,8) = fontType;
	WBUFW(buf,10) = fontSize;
	WBUFW(buf,12) = fontAlign;
	WBUFW(buf,14) = fontY;
	memcpy(WBUFP(buf,16), mes, len - 16);
	chmapif_sendallwos(sfd, buf, len);

	aFree(buf);
	return 0;
}

// Wis sending
int mapif_wis_message(struct WisData *wd)
{
	unsigned char buf[2048];
	int headersize = 12 + 2 * NAME_LENGTH;

	if (wd->len > 2047-headersize) wd->len = 2047-headersize; //Force it to fit to avoid crashes. [Skotlex]

	WBUFW(buf, 0) = 0x3801;
	WBUFW(buf, 2) = headersize+wd->len;
	WBUFL(buf, 4) = wd->id;
	WBUFL(buf, 8) = wd->gmlvl;
	safestrncpy(WBUFCP(buf,12), wd->src, NAME_LENGTH);
	safestrncpy(WBUFCP(buf,12 + NAME_LENGTH), wd->dst, NAME_LENGTH);
	safestrncpy(WBUFCP(buf,12 + 2*NAME_LENGTH), wd->msg, wd->len);
	wd->count = chmapif_sendall(buf,WBUFW(buf,2));

	return 0;
}

// Send the requested account_reg
int mapif_account_reg_reply(int fd, uint32 account_id, uint32 char_id, int type)
{
	inter_accreg_fromsql(account_id,char_id,fd,type);
	return 0;
}

//Request to kick char from a certain map server. [Skotlex]
int mapif_disconnectplayer(int fd, uint32 account_id, uint32 char_id, int reason)
{
	if (fd >= 0)
	{
		WFIFOHEAD(fd,7);
		WFIFOW(fd,0) = 0x2b1f;
		WFIFOL(fd,2) = account_id;
		WFIFOB(fd,6) = reason;
		WFIFOSET(fd,7);
		return 0;
	}
	return -1;
}

//--------------------------------------------------------

/**
 * Existence check of WISP data
 * @see DBApply
 */
int check_ttl_wisdata_sub(DBKey key, DBData *data, va_list ap)
{
	unsigned long tick;
	struct WisData *wd = (struct WisData *)db_data2ptr(data);
	tick = va_arg(ap, unsigned long);

	if (DIFF_TICK(tick, wd->tick) > WISDATA_TTL && wis_delnum < WISDELLIST_MAX)
		wis_dellist[wis_delnum++] = wd->id;

	return 0;
}

int check_ttl_wisdata(void)
{
	unsigned long tick = gettick();
	int i;

	do {
		wis_delnum = 0;
		wis_db->foreach(wis_db, check_ttl_wisdata_sub, tick);
		for(i = 0; i < wis_delnum; i++) {
			struct WisData *wd = (struct WisData*)idb_get(wis_db, wis_dellist[i]);
			ShowWarning("inter: wis data id=%d time out : from %s to %s\n", wd->id, wd->src, wd->dst);
			// removed. not send information after a timeout. Just no answer for the player
			//mapif_wis_reply(wd->fd, wd->src, 1); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
			idb_remove(wis_db, wd->id);
		}
	} while(wis_delnum >= WISDELLIST_MAX);

	return 0;
}

//--------------------------------------------------------

// broadcast sending
int mapif_parse_broadcast(int fd)
{
	mapif_broadcast(RFIFOP(fd,16), RFIFOW(fd,2), RFIFOL(fd,4), RFIFOW(fd,8), RFIFOW(fd,10), RFIFOW(fd,12), RFIFOW(fd,14), fd);
	return 0;
}

/**
 * Parse received item broadcast and sends it to all connected map-serves
 * ZI 3009 <cmd>.W <len>.W <nameid>.W <source>.W <type>.B <name>.24B <srcname>.24B
 * IZ 3809 <cmd>.W <len>.W <nameid>.W <source>.W <type>.B <name>.24B <srcname>.24B
 * @param fd
 * @return
 **/
int mapif_parse_broadcast_item(int fd) {
	unsigned char buf[9 + NAME_LENGTH*2];

	memcpy(WBUFP(buf, 0), RFIFOP(fd, 0), RFIFOW(fd,2));
	WBUFW(buf, 0) = 0x3809;
	chmapif_sendallwos(fd, buf, RFIFOW(fd,2));

	return 0;
}

// Wis sending result
// flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
int mapif_wis_reply( int mapserver_fd, char* target, uint8 flag ){
	unsigned char buf[27];

	WBUFW(buf, 0) = 0x3802;
	safestrncpy(WBUFCP(buf, 2), target, NAME_LENGTH);
	WBUFB(buf,26) = flag;

	return chmapif_send(mapserver_fd, buf, 27);
}

// Wisp/page request to send
int mapif_parse_WisRequest(int fd)
{
	struct WisData* wd;
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1];// escaped name
	char* data;
	size_t len;
	int headersize = 8+2*NAME_LENGTH;


	if ( fd <= 0 ) {return 0;} // check if we have a valid fd

	if (RFIFOW(fd,2)-headersize >= sizeof(wd->msg)) {
		ShowWarning("inter: Wis message size too long.\n");
		return 0;
	} else if (RFIFOW(fd,2)-headersize <= 0) { // normaly, impossible, but who knows...
		ShowError("inter: Wis message doesn't exist.\n");
		return 0;
	}

	safestrncpy(name, RFIFOCP(fd,8+NAME_LENGTH), NAME_LENGTH); //Received name may be too large and not contain \0! [Skotlex]

	Sql_EscapeStringLen(sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `name` FROM `%s` WHERE `name`='%s'", schema_config.char_db, esc_name) )
		Sql_ShowDebug(sql_handle);

	// search if character exists before to ask all map-servers
	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		mapif_wis_reply(fd, RFIFOCP(fd, 8), 1);
	}
	else
	{// Character exists. So, ask all map-servers
		// to be sure of the correct name, rewrite it
		Sql_GetData(sql_handle, 0, &data, &len);
		memset(name, 0, NAME_LENGTH);
		memcpy(name, data, zmin(len, NAME_LENGTH));
		// if source is destination, don't ask other servers.
		if( strncmp(RFIFOCP(fd,8), name, NAME_LENGTH) == 0 )
		{
			mapif_wis_reply(fd, RFIFOCP(fd, 8), 1);
		}
		else
		{
			static int wisid = 0;

			CREATE(wd, struct WisData, 1);

			// Whether the failure of previous wisp/page transmission (timeout)
			check_ttl_wisdata();

			wd->id = ++wisid;
			wd->fd = fd;
			wd->len= RFIFOW(fd,2)-headersize;
			wd->gmlvl = RFIFOL(fd,4);
			safestrncpy(wd->src, RFIFOCP(fd,8), NAME_LENGTH);
			safestrncpy(wd->dst, RFIFOCP(fd,8+NAME_LENGTH), NAME_LENGTH);
			safestrncpy(wd->msg, RFIFOCP(fd,8+2*NAME_LENGTH), wd->len);
			wd->tick = gettick();
			idb_put(wis_db, wd->id, wd);
			mapif_wis_message(wd);
		}
	}

	Sql_FreeResult(sql_handle);
	return 0;
}


// Wisp/page transmission result
int mapif_parse_WisReply(int fd)
{
	int id;
	uint8 flag;
	struct WisData *wd;

	id = RFIFOL(fd,2);
	flag = RFIFOB(fd,6);
	wd = (struct WisData*)idb_get(wis_db, id);
	if (wd == NULL)
		return 0;	// This wisp was probably suppress before, because it was timeout of because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif_wis_reply(wd->fd, wd->src, flag); // flag: 0: success to send wisper, 1: target character is not loged in?, 2: ignored by target
		idb_remove(wis_db, id);
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd)
{
	unsigned char buf[2048]; // 0x3003/0x3803 <packet_len>.w <wispname>.24B <permission>.L <message>.?B

	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2));
	WBUFW(buf, 0) = 0x3803;
	chmapif_sendall(buf, RFIFOW(fd,2));

	return 0;
}

// Save account_reg into sql (type=2)
int mapif_parse_Registry(int fd)
{
	int account_id = RFIFOL(fd, 4), char_id = RFIFOL(fd, 8), count = RFIFOW(fd, 12);

	if( count ) {
		int cursor = 14, i;
		bool isLoginActive = session_isActive(login_fd);

		if( isLoginActive )
			chlogif_upd_global_accreg(account_id,char_id);

		for(i = 0; i < count; i++) {
			size_t lenkey = RFIFOB( fd, cursor );
			const char* src_key= RFIFOCP(fd, cursor + 1);
			std::string key( src_key, lenkey );
			cursor += lenkey + 1;

			unsigned int  index = RFIFOL(fd, cursor);
			cursor += 4;

			switch (RFIFOB(fd, cursor++)) {
				// int
				case 0:
				{
					intptr_t lVal = RFIFOL( fd, cursor );
					inter_savereg( account_id, char_id, key.c_str(), index, lVal, false );
					cursor += 4;
					break;
				}
				case 1:
					inter_savereg(account_id,char_id,key.c_str(),index,0,false);
					break;
				// str
				case 2:
				{
					size_t len_val = RFIFOB( fd, cursor );
					const char* src_val= RFIFOCP(fd, cursor + 1);
					std::string sval( src_val, len_val );
					cursor += len_val + 1;
					inter_savereg( account_id, char_id, key.c_str(), index, (intptr_t)sval.c_str(), true );
					break;
				}
				case 3:
					inter_savereg(account_id,char_id,key.c_str(),index,0,true);
					break;
				default:
					ShowError("mapif_parse_Registry: unknown type %d\n",RFIFOB(fd, cursor - 1));
					return 1;
			}

		}

		if (isLoginActive)
			chlogif_prepsend_global_accreg();
	}
	return 0;
}

// Request the value of all registries.
int mapif_parse_RegistryRequest(int fd)
{
	//Load Char Registry
	if (RFIFOB(fd,12)) mapif_account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),3);
	//Load Account Registry
	if (RFIFOB(fd,11)) mapif_account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),2);
	//Ask Login Server for Account2 values.
	if (RFIFOB(fd,10)) chlogif_request_accreg2(RFIFOL(fd,2),RFIFOL(fd,6));
	return 1;
}

void mapif_namechange_ack(int fd, uint32 account_id, uint32 char_id, int type, int flag, char *name)
{
	WFIFOHEAD(fd, NAME_LENGTH+13);
	WFIFOW(fd, 0) = 0x3806;
	WFIFOL(fd, 2) = account_id;
	WFIFOL(fd, 6) = char_id;
	WFIFOB(fd,10) = type;
	WFIFOB(fd,11) = flag;
	memcpy(WFIFOP(fd, 12), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+13);
}

int mapif_parse_NameChangeRequest(int fd)
{
	uint32 account_id, char_id;
	int type;
	char* name;
	int i;

	account_id = RFIFOL(fd,2);
	char_id = RFIFOL(fd,6);
	type = RFIFOB(fd,10);
	name = RFIFOCP(fd,11);

	// Check Authorised letters/symbols in the name
	if (charserv_config.char_config.char_name_option == 1) { // only letters/symbols in char_name_letters are authorised
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(charserv_config.char_config.char_name_letters, name[i]) == NULL) {
			mapif_namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	} else if (charserv_config.char_config.char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(charserv_config.char_config.char_name_letters, name[i]) != NULL) {
			mapif_namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	}
	//TODO: type holds the type of object to rename.
	//If it were a player, it needs to have the guild information and db information
	//updated here, because changing it on the map won't make it be saved [Skotlex]

	//name allowed.
	mapif_namechange_ack(fd, account_id, char_id, type, 1, name);
	return 0;
}

//--------------------------------------------------------

/// Returns the length of the next complete packet to process,
/// or 0 if no complete packet exists in the queue.
///
/// @param length The minimum allowed length, or -1 for dynamic lookup
int inter_check_length(int fd, int length)
{
	if( length == -1 )
	{// variable-length packet
		if( RFIFOREST(fd) < 4 )
			return 0;
		length = RFIFOW(fd,2);
	}

	if( (int)RFIFOREST(fd) < length )
		return 0;

	return length;
}

int inter_parse_frommap(int fd)
{
	int cmd;
	int len = 0;
	cmd = RFIFOW(fd,0);
	// Check is valid packet entry
	if(cmd < 0x3000 || cmd >= 0x3000 + ARRAYLENGTH(inter_recv_packet_length) || inter_recv_packet_length[cmd - 0x3000] == 0)
		return 0;

	// Check packet length
	if((len = inter_check_length(fd, inter_recv_packet_length[cmd - 0x3000])) == 0)
		return 2;

	switch(cmd) {
	case 0x3000: mapif_parse_broadcast(fd); break;
	case 0x3001: mapif_parse_WisRequest(fd); break;
	case 0x3002: mapif_parse_WisReply(fd); break;
	case 0x3003: mapif_parse_WisToGM(fd); break;
	case 0x3004: mapif_parse_Registry(fd); break;
	case 0x3005: mapif_parse_RegistryRequest(fd); break;
	case 0x3006: mapif_parse_NameChangeRequest(fd); break;
	case 0x3007: mapif_parse_accinfo(fd); break;
	/* 0x3008 unused */
	case 0x3009: mapif_parse_broadcast_item(fd); break;
	default:
		if(  inter_party_parse_frommap(fd)
		  || inter_guild_parse_frommap(fd)
		  || inter_storage_parse_frommap(fd)
		  || inter_pet_parse_frommap(fd)
		  || inter_homunculus_parse_frommap(fd)
		  || inter_mercenary_parse_frommap(fd)
		  || inter_elemental_parse_frommap(fd)
		  || inter_mail_parse_frommap(fd)
		  || inter_auction_parse_frommap(fd)
		  || inter_quest_parse_frommap(fd)
		  || inter_clan_parse_frommap(fd)
		  || inter_achievement_parse_frommap(fd)
		   )
			break;
		else
			return 0;
	}

	RFIFOSKIP(fd, len);
	return 1;
}


