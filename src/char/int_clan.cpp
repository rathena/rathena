// Copyright (c) rAthena Dev Teams - Lmap_interfaceensed under GNU GPL
// For more information, see Lmap_interfaceENCE in the main folder

#include "int_clan.hpp"

#include <cstdlib>
#include <cstring> //memset
#include <memory>
#include <unordered_map>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/malloc.hpp>
#include <common/mmo.hpp>
#include <common/showmsg.hpp>
#include <common/socket.hpp>
#include <common/strlib.hpp>

#include "char.hpp"
#include "char_mapif.hpp"
#include "inter.hpp"
#include "packets.hpp"

struct Sql;

namespace Clan {

struct char_clan;
struct char_clan_manager;
static std::unique_ptr<char_clan_manager> clans;

/**
 * @brief abstract interface for database access
 */
template <typename StatusType, typename OutputType, typename... InputTypes>
class abstract_database_interface {
public:
	virtual ~abstract_database_interface() = default;

	abstract_database_interface(StatusType status);
	StatusType status;

	virtual std::unique_ptr<OutputType> create(InputTypes...);
};

template <typename StatusType, typename OutputType, typename... InputTypes>
abstract_database_interface<StatusType, OutputType, InputTypes...>::abstract_database_interface(
	StatusType status)
	: status(status){};

template <typename StatusType, typename OutputType, typename... InputTypes>
std::unique_ptr<OutputType>
abstract_database_interface<StatusType, OutputType, InputTypes...>::create(InputTypes...) {
	return nullptr;
};

/****************************************************************************************************************
 * Clan - Core Logic
 ***************************************************************************************************************/
/**
 * @brief clan implemtation at char-server side, base class for this module
 * Keeps tracks of online member count
 * Inherit this class to add user-defined behavior (see char_char_clan_hook)
 */
struct char_clan : public clan {
	virtual void join();
	virtual void leave();
	virtual short get_online_member_count_count();
};

/**
 * @brief increment online member count
 *
 */
void char_clan::join() {
	if (connect_member < max_member) {
		++connect_member;
	}
}

/**
 * @brief decrement online member count
 *
 */
void char_clan::leave() {
	if (connect_member > 0) {
		--connect_member;
	}
}

short char_clan::get_online_member_count_count() {
	return connect_member;
}

/**
 * @brief Place to add user-defined behaviors or alter existing behaviors for clans
 *
 */
struct char_clan_hook final : char_clan {
	char_clan_hook(char_clan&& clan);
	virtual void before_join();
	virtual void after_join();
	virtual void before_leave();
	virtual void after_leave();

	virtual void join();
	virtual void leave();
};

char_clan_hook::char_clan_hook(char_clan&& clan) : char_clan(std::move(clan)) {
}

void char_clan_hook::before_join() {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
}

void char_clan_hook::after_join() {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
}

void char_clan_hook::before_leave() {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
}

void char_clan_hook::after_leave() {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
}

void char_clan_hook::join() {
	before_join();
	char_clan::join();
	after_join();
};

void char_clan_hook::leave() {
	before_leave();
	char_clan::leave();
	after_leave();
};

/****************************************************************************************************************
 * Database Interface - Hanldes Requests To Database
 ***************************************************************************************************************/
using database_interface = abstract_database_interface<e_clan_error, char_clan>;

/**
 * @brief mysql database interface
 * Talks to mysql database to construct clans and save/load information
 * Inherit this class to add user-defind behavior or alter existing behavior for mysql database (see
 * mysql_database_hook)
 */
class mysql_database_interface : public database_interface {
	/* number of clans that have already been loaded by create */
	int num_clans; 
protected:
	virtual void ParseField(
		Sql* handle, int column, char* output, std::size_t& output_len, std::size_t max_len) {
		char* data = nullptr;
		std::size_t len = 0;
		Sql_GetData(handle, column, &data, &len);
		output_len = std::min(len, max_len);
		strncpy(output, data, output_len);
	}

