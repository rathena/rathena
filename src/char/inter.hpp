// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INTER_HPP
#define INTER_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include <common/cbasetypes.hpp>
#include <common/database.hpp>
#include <common/sql.hpp>

struct s_storage_table;

class InterServerDatabase : public TypesafeYamlDatabase<uint32, s_storage_table>{
public:
	InterServerDatabase() : TypesafeYamlDatabase( "INTER_SERVER_DB", 1 ){

	}

	const std::string getDefaultLocation() override;
	uint64 parseBodyNode( const ryml::NodeRef& node ) override;
};

extern InterServerDatabase interServerDb;

int32 inter_init_sql(const char *file);
void inter_final(void);
int32 inter_parse_frommap(int32 fd);
int32 inter_mapif_init(int32 fd);
int32 mapif_disconnectplayer(int32 fd, uint32 account_id, uint32 char_id, int32 reason);
void mapif_accinfo_ack( bool success, int32 map_fd, int32 u_fd, int32 u_aid, int32 account_id, int32 group_id, int32 logincount, int32 state, const char* email, const char* last_ip, const char* lastlogin, const char* birthdate, const char* userid );

int32 inter_log(const char *fmt,...);

extern uint32 party_share_level;

extern Sql* sql_handle;
extern Sql* lsql_handle;

int32 inter_accreg_fromsql(uint32 account_id, uint32 char_id, int32 fd, int32 type);

#endif /* INTER_HPP */
