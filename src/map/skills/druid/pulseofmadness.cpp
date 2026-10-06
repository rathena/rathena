// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "pulseofmadness.hpp"

#include <common/random.hpp>

#include "map/status.hpp"

SkillPulseOfMadness::SkillPulseOfMadness() : StatusSkillImpl(AT_PULSE_OF_MADNESS) {
}

void SkillPulseOfMadness::updateMadness(block_list* src) {
	status_change *sc = status_get_sc(src);

	if (sc == nullptr)
		return;

	status_change_entry *pulse = sc->getSCE(SC_PULSE_OF_MADNESS);

	if (pulse == nullptr)
		return;

	int32 chance = 20 + 10 * (pulse->val1 - 1);

	if (!rnd_chance(chance, 100))
		return;

	t_tick duration = skill_get_time2(AT_PULSE_OF_MADNESS, pulse->val1);

	// Give SC_INSANE* sc (independent of SC_ALPHA_PHASE)
	if (sc->hasSCE(SC_INSANE3)) {
		sc_start(src, src, SC_INSANE3, 100, 1, duration);
		return;
	}

	if (sc->hasSCE(SC_INSANE2)) {
		sc_start(src, src, SC_INSANE3, 100, 1, duration);
		return;
	}

	if (sc->hasSCE(SC_INSANE)) {
		sc_start(src, src, SC_INSANE2, 100, 1, duration);
		return;
	}

	sc_start(src, src, SC_INSANE, 100, 1, duration);
}
