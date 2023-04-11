// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder


#include "webutils.hpp"
#include <string>
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>


/**
 * Merge patch into orig recursively
 * if merge_null is true, this operates like json::merge_patch
 * if merge_null is false, then if patch has null, it does not override orig
 * Returns true on success
 */
bool mergeData(nlohmann::json &orig, const nlohmann::json &patch, bool merge_null) {
	if (!patch.is_object()) {
		// then it's a value
		if ((patch.is_null() && merge_null) || (!patch.is_null())) {
			orig = patch;
		}
		return true;
	}

	if (!orig.is_object()) {
		orig = nlohmann::json::object();
	}

	for (auto it = patch.begin(); it != patch.end(); ++it) {
		if (it.value().is_null()) {
			if (merge_null) {
				orig.erase(it.key());
			}
		} else {
			mergeData(orig[it.key()], it.value(), merge_null);
		}
	}
	return true;
}
