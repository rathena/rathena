/**
 * Network/PacketVerManager.js
 *
 * Manager to find the server protocol
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import Configs from 'Core/Configs.js';
import PacketLength from 'Network/PacketLength.js';

/**
 * PACKETVER range
 * @var integer
 */
let _value = 0;

/**
 * Loop over version to find the good one
 * @return integer[] offset
 */
function getPacketVersion() {
	const versions = this.versions;
	let i;
	const count = versions.length;

	for (i = 0; i < count - 1; ++i) {
		if (_value < versions[i + 1][0]) {
			return versions[i];
		}
	}
	return versions[i];
}

/**
 * Get block size based on packetver
 *
 * @return {number} blocksize
 */
function calculateBlockSize() {
	// Not dependent of PACKETVER
	let blockSize = 106;
	blockSize += 2; // hairColor;
	blockSize += 4; // unknown value

	// slot/haircolor (2 * (short)) -> (2 * (char))
	// remove unknown 4 bytes
	blockSize += -(2 * 2) + 1 * 2 - 4;

	// Start check
	if (_value >= 20061023) {
		blockSize += 2; // .bIsChangedCharName
	}

	// hp/maxhp (2 * (short)) -> (2 * (int))
	if (_value > 20081217) {
		blockSize += -(2 * 2) + 2 * 4;
	}

	if ((_value >= 20100720 && _value <= 20100727) || _value >= 20100803) {
		blockSize += 12; // lastMap(14) - never found it...
		blockSize += 4; // lastMap(16)
	}

	// delete date
	if (_value >= 20100803) {
		blockSize += 4;
	}

	// robe
	if (_value >= 20110111) {
		blockSize += 4;
	}

	// slot addon
	if (_value >= 20110928) {
		blockSize += 4;
	}

	// rename addon
	if (_value >= 20111025) {
		blockSize += 4;
	}

	// Character sex
	if (_value >= 20141016) {
		blockSize++;
	}

	// Body
	if (_value >= 20141022) {
		blockSize += 2;
	}

	if (_value >= 20170830) {
		blockSize += 4; // base_exp
		blockSize += 4; // job_exp
	}

	if (_value >= 20211103) {
		blockSize += 4; // hp
		blockSize += 4; // maxhp
		blockSize += 6; // sp
		blockSize += 6; // maxsp
	}

	return blockSize;
}

/**
 * Parse char list
 *
 * @param {BinaryReader} fp
 * @param {number} end of the reader
 */
