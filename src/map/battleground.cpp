// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "battleground.hpp"

#include <unordered_map>
#include <yaml-cpp/yaml.h>

#include "../common/cbasetypes.hpp"
#include "../common/malloc.hpp"
#include "../common/nullpo.hpp"
#include "../common/random.hpp"
#include "../common/showmsg.hpp"
#include "../common/strlib.hpp"
#include "../common/timer.hpp"
#include "../common/utilities.hpp"

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

const std::string BattlegroundDatabase::getDefaultLocation() {
	return std::string(db_path) + "/battleground_db.yml";
}

/**
 * Reads and parses an entry from the battleground_db
 * @param node: The YAML node containing the entry
 * @return count of successfully parsed rows
 */
uint64 BattlegroundDatabase::parseBodyNode(const YAML::Node &node) {
	uint32 id;

	if (!this->asUInt32(node, "Id", id))
		return 0;

	std::shared_ptr<s_battleground_type> bg = this->find(id);
	bool exists = bg != nullptr;

	if (!exists) {
		if (!this->nodeExists(node, "Name")) {
			this->invalidWarning(node, "Node \"Name\" is missing.\n");
			return 0;
		}

		if (!this->nodeExists(node, "Locations")) {
			this->invalidWarning(node, "Node \"Locations\" is missing.\n");
			return 0;
		}

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

		if (min * 2 > MAX_BG_MEMBERS) {
			this->invalidWarning(node, "Minimum players %d exceeds MAX_BG_MEMBERS, capping to %d.\n", min, MAX_BG_MEMBERS / 2);
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

		if (max * 2 > MAX_BG_MEMBERS) {
			this->invalidWarning(node, "Maximum players %d exceeds MAX_BG_MEMBERS, capping to %d.\n", max, MAX_BG_MEMBERS / 2);
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
			this->invalidWarning(node, "Minimum level %d exceeds MAX_LEVEL, capping to %d.\n", min, MAX_LEVEL);
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
			this->invalidWarning(node, "Maximum level %d exceeds MAX_LEVEL, capping to %d.\n", max, MAX_LEVEL);
			max = MAX_LEVEL;
		}

		bg->max_lvl = max;
	} else {
		if (!exists)
			bg->max_lvl = MAX_LEVEL;
	}

	if (this->nodeExists(node, "Deserter")) {
		int deserter;

		if (!this->asInt32(node, "Deserter", deserter))
			return 0;

		bg->deserter_time = deserter;
	} else {
		if (!exists)
			bg->deserter_time = 600;
	}

	if (this->nodeExists(node, "Locations")) {
		const YAML::Node &locations = node["Locations"];
		int count = 0;

		for (const auto &locationit : locations) {
			const YAML::Node &location = locationit;
			struct s_battleground_map map_entry;

			if (this->nodeExists(location, "Map")) {
				std::string map_name;

				if (!this->asString(location, "Map", map_name))
					return 0;

				map_entry.mapid = map_mapname2mapid(map_name.c_str());

				if (map_entry.mapid == -1) {
					this->invalidWarning(location["Map"], "Invalid battleground map name %s, skipping.\n", map_name.c_str());
					return 0;
				}
			}

			if (this->nodeExists(location, "StartEvent")) {
				if (!this->asString(location, "StartEvent", map_entry.bgcallscript))
					return 0;

				map_entry.bgcallscript.resize(EVENT_NAME_LENGTH);

				if (map_entry.bgcallscript.find("::On") == std::string::npos) {
					this->invalidWarning(location["StartEvent"], "Battleground StartEvent label %s should begin with '::On', skipping.\n", map_entry.bgcallscript.c_str());
					return 0;
				}
			}

			std::vector<std::string> team_list = { "TeamA", "TeamB" };

			for (const auto &it : team_list) {
				const YAML::Node &team = location[it];

				if (this->nodeExists(team, it)) {
					struct s_battleground_team *team_ptr; // TODO: might be better to use a map than this construct

					if (it.find("TeamA"))
						team_ptr = &map_entry.team1;
					else
						team_ptr = &map_entry.team2;

					if (this->nodeExists(team, "RespawnX")) {
						if (!this->asInt16(team, "RespawnX", team_ptr->warp_x))
							return 0;
					}

					if (this->nodeExists(team, "RespawnY")) {
						if (!this->asInt16(team, "RespawnY", team_ptr->warp_y))
							return 0;
					}

					if (this->nodeExists(team, "DeathEvent")) {
						if (this->asString(team, "DeathEvent", team_ptr->death_event))
							return 0;

						team_ptr->death_event.resize(EVENT_NAME_LENGTH);

						if (team_ptr->death_event.find("::On") == std::string::npos) {
							this->invalidWarning(team["DeathEvent"], "Battleground DeathEvent label %s should begin with '::On', skipping.\n", team_ptr->death_event.c_str());
							return 0;
						}
					}

					if (this->nodeExists(team, "QuitEvent")) {
						if (this->asString(team, "QuitEvent", team_ptr->quit_event))
							return 0;

						team_ptr->quit_event.resize(EVENT_NAME_LENGTH);

						if (team_ptr->quit_event.find("::On") == std::string::npos) {
							this->invalidWarning(team["QuitEvent"], "Battleground QuitEvent label %s should begin with '::On', skipping.\n", team_ptr->quit_event.c_str());
							return 0;
						}
					}

					if (this->nodeExists(team, "Variable")) {
						if (!this->asString(team, "Variable", team_ptr->bg_id_var))
							return 0;

						team_ptr->bg_id_var.resize(NAME_LENGTH);
					}

					map_entry.id = count++;
					map_entry.isReserved = false;
					bg->maps.push_back(map_entry);
				}
			}
		}
	}

	if (!exists)
		this->put(id, bg);

	return 1;
}

/**
 * Search for a battleground from the database
 * @param bg_id: ID to lookup
 * @return s_battleground_type on success or nullptr on failure
 */
std::shared_ptr<s_battleground_type> bg_search(int bg_id)
{
	if (!battleground_db.exists(bg_id))
		return nullptr;

	return battleground_db.find(bg_id);
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

	ShowError("bg_search_name: Fatal error. Can't find battleground %s in the battlegrounds database.\n", name);
	return nullptr;
}

/**
 * Searches for a battleground team by ID
 * @param bg_id: ID to lookup
 * @return True if battleground team exists or false if it doesn't
 */
bool battleground_team_exists(int bg_id)
{
	return bg_team_db.find(bg_id) != bg_team_db.end();
}

/**
 * Search for a Battleground team based on the give ID
 * @param bg_id: Battleground ID
 * @return s_battleground_data or nullptr if not found
 */
std::shared_ptr<s_battleground_data> bg_team_search(int bg_id)
{
	if (!battleground_team_exists(bg_id))
		return nullptr;

	return bg_team_db[bg_id];
}

/**
 * Search for an available player in Battleground
 * @param bg: Battleground data
 * @return map_session_data
 */
struct map_session_data* bg_getavailablesd(struct s_battleground_data *bg)
{
	nullpo_retr(nullptr, bg);

	return (bg->members.size() != 0) ? bg->members[0].sd : nullptr;
}

/**
 * Delete a Battleground team from the db
 * @param bg_id: Battleground ID
 * @return True on success or false otherwise
 */
bool bg_team_delete(int bg_id)
{
	std::shared_ptr<s_battleground_data> bgteam = bg_team_search(bg_id);

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
	std::shared_ptr<s_battleground_data> bgteam = bg_team_search(bg_id);

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
void bg_send_dot_remove(struct map_session_data *sd)
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
bool bg_team_join(int bg_id, struct map_session_data *sd, bool is_queue)
{
	if (!sd || sd->bg_id)
		return false;

	std::shared_ptr<s_battleground_data> bgteam = bg_team_search(bg_id);

	if (bgteam) {
		if (bgteam->members.size() == MAX_BG_MEMBERS)
			return false; // No free slots

		struct s_battleground_member_data member = {};

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
				clif_hpmeter_single(sd->fd, pl_sd.sd->bl.id, pl_sd.sd->battle_status.hp, pl_sd.sd->battle_status.max_hp);
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
int bg_team_leave(struct map_session_data *sd, bool quit, bool deserter)
{
	if (!sd || !sd->bg_id)
		return -1;

	bg_send_dot_remove(sd);

	int bg_id = sd->bg_id;
	std::shared_ptr<s_battleground_data> bgteam = bg_team_search(bg_id);

	sd->bg_id = 0;

	if (bgteam) {
		char output[CHAT_SIZE_MAX];
		int i;

		ARR_FIND(0, bgteam->members.size(), i, bgteam->members[i].sd == sd);
		if (i < bgteam->members.size()) { // Removes member from BG
			if (bgteam->members[i].entry_point.map != 0) {
				unsigned short map_id = map_mapindex2mapid(bgteam->members[i].entry_point.map);

				if (!map_getmapflag(map_id, MF_NOSAVE))
					pc_setpos(sd, bgteam->members[i].entry_point.map, bgteam->members[i].entry_point.x, bgteam->members[i].entry_point.y, CLR_TELEPORT);
				else
					pc_setpos(sd, sd->status.save_point.map, sd->status.save_point.x, sd->status.save_point.y, CLR_TELEPORT); // Warp to save point if the entry map has no save flag.
			}
			util::erase_at(bgteam->members, i);
		}

		if (quit)
			sprintf(output, "Server: %s has quit the game...", sd->status.name);
		else
			sprintf(output, "Server: %s is leaving the battlefield...", sd->status.name);

		clif_bg_message(bgteam.get(), 0, "Server", output, strlen(output) + 1);

		if (!bgteam->logout_event.empty() && quit)
			npc_event(sd, bgteam->logout_event.c_str(), 0);

		if (deserter) {
			std::shared_ptr<s_battleground_type> bg = bg_search(bg_id);

			if (bg)
				sc_start(NULL, &sd->bl, SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT, 100, 1, bg->deserter_time * 1000); // Deserter timer
		}

		return bgteam->members.size();
	}

	return -1;
}

/**
 * Respawn a Battleground player
 * @param sd: Player data
 * @return True on success or false otherwise
 */
bool bg_member_respawn(struct map_session_data *sd)
{
	if (!sd || !sd->bg_id || !pc_isdead(sd))
		return false;

	std::shared_ptr<s_battleground_data> bgteam = bg_team_search(sd->bg_id);

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
int bg_create(uint16 mapindex, struct s_battleground_team* team)
{
	int bg_team_counter = 1;

	while (battleground_team_exists(bg_team_counter))
		bg_team_counter++;

	bg_team_db[bg_team_counter] = std::make_shared<s_battleground_data>();

	std::shared_ptr<s_battleground_data> bg = bg_team_search(bg_team_counter);

	bg->bg_id = bg_team_counter;
	bg->cemetery.map = mapindex;
	bg->cemetery.x = team->warp_x;
	bg->cemetery.y = team->warp_y;
	bg->logout_event = team->quit_event.c_str();
	bg->die_event = team->death_event.c_str();
	bg->members.clear();

	return bg->bg_id;
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
			struct map_session_data *msd;
			struct mob_data *md = (TBL_MOB*)bl;

			if( md->special_state.ai && !(msd = map_id2sd(md->master_id)) )
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
void bg_send_message(struct map_session_data *sd, const char *mes, int len)
{
	nullpo_retv(sd);

	if (!sd->bg_id)
		return;
	
	std::shared_ptr<s_battleground_data> bgteam = bg_team_search(sd->bg_id);

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
	struct map_session_data *sd;

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
 * @param tid: Tick ID
 * @param tick: Timer
 * @param id: Timer ID
 */
TIMER_FUNC(bg_send_xy_timer)
{
	for (const auto &entry : bg_team_db)
		bg_send_xy_timer_sub(entry.second);

	return 0;
}

/**
 * Reset Battleground queue data
 * @param tid: Tick ID
 * @param tick: Timer
 * @param id: Timer ID
 */
static TIMER_FUNC(bg_on_ready_expire)
{
	struct s_battleground_queue *queue = (struct s_battleground_queue*)data;
	struct s_battleground_map *bgmap = nullptr;

	nullpo_ret(queue);

	queue->in_ready_state = false;
	queue->map.isReserved = false;
	queue->map = *bgmap;
	queue->accepted_players = 0; // Reset the queue count

	for (const auto &sd : queue->teama_members) {
		sd->bg_queue_accept_state = false;
		clif_displaymessage(sd->fd, msg_txt(sd, 341)); // Someone declined or didn't accept the invitation to enter. You have been returned to the battleground queue.
	}

	for (const auto &sd : queue->teamb_members) {
		sd->bg_queue_accept_state = false;
		clif_displaymessage(sd->fd, msg_txt(sd, 341)); // Someone declined or didn't accept the invitation to enter. You have been returned to the battleground queue.
	}
	return 0;
}

/**
 * Mark a Battleground as ready to begin queuing
 * @param tid: Tick ID
 * @param tick: Timer
 * @param id: Timer ID
 * @return 0 on success or 1 otherwise
 */
static TIMER_FUNC(bg_on_ready_loopback)
{
	struct s_battleground_queue *queue = (struct s_battleground_queue*)data;

	nullpo_ret(queue);

	std::shared_ptr<s_battleground_type> bg = bg_search(queue->bg_type);

	if (bg) {
		bg_queue_on_ready(bg->name.c_str(), std::shared_ptr<s_battleground_queue>(queue));
		return 0;
	} else {
		ShowError("bg_on_ready_loopback: Fatal error. Can't find battleground %d in the battlegrounds database.\n", queue->bg_type);
		return 1;
	}
}

/**
 * Check if the given player is in a battleground
 * @param sd: Player data
 * @return True if in a battleground or false otherwise
 */
bool bg_player_is_in_bg_map(struct map_session_data *sd)
{
	nullpo_retr(false, sd);

	for (const auto &pair : battleground_db) {
		for (const auto &it : pair.second->maps) {
			if (it.mapid == sd->bl.m)
				return true;
		}
	}

	return false;
}

/**
 * Check to see if a Battleground is joinable
 * @param bg: Battleground data
 * @param sd: Player data
 * @param name: Battleground name
 * @return True on success or false otherwise
 */
bool bg_queue_check_joinable(std::shared_ptr<s_battleground_type> bg, struct map_session_data *sd, char *name)
{
	nullpo_retr(false, sd);

	if (bg->min_lvl && sd->status.base_level < bg->min_lvl) { // Check min level if min_lvl isn't 0
		clif_bg_queue_apply_result(BG_APPLY_PLAYER_LEVEL, name, sd);
		return false;
	}

	if (bg->max_lvl && sd->status.base_level > bg->max_lvl) { // Check max level if max_lvl isn't 0
		clif_bg_queue_apply_result(BG_APPLY_PLAYER_LEVEL, name, sd);
		return false;
	}

	if (sd->sc.data[SC_ENTRY_QUEUE_APPLY_DELAY]) { // Exclude any player who's recently left a battleground queue
		char buf[CHAT_SIZE_MAX];

		sprintf(buf, msg_txt(sd, 339), (get_timer(sd->sc.data[SC_ENTRY_QUEUE_APPLY_DELAY]->timer)->tick-gettick()) / 1000); // You can't apply to a battleground queue for %d seconds due to recently leaving one.
		clif_bg_queue_apply_result(BG_APPLY_NONE, name, sd);
		clif_displaymessage(sd->fd, buf);
		return false;
	}

	if (sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]) { // Exclude any player who's recently deserted a battleground
		char buf[CHAT_SIZE_MAX];

		sprintf(buf, msg_txt(sd, 338), ((get_timer(sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]->timer)->tick-gettick()) / 1000) / 60, ((get_timer(sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]->timer)->tick-gettick()) / 1000) % 60); // You can't apply to a battleground queue due to recently deserting a battleground. Time remaining: %d minutes and %d seconds.
		clif_bg_queue_apply_result(BG_APPLY_NONE, name, sd);
		clif_displaymessage(sd->fd, buf);
		return false;
	}

	if (bg_player_is_in_bg_map(sd)) { // Is the player currently in a battleground map? Reject them.
		clif_bg_queue_apply_result(BG_APPLY_NONE, name, sd);
		clif_displaymessage(sd->fd, msg_txt(sd, 337)); // You may not join a battleground queue when you're in a battleground map.
		return false;
	}

	return true; // Return true if all conditions are met.
}

/**
 * Sub function for reserving a slot in the Battleground if it's joinable
 * @param name: Battleground map name
 * @param state: Whether to mark reserved or not
 * @return True on success or false otherwise
 */
bool bg_queue_reservation(const char *name, bool state)
{
	int16 mapid = map_mapname2mapid(name);

	for (const auto &pair : battleground_db) {
		// Bound checking isn't needed since we iterate within battleground_db's bound.
		for (auto &it : pair.second->maps) {
			if (it.mapid == mapid) {
				it.isReserved = state;
				return true;
			}
		}
	}

	return false;
}

/**
 * Initialize a Battleground queue
 * @param bg_id: Battleground ID
 * @param req_players: Required amount of players
 * @return s_battleground_queue*
 */
std::shared_ptr<s_battleground_queue> bg_queue_create(int bg_id, int req_players)
{
	auto queue = std::make_shared<s_battleground_queue>();

	queue->bg_type = bg_id;
	queue->required_players = req_players;
	queue->accepted_players = 0;
	queue->tid_expire = 0;
	queue->tid_requeue = 0;
	queue->in_ready_state = false;

	return queue;
}

/**
 * Allow a player to join a Battleground queue
 * @param name: Battleground name
 * @param sd: Player data
 * @return @see e_bg_queue_apply_ack
 */
enum e_bg_queue_apply_ack bg_queue_join(char *name, struct map_session_data *sd)
{
	if (!sd) {
		ShowError("bg_queue_join: Fatal error. Tried to join non-existent player.\n.");
		return BG_APPLY_NONE;
	}

	if (sd->sc.data[SC_ENTRY_QUEUE_APPLY_DELAY]) {
		char buf[CHAT_SIZE_MAX];

		sprintf(buf, msg_txt(sd, 339), (get_timer(sd->sc.data[SC_ENTRY_QUEUE_APPLY_DELAY]->timer)->tick - gettick()) / 1000); // You can't apply to a battleground queue for %d seconds due to recently leaving one.
		clif_displaymessage(sd->fd, buf);
		return BG_APPLY_NONE;
	}

	if (sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]) {
		char buf[CHAT_SIZE_MAX];

		sprintf(buf, msg_txt(sd, 338), ((get_timer(sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]->timer)->tick - gettick()) / 1000) / 60, ((get_timer(sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]->timer)->tick - gettick()) / 1000) % 60); // You can't apply to a battleground queue due to recently deserting a battleground. Time remaining: %d minutes and %d seconds.
		clif_displaymessage(sd->fd, buf);
		return BG_APPLY_NONE;
	}

	if (bg_player_is_in_bg_map(sd)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 337)); // You may not join a battleground queue when you're in a battleground map.
		return BG_APPLY_NONE;
	}

	std::shared_ptr<s_battleground_type> bg = bg_search_name(name);

	if (!bg) {
		ShowWarning("bq_queue_join: Could not find battleground \"%s\" requested by %s (AID: %d / CID: %d)\n", name, sd->status.name, sd->status.account_id, sd->status.char_id);
		return BG_APPLY_INVALID_NAME;
	}

	std::shared_ptr<s_battleground_queue> queue;
	bool r;

	if (bg->min_lvl && sd->status.base_level < bg->min_lvl)
		return BG_APPLY_PLAYER_LEVEL; // Level too low

	if (bg->max_lvl && sd->status.base_level > bg->max_lvl)
		return BG_APPLY_PLAYER_LEVEL; // Level too high

	if (bg_queues.empty()) {
		r = rnd() % 2 != 0;

		queue = bg_queue_create(bg->id, bg->required_players);

		if (!r)
			queue->teama_members.push_back(sd);
		else
			queue->teamb_members.push_back(sd);

		sd->bg_queue = queue;
		bg_queues.insert(bg_queues.begin(), queue);
		return BG_APPLY_ACCEPT;
	} else {
		r = rnd() % 2 != 0;

		for (const auto &it : bg_queues) {
			try {
				queue = it;
			} catch (std::out_of_range &) {
				continue;
			}

			if (queue->in_ready_state)
				continue;
			if (!r) {
				if (queue->teama_members.size() != queue->required_players) {
					sd->bg_queue = queue;
					queue->teama_members.push_back(sd);

					if (queue->teama_members.size() == bg->required_players && queue->teamb_members.size() == bg->required_players) // Enough players have joined
						bg_queue_on_ready(name, queue);
					return BG_APPLY_ACCEPT;
				} else if (queue->teamb_members.size() != queue->required_players) {
					sd->bg_queue = queue;
					queue->teamb_members.push_back(sd);

					if (queue->teama_members.size() == bg->required_players && queue->teamb_members.size() == bg->required_players) // Enough players have joined
						bg_queue_on_ready(name, queue);
					return BG_APPLY_ACCEPT;
				}
			} else {
				if (queue->teamb_members.size() != queue->required_players) {
					sd->bg_queue = queue;
					queue->teamb_members.push_back(sd);

					if (queue->teama_members.size() == bg->required_players && queue->teamb_members.size() == bg->required_players) // Enough players have joined
						bg_queue_on_ready(name, queue);
					return BG_APPLY_ACCEPT;
				} else if (queue->teama_members.size() != queue->required_players) {
					sd->bg_queue = queue;
					queue->teama_members.push_back(sd);

					if (queue->teama_members.size() == bg->required_players && queue->teamb_members.size() == bg->required_players) // Enough players have joined
						bg_queue_on_ready(name, queue);
					return BG_APPLY_ACCEPT;
				}
			}
		}
	}

	queue = bg_queue_create(bg->id, bg->required_players);
	r = rnd() % 2 != 0;

	if (!r)
		queue->teama_members.push_back(sd);
	else
		queue->teamb_members.push_back(sd);

	sd->bg_queue = queue;
	bg_queues.insert(bg_queues.begin(), queue);
	return BG_APPLY_ACCEPT;
}

