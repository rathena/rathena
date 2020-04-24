// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef BATTLEGROUND_HPP
#define BATTLEGROUND_HPP

#include <memory>
#include <string>
#include <vector>

#include "../common/cbasetypes.hpp"
#include "../common/database.hpp"
#include "../common/mmo.hpp" // struct party

#define MAX_BG_MEMBERS 30

struct s_battleground_member_data {
	unsigned short x, y;
	struct map_session_data *sd;
	unsigned afk : 1;
	struct point entry_point; ///< Battleground queue entry point
};

struct s_battleground_data {
	int id; ///< Battleground ID
	std::vector<s_battleground_member_data> members; ///< List of players in battleground
	struct point cemetery; ///< Respawn point for players who die
	std::string logout_event; ///< NPC Event to call on log out events
	std::string die_event; ///< NPC Event to call on death events
};

struct s_battleground_team {
	int16 warp_x, warp_y; ///< Team respawn coordinates
	std::string quit_event, ///< Team NPC Event to call on log out events
		death_event, ///< Team NPC Event to call on death events
		bg_id_var; ///< Team NPC variable name
};

struct s_battleground_map {
	int id; ///< Battleground ID
	int16 mapid; ///< ID of the map
	s_battleground_team team1, team2; ///< Team data
	std::string bgcallscript; ///< Script to be called when players join the battleground
	bool isReserved; ///< Reserve BG maps that are used so that the system won't create multiple BG instances on the same map
};

/// Battlegrounds client interface queue system [MasterOfMuppets]
struct s_battleground_queue {
	int id; ///< Battlegrounds database ID
	std::vector<map_session_data *> teama_members; ///< List of members on team A
	std::vector<map_session_data *> teamb_members; ///< List of members on team B
	int required_players; ///< Amount of players required on each side to start
	int max_players; ///< Maximum amount of players on each side
	int accepted_players; ///< Amount of players who accepted the offer to enter the battleground
	bool in_ready_state; ///< Is this BG queue waiting for players to enter the BG?
	int tid_expire; ///< Timer ID associated with the time out at the ready to enter window
	int tid_start; ///< Timer ID associated with the start delay
	int tid_requeue; ///< Timer ID associated with requeuing this group if all BG maps are reserved
	s_battleground_map *map; ///< Map this BG queue has been assigned to
};

struct s_battleground_type {
	int id; ///< Battlegrounds database ID
	std::string name; ///< Name of the battleground type
	int required_players; ///< Amount of players required on each side to start
	int max_players; ///< Maximum amount of players on each side
	int min_lvl; ///< Minimum level to participate in this battleground type
	int max_lvl; ///< Maximum level to participate in this battleground type
	std::vector<s_battleground_map> maps; ///< List of battleground locations
	uint32 deserter_time; ///< Amount of time a player is marked deserter (seconds)
	uint32 start_delay; ///< Amount of time before the start message is sent to players (seconds)
};

/// Enum of responses when applying for a Battleground
enum e_bg_queue_apply_ack : uint16 {
	BG_APPLY_NONE = 0,
	BG_APPLY_ACCEPT, ///< Accept
	BG_APPLY_QUEUE_FINISHED, ///< Queuing has finished
	BG_APPLY_INVALID_NAME, ///< Invalid name of Battleground
	BG_APPLY_INVALID_APP, ///< Invalid application
	BG_APPLY_PLAYER_COUNT, ///< Too many players in party/guild
	BG_APPLY_PLAYER_LEVEL, ///< Level too low/high
	BG_APPLY_DUPLICATE, ///< Duplicate application
	BG_APPLY_RECONNECT, ///< Reconnect then apply
	BG_APPLY_PARTYGUILD_LEADER, ///< Only party/guild leader can apply
	BG_APPLY_PLAYER_CLASS, ///< Your class can't apply
};

/// Enum of script command bg_info types
enum e_bg_info : uint16 {
	BG_INFO_ID = 0,
	BG_INFO_REQUIRED_PLAYERS,
	BG_INFO_MAX_PLAYERS,
	BG_INFO_MIN_LEVEL,
	BG_INFO_MAX_LEVEL,
	BG_INFO_MAPS,
	BG_INFO_DESERTER_TIME,
};

class BattlegroundDatabase : public TypesafeYamlDatabase<uint32, s_battleground_type> {
public:
	BattlegroundDatabase() : TypesafeYamlDatabase("BATTLEGROUND_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const YAML::Node& node);
};

extern BattlegroundDatabase battleground_db;
extern std::unordered_map<int, std::shared_ptr<s_battleground_data>> bg_team_db;

std::shared_ptr<s_battleground_type> bg_search_name(const char *name);
void bg_send_dot_remove(struct map_session_data *sd);
int bg_team_get_id(struct block_list *bl);
struct map_session_data *bg_getavailablesd(s_battleground_data *bg);

bool bg_queue_reservation(const char *name, bool state);
#define bg_queue_reserve(name) (bg_queue_reservation(name, true))
#define bg_queue_unbook(name) (bg_queue_reservation(name, false))

int bg_create(uint16 mapindex, s_battleground_team* team);
bool bg_team_join(int bg_id, struct map_session_data *sd, bool is_queue);
bool bg_team_delete(int bg_id);
int bg_team_leave(struct map_session_data *sd, bool quit, bool deserter);
bool bg_team_warp(int bg_id, unsigned short mapindex, short x, short y);
bool bg_player_is_in_bg_map(struct map_session_data *sd);
bool bg_queue_check_joinable(std::shared_ptr<s_battleground_type> bg, struct map_session_data *sd, const char *name);
void bg_queue_join_party(const char *name, struct map_session_data *sd);
void bg_queue_join_guild(const char *name, struct map_session_data *sd);
void bg_queue_join_multi(const char *name, struct map_session_data *sd, std::vector<map_session_data *> list);
bool bg_queue_leave(struct map_session_data *sd);
bool bg_queue_on_ready(const char *name, std::shared_ptr<s_battleground_queue> queue);
void bg_queue_on_accept_invite(std::shared_ptr<s_battleground_queue> queue, struct map_session_data *sd);
void bg_queue_start_battleground(s_battleground_queue *queue);
bool bg_member_respawn(struct map_session_data *sd);
void bg_send_message(struct map_session_data *sd, const char *mes, int len);

void do_init_battleground(void);
void do_final_battleground(void);

#endif /* BATTLEGROUND_HPP */
