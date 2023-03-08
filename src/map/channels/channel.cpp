// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "channel.hpp"

#include <common/utilities.hpp>
#include <common/socket.hpp>

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
	// TODO(vstumpf): Fix this
	// if (users.size() == 0 && !(flag & 1))
	// 	channel_db->deleteChannel(shared_from_this(), false);

	return 0;
}

int Channel::join(map_session_data *sd) {
	if (!sd)
		return -1;

	if (sd->state.autotrade)
		return 0; // fake success
	
	if (hasPC(sd))
		return -2;
	
	if (!pc_has_permission(sd, PC_PERM_CHANNEL_ADMIN) && !checkGroup(sd->group_id)) {
		return -2;
	}

	if (isBanned(sd) == 1) {
		char output[CHAT_SIZE_MAX];
		sprintf(output, msg_txt(sd, 1438), name); // You're currently banned from the '%s' channel.
		clif_displaymessage(sd->fd, output);
		return -3;
	}
	
	if (type == ChannelType::Private && users.size() > channel_db->getChannelConfig().private_channel.max_member) {
		char output[CHAT_SIZE_MAX];
		sprintf(output, msg_txt(sd, 760), name,
				channel_db->getChannelConfig()
					.private_channel
					.max_member);  // You cannot join channel '%s'. Limit of %d has been met.
		clif_displaymessage(sd->fd, output);
		return -4;
	}

	sd->channels.push_back(shared_from_this());

	if (sd->stealth) {
		sd->stealth = false;
	} else if (opt & CHAN_OPT_JOINANNOUNCE) {
		char output[CHAT_SIZE_MAX];
		safesnprintf(output, CHAT_SIZE_MAX, msg_txt(sd, 761), alias, sd->status.name);  // %s %s has joined the channel.
		clif_channel_msg(this, output, color);
	}

	// someone is cheating, we kindly disconnect the bastard
	if (sd->channels.size() > 200) {
		set_eof(sd->fd);
	}


	if (this == sd->gcbind)
		sd->gcbind = nullptr;

	if (std::find_if(sd->channels.begin(), sd->channels.end(), [this](auto &channel) {
		return channel.get() == this;
	}) != sd->channels.end())
		return -1;

	if (std::find(users.begin(), users.end(), sd) != users.end())
		return -1;

	sd->channels.push_back(shared_from_this());
	users.push_back(sd);

	return 0;
}
