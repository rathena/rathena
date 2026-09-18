/**
 * UI/Components/ShortCuts/ShortCuts.js
 *
 * Chararacter ShortCuts
 *
 * @author Francisco Wallison
 *
 */

import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import Mouse from 'Controls/MouseEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Emoticons from 'UI/Components/Emoticons/Emoticons.js';
import htmlText from './ShortCuts.html?raw';
import cssText from './ShortCuts.css?raw';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import ProcessCommand from 'Controls/ProcessCommand.js';
import KEYS from 'Controls/KeyEventHandler.js';

/**
 * Create Component
 */
const ShortCuts = new GUIComponent('ShortCuts', cssText);
ShortCuts.render = () => htmlText;

/**
 * @var {Preferences} structure
 */
const _MACRO_INIT = Preferences.get(
	'_MACRO_CMD',
	{
		Num_1: '/hide',
		Num_2: '/?',
		Num_3: '/ho',
		Num_4: '/lv',
		Num_5: '/swt',
		Num_6: '/ic',
		Num_7: '/an',
		Num_8: '/ag',
		Num_9: '/$',
		Num_0: '/...'
	},
	1.0
);

// Fixed, can't change them
const _FLAG_INIT = {
	Num_1: 13, //ET_FLAG
	Num_2: 35, //ET_INDONESIA_FLAG
	Num_3: 48, // ET_PH_FLAG
	Num_4: 49, //ET_MY_FLAG
	Num_5: 50, //ET_SI_FLAG
	Num_6: 51, //ET_BR_FLAG
	Num_7: 64, //ET_INDIA_FLAG
	Num_8: 66, //ET_FLAG8
	Num_9: 67 //ET_FLAG9
};

/**
 * Store ShortCuts items
 */
ShortCuts.list = [];

/**
 * @var {number} used to remember the window height
 */
const _realSize = 0;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'ShortCuts',
	{
		x: 0,
		y: 172,
		width: 7,
		height: 4,
		show: false,
		reduce: false,
		magnet_top: false,
		magnet_bottom: false,
		magnet_left: true,
		magnet_right: false
	},
	1.0
);

/**
 * Capture key events so text inputs inside Shadow DOM work correctly
 */
ShortCuts.captureKeyEvents = true;

/**
 * Key handler: let typing work in macro inputs, handle Enter/Escape
 */
ShortCuts.onKeyDown = function onKeyDown(event) {
	const focused = this.getRoot().activeElement;

	if (this.isEditableFocused()) {
		if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
			focused.blur();
			event.stopImmediatePropagation();
			return false;
		}
		// Block other handlers from consuming the keystroke, but let the browser type it
		event.stopImmediatePropagation();
		return true;
	}

	return true;
};

/**
 * Initialize UI
 */
ShortCuts.init = function init() {
	const root = ShortCuts.getRoot();

	const footerBtns = root.querySelectorAll('.footer button');
	footerBtns.forEach(btn => {
		btn.addEventListener('mousedown', () => {
			if (btn.classList.contains('emoticons')) {
				Emoticons.onShortCut({ cmd: 'TOGGLE' });
			}
		});
	});

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', onClose);
	}

	const macroInputs = root.querySelectorAll('.macro input');
	macroInputs.forEach(input => {
		input.addEventListener('mousedown', () => {
			root.querySelectorAll('.macro_').forEach(el => el.classList.remove('input_macro_focus'));
			input.classList.add('input_macro_focus');
			input.select();
		});
	});

	root.querySelectorAll('input[type=text]').forEach(input => {
		input.addEventListener('drop', onDropText);
		input.addEventListener('dragover', e => {
			e.stopImmediatePropagation();
			e.preventDefault();
		});
	});

	addValuesAlt();
	loadValuesAlt();

	this.draggable('.titlebar');
};

/**
 * Apply preferences once append to body
 */
