/**
 * Utils/ConsoleManager.js
 *
 * Console Manager
 *
 * Enables/Disables console
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import Configs from 'Core/Configs.js';

let _console;

function init() {
	// Save original console
	_console = console;
}

/**
 * Returns either the real or the dummy console based on configs
 */
function toggleConsole() {
	const optDev = Configs.get('development', false);
	const optEC = Configs.get('enableConsole', false);
	const optDC = Configs.get('disableConsole', false);

	if (!optDC && (optDev || optEC)) {
		_console.log(
			'%c[ConsoleManager] %cOutput to console is %cENABLED',
			'color:#3344EE',
			'color:inherit',
			'color:#007000'
		);
		// eslint-disable-next-line no-global-assign
		console = _console;
	} else {
		_console.log(
			'%c[ConsoleManager] %cOutput to console is %cDISABLED',
			'color:#3344EE',
			'color:inherit',
			'color:#700000'
		);
		// eslint-disable-next-line no-global-assign
		console = noConsole;
	}
}

/**
 * Dummy function
 */
const dummy = function () {
	return true;
};

/**
 * Disabled console object
 * Has all the console functions for Function.prototype.apply() compatibility, but replaced with the dummy function
 */
const noConsole = {
	assert: dummy,
	clear: dummy,
	count: dummy,
	countReset: dummy,
	debug: dummy,
	dir: dummy,
	dirxml: dummy,
	error: dummy,
	group: dummy,
	groupCollapsed: dummy,
	groupEnd: dummy,
	info: dummy,
	log: dummy,
	profile: dummy,
	profileEnd: dummy,
	table: dummy,
	time: dummy,
	timeEnd: dummy,
	timeLog: dummy,
	timeStamp: dummy,
	trace: dummy,
	warn: dummy
};

/**
 * Export
 */
export default {
	init: init,
	toggle: toggleConsole
};