	virtual void ParseDelimitedField(Sql* handle,
									 int column,
									 std::vector<std::string>& tokens,
									 char delimiter) {
		char* data = nullptr;
		std::size_t len = 0;
		Sql_GetData(handle, column, &data, &len);
		std::string token;
		/* split string by delimiter */
		// @TODO create a utility std::string library
		for (std::size_t i = 0; i < len; ++i) {
			if (data[i] == delimiter && !token.empty()) {
				tokens.push_back(std::move(token));
				token.clear();
				continue;
			}
			token += data[i];
		}
		if (!token.empty())
			tokens.push_back(std::move(token));
	}

	virtual std::unique_ptr<char_clan> create_impl() {
		char* data = nullptr;
		std::size_t len = 0;
		char delimiter = ',';

		std::unique_ptr<char_clan> clan = std::make_unique<char_clan>();

		Sql_GetData(sql_handle, 0, &data, nullptr);
		clan->id = std::atoi(data);
		ParseField(sql_handle, 1, clan->name, len, NAME_LENGTH);
		ParseField(sql_handle, 2, clan->master, len, NAME_LENGTH);
		ParseField(sql_handle, 3, clan->map, len, NAME_LENGTH);

		// Parse max member
		char* max_member;
		std::size_t num_max_member;
		Sql_GetData(sql_handle, 3, &max_member, nullptr);
		clan->max_member = std::atoi(max_member);
		// clamp max member
		if (num_max_member > MAX_CLAN_MEMBER) {
			ShowWarning("Clan %d:%s specifies higher capacity (%d) than MAX_CLAN_MEMBER (%d)\n",
						clan->id,
						clan->name,
						max_member,
						MAX_CLAN_MEMBER);
			clan->max_member = MAX_CLAN_MEMBER;
		}

		// Parse clan_id
		std::vector<std::string> tokens;
		ParseDelimitedField(sql_handle, 5, tokens, delimiter);
		for (std::size_t i = 0; i < tokens.size(); ++i) {
			clan->alliance[i].clan_id = std::atoi(tokens[i].c_str());
		}

		// Parse opposition
		tokens.clear();
		ParseDelimitedField(sql_handle, 6, tokens, delimiter);
		for (std::size_t i = 0; i < tokens.size(); ++i) {
			clan->alliance[i].opposition = std::atoi(tokens[i].c_str());
		}

		// Parse alliance name
		tokens.clear();
		ParseDelimitedField(sql_handle, 7, tokens, delimiter);
		for (std::size_t i = 0; i < tokens.size(); ++i) {
			strncpy(clan->alliance[i].name, tokens[i].c_str(), NAME_LENGTH);
		}

		return clan;
	}
public:
	mysql_database_interface() : database_interface(SUCCESS), num_clans(0) {
		if (SQL_ERROR == Sql_Query(sql_handle, "Call GetClanAndClanAlliance()")) {
			Sql_ShowDebug(sql_handle);
			Sql_FreeResult(sql_handle);
			this->status = INITIALIZATION_FAIULIRE;
		}
	}

