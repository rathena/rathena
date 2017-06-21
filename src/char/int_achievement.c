// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "../common/db.h"
#include "../common/malloc.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/sql.h"
#include "../common/strlib.h"

#include "char.h"
#include "inter.h"
#include "int_achievement.h"
#include "int_mail.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Load achievements for a character.
 * @param char_id: Character ID
 * @param count: Pointer to return the number of found entries.
 * @return Array of found entries. It has *count entries, and it is care of the caller to aFree() it afterwards.
 */
struct achievement *mapif_achievements_fromsql(uint32 char_id, int *count)
{
	struct achievement *achievelog = NULL;
	struct achievement tmp_achieve;
	SqlStmt *stmt;
	StringBuf buf;
	int i;

	if (!count)
		return NULL;

	memset(&tmp_achieve, 0, sizeof(tmp_achieve));

	StringBuf_Init(&buf);
	StringBuf_AppendStr(&buf, "SELECT `id`, `complete`, `completeDate`, `gotReward`");
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", `count%d`", i + 1);
	StringBuf_Printf(&buf, " FROM `%s` WHERE `char_id` = '%u'", schema_config.achievement_table, char_id);

	stmt = SqlStmt_Malloc(sql_handle);
	if( SQL_ERROR == SqlStmt_PrepareStr(stmt, StringBuf_Value(&buf))
	||  SQL_ERROR == SqlStmt_Execute(stmt) )
	{
		SqlStmt_ShowDebug(stmt);
		SqlStmt_Free(stmt);
		StringBuf_Destroy(&buf);
		*count = 0;
		return NULL;
	}

	SqlStmt_BindColumn(stmt, 0, SQLDT_INT,  &tmp_achieve.achievement_id, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 1, SQLDT_CHAR, &tmp_achieve.complete, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_INT,  &tmp_achieve.completeDate, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 3, SQLDT_INT,  &tmp_achieve.gotReward, 0, NULL, NULL);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		SqlStmt_BindColumn(stmt, 4 + i, SQLDT_INT, &tmp_achieve.count[i], 0, NULL, NULL);

	*count = (int)SqlStmt_NumRows(stmt);
	if (*count > 0) {
		int i = 0;

		achievelog = (struct achievement *)aCalloc(*count, sizeof(struct achievement));
		while (SQL_SUCCESS == SqlStmt_NextRow(stmt)) {
			if (i >= *count) // Sanity check, should never happen
				break;
			memcpy(&achievelog[i++], &tmp_achieve, sizeof(tmp_achieve));
		}
		if (i < *count) {
			// Should never happen. Compact array
			*count = i;
			achievelog = (struct achievement *)aRealloc(achievelog, sizeof(struct achievement) * i);
		}
	}

	SqlStmt_Free(stmt);
	StringBuf_Clear(&buf);
	return achievelog;
}

/**
 * Deletes an achievement from a character's achievementlog.
 * @param char_id: Character ID
 * @param achievement_id: Achievement ID
 * @return false in case of errors, true otherwise
 */
bool mapif_achievement_delete(uint32 char_id, int achievement_id)
{
	if (SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `id` = '%d' AND `char_id` = '%u'", schema_config.achievement_table, achievement_id, char_id)) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Adds an achievement to a character's achievementlog.
 * @param char_id: Character ID
 * @param ad: Achievement data
 * @return false in case of errors, true otherwise
 */
bool mapif_achievement_add(uint32 char_id, struct achievement ad)
{
	StringBuf buf;
	int i;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`char_id`, `id`, `complete`, `completeDate`, `gotReward`", schema_config.achievement_table);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", `count%d`", i + 1);
	StringBuf_AppendStr(&buf, ")");
	StringBuf_Printf(&buf, " VALUES ('%u', '%d', '%d', '%u', '%d'", char_id, ad.achievement_id, ad.complete, (uint32)ad.completeDate, ad.gotReward);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", '%d'", ad.count[i]);
	StringBuf_AppendStr(&buf, ")");

	if (SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf))) {
		Sql_ShowDebug(sql_handle);
		StringBuf_Clear(&buf);
		return false;
	}

	StringBuf_Clear(&buf);

	return true;
}

/**
 * Updates an achievement in a character's achievementlog.
 * @param char_id: Character ID
 * @param ad: Achievement data
 * @return false in case of errors, true otherwise
 */
bool mapif_achievement_update(uint32 char_id, struct achievement ad)
{
	StringBuf buf;
	int i;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET `complete` = '%d', `completeDate` = '%u', `gotReward` = '%d'", schema_config.achievement_table, ad.complete, (uint32)ad.completeDate, ad.gotReward);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", `count%d` = '%d'", i + 1, ad.count[i]);
	StringBuf_Printf(&buf, " WHERE `id` = %d AND `char_id` = %u", ad.achievement_id, char_id);

	if (SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf))) {
		Sql_ShowDebug(sql_handle);
		StringBuf_Clear(&buf);
		return false;
	}

	StringBuf_Clear(&buf);

	return true;
}

/**
 * Notifies the map-server of the result of saving a character's achievementlog.
 */
void mapif_achievement_save( int fd, uint32 char_id, bool success ){
	WFIFOHEAD(fd, 7);
	WFIFOW(fd, 0) = 0x3863;
	WFIFOL(fd, 2) = char_id;
	WFIFOB(fd, 6) = success;
	WFIFOSET(fd, 7);
}

/**
 * Handles the save request from mapserver for a character's achievementlog.
 * Received achievements are saved, and an ack is sent back to the map server.
 * @see inter_parse_frommap
 */
