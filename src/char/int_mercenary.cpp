// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "int_mercenary.hpp"

#include <cstdlib>
#include <cstring>

#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/sql.hpp>
#include <common/strlib.hpp>

#include "char.hpp"
#include "inter.hpp"

bool mercenary_owner_fromsql(uint32 char_id, struct mmo_charstatus *status)
{
	char* data;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith` FROM `%s` WHERE `char_id` = '%d'", schema_config.mercenary_owner_db, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, nullptr); status->mer_id = atoi(data);
	Sql_GetData(sql_handle,  1, &data, nullptr); status->arch_calls = atoi(data);
	Sql_GetData(sql_handle,  2, &data, nullptr); status->arch_faith = atoi(data);
	Sql_GetData(sql_handle,  3, &data, nullptr); status->spear_calls = atoi(data);
	Sql_GetData(sql_handle,  4, &data, nullptr); status->spear_faith = atoi(data);
	Sql_GetData(sql_handle,  5, &data, nullptr); status->sword_calls = atoi(data);
	Sql_GetData(sql_handle,  6, &data, nullptr); status->sword_faith = atoi(data);
	Sql_FreeResult(sql_handle);

	return true;
}

bool mercenary_owner_tosql(uint32 char_id, struct mmo_charstatus *status)
{
	if( SQL_ERROR == Sql_Query(sql_handle, "REPLACE INTO `%s` (`char_id`, `merc_id`, `arch_calls`, `arch_faith`, `spear_calls`, `spear_faith`, `sword_calls`, `sword_faith`) VALUES ('%d', '%d', '%d', '%d', '%d', '%d', '%d', '%d')",
		schema_config.mercenary_owner_db, char_id, status->mer_id, status->arch_calls, status->arch_faith, status->spear_calls, status->spear_faith, status->sword_calls, status->sword_faith) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

bool mercenary_owner_delete(uint32 char_id)
{
	if (SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `mer_id` IN ( SELECT `merc_id` FROM `%s` WHERE `char_id` = '%d' )", schema_config.skillcooldown_mercenary_db, schema_config.mercenary_owner_db, char_id))
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.mercenary_owner_db, char_id) )
		Sql_ShowDebug(sql_handle);

	if( SQL_ERROR == Sql_Query(sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d'", schema_config.mercenary_db, char_id) )
		Sql_ShowDebug(sql_handle);

	return true;
}

bool mapif_mercenary_save(struct s_mercenary* merc)
{
	if( merc->mercenary_id == 0 )
	{ // Create new DB entry
		if( SQL_ERROR == Sql_Query(sql_handle,
			"INSERT INTO `%s` (`char_id`,`class`,`hp`,`sp`,`kill_counter`,`life_time`) VALUES ('%d','%d','%u','%u','%u','%" PRtf "')",
			schema_config.mercenary_db, merc->char_id, merc->class_, merc->hp, merc->sp, merc->kill_count, merc->life_time) )
		{
			Sql_ShowDebug(sql_handle);
			return false;
		}
		else
			merc->mercenary_id = (int32)Sql_LastInsertId(sql_handle);
	}
	else if( SQL_ERROR == Sql_Query(sql_handle,
		"UPDATE `%s` SET `char_id` = '%d', `class` = '%d', `hp` = '%u', `sp` = '%u', `kill_counter` = '%u', `life_time` = '%" PRtf "' WHERE `mer_id` = '%d'",
		schema_config.mercenary_db, merc->char_id, merc->class_, merc->hp, merc->sp, merc->kill_count, merc->life_time, merc->mercenary_id) )
	{ // Update DB entry
		Sql_ShowDebug(sql_handle);
		return false;
	}

	// Save skill cooldowns
	SqlStmt stmt{ *sql_handle };

	if( SQL_ERROR == Sql_Query( sql_handle, "DELETE FROM `%s` WHERE `mer_id` = '%d'", schema_config.skillcooldown_mercenary_db, merc->mercenary_id )
	 || SQL_ERROR == stmt.Prepare( "INSERT INTO `%s` (`mer_id`, `skill`, `tick`) VALUES (%d, ?, ?)", schema_config.skillcooldown_mercenary_db, merc->mercenary_id ) ){
		SqlStmt_ShowDebug(stmt);
		return false;
	}

	for (uint16 i = 0; i < MAX_SKILLCOOLDOWN; ++i) {
		if (merc->scd[i].skill_id == 0) {
			continue;
		}

		if (merc->scd[i].tick == 0) {
			continue;
		}

		if (SQL_ERROR == stmt.BindParam(0, SQLDT_UINT16, &merc->scd[i].skill_id, 0)
			|| SQL_ERROR == stmt.BindParam(1, SQLDT_LONGLONG, &merc->scd[i].tick, 0)
			|| SQL_ERROR == stmt.Execute()) {
			SqlStmt_ShowDebug(stmt);
			return false;
		}
	}

	return true;
}

bool mapif_mercenary_load(int32 merc_id, uint32 char_id, struct s_mercenary *merc)
{
	char* data;

	memset(merc, 0, sizeof(struct s_mercenary));
	merc->mercenary_id = merc_id;
	merc->char_id = char_id;

	if( SQL_ERROR == Sql_Query(sql_handle, "SELECT `class`, `hp`, `sp`, `kill_counter`, `life_time` FROM `%s` WHERE `mer_id` = '%d' AND `char_id` = '%d'", schema_config.mercenary_db, merc_id, char_id) )
	{
		Sql_ShowDebug(sql_handle);
		return false;
	}

	if( SQL_SUCCESS != Sql_NextRow(sql_handle) )
	{
		Sql_FreeResult(sql_handle);
		return false;
	}

	Sql_GetData(sql_handle,  0, &data, nullptr); merc->class_ = atoi(data);
	Sql_GetData(sql_handle,  1, &data, nullptr); merc->hp = atoi(data);
	Sql_GetData(sql_handle,  2, &data, nullptr); merc->sp = atoi(data);
	Sql_GetData(sql_handle,  3, &data, nullptr); merc->kill_count = atoi(data);
	Sql_GetData(sql_handle,  4, &data, nullptr); merc->life_time = strtoll( data, nullptr, 10 );
	Sql_FreeResult(sql_handle);
	if( charserv_config.save_log )
		ShowInfo("Mercenary loaded (ID: %d / Class: %d / CID: %d).\n", merc->mercenary_id, merc->class_, merc->char_id);

	// Load Mercenary Skill Cooldown
	if (SQL_ERROR == Sql_Query(sql_handle, "SELECT `skill`,`tick` FROM `%s` WHERE `mer_id`=%d", schema_config.skillcooldown_mercenary_db, merc_id)) {
		Sql_ShowDebug(sql_handle);
		return false;
	}

	uint16 count = 0;

	while (SQL_SUCCESS == Sql_NextRow(sql_handle)) {
		if (count == MAX_SKILLCOOLDOWN) {
			ShowWarning("Too many skillcooldowns for mercenary %d, skipping.\n", merc_id);
			break;
		}

		// Skill
		Sql_GetData(sql_handle, 0, &data, nullptr);
		uint16 skill_id = static_cast<uint16>(strtoul(data, nullptr, 10));

		if (skill_id < MC_SKILLBASE || skill_id >= MC_SKILLBASE + MAX_MERCSKILL)
			continue; // invalid skill ID
		merc->scd[count].skill_id = skill_id;

		// Tick
		Sql_GetData(sql_handle, 1, &data, nullptr);
		merc->scd[count].tick = strtoll(data, nullptr, 10);

		count++;
	}
	Sql_FreeResult(sql_handle);

	return true;
}

bool mapif_mercenary_delete(int32 merc_id)
{
	if( SQL_ERROR == Sql_Query( sql_handle, "DELETE FROM `%s` WHERE `mer_id` = '%d'", schema_config.skillcooldown_mercenary_db, merc_id )
	 || SQL_ERROR == Sql_Query( sql_handle, "DELETE FROM `%s` WHERE `mer_id` = '%d'", schema_config.mercenary_db, merc_id ) ){
		Sql_ShowDebug(sql_handle);
		return false;
	}

	return true;
}

void mapif_mercenary_send(int32 fd, struct s_mercenary *merc, unsigned char flag)
{
	int32 size = sizeof(struct s_mercenary) + 5;

	WFIFOHEAD(fd,size);
	WFIFOW(fd,0) = 0x3870;
	WFIFOW(fd,2) = size;
	WFIFOB(fd,4) = flag;
	memcpy(WFIFOP(fd,5),merc,sizeof(struct s_mercenary));
	WFIFOSET(fd,size);
}

void mapif_parse_mercenary_create(int32 fd, struct s_mercenary* merc)
{
	bool result = mapif_mercenary_save(merc);
	mapif_mercenary_send(fd, merc, result);
}

void mapif_parse_mercenary_load(int32 fd, int32 merc_id, uint32 char_id)
{
	struct s_mercenary merc;
	bool result = mapif_mercenary_load(merc_id, char_id, &merc);
	mapif_mercenary_send(fd, &merc, result);
}

void mapif_mercenary_deleted(int32 fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3871;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void mapif_parse_mercenary_delete(int32 fd, int32 merc_id)
{
	bool result = mapif_mercenary_delete(merc_id);
	mapif_mercenary_deleted(fd, result);
}

void mapif_mercenary_saved(int32 fd, unsigned char flag)
{
	WFIFOHEAD(fd,3);
	WFIFOW(fd,0) = 0x3872;
	WFIFOB(fd,2) = flag;
	WFIFOSET(fd,3);
}

void mapif_parse_mercenary_save(int32 fd, struct s_mercenary* merc)
{
	bool result = mapif_mercenary_save(merc);
	mapif_mercenary_saved(fd, result);
}

int32 inter_mercenary_sql_init(void)
{
	return 0;
}
void inter_mercenary_sql_final(void)
{
	return;
}

/*==========================================
 * Inter Packets
 *------------------------------------------*/
int32 inter_mercenary_parse_frommap(int32 fd)
{
	uint16 cmd = RFIFOW(fd,0);

	switch( cmd )
	{
		case 0x3070: mapif_parse_mercenary_create(fd, (struct s_mercenary*)RFIFOP(fd,4)); break;
		case 0x3071: mapif_parse_mercenary_load(fd, (int32)RFIFOL(fd,2), (int32)RFIFOL(fd,6)); break;
		case 0x3072: mapif_parse_mercenary_delete(fd, (int32)RFIFOL(fd,2)); break;
		case 0x3073: mapif_parse_mercenary_save(fd, (struct s_mercenary*)RFIFOP(fd,4)); break;
		default:
			return 0;
	}
	return 1;
}