/**
 * Join a party onto the same side of a Battleground
 * @param name: Battleground name
 * @param sd: Player who requested to join the battlegrounds
 * @return @see e_bg_queue_apply_ack
 */
enum e_bg_queue_apply_ack bg_queue_join_party(char *name, struct map_session_data *sd)
{
	struct party_data *p = party_search(sd->status.party_id);

	if (!p)
		return BG_APPLY_INVALID_APP; // Someone has bypassed the client check for being in a party

	for (const auto &it : p->party.member) {
		if (it.leader && sd->status.char_id != it.char_id)
			return BG_APPLY_PARTYGUILD_LEADER; // Not the party leader
	}

	std::shared_ptr<s_battleground_type> bg = bg_search_name(name);

	if (bg) {
		int p_online = 0;

		for (const auto &it : p->party.member) {
			if (it.online)
				p_online++;
		}

		if (p_online > bg->max_players)
			return BG_APPLY_PLAYER_COUNT; // Too many party members online
		
		std::vector<struct map_session_data *> list;

		for (const auto &it : p->party.member) {
			if (list.size() == bg->max_players)
				break;

			if (it.online) {
				struct map_session_data *pl_sd = map_charid2sd(it.char_id);

				if (pl_sd)
					list.push_back(pl_sd);
			}
		}

		return bg_queue_join_multi(name, sd, list); // Join as party, all on the same side of the BG
	} else {
		ShowWarning("clif_parse_bg_queue_apply_request: Could not find Battleground: \"%s\" requested by player: %s (AID:%d CID:%d)\n", name, sd->status.name, sd->status.account_id, sd->status.char_id);
		return BG_APPLY_INVALID_NAME; // Invalid BG name
	}
}