function parseCharList(fp, end) {
	if (!end) {
		end = fp.length;
	}

	let i, count;
	const out = [];
	let blockSize = Configs.get('charBlockSize') || calculateBlockSize();
	const length = end - fp.tell();

	// Nothing to parse
	if (length <= 0) {
		return out;
	}

	// Invalid blocksize...
	if (!blockSize || length % blockSize) {
		console.error(
			'CHARACTER_INFO size error!! blockSize : "' + blockSize + '", list length: ' + length + ', auto-detect...'
		);

		const knownSize = [106, 108, 112, 116, 124, 128, 132, 136, 140, 144, 145, 147, 155, 175];
		const matches = [];

		for (i = 0, count = knownSize.length; i < count; ++i) {
			if (length % knownSize[i] === 0) {
				matches.push(knownSize[i]);
			}
		}

		// No result, or multiple ones...
		if (matches.length !== 1) {
			import('UI/UIManager.js').then(UIManager => {
				UIManager.default.showErrorBox(
					'CHARACTER_INFO size error!! blockSize : "' +
						blockSize +
						'", list length: ' +
						length +
						', auto-detect...'
				);
			});
			return out;
		}

		blockSize = matches[0];
	}

	for (i = 0, count = length / blockSize; i < count; ++i) {
		out[i] = {};
		out[i].GID = fp.readULong();
		out[i].exp = fp.readLong();
		if (_value >= 20170830 || blockSize >= 155) {
			fp.readLong();
		}
		out[i].money = fp.readLong();
		out[i].jobexp = fp.readLong();
		if (_value >= 20170830 || blockSize >= 155) {
			fp.readLong();
		}
		out[i].joblevel = fp.readLong();
		out[i].bodyState = fp.readLong();
		out[i].healthState = fp.readLong();
		out[i].effectState = fp.readLong();
		out[i].virtue = fp.readLong();
		out[i].honor = fp.readLong();
		out[i].jobpoint = fp.readShort();

		if (blockSize < 112) {
			out[i].hp = fp.readShort();
			out[i].maxhp = fp.readShort();
		} else {
			out[i].hp = fp.readLong();
			if (_value >= 20211103 || blockSize >= 175) {
				fp.readLong();
			}
			out[i].maxhp = fp.readLong();
			if (_value >= 20211103 || blockSize >= 175) {
				fp.readLong();
			}
		}

		if (_value < 20201007 || blockSize < 175) {
			out[i].sp = fp.readShort();
			out[i].maxsp = fp.readShort();
		} else {
			out[i].sp = fp.readLong();
			if (_value >= 20211103 || blockSize >= 175) {
				fp.readLong();
			}
			out[i].maxsp = fp.readLong();
			if (_value >= 20211103 || blockSize >= 175) {
				fp.readLong();
			}
		}
		out[i].speed = fp.readShort();
		out[i].job = fp.readShort();
		out[i].head = fp.readShort();

		if (blockSize >= 147) {
			out[i].body = fp.readShort();
		}

		out[i].weapon = fp.readShort();
		out[i].level = fp.readShort();
		out[i].sppoint = fp.readShort();
		out[i].accessory = fp.readShort();
		out[i].shield = fp.readShort();
		out[i].accessory2 = fp.readShort();
		out[i].accessory3 = fp.readShort();
		out[i].headpalette = fp.readShort();
		out[i].bodypalette = fp.readShort();
		out[i].name = fp.readString(24);
		out[i].Str = fp.readUChar();
		out[i].Agi = fp.readUChar();
		out[i].Vit = fp.readUChar();
		out[i].Int = fp.readUChar();
		out[i].Dex = fp.readUChar();
		out[i].Luk = fp.readUChar();

		if (blockSize < 108) {
			out[i].CharNum = fp.readUShort();
		} else if (blockSize < 124) {
			out[i].CharNum = fp.readUShort();
			out[i].haircolor = fp.readUShort();
		} else {
			out[i].CharNum = fp.readUChar();
			out[i].haircolor = fp.readUChar();
		}

		if (blockSize === 116) {
			fp.seek(0x04, SEEK_CUR); // unknown
		}

		if (blockSize >= 124) {
			out[i].bIsChangedCharName = fp.readShort();
			out[i].lastMap = fp.readBinaryString(blockSize === 124 ? 12 : 16);
		}

		if (blockSize >= 132) {
			out[i].DeleteDate = fp.readLong();
		}

		if (blockSize >= 136) {
			out[i].Robe = fp.readLong();
		}

		if (blockSize >= 140) {
			out[i].SlotAddon = fp.readLong();
		}

		if (blockSize >= 144) {
			out[i].RenameAddon = fp.readLong();
		}

		if (blockSize >= 145) {
			out[i].sex = fp.readUChar();
		}
	}

	return out;
}

/**
 * Add support for a new packet version
 *
 * @param {number} date
 * @param {number[]} list of offsets
 */
function addSupport(date, list) {
	let packet, param;
	const count = list.length;

	for (let i = 0; i < count; ++i) {
		param = list[i];
		packet = param[0];
		param[0] = date;

		if (!packet?.prototype?.versions) {
			packet.prototype.versions = [];
		}

		packet.prototype.versions.push(list[i]);
		packet.prototype.getPacketVersion = getPacketVersion;
	}
}

/**
 * Export
 */
export default {
	// Get Back data
	get value() {
		return _value > 0 ? _value : window.ROConfig?.servers?.[0]?.packetver || window.ROConfig?.packetver || 0;
	},

	set value(v) {
		if (v !== _value) {
			PacketLength.init(v);
			console.log('%c[PACKETVER] Set packet version ', 'color:#007070', (_value = v));
		}
	},

	// Add support for packet version
	addSupport: addSupport,
	parseCharInfo: parseCharList
};
