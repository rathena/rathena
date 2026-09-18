/**
 * UI/Components/ContextMenu/ContextMenu.js
 *
 * Manage ContextMenu (right click on a target)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import Renderer from 'Renderer/Renderer.js';
import Mouse from 'Controls/MouseEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import cssText from './ContextMenu.css?raw';

/**
 * Create Component
 */
const ContextMenu = new GUIComponent('ContextMenu', cssText);

/**
 * Render HTML
 */
ContextMenu.render = () => '<div id="ContextMenu"><div class="menu"></div></div>';

/**
 * @var {boolean} focus this UI
 */
ContextMenu.needFocus = true;

/**
 * Initialize event handler
 */
ContextMenu.init = function init() {
	const root = this.getRoot();

	// Click anywhere on the overlay → close
	root.querySelector('#ContextMenu').addEventListener('mousedown', () => {
		ContextMenu.remove();
	});

	// Prevent menu item clicks from closing via overlay
	root.querySelector('.menu').addEventListener('mousedown', event => {
		event.stopImmediatePropagation();
	});
};

/**
 * Position menu at mouse cursor
 */
ContextMenu.onAppend = function onAppend() {
	const root = this.getRoot();
	const menu = root.querySelector('.menu');
	const width = menu.offsetWidth;
	const height = menu.offsetHeight;
	let x = Mouse.screen.x;
	let y = Mouse.screen.y;

	if (x + width > Renderer.width) {
		x = x - width;
	}

	if (y + height > Renderer.height) {
		y = y - height;
	}

	menu.style.top = y + 'px';
	menu.style.left = x + 'px';
};

/**
 * Clean up menu contents
 */
ContextMenu.onRemove = function onRemove() {
	const root = this.getRoot();
	root.querySelector('.menu').innerHTML = '';
};

/**
 * Add a clickable node to the context menu
 *
 * @param {string} text
 * @param {function} callback once clicked
 */
ContextMenu.addElement = function addElement(text, callback) {
	const root = this.getRoot();
	const item = document.createElement('div');
	item.textContent = text;
	item.addEventListener('click', () => {
		ContextMenu.remove();
		callback();
	});
	root.querySelector('.menu').appendChild(item);
};

/**
 * Add a delimiter to the links
 */
ContextMenu.nextGroup = function nextGroup() {
	const root = this.getRoot();
	root.querySelector('.menu').appendChild(document.createElement('hr'));
};

/**
 * Create component and export it
 */
export default UIManager.addComponent(ContextMenu);
