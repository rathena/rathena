/**
 * Utils/Executable.js
 *
 * Executable
 *
 * Helper to load an executable and extract some informations (compiled date, and some RO things).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import BinaryReader from './BinaryReader.js';

/**
 * Binary data of the executable
 * @var {BinaryReader}
 */
let _fp;

/**
 * Initialize the executable
 * Execute callback once ready
 *
 * @param {File} executable
 * @param {function} callback
 */
function getDate(executable, callback) {
	const reader = new FileReader();
	reader.onload = function (event) {
		_fp = new BinaryReader(event.target.result);
		callback(getDateSub());
	};
	reader.readAsArrayBuffer(executable);
}

/**
 * Get compilation date of an executable
 *
 * @return {number}
 */
function getDateSub() {
	if (!_fp) {
		throw new Error('Executable::getDate() - Executable is not loaded yet, or not specified');
	}

	// Jump to header and extract
	// PEHeader structure position
	_fp.seek(0x3c, SEEK_SET);
	const offset = _fp.readULong();

	if (offset > _fp.length) {
		throw new Error('Executable::getDate() - Invalid executable specified.');
	}

	// Jump to PEHeader structure
	_fp.seek(offset, SEEK_SET);
	if (_fp.readString(4) !== 'PE') {
		throw new Error('Executable::getDate() - Invalid executable specified.');
	}

	// Extract compiled date from executable
	_fp.seek(0x04, SEEK_CUR);
	const date = new Date(_fp.readULong() * 1000);

	// Convert date to YYYYMMDD
	return date.getFullYear() * 1e4 + (date.getMonth() + 1) * 1e2 + date.getDate();
}

/**
 * Check if a file is a RO executable
 *
 * @param {File} file
 * @return {boolean} true if it's a RO file
 */
function isROExec(file) {
	if (!file.name.match(/\.exe$/i)) {
		return false;
	}

	// TODO: check in the Executable binary
	if (file.size < 1024 * 1024 * 3 || file.size > 1024 * 1024 * 7) {
		return false;
	}

	return true;
}

/**
 * Exports
 */
export default {
	getDate: getDate,
	isROExec: isROExec
};
