// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_quest.hpp"

#include <cstdlib>
#include <cstring>

#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>

#include "char.hpp"
#include "inter.hpp"

/**
 * Loads the entire questlog for a character.
 *
 * @param char_id Character ID
 * @param count   Pointer to return the number of found entries.
 * @return Array of found entries. It has *count entries, and it is care of the
 *         caller to aFree() it afterwards.
 */
struct quest *mapif_quests_fromsql( uint32 char_id, size_t& count ){
	struct quest *questlog = nullptr;
	struct quest tmp_quest;
	SqlStmt stmt{ *sql_handle };

	memset(&tmp_quest, 0, sizeof(struct quest));

	if( SQL_ERROR == stmt.Prepare("SELECT `quest_id`, `state`, `time`, `count1`, `count2`, `count3` FROM `%s` WHERE `char_id`=? ", schema_config.quest_db)
	||	SQL_ERROR == stmt.BindParam(0, SQLDT_INT32, &char_id, 0)
	||	SQL_ERROR == stmt.Execute()
	||	SQL_ERROR == stmt.BindColumn(0, SQLDT_INT32,  &tmp_quest.quest_id, 0, nullptr, nullptr)
	||	SQL_ERROR == stmt.BindColumn(1, SQLDT_INT32,  &tmp_quest.state,    0, nullptr, nullptr)
	||	SQL_ERROR == stmt.BindColumn(2, SQLDT_UINT32, &tmp_quest.time,     0, nullptr, nullptr)
	||	SQL_ERROR == stmt.BindColumn(3, SQLDT_INT32,  &tmp_quest.count[0], 0, nullptr, nullptr)
	||	SQL_ERROR == stmt.BindColumn(4, SQLDT_INT32,  &tmp_quest.count[1], 0, nullptr, nullptr)
	||	SQL_ERROR == stmt.BindColumn(5, SQLDT_INT32,  &tmp_quest.count[2], 0, nullptr, nullptr)
	) {
		SqlStmt_ShowDebug(stmt);
		count = 0;
		return nullptr;
	}

	count = static_cast<std::remove_reference<decltype(count)>::type>( stmt.NumRows() );
	if( count > 0 ) {
		size_t i = 0;

		questlog = (struct quest *)aCalloc( count, sizeof( struct quest ) );
		while( SQL_SUCCESS == stmt.NextRow() ) {
			// Sanity check, should never happen
			if( i >= count ){
				break;
			}

			memcpy(&questlog[i++], &tmp_quest, sizeof(tmp_quest));
		}

		if( i < count ) {
			//Should never happen. Compact array
			count = i;
			questlog = (struct quest *)aRealloc(questlog, sizeof(struct quest) * i);
		}
	}

	return questlog;
}

/**
 * Deletes a quest from a character's questlog.
 *
 * @param char_id  Character ID
 * @param quest_id Quest ID
 * @return false in case of errors, true otherwise
 */
