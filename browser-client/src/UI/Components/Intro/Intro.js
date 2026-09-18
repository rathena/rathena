/**
 * UI/Components/Intro/Intro.js
 *
 * Intro Manager — GUIComponent
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * @refactor AoShinHo
 */

import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './Intro.html?raw';
import cssText from './Intro.css?raw';
import Context from 'Core/Context.js';
import Configs from 'Core/Configs.js';
import Particle from './Particle.js';
import Preferences from './Preferences.js';
import FileSystem from './FileSystem.js';
import { roInitSpinner } from 'App/PreLoader.js';

/**
 * Create Intro component
 */
const Intro = new GUIComponent('Intro', cssText);

Intro.render = () => htmlText;

/**
 * @var {FileList}
 */
Intro.files = [];

/**
 * @var {function} resize handler ref for cleanup
 */
let _resizeHandler = null;

/**
 * Helper: query inside shadow DOM
 */
function querySelector(sel) {
	return Intro._shadow.querySelector(sel);
}

/**
 * Manage Escape key to exit
 */
Intro.onKeyDown = function OnKeyDown(event) {
	if (!this._host || !this._host.parentNode) {
		return true;
	}
	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		if (Context.isFullScreen()) {
			Context.cancelFullScreen();
		}
		event.stopImmediatePropagation();
		return false;
	}
	return true;
};

/**
 * Show an overlay with fade-in
 */
function showOverlay(el) {
	el.style.display = 'block';
	// eslint-disable-next-line no-unused-expressions
	el.offsetHeight;
	el.style.opacity = '1';
}

/**
 * Hide an overlay with fade-out
 */
function hideOverlay(el) {
	el.style.opacity = '0';
	const handler = () => {
		el.style.display = 'none';
		el.removeEventListener('transitionend', handler);
	};
	el.addEventListener('transitionend', handler);
	// Fallback caso transitionend não dispare (ex: sem transition no CSS)
	setTimeout(() => {
		if (el.style.display !== 'none') {
			el.style.display = 'none';
			el.removeEventListener('transitionend', handler);
		}
	}, 300);
}

/**
 * Initialize
 */