ShortCuts.onAppend = function onAppend() {
	if (!_preferences.show) {
		this._host.style.display = 'none';
	}

	const rect = this._host.getBoundingClientRect();
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - rect.height)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - rect.width)}px`;
};

/**
 * Remove ShortCuts from window (and so clean up items)
 */
ShortCuts.onRemove = function onRemove() {
	const root = ShortCuts.getRoot();
	const content = root.querySelector('.container .content');
	if (content) {
		content.innerHTML = '';
	}
	this.list.length = 0;

	// Clean up any leftover ItemInfo elements in the global DOM
	document.querySelectorAll('.ItemInfo').forEach(el => el.remove());

	// Save preferences
	_preferences.show = this._host.style.display !== 'none';
	_preferences.reduce = !!_realSize;
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	const hostRect = this._host.getBoundingClientRect();
	_preferences.width = Math.floor((hostRect.width - (23 + 16 + 16 - 30)) / 32);
	_preferences.height = Math.floor((hostRect.height - (31 + 19 - 30)) / 32);
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
ShortCuts.onShortCut = function onShortCut(key) {
	switch (key.cmd) {
		case 'TOGGLE':
			if (this._host.style.display === 'none') {
				this._host.style.display = '';
				this._fixPositionOverflow();
			} else {
				this._host.style.display = 'none';
			}
			// Remove input focus
			this.getRoot()
				.querySelectorAll('.macro_')
				.forEach(el => el.classList.remove('input_macro_focus'));
			if (this._host.style.display !== 'none') {
				this.focus();
			}
			break;
		// Macros
		case 'EXECUTE_MACRO_1':
			executeCmd(1);
			break;
		case 'EXECUTE_MACRO_2':
			executeCmd(2);
			break;
		case 'EXECUTE_MACRO_3':
			executeCmd(3);
			break;
		case 'EXECUTE_MACRO_4':
			executeCmd(4);
			break;
		case 'EXECUTE_MACRO_5':
			executeCmd(5);
			break;
		case 'EXECUTE_MACRO_6':
			executeCmd(6);
			break;
		case 'EXECUTE_MACRO_7':
			executeCmd(7);
			break;
		case 'EXECUTE_MACRO_8':
			executeCmd(8);
			break;
		case 'EXECUTE_MACRO_9':
			executeCmd(9);
			break;
		case 'EXECUTE_MACRO_0':
			executeCmd(0);
			break;

		// Flags
		case 'EXECUTE_FLAG_1':
			executeFlag(1);
			break;
		case 'EXECUTE_FLAG_2':
			executeFlag(2);
			break;
		case 'EXECUTE_FLAG_3':
			executeFlag(3);
			break;
		case 'EXECUTE_FLAG_4':
			executeFlag(4);
			break;
		case 'EXECUTE_FLAG_5':
			executeFlag(5);
			break;
		case 'EXECUTE_FLAG_6':
			executeFlag(6);
			break;
		case 'EXECUTE_FLAG_7':
			executeFlag(7);
			break;
		case 'EXECUTE_FLAG_8':
			executeFlag(8);
			break;
		case 'EXECUTE_FLAG_9':
			executeFlag(9);
			break;
	}
};

/**
 * Extend ShortCuts window size
 *
 * @param {number} width
 * @param {number} height
 */
ShortCuts.resize = function resize(width, height) {
	width = Math.min(Math.max(width, 6), 9);
	height = Math.min(Math.max(height, 2), 6);

	const root = ShortCuts.getRoot();
	const content = root.querySelector('.container .content');
	if (content) {
		content.style.width = `${width * 32 + 13}px`;
		content.style.height = `${height * 32}px`;
	}

	this._host.style.width = `${23 + 16 + 16 + width * 32}px`;
	this._host.style.height = `${31 + 19 + height * 32}px`;
};

/**
 * Exit window
 */
function onClose() {
	ShortCuts._host.style.display = 'none';
}

/**
 * Extend ShortCuts window size
 */
// Currently unused, preserved for future development.
function _onResize() {
	const host = ShortCuts._host;
	const root = ShortCuts.getRoot();
	const content = root.querySelector('.container .content');
	const hide = root.querySelector('.hide');
	const top = host.offsetTop;
	const left = host.offsetLeft;
	let lastWidth = 0;
	let lastHeight = 0;

	function resizing() {
		const extraX = 23 + 16 + 16 - 30;
		const extraY = 31 + 19 - 30;

		let w = Math.floor((Mouse.screen.x - left - extraX) / 32);
		let h = Math.floor((Mouse.screen.y - top - extraY) / 32);

		// Maximum and minimum window size
		w = Math.min(Math.max(w, 6), 9);
		h = Math.min(Math.max(h, 2), 6);

		if (w === lastWidth && h === lastHeight) {
			return;
		}

		ShortCuts.resize(w, h);
		lastWidth = w;
		lastHeight = h;

		// Show or hide scrollbar
		if (content && content.offsetHeight === content.scrollHeight) {
			if (hide) {
				hide.style.display = '';
			}
		} else {
			if (hide) {
				hide.style.display = 'none';
			}
		}
	}

	// Start resizing
	const _Interval = setInterval(resizing, 30);

	// Stop resizing on left click
	const mouseUpHandler = event => {
		if (event.which === 1) {
			clearInterval(_Interval);
			window.removeEventListener('mouseup', mouseUpHandler);
		}
	};
	window.addEventListener('mouseup', mouseUpHandler);
}

function executeCmd(value) {
	const command = _MACRO_INIT[`Num_${value}`];

	// Nothing to submit
	if (command.length < 1 || command === '/hide') {
		return;
	}

	// Process commands
	if (command[0] === '/') {
		ProcessCommand.processCommand.call(ChatBox, command.substr(1));
		return;
	}

	ChatBox.onRequestTalk('', command, ChatBox.sendTo);
}

function executeFlag(value) {
	const command = _FLAG_INIT[`Num_${value}`];

	// Nothing to submit
	if (command.length < 1) {
		return;
	}

	const pkt = new PACKET.CZ.REQ_EMOTION();
	pkt.type = command;
	Network.sendPacket(pkt);
}

function loadValuesAlt() {
	const root = ShortCuts.getRoot();
	const length = Object.keys(_MACRO_INIT).length - 3;
	for (let index = 0; index < length; index++) {
		const element = _MACRO_INIT[`Num_${index}`];
		const input = root.querySelector(`#macro_${index}`);
		if (input) {
			input.value = element;
		}
	}
}

function addValuesAlt() {
	const root = ShortCuts.getRoot();
	root.querySelectorAll('.macro input').forEach(input => {
		input.addEventListener('blur', () => {
			const index = input.id.split('macro_')[1];
			_MACRO_INIT[`Num_${index}`] = input.value;
			_MACRO_INIT.save();
		});
	});
}

/**
 * Validate the type of information being dropped into the text field
 */
function onDropText(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
	let data;
	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
	} catch (_e) {
		return;
	}

	// Valid if the message type
	if (data.type === 'item') {
		return;
	}

	event.currentTarget.value = data;
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(ShortCuts);
