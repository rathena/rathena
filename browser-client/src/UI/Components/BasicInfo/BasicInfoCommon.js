/**
 * UI/Components/BasicInfo/BasicInfoCommon.js
 *
 * Shared factory for the Character Basic Information windows.
 *
 * Collapses the near-identical BasicInfo version siblings into a single
 * createBasicInfo(config) factory. Each version file passes its own
 * htmlText/cssText plus capability flags describing its real differences.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Configs from 'Core/Configs.js';
import PACKETVER from 'Network/PacketVerManager.js';
import MonsterTable from 'DB/Monsters/MonsterTable.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import Session from 'Engine/SessionStorage.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import PartyFriends from 'UI/Components/PartyFriends/PartyFriends.js';
import Guild from 'UI/Components/Guild/Guild.js';
import Bank from 'UI/Components/Bank/Bank.js';
import Escape from 'UI/Components/Escape/Escape.js';
import WorldMap from 'UI/Components/WorldMap/WorldMap.js';
import CheckAttendance from 'UI/Components/CheckAttendance/CheckAttendance.js';
import ChatRoomCreate from 'UI/Components/ChatRoomCreate/ChatRoomCreate.js';
import Rodex from 'UI/Components/Rodex/Rodex.js';
import WinStats from 'UI/Components/WinStats/WinStats.js';
import Navigation from 'UI/Components/Navigation/Navigation.js';
import SkillList from 'UI/Components/SkillList/SkillList.js';
import Quest from 'UI/Components/Quest/Quest.js';
import Achievement from 'UI/Components/Achievement/Achievement.js';
import Reputation from 'UI/Components/Reputation/Reputation.js';

export function createBasicInfo(config) {
	const {
		name,
		htmlText,
		cssText,
		prefKey,
		reduceDefault = true,
		innerId,
		topbarItemSelector = '.topbar button',
		topbarDblClick = false,
		toggleButtonsEvent = 'mousedown',
		buttonsSelector = '.buttons button',
		buttonsEvent = 'mousedown',
		buttonKeyBy = 'class',
		infoOpensWinStats = true,
		partyViaGetUI = false,
		hasToolbarToggle = false,
		miniLayout = false,
		hideIds = [],
		barScale = 1.27,
		hasApBar = false
	} = config;

	const Component = new GUIComponent(name, cssText);

	/**
	 * Stored data
	 */
	Component.base_exp = 0;
	Component.base_exp_next = 1;
	Component.job_exp = 0;
	Component.job_exp_next = -1;
	Component.weight = 0;
	Component.weight_max = 1;

	Component.render = () => htmlText;

	/**
	 * @let {Preferences} structure
	 */
	const _preferences = Preferences.get(
		prefKey,
		{
			x: 0,
			y: 0,
			reduce: reduceDefault,
			buttons: true,
			magnet_top: true,
			magnet_bottom: false,
			magnet_left: true,
			magnet_right: false
		},
		1.0
	);

	/**
	 * Dispatch a top-level shortcut button to its window.
	 *
	 * @param {string} key button identifier
	 */
	function dispatchButton(key) {
		switch (key) {
			case 'item':
				Inventory.getUI().toggle();
				break;
			case 'info':
				(infoOpensWinStats ? WinStats.getUI() : Equipment.getUI()).toggle();
				break;
			case 'equip':
				Equipment.getUI().toggle();
				break;
			case 'skill':
				SkillList.getUI().toggle();
				break;
			case 'option':
				Escape.ui.toggle();
				break;
			case 'party':
				if (partyViaGetUI) {
					PartyFriends.getUI().toggle();
				} else {
					PartyFriends.toggle();
				}
				break;
			case 'guild':
				Guild.toggle();
				break;
			case 'chat':
				ChatRoomCreate.toggle();
				break;
			case 'map':
				WorldMap.toggle();
				break;
			case 'bank':
				Bank.toggle();
				break;
			case 'quest':
				Quest.getUI().toggle();
				break;
			case 'mail':
				Rodex.toggle();
				break;
			case 'navigation':
				Navigation.toggle();
				break;
			case 'attendance':
				if (Configs.get('enableCheckAttendance') && PACKETVER.value >= 20180307) {
					CheckAttendance.toggle();
				}
				break;
			case 'achievment':
				if (Configs.get('enableAchievements') && PACKETVER.value >= 20150513) {
					Achievement.toggle();
				}
				break;
			case 'repute':
				Reputation.toggle();
				break;
		}
	}

	/**
	 * Apply the button-panel visibility for the toolbar layout (btn_open/btn_close).
	 */
	function applyToolbarButtons(root) {
		const buttons = root.querySelector('.buttons');
		const btnOpen = root.querySelector('.btn_open');
		const btnClose = root.querySelector('.btn_close');

		if (_preferences.buttons) {
			if (buttons) buttons.style.display = '';
			if (btnOpen) btnOpen.style.display = 'none';
			if (btnClose) btnClose.style.display = '';
		} else {
			if (buttons) buttons.style.display = 'none';
			if (btnOpen) btnOpen.style.display = '';
			if (btnClose) btnClose.style.display = 'none';
		}
	}

	/**
	 * Swap the `.toggle_btns` background between the view-on / view-off images.
	 */
	function loadViewToggleImage(root) {
		const type = _preferences.buttons ? 'off' : 'on';
		Client.loadFile(`${DB.INTERFACE_PATH}basic_interface/view${type}.bmp`, url => {
			const toggleBtn = root.querySelector('.toggle_btns');
			if (toggleBtn) {
				toggleBtn.style.backgroundImage = `url(${url})`;
			}
		});
	}

	/**
	 * Initialize UI
	 */
	Component.init = function init() {
		const root = this.getRoot();

		root.querySelectorAll(topbarItemSelector).forEach(el => {
			el.addEventListener('mousedown', e => e.stopImmediatePropagation());
		});

		if (topbarDblClick) {
			const topbar = root.querySelector('.topbar');
			if (topbar) {
				topbar.addEventListener('dblclick', () => Component.toggleMode());
			}
		}

		const rightBtn = root.querySelector('.topbar .right');
		if (rightBtn) {
			rightBtn.addEventListener('click', () => Component.toggleMode());
		}

		root.querySelectorAll('.toggle_btns').forEach(btn => {
			btn.addEventListener(toggleButtonsEvent, e => Component.toggleButtons(e));
		});

		root.querySelectorAll(buttonsSelector).forEach(el => {
			el.addEventListener(buttonsEvent, () => {
				const key = buttonKeyBy === 'id' ? el.id : el.className.split(' ')[0];
				dispatchButton(key);
			});
		});

		this.draggable();
	};

	/**
	 * When append the element to html
	 * Execute elements in memory
	 */
	Component.onAppend = function onAppend() {
		const root = this.getRoot();
		const hostRect = this._host.getBoundingClientRect();

		this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - hostRect.height)}px`;
		this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - hostRect.width)}px`;

		this.magnet.TOP = _preferences.magnet_top;
		this.magnet.BOTTOM = _preferences.magnet_bottom;
		this.magnet.LEFT = _preferences.magnet_left;
		this.magnet.RIGHT = _preferences.magnet_right;

		const inner = root.querySelector(innerId);
		if (inner) {
			inner.classList.remove('small', 'large');
			inner.classList.add(_preferences.reduce ? 'small' : 'large');
		}

		if (hasToolbarToggle) {
			applyToolbarButtons(root);
		} else if (inner) {
			const buttons = root.querySelector('.buttons');
			if (_preferences.reduce) {
				if (miniLayout) {
					if (buttons) buttons.style.display = 'none';
					const bgAttr = inner.dataset.miniBackground;
					if (bgAttr) {
						Client.loadFile(DB.INTERFACE_PATH + bgAttr, url => {
							inner.style.backgroundImage = `url(${url})`;
						});
					}
				} else if (buttons) {
					buttons.style.display = _preferences.buttons ? '' : 'none';
				}
			} else if (miniLayout && buttons) {
				buttons.style.display = '';
			}
		}

		hideIds.forEach(id => {
			const el = root.querySelector(`#${id}`);
			if (el) {
				el.style.display = 'none';
			}
		});
	};

	/**
	 * Once remove, save preferences
	 */
	Component.onRemove = function onRemove() {
		const root = this.getRoot();
		const inner = root.querySelector(innerId);
		const buttons = root.querySelector('.buttons');

		_preferences.x = parseInt(this._host.style.left, 10);
		_preferences.y = parseInt(this._host.style.top, 10);
		_preferences.reduce = inner ? inner.classList.contains('small') : _preferences.reduce;
		_preferences.buttons = buttons ? buttons.style.display !== 'none' : _preferences.buttons;
		_preferences.magnet_top = this.magnet.TOP;
		_preferences.magnet_bottom = this.magnet.BOTTOM;
		_preferences.magnet_left = this.magnet.LEFT;
		_preferences.magnet_right = this.magnet.RIGHT;
		_preferences.save();
	};

	/**
	 * Process shortcut
	 *
	 * @param {object} key
	 */
	Component.onShortCut = function onShortCut(key) {
		switch (key.cmd) {
			case 'EXTEND':
				this.toggleMode();
				break;
		}
	};

	/**
	 * Switch window size
	 */
	Component.toggleMode = function toggleMode() {
		const root = this.getRoot();
		const inner = root.querySelector(innerId);
		if (!inner) {
			return;
		}

		inner.classList.toggle('small');
		inner.classList.toggle('large');

		if (hasToolbarToggle) {
			applyToolbarButtons(root);
			return;
		}

		const buttons = root.querySelector('.buttons');

		if (inner.classList.contains('large')) {
			if (buttons) {
				buttons.style.display = '';
			}
			if (miniLayout) {
				const bgAttr = inner.dataset.background;
				if (bgAttr) {
					Client.loadFile(DB.INTERFACE_PATH + bgAttr, url => {
						inner.style.backgroundImage = `url(${url})`;
					});
				}
			}
			return;
		}

		if (miniLayout) {
			const bgAttr = inner.dataset.miniBackground;
			if (bgAttr) {
				Client.loadFile(DB.INTERFACE_PATH + bgAttr, url => {
					inner.style.backgroundImage = `url(${url})`;
				});
			}
			if (buttons) {
				buttons.style.display = 'none';
			}
		} else {
			if (buttons) {
				buttons.style.display = _preferences.buttons ? '' : 'none';
			}
			loadViewToggleImage(root);
		}
	};

	/**
	 * Toggle the list of buttons
	 */
	Component.toggleButtons = function toggleButtons(event) {
		const root = this.getRoot();
		const buttons = root.querySelector('.buttons');
		if (!buttons) {
			return;
		}

		_preferences.buttons = buttons.style.display === 'none';

		if (hasToolbarToggle) {
			applyToolbarButtons(root);
		} else {
			buttons.style.display = _preferences.buttons ? '' : 'none';
			loadViewToggleImage(root);
		}

		event.stopImmediatePropagation();
	};

	/**
	 * Update a gauge bar (hp / sp / ap) and its numeric readouts
	 *
	 * @param {string} type gauge identifier
	 * @param {number} val1 current value
	 * @param {number} val2 maximum value
	 * @param {string} color bar color prefix
	 */
	function updateBar(root, type, val1, val2, color) {
		const perc = Math.floor((val1 * 100) / val2);

		root.querySelectorAll(`.${type}_value`).forEach(el => {
			el.textContent = val1;
		});
		root.querySelectorAll(`.${type}_max_value`).forEach(el => {
			el.textContent = val2;
		});
		root.querySelectorAll(`.${type}_perc`).forEach(el => {
			el.textContent = `${perc}%`;
		});

		if (perc <= 0) {
			root.querySelectorAll(`.${type}_bar div`).forEach(el => {
				el.style.backgroundImage = 'none';
			});
			return;
		}

		Client.loadFile(`${DB.INTERFACE_PATH}basic_interface/gze${color}_left.bmp`, url => {
			const el = root.querySelector(`.${type}_bar_left`);
			if (el) {
				el.style.backgroundImage = `url(${url})`;
			}
		});

		Client.loadFile(`${DB.INTERFACE_PATH}basic_interface/gze${color}_mid.bmp`, url => {
			const el = root.querySelector(`.${type}_bar_middle`);
			if (el) {
				el.style.backgroundImage = `url(${url})`;
				el.style.width = `${Math.floor(Math.min(perc, 100) * barScale)}px`;
			}
		});

		Client.loadFile(`${DB.INTERFACE_PATH}basic_interface/gze${color}_right.bmp`, url => {
			const el = root.querySelector(`.${type}_bar_right`);
			if (el) {
				el.style.backgroundImage = `url(${url})`;
				el.style.left = `${Math.floor(Math.min(perc, 100) * barScale)}px`;
			}
		});
	}

	/**
	 * Update UI elements
	 *
	 * @param {string} type identifier
	 * @param {number} val1
	 * @param {number} val2 (optional)
	 */
	Component.update = function update(type, val1, val2) {
		const root = this.getRoot();
		if (!root) {
			return;
		}

		switch (type) {
			case 'name':
			case 'blvl':
			case 'jlvl':
				root.querySelectorAll(`.${type}_value`).forEach(el => {
					el.textContent = val1;
				});
				break;

			case 'zeny': {
				const zeny = val1 || 0;
				Session.zeny = zeny;
				const list = zeny.toString().split('');
				const count = list.length;
				let str = '';
				for (let i = 0; i < count; i++) {
					str = list[count - i - 1] + (i && i % 3 === 0 ? ',' : '') + str;
				}
				root.querySelectorAll(`.${type}_value`).forEach(el => {
					el.textContent = str;
				});
				break;
			}
			case 'job':
				root.querySelectorAll('.job_value').forEach(el => {
					el.textContent = MonsterTable[val1];
				});
				break;

			case 'bexp':
			case 'jexp': {
				const expEl = root.querySelector(`.${type}`);
				if (!val2) {
					if (expEl) {
						expEl.style.display = 'none';
					}
					break;
				}
				if (expEl) {
					expEl.style.display = '';
					const bar = expEl.querySelector('div');
					if (bar) {
						bar.style.width = `${Math.min(100, Math.floor((val1 * 100) / val2))}%`;
					}
					expEl.title = `${((val1 / val2) * 100).toFixed(1)}%`;
				}
				root.querySelectorAll(`.${type}_value`).forEach(el => {
					el.textContent = `${Math.min(100, (Math.floor((val1 * 1000) / val2) * 0.1).toFixed(1))}%`;
				});
				break;
			}

			case 'weight':
				root.querySelectorAll('.weight_value').forEach(el => {
					el.textContent = (val1 / 10) | 0;
				});
				root.querySelectorAll('.weight_total').forEach(el => {
					el.textContent = (val2 / 10) | 0;
				});
				root.querySelectorAll('.weight').forEach(el => {
					el.style.color = val1 < val2 / 2 ? '' : 'red';
					el.title = `${((val1 / val2) * 100).toFixed(1)}%`;
				});
				break;

			case 'hp':
			case 'sp': {
				const perc = Math.floor((val1 * 100) / val2);
				updateBar(root, type, val1, val2, perc < 25 ? 'red' : 'blue');
				break;
			}
			case 'ap': {
				if (!hasApBar) {
					break;
				}
				const perc = Math.floor((val1 * 100) / val2);
				updateBar(root, type, val1, val2, perc === 100 ? 'red' : 'blue');
				break;
			}
		}
	};

	/**
	 * Create component and export it
	 */
	return UIManager.addComponent(Component);
}