/**
 * Join a guild onto the same side of a Battleground
 * @param name: Battleground name
 * @param sd: Player who requested to join the battlegrounds
 * @return @see e_bg_queue_apply_ack
 */
enum e_bg_queue_apply_ack bg_queue_join_guild(char *name, struct map_session_data *sd)
{
	if (!sd->guild)
		return BG_APPLY_INVALID_APP; // Someone has bypassed the client check for being in a guild
	
	if (strcmp(sd->status.name, sd->guild->master) != 0)
		return BG_APPLY_PARTYGUILD_LEADER; // Not the guild leader

	std::shared_ptr<s_battleground_type> bg = bg_search_name(name);

	if (bg) {
		struct guild *g = guild_search(sd->status.guild_id);
		int g_online = 0;

		for (const auto &it : g->member) {
			if (it.online)
				g_online++;
		}

		if (g_online > bg->max_players)
			return BG_APPLY_PLAYER_COUNT; // Too many guild members online

		std::vector<struct map_session_data *> list;

		for (const auto &it : g->member) {
			if (list.size() == bg->max_players)
				break;

			if (it.online) {
				struct map_session_data *pl_sd = map_charid2sd(it.char_id);

				if (pl_sd)
					list.push_back(pl_sd);
			}
		}

		return bg_queue_join_multi(name, sd, list); // Join as guild, all on the same side of the BG
	} else {
		ShowWarning("clif_parse_bg_queue_apply_request: Could not find Battleground: \"%s\" requested by player: %s (AID:%d CID:%d)\n", name, sd->status.name, sd->status.account_id, sd->status.char_id);
		return BG_APPLY_INVALID_NAME; // Invalid BG name
	}
}

