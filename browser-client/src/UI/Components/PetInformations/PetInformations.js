/**
 * UI/Components/PetInformations/PetInformations.js
 *
 * Display pet informations
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './PetInformations.html?raw';
import cssText from './PetInformations.css?raw';
import KEYS from 'Controls/KeyEventHandler.js';
import PACKETVER from 'Network/PacketVerManager.js';

/**
 * Create Component
 */
const PetInformations = new GUIComponent('PetInformations', cssText);

PetInformations.render = () => htmlText;

PetInformations.captureKeyEvents = true;

/**
 * @var {Preferences} Window preferences
 */
const _preferences = Preferences.get(
	'PetInformations',
	{
		x: 100,
		y: 200,
		show: true
	},
	1.0
);

/**
 * Auto Feed Flag
 */
let petAutoFeeding = 0;

/**
 * Initialize component
 */
PetInformations.init = function init() {
	this.draggable('.titlebar');

	const root = PetInformations.getRoot();

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		closeBtn.addEventListener('click', () => {
			this._host.style.display = 'none';
		});
	}

	const modifyBtn = root.querySelector('.modify');
	if (modifyBtn) {
		modifyBtn.addEventListener('click', () => {
			const input = root.querySelector('.name');
			if (input) {
				PetInformations.reqNameEdit(input.value);
			}
		});
	}

	const commandSelect = root.querySelector('.command');
	if (commandSelect) {
		commandSelect.addEventListener('change', function () {
			const value = this.value;

			if (value.startsWith('evolution_')) {
				const baseJobID = Number(value.replace('evolution_', ''));
				PetInformations.reqEvolution(baseJobID);
				this.value = 'default';
				return;
			}

			switch (value) {
				case 'feed':
					PetInformations.reqPetFeed();
					break;

				case 'action':
					PetInformations.reqPetAction();
					break;

				case 'release':
					PetInformations.reqBackToEgg();
					break;

				case 'unequip':
					PetInformations.reqUnEquipPet();
					break;

				default:
			}

			this.value = 'default';
		});
	}

	const autoFeedBtn = root.querySelector('.pet_auto_feed');
	if (autoFeedBtn) {
		autoFeedBtn.addEventListener('mousedown', () => {
			PetInformations.onConfigUpdate(2, !petAutoFeeding ? 1 : 0);
		});
	}
};

