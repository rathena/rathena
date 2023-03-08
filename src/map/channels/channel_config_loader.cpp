// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include <string>

#include <common/showmsg.hpp>
#include <common/strlib.hpp>
#include <common/utils.hpp>


#include "channel_config_loader.hpp"
#include "channel_db.hpp"
#include "../pc_groups.hpp"
#include "../script.hpp"

const std::string ChannelConfigLoader::getDefaultLocation() {
	return std::string(conf_path) + "/channels.yml";
}

bool ChannelConfigLoader::parseBody(const ryml::NodeRef& body) {
	if (this->nodeExists(body, "Private")) {
		parsePrivate(body["Private"]);
	}

	if (this->nodeExists(body, "Public")) {
		for (const auto& node : body["Public"]) {
			parsePublicNode(node);
		}
	}
	return true;
}

bool ChannelConfigLoader::parsePrivate(const ryml::NodeRef& node) {
	auto& conf = db_.getMutChannelConfig().private_channel;

	if (nodeExists(node, "Allow")) {
		bool active;
		if (!asBool(node, "Allow", active))
			return false;

		conf.allow = active;
	}

	if (nodeExists(node, "Color")) {
		uint32 color;
		if (!asUInt32(node, "Color", color))
			return false;
		if (color > 0xFF'FF'FF) {
			invalidWarning(node["Color"],
						   "Invalid private channel color 0x%06x, defaulting to 0xFFFFFF.\n",
						   color);
			color = 0xFF'FF'FF;
		}
		conf.color = RGB2BGR(color);
	}

	if (nodeExists(node, "Delay")) {
		uint32 delay;

		if (!this->asUInt32(node, "Delay", delay))
			return false;

		conf.msg_delay = delay;
	}

	if (nodeExists(node, "MaxMembers")) {
		uint16 members;
		if (!asUInt16(node, "MaxMembers", members))
			return false;
		conf.max_member = members;
	}

	parseOptFlag(node, "SelfAnnounce", CHAN_OPT_SELFANNOUNCE, conf.opt);
	parseOptFlag(node, "JoinAnnounce", CHAN_OPT_JOINANNOUNCE, conf.opt);
	parseOptFlag(node, "LeaveAnnounce", CHAN_OPT_LEAVEANNOUNCE, conf.opt);
	parseOptFlag(node, "ChangeDelay", CHAN_OPT_CHANGEDELAY, conf.opt);
	parseOptFlag(node, "ChangeColor", CHAN_OPT_CHANGECOLOR, conf.opt);
	parseOptFlag(node, "Ban", CHAN_OPT_BAN, conf.opt);
	parseOptFlag(node, "Kick", CHAN_OPT_KICK, conf.opt);
	return true;
}

bool ChannelConfigLoader::parseOptFlag(const ryml::NodeRef& node, const char* name,
									   enum e_channel_options opt, uint16& opts) {
	if (nodeExists(node, name)) {
		bool active;
		if (!asBool(node, name, active))
			return false;
		if (active)
			opts |= opt;
		else
			opts &= ~opt;
	}
	return true;
}

