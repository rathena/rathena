/**
 * UI/Components/EntityRoom/EntityRoom.js
 *
 * Entity room (chat room, shop room, ...)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './EntityRoom.html?raw';
import cssText from './EntityRoom.css?raw';

/**
 * Create component
 */
const EntityRoom = new GUIComponent('EntityRoom', cssText);

/**
 * Render HTML
 */
EntityRoom.render = () => htmlText;

/**
 * @var {boolean} do not focus this UI
 */
EntityRoom.needFocus = false;

/**
 * Initialize events
 */
EntityRoom.init = function init() {};

/**
 * Once in HTML
 */
EntityRoom.onAppend = function onAppend() {
	// event listeners registered here because src/Renderer/Entity/EntityRoom.js:99 overrides init (this.node = EntityRoom.clone('EntityRoom', true); this.node.init = init;)
	const root = this.getRoot();
	const btn = root.querySelector('button');

	if (btn) {
		if (this._dblclickHandler) {
			btn.removeEventListener('dblclick', this._dblclickHandler);
		}
		if (this._mousedownHandler) {
			btn.removeEventListener('mousedown', this._mousedownHandler);
		}
	}

	// Save reference for cleanup on onRemove
	this._dblclickHandler = () => {
		if (this.onEnter) {
			this.onEnter();
		}
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
EntityRoom.onRemove = function onRemove() {
	const root = this.getRoot();
	const btn = root.querySelector('button');

	// Remove the handler to avoid stacking when re-append
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
 * @param {string} url - icon url
 */
EntityRoom.setTitle = function setTitle(title, url) {
	const root = this.getRoot();
	const imgEl = root.querySelector('button img');
	const titleEl = root.querySelector('.title');
	const overlayEl = root.querySelector('.overlay');

	imgEl.src = url;
	titleEl.textContent = title;
	overlayEl.textContent = title;

	// Remove old listeners (setTitle can be called multiple times on clones)
	if (this._hoverEnter) {
		titleEl.removeEventListener('mouseenter', this._hoverEnter);
		titleEl.removeEventListener('mouseleave', this._hoverLeave);
	}

	// Only show overlay when text is truncated (ellipsis)
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
};

/**
 * function to be hooked
 */
EntityRoom.onEnter = function onEnter() {};

EntityRoom.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Stored component and return it
 */
export default UIManager.addComponent(EntityRoom);
