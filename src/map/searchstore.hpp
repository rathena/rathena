// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef SEARCHSTORE_HPP
#define SEARCHSTORE_HPP

#include <memory>
#include <vector>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>

#include "clif.hpp"
#include "map.hpp"

#define SEARCHSTORE_RESULTS_PER_PAGE 10

/// Failure constants for clif functions
enum e_searchstore_failure : uint16
{
	// "No matching stores were found." (1803)
	SSI_FAILED_NOTHING_SEARCH_ITEM = 0,

	// "There are too many results. Please enter more detailed search term." (1784)
	SSI_FAILED_OVER_MAXCOUNT,

	// "You cannot search anymore." (1798)
	SSI_FAILED_SEARCH_CNT,

	// "You cannot search yet." (1800)
	SSI_FAILED_LIMIT_SEARCH_TIME,

	// "No sale (purchase) information available." (1797)
	SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE,
};

/// Search type constants
enum e_searchstore_searchtype : uint16
{
	// Search for vending stores
	SEARCHTYPE_VENDING = 0,

	// Search for buying stores
	SEARCHTYPE_BUYING_STORE,
};

/// Search effect constants
enum e_searchstore_effecttype : uint16
{
	// Displays the coordinates of the store
	SEARCHSTORE_EFFECT_NORMAL = 0,

	// Opens the store remotely
	SEARCHSTORE_EFFECT_REMOTE,

	SEARCHSTORE_EFFECT_MAX
};

/// information about the search being performed
struct s_search_store_search {
	map_session_data* search_sd;  // sd of the searching player
	const struct PACKET_CZ_SEARCH_STORE_INFO_item* itemlist;
	const struct PACKET_CZ_SEARCH_STORE_INFO_item* cardlist;
	uint32 item_count;
	uint32 card_count;
	uint32 min_price;
	uint32 max_price;
};

struct s_search_store_info_item {
	int32 store_id;
	uint32 account_id;
	char store_name[MESSAGE_SIZE];
	t_itemid nameid;
	uint16 amount;
	uint32 price;
	t_itemid card[MAX_SLOTS];
	unsigned char refine;
	uint8 enchantgrade;
};

struct s_search_store_info {
	std::vector<std::shared_ptr<s_search_store_info_item>> items;
	uint32 pages;  // amount of pages already sent to client
	uint16 uses;
	int32 remote_id;
	time_t nextquerytime;
	e_searchstore_effecttype effect;
	e_searchstore_searchtype type;
	int16 mapid;
	bool open;
};

bool searchstore_open(map_session_data& sd, uint16 uses, e_searchstore_effecttype effect, int16 mapid);
void searchstore_query(map_session_data& sd, e_searchstore_searchtype type, uint32 min_price, uint32 max_price, const struct PACKET_CZ_SEARCH_STORE_INFO_item* itemlist, uint32 item_count, const struct PACKET_CZ_SEARCH_STORE_INFO_item* cardlist, uint32 card_count);
bool searchstore_querynext(map_session_data& sd);
void searchstore_next(map_session_data& sd);
void searchstore_clear(map_session_data& sd);
void searchstore_close(map_session_data& sd);
void searchstore_click(map_session_data& sd, uint32 account_id, int32 store_id, t_itemid nameid);
bool searchstore_queryremote(map_session_data& sd, uint32 account_id);
void searchstore_clearremote(map_session_data& sd);

#endif /* SEARCHSTORE_HPP */