Intro.init = function init() {
	if (!Configs.get('servers')) {
		Configs.set('_serverEditMode', true);
	}

	// Inject Google Fonts if not already present
	if (!document.querySelector('link[href*="Cinzel"]')) {
		const link = document.createElement('link');
		link.rel = 'stylesheet';
		link.href =
			'https://fonts.googleapis.com/css2?family=Cinzel:wght@400;600;700&family=Inter:wght@300;400;500;600&display=swap';
		document.head.appendChild(link);
	}

	const root = this._shadow;

	// ── About overlay ──
	const aboutOverlay = root.querySelector('.overlay.about');

	root.querySelector('.btn_about').addEventListener('click', () => {
		showOverlay(aboutOverlay);
	});

	aboutOverlay.addEventListener('click', e => {
		if (e.target.closest('a, input, button, select')) return;
		hideOverlay(aboutOverlay);
	});

	// ── Settings overlay ──
	root.querySelector('.btn_settings').addEventListener('click', () => {
		Preferences.load(root);
		showOverlay(root.querySelector('.overlay.settings'));
	});

	// ── Drop zone (replaces old .box) ──
	const dropZone = root.querySelector('.drop-zone');

	dropZone.addEventListener('mouseover', () => dropZone.classList.add('dragover'));
	dropZone.addEventListener('mouseout', () => dropZone.classList.remove('dragover'));
	dropZone.addEventListener('click', () => root.querySelector('input[type="file"]').click());

	dropZone.addEventListener('dragover', e => {
		e.preventDefault();
		dropZone.classList.add('dragover');
	});
	dropZone.addEventListener('dragleave', () => {
		dropZone.classList.remove('dragover');
	});
	dropZone.addEventListener('drop', e => {
		e.preventDefault();
		e.stopImmediatePropagation();
		dropZone.classList.remove('dragover');
		processDropEvent(e);
	});

	// ── File input ──
	root.querySelector('input[type="file"]').addEventListener('change', function () {
		processFileInput(this);
	});

	// ── Quality slider ──
	root.querySelector('.quality').addEventListener('input', function () {
		root.querySelector('.quality_result').textContent = this.value + '%';
	});

	// ── Clean cache ──
	root.querySelector('.clean').addEventListener('click', function () {
		this.style.display = 'none';
		const status = root.querySelector('.clean-status');
		status.innerHTML = '<i>Cleaning cache...</i>';

		FileSystem.cleanup(() => {
			status.innerHTML = '';
			const msg = root.querySelector('.msg');
			if (msg) msg.textContent = '';
		});
	});

	// ── Stop propagation on interactive elements inside overlays ──
	root.querySelectorAll('.overlay').forEach(overlay => {
		overlay.addEventListener('click', e => {
			const tag = e.target.tagName;
			if (tag === 'INPUT' || tag === 'A' || tag === 'BUTTON' || tag === 'SELECT') {
				if (tag === 'INPUT' && e.target.type === 'text') {
					e.target.select();
				}
				e.stopImmediatePropagation();
			}
		});
	});

	// ── Hide server edit if not in edit mode ──
	if (!Configs.get('_serverEditMode')) {
		const se = root.querySelector('.serveredit');
		if (se) se.style.display = 'none';
	}

	// ── Volume sliders ──
	root.querySelector('.bgmvol').addEventListener('input', function () {
		root.querySelector('.bgmvol_result').textContent = this.value + '%';
	});
	root.querySelector('.soundvol').addEventListener('input', function () {
		root.querySelector('.soundvol_result').textContent = this.value + '%';
	});

	// ── Add server ──
	root.querySelector('.btn_add').addEventListener('click', () => {
		const tbody = root.querySelector('.servers');
		const count = tbody.querySelectorAll('tr').length;
		const tr = document.createElement('tr');
		tr.innerHTML =
			'<td><input type="text" class="display" value="Server ' +
			count +
			'"/></td>' +
			'<td><input type="text" class="address" value="127.0.0.1:6900"/></td>' +
			'<td><input type="text" class="version" value="22"/></td>' +
			'<td><input type="text" class="langtype" value="12"/></td>' +
			'<td><input type="text" class="packetver" value="auto"/></td>' +
			'<td><button class="btn_delete">\u00D7</button></td>';
		tbody.appendChild(tr);
		tr.querySelector('input').focus();
	});

	// ── Save settings ──
	root.querySelector('.btn_save').addEventListener('click', () => {
		Preferences.save(root);
		const overlays = root.querySelectorAll('.overlay');
		overlays.forEach(o => {
			if (o.style.display !== 'none') {
				hideOverlay(o);
			}
		});
	});

	// ── Delete server (event delegation) ──
	root.querySelector('.servers').addEventListener('click', e => {
		if (e.target.classList.contains('btn_delete')) {
			e.target.closest('tr').remove();
		}
	});

	// ── Start roBrowser ──
	root.querySelector('.btn_play').addEventListener('click', () => {
		showOverlay(root.querySelector('.overlay.loading'));
		Intro.onFilesSubmit(Intro.files);
	});
};

/**
 * Once appended to body
 */
