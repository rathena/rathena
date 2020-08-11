// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_achievement.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/db.hpp"
#include "../common/malloc.hpp"
#include "../common/mmo.hpp"
#include "../common/showmsg.hpp"
#include "../common/socket.hpp"
#include "../common/sql.hpp"
#include "../common/strlib.hpp"

#include "char.hpp"
#include "inter.hpp"
#include "int_mail.hpp"

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
	StringBuf_AppendStr(&buf, "SELECT `id`, COALESCE(UNIX_TIMESTAMP(`completed`),0), COALESCE(UNIX_TIMESTAMP(`rewarded`),0)");
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
	SqlStmt_BindColumn(stmt, 1, SQLDT_INT,  &tmp_achieve.completed, 0, NULL, NULL);
	SqlStmt_BindColumn(stmt, 2, SQLDT_INT,  &tmp_achieve.rewarded, 0, NULL, NULL);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		SqlStmt_BindColumn(stmt, 3 + i, SQLDT_INT, &tmp_achieve.count[i], 0, NULL, NULL);

	*count = (int)SqlStmt_NumRows(stmt);
	if (*count > 0) {
		i = 0;

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
	StringBuf_Destroy(&buf);

	ShowInfo("achievement load complete from DB - id: %d (total: %d)\n", char_id, *count);

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
bool mapif_achievement_add(uint32 char_id, struct achievement* ad)
{
	StringBuf buf;
	int i;

	ARR_FIND( 0, MAX_ACHIEVEMENT_OBJECTIVES, i, ad->count[i] != 0 );

	if( i == MAX_ACHIEVEMENT_OBJECTIVES && ad->completed == 0 && ad->rewarded == 0 ){
		// Do not save
		return true;
	}

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "INSERT INTO `%s` (`char_id`, `id`, `completed`, `rewarded`", schema_config.achievement_table);
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", `count%d`", i + 1);
	StringBuf_AppendStr(&buf, ")");
	StringBuf_Printf(&buf, " VALUES ('%u', '%d',", char_id, ad->achievement_id, (uint32)ad->completed, (uint32)ad->rewarded);
	if( ad->completed ){
		StringBuf_Printf(&buf, "FROM_UNIXTIME('%u'),", (uint32)ad->completed);
	}else{
		StringBuf_AppendStr(&buf, "NULL,");
	}
	if( ad->rewarded ){
		StringBuf_Printf(&buf, "FROM_UNIXTIME('%u')", (uint32)ad->rewarded);
	}else{
		StringBuf_AppendStr(&buf, "NULL");
	}
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", '%d'", ad->count[i]);
	StringBuf_AppendStr(&buf, ")");

	if (SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf))) {
		Sql_ShowDebug(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	StringBuf_Destroy(&buf);

	return true;
}

/**
 * Updates an achievement in a character's achievementlog.
 * @param char_id: Character ID
 * @param ad: Achievement data
 * @return false in case of errors, true otherwise
 */
bool mapif_achievement_update(uint32 char_id, struct achievement* ad)
{
	StringBuf buf;
	int i;

	StringBuf_Init(&buf);
	StringBuf_Printf(&buf, "UPDATE `%s` SET ", schema_config.achievement_table);
	if( ad->completed ){
		StringBuf_Printf(&buf, "`completed` = FROM_UNIXTIME('%u'),", (uint32)ad->completed);
	}else{
		StringBuf_AppendStr(&buf, "`completed` = NULL,");
	}
	if( ad->rewarded ){
		StringBuf_Printf(&buf, "`rewarded` = FROM_UNIXTIME('%u')", (uint32)ad->rewarded);
	}else{
		StringBuf_AppendStr(&buf, "`rewarded` = NULL");
	}
	for (i = 0; i < MAX_ACHIEVEMENT_OBJECTIVES; ++i)
		StringBuf_Printf(&buf, ", `count%d` = '%d'", i + 1, ad->count[i]);
	StringBuf_Printf(&buf, " WHERE `id` = %d AND `char_id` = %u", ad->achievement_id, char_id);

	if (SQL_ERROR == Sql_QueryStr(sql_handle, StringBuf_Value(&buf))) {
		Sql_ShowDebug(sql_handle);
		StringBuf_Destroy(&buf);
		return false;
	}

	StringBuf_Destroy(&buf);

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
			if (k != MAX_ACHIEVEMENT_OBJECTIVES || new_ad[i].completed != old_ad[j].completed || new_ad[i].rewarded != old_ad[j].rewarded) {
				if ((success = mapif_achievement_update(char_id, &new_ad[i])) == false)
					break;
			}

			if (j < (--old_n)) {
				// Compact array
				memmove(&old_ad[j], &old_ad[j + 1], sizeof(struct achievement) * (old_n - j));
				memset(&old_ad[old_n], 0, sizeof(struct achievement));
			}
		} else { // Add new achievements
			if (new_ad[i].achievement_id) {
				if ((success = mapif_achievement_add(char_id, &new_ad[i])) == false)
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
void mapif_achievement_reward( int fd, uint32 char_id, int32 achievement_id, time_t rewarded ){
	WFIFOHEAD(fd, 14);
	WFIFOW(fd, 0) = 0x3864;
	WFIFOL(fd, 2) = char_id;
	WFIFOL(fd, 6) = achievement_id;
	WFIFOL(fd, 10) = (uint32)rewarded;
	WFIFOSET(fd, 14);
}

/**
 * Request of the map-server that a player claimed his achievement rewards.
 * @see inter_parse_frommap
 */
int mapif_parse_achievement_reward(int fd){
	time_t current = time(NULL);
	uint32 char_id = RFIFOL(fd, 2);
	int32 achievement_id = RFIFOL(fd, 6);

	if( Sql_Query( sql_handle, "UPDATE `%s` SET `rewarded` = FROM_UNIXTIME('%u') WHERE `char_id`='%u' AND `id` = '%d' AND `completed` IS NOT NULL AND `rewarded` IS NULL", schema_config.achievement_table, (uint32)current, char_id, achievement_id ) == SQL_ERROR ||
		Sql_NumRowsAffected(sql_handle) <= 0 ){
		current = 0;
	}else if( RFIFOW(fd,10) > 0 ){ // Do not send a mail if no item reward
		char mail_sender[NAME_LENGTH];
		char mail_receiver[NAME_LENGTH];
		char mail_title[MAIL_TITLE_LENGTH];
		char mail_text[MAIL_BODY_LENGTH];
		struct item item;

		memset(&item, 0, sizeof(struct item));
		item.nameid = RFIFOL(fd, 10);
		item.amount = RFIFOW(fd, 14);
		item.identify = 1;

		safesnprintf(mail_sender, NAME_LENGTH, char_msg_txt(227)); // 227: GM
		safestrncpy(mail_receiver, RFIFOCP(fd,16), NAME_LENGTH);
		safesnprintf(mail_title, MAIL_TITLE_LENGTH, char_msg_txt(228)); // 228: Achievement Reward Mail
		safesnprintf(mail_text, MAIL_BODY_LENGTH, char_msg_txt(229), RFIFOCP(fd,16+NAME_LENGTH) ); // 229: [%s] Achievement Reward.

		if( !mail_sendmail(0, mail_sender, char_id, mail_receiver, mail_title, mail_text, 0, &item, 1) ){
			current = 0;
		}
	}

	mapif_achievement_reward(fd, char_id, achievement_id, current);

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