int mapif_parse_achievement_save(int fd)
{
	int i, j, k, old_n, new_n = (RFIFOW(fd, 2) - 8) / sizeof(struct achievement);
	uint32 char_id = RFIFOL(fd, 4);
	struct achievement *old_ad = NULL, *new_ad = NULL;
	bool success = true;

	if (new_n > 0)
		new_ad = (struct achievement *)RFIFOP(fd, 8);

	old_ad = mapif_achievements_fromsql(char_id, &old_n);

	for (i = 0; i < new_n; i++) {
		ARR_FIND(0, old_n, j, new_ad[i].achievement_id == old_ad[j].achievement_id);
		if (j < old_n) { // Update existing achievements
			// Only counts, complete, and reward are changable.
			ARR_FIND(0, MAX_ACHIEVEMENT_OBJECTIVES, k, new_ad[i].count[k] != old_ad[j].count[k]);
			if (k != MAX_ACHIEVEMENT_OBJECTIVES || new_ad[i].complete != old_ad[j].complete || new_ad[i].gotReward != old_ad[j].gotReward) {
				if ((success = mapif_achievement_update(char_id, new_ad[i])) == false)
					break;
			}

			if (j < (--old_n)) {
				// Compact array
				memmove(&old_ad[j], &old_ad[j + 1], sizeof(struct achievement) * (old_n - j));
				memset(&old_ad[old_n], 0, sizeof(struct achievement));
			}
		} else { // Add new achievements
			if (new_ad[i].achievement_id) {
				if ((success = mapif_achievement_add(char_id, new_ad[i])) == false)
					break;
			}
		}
	}

	for (i = 0; i < old_n; i++) { // Achievements not in new_ad but in old_ad are to be erased.
		if ((success = mapif_achievement_delete(char_id, old_ad[i].achievement_id)) == false)
			break;
	}

	if (old_ad)
		aFree(old_ad);

	mapif_achievement_save(fd, char_id, success);

	return 0;
}

/**
 * Sends the achievementlog of a character to the map-server.
 */
void mapif_achievement_load( int fd, uint32 char_id ){
	struct achievement *tmp_achievementlog = NULL;
	int num_achievements = 0;

	tmp_achievementlog = mapif_achievements_fromsql(char_id, &num_achievements);

	WFIFOHEAD(fd, num_achievements * sizeof(struct achievement) + 8);
	WFIFOW(fd, 0) = 0x3862;
	WFIFOW(fd, 2) = num_achievements * sizeof(struct achievement) + 8;
	WFIFOL(fd, 4) = char_id;

	if (num_achievements > 0)
		memcpy(WFIFOP(fd, 8), tmp_achievementlog, sizeof(struct achievement) * num_achievements);

	WFIFOSET(fd, num_achievements * sizeof(struct achievement) + 8);

	if (tmp_achievementlog)
		aFree(tmp_achievementlog);
}

/**
 * Sends achievementlog to the map server
 * NOTE: Achievements sent to the player are only completed ones
 * @see inter_parse_frommap
 */
int mapif_parse_achievement_load(int fd)
{
	mapif_achievement_load( fd, RFIFOL(fd, 2) );

	return 0;
}

/**
 * Notify the map-server if claiming the reward has succeeded.
 */
void mapif_achievement_reward( int fd, uint32 char_id, int32 achievement_id, bool success ){
	WFIFOHEAD(fd, 11);
	WFIFOW(fd, 0) = 0x3864;
	WFIFOL(fd, 2) = char_id;
	WFIFOL(fd, 6) = achievement_id;
	WFIFOB(fd, 10) = success;
	WFIFOSET(fd, 11);
}

/**
 * Request of the map-server that a player claimed his achievement rewards.
 * @see inter_parse_frommap
 */
int mapif_parse_achievement_reward(int fd){
	uint32 char_id = RFIFOL(fd, 2);
	int32 achievement_id = RFIFOL(fd, 6);
	bool success;

	if( Sql_Query( sql_handle, "UPDATE `%s` SET `gotReward` = '1' WHERE `char_id`='%u' AND `id` = '%d' AND `completeDate` <> '0' AND `gotReward` = '0'", schema_config.achievement_table, char_id, achievement_id ) == SQL_ERROR ||
		Sql_NumRowsAffected(sql_handle) <= 0 ){
		success = false;
	}else{
		char mail_sender[NAME_LENGTH];
		char mail_receiver[NAME_LENGTH];
		char mail_title[MAIL_TITLE_LENGTH];
		char mail_text[MAIL_BODY_LENGTH];
		struct item item;

		success = true;

		memset(&item, 0, sizeof(struct item));
		item.nameid = RFIFOW(fd, 10);
		item.amount = RFIFOL(fd, 12);
		item.identify = 1;

		safesnprintf(mail_sender, NAME_LENGTH, char_msg_txt(227)); // 227: GM
		safestrncpy(mail_receiver, RFIFOCP(fd,16), NAME_LENGTH);
		safesnprintf(mail_title, MAIL_TITLE_LENGTH, char_msg_txt(228)); // 228: Achievement Reward Mail
		safesnprintf(mail_text, MAIL_BODY_LENGTH, char_msg_txt(229), RFIFOCP(fd,16+NAME_LENGTH) ); // 229: [%s] Achievement Reward.

		success = mail_sendmail(0, mail_sender, char_id, mail_receiver, mail_title, mail_text, 0, &item, 1);
	}

	mapif_achievement_reward(fd, char_id, achievement_id, success);

	return 0;
}

/**
 * Parses achievementlog related packets from the map server.
 * @see inter_parse_frommap
 */
int inter_achievement_parse_frommap(int fd)
{
	switch (RFIFOW(fd, 0)) {
		case 0x3062: mapif_parse_achievement_load(fd); break;
		case 0x3063: mapif_parse_achievement_save(fd); break;
		case 0x3064: mapif_parse_achievement_reward(fd); break;
		default:
			return 0;
	}
	return 1;
}
