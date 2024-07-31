// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "battleground.hpp"

#include <unordered_map>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/nullpo.hpp>
#include <common/random.hpp>
#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/timer.hpp>
#include <common/utilities.hpp>

#include "battle.hpp"
#include "clif.hpp"
#include "guild.hpp"
#include "homunculus.hpp"
#include "mapreg.hpp"
#include "mercenary.hpp"
#include "mob.hpp"
#include "npc.hpp"
#include "party.hpp"
#include "pc.hpp"
#include "pet.hpp"

using namespace rathena;

BattlegroundDatabase battleground_db;
std::unordered_map<int, std::shared_ptr<s_battleground_data>> bg_team_db;
std::vector<std::shared_ptr<s_battleground_queue>> bg_queues;
int bg_queue_count = 1;

const std::string BattlegroundDatabase::getDefaultLocation() {
	return std::string(db_path) + "/battleground_db.yml";
}

/**
 * Reads and parses an entry from the battleground_db
 * @param node: The YAML node containing the entry
 * @return count of successfully parsed rows
 */
uint64 BattlegroundDatabase::parseBodyNode(const ryml::NodeRef& node) {
	uint32 id;

	if (!this->asUInt32(node, "Id", id))
		return 0;

	std::shared_ptr<s_battleground_type> bg = this->find(id);
	bool exists = bg != nullptr;

	if (!exists) {
		if (!this->nodesExist(node, { "Name", "Locations" }))
			return 0;

		bg = std::make_shared<s_battleground_type>();
		bg->id = id;
	}

	if (this->nodeExists(node, "Name")) {
		std::string name;

		if (!this->asString(node, "Name", name))
			return 0;

		name.resize(NAME_LENGTH);
		bg->name = name;
	}

	if (this->nodeExists(node, "MinPlayers")) {
		int min;

		if (!this->asInt32(node, "MinPlayers", min))
			return 0;

		if (min < 1) {
			this->invalidWarning(node["MinPlayers"], "Minimum players %d cannot be less than 1, capping to 1.\n", min);
			min = 1;
		}

		if (min * 2 > MAX_BG_MEMBERS) {
			this->invalidWarning(node["MinPlayers"], "Minimum players %d exceeds MAX_BG_MEMBERS, capping to %d.\n", min, MAX_BG_MEMBERS / 2);
			min = MAX_BG_MEMBERS / 2;
		}

		bg->required_players = min;
	} else {
		if (!exists)
			bg->required_players = 1;
	}

	if (this->nodeExists(node, "MaxPlayers")) {
		int max;

		if (!this->asInt32(node, "MaxPlayers", max))
			return 0;

		if (max < 1) {
			this->invalidWarning(node["MaxPlayers"], "Maximum players %d cannot be less than 1, capping to 1.\n", max);
			max = 1;
		}

		if (max * 2 > MAX_BG_MEMBERS) {
			this->invalidWarning(node["MaxPlayers"], "Maximum players %d exceeds MAX_BG_MEMBERS, capping to %d.\n", max, MAX_BG_MEMBERS / 2);
			max = MAX_BG_MEMBERS / 2;
		}

		bg->max_players = max;
	} else {
		if (!exists)
			bg->max_players = MAX_BG_MEMBERS / 2;
	}

	if (this->nodeExists(node, "MinLevel")) {
		int min;

		if (!this->asInt32(node, "MinLevel", min))
			return 0;

		if (min > MAX_LEVEL) {
			this->invalidWarning(node["MinLevel"], "Minimum level %d exceeds MAX_LEVEL, capping to %d.\n", min, MAX_LEVEL);
			min = MAX_LEVEL;
		}

		bg->min_lvl = min;
	} else {
		if (!exists)
			bg->min_lvl = 1;
	}

	if (this->nodeExists(node, "MaxLevel")) {
		int max;

		if (!this->asInt32(node, "MaxLevel", max))
			return 0;

		if (max > MAX_LEVEL) {
			this->invalidWarning(node["MaxLevel"], "Maximum level %d exceeds MAX_LEVEL, capping to %d.\n", max, MAX_LEVEL);
			max = MAX_LEVEL;
		}

		bg->max_lvl = max;
	} else {
		if (!exists)
			bg->max_lvl = MAX_LEVEL;
	}

	if (this->nodeExists(node, "Deserter")) {
		uint32 deserter;

		if (!this->asUInt32(node, "Deserter", deserter))
			return 0;

		bg->deserter_time = deserter;
	} else {
		if (!exists)
			bg->deserter_time = 600;
	}

	if (this->nodeExists(node, "StartDelay")) {
		uint32 delay;

		if (!this->asUInt32(node, "StartDelay", delay))
			return 0;

		bg->start_delay = delay;
	} else {
		if (!exists)
			bg->start_delay = 0;
	}

	if (this->nodeExists(node, "Join")) {
		const auto& joinNode = node["Join"];

		if (this->nodeExists(joinNode, "Solo")) {
			bool active;

			if (!this->asBool(joinNode, "Solo", active))
				return 0;

			bg->solo = active;
		} else {
			if (!exists)
				bg->solo = true;
		}

		if (this->nodeExists(joinNode, "Party")) {
			bool active;

			if (!this->asBool(joinNode, "Party", active))
				return 0;

			bg->party = active;
		} else {
			if (!exists)
				bg->party = true;
		}

		if (this->nodeExists(joinNode, "Guild")) {
			bool active;

			if (!this->asBool(joinNode, "Guild", active))
				return 0;

			bg->guild = active;
		} else {
			if (!exists)
				bg->guild = true;
		}
	} else {
		if (!exists) {
			bg->solo = true;
			bg->party = true;
			bg->guild = true;
		}
	}

	if (this->nodeExists(node, "JobRestrictions")) {
		const auto& jobsNode = node["JobRestrictions"];

		for (const auto& jobit : jobsNode) {
			std::string job_name;
			c4::from_chars(jobit.key(), &job_name);
			std::string job_name_constant = "JOB_" + job_name;
			int64 constant;

			if (!script_get_constant(job_name_constant.c_str(), &constant)) {
				this->invalidWarning(node["JobRestrictions"], "Job %s does not exist.\n", job_name.c_str());
				continue;
			}

			bool active;

			if (!this->asBool(jobsNode, job_name, active))
				return 0;

			if (active)
				bg->job_restrictions.push_back(static_cast<int32>(constant));
			else
				util::vector_erase_if_exists(bg->job_restrictions, static_cast<int32>(constant));
		}
	}

	if (this->nodeExists(node, "Locations")) {
		int count = 0;

		for (const auto& location : node["Locations"]) {
			s_battleground_map map_entry;

			if (this->nodeExists(location, "Map")) {
				std::string map_name;

				if (!this->asString(location, "Map", map_name))
					return 0;

				map_entry.mapindex = mapindex_name2id(map_name.c_str());

				if (map_entry.mapindex == 0) {
					this->invalidWarning(location["Map"], "Invalid battleground map name %s, skipping.\n", map_name.c_str());
					return 0;
				}

				if( map_mapindex2mapid( map_entry.mapindex ) < 0 ){
					// Ignore silently, the map is on another mapserver
					return 0;
				}
			}

			if (this->nodeExists(location, "StartEvent")) {
				if (!this->asString(location, "StartEvent", map_entry.bgcallscript))
					return 0;

				if (map_entry.bgcallscript.length() > EVENT_NAME_LENGTH) {
					this->invalidWarning(location["StartEvent"], "StartEvent \"%s\" exceeds maximum of %d characters, capping...\n", map_entry.bgcallscript.c_str(), EVENT_NAME_LENGTH - 1);
				}

				map_entry.bgcallscript.resize(EVENT_NAME_LENGTH);

				if (map_entry.bgcallscript.find("::On") == std::string::npos) {
					this->invalidWarning(location["StartEvent"], "Battleground StartEvent label %s should begin with '::On', skipping.\n", map_entry.bgcallscript.c_str());
					return 0;
				}
			}

			std::vector<std::string> team_list = { "TeamA", "TeamB" };

			for (const auto &it : team_list) {
				c4::csubstr team_name = c4::to_csubstr(it);
				const ryml::NodeRef& team = location;

				if (this->nodeExists(team, it)) {
					s_battleground_team *team_ptr;

					if (it.find("TeamA") != std::string::npos)
						team_ptr = &map_entry.team1;
					else if (it.find("TeamB") != std::string::npos)
						team_ptr = &map_entry.team2;
					else {
						this->invalidWarning(team[team_name], "An invalid Team is defined.\n");
						return 0;
					}

					if (this->nodeExists(team[team_name], "RespawnX")) {
						uint16 warp_x;

						if (!this->asUInt16(team[team_name], "RespawnX", warp_x))
							return 0;

						if (warp_x == 0) {
							this->invalidWarning(node["RespawnX"], "RespawnX has to be greater than zero.\n");
							return 0;
						}

						map_data *md = map_getmapdata(map_mapindex2mapid(map_entry.mapindex));

						if (warp_x >= md->xs) {
							this->invalidWarning(node["RespawnX"], "RespawnX has to be smaller than %hu.\n", md->xs);
							return 0;
						}

						team_ptr->warp_x = warp_x;
					}

					if (this->nodeExists(team[team_name], "RespawnY")) {
						uint16 warp_y;

						if (!this->asUInt16(team[team_name], "RespawnY", warp_y))
							return 0;

						if (warp_y == 0) {
							this->invalidWarning(node["RespawnY"], "RespawnY has to be greater than zero.\n");
							return 0;
						}

						map_data *md = map_getmapdata(map_mapindex2mapid(map_entry.mapindex));

						if (warp_y >= md->ys) {
							this->invalidWarning(node["RespawnY"], "RespawnY has to be smaller than %hu.\n", md->ys);
							return 0;
						}

						team_ptr->warp_y = warp_y;
					}

					if (this->nodeExists(team[team_name], "DeathEvent")) {
						if (!this->asString(team[team_name], "DeathEvent", team_ptr->death_event))
							return 0;

						team_ptr->death_event.resize(EVENT_NAME_LENGTH);

						if (team_ptr->death_event.find("::On") == std::string::npos) {
							this->invalidWarning(team["DeathEvent"], "Battleground DeathEvent label %s should begin with '::On', skipping.\n", team_ptr->death_event.c_str());
							return 0;
						}
					}

					if (this->nodeExists(team[team_name], "QuitEvent")) {
						if (!this->asString(team[team_name], "QuitEvent", team_ptr->quit_event))
							return 0;

						team_ptr->quit_event.resize(EVENT_NAME_LENGTH);

						if (team_ptr->quit_event.find("::On") == std::string::npos) {
							this->invalidWarning(team["QuitEvent"], "Battleground QuitEvent label %s should begin with '::On', skipping.\n", team_ptr->quit_event.c_str());
							return 0;
						}
					}

					if (this->nodeExists(team[team_name], "ActiveEvent")) {
						if (!this->asString(team[team_name], "ActiveEvent", team_ptr->active_event))
							return 0;

						team_ptr->active_event.resize(EVENT_NAME_LENGTH);

						if (team_ptr->active_event.find("::On") == std::string::npos) {
							this->invalidWarning(team["ActiveEvent"], "Battleground ActiveEvent label %s should begin with '::On', skipping.\n", team_ptr->active_event.c_str());
							return 0;
						}
					}

					if (this->nodeExists(team[team_name], "Variable")) {
						if (!this->asString(team[team_name], "Variable", team_ptr->bg_id_var))
							return 0;

						team_ptr->bg_id_var.resize(NAME_LENGTH);
					}

					map_entry.id = count++;
					map_entry.isReserved = false;
				}
			}

			bg->maps.push_back(map_entry);
		}
	}

	if (!exists)
		this->put(id, bg);

	return 1;
}

