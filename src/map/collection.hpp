// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef COLLECTION_HPP
#define COLLECTION_HPP

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"
#include "../common/mmo.hpp"
#include "../common/timer.hpp"

#include "battle.hpp"
#include "mob.hpp"
#include "pc.hpp"
#include "script.hpp"

#include <unordered_map>

/// Collection DB
struct s_collection_db {
	t_itemid ConsumeID; ///< Consume ID
	std::vector<uint16> MobID;
	uint16 CaptureRate; ///< Capture success rate 10000 = 100%
	uint16 GroupID;
};

enum e_collection_itemtype : uint8 { COLLECTION_CATCH };

enum e_collection_catch : uint16 {
	COLLECTION_CATCH_FAIL = 0, ///< A catch attempt failed
	COLLECTION_CATCH_UNIVERSAL = 1, ///< The catch attempt is universal (ignoring MD_STATUS_IMMUNE/Boss)
	COLLECTION_CATCH_UNIVERSAL_ITEM = 2,
};

class CollectionDatabase : public TypesafeYamlDatabase<t_itemid,s_collection_db>{
public:
	CollectionDatabase() : TypesafeYamlDatabase( "COLLECTION_DB", 1 ){

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode( const YAML::Node& node );
	bool reload();
};


int collection_catch_process1(struct map_session_data *sd, t_itemid item_id);
int collection_catch_process2(struct map_session_data *sd, uint32 target_id);

void do_init_collection(void);
void do_final_collection(void);

#endif /* COLLECTION_HPP */