/**
 * Join multiple players onto the same side of a Battleground
 * @param name: Battleground name
 * @param sd: Player who requested to join the battlegrounds
 * @param list: Contains all players including the player who requested to join
 * @return @see e_bg_queue_apply_ack
 */
enum e_bg_queue_apply_ack bg_queue_join_multi(char *name, struct map_session_data *sd, std::vector <map_session_data *> list)
{
	if (!sd) {
		ShowError("bg_queue_join_multi: Fatal error. Tried to join non-existent player\n.");
		return BG_APPLY_NONE;
	}

	if (sd->sc.data[SC_ENTRY_QUEUE_APPLY_DELAY]) {
		char buf[CHAT_SIZE_MAX];

		sprintf(buf, msg_txt(sd, 339), (get_timer(sd->sc.data[SC_ENTRY_QUEUE_APPLY_DELAY]->timer)->tick - gettick()) / 1000); // You can't apply to a battleground queue for %d seconds due to recently leaving one.
		clif_displaymessage(sd->fd, buf);
		return BG_APPLY_NONE;
	}

	if (sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]) {
		char buf[CHAT_SIZE_MAX];

		sprintf(buf, msg_txt(sd, 338), ((get_timer(sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]->timer)->tick - gettick()) / 1000) / 60, ((get_timer(sd->sc.data[SC_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT]->timer)->tick - gettick()) / 1000) % 60); // You can't apply to a battleground queue due to recently deserting a battleground. Time remaining: %d minutes and %d seconds.
		clif_displaymessage(sd->fd, buf);
		return BG_APPLY_NONE;
	}

	if (bg_player_is_in_bg_map(sd)) {
		clif_displaymessage(sd->fd, msg_txt(sd, 337)); // You may not join a battleground queue when you're in a battleground map.
		return BG_APPLY_NONE;
	}

	std::shared_ptr<s_battleground_type> bg = bg_search_name(name);

	if (!bg) {
		ShowWarning("bq_queue_join_multi: Could not find battleground \"%s\" requested by %s (AID: %d / CID: %d)\n", name, sd->status.name, sd->status.account_id, sd->status.char_id);
		return BG_APPLY_INVALID_NAME;
	}

	if (bg->min_lvl && sd->status.base_level < bg->min_lvl)
		return BG_APPLY_PLAYER_LEVEL; // Level too low
	
	if (bg->max_lvl && sd->status.base_level > bg->max_lvl)
		return BG_APPLY_PLAYER_LEVEL; // Level too high

	if (bg_queues.empty()) {
		std::shared_ptr<s_battleground_queue> queue = bg_queue_create(bg->id, bg->required_players);
		bool r = rnd() % 2 != 0;

		if (!r) {
			while (!list.empty() && queue->teama_members.size() < bg->required_players) {
				struct map_session_data *sd2 = list.back();

				list.pop_back();

				if (!sd2 || sd2->bg_queue)
					continue;

				if (!bg_queue_check_joinable(bg, sd2, name))
					continue;

				sd2->bg_queue = queue;
				clif_bg_queue_apply_result(BG_APPLY_ACCEPT, name, sd2);
				clif_bg_queue_apply_notify(name, sd2);
				queue->teama_members.insert(queue->teama_members.begin(), sd2);
			}
		} else {
			while (!list.empty() && queue->teamb_members.size() < bg->required_players) {
				struct map_session_data *sd2 = list.back();

				list.pop_back();

				if (!sd2 || sd2->bg_queue)
					continue;

				if (!bg_queue_check_joinable(bg, sd2, name))
					continue;

				sd2->bg_queue = queue;
				clif_bg_queue_apply_result(BG_APPLY_ACCEPT, name, sd2);
				clif_bg_queue_apply_notify(name, sd2);
				queue->teamb_members.insert(queue->teamb_members.begin(), sd2);
			}
		}

		bg_queues.insert(bg_queues.begin(), queue);

		return BG_APPLY_ACCEPT;
	} else {
		std::shared_ptr<s_battleground_queue> queue;
		bool r = rnd() % 2 != 0;

		for (const auto &it : bg_queues) {
			try {
				queue = it;
			} catch (std::out_of_range &) {
				continue;
			}

			if (queue->in_ready_state)
				continue;

			if (queue->teama_members.size() + list.size() <= bg->required_players || queue->teamb_members.size() + list.size() <= bg->required_players) { // Make sure there's enough space on one side to join as a party/guild in this queue
				if (!r && queue->teama_members.size() + list.size() <= bg->required_players) {
					while (!list.empty() && queue->teama_members.size() < bg->required_players) {
						struct map_session_data *sd2 = list.back();

						list.pop_back();

						if (!sd2 || sd2->bg_queue)
							continue;

						if (!bg_queue_check_joinable(bg, sd2, name))
							continue;

						sd2->bg_queue = queue;
						clif_bg_queue_apply_result(BG_APPLY_ACCEPT, name, sd2);
						clif_bg_queue_apply_notify(name, sd2);
						queue->teama_members.insert(queue->teama_members.begin(), sd2);
					}

					if (queue->teama_members.size() == bg->required_players && queue->teamb_members.size() == bg->required_players) // Enough players have joined
						bg_queue_on_ready(name, queue);

					return BG_APPLY_ACCEPT;
				} else {
					while (!list.empty() && queue->teamb_members.size() < bg->required_players) {
						struct map_session_data *sd2 = list.back();

						list.pop_back();

						if (!sd2)
							continue;

						if (!bg_queue_check_joinable(bg, sd2, name))
							continue;

						sd2->bg_queue = queue;
						clif_bg_queue_apply_result(BG_APPLY_ACCEPT, name, sd2);
						clif_bg_queue_apply_notify(name, sd2);
						queue->teamb_members.insert(queue->teamb_members.begin(), sd2);
					}
				}

				return BG_APPLY_ACCEPT;
			}
		}

		// Create a new queue if none of the existing ones are joinable for this party/guild
		queue = nullptr;
		queue = bg_queue_create(bg->id, bg->required_players);
		r = rnd() % 2 != 0;

		if (!r) {
			while (!list.empty() && queue->teama_members.size() < bg->required_players) {
				struct map_session_data *sd2 = list.back();

				list.pop_back();

				if (!sd2 || sd2->bg_queue)
					continue;

				if (!bg_queue_check_joinable(bg, sd2, name))
					continue;

				sd2->bg_queue = queue;
				clif_bg_queue_apply_result(BG_APPLY_ACCEPT, name, sd2);
				clif_bg_queue_apply_notify(name, sd2);
				queue->teama_members.insert(queue->teama_members.begin(), sd2);
			}
		} else {
			while (!list.empty() && queue->teamb_members.size() < bg->required_players) {
				struct map_session_data *sd2 = list.back();

				list.pop_back();

				if (!sd2 || sd2->bg_queue)
					continue;

				if (!bg_queue_check_joinable(bg, sd2, name))
					continue;

				sd2->bg_queue = queue;
				clif_bg_queue_apply_result(BG_APPLY_ACCEPT, name, sd2);
				clif_bg_queue_apply_notify(name, sd2);
				queue->teamb_members.insert(queue->teamb_members.begin(), sd2);
			}
		}

		bg_queues.insert(bg_queues.begin(), queue);

		return BG_APPLY_ACCEPT;
	}

	return BG_APPLY_RECONNECT; // Something went wrong, sends reconnect and then reapply message to client.
}