/**
 * Search for a battleground based on the given name
 * @param name: Battleground name
 * @return s_battleground_type on success or nullptr on failure
 */
std::shared_ptr<s_battleground_type> bg_search_name(const char *name)
{
	for (const auto &entry : battleground_db) {
		auto bg = entry.second;

		if (!stricmp(bg->name.c_str(), name))
			return bg;
	}

	return nullptr;
}

/**
 * Search for a Battleground queue based on the given queue ID
 * @param queue_id: Queue ID
 * @return s_battleground_queue on success or nullptr on failure
 */
std::shared_ptr<s_battleground_queue> bg_search_queue(int queue_id)
{
	for (const auto &queue : bg_queues) {
		if (queue_id == queue->queue_id)
			return queue;
	}

	return nullptr;
}

/**
 * Search for an available player in Battleground
 * @param bg: Battleground data
 * @return map_session_data
 */
map_session_data* bg_getavailablesd(s_battleground_data *bg)
{
	nullpo_retr(nullptr, bg);

	for (const auto &member : bg->members) {
		if (member.sd != nullptr)
			return member.sd;
	}

	return nullptr;
}

/**
 * Delete a Battleground team from the db
 * @param bg_id: Battleground ID
 * @return True on success or false otherwise
 */
bool bg_team_delete(int bg_id)
{
	std::shared_ptr<s_battleground_data> bgteam = util::umap_find(bg_team_db, bg_id);

	if (bgteam) {
		for (const auto &pl_sd : bgteam->members) {
			bg_send_dot_remove(pl_sd.sd);
			pl_sd.sd->bg_id = 0;
		}

		bg_team_db.erase(bg_id);

		return true;
	}
	
	return false;
}

