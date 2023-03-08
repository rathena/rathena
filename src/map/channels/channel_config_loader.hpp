// Copyright (C) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHANNEL_CONFIG_LOADER_HPP
#define CHANNEL_CONFIG_LOADER_HPP

#include <common/database.hpp>
#include "channel.hpp"

// forward declare ChannelDatabase
class ChannelDatabase;

class ChannelConfigLoader : public YamlDatabase {
   public:
	ChannelConfigLoader(ChannelDatabase& db) : YamlDatabase("CHANNEL_CONF", 1), db_(db) {}

	const std::string getDefaultLocation() override;
	bool parseBody(const ryml::NodeRef& body) override;
	bool parsePrivate(const ryml::NodeRef& node);
	bool parsePublicNode(const ryml::NodeRef& node);
	bool parseOptFlag(const ryml::NodeRef& node, const char* name, enum e_channel_options opt,
					  uint16& opts);

	void clear() override{};
	uint64 parseBodyNode(const ryml::NodeRef& node) override { return 0; };
	
   private:
	ChannelDatabase& db_;
};

#endif /* CHANNEL_CONFIG_LOADER_HPP */