/**
 * Sub function for leaving a Battleground queue
 * @param sd: Player leaving
 * @param lista: List of players in queue data
 * @param listb: List of players in second queue data
 * @return True on success or false otherwise
 */
static bool bg_queue_leave_sub(struct map_session_data *sd, std::vector<map_session_data *> lista, std::vector<map_session_data *> listb)
{
	if (!sd)
		return false;

	auto list_it = lista.begin();

	while (list_it != lista.end()) {
		struct map_session_data *player = *list_it;

		if (player == sd) {
			if (sd->bg_queue->in_ready_state) {
				sd->bg_queue->accepted_players = 0;
				sd->bg_queue->in_ready_state = false;
				sd->bg_queue_accept_state = false;
			}

			list_it = lista.erase(list_it);

			if (lista.empty() && listb.empty()) { // If there are no players left in the queue, discard it
				auto queue_it = bg_queues.begin();

				while (queue_it != bg_queues.end()) {
					std::shared_ptr<s_battleground_queue> q = *queue_it;

					if (sd->bg_queue == q) {
						if (q->tid_requeue && get_timer(q->tid_requeue)) {
							delete_timer(q->tid_requeue, bg_on_ready_loopback);
							q->tid_requeue = 0;
						}

						queue_it = bg_queues.erase(queue_it);
					}
				}
			}

			clif_displaymessage(sd->fd, msg_txt(sd, 340)); // You have been removed from the battlgrounds queue and can't join again for a minute.
			sc_start(nullptr, &sd->bl, SC_ENTRY_QUEUE_APPLY_DELAY, 100, 1, 60000);
			sd->bg_queue = nullptr;
			return true;
		}
	}

	return false;
}