/**
 * Warps a Battleground team
 * @param bg_id: Battleground ID
 * @param mapindex: Map Index
 * @param x: X coordinate
 * @param y: Y coordinate
 * @return True on success or false otherwise
 */
bool bg_team_warp(int bg_id, unsigned short mapindex, short x, short y)
{
	std::shared_ptr<s_battleground_data> bgteam = util::umap_find(bg_team_db, bg_id);

	if (bgteam) {
		for (const auto &pl_sd : bgteam->members)
			pc_setpos(pl_sd.sd, mapindex, x, y, CLR_TELEPORT);

		return true;
	}

	return false;
}

/**
 * Remove a player's Battleground map marker
 * @param sd: Player data
 */
void bg_send_dot_remove(map_session_data *sd)
{
	nullpo_retv(sd);

	if( sd && sd->bg_id )
		clif_bg_xy_remove(sd);
	return;
}

/**
 * Join a player to a Battleground team
 * @param bg_id: Battleground ID
 * @param sd: Player data
 * @param is_queue: Joined from queue
 * @return True on success or false otherwise
 */
bool bg_team_join(int bg_id, map_session_data *sd, bool is_queue)
{
	if (!sd || sd->bg_id)
		return false;

	std::shared_ptr<s_battleground_data> bgteam = util::umap_find(bg_team_db, bg_id);

	if (bgteam) {
		if (bgteam->members.size() == MAX_BG_MEMBERS)
			return false; // No free slots

		s_battleground_member_data member = {};

		sd->bg_id = bg_id;
		member.sd = sd;
		member.x = sd->bl.x;
		member.y = sd->bl.y;
		if (is_queue) { // Save the location from where the person entered the battleground
			member.entry_point.map = sd->mapindex;
			member.entry_point.x = sd->bl.x;
			member.entry_point.y = sd->bl.y;
		}
		bgteam->members.push_back(member);

		guild_send_dot_remove(sd);

		for (const auto &pl_sd : bgteam->members) {
			if (pl_sd.sd != sd)
				clif_hpmeter_single( *sd, pl_sd.sd->bl.id, pl_sd.sd->battle_status.hp, pl_sd.sd->battle_status.max_hp );
		}

		clif_bg_hp(sd);
		clif_bg_xy(sd);
		return true;
	}

	return false;
}

/**
 * Remove a player from Battleground team
 * @param sd: Player data
 * @param quit: True if closed client or false otherwise
 * @param deserter: Whether to apply the deserter status or not
 * @return Remaining count in Battleground team or -1 on failure
 */
int bg_team_leave(map_session_data *sd, bool quit, bool deserter)
{
	if (!sd || !sd->bg_id)
		return -1;

	bg_send_dot_remove(sd);

	int bg_id = sd->bg_id;
	std::shared_ptr<s_battleground_data> bgteam = util::umap_find(bg_team_db, bg_id);

	sd->bg_id = 0;

	if (bgteam) {
		// Warping members out only applies to the Battleground Queue System
		if (battle_config.feature_bgqueue) {
			auto member = bgteam->members.begin();

			while (member != bgteam->members.end()) {
				if (member->sd == sd) {
					if (member->entry_point.map != 0 && !map_getmapflag(map_mapindex2mapid(member->entry_point.map), MF_NOSAVE))
						pc_setpos(sd, member->entry_point.map, member->entry_point.x, member->entry_point.y, CLR_TELEPORT);
					else
						pc_setpos( sd, mapindex_name2id( sd->status.save_point.map ), sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT ); // Warp to save point if the entry map has no save flag.

					bgteam->members.erase(member);
					break;
				} else
					member++;
			}
		}

		char output[CHAT_SIZE_MAX];

		if (quit)
			sprintf(output, "Server: %s has quit the game...", sd->status.name);
		else
			sprintf(output, "Server: %s is leaving the battlefield...", sd->status.name);

		clif_bg_message(bgteam.get(), 0, "Server", output, strlen(output) + 1);

		if (!bgteam->logout_event.empty() && quit)
			npc_event(sd, bgteam->logout_event.c_str(), 0);

		if (deserter) {
			std::shared_ptr<s_battleground_type> bg = battleground_db.find(bg_id);

			if (bg)
				sc_start(nullptr, &sd->bl, SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT, 100, 1, static_cast<t_tick>(bg->deserter_time) * 1000); // Deserter timer
		}

		return static_cast<int>( bgteam->members.size() );
	}

	return -1;
}

/**
 * Respawn a Battleground player
 * @param sd: Player data
 * @return True on success or false otherwise
 */
bool bg_member_respawn(map_session_data *sd)
{
	if (!sd || !sd->bg_id || !pc_isdead(sd))
		return false;

	std::shared_ptr<s_battleground_data> bgteam = util::umap_find(bg_team_db, sd->bg_id);

	if (bgteam) {
		if (bgteam->cemetery.map == 0)
			return false; // Respawn not handled by Core

		pc_setpos(sd, bgteam->cemetery.map, bgteam->cemetery.x, bgteam->cemetery.y, CLR_OUTSIGHT);
		status_revive(&sd->bl, 1, 100);

		return true; // Warped
	}

	return false;
}

/**
 * Initialize Battleground data
 * @param mapindex: Map Index
 * @param rx: Return X coordinate (on death)
 * @param ry: Return Y coordinate (on death)
 * @param ev: Logout NPC Event
 * @param dev: Death NPC Event
 * @return Battleground ID
 */
int bg_create(uint16 mapindex, s_battleground_team* team)
{
	int bg_team_counter = 1;

	while (bg_team_db.find(bg_team_counter) != bg_team_db.end())
		bg_team_counter++;

	bg_team_db[bg_team_counter] = std::make_shared<s_battleground_data>();

	std::shared_ptr<s_battleground_data> bg = util::umap_find(bg_team_db, bg_team_counter);

	bg->id = bg_team_counter;
	bg->cemetery.map = mapindex;
	bg->cemetery.x = team->warp_x;
	bg->cemetery.y = team->warp_y;
	bg->logout_event = team->quit_event.c_str();
	bg->die_event = team->death_event.c_str();
	bg->active_event = team->active_event.c_str();

	return bg->id;
}