bool ChannelConfigLoader::parsePublicNode(const ryml::NodeRef& node) {
	std::string name;

	if (!asString(node, "Name", name))
		return false;

	ShowInfo("Parsing public node %s\n", name.c_str());

	if (name[0] != '#') {
		invalidWarning(node["Name"], "Channel name %s must begin with '#'.\n", name.c_str());
		return false;
	}

	name.resize(CHAN_NAME_LENGTH);

	ChannelType type = ChannelType::Public;

	if (nodeExists(node, "Type")) {
		std::string type_name;
		if (!asString(node, "Type", type_name))
			return false;
		int64 constant;
		std::string type_constant = "CHAN_TYPE_" + type_name;
		if (!script_get_constant(type_constant.c_str(), &constant)) {
			invalidWarning(node["Type"], "Channel type %s is invalid.\n", type_name.c_str());
			return false;
		}

		if (constant < static_cast<int64>(ChannelType::Public) ||
			constant == static_cast<int64>(ChannelType::Private) ||
			constant > static_cast<int64>(ChannelType::Ally)) {
			invalidWarning(node["Type"], "Channel type %s is not a supported type for Public.\n",
						   type_name.c_str());
			return false;
		}
		type = static_cast<ChannelType>(constant);
	}

	Channel* channel = nullptr;

	switch (type) {
		case ChannelType::Public:
			channel = db_.getChannel(name).get();
			break;
		case ChannelType::Map:
			channel = &db_.getMutChannelConfig().map_tmpl;
			break;
		case ChannelType::Ally:
			channel = &db_.getMutChannelConfig().ally_tmpl;
			break;
		default:
			ShowError("ChannelConfigLoader::parsePublicNode: Type %d not handled\n", type);
			return false;
	}

	bool exists = channel != nullptr;

	Channel tmp_channel{};

	if (!exists) {
		channel = &tmp_channel;
		// let's set a bunch of defaults
		channel->color = 0xFF'FF'FF;
		channel->msg_delay = 1000;
	}

	if (nodeExists(node, "Password")) {
		std::string password;
		if (!asString(node, "Password", password))
			return false;

		password.resize(CHAN_NAME_LENGTH);
		safestrncpy(channel->pass, password.c_str(), CHAN_NAME_LENGTH);
	}

	if (nodeExists(node, "Alias")) {
		std::string alias;
		if (!asString(node, "Alias", alias))
			return false;

		alias.resize(CHAN_NAME_LENGTH);
		safestrncpy(channel->alias, alias.c_str(), CHAN_NAME_LENGTH);
	}

	if (nodeExists(node, "Color")) {
		uint32 color;
		if (!asUInt32(node, "Color", color))
			return 0;

		if (color > 0xFF'FF'FF) {
			invalidWarning(node["Color"], "Invalid channel color 0x%06x, defaulting to 0xFFFFFF\n",
						   color);
			color = 0xFF'FF'FF;
		}
		channel->color = RGB2BGR(color);
	}

	if (nodeExists(node, "Delay")) {
		uint32 delay;
		if (!asUInt32(node, "Delay", delay))
			return 0;

		channel->msg_delay = delay;
	}

	parseOptFlag(node, "Autojoin", CHAN_OPT_AUTOJOIN, channel->opt);
	parseOptFlag(node, "Leave", CHAN_OPT_CANLEAVE, channel->opt);
	parseOptFlag(node, "Chat", CHAN_OPT_CANCHAT, channel->opt);
	parseOptFlag(node, "ChangeColor", CHAN_OPT_CHANGECOLOR, channel->opt);
	parseOptFlag(node, "SelfAnnounce", CHAN_OPT_SELFANNOUNCE, channel->opt);
	parseOptFlag(node, "JoinAnnounce", CHAN_OPT_JOINANNOUNCE, channel->opt);
	parseOptFlag(node, "LeaveAnnounce", CHAN_OPT_LEAVEANNOUNCE, channel->opt);

	if (nodeExists(node, "Groups")) {
		const auto& groupsNode = node["Groups"];
		for (const auto& it : groupsNode) {
			std::string group_name;
			c4::from_chars(it.key(), &group_name);

			auto group = player_group_db.search_groupname(group_name);
			if (!group) {
				invalidWarning(groupsNode, "Invalid group name %s, skipping.\n",
							   group_name.c_str());
				continue;
			}

			bool active;
			if (!asBool(groupsNode, group_name, active))
				return 0;
			if (active)
				channel->groups.insert(group->id);
			else
				channel->groups.erase(group->id);
		}
	}

	if (type == ChannelType::Public && !exists) {
		db_.createChannel(*channel);
	}
	return true;
}
