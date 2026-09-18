/**
 * UI/Components/ChatRoomCreate/ChatRoomCreate.js
 *
 * Character room setup UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */
import DB from 'DB/DBManager.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Renderer from 'Renderer/Renderer.js';
import Preferences from 'Core/Preferences.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './ChatRoomCreate.html?raw';
import cssText from './ChatRoomCreate.css?raw';
import NpcBox from 'UI/Components/NpcBox/NpcBox.js';
import NpcMenu from 'UI/Components/NpcMenu/NpcMenu.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';

/**
 * Create Component
 */
const ChatRoomCreate = new GUIComponent('ChatRoomCreate', cssText);

/**
 * Render HTML
 */
ChatRoomCreate.render = () => htmlText;

/**
 * @var {string} chat room title
 */
ChatRoomCreate.title = '';

/**
 * @var {number} chat room limit
 */
ChatRoomCreate.limit = 20;

/**
 * @var {number} type
 * 0 = Private
 * 1 = Public
 */
ChatRoomCreate.type = 1;

/**
 * @var {string} password
 */
ChatRoomCreate.password = '';

/**
 * @var {Preference} structure to save
 */
const _preferences = Preferences.get(
	'ChatRoomCreate',
	{
		x: 480,
		y: 200,
		show: false
	},
	1.0
);

/**
 * Initialize UI
 */
ChatRoomCreate.init = function init() {
	const root = this.getRoot();

	// Close / Cancel
	root.querySelector('.close').addEventListener('mousedown', event => {
		event.stopImmediatePropagation();
	});
	root.querySelector('.close').addEventListener('click', () => ChatRoomCreate.hide());

	root.querySelector('.cancel').addEventListener('mousedown', event => {
		event.stopImmediatePropagation();
	});
	root.querySelector('.cancel').addEventListener('click', () => ChatRoomCreate.hide());

	// OK button
	root.querySelector('.ok').addEventListener('click', () => parseChatSetup.call(ChatRoomCreate));

	// Prevent form submit
	root.querySelector('.setup').addEventListener('submit', event => {
		event.preventDefault();
	});

	// Prevent inputs from moving character
	root.querySelectorAll('input, select').forEach(el => {
		el.addEventListener('mousedown', event => {
			event.stopImmediatePropagation();
		});
	});

	this.draggable('.titlebar');
	this._host.style.display = 'none';
};

/**
 * Once append to body
 */
ChatRoomCreate.onAppend = function onAppend() {
	if (!_preferences.show) {
		this._host.style.display = 'none';
	}

	this._host.style.top = Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight) + 'px';
	this._host.style.left = Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth) + 'px';
};

/**
 * Once removed from DOM, save preferences
 */
ChatRoomCreate.onRemove = function onRemove() {
	_preferences.show = this._host.style.display !== 'none';
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();
	ChatRoomCreate.editMode = false;
};

/**
 * Show the setup for room creation
 */
ChatRoomCreate.show = function showSetup() {
	this._host.style.display = '';
	const root = this.getRoot();
	root.querySelector('.title').focus();
	this._fixPositionOverflow();

	_preferences.show = true;
};

/**
 * Hide the setup ui
 */
ChatRoomCreate.hide = function hideSetup() {
	this._host.style.display = 'none';
	const root = this.getRoot();
	root.querySelector('.setup').reset();
	ChatRoomCreate.editMode = false;
	_preferences.show = false;
};

/**
 * Pre-fill form with values (used by ChatRoom.openRoomSettings)
 * @param {string} title
 * @param {number} limit
 * @param {number} type
 * @param {string} password
 */
ChatRoomCreate.prefill = function prefill(title, limit, type) {
	const root = this.getRoot();
	root.querySelector('input[name=title]').value = title ?? '';
	root.querySelector('select[name=limit]').value = limit ?? 20;
	const radio = root.querySelector('input[name=public][value="' + (type ?? 1) + '"]');
	if (radio) radio.checked = true;

	root.querySelector('input[name=password]').value = '';
};

/**
 * Key Listener
 *
 * @param {object} event
 * @return {boolean}
 */
ChatRoomCreate.onKeyDown = function onKeyDown(event) {
	if (InputBox._host && InputBox._host.style.display !== 'none' && InputBox.__active) {
		return true;
	}

	if (NpcMenu._host && NpcMenu._host.style.display !== 'none' && NpcMenu.__active) {
		return true;
	}

	if (NpcBox._host && NpcBox._host.style.display !== 'none' && NpcBox.__active) {
		return true;
	}

	// Guard: don't intercept keys when hidden
	if (this._host.style.display === 'none') {
		return true;
	}

	// Input inside our shadow is focused — protect keystrokes
	if (this.isEditableFocused()) {
		if (event.which === KEYS.ENTER) {
			parseChatSetup.call(this);
			event.stopImmediatePropagation();
			return false;
		}
		if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
			ChatRoomCreate.hide();
			event.stopImmediatePropagation();
			return false;
		}
		// Block other window handlers from consuming the key
		event.stopImmediatePropagation();
		return true; // Don't preventDefault — let the character be typed
	}

	// No input focused — only handle Escape
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		ChatRoomCreate.hide();
		event.stopImmediatePropagation();
		return false;
	}
	return true;
};

/**
 * Process shortcut
 *
 * @param {object} key
 */
ChatRoomCreate.onShortCut = function onShortCut(key) {
	switch (key.cmd) {
		case 'TOGGLE':
			ChatRoomCreate.toggle();
			break;
	}
};

ChatRoomCreate.toggle = function toggle() {
	if (this._host.style.display === 'none') {
		this._host.style.display = '';
		this._fixPositionOverflow();
		this.focus();
	} else {
		this._host.style.display = 'none';
	}
};

/**
 * Parse and send chat room request
 */
function parseChatSetup() {
	const root = ChatRoomCreate.getRoot();

	this.title = root.querySelector('input[name=title]').value;
	this.limit = parseInt(root.querySelector('select[name=limit]').value, 10);
	this.type = parseInt(root.querySelector('input[name=public]:checked').value, 10);
	this.password = root.querySelector('input[name=password]').value;

	if (this.title.length < 1) {
		const overlay = document.createElement('div');
		overlay.className = 'win_popup_overlay';
		document.body.appendChild(overlay);

		const popup = UIManager.showMessageBox(
			DB.getMessage(13),
			'ok',
			() => {
				document.body.removeChild(overlay);
			},
			true
		);
		popup._host.style.top = parseInt(this._host.style.top, 10) - 120 + 'px';
		popup._host.style.left = parseInt(this._host.style.left, 10) + 'px';
		return;
	}

	this.requestRoom();
	this.hide();
}

/**
 * Pseudo functions :)
 */
ChatRoomCreate.requestRoom = function requestRoom() {};

ChatRoomCreate.mouseMode = GUIComponent.MouseMode.STOP;
ChatRoomCreate.captureKeyEvents = true;
ChatRoomCreate.needFocus = true;
ChatRoomCreate.editMode = false;
/**
 * Create component and export it
 */
export default UIManager.addComponent(ChatRoomCreate);