/**
 * Get an object's Battleground ID
 * @param bl: Object
 * @return Battleground ID
 */
int bg_team_get_id(struct block_list *bl)
{
	nullpo_ret(bl);

	switch( bl->type ) {
		case BL_PC:
			return ((TBL_PC*)bl)->bg_id;
		case BL_PET:
			if( ((TBL_PET*)bl)->master )
				return ((TBL_PET*)bl)->master->bg_id;
			break;
		case BL_MOB: {
			map_session_data *msd;
			struct mob_data *md = (TBL_MOB*)bl;

			if( md->special_state.ai && (msd = map_id2sd(md->master_id)) != nullptr )
				return msd->bg_id;

			return md->bg_id;
		}
		case BL_HOM:
			if( ((TBL_HOM*)bl)->master )
				return ((TBL_HOM*)bl)->master->bg_id;
			break;
		case BL_MER:
			if( ((TBL_MER*)bl)->master )
				return ((TBL_MER*)bl)->master->bg_id;
			break;
		case BL_SKILL:
			return ((TBL_SKILL*)bl)->group->bg_id;
	}

	return 0;
}

/**
 * Send a Battleground chat message
 * @param sd: Player data
 * @param mes: Message
 * @param len: Message length
 */
void bg_send_message(map_session_data *sd, const char *mes, size_t len)
{
	nullpo_retv(sd);

	if (sd->bg_id == 0)
		return;
	
	std::shared_ptr<s_battleground_data> bgteam = util::umap_find(bg_team_db, sd->bg_id);

	if (bgteam)
		clif_bg_message(bgteam.get(), sd->bl.id, sd->status.name, mes, len);

	return;
}

/**
 * Update a player's Battleground minimap icon
 * @see DBApply
 */
int bg_send_xy_timer_sub(std::shared_ptr<s_battleground_data> bg)
{
	map_session_data *sd;

	for (auto &pl_sd : bg->members) {
		sd = pl_sd.sd;

		if (sd->bl.x != pl_sd.x || sd->bl.y != pl_sd.y) { // xy update
			pl_sd.x = sd->bl.x;
			pl_sd.y = sd->bl.y;
			clif_bg_xy(sd);
		}
	}

	return 0;
}

/**
 * Update a player's Battleground minimap icon
 * @param tid: Timer ID
 * @param tick: Timer
 * @param id: ID
 * @return 0 on success or 1 otherwise
 */
TIMER_FUNC(bg_send_xy_timer)
{
	for (const auto &entry : bg_team_db)
		bg_send_xy_timer_sub(entry.second);

	return 0;
}

/**
 * Mark a Battleground as ready to begin queuing for a free map
 * @param tid: Timer ID
 * @param tick: Timer
 * @param id: ID
 * @return 0 on success or 1 otherwise
 */
static TIMER_FUNC(bg_on_ready_loopback)
{
	int queue_id = (int)data;
	std::shared_ptr<s_battleground_queue> queue = bg_search_queue(queue_id);

	if (queue == nullptr) {
		ShowError("bg_on_ready_loopback: Invalid battleground queue %d.\n", queue_id);
		return 1;
	}

	std::shared_ptr<s_battleground_type> bg = battleground_db.find(queue->id);

	if (bg) {
		bg_queue_on_ready(bg->name.c_str(), queue);
		return 0;
	} else {
		ShowError("bg_on_ready_loopback: Can't find battleground %d in the battlegrounds database.\n", queue->id);
		return 1;
	}
}

/**
 * Reset Battleground queue data if players don't accept in time
 * @param tid: Timer ID
 * @param tick: Timer
 * @param id: ID
 * @return 0 on success or 1 otherwise
 */
static TIMER_FUNC(bg_on_ready_expire)
{
	int queue_id = (int)data;
	std::shared_ptr<s_battleground_queue> queue = bg_search_queue(queue_id);

	if (queue == nullptr) {
		ShowError("bg_on_ready_expire: Invalid battleground queue %d.\n", queue_id);
		return 1;
	}

	std::string bg_name = battleground_db.find(queue->id)->name;

	for (const auto &sd : queue->teama_members) {
		clif_bg_queue_apply_result(BG_APPLY_QUEUE_FINISHED, bg_name.c_str(), sd);
		clif_bg_queue_entry_init(sd);
	}

	for (const auto &sd : queue->teamb_members) {
		clif_bg_queue_apply_result(BG_APPLY_QUEUE_FINISHED, bg_name.c_str(), sd);
		clif_bg_queue_entry_init(sd);
	}

	bg_queue_clear(queue, true);
	return 0;
}

/**
 * Start a Battleground when all players have accepted
 * @param tid: Timer ID
 * @param tick: Timer
 * @param id: ID
 * @return 0 on success or 1 otherwise
 */
static TIMER_FUNC(bg_on_ready_start)
{
	int queue_id = (int)data;
	std::shared_ptr<s_battleground_queue> queue = bg_search_queue(queue_id);

	if (queue == nullptr) {
		ShowError("bg_on_ready_start: Invalid battleground queue %d.\n", queue_id);
		return 1;
	}

	queue->tid_start = INVALID_TIMER;
	bg_queue_start_battleground(queue);

	return 0;
}

/**
 * Check if the given player is in a battleground
 * @param sd: Player data
 * @return True if in a battleground or false otherwise
 */
bool bg_player_is_in_bg_map(map_session_data *sd)
{
	nullpo_retr(false, sd);

	for (const auto &pair : battleground_db) {
		for (const auto &it : pair.second->maps) {
			if (it.mapindex == sd->mapindex)
				return true;
		}
	}

	return false;
}

/**
 * Battleground status change check
 * @param sd: Player data
 * @param name: Battleground name
 * @return True if the player is good to join a queue or false otherwise
 */
