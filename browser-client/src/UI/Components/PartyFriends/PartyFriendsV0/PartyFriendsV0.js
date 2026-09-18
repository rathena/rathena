/**
 * UI/Components/PartyFriends/PartyFriendsV0/PartyFriendsV0.js
 *
 * Manage interface for parties and friends (classic layout)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import { createPartyFriends } from '../PartyFriendsCommon.js';
import htmlText from './PartyFriendsV0.html?raw';
import cssText from './PartyFriendsV0.css?raw';

/**
 * Storing Requirement
 */
export default createPartyFriends({
	name: 'PartyFriendsV0',
	htmlText,
	cssText
});
