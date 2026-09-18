/**
 * UI/Components/WinStats/WinStatsCommon.js
 *
 * Shared factory for all WinStats versions.
 * Migrated to GUIComponent with native DOM APIs.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Session from 'Engine/SessionStorage.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';

/**
 * Factory: creates a WinStats GUIComponent
 *
 * @param {string}  name      - component name
 * @param {string}  htmlText  - raw HTML
 * @param {string}  cssText   - raw CSS
 * @param {boolean} hasTraits - whether this version has trait stats
 */
export function createWinStats({ name, htmlText, cssText, hasTraits }) {
	const Component = new GUIComponent(name, cssText);
	Component.render = () => htmlText;

	const _preferences = Preferences.get('WinStats', { x: 0, y: 233, show: false, reduce: false }, 1.0);

	let _root;

	// ─── Helpers ───────────────────────────────────────

	function setText(selector, value) {
		const el = _root.querySelector(selector);
		if (el) el.textContent = value;
	}

	function setUpVisibility(selector, visible) {
		const el = _root.querySelector(selector);
		if (el) {
			el.style.opacity = visible ? 1 : 0;
			el.style.pointerEvents = visible ? 'initial' : 'none';
		}
	}

	// ─── Stat button map ──────────────────────────────

	const statButtonMap = {
		str: 13,
		agi: 14,
		vit: 15,
		int: 16,
		dex: 17,
		luk: 18
	};

	const traitButtonMap = {
		pow: 219,
		sta: 220,
		wis: 221,
		spl: 222,
		con: 223,
		crt: 224
	};

	// ─── init ──────────────────────────────────────────

	Component.init = function init() {
		this.statuspoint = 0;
		this.t_statuspoint = 0;

		this.draggable('.titlebar');

		_root = this.getRoot();

		// Base stat up buttons
		const upButtons = _root.querySelectorAll('.up button');
		upButtons.forEach(btn => {
			btn.addEventListener('mousedown', () => {
				const id = statButtonMap[btn.className];
				if (id) Component.onRequestUpdate(id, 1);
			});
		});

		// Trait stat up buttons
		if (hasTraits) {
			const tUpButtons = _root.querySelectorAll('.t_up button');
			tUpButtons.forEach(btn => {
				btn.addEventListener('mousedown', () => {
					const id = traitButtonMap[btn.className];
					if (id) Component.onRequestUpdate(id, 1);
				});
			});
		}

		// Titlebar mini
		const miniBtn = _root.querySelector('.titlebar .mini');
		if (miniBtn) {
			miniBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
			miniBtn.addEventListener('click', () => {
				const panel = _root.querySelector('.panel');
				if (panel) panel.style.display = panel.style.display === 'none' ? '' : 'none';
			});
		}

		// Titlebar close
		const closeBtn = _root.querySelector('.titlebar .close');
		if (closeBtn) {
			closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
			closeBtn.addEventListener('click', () => {
				this._host.style.display = 'none';
			});
		}

		// View traits toggle
		if (hasTraits) {
			const viewTraitsBtn = _root.querySelector('.view_traits');
			if (viewTraitsBtn) {
				viewTraitsBtn.addEventListener('mousedown', () => toggleTraits());
			}
		}
	};

	// ─── stack ─────────────────────────────────────────

	Component.stack = [];

	// ─── Embed mode ────────────────────────────────────

	const EMBED_CSS = `
		.titlebar { display: none !important; }
		.group { top: 6px !important; }
		.column1 { top: 5px !important; }
		.column2 { top: 5px !important; }
		.up { top: 2px !important; }
	`;

	let _embedStyleEl = null;
	let _embedAnchor = null;
	let _embedObserver = null;

	Component.embed = function embed(anchorHost) {
		// Ensure component is prepared (has _shadow/_host)
		if (!this.__loaded) {
			this.prepare();
		}
		// Ensure component is in the DOM
		if (!this._host.parentNode) {
			document.body.appendChild(this._host);
		}

		// Disconnect previous observer if re-embedding
		if (_embedObserver) {
			_embedObserver.disconnect();
			_embedObserver = null;
		}

		_embedAnchor = anchorHost;

		// Inject embed CSS into shadow
		if (!_embedStyleEl) {
			_embedStyleEl = document.createElement('style');
			_embedStyleEl.textContent = EMBED_CSS;
		}
		if (!_embedStyleEl.parentNode) {
			this._shadow.appendChild(_embedStyleEl);
		}

		// Position below anchor
		const syncPosition = () => {
			const rect = anchorHost.getBoundingClientRect();
			this._host.style.left = rect.left + 'px';
			this._host.style.top = rect.top + rect.height + 'px';
		};
		syncPosition();

		// Observe anchor movement
		_embedObserver = new MutationObserver(syncPosition);
		_embedObserver.observe(anchorHost, { attributes: true, attributeFilter: ['style'] });

		this._host.style.display = '';
	};

	Component.unembed = function unembed() {
		if (_embedStyleEl && _embedStyleEl.parentNode) {
			_embedStyleEl.parentNode.removeChild(_embedStyleEl);
		}
		if (_embedObserver) {
			_embedObserver.disconnect();
			_embedObserver = null;
		}
		_embedAnchor = null;
		this._host.style.display = 'none';
	};

	Component.isEmbedded = function isEmbedded() {
		return _embedAnchor !== null;
	};

	// ─── toggle ────────────────────────────────────────
	Component.toggle = function toggle() {
		// If embedded, unembed instead of normal toggle
		if (_embedAnchor) {
			this.unembed();
			return;
		}

		if (this._host.style.display === 'none') {
			this._host.style.display = '';
			this.focus();
		} else {
			this._host.style.display = 'none';
		}
	};

	// ─── onAppend ──────────────────────────────────────

	Component.onAppend = function onAppend() {
		for (let i = 0, count = this.stack.length; i < count; ++i) {
			this.update.apply(this, this.stack[i]);
		}
		this.stack.length = 0;

		if (!_embedAnchor) {
			this._host.style.top =
				Math.min(Math.max(0, _preferences.y), Renderer.height - (this._host.offsetHeight || 140)) + 'px';
			this._host.style.left =
				Math.min(Math.max(0, _preferences.x), Renderer.width - (this._host.offsetWidth || 280)) + 'px';
		}

		if (!_preferences.show && !_embedAnchor) {
			this._host.style.display = 'none';
		}
	};

	const _origFix = Component._fixPositionOverflow;
	Component._fixPositionOverflow = function () {
		if (!_embedAnchor) _origFix.call(this);
	};

	// ─── update ────────────────────────────────────────

	Component.update = function update(type, val) {
		let str;

		if (!this.__loaded) {
			this.stack.push(arguments);
			return;
		}

		switch (type) {
			// ── Status points ──
			case 'statuspoint':
				this.statuspoint = val;
				_root.querySelectorAll('.requirements div').forEach(div => {
					const req = parseInt(div.textContent, 10);
					setUpVisibility('.up .' + div.className, req > 0 && req <= val);
				});
				setText('.' + type, val);
				break;

			// ── Trait points ──
			case 'trait_point':
				this.t_statuspoint = val;
				_root.querySelectorAll('.t_requirements div').forEach(div => {
					const req = parseInt(div.textContent, 10);
					setUpVisibility('.t_up .' + div.className, req > 0 && req <= val);
				});
				setText('.trait_point', val);
				break;

			// ── Simple text values ──
			case 'guildname':
			case 'atak':
			case 'matak':
			case 'def':
			case 'mdef':
			case 'hit':
			case 'flee':
			case 'critical':
			case 'patk':
			case 'smatk':
			case 'hplus':
			case 'crate':
			case 'res':
			case 'mres':
				setText('.' + type, val);
				break;

			// ── ASPD ──
			case 'aspd':
				setText('.' + type, Math.floor(200 - val / 10));
				break;

			// ── Plus values ──
			case 'matak2':
				if (!Session.isRenewal) {
					setText('.' + type, '~ ' + val);
					break;
				}
			// falls through
			case 'atak2':
			case 'def2':
			case 'mdef2':
			case 'flee2':
				str = val < 0 ? '- ' + -val : '+ ' + val;
				setText('.' + type, str);
				break;

			// ── Base stats ──
			case 'str':
			case 'agi':
			case 'vit':
			case 'int':
			case 'dex':
			case 'luk':
			// ── Trait stats ──
			case 'pow':
			case 'sta':
			case 'wis':
			case 'spl':
			case 'con':
			case 'crt':
				setText('.stats .' + type, val);
				break;

			// ── Base bonus ──
			case 'str2':
			case 'agi2':
			case 'vit2':
			case 'int2':
			case 'dex2':
			case 'luk2':
			// ── Trait bonus ──
			case 'pow2':
			case 'sta2':
			case 'wis2':
			case 'spl2':
			case 'con2':
			case 'crt2':
				str = val < 0 ? '- ' + -val : val > 0 ? '+' + val : '';
				setText('.bonus .' + type.replace('2', ''), str);
				break;

			// ── Base requirements ──
			case 'str3':
			case 'agi3':
			case 'vit3':
			case 'int3':
			case 'dex3':
			case 'luk3':
				setText('.requirements .' + type.replace('3', ''), val);
				setUpVisibility('.up .' + type.replace('3', ''), val > 0 && val <= this.statuspoint);
				break;

			// ── Trait requirements ──
			case 'pow3':
			case 'sta3':
			case 'wis3':
			case 'spl3':
			case 'con3':
			case 'crt3':
				setText('.t_requirements .' + type.replace('3', ''), val);
				setUpVisibility('.t_up .' + type.replace('3', ''), val > 0 && val <= this.t_statuspoint);
				break;
		}
	};

	// ─── toggleTraits ─────────────────────────────

	function toggleTraits() {
		const traitsEl = _root.querySelector('.traits_component');
		const selfBtn = _root.querySelector('.view_traits');
		if (!traitsEl) return;

		const isVisible = traitsEl.style.display !== 'none';
		traitsEl.style.display = isVisible ? 'none' : '';
		const state = isVisible ? 'on' : 'off';

		Client.loadFile(DB.INTERFACE_PATH + 'statuswnd/expand_' + state + '_normal.bmp', function (data) {
			if (selfBtn) selfBtn.style.backgroundImage = 'url(' + data + ')';
		});
	}

	// ─── onShortCut ────────────────────────────────────

	Component.onShortCut = function onShortCut(key) {
		switch (key.cmd) {
			case 'TOGGLE':
				this.toggle();
				break;
		}
	};

	// ─── onRemove ──────────────────────────────────────

	Component.onRemove = function onRemove() {
		if (_preferences) {
			// Don't save embed-relative position/show as standalone preferences
			if (!_embedAnchor) {
				_preferences.show = this._host.style.display !== 'none';
				_preferences.x = parseInt(this._host.style.left, 10);
				_preferences.y = parseInt(this._host.style.top, 10);
			}
			const panel = _root.querySelector('.panel');
			_preferences.reduce = panel ? panel.style.display === 'none' : false;
			_preferences.save();
		}
	};

	// ─── Abstract ──────────────────────────────────────

	Component.onRequestUpdate = function onRequestUpdate(/*id, amount*/) {};

	return UIManager.addComponent(Component);
}