/**
 * Leave a Battleground queue
 * @param sd: Player data
 * @return True on success or false otherwise
 */
bool bg_queue_leave(struct map_session_data *sd)
{
	if (!sd || !sd->bg_queue)
		return false;

	if (!bg_queue_leave_sub(sd, sd->bg_queue->teama_members, sd->bg_queue->teamb_members) && !bg_queue_leave_sub(sd, sd->bg_queue->teamb_members, sd->bg_queue->teama_members)) {
		ShowError("bg_queue_leave: Fatal error. Couldn't find player %s in battlegrounds queue.\n", sd->status.name);
		return false;
	} else
		return true;
}

/**
 * Send packets to all clients in queue to notify them that the battleground is ready to be joined
 * @param name: Battleground name
 * @param queue: Battleground queue
 * @return True on success or false otherwise
 */
bool bg_queue_on_ready(const char *name, std::shared_ptr<s_battleground_queue> queue)
{
	std::shared_ptr<s_battleground_type> bg = bg_search(queue->bg_type);

	if (!bg) {
		ShowError("bg_queue_on_ready: Fatal error. Couldn't find BG type with %d in battlegrounds database.\n", queue->bg_type);
		return false;
	}

	queue->accepted_players = 0; // Reset the counter just in case.

	if (queue->teama_members.size() != queue->required_players || queue->teamb_members.size() != queue->required_players)
		return false; // Return players to the queue and stop reapplying the timer

	struct s_battleground_map *bgmap = nullptr;

	for (auto &it : bg->maps) {
		if (!it.isReserved) {
			it.isReserved = true;
			bgmap = &it;
			queue->map = it;
			break;
		}
	}

	if (!bgmap) { // All the battleground maps are reserved. Set a timer to check for an open battleground every 10 seconds.
		queue->tid_requeue = add_timer(gettick() + 10000, bg_on_ready_loopback, 0, (intptr_t)queue.get());
		return false;
	}

	queue->in_ready_state = true;
	queue->tid_expire = add_timer(gettick() + 20000, bg_on_ready_expire, 0, (intptr_t)queue.get());

	for (const auto &sd : queue->teama_members)
		clif_bg_queue_lobby_notify(name, sd);

	for (const auto &sd : queue->teamb_members)
		clif_bg_queue_lobby_notify(name, sd);

	return true;
}

