/**
 * UI/Components/ItemObtain/ItemObtain.js
 *
 * Item Obtain window (when you get an item, a window popup near the announce box)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Events from 'Core/Events.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './ItemObtain.html?raw';
import cssText from './ItemObtain.css?raw';

/**
 * Create component
 */
const ItemObtain = new GUIComponent('ItemObtain', cssText);

ItemObtain.render = () => htmlText;

/**
 * Mouse can cross this UI
 */
ItemObtain.mouseMode = GUIComponent.MouseMode.CROSS;

/**
 * @var {boolean} do not focus this UI
 */
ItemObtain.needFocus = false;

/**
 * Sanitize HTML, allowing only whitelisted tags (font, i, b)
 */
function _sanitizeHtml(str) {
	const whitelist = ['font', 'i', 'b'];
	const div = document.createElement('div');
	div.innerHTML = str;
	div.querySelectorAll('*').forEach(el => {
		if (!whitelist.includes(el.tagName.toLowerCase())) {
			el.replaceWith(...el.childNodes);
		}
	});
	return div.innerHTML;
}

/**
 * @var {TimeOut} timer
 */
let _timer = 0;

/**
 * @var {number} time to display
 */
const _life = 5 * 1000;

/**
 * Initialize component
 */
ItemObtain.init = function init() {
	// this._host.style.zIndex = '45'; // Between Interface and Game Announce
};

/**
 * Once append to body
 */
ItemObtain.onAppend = function onAppend() {
	const root = this.getRoot();
	const el = root.querySelector('#ItemObtain');
	this._host.style.left = `${(Renderer.width - (el ? el.offsetWidth : 0)) >> 1}px`;
};

/**
 * Once removed from HTML, clean timer
 */
ItemObtain.onRemove = function onRemove() {
	if (_timer) {
		Events.clearTimeout(_timer);
		_timer = 0;
	}
};

/**
 * Timer end, cleaning box
 */
ItemObtain.timeEnd = function timeEnd() {
	this.remove();
};

/**
 * Add item informations
 *
 * @param {object} item
 */
ItemObtain.set = function set(item) {
	const root = this.getRoot();
	const it = DB.getItemInfo(item.ITID);
	const display = DB.getItemName(item, { showItemSlots: false, showItemOptions: false });
	const resource = item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName;

	this.placeOnTop();

	const content = root.querySelector('.content');
	if (content) {
		content.innerHTML =
			`<img src="data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==" class="item-${item.ITID}" width="24" height="24" /> ` +
			_sanitizeHtml(`${display} ${DB.getMessage(696).replace('%d', item.count || 1)}`);
	}

	const el = root.querySelector('#ItemObtain');
	this._host.style.left = `${(Renderer.width - (el ? el.offsetWidth : 0)) >> 1}px`;

	Client.loadFile(DB.INTERFACE_PATH + 'item/' + resource + '.bmp', url => {
		const img = root.querySelector(`img.item-${item.ITID}`);
		if (img) {
			img.src = url;
		}
	});

	// Start timer
	if (_timer) {
		Events.clearTimeout(_timer);
	}

	_timer = Events.setTimeout(this.timeEnd.bind(this), _life);
};

/**
 * Create component and return it
 */
export default UIManager.addComponent(ItemObtain);
