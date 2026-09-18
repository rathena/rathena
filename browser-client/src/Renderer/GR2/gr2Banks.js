/**
 * Renderer/GR2/gr2Banks.js
 *
 * Per-model external animation bank registry (port of the sandbox models.js
 * `banks` field). The main data/model/3dmob/<name>90_<N>.gr2 carries the mesh +
 * embedded standby (anim 0) only; move/attack/dead/damage live in separate bank
 * files data/model/3dmob_bone/<setId>_<suffix>.gr2 (client fcn.00683e40; setId
 * from fcn.006800d0). A model that does not declare a bank (Emperium dead/damage,
 * Treasure Box move/attack) correctly falls to the bind pose on that action.
 *
 * Pure (no GL / IO) so the headless unit tests exercise it directly.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

/**
 * setId -> the external bank suffixes that model ships. setId is the integer <N>
 * suffix of the model filename (guildflag90_1 -> 1, treasurebox_2 -> 2,
 * empelium90_0 -> 0, sguardian90_9 -> 9). Sourced from the sandbox models.js oracle:
 * guardians (7/8/9) ship all four, treasure box (2) ships {dead,damage}, emperium (0)
 * and flag (1) ship none (embedded standby only).
 */
// Keys are strings: setId is captured as a string from the filename regex below, so the
// lookup GR2_BANKS[setId] matches on the string key directly (no JS numeric-key coercion).
const GR2_BANKS = {
	2: ['dead', 'damage'], // treasurebox
	7: ['attack', 'damage', 'dead', 'move'], // kguardian
	8: ['attack', 'damage', 'dead', 'move'], // aguardian
	9: ['attack', 'damage', 'dead', 'move'] // sguardian
	// 0 emperium, 1 flag -> none (embedded standby only)
};

/**
 * bankPathsFor(path) -> [{ name, path }] : the declared external bank files for a
 * model, given its main .gr2 path. Empty when the model ships no external banks.
 * @param {string} path e.g. 'data/model/3dmob/sguardian90_9.gr2'
 */
export function bankPathsFor(path) {
	const match = path.match(/_(\d+)\.gr2$/i);
	const setId = match ? match[1] : null;
	const names = (setId != null && GR2_BANKS[setId]) || [];
	return names.map(name => ({
		name: name,
		path: 'data/model/3dmob_bone/' + setId + '_' + name + '.gr2'
	}));
}
