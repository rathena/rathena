/**
 * UI/Components/Captcha/CaptchaSelector.js
 *
 * Captcha Target Selector Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import EntityManager from 'Renderer/EntityManager.js';
import Session from 'Engine/SessionStorage.js';
import DB from 'DB/DBManager.js';
import MonsterTable from 'DB/Monsters/MonsterTable.js';
import 'UI/Elements/Elements.js';
import htmlText from './CaptchaSelector.html?raw';
import cssText from './CaptchaSelector.css?raw';

/**
 * Create Component
 */
const CaptchaSelector = new GUIComponent('CaptchaSelector', cssText);

/**
 * Preferences
 */
const _preferences = Preferences.get(
	'CaptchaSelector',
	{
		x: 230,
		y: 295
	},
	2.0
);

/**
 * AID list
 */
let _aidList = [];

/**
 * AID information
 */
let _aidInformation = [];

/**
 * Range
 */
let _range = 1;

/**
 * Active
 */
let _active = false;

CaptchaSelector.render = () => htmlText;

CaptchaSelector.captureKeyEvents = true;

/**
 * Initialize GUI
 */
CaptchaSelector.init = function init() {
	this.draggable('.titlebar');
	const root = this.getRoot();

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', () => this.remove());
	}

	const activeBtn = root.querySelector('.btn_active');
	if (activeBtn) {
		activeBtn.addEventListener('click', () => {
			_active = !_active;

			if (_active) {
				const checked = root.querySelector('input[name="target_type"]:checked');
				const type = checked ? checked.value : 'character';
				const rangeInput = root.querySelector('.range_val');
				_range = parseInt(rangeInput ? rangeInput.value : '1', 10) || 1;
				_range = Math.min(Math.max(1, _range), 9);

				if (type === 'character') {
					Session.captchaGetIdOnEntityClick = true;
					Session.captchaGetIdOnFloorClick = false;
				} else if (type === 'range') {
					Session.captchaGetIdOnFloorClick = true;
					Session.captchaGetIdOnFloorRange = _range;
					Session.captchaGetIdOnEntityClick = false;
				}
			} else {
				Session.captchaGetIdOnEntityClick = false;
				Session.captchaGetIdOnFloorClick = false;
			}
		});
	}

	const okBtn = root.querySelector('.ok');
	if (okBtn) {
		okBtn.addEventListener('click', () => {
			if (_aidList.length > 0) {
				UIManager.showPromptBox(
					DB.getMessage(2876).replace('%d', _aidList.length),
					'ok',
					'cancel',
					() => {
						CaptchaSelector.sendCaptchaToPlayers();
					},
					() => {}
				);
			}
		});
	}

	const closeCharBtn = root.querySelector('.close-character');
	if (closeCharBtn) {
		closeCharBtn.addEventListener('click', () => {
			const charInfo = root.querySelector('.character_info');
			if (charInfo) {
				charInfo.style.display = 'none';
			}
		});
	}
};

CaptchaSelector.onKeyDown = function onKeyDown(event) {
	if (CaptchaSelector.isEditableFocused()) {
		event.stopImmediatePropagation();
		return true;
	}
	return true;
};

/**
 * Append to DOM
 */
CaptchaSelector.onAppend = function onAppend() {
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth)}px`;
};

/**
 * Remove data from UI
 */
CaptchaSelector.onRemove = function onRemove() {
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();

	const root = this.getRoot();
	const charInfo = root.querySelector('.character_info');
	if (charInfo) {
		charInfo.style.display = 'none';
	}

	this.cleanUIList();

	_aidList = [];
	_aidInformation = [];
	_range = 1;
	_active = false;

	Session.captchaGetIdOnEntityClick = false;
	Session.captchaGetIdOnFloorClick = false;
	Session.captchaGetIdOnFloorRange = 1;
};

/**
 * Set player list
 * @param {Array} players - List of player AIDs
 */
CaptchaSelector.setPlayers = function setPlayers(players) {
	this.cleanUIList();
	_aidInformation = [];

	const root = this.getRoot();
	const liElements = root.querySelectorAll('.player_list li');

	players = players.filter(aid => Session.Entity.GID !== aid);

	for (let i = 0; i < players.length && i < liElements.length; i++) {
		const li = liElements[i];
		const entity = EntityManager.get(players[i]);
		const name = entity?.display?.name ?? 'Unknown';
		const aid = players[i];

		li.classList.add('player');
		li.dataset.aid = aid;
		li.innerHTML = '';

		const removeBtn = document.createElement('ui-button');
		removeBtn.classList.add('base', 'remove');
		removeBtn.setAttribute('bg', 'basic_interface/sys_close_off.bmp');
		removeBtn.setAttribute('hover', 'basic_interface/sys_close_on.bmp');
		removeBtn.dataset.aid = aid;
		removeBtn.addEventListener('click', () => {
			_aidList = _aidList.filter(item => item !== aid);
			CaptchaSelector.setPlayers(_aidList);
		});
		li.appendChild(removeBtn);

		const span = document.createElement('span');
		const link = document.createElement('a');
		link.dataset.aid = aid;
		link.textContent = name;
		link.addEventListener('click', () => {
			const charEntity = EntityManager.get(aid);
			const charName = charEntity?.display?.name ?? 'Unknown';
			const charJob = MonsterTable[charEntity?._job ?? 0] ?? 'Unknown';

			const charInfo = root.querySelector('.character_info');
			if (charInfo) {
				const nameEl = charInfo.querySelector('.character-name');
				if (nameEl) {
					nameEl.textContent = charName;
				}
				const jobEl = charInfo.querySelector('.character-job');
				if (jobEl) {
					jobEl.textContent = charJob;
				}

				charInfo.style.top = `${li.offsetTop}px`;
				charInfo.style.left = '0px';
				charInfo.style.display = 'block';
			}
		});
		span.appendChild(link);
		li.appendChild(document.createTextNode(' '));
		li.appendChild(span);

		_aidInformation.push({
			aid: aid,
			name: name,
			job: MonsterTable[entity?._job ?? 0] ?? 'Unknown'
		});
	}

	_aidList = players;
};

CaptchaSelector.cleanUIList = function cleanList() {
	const root = this.getRoot();
	root.querySelectorAll('.player_list li').forEach(li => {
		li.innerHTML = '';
		li.classList.remove('player');
		delete li.dataset.aid;
	});
};

CaptchaSelector.addPlayer = function addPlayer(aid) {
	if (_aidList.includes(aid)) {
		return;
	}
	_aidList.push(aid);
	CaptchaSelector.setPlayers(_aidList);
};

CaptchaSelector.requestPlayersIds = function requestPlayersIds(xPos, yPos) {
	if (CaptchaSelector.requestPlayersIdsInRange) {
		CaptchaSelector.requestPlayersIdsInRange(xPos, yPos, _range);
	}
};

CaptchaSelector.sendCaptchaToPlayers = function sendCaptchaToPlayers() {
	_aidList.forEach(aid => {
		if (CaptchaSelector.sendCaptchaToPlayer) {
			CaptchaSelector.sendCaptchaToPlayer(aid);
		}
	});
	this.cleanUIList();
	_aidList = [];
};

/**
 * Callbacks
 */
CaptchaSelector.requestPlayersIdsInRange = null;
CaptchaSelector.sendCaptchaToPlayer = null;

/**
 * Stored component and return it
 */
export default UIManager.addComponent(CaptchaSelector);