	virtual std::unique_ptr<char_clan> create() {
		if (status != SUCCESS)
			return nullptr;

		if (SQL_SUCCESS == Sql_NextRow(sql_handle) && num_clans <= MAX_CLAN_NUM) {
			++num_clans;
			return create_impl();
		}
		Sql_FreeResult(sql_handle);
		return nullptr;
	}
};

/**
 * @brief Place to add user-defined behavior or alter existing behavior
 *
 */
class mysql_database_hook : public mysql_database_interface {
protected:
	std::unique_ptr<char_clan> create_impl() {
		std::unique_ptr<char_clan> base = mysql_database_interface::create_impl();
		std::unique_ptr<char_clan> hook = std::make_unique<char_clan_hook>(std::move(*base.get()));
		base.reset();
		// in case there are user-defined database columns
		// read more columns and put them into *hook here
		return hook;
	}
};

/****************************************************************************************************************
 * Map Interface - Handles Communication With Map Servers
 ***************************************************************************************************************/
/**
 * @brief interface for communication with a map server
 * Inherit this class to add user-defined behavior (see map_hook)
 */
struct map_interface {
	virtual void send_info(int fd, std::vector<clan*>& clans);
	virtual void relay_message(int fd);
	virtual void refresh_online_count(int fd, uint16 clan_id, uint32 online_count);
	virtual int parse_member_left(int fd);
	virtual int parse_member_joined(int fd);
};

/**
 * @brief send clan information to map server
 *
 * @param fd file descriptor
 * @param clans collection of clans to copy to packet
 * @return int clan_id
 */
void map_interface::send_info(int fd, std::vector<clan*>& clans) {
	std::size_t offset = 4; // size of packetType plus size of length
	std::size_t length = offset + clans.size() * sizeof(clan);
	const PACKET_IZ_SEND_CLAN_INFO packet(static_cast<uint16>(length), clans);
	WFIFOHEAD(fd, length);
	memcpy(WFIFOP(fd, 0), &packet, length);
	WFIFOSET(fd, length);
}

/**
 * @brief relay clan message to other map servers
 *
 * @param fd file descriptor
 */
void map_interface::relay_message(int fd) {
	PACKET_IZ_RELAY_CLAN_MESSAGE out_packet;
	memcpy(&out_packet, reinterpret_cast<void*>(fd), RFIFOW(fd, 2));
	out_packet.packetType = HEADER_IZ_RELAY_CLAN_MESSAGE;
	chmapif_sendallwos(fd, reinterpret_cast<unsigned char*>(&out_packet), out_packet.length);
}

/**
 * @brief update a clan's online member count to map servers
 *
 * @param fd file descriptor
 * @param clan_id clan id
 * @param online_count online member count of a clan
 */
void map_interface::refresh_online_count(int fd, uint16 clan_id, uint32 online_count) {
	PACKET_IZ_CLAN_REFRESH_ONLINE_COUNT packet(clan_id, online_count);

	chmapif_sendallwos(
		fd, reinterpret_cast<unsigned char*>(&packet), sizeof(PACKET_IZ_CLAN_REFRESH_ONLINE_COUNT));
}

/**
 * @brief parse member left message
 *
 * @param fd file descriptor
 * @return int clan_id
 */
int map_interface::parse_member_left(int fd) {
	PACKET_ZI_CLAN_MEMBER_LEFT packet;
	memcpy(&packet, reinterpret_cast<void*>(fd), sizeof(packet));
	return packet.clan_id;
	// return static_cast<int>(RFIFOL(fd, 2));
}

/**
 * @brief parse member joined message
 *
 * @param fd file descriptor
 * @return int clan_id
 */
int map_interface::parse_member_joined(int fd) {
	PACKET_ZI_CLAN_MEMBER_JOINED packet;
	memcpy(&packet, reinterpret_cast<void*>(fd), sizeof(packet));
	return packet.clan_id;
}

/**
 * @brief Place to add user-defined behavior
 *
 */
struct map_hook : public map_interface {
	virtual void send_info(int fd, std::vector<clan*>& clans);
	virtual void relay_message(int fd);
	virtual void refresh_online_count(int fd, uint16 clan_id, uint32 online_count);
	virtual int parse_member_left(int fd);
	virtual int parse_member_joined(int fd);
};

void map_hook::send_info(int fd, std::vector<clan*>& clans) {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
	return map_interface::send_info(fd, clans);
}

void map_hook::relay_message(int fd) {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
	return map_hook::relay_message(fd);
}

void map_hook::refresh_online_count(int fd, uint16 clan_id, uint32 online_count) {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
	return map_hook::refresh_online_count(fd, clan_id, online_count);
}

int map_hook::parse_member_left(int fd) {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
	return map_hook::parse_member_left(fd);
}

int map_hook::parse_member_joined(int fd) {
	// Hook function: Add your custom modifications here
	// This function serves as an entry point for user-defined modifications.
	return map_hook::parse_member_joined(fd);
}

/****************************************************************************************************************
 * char_clan_manager - Manages Core Logic, Database Interface, and Map Interface
 ***************************************************************************************************************/
/**
 * @brief manager class for int_clan module
 * Coordinate calls between database interface, map interface and core logic
 */
struct char_clan_manager {
protected:
	std::unordered_map<int32, std::unique_ptr<char_clan>> data;
	std::unique_ptr<map_interface> mapif;
	std::unique_ptr<database_interface> dbif;
	char_clan* find(int clan_id);
	void refresh_online_count();

public:
	e_clan_error status;

