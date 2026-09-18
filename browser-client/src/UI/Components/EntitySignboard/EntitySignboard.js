/**
 * UI/Components/EntitySignboard/EntitySignboard.js
 *
 * Entity room (chat room, shop room, ...)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Client from 'Core/Client.js';
import DB from 'DB/DBManager.js';
import htmlText from './EntitySignboard.html?raw';
import cssText from './EntitySignboard.css?raw';

/**
 * Create component
 */
const EntitySignboard = new GUIComponent('EntitySignboard', cssText);

EntitySignboard.render = () => htmlText;

/**
 * @var {boolean} do not focus this UI
 */
EntitySignboard.needFocus = false;

/**
 * Once in HTML
 */
EntitySignboard.onAppend = function onAppend() {
	const root = this.getRoot();
	const btn = root.querySelector('button');

	if (btn) {
		// Cleanup previous handlers (clone reuse)
		if (this._dblclickHandler) {
			btn.removeEventListener('dblclick', this._dblclickHandler);
		}
		if (this._mousedownHandler) {
			btn.removeEventListener('mousedown', this._mousedownHandler);
		}
	}

	this._dblclickHandler = () => {
		this.onEnter();
	};

	this._mousedownHandler = e => {
		e.stopImmediatePropagation();
		e.preventDefault();
	};

	if (btn) {
		btn.addEventListener('dblclick', this._dblclickHandler);
		btn.addEventListener('mousedown', this._mousedownHandler);
	}

	this._host.style.zIndex = '45';
};

/**
 * Remove data from UI
 */
EntitySignboard.onRemove = function onRemove() {
	const root = this.getRoot();
	const btn = root.querySelector('button');

	if (btn) {
		if (this._dblclickHandler) {
			btn.removeEventListener('dblclick', this._dblclickHandler);
			this._dblclickHandler = null;
		}
		if (this._mousedownHandler) {
			btn.removeEventListener('mousedown', this._mousedownHandler);
			this._mousedownHandler = null;
		}
	}
};

/**
 * Define title and icons
 *
 * @param {string} title
 * @param {string} icon_location - icon url
 */
EntitySignboard.setTitle = function setTitle(title, icon_location) {
	const root = this.getRoot();
	const signboard = root.querySelector('.EntitySignboard');

	// Load signboard background
	Client.loadFile(`${DB.INTERFACE_PATH}signboard/bg_signboard.bmp`, url => {
		signboard.style.backgroundImage = `url('${url}')`;
	});

	const titleEl = root.querySelector('.title');
	const overlayEl = root.querySelector('.overlay');

	titleEl.textContent = title;
	overlayEl.textContent = title;
	titleEl.style.display = 'inline-block';

	// Remove old listeners (setTitle can be called multiple times on clones)
	if (this._hoverEnter) {
		titleEl.removeEventListener('mouseenter', this._hoverEnter);
		titleEl.removeEventListener('mouseleave', this._hoverLeave);
	}
	// Show overlay only when text is truncated
	this._hoverEnter = () => {
		if (titleEl.scrollWidth > titleEl.clientWidth) {
			overlayEl.style.display = 'block';
		}
	};
	this._hoverLeave = () => {
		overlayEl.style.display = 'none';
	};
	titleEl.addEventListener('mouseenter', this._hoverEnter);
	titleEl.addEventListener('mouseleave', this._hoverLeave);

	// Load icon
	Client.loadFile(icon_location, url => {
		root.querySelector('button').style.backgroundImage = `url('${url}')`;
	});
};

/**
 * Set icon only mode (no title text)
 *
 * @param {string} icon_location - icon url
 */
EntitySignboard.setIconOnly = function setIconOnly(icon_location) {
	const root = this.getRoot();
	root.querySelector('.title').style.display = 'none';
	root.querySelector('.overlay').style.display = 'none';

	Client.loadFile(icon_location, url => {
		const btn = root.querySelector('button');
		btn.classList.add('icon-only');
		btn.style.backgroundImage = `url('${url}')`;
	});
};

/**
 * function to define
 */
EntitySignboard.onEnter = function onEnter() {};

EntitySignboard.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Stored component and return it
 */
export default UIManager.addComponent(EntitySignboard);
