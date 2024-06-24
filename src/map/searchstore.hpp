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
enum e_searchstore_failure : uint8
{
	SSI_FAILED_NOTHING_SEARCH_ITEM = 0,  // "No matching stores were found."
	SSI_FAILED_OVER_MAXCOUNT = 1,  // "There are too many results. Please enter more detailed search term."
	SSI_FAILED_SEARCH_CNT = 2,  // "You cannot search anymore."
	SSI_FAILED_LIMIT_SEARCH_TIME = 3,  // "You cannot search yet."
	SSI_FAILED_SSILIST_CLICK_TO_OPEN_STORE = 4,  // "No sale (purchase) information available."
};

/// Search type constants
enum e_searchstore_searchtype : uint8
{
	SEARCHTYPE_VENDING = 0,
	SEARCHTYPE_BUYING_STORE = 1,
};

/// Search effect constants
enum e_searchstore_effecttype : uint8
{
	SEARCHSTORE_EFFECT_NORMAL = 0,
	SEARCHSTORE_EFFECT_REMOTE = 1,
	SEARCHSTORE_EFFECT_MAX
};

/// information about the search being performed
struct s_search_store_search {
	map_session_data* search_sd;  // sd of the searching player
	const struct PACKET_CZ_SEARCH_STORE_INFO_item* itemlist;
	const struct PACKET_CZ_SEARCH_STORE_INFO_item* cardlist;
	unsigned int item_count;
	unsigned int card_count;
	unsigned int min_price;
	unsigned int max_price;
};

struct s_search_store_info_item {
	int store_id;
	uint32 account_id;
	char store_name[MESSAGE_SIZE];
	t_itemid nameid;
	unsigned short amount;
	unsigned int price;
	t_itemid card[MAX_SLOTS];
	unsigned char refine;
	uint8 enchantgrade;
};

struct s_search_store_info {
	std::vector<std::shared_ptr<s_search_store_info_item>> items;
	unsigned int pages;  // amount of pages already sent to client
	uint8 uses;
	int remote_id;
	time_t nextquerytime;
	e_searchstore_effecttype effect;  // 0 = Normal (display coords), 1 = Remote (remotely open store)
	e_searchstore_searchtype type;  // 0 = Vending, 1 = Buying Store
	int16 mapid;
	bool open;
};

bool searchstore_open(map_session_data* sd, uint8 uses, e_searchstore_effecttype effect, int16 mapid);
void searchstore_query(map_session_data* sd, e_searchstore_searchtype type, unsigned int min_price, unsigned int max_price, const struct PACKET_CZ_SEARCH_STORE_INFO_item* itemlist, unsigned int item_count, const struct PACKET_CZ_SEARCH_STORE_INFO_item* cardlist, unsigned int card_count);
bool searchstore_querynext(map_session_data* sd);
void searchstore_next(map_session_data* sd);
void searchstore_clear(map_session_data* sd);
void searchstore_close(map_session_data* sd);
void searchstore_click(map_session_data* sd, uint32 account_id, int store_id, t_itemid nameid);
bool searchstore_queryremote(map_session_data* sd, uint32 account_id);
void searchstore_clearremote(map_session_data* sd);

#endif /* SEARCHSTORE_HPP */