Intro.onAppend = function onAppend() {
	const root = this._shadow;

	// Make host cover the full viewport
	this._host.style.position = 'fixed';
	this._host.style.top = '0';
	this._host.style.left = '0';
	this._host.style.width = '100%';
	this._host.style.height = '100%';
	this._host.style.zIndex = '9999';

	// Can't resize the window if it's not a popup
	if (!Context.Is.POPUP) {
		const res = root.querySelector('.resolution');
		if (res) res.style.display = 'none';
	}

	// Show cached file size
	const cleanBtn = root.querySelector('.clean');
	if (cleanBtn) cleanBtn.style.display = 'none';

	FileSystem.getSize(used => {
		if (!used) return;
		let msg = '';
		if (used > 1024 * 1024 * 1024) {
			msg = (used / 1024 / 1024 / 1024).toFixed(2) + ' GiB saved';
		} else if (used > 1024 * 1024) {
			msg = (used / 1024 / 1024).toFixed(2) + ' MiB saved';
		} else {
			msg = (used / 1024).toFixed(2) + ' KiB saved';
		}
		const msgEl = root.querySelector('.msg');
		if (msgEl) msgEl.textContent = msg;
		if (cleanBtn) cleanBtn.style.display = 'inline-block';
	});

	// Resize handler
	const introEl = root.querySelector('.intro');
	_resizeHandler = () => {
		const scaleX = window.innerWidth / 800;
		const scaleY = window.innerHeight / 600;
		const scaleMin = Math.min(scaleX, scaleY);
		introEl.style.transform = 'scale(' + scaleX + ',' + scaleY + ')';
		introEl.style.setProperty('--cx', scaleMin / scaleX);
		introEl.style.setProperty('--cy', scaleMin / scaleY);
	};
	window.addEventListener('resize', _resizeHandler);
	_resizeHandler();

	// Wait for background image and fonts, then remove the HTML preloader.
	let particleReady = false;
	let fontsReady = false;

	function checkReady() {
		if (particleReady && fontsReady) {
			roInitSpinner.remove();
		}
	}

	Particle.init(30, root.querySelector('canvas'), () => {
		particleReady = true;
		checkReady();
	});

	document.fonts.ready.then(() => {
		fontsReady = true;
		checkReady();
	});
};

/**
 * Once removed
 */
Intro.onRemove = function onRemove() {
	if (_resizeHandler) {
		window.removeEventListener('resize', _resizeHandler);
		_resizeHandler = null;
	}
	Particle.stop();

	// Hide all overlays
	this._shadow.querySelectorAll('.overlay').forEach(o => {
		o.style.display = 'none';
		o.style.opacity = '0';
	});
};

/**
 * Process file input (click to browse)
 */
function processFileInput(input) {
	if (!input.files || !input.files.length) return;

	const files = input.files;
	const token =
		'webkitRelativePath' in files[0] ? 'webkitRelativePath' : 'relativePath' in files[0] ? 'relativePath' : null;

	if (token) {
		const baseFolder = /^[^(\/|\\)]+(\/|\\)/;
		for (let i = 0; i < files.length; i++) {
			files[i].fullPath = files[i][token].replace(baseFolder, '');
		}
	}

	addFiles(files);
}

/**
 * Process drop event (drag & drop)
 */
function processDropEvent(event) {
	const data = event.dataTransfer;
	if (!data) return;

	let _dir_count = 0,
		_dir_loaded = 0;
	let _file_count = 0,
		_file_loaded = 0;
	const _files = [];

	function recursiveReader(entry, skip) {
		if (entry.isFile) {
			++_file_count;
			entry.file(file => {
				file.fullPath = entry.fullPath.substr(skip);
				_files.push(file);
				if (++_file_loaded === _file_count && _dir_loaded === _dir_count) {
					addFiles(_files);
				}
			});
		} else if (entry.isDirectory) {
			++_dir_count;
			entry.createReader().readEntries(entries => {
				for (let i = 0; i < entries.length; i++) {
					recursiveReader(entries[i], skip);
				}
				if (++_dir_loaded === _dir_count && _file_loaded === _file_count) {
					addFiles(_files);
				}
			});
		}
	}

	if (data.items && data.items.length && data.items[0].webkitGetAsEntry) {
		let skip = 1;
		const entry = data.items[0].webkitGetAsEntry();
		if (data.items.length === 1 && entry.isDirectory) {
			skip = entry.fullPath.split('/')[1].length + 2;
		}
		for (let i = 0; i < data.items.length; i++) {
			recursiveReader(data.items[i].webkitGetAsEntry(), skip);
		}
	} else if (data.files) {
		addFiles(data.files);
	}
}

/**
 * Add files to the list and update UI
 */
function addFiles(files) {
	if (!files.length) return;
	Intro.files.push.apply(Intro.files, files);
	const msgEl = querySelector('.msg');
	if (msgEl) msgEl.textContent = Intro.files.length + ' files selected';
}

/**
 * Callback to be used externally
 */
Intro.onFilesSubmit = function OnFilesSubmit() {};

/**
 * Store component and return it
 */
export default UIManager.addComponent(Intro);
