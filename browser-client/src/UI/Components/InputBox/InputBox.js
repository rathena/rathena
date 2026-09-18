/**
 * UI/Components/InputBox/InputBox.js
 *
 * NPC input GUI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Renderer from 'Renderer/Renderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import DB from 'DB/DBManager.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './InputBox.html?raw';
import cssText from './InputBox.css?raw';

/**
 * Create InputBox component
 */
const InputBox = new GUIComponent('InputBox', cssText);

InputBox.render = () => htmlText;

/**
 * Freeze mouse — modal dialog
 */
InputBox.mouseMode = GUIComponent.MouseMode.FREEZE;

/**
 * Capture key events to allow typing in the input
 */
InputBox.captureKeyEvents = true;

/**
 * Initialize GUI
 */
InputBox.init = function init() {
	this.draggable();
	this._host.style.top = `${(Renderer.height - 120) / 1.5 - 49}px`;
	this._host.style.left = `${(Renderer.width - 280) / 2 + 1}px`;

	const root = InputBox.getRoot();

	const btn = root.querySelector('ui-button');
	if (btn) {
		btn.addEventListener('click', () => validate());
	}

	const input = root.querySelector('input');
	if (input) {
		input.addEventListener('mousedown', e => e.stopImmediatePropagation());
	}

	this._overlay = document.createElement('div');
	this._overlay.className = 'win_popup_overlay';
	this._overlay.style.zIndex = '30';
	this._overlay.style.position = 'fixed';
	this._overlay.style.top = '0';
	this._overlay.style.left = '0';
	this._overlay.style.width = '100%';
	this._overlay.style.height = '100%';
	this._overlay.addEventListener('click', () => {
		InputBox.remove();
	});
};

/**
 * Input Post-Render callback
 */
InputBox.onAppend = function onAppend() {
	const root = InputBox.getRoot();
	const input = root.querySelector('input');
	if (input) {
		input.focus();
		if (input.value) {
			input.select();
		}
	}
};

/**
 * Remove data from UI
 */
InputBox.onRemove = function onRemove() {
	const root = InputBox.getRoot();
	const input = root.querySelector('input');
	if (input) {
		input.value = '';
		input.removeEventListener('keydown', null);
	}
	const text = root.querySelector('.text');
	if (text) {
		text.textContent = '';
	}
	if (this._overlay && this._overlay.parentNode) {
		this._overlay.remove();
	}
};

/**
 * Key Listener
 *
 * @param {object} event
 * @return {boolean}
 */
InputBox.onKeyDown = function onKeyDown(event) {
	if (event.which === KEYS.ENTER) {
		validate();
		event.stopImmediatePropagation();
		return false;
	}
	if (this.isEditableFocused()) {
		event.stopImmediatePropagation();
		return true;
	}
	return true;
};

/**
 * Validate input
 */
function validate() {
	const root = InputBox.getRoot();
	const input = root.querySelector('input');
	let text = input ? input.value : '';

	if (!InputBox.isPersistent || text.length) {
		const innerRoot = root.querySelector('#inputbox');
		if (innerRoot && innerRoot.classList.contains('number')) {
			text = parseInt(text, 10) | 0;
		}

		InputBox.onSubmitRequest(text);
	}
}

/**
 * Set input type
 *
 * @param {string} type (number or text)
 * @param {boolean} isPersistent
 * @param {string|number} defaultVal
 * @param {number} itemId
 */
InputBox.setType = function setType(type, isPersistent, defaultVal, itemId = null) {
	this.isPersistent = !!isPersistent;
	const root = InputBox.getRoot();
	const innerRoot = root.querySelector('#inputbox');
	const textEl = root.querySelector('.text');
	const input = root.querySelector('input');

	if (!this.isPersistent) {
		document.body.appendChild(this._overlay);
	}

	switch (type) {
		case 'number':
			innerRoot.classList.add('number');
			if (textEl) {
				textEl.textContent = DB.getMessage(1259);
			}
			if (input) {
				input.type = 'text';
			}
			defaultVal = defaultVal || 0;
			break;

		case 'price':
			innerRoot.classList.add('number');
			if (textEl) {
				textEl.textContent = 'Input Price';
			}
			if (input) {
				input.type = 'text';
			}
			defaultVal = defaultVal || 0;
			break;

		case 'text':
			innerRoot.classList.remove('number');
			if (textEl) {
				textEl.textContent = '';
			}
			if (input) {
				input.type = 'text';
			}
			break;

		case 'shopname':
			innerRoot.classList.remove('number');
			if (textEl) {
				textEl.textContent = 'Input your Shop Name';
			}
			if (input) {
				input.type = 'text';
			}
			break;

		case 'pass':
			innerRoot.classList.remove('number');
			if (textEl) {
				textEl.textContent = '';
			}
			if (input) {
				input.type = 'password';
			}
			break;

		case 'mail':
			innerRoot.classList.remove('number');
			if (textEl) {
				textEl.textContent = DB.getMessage(300);
			}
			if (input) {
				input.type = 'password';
			}
			break;

		case 'birthdate':
			innerRoot.classList.remove('number');
			if (textEl) {
				textEl.textContent = DB.getMessage(1815);
			}
			if (input) {
				input.type = 'text';
			}
			break;

		case 'item':
			innerRoot.classList.add('number');
			if (textEl) {
				textEl.textContent = DB.getItemInfo(itemId).identifiedDisplayName;
			}
			if (input) {
				input.type = 'text';
			}
			defaultVal = defaultVal || 0;
			break;
	}

	if (typeof defaultVal !== 'undefined' && input) {
		input.value = defaultVal;
		input.select();
	}
};

/**
 * Callback to define
 */
InputBox.onSubmitRequest = function onSubmitRequest() {};

/**
 * Stored component and return it
 */
export default UIManager.addComponent(InputBox);