static bool bg_queue_check_status(map_session_data* sd, const char *name)
{
	nullpo_retr(false, sd);

	if (sd->sc.count) {
		if (sd->sc.getSCE(SC_ENTRY_QUEUE_APPLY_DELAY)) { // Exclude any player who's recently left a battleground queue
			char buf[CHAT_SIZE_MAX];

			sprintf(buf, msg_txt(sd, 339), static_cast<int32>((get_timer(sd->sc.getSCE(SC_ENTRY_QUEUE_APPLY_DELAY)->timer)->tick - gettick()) / 1000)); // You can't apply to a battleground queue for %d seconds due to recently leaving one.
			clif_bg_queue_apply_result(BG_APPLY_NONE, name, sd);
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], buf, false, SELF);
			return false;
		}

		if (sd->sc.getSCE(SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT)) { // Exclude any player who's recently deserted a battleground
			char buf[CHAT_SIZE_MAX];
			int32 status_tick = static_cast<int32>(DIFF_TICK(get_timer(sd->sc.getSCE(SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT)->timer)->tick, gettick()) / 1000);

			sprintf(buf, msg_txt(sd, 338), status_tick / 60, status_tick % 60); // You can't apply to a battleground queue due to recently deserting a battleground. Time remaining: %d minutes and %d seconds.
			clif_bg_queue_apply_result(BG_APPLY_NONE, name, sd);
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], buf, false, SELF);
			return false;
		}
	}

	return true;
}

/**
 * Check to see if a Battleground is joinable
 * @param bg: Battleground data
 * @param sd: Player data
 * @param name: Battleground name
 * @return True on success or false otherwise
 */
bool bg_queue_check_joinable(std::shared_ptr<s_battleground_type> bg, map_session_data *sd, const char *name)
{
	nullpo_retr(false, sd);

	for (const auto &job : bg->job_restrictions) { // Check class requirement
		if (sd->status.class_ == job) {
			clif_bg_queue_apply_result(BG_APPLY_PLAYER_CLASS, name, sd);
			return false;
		}
	}

	if (bg->min_lvl > 0 && sd->status.base_level < bg->min_lvl) { // Check minimum level requirement
		clif_bg_queue_apply_result(BG_APPLY_PLAYER_LEVEL, name, sd);
		return false;
	}

	if (bg->max_lvl > 0 && sd->status.base_level > bg->max_lvl) { // Check maximum level requirement
		clif_bg_queue_apply_result(BG_APPLY_PLAYER_LEVEL, name, sd);
		return false;
	}

	if (!bg_queue_check_status(sd, name)) // Check status blocks
		return false;

	if (bg_player_is_in_bg_map(sd)) { // Is the player currently in a battleground map? Reject them.
		clif_bg_queue_apply_result(BG_APPLY_NONE, name, sd);
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], msg_txt(sd, 337), false, SELF); // You can't apply to a battleground queue from this map.
		return false;
	}

	if (battle_config.bgqueue_nowarp_mapflag > 0 && map_getmapflag(sd->bl.m, MF_NOWARP)) { // Check player's current position for mapflag check
		clif_bg_queue_apply_result(BG_APPLY_NONE, name, sd);
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], msg_txt(sd, 337), false, SELF); // You can't apply to a battleground queue from this map.
		return false;
	}

	return true;
}

/**
 * Mark a map as reserved for a Battleground
 * @param name: Battleground map name
 * @param state: Whether to mark reserved or not
 * @param ended: Whether the Battleground event is complete; players getting prize
 * @return True on success or false otherwise
 */
bool bg_queue_reservation(const char *name, bool state, bool ended)
{
	uint16 mapindex = mapindex_name2id(name);

	for (auto &pair : battleground_db) {
		for (auto &map : pair.second->maps) {
			if (map.mapindex == mapindex) {
				map.isReserved = state;
				for (auto &queue : bg_queues) {
					if (queue->map == &map) {
						if (ended) // The ended flag is applied from bg_reserve (bg_unbook clears it for the next queue)
							queue->state = QUEUE_STATE_ENDED;
						if (!state)
							bg_queue_clear(queue, true);
					}
				}
				return true;
			}
		}
	}

	return false;
}

/**
 * Join as an individual into a Battleground
 * @param name: Battleground name
 * @param sd: Player who requested to join the battlegrounds
 */
void bg_queue_join_solo(const char *name, map_session_data *sd)
{
	if (!sd) {
		ShowError("bg_queue_join_solo: Tried to join non-existent player\n.");
		return;
	}

	std::shared_ptr<s_battleground_type> bg = bg_search_name(name);

	if (!bg) {
		ShowWarning("bq_queue_join_solo: Could not find battleground \"%s\" requested by %s (AID: %d / CID: %d)\n", name, sd->status.name, sd->status.account_id, sd->status.char_id);
		return;
	}

	if (!bg->solo) {
		clif_bg_queue_apply_result(BG_APPLY_INVALID_APP, name, sd);
		return;
	}

	bg_queue_join_multi(name, sd, { sd }); // Join as solo
}

/**
 * Join a party onto the same side of a Battleground
 * @param name: Battleground name
 * @param sd: Player who requested to join the battlegrounds
 */
void bg_queue_join_party(const char *name, map_session_data *sd)
{
	if (!sd) {
		ShowError("bg_queue_join_party: Tried to join non-existent player\n.");
		return;
	}

	struct party_data *p = party_search(sd->status.party_id);

	if (!p) {
		clif_bg_queue_apply_result(BG_APPLY_INVALID_APP, name, sd);
		return; // Someone has bypassed the client check for being in a party
	}

	for (const auto &it : p->party.member) {
		if (it.leader && sd->status.char_id != it.char_id) {
			clif_bg_queue_apply_result(BG_APPLY_PARTYGUILD_LEADER, name, sd);
			return; // Not the party leader
		}
	}

	std::shared_ptr<s_battleground_type> bg = bg_search_name(name);

	if (bg) {
		if (!bg->party) {
			clif_bg_queue_apply_result(BG_APPLY_INVALID_APP, name, sd);
			return;
		}

		int p_online = 0;

		for (const auto &it : p->party.member) {
			if (it.online)
				p_online++;
		}

		if (p_online > bg->max_players) {
			clif_bg_queue_apply_result(BG_APPLY_PLAYER_COUNT, name, sd);
			return; // Too many party members online
		}

		std::vector<map_session_data *> list;

		for (const auto &it : p->party.member) {
			if (list.size() == bg->max_players)
				break;

			if (it.online) {
				map_session_data *pl_sd = map_charid2sd(it.char_id);

				if (pl_sd)
					list.push_back(pl_sd);
			}
		}

		bg_queue_join_multi(name, sd, list); // Join as party, all on the same side of the BG
	} else {
		ShowWarning("clif_parse_bg_queue_apply_request: Could not find Battleground: \"%s\" requested by player: %s (AID:%d CID:%d)\n", name, sd->status.name, sd->status.account_id, sd->status.char_id);
		clif_bg_queue_apply_result(BG_APPLY_INVALID_NAME, name, sd);
		return; // Invalid BG name
	}
}

