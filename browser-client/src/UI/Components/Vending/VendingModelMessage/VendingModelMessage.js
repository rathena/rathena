/**
 * UI/Components/Vending/VendingModelMessage/VendingModelMessage.js
 *
 * VendingModelMessage windows
 *
 * @author Francisco Wallison
 */

import DB from 'DB/DBManager.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './VendingModelMessage.html?raw';
import cssText from './VendingModelMessage.css?raw';

/**
 * Create VendingModelMessage namespace
 */
const VendingModelMessage = new GUIComponent('VendingModelMessage', cssText);

VendingModelMessage.render = () => htmlText;

/**
 * Initialize UI
 */
VendingModelMessage.init = function init() {
	// Show at center.
	Object.assign(this._host.style, {
		top: (Renderer.height - 200) / 2 + 'px',
		left: (Renderer.width - 200) / 2 + 'px'
	});

	this._shadow.querySelector('.ok').addEventListener('click', e => {
		e.stopImmediatePropagation();
		VendingModelMessage.onRemove();
	});
	this.draggable('.titlebar');
};

VendingModelMessage.setInit = function setInit(numMessage) {
	VendingModelMessage.append();
	this._host.style.display = '';
	const messageText = DB.getMessage(numMessage);
	this._shadow.querySelector('.message').textContent = messageText;
};

VendingModelMessage.onRemove = function onRemove() {
	if (!this._host) {
		return;
	}

	this._host.style.display = 'none';
};

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(VendingModelMessage);