bool mapif_quest_delete(uint32 char_id, int32 quest_id) {
	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `quest_id` = '%d' AND `char_id` = '%d'", schema_config.quest_db, quest_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Adds a quest to a character's questlog.
 *
 * @param char_id Character ID
 * @param qd      Quest data
 * @return false in case of errors, true otherwise
 */
bool mapif_quest_add(uint32 char_id, struct quest qd) {
	if( SQL_ERROR == Sql_Query(sql_handle, "INSERT INTO `%s`(`quest_id`, `char_id`, `state`, `time`, `count1`, `count2`, `count3`) VALUES ('%d', '%d', '%d', '%u', '%d', '%d', '%d')", schema_config.quest_db, qd.quest_id, char_id, qd.state, qd.time, qd.count[0], qd.count[1], qd.count[2]) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Updates a quest in a character's questlog.
 *
 * @param char_id Character ID
 * @param qd      Quest data
 * @return false in case of errors, true otherwise
 */
bool mapif_quest_update(uint32 char_id, struct quest qd) {
	if( SQL_ERROR == Sql_Query(sql_handle, "UPDATE `%s` SET `state`='%d', `count1`='%d', `count2`='%d', `count3`='%d' WHERE `quest_id` = '%d' AND `char_id` = '%d'", schema_config.quest_db, qd.state, qd.count[0], qd.count[1], qd.count[2], qd.quest_id, char_id) ) 
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

/**
 * Handles the save request from mapserver for a character's questlog.
 *
 * Received quests are saved, and an ack is sent back to the map server.
 *
 * @see inter_parse_frommap
 */
int32 mapif_parse_quest_save(int32 fd) {
	uint32 char_id = RFIFOL(fd,4);
	struct quest *old_qd = nullptr, *new_qd = nullptr;
	bool success = true;
	size_t old_n, new_n = ( RFIFOW( fd, 2 ) - 8 ) / sizeof( struct quest );

	if( new_n > 0 )
		new_qd = (struct quest*)RFIFOP(fd,8);

	old_qd = mapif_quests_fromsql( char_id, old_n );

	for( size_t i = 0, j; i < new_n; i++ ) {
		ARR_FIND(0, old_n, j, new_qd[i].quest_id == old_qd[j].quest_id);
		if( j < old_n ) { //Update existing quests
			size_t k;

			//Only states and counts are changable.
			ARR_FIND(0, MAX_QUEST_OBJECTIVES, k, new_qd[i].count[k] != old_qd[j].count[k]);
			if( k != MAX_QUEST_OBJECTIVES || new_qd[i].state != old_qd[j].state )
				success &= mapif_quest_update(char_id, new_qd[i]);

			if( j < (--old_n) ) {
				//Compact array
				memmove(&old_qd[j], &old_qd[j + 1], sizeof(struct quest) * (old_n - j));
				memset(&old_qd[old_n], 0, sizeof(struct quest));
			}
		} else //Add new quests
			success &= mapif_quest_add(char_id, new_qd[i]);
	}

	// Quests not in new_qd but in old_qd are to be erased.
	for( size_t i = 0; i < old_n; i++ ){
		success &= mapif_quest_delete(char_id, old_qd[i].quest_id);
	}

	if( old_qd )
		aFree(old_qd);

	//Send ack
	WFIFOHEAD(fd,7);
	WFIFOW(fd,0) = 0x3861;
	WFIFOL(fd,2) = char_id;
	WFIFOB(fd,6) = success ? 1 : 0;
	WFIFOSET(fd,7);

	return 0;
}

/**
 * Sends questlog to the map server
 *
 * NOTE: Completed quests (state == Q_COMPLETE) are guaranteed to be sent last
 * and the map server relies on this behavior (once the first Q_COMPLETE quest,
 * all of them are considered to be Q_COMPLETE)
 *
 * @see inter_parse_frommap
 */
int32 mapif_parse_quest_load(int32 fd) {
	uint32 char_id = RFIFOL(fd,2);
	size_t num_quests;
	struct quest* tmp_questlog = mapif_quests_fromsql( char_id, num_quests );

	WFIFOHEAD(fd,num_quests * sizeof(struct quest) + 8);
	WFIFOW(fd,0) = 0x3860;
	WFIFOW(fd,2) = static_cast<int16>( num_quests * sizeof( struct quest ) + 8 );
	WFIFOL(fd,4) = char_id;

	if( num_quests > 0 )
		memcpy(WFIFOP(fd,8), tmp_questlog, sizeof(struct quest) * num_quests);

	WFIFOSET(fd,num_quests * sizeof(struct quest) + 8);

	if( tmp_questlog )
		aFree(tmp_questlog);

	return 0;
}

/**
 * Parses questlog related packets from the map server.
 *
 * @see inter_parse_frommap
 */
int32 inter_quest_parse_frommap(int32 fd) {
	switch(RFIFOW(fd,0)) {
		case 0x3060: mapif_parse_quest_load(fd); break;
		case 0x3061: mapif_parse_quest_save(fd); break;
		default:
			return 0;
	}
	return 1;
}