/**
 * Join a guild onto the same side of a Battleground
 * @param name: Battleground name
 * @param sd: Player who requested to join the battlegrounds
 */
void bg_queue_join_guild(const char *name, map_session_data *sd)
{
	if (!sd) {
		ShowError("bg_queue_join_guild: Tried to join non-existent player\n.");
		return;
	}

	if (!sd->guild) {
		clif_bg_queue_apply_result(BG_APPLY_INVALID_APP, name, sd);
		return; // Someone has bypassed the client check for being in a guild
	}
	
	if (strcmp(sd->status.name, sd->guild->guild.master) != 0) {
		clif_bg_queue_apply_result(BG_APPLY_PARTYGUILD_LEADER, name, sd);
		return; // Not the guild leader
	}

	std::shared_ptr<s_battleground_type> bg = bg_search_name(name);

	if (bg) {
		if (!bg->guild) {
			clif_bg_queue_apply_result(BG_APPLY_INVALID_APP, name, sd);
			return;
		}

		auto &g = sd->guild;

		if (g->guild.connect_member > bg->max_players) {
			clif_bg_queue_apply_result(BG_APPLY_PLAYER_COUNT, name, sd);
			return; // Too many guild members online
		}

		std::vector<map_session_data *> list;

		for (const auto &it : g->guild.member) {
			if (list.size() == bg->max_players)
				break;

			if (it.online) {
				map_session_data *pl_sd = map_charid2sd(it.char_id);

				if (pl_sd)
					list.push_back(pl_sd);
			}
		}

		bg_queue_join_multi(name, sd, list); // Join as guild, all on the same side of the BG
	} else {
		ShowWarning("clif_parse_bg_queue_apply_request: Could not find Battleground: \"%s\" requested by player: %s (AID:%d CID:%d)\n", name, sd->status.name, sd->status.account_id, sd->status.char_id);
		clif_bg_queue_apply_result(BG_APPLY_INVALID_NAME, name, sd);
		return; // Invalid BG name
	}
}

/**
 * Join multiple players onto the same side of a Battleground
 * @param name: Battleground name
 * @param sd: Player who requested to join the battlegrounds
 * @param list: Contains all players including the player who requested to join
 */
void bg_queue_join_multi(const char *name, map_session_data *sd, std::vector <map_session_data *> list)
{
	if (!sd) {
		ShowError("bg_queue_join_multi: Tried to join non-existent player\n.");
		return;
	}

	std::shared_ptr<s_battleground_type> bg = bg_search_name(name);

	if (!bg) {
		ShowWarning("bq_queue_join_multi: Could not find battleground \"%s\" requested by %s (AID: %d / CID: %d)\n", name, sd->status.name, sd->status.account_id, sd->status.char_id);
		return;
	}

	if (!bg_queue_check_joinable(bg, sd, name)){
		return;
	}

	for (const auto &queue : bg_queues) {
		if (queue->id != bg->id || queue->state == QUEUE_STATE_SETUP_DELAY || queue->state == QUEUE_STATE_ENDED)
			continue;

		// Make sure there's enough space on one side to join as a party/guild in this queue
		if (queue->teama_members.size() + list.size() > bg->max_players && queue->teamb_members.size() + list.size() > bg->max_players) {
			break;
		}

		bool r = rnd_chance(50, 100);
		std::vector<map_session_data *> *team = r ? &queue->teamb_members : &queue->teama_members;

		if (queue->state == QUEUE_STATE_ACTIVE) {
			// If one team has lesser members try to balance (on an active BG)
			if (r && queue->teama_members.size() < queue->teamb_members.size())
				team = &queue->teama_members;
			else if (!r && queue->teamb_members.size() < queue->teama_members.size())
				team = &queue->teamb_members;
		} else {
			// If the designated team is full, put the player into the other team
			if (team->size() + list.size() > bg->required_players)
				team = r ? &queue->teama_members : &queue->teamb_members;
		}

		while (!list.empty() && team->size() < bg->max_players) {
			map_session_data *sd2 = list.back();

			list.pop_back();

			if (!sd2 || sd2->bg_queue_id > 0)
				continue;

			if (!bg_queue_check_joinable(bg, sd2, name))
				continue;

			sd2->bg_queue_id = queue->queue_id;
			team->push_back(sd2);
			clif_bg_queue_apply_result(BG_APPLY_ACCEPT, name, sd2);
			clif_bg_queue_apply_notify(name, sd2);
		}

		if (queue->state == QUEUE_STATE_ACTIVE) { // Battleground is already active
			for (auto &pl_sd : *team) {
				if (queue->map->mapindex == pl_sd->mapindex)
					continue;

				pc_set_bg_queue_timer(pl_sd);
				clif_bg_queue_lobby_notify(name, pl_sd);
			}
		} else if (queue->state == QUEUE_STATE_SETUP && queue->teamb_members.size() >= bg->required_players && queue->teama_members.size() >= bg->required_players) // Enough players have joined
			bg_queue_on_ready(name, queue);

		return;
	}

	// Something went wrong, sends reconnect and then reapply message to client.
	clif_bg_queue_apply_result(BG_APPLY_RECONNECT, name, sd);
}

/**
 * Clear Battleground queue for next one
 * @param queue: Queue to clean up
 * @param ended: If a Battleground has ended through normal means (by script command bg_unbook)
 */
void bg_queue_clear(std::shared_ptr<s_battleground_queue> queue, bool ended)
{
	if (queue == nullptr)
		return;

	if (queue->tid_requeue != INVALID_TIMER) {
		delete_timer(queue->tid_requeue, bg_on_ready_loopback);
		queue->tid_requeue = INVALID_TIMER;
	}

	if (queue->tid_expire != INVALID_TIMER) {
		delete_timer(queue->tid_expire, bg_on_ready_expire);
		queue->tid_expire = INVALID_TIMER;
	}

	if (queue->tid_start != INVALID_TIMER) {
		delete_timer(queue->tid_start, bg_on_ready_start);
		queue->tid_start = INVALID_TIMER;
	}

	if (ended) {
		if (queue->map != nullptr) {
			queue->map->isReserved = false; // Remove reservation to free up for future queue
			queue->map = nullptr;
		}

		for (const auto &sd : queue->teama_members)
			sd->bg_queue_id = 0;

		for (const auto &sd : queue->teamb_members)
			sd->bg_queue_id = 0;

		queue->teama_members.clear();
		queue->teamb_members.clear();
		queue->teama_members.shrink_to_fit();
		queue->teamb_members.shrink_to_fit();
		queue->accepted_players = 0;
		queue->state = QUEUE_STATE_SETUP;
	}
}

