// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef INT_CLAN_HPP
#define INT_CLAN_HPP

namespace Clan {

enum e_clan_error : int {
	SUCCESS = 0,
	FAILURE = 1,
	INITIALIZATION_FAIULIRE = 2,
	NOT_FOUND_ERROR = 3,
};

e_clan_error handle_request(int fd);
e_clan_error init(void);

} // namespace Clan

#endif /* INT_CLAN_HPP */
