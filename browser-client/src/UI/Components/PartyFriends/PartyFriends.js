/**
 * UI/Components/PartyFriends/PartyFriends.js
 *
 * Manage interface for parties and friends - Version Router
 *
 */

import PartyFriendsV0 from './PartyFriendsV0/PartyFriendsV0.js';
import PartyFriendsV1 from './PartyFriendsV1/PartyFriendsV1.js';

import UIVersionManager from 'UI/UIVersionManager.js';

const publicName = 'PartyFriends';

const versionInfo = {
	default: PartyFriendsV0,
	common: {
		20170524: PartyFriendsV1
	},
	re: {},
	prere: {}
};

const controller = UIVersionManager.getUIController(publicName, versionInfo);

/**
 * Proxy for isGroupMember
 */
controller.isGroupMember = function isGroupMember(name) {
	const ui = controller.getUI();
	return ui && ui.isGroupMember && ui.isGroupMember(name);
};

/**
 * Proxy for onOpenChat1to1
 */
controller.onOpenChat1to1 = function onOpenChat1to1(name) {
	const ui = controller.getUI();
	if (ui && ui.onOpenChat1to1) {
		ui.onOpenChat1to1(name);
	}
};

/**
 * Proxy for toggle
 */
controller.toggle = function toggle() {
	const ui = controller.getUI();
	if (ui && ui.toggle) {
		ui.toggle();
	}
};

export default controller;