/**
 * Sub function for leaving a Battleground queue
 * @param sd: Player leaving
 * @param members: List of players in queue data
 * @param apply_sc: Apply the SC_ENTRY_QUEUE_APPLY_DELAY status on queue leave (default: true)
 * @return True on success or false otherwise
 */
static bool bg_queue_leave_sub(map_session_data *sd, std::vector<map_session_data *> &members, bool apply_sc)
{
	if (!sd)
		return false;

	auto list_it = members.begin();

	while (list_it != members.end()) {
		if (*list_it == sd) {
			members.erase(list_it);

			if (apply_sc)
				sc_start(nullptr, &sd->bl, SC_ENTRY_QUEUE_APPLY_DELAY, 100, 1, 60000);
			sd->bg_queue_id = 0;
			pc_delete_bg_queue_timer(sd);
			return true;
		} else {
			list_it++;
		}
	}

	return false;
}

/**
 * Leave a Battleground queue
 * @param sd: Player data
 * @param apply_sc: Apply the SC_ENTRY_QUEUE_APPLY_DELAY status on queue leave (default: true)
 * @return True on success or false otherwise
 */
bool bg_queue_leave(map_session_data *sd, bool apply_sc)
{
	if (!sd || sd->bg_queue_id == 0)
		return false;

	pc_delete_bg_queue_timer(sd);

	for (auto &queue : bg_queues) {
		if (sd->bg_queue_id == queue->queue_id) {
			if (!bg_queue_leave_sub(sd, queue->teama_members, apply_sc) && !bg_queue_leave_sub(sd, queue->teamb_members, apply_sc)) {
				ShowError("bg_queue_leave: Couldn't find player %s in battlegrounds queue.\n", sd->status.name);
				return false;
			} else {
				if ((queue->state == QUEUE_STATE_SETUP || queue->state == QUEUE_STATE_SETUP_DELAY) && queue->teama_members.empty() && queue->teamb_members.empty()) // If there are no players left in the queue (that hasn't started), discard it
					bg_queue_clear(queue, true);

				return true;
			}
		}
	}

	return false;
}

/**
 * Send packets to all clients in queue to notify them that the battleground is ready to be joined
 * @param name: Battleground name
 * @param queue: Battleground queue
 * @return True on success or false otherwise
 */
bool bg_queue_on_ready(const char *name, std::shared_ptr<s_battleground_queue> queue)
{
	std::shared_ptr<s_battleground_type> bg = battleground_db.find(queue->id);

	if (!bg) {
		ShowError("bg_queue_on_ready: Couldn't find battleground ID %d in battlegrounds database.\n", queue->id);
		return false;
	}

	if (queue->teama_members.size() < queue->required_players || queue->teamb_members.size() < queue->required_players)
		return false; // Return players to the queue and stop reapplying the timer

	bool map_reserved = false;

	for (auto &map : bg->maps) {
		if (!map.isReserved) {
			map.isReserved = true;
			map_reserved = true;
			queue->map = &map;
			break;
		}
	}

	if (!map_reserved) { // All the battleground maps are reserved. Set a timer to check for an open battleground every 10 seconds.
		queue->tid_requeue = add_timer(gettick() + 10000, bg_on_ready_loopback, 0, (intptr_t)queue->queue_id);
		return false;
	}

	queue->state = QUEUE_STATE_SETUP_DELAY;
	queue->tid_expire = add_timer(gettick() + 20000, bg_on_ready_expire, 0, (intptr_t)queue->queue_id);

	for (const auto &sd : queue->teama_members)
		clif_bg_queue_lobby_notify(name, sd);

	for (const auto &sd : queue->teamb_members)
		clif_bg_queue_lobby_notify(name, sd);

	return true;
}

/**
 * Send a player into an active Battleground
 * @param sd: Player to send in
 * @param queue: Queue data
 */
void bg_join_active(map_session_data *sd, std::shared_ptr<s_battleground_queue> queue)
{
	if (sd == nullptr || queue == nullptr)
		return;

	// Check player's current position for mapflag check
	if (battle_config.bgqueue_nowarp_mapflag > 0 && map_getmapflag(sd->bl.m, MF_NOWARP)) {
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], msg_txt(sd, 337), false, SELF); // You can't apply to a battleground queue from this map.
		bg_queue_leave(sd);
		clif_bg_queue_entry_init(sd);
		return;
	}

	pc_delete_bg_queue_timer(sd); // Cancel timer so player doesn't leave the queue.

	int bg_id_team_1 = static_cast<int>(mapreg_readreg(add_str(queue->map->team1.bg_id_var.c_str())));
	std::shared_ptr<s_battleground_data> bgteam_1 = util::umap_find(bg_team_db, bg_id_team_1);

	for (auto &pl_sd : queue->teama_members) {
		if (sd != pl_sd)
			continue;

		if (bgteam_1 == nullptr) {
			bg_queue_leave(sd);
			clif_bg_queue_apply_result(BG_APPLY_RECONNECT, battleground_db.find(queue->id)->name.c_str(), sd);
			clif_bg_queue_entry_init(sd);
			return;
		}

		clif_bg_queue_entry_init(pl_sd);
		bg_team_join(bg_id_team_1, pl_sd, true);
		npc_event(pl_sd, bgteam_1->active_event.c_str(), 0);
		return;
	}

	int bg_id_team_2 = static_cast<int>(mapreg_readreg(add_str(queue->map->team2.bg_id_var.c_str())));
	std::shared_ptr<s_battleground_data> bgteam_2 = util::umap_find(bg_team_db, bg_id_team_2);

	for (auto &pl_sd : queue->teamb_members) {
		if (sd != pl_sd)
			continue;

		if (bgteam_2 == nullptr) {
			bg_queue_leave(sd);
			clif_bg_queue_apply_result(BG_APPLY_RECONNECT, battleground_db.find(queue->id)->name.c_str(), sd);
			clif_bg_queue_entry_init(sd);
			return;
		}

		clif_bg_queue_entry_init(pl_sd);
		bg_team_join(bg_id_team_2, pl_sd, true);
		npc_event(pl_sd, bgteam_2->active_event.c_str(), 0);
		return;
	}

	return;
}

