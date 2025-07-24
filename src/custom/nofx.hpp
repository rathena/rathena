// src/custom/nofx.hpp

#ifndef NOFX_HPP
#define NOFX_HPP

#include <unordered_set>
#include <unordered_map>

#include "../common/cbasetypes.hpp"

class custom_effects_data {
private:
	std::unordered_set<uint16> hidden_skill_ids;

public:
	void set_skill_effect_hidden(uint16 skill_id, bool hide) {
		if (hide) {
			hidden_skill_ids.insert(skill_id);
		}
		else {
			hidden_skill_ids.erase(skill_id);
		}
	}

	bool is_skill_hidden(uint16 skill_id) const {
		return hidden_skill_ids.count(skill_id) > 0;
	}

	void clear_all_hidden_skills() {
		hidden_skill_ids.clear();
	}
};

class CustomEffectManager {
private:

	std::unordered_map<uint32, custom_effects_data> player_effects;

	CustomEffectManager() {}

	CustomEffectManager(const CustomEffectManager&) = delete;
	CustomEffectManager& operator=(const CustomEffectManager&) = delete;

public:
	static CustomEffectManager* get_instance() {
		static CustomEffectManager instance; 
		return &instance;
	}

	custom_effects_data* get_effects(uint32 char_id) {
		return &player_effects[char_id];
	}

	void remove_effects(uint32 char_id) {
		player_effects.erase(char_id);
	}

	static bool clif_is_skill_hidden_for_viewer(uint32 viewer_char_id, uint16 skill_id) {
		CustomEffectManager* instance = CustomEffectManager::get_instance();
		custom_effects_data* ce_data = instance->get_effects(viewer_char_id);

		return ce_data->is_skill_hidden(skill_id);
	}
};

#endif
