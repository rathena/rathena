// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <unordered_set>

#include <common/cbasetypes.hpp>
#include <common/mmo.hpp>
#include <common/database.hpp>

//namespace rA {

class map_session_data;
struct guild;
struct DBMap;


struct Channel_Config {
	unsigned long *colors;		///< List of available colors
	char **colors_name;			///< Name list of available colors
	unsigned char colors_count;	///< Number of available colors

	/// Private channel default configs
	struct {
		uint16 opt;					 ///< Options @see enum Channel_Opt
		unsigned long color;		 ///< Default color
		unsigned int delay;			 ///< Message delay
		unsigned short max_member;	 ///< Max member for each channel
		unsigned allow : 1;			 ///< Allow private channel creation?
	} private_channel;

	struct Channel map_tmpl;  ///< Map channel default config
	struct Channel ally_tmpl; ///< Alliance channel default config

	bool closing; ///< Server is closing
};
extern struct Channel_Config channel_config;

DBMap* channel_get_db(void);

struct Channel* channel_create(struct Channel *tmp_chan);
struct Channel* channel_create_simple(char *name, char *pass, enum Channel_Type chantype, unsigned int owner);
int channel_delete(struct Channel *channel, bool force);

int channel_join(struct Channel *channel, map_session_data *sd);
int channel_mjoin(map_session_data *sd);
int channel_gjoin(map_session_data *sd, int flag);
int channel_ajoin(struct guild *g);
int channel_clean(struct Channel *channel, map_session_data *sd, int flag);
int channel_pcquit(map_session_data *sd, int type);

unsigned long channel_getColor(const char *color_str);

int channel_send(struct Channel *channel, map_session_data *sd, const char *msg);
void channel_read_config(void);

int channel_chk(char *name, char *pass, int type);
struct Channel* channel_name2channel(char *chname, map_session_data *sd, int flag);
int channel_haspc(struct Channel *channel,map_session_data *sd);
int channel_haspcbanned(struct Channel *channel,map_session_data *sd);
int channel_pc_haschan(map_session_data *sd, struct Channel *channel);
int channel_display_list(map_session_data *sd, const char *option);

void channel_autojoin(map_session_data *sd);
bool channel_pccheckgroup(struct Channel *channel, int group_id);

int channel_pccreate(map_session_data *sd, char *chname, char *pass);
int channel_pcdelete(map_session_data *sd, char *chname);
int channel_pcjoin(map_session_data *sd, char *chname, char *pass);
int channel_pcleave(map_session_data *sd, char *chname);
int channel_pccolor(map_session_data *sd, char *chname, char *color);
int channel_pcbind(map_session_data *sd, char *chname);
int channel_pcunbind(map_session_data *sd);
int channel_pcban(map_session_data *sd, char *chname, char *pname, int flag);
int channel_pckick(map_session_data *sd, char *chname, char *pname);
int channel_pcsetopt(map_session_data *sd, char *chname, const char *option, const char *val);


class ChannelConfigLoader : public YamlDatabase {
public:
	ChannelConfigLoader() : YamlDatabase("CHANNEL_CONF", 1) {}

	const std::string getDefaultLocation() override;
	bool parseBody(const ryml::NodeRef& body) override;
	bool parsePrivate(const ryml::NodeRef& node);
	bool parsePublicNode(const ryml::NodeRef& node);
	bool parseOptFlag(const ryml::NodeRef& node, const char * name, enum e_channel_options opt, uint16 &opts);

	void clear() override {};
	uint64 parseBodyNode(const ryml::NodeRef& node) override {return 0;};
};

void do_init_channel(void);
void do_final_channel(void);

#endif /* CHANNEL_HPP */