/**
 * Check to see if any players in the queue are on a map with MF_NOWARP and remove them from the queue
 * @param queue: Queue data
 * @return True if the player is on a map with MF_NOWARP or false otherwise
 */
bool bg_mapflag_check(std::shared_ptr<s_battleground_queue> queue) {
	if (queue == nullptr || battle_config.bgqueue_nowarp_mapflag == 0)
		return false;

	bool found = false;

	for (const auto &sd : queue->teama_members) {
		if (map_getmapflag(sd->bl.m, MF_NOWARP)) {
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], msg_txt(sd, 337), false, SELF); // You can't apply to a battleground queue from this map.
			bg_queue_leave(sd);
			clif_bg_queue_entry_init(sd);
			found = true;
		}
	}

	for (const auto &sd : queue->teamb_members) {
		if (map_getmapflag(sd->bl.m, MF_NOWARP)) {
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], msg_txt(sd, 337), false, SELF); // You can't apply to a battleground queue from this map.
			bg_queue_leave(sd);
			clif_bg_queue_entry_init(sd);
			found = true;
		}
	}

	if (found) {
		queue->state = QUEUE_STATE_SETUP; // Set back to queueing state
		queue->accepted_players = 0; // Reset acceptance count

		// Free map to avoid creating a reservation delay
		if (queue->map != nullptr) {
			queue->map->isReserved = false;
			queue->map = nullptr;
		}

		// Announce failure to remaining players
		for (const auto &sd : queue->teama_members)
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], msg_txt(sd, 340), false, SELF); // Participants were unable to join. Delaying entry for more participants.

		for (const auto &sd : queue->teamb_members)
			clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], msg_txt(sd, 340), false, SELF); // Participants were unable to join. Delaying entry for more participants.
	}

	return found;
}

/**
 * Update the Battleground queue when the player accepts the invite
 * @param queue: Battleground queue
 * @param sd: Player data
 */
void bg_queue_on_accept_invite(map_session_data *sd)
{
	nullpo_retv(sd);

	std::shared_ptr<s_battleground_queue> queue = bg_search_queue(sd->bg_queue_id);

	if (queue == nullptr) {
		ShowError("bg_queue_on_accept_invite: Couldn't find player %s in battlegrounds queue.\n", sd->status.name);
		return;
	}

	queue->accepted_players++;
	clif_bg_queue_ack_lobby(true, mapindex_id2name(queue->map->mapindex), mapindex_id2name(queue->map->mapindex), sd);

	if (queue->state == QUEUE_STATE_ACTIVE) // Battleground is already active
		bg_join_active(sd, queue);
	else if (queue->state == QUEUE_STATE_SETUP_DELAY) {
		if (queue->accepted_players == queue->required_players * 2) {
			if (queue->tid_expire != INVALID_TIMER) {
				delete_timer(queue->tid_expire, bg_on_ready_expire);
				queue->tid_expire = INVALID_TIMER;
			}

			// Check player's current position for mapflag check
			if (battle_config.bgqueue_nowarp_mapflag > 0 && bg_mapflag_check(queue))
				return;

			queue->tid_start = add_timer(gettick() + battleground_db.find(queue->id)->start_delay * 1000, bg_on_ready_start, 0, (intptr_t)queue->queue_id);
		}
	}
}

/**
 * Begin the Battleground from the given queue
 * @param queue: Battleground queue
 */
void bg_queue_start_battleground(std::shared_ptr<s_battleground_queue> queue)
{
	if (queue == nullptr)
		return;

	std::shared_ptr<s_battleground_type> bg = battleground_db.find(queue->id);

	if (!bg) {
		bg_queue_clear(queue, true);
		ShowError("bg_queue_start_battleground: Could not find battleground ID %d in battlegrounds database.\n", queue->id);
		return;
	}

	// Check player's current position for mapflag check
	if (battle_config.bgqueue_nowarp_mapflag > 0 && bg_mapflag_check(queue))
		return;

	uint16 map_idx = queue->map->mapindex;
	int bg_team_1 = bg_create(map_idx, &queue->map->team1);
	int bg_team_2 = bg_create(map_idx, &queue->map->team2);

	for (const auto &sd : queue->teama_members) {
		clif_bg_queue_entry_init(sd);
		bg_team_join(bg_team_1, sd, true);
	}

	for (const auto &sd : queue->teamb_members) {
		clif_bg_queue_entry_init(sd);
		bg_team_join(bg_team_2, sd, true);
	}

	mapreg_setreg(add_str(queue->map->team1.bg_id_var.c_str()), bg_team_1);
	mapreg_setreg(add_str(queue->map->team2.bg_id_var.c_str()), bg_team_2);
	npc_event_do(queue->map->bgcallscript.c_str());
	queue->state = QUEUE_STATE_ACTIVE;

	bg_queue_clear(queue, false);
}

/**
 * Initialize a Battleground queue
 * @param bg_id: Battleground ID
 * @param req_players: Required amount of players
 * @return s_battleground_queue*
 */
static void bg_queue_create(int bg_id, int req_players)
{
	auto queue = std::make_shared<s_battleground_queue>();

	queue->queue_id = bg_queue_count++;
	queue->id = bg_id;
	queue->required_players = req_players;
	queue->accepted_players = 0;
	queue->tid_expire = INVALID_TIMER;
	queue->tid_start = INVALID_TIMER;
	queue->tid_requeue = INVALID_TIMER;
	queue->state = QUEUE_STATE_SETUP;

	bg_queues.push_back(queue);
}

/**
 * Initialize the Battleground data
 */
void do_init_battleground(void)
{
	if (battle_config.feature_bgqueue) {
		battleground_db.load();

		for (const auto &bg : battleground_db)
			bg_queue_create(bg.first, bg.second->required_players);
	}

	add_timer_func_list(bg_send_xy_timer, "bg_send_xy_timer");
	add_timer_func_list(bg_on_ready_loopback, "bg_on_ready_loopback");
	add_timer_func_list(bg_on_ready_expire, "bg_on_ready_expire");
	add_timer_func_list(bg_on_ready_start, "bg_on_ready_start");
	add_timer_interval(gettick() + battle_config.bg_update_interval, bg_send_xy_timer, 0, 0, battle_config.bg_update_interval);
}

/**
 * Clear the Battleground data from memory
 */
void do_final_battleground(void)
{
}
