#ifndef CHANNEL_CONFIG_HPP
#define CHANNEL_CONFIG_HPP

#include <unordered_map>

#include <common/cbasetypes.hpp>

#include "channel.hpp"

class ChannelConfig {
public:
	std::unordered_map<std::string, uint32> colors;  // List of available colors in RGB format

	// Private channel default configs
	struct {
		uint16 opt;					// Options @see enum Channel_Opt
		unsigned long color;		// Default color
		unsigned int msg_delay;			// Message delay
		unsigned short max_member;	// Max member for each channel
		unsigned allow : 1;			// Allow private channel creation?
	} private_channel;

	struct Channel map_tmpl;   // Map channel default config
	struct Channel ally_tmpl;  // Alliance channel default config

	uint32 getColor(const std::string& color_str) const;

	ChannelConfig() {
		colors["Default"] = 0xFF'FF'FF;
	}
};

#endif /* CHANNEL_CONFIG_HPP */