/**
 * Preferences/Map.js
 *
 * Map user preferences
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Preferences from 'Core/Preferences.js';

/**
 * Export
 */
export default Preferences.get(
	'Map',
	{
		/**
		 * Display the fog ?
		 *
		 * Toggle by using "/fog" in the chatbox
		 */
		fog: true,

		/**
		 * Display lightmap ?
		 *
		 * Toggle using "/lightmap" in the chatbox
		 */
		lightmap: true,

		/**
		 * Posterize lightmap ?
		 *
		 * Toggle using "/smoothlight" in the chatbox
		 */
		smoothlight: 0,

		/**
		 * Display effects ?
		 *
		 * Toggle using "/effect" in the chatbox
		 */
		effect: true,

		/**
		 * Display minify effects ?
		 *
		 * Toggle using "/mineeffect" in the chatbox
		 */
		mineffect: false,

		/**
		 * Should we display "miss" when monster/player miss an attack ?
		 *
		 * Toggle using "/miss" in the chatbox
		 */
		miss: true,

		/**
		 * Display aura (2) or simplified aura (1) or disable entirely (0)
		 *
		 * Toggle using "/aura" or "/aura2" in the chatbox
		 */
		aura: 1,

		/**
		 * Display different font style ?
		 *
		 * Toggle using "/showname" changes font styles.
		 */
		showname: true
	},
	1.1
);
