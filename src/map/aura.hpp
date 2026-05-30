// Copyright (c) Pandas Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#pragma once

#include <common/database.hpp>
#include <common/timer.hpp>
#include <common/mmo.hpp>
#include "clif.hpp"

struct s_aura {
	uint32 aura_id = 0;
	std::vector<std::shared_ptr<s_aura_effect>> effects;
};

enum e_aura_special : uint32 {
	AURA_SPECIAL_NOTHING		= 0x0000,	// 普通效果, 没有什么特别的
	AURA_SPECIAL_HIDE_DISAPPEAR = 0x0001	// 当角色使用隐藏技能之后, 这个效果会消失 (等角色恢复显示后, 该效果需要被重新绘制)
};

class AuraDatabase : public TypesafeYamlDatabase<uint32, s_aura> {
public:
	AuraDatabase() : TypesafeYamlDatabase("AURA_DB", 1) {

	}

	const std::string getDefaultLocation();
	uint64 parseBodyNode(const ryml::NodeRef& node) override;
};

extern AuraDatabase aura_db;

void aura_reload(void);
void do_final_aura(void);
void do_init_aura(void);

std::shared_ptr<s_aura> aura_search(uint32 aura_id);
enum e_aura_special aura_special(uint16 effect_id);
TIMER_FUNC(aura_effects_timer);
bool aura_need_hiding(struct block_list* bl, struct block_list* observer_bl = nullptr);
void aura_effects_clear(struct block_list* bl);
void aura_effects_refill(struct block_list* bl);
void aura_refresh_client(struct block_list* bl);
void aura_make_effective(struct block_list* bl, uint32 aura_id, bool pc_saved = true);
void clif_send_auras_single(struct block_list* bl, map_session_data* tsd);
void clif_send_auras(struct block_list* bl, int target, bool ignore_when_hidden, int flag);
