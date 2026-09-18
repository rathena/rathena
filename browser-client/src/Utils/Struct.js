/**
 * Utils/Struct.js
 *
 * Implement C like structure in JS, example:
 *
 * const auth = new Struct(
 *     "unsigned char username[24]",
 *     "unsigned char password[24]",
 *     "bool stay_connect",
 *     "float version",
 *     "int tick"
 * );
 *
 * fp.readStruct(auth);
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Struct Constructor
 *
 * @param {string[]} C like structure
 */
export default function Struct() {
	let args, unsigned;
	let i,
		count,
		total = 0,
		size;
	let type, name, func;
	const out = {};

	const len = arguments.length;

	for (i = 0; i < len; ++i) {
		args = arguments[i].match(
			/(unsigned\s)?(bool|char|short|int|long|float|double)\s([a-zA-Z0-9_-]+)(\[(\d+)\])?;?/
		);
		unsigned = !!args[1];
		type = args[2].toLowerCase();
		name = args[3];
		count = args[5] ? parseInt(args[5], 10) : 1;

		switch (type) {
			case 'bool':
				size = 1;
				func = 'int8';
				break;
			case 'char':
				size = 1;
				func = 'int8';
				break;
			case 'short':
				size = 2;
				func = 'int16';
				break;
			case 'int':
				size = 4;
				func = 'int32';
				break;
			case 'long':
				size = 4;
				func = 'int32';
				break;
			case 'float':
				size = 4;
				func = 'float32';
				break;
			case 'double':
				size = 8;
				func = 'float64';
				break;
			default:
				throw new Error("Struct() - Undefined type '" + type + "'.");
		}

		func = 'get' + (unsigned ? 'U' + func : func.charAt(0).toUpperCase() + func.substr(1));
		total += size * count;

		out[name] = {
			func: func,
			count: count
		};
	}

	this._list = out;
	this.size = total;
}