/**
 * Update the Battleground queue when the player accepts the invite
 * @param queue: Battleground queue
 * @param sd: Player data
 */
void bg_queue_on_accept_invite(std::shared_ptr<s_battleground_queue> queue, struct map_session_data *sd)
{
	nullpo_retv(sd);

	sd->bg_queue_accept_state = true;
	queue->accepted_players++;
	clig_bg_queue_ack_lobby(true, map_mapid2mapname(queue->map.mapid), map_mapid2mapname(queue->map.mapid), sd);

	if (queue->accepted_players == queue->required_players * 2)
		bg_queue_start_battleground(queue);
}

/**
 * Begin the Battleground from the given queue
 * @param queue: Battleground queue
 * @return True on success or false otherwise
 */
int bg_queue_start_battleground(std::shared_ptr<s_battleground_queue> queue)
{
	if (queue->tid_expire && get_timer(queue->tid_expire)) {
		delete_timer(queue->tid_expire, bg_on_ready_expire);
		queue->tid_expire = 0;
	}

	std::shared_ptr<s_battleground_type> bg = bg_search(queue->bg_type);

	if (!bg) {
		queue->map.isReserved = false;
		queue->map = {};
		ShowError("bg_queue_start_battleground: Fatal error. Could not find battleground with ID: %d in database.\n", queue->bg_type);
		return false;
	}

	uint16 map_idx = map_id2index(queue->map.mapid);
	int bg_team_1 = bg_create(map_idx, &queue->map.team1);
	int bg_team_2 = bg_create(map_idx, &queue->map.team2);

	for (const auto &sd : queue->teama_members) {
		sd->bg_queue = nullptr;
		sd->bg_queue_accept_state = false;
		clif_bg_queue_entry_init(sd);
		bg_team_join(bg_team_1, sd, true);
	}

	for (const auto &sd : queue->teamb_members) {
		sd->bg_queue = nullptr;
		sd->bg_queue_accept_state = false;
		clif_bg_queue_entry_init(sd);
		bg_team_join(bg_team_2, sd, true);
	}

	mapreg_setreg(add_str(queue->map.team1.bg_id_var.c_str()), bg_team_1);
	mapreg_setreg(add_str(queue->map.team2.bg_id_var.c_str()), bg_team_2);
	npc_event_do(queue->map.bgcallscript.c_str());
	queue->teama_members.clear();
	queue->teamb_members.clear();

	auto queue_it = bg_queues.begin();

	while (queue_it != bg_queues.end()) {
		if (*queue_it == queue)
			queue_it = bg_queues.erase(queue_it);
	}

	return true;
}

/**
 * Initialize the Battleground data
 */
void do_init_battleground(void)
{
	if (battle_config.feature_bgqueue)
		battleground_db.load();

	add_timer_func_list(bg_send_xy_timer, "bg_send_xy_timer");
	add_timer_func_list(bg_on_ready_loopback, "bg_on_ready_loopback");
	add_timer_func_list(bg_on_ready_expire, "bg_on_ready_expire");
	add_timer_interval(gettick() + battle_config.bg_update_interval, bg_send_xy_timer, 0, 0, battle_config.bg_update_interval);
}

/**
 * Clear the Battleground data from memory
 */
void do_final_battleground(void)
{
}
