/**
 * UI/Components/Intro/Preferences.js
 *
 * Manage User Preferences
 *
 * @author Vincent Thibault
 * @refactor AoShinHo
 */

import Configs from 'Core/Configs.js';
import Context from 'Core/Context.js';
import Preferences from 'Core/Preferences.js';
import Audio from 'Preferences/Audio.js';
import Graphics from 'Preferences/Graphics.js';

const _preferences = Preferences.get(
	'Window',
	{
		serverfile: 'clientinfo.xml',
		serverlist: [],
		serverdef: 'serverfile',
		save: true
	},
	1.1
);

/**
 * Load preferences into the settings overlay
 * @param {Element|ShadowRoot} root
 */
function load(root) {
	if (Graphics.screensize === 'full' && !Context.isFullScreen()) {
		Graphics.screensize = '800x600';
	}

	const q = sel => root.querySelector(sel);
	const qa = sel => root.querySelectorAll(sel);

	// Screen
	q('.screensize').value = Graphics.screensize;

	const qualityEl = q('.quality');
	qualityEl.value = Graphics.quality;
	qualityEl.dispatchEvent(new Event('input'));

	// Cursor (fix: original used .cursor-options which didn't exist)
	const cursorEl = q('.cursor');
	if (cursorEl) cursorEl.checked = !!Graphics.cursor;

	// Server def
	qa('.serverdef').forEach(r => (r.checked = false));
	const activeRadio = q('.serverdef[value="' + _preferences.serverdef + '"]');
	if (activeRadio) {
		activeRadio.checked = true;
		activeRadio.dispatchEvent(new Event('click'));
	}
	q('.clientinfo').value = _preferences.serverfile;

	// Audio
	const bgmEl = q('.bgmvol');
	bgmEl.value = Audio.BGM.volume * 100;
	bgmEl.dispatchEvent(new Event('input'));

	const soundEl = q('.soundvol');
	soundEl.value = Audio.Sound.volume * 100;
	soundEl.dispatchEvent(new Event('input'));

	// Save files
	const saveEl = q('.save');
	if (!window.requestFileSystem && !window.webkitRequestFileSystem) {
		Configs.set('saveFiles', false);
		saveEl.disabled = true;
	} else if (!Configs.get('saveFiles')) {
		saveEl.disabled = true;
	} else {
		saveEl.checked = !!_preferences.saveFiles;
	}

	// Server list
	const tbody = q('.servers');
	tbody.innerHTML = '';
	const serverlist = _preferences.serverlist;

	for (let i = 0; i < serverlist.length; i++) {
		const tr = document.createElement('tr');
		tr.innerHTML =
			'<td><input type="text" class="display"/></td>' +
			'<td><input type="text" class="address"/></td>' +
			'<td><input type="text" class="version"/></td>' +
			'<td><input type="text" class="langtype"/></td>' +
			'<td><input type="text" class="packetver"/></td>' +
			'<td><button class="btn_delete">\u00D7</button></td>';

		tr.querySelector('.display').value = serverlist[i].display;
		tr.querySelector('.address').value = serverlist[i].address + ':' + serverlist[i].port;
		tr.querySelector('.version').value = serverlist[i].version;
		tr.querySelector('.langtype').value = serverlist[i].langtype;
		tr.querySelector('.packetver').value = serverlist[i].packetver;

		tbody.appendChild(tr);
	}

	apply();
}

/**
 * Save preferences from the settings overlay
 * @param {Element|ShadowRoot} root
 */
function save(root) {
	const q = sel => root.querySelector(sel);
	const qa = sel => root.querySelectorAll(sel);

	Graphics.screensize = q('.screensize').value;
	Graphics.quality = q('.quality').value;
	Graphics.cursor = q('.cursor') ? q('.cursor').checked : false;
	_preferences.saveFiles = q('.save') ? q('.save').checked : false;

	if (Configs.get('_serverEditMode')) {
		const checkedRadio = q('.serverdef:checked');
		_preferences.serverdef = checkedRadio ? checkedRadio.value : 'serverfile';
		_preferences.serverfile = q('.clientinfo').value;
		_preferences.serverlist = [];

		const rows = qa('.servers tr');
		rows.forEach(row => {
			const addr = row.querySelector('.address').value;
			_preferences.serverlist.push({
				display: row.querySelector('.display').value,
				address: addr.split(':')[0],
				port: parseInt(addr.split(':')[1], 10),
				version: row.querySelector('.version').value,
				langtype: row.querySelector('.langtype').value,
				packetver: row.querySelector('.packetver').value
			});
		});
	}

	Audio.BGM.volume = q('.bgmvol').value / 100;
	Audio.BGM.play = Audio.BGM.volume > 0;
	Audio.Sound.volume = q('.soundvol').value / 100;
	Audio.Sound.play = Audio.Sound.volume > 0;

	Audio.save();
	Graphics.save();
	_preferences.save();

	apply();
}

/**
 * Apply preferences
 */
function apply() {
	const isFullScreen = Context.isFullScreen();

	if (Graphics.screensize === 'full') {
		if (!isFullScreen) Context.requestFullScreen();
	} else {
		if (isFullScreen) Context.cancelFullScreen();
		if (Context.Is.POPUP) {
			const size = Graphics.screensize.split('x');
			if (size[0] != window.innerWidth && size[1] != window.innerHeight) {
				window.resizeTo(size[0], size[1]);
				window.moveTo((screen.availWidth - size[0]) / 2, (screen.availHeight - size[1]) / 2);
			}
		}
	}

	if (Configs.get('_serverEditMode')) {
		if (_preferences.serverdef === 'serverlist') {
			Configs.set('servers', _preferences.serverlist);
		} else {
			Configs.set('servers', 'data/' + _preferences.serverfile);
		}
	}

	Configs.set('saveFiles', _preferences.saveFiles);
	Configs.set('quality', Graphics.quality);
}

export default { save, load };