PetInformations.onAppend = function onAppend() {
	const root = PetInformations.getRoot();

	Client.loadFile(DB.INTERFACE_PATH + 'checkbox_' + (petAutoFeeding ? '1' : '0') + '.bmp', data => {
		const el = root.querySelector('.pet_auto_feed');
		if (el) {
			el.style.backgroundImage = `url(${data})`;
		}
	});

	if (PACKETVER.value < 20141008) {
		const feeding = root.querySelector('.feeding');
		if (feeding) {
			feeding.style.display = 'none';
		}
	}

	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.getBoundingClientRect().height)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.getBoundingClientRect().width)}px`;
};

/**
 * Once remove from body, save user preferences
 */
PetInformations.onRemove = function onRemove() {
	_preferences.show = this._host.style.display !== 'none';
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();
};

/**
 * Process shortcut
 *
 * @param {object} key
 */
PetInformations.onShortCut = function onShortCut(key) {
	switch (key.cmd) {
		case 'TOGGLE':
			if (this._host.style.display === 'none') {
				this._host.style.display = '';
				this.focus();
			} else {
				this._host.style.display = 'none';
			}
			break;
	}
};

/**
 * Update UI
 *
 * @param {object} pet info
 */
PetInformations.setInformations = function setInformations(info) {
	const root = PetInformations.getRoot();

	const nameInput = root.querySelector('.name');
	if (nameInput) {
		nameInput.value = info.szName;
	}

	const levelEl = root.querySelector('.level');
	if (levelEl) {
		levelEl.textContent = info.nLevel;
	}

	this.setHunger(info.nFullness);
	this.setIntimacy(info.nRelationship);

	const accessoryEl = root.querySelector('.accessory');
	if (accessoryEl) {
		accessoryEl.textContent = DB.getMessage(info.ITID ? 598 : 600);
	}

	Client.loadFile(DB.getPetIllustPath(info.job), data => {
		const contentEl = root.querySelector('.content');
		if (contentEl) {
			contentEl.style.backgroundImage = `url(${data})`;
		}
	});

	if (!info.bModified) {
		if (nameInput) {
			nameInput.classList.remove('disabled');
			nameInput.disabled = false;
		}
		const modifyBtn = root.querySelector('.modify');
		if (modifyBtn) {
			modifyBtn.classList.remove('disabled');
			modifyBtn.disabled = false;
		}
	} else {
		if (nameInput) {
			nameInput.classList.add('disabled');
			nameInput.disabled = true;
		}
		const modifyBtn = root.querySelector('.modify');
		if (modifyBtn) {
			modifyBtn.classList.add('disabled');
			modifyBtn.disabled = true;
		}
	}

	const cmdSelect = root.querySelector('.command');

	// remove old evolution options
	if (cmdSelect) {
		const oldOptions = cmdSelect.querySelectorAll('.evolution-option');
		for (const opt of oldOptions) {
			opt.remove();
		}

		const evolution = DB.getPetEvolutionByJob(info.job);

		if (evolution) {
			for (const targetEggID in evolution) {
				const evoPet = DB.getPetByEggID(Number(targetEggID));
				const evoName = evoPet ? evoPet.PetString || evoPet.PetName : `Evolution ${targetEggID}`;

				const option = document.createElement('option');
				option.className = 'evolution-option';
				option.value = `evolution_${info.job}`;
				option.textContent = `Evolution - ${evoName}`;

				cmdSelect.appendChild(option);
			}
		}
	}
};

/**
 * Set intimacy
 *
 * @param {number} intimacy
 */
PetInformations.setIntimacy = function setIntimacy(val) {
	const root = PetInformations.getRoot();
	const el = root.querySelector('.intimacy');
	if (el) {
		el.textContent = DB.getMessage(val < 100 ? 672 : val < 250 ? 673 : val < 600 ? 669 : val < 900 ? 674 : 675);
	}
};

PetInformations.setFeedConfig = function setFeedConfig(flag) {
	petAutoFeeding = flag;

	const root = PetInformations.getRoot();
	if (root) {
		Client.loadFile(DB.INTERFACE_PATH + 'checkbox_' + (petAutoFeeding ? '1' : '0') + '.bmp', data => {
			const el = root.querySelector('.pet_auto_feed');
			if (el) {
				el.style.backgroundImage = `url(${data})`;
			}
		});
	}
};

/**
 * Set hunger value
 *
 * @param {number} hunger
 */
PetInformations.setHunger = function setHunger(val) {
	const root = PetInformations.getRoot();
	const el = root.querySelector('.hunger');
	if (el) {
		el.textContent = DB.getMessage(val < 10 ? 667 : val < 25 ? 668 : val < 75 ? 669 : val < 90 ? 670 : 671);
	}
};

/**
 * Handle keyboard events inside shadow DOM
 * Protects input fields from being consumed by global handlers
 */
PetInformations.onKeyDown = function onKeyDown(event) {
	const focused = this.getRoot().activeElement;

	if (this.isEditableFocused()) {
		if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
			focused.blur();
			event.stopImmediatePropagation();
			return false;
		}
		if (event.which === KEYS.ENTER) {
			if (focused.classList.contains('name')) {
				PetInformations.reqNameEdit(focused.value);
			}
			event.stopImmediatePropagation();
			return false;
		}
		event.stopImmediatePropagation();
		return true;
	}

	return true;
};

/**
 * Functions defined in Engine/MapEngine/Pet.js
 */
PetInformations.reqPetFeed = function reqPetFeed() {};
PetInformations.reqPetAction = function reqPetAction() {};
PetInformations.reqNameEdit = function reqNameEdit() {};
PetInformations.reqUnEquipPet = function reqUnEquipPet() {};
PetInformations.reqBackToEgg = function reqBackToEgg() {};
PetInformations.reqEvolution = function reqEvolution() {};
PetInformations.onConfigUpdate = function onConfigUpdate(/* type, value*/) {};

/**
 * Create component and export it
 */
export default UIManager.addComponent(PetInformations);
