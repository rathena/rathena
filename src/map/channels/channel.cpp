// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "channel.hpp"

#include <common/utilities.hpp>

#include "../pc.hpp"
#include "channel_db.hpp"

using namespace rathena;

int Channel::clean(map_session_data *sd, int flag) {
	if (!sd)
		return -1;

	if (this == sd->gcbind)
		sd->gcbind = nullptr;

	auto it = std::find_if(sd->channels.begin(), sd->channels.end(), [this](auto &channel) {
		return channel.get() == this;
	});

	if (it != sd->channels.end()) {
		sd->channels.erase(it);
	}

	util::vector_erase_if_exists(users, sd);
	if (users.size() == 0 && !(flag & 1))
		channel_db->deleteChannel(this->name, false);

	return 0;
}