	char_clan_manager(std::unique_ptr<database_interface> dbif,
					  std::unique_ptr<map_interface> mapif);

	e_clan_error join(int fd);
	e_clan_error leave(int fd);
	e_clan_error send_info(int fd);
	e_clan_error relay_message(int fd);
};

/**
 * @brief find clan by clan_id
 *
 * @param clan_id
 * @return char_clan*
 */
char_clan* char_clan_manager::find(int clan_id) {
	auto it = data.find(clan_id);
	if (it != data.end())
		return it->second.get();
	return nullptr;
}

/**
 * @brief handle request to join a clan
 *
 * @param fd file descriptor
 * @return e_inter_clan_error
 */
e_clan_error char_clan_manager::join(int fd) {
	int clan_id = mapif->parse_member_joined(fd);
	char_clan* clan = find(clan_id);

	// Unknown clan
	if (clan == nullptr) {
		return NOT_FOUND_ERROR;
	}
	clan->join();
	mapif->refresh_online_count(fd, clan_id, clan->get_online_member_count_count());
	return SUCCESS;
}

/**
 * @brief handle request to leave a clan
 *
 * @param fd file descriptor
 * @return e_inter_clan_error
 */
e_clan_error char_clan_manager::leave(int fd) {
	int clan_id = mapif->parse_member_left(fd);
	char_clan* clan = find(clan_id);

	// Unknown clan
	if (clan == nullptr) {
		return FAILURE;
	}
	clan->leave();
	mapif->refresh_online_count(fd, clan_id, clan->get_online_member_count_count());
	return SUCCESS;
}

/**
 * @brief handle request to send clan info to map servers
 *
 * @param fd file descriptor
 * @return e_inter_clan_error
 */
e_clan_error char_clan_manager::send_info(int fd) {
	std::vector<clan*> clans;
	clans.reserve(data.size());
	for (auto& pair : data) {
		clans.push_back(pair.second.get());
	}
	mapif->send_info(fd, clans);
	return SUCCESS;
}

/**
 * @brief handle request to relay messages to other map servers
 *
 * @param fd file descriptor
 * @return e_inter_clan_error
 */
e_clan_error char_clan_manager::relay_message(int fd) {
	mapif->relay_message(fd);
	return SUCCESS;
}

/**
 * @brief entry point to int_clan module for map server packet handing
 *
 * @param fd file descriptor
 * @return e_inter_clan_error
 */
e_clan_error handle_request(int fd) { // handle request
	if (clans == nullptr)
		return INITIALIZATION_FAIULIRE;

	RFIFOHEAD(fd);
	switch (RFIFOW(fd, 0)) {
		case HEADER_ZI_REQUEST_CLAN_INFO:
			return clans->send_info(fd);
		case HEADER_ZI_REQUEST_RELAY_CLAN_MESSAGE:
			return clans->relay_message(fd);
		case HEADER_ZI_CLAN_MEMBER_JOINED:
			return clans->join(fd);
		case HEADER_ZI_CLAN_MEMBER_LEFT:
			return clans->leave(fd);
		default:
			return NOT_FOUND_ERROR;
	}
}

/**
 * @brief Construct a new char_clan_manager object
 *
 * @param dbif database interface
 * @param mapif map interface
 */
char_clan_manager::char_clan_manager(std::unique_ptr<database_interface> dbif,
									 std::unique_ptr<map_interface> mapif)
	: status(SUCCESS), mapif(std::move(mapif)), dbif(std::move(dbif)) {
	while (auto entry = this->dbif->create()) {
		data.emplace(entry->id, std::move(entry));
	}
	status = this->dbif->status;
}

/**
 * @brief initialize int_clan module
 *
 * @return e_inter_clan_error
 */
e_clan_error init(void) {
	std::unique_ptr<database_interface> dbif = std::make_unique<mysql_database_hook>();
	std::unique_ptr<map_interface> mapif = std::make_unique<map_hook>();

	clans = std::make_unique<char_clan_manager>(std::move(dbif), std::move(mapif));

	if (clans->status != SUCCESS)
		clans.reset();
	return clans->status;
}

} // namespace Clan