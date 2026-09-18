/**
 * Network/PacketLength.js
 *
 * Network Packet Length
 * Manage packets length
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Alison Serafim
 */

// Eagerly import all packet length definitions as ES modules
// (explicit imports for browser compatibility without Vite)
import * as p2003 from './Packets/packets2003_len_main.js';
import * as p2004 from './Packets/packets2004_len_main.js';
import * as p2005 from './Packets/packets2005_len_main.js';
import * as p2006 from './Packets/packets2006_len_main.js';
import * as p2007 from './Packets/packets2007_len_main.js';
import * as p2008 from './Packets/packets2008_len_main.js';
import * as p2009 from './Packets/packets2009_len_main.js';
import * as p2010 from './Packets/packets2010_len_main.js';
import * as p2011 from './Packets/packets2011_len_main.js';
import * as p2012 from './Packets/packets2012_len_main.js';
import * as p2013 from './Packets/packets2013_len_main.js';
import * as p2014 from './Packets/packets2014_len_main.js';
import * as p2015 from './Packets/packets2015_len_main.js';
import * as p2016 from './Packets/packets2016_len_main.js';
import * as p2017 from './Packets/packets2017_len_main.js';
import * as p2018 from './Packets/packets2018_len_main.js';
import * as p2019 from './Packets/packets2019_len_main.js';
import * as p2020 from './Packets/packets2020_len_main.js';
import * as p2021 from './Packets/packets2021_len_main.js';
import * as p2022 from './Packets/packets2022_len_main.js';
import * as p2023 from './Packets/packets2023_len_main.js';
import * as p2024 from './Packets/packets2024_len_main.js';
import * as p2025 from './Packets/packets2025_len_main.js';

const packetModules = {
	'./Packets/packets2003_len_main.js': p2003,
	'./Packets/packets2004_len_main.js': p2004,
	'./Packets/packets2005_len_main.js': p2005,
	'./Packets/packets2006_len_main.js': p2006,
	'./Packets/packets2007_len_main.js': p2007,
	'./Packets/packets2008_len_main.js': p2008,
	'./Packets/packets2009_len_main.js': p2009,
	'./Packets/packets2010_len_main.js': p2010,
	'./Packets/packets2011_len_main.js': p2011,
	'./Packets/packets2012_len_main.js': p2012,
	'./Packets/packets2013_len_main.js': p2013,
	'./Packets/packets2014_len_main.js': p2014,
	'./Packets/packets2015_len_main.js': p2015,
	'./Packets/packets2016_len_main.js': p2016,
	'./Packets/packets2017_len_main.js': p2017,
	'./Packets/packets2018_len_main.js': p2018,
	'./Packets/packets2019_len_main.js': p2019,
	'./Packets/packets2020_len_main.js': p2020,
	'./Packets/packets2021_len_main.js': p2021,
	'./Packets/packets2022_len_main.js': p2022,
	'./Packets/packets2023_len_main.js': p2023,
	'./Packets/packets2024_len_main.js': p2024,
	'./Packets/packets2025_len_main.js': p2025
};

let packets_len = new Array();

/**
 * Get a Packet Length
 *
 * @param {number} packetver - packet version (clientdate)
 */
function init(packetver) {
	packetver = parseInt(packetver);

	// Find the matching year for the packet version
	const years = [
		2025, 2024, 2023, 2022, 2021, 2020, 2019, 2018, 2017, 2016, 2015, 2014, 2013, 2012, 2011, 2010, 2009, 2008,
		2007, 2006, 2005, 2004, 2003
	];

	let selectedYear = 2003;
	for (const year of years) {
		if (packetver >= year * 10000) {
			selectedYear = year;
			break;
		}
	}

	const modulePath = `./Packets/packets${selectedYear}_len_main.js`;
	const Lengths = packetModules[modulePath]?.default || packetModules[modulePath];

	if (Lengths && typeof Lengths.init === 'function') {
		packets_len = Lengths.init(packetver);
		packets_len[0x097f] = packets_len[0x097f] || -1;
		packets_len[0x0980] = packets_len[0x0980] || 7;
		console.log('%c[Network] Packet Length initialized ', 'color:#007000', packetver);
	} else {
		console.error(`[Network] Failed to load packet lengths for year ${selectedYear} (path: ${modulePath})`);
	}
}

/**
 * Get a Packet Length
 *
 * @param {number} id - packet ID
 */
function getPacketLength(id) {
	return packets_len[id] || false;
}

/**
 * Export
 */
export default {
	init: init,
	getPacketLength: getPacketLength
};
