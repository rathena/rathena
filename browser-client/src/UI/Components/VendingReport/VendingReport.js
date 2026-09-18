/**
 * UI/Components/VendingReport/VendingReport.js
 *
 * Item Sell History
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import EntityManager from 'Renderer/EntityManager.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import VendingShop from 'UI/Components/VendingShop/VendingShop.js';
import 'UI/Elements/Elements.js';
import htmlText from './VendingReport.html?raw';
import cssText from './VendingReport.css?raw';

/**
 * Create Component
 */
const VendingReport = new GUIComponent('VendingReport', cssText);

/**
 * Store bought items
 */
const VendingReportTable = {
	list: [],
	_nextIndex: 0
};

/**
 * @var {number} used to remember the window height
 */
VendingReport._resizing = false;
VendingReport._startY = 0;
VendingReport._startHeight = 0;
VendingReport._boundResizeDrag = null;
VendingReport._boundResizeStop = null;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'VendingReport',
	{
		x: 200,
		y: 200,
		width: 400,
		height: 180
	},
	1.0
);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return VendingReport._shadow || VendingReport._host;
}

/**
 * Render HTML
 */
VendingReport.render = () => htmlText;

/**
 * Initialize UI
 */
VendingReport.init = function Init() {
	const root = _root();

	// Bind close button
	const closeBtn = root.querySelector('.btn.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', () => {
			VendingReport.onClose();
		});
	}

	// Bind resize handle
	const extendBtn = root.querySelector('.extend');
	if (extendBtn) {
		extendBtn.addEventListener('mousedown', e => {
			e.preventDefault();

			this._resizing = true;
			this._startY = e.clientY;
			const content = root.querySelector('.container .content');
			this._startHeight = content ? content.getBoundingClientRect().height : 0;

			this._boundResizeDrag = this.onResizeDrag.bind(this);
			this._boundResizeStop = this.onResizeStop.bind(this);

			document.addEventListener('mousemove', this._boundResizeDrag);
			document.addEventListener('mouseup', this._boundResizeStop);
		});
	}

	// Content events
	const content = root.querySelector('.container .content');
	if (content) {
		content.addEventListener('wheel', onScrollWheel);
		content.addEventListener('scroll', onScrollSync);

		content.addEventListener('mouseover', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemOver.call(item, e);
			}
		});

		content.addEventListener('mouseout', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemOut.call(item, e);
			}
		});

		content.addEventListener('contextmenu', e => {
			const item = e.target.closest('.item');
			if (item) {
				onItemInfo.call(item, e);
			}
		});
	}

	this.draggable(root.querySelector('.titlebar'));
};

/**
 * Apply preferences once append to body
 */
VendingReport.onAppend = function OnAppend() {
	this._host.style.display = '';
};

/**
 * Remove Inventory from window (and so clean up items)
 */
VendingReport.onRemove = function OnRemove() {
	VendingReport.reset();
	ItemInfo.remove();

	// Save preferences
	_preferences.y = parseInt(this._host.style.top, 10) || 0;
	_preferences.x = parseInt(this._host.style.left, 10) || 0;
	_preferences.save();

	this._host.style.display = 'none';
};

VendingReport.reset = function reset() {
	VendingReportTable.list.length = 0;
	VendingReportTable._nextIndex = 0;

	const root = _root();
	const content = root.querySelector('.container .content');
	if (content) {
		content.querySelectorAll('.item').forEach(el => el.remove());
	}
};

/**
 * Extend window size
 */
VendingReport.onResizeDrag = function (e) {
	if (!this._resizing) {
		return;
	}

	const MIN_HEIGHT = 100;
	const MAX_HEIGHT = 260;

	const deltaY = e.clientY - this._startY;
	let newHeight = this._startHeight + deltaY;

	newHeight = Math.min(Math.max(newHeight, MIN_HEIGHT), MAX_HEIGHT);

	const root = _root();
	const content = root.querySelector('.container .content');
	if (content) {
		content.style.height = `${newHeight}px`;
	}
	this._host.style.height = `${newHeight + 31 + 19}px`;
};

/**
 * Stop resizing window
 *
 * This function is called when the user stops resizing the window.
 * It resets the _resizing flag and removes the event listener for the
 * mousemove event.
 */
VendingReport.onResizeStop = function () {
	this._resizing = false;

	if (this._boundResizeDrag) {
		document.removeEventListener('mousemove', this._boundResizeDrag);
	}
	if (this._boundResizeStop) {
		document.removeEventListener('mouseup', this._boundResizeStop);
	}
	this._boundResizeDrag = null;
	this._boundResizeStop = null;
};

/**
 * Add a sold item to the Vending Report Table.
 *
 * @param {object} pkt - Sold item packet (contains index, count, zeny, date, and CID of buyer)
 */
VendingReport.add = function add(pkt) {
	if (!pkt) {
		return;
	}

	const shopItem = VendingShop.getItemByIndex(pkt.index);

	if (!shopItem) {
		return;
	}

	const entity = EntityManager.getByCID(pkt.CID);
	const buyer = entity ? entity.display.name : 'Unknown';

	const reportItem = {
		...shopItem,
		reportId: ++VendingReportTable._nextIndex,
		shopindex: pkt.index,
		count: pkt.count,
		buyer: buyer,
		zeny: pkt.zeny,
		date: formatUnixDate(pkt.date),
		date_raw: pkt.date
	};

	this.addItem(reportItem);
};

/**
 * Insert Item to Vending Report Table
 *
 * @param {object} Item
 */
VendingReport.addItem = function addItem(item) {
	VendingReportTable.list.push(item);
	this.addItemSub(item);
};

/**
 * Add item to Vending Report UI
 *
 * @param {object} Item
 */
VendingReport.addItemSub = function addItemSub(item) {
	const it = DB.getItemInfo(item.ITID);
	const root = _root();
	const content = root.querySelector('.container .content');

	if (!content) {
		return false;
	}

	content.insertAdjacentHTML(
		'beforeend',
		`<div class="item" data-ITID="${item.ITID}" data-index="${item.shopindex}" data-id="${item.reportId}">` +
			`<div class="icon"></div>` +
			`<div class="info">` +
			`<div class="count">${item.count}</div>` +
			`<div class="buyer">${item.buyer}</div>` +
			`<div class="zeny">${prettyZeny(item.zeny, false)} Zeny</div>` +
			`<div class="date">${item.date}</div>` +
			`</div>` +
			`</div>`
	);

	Client.loadFile(
		`${DB.INTERFACE_PATH}item/${item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName}.bmp`,
		data => {
			const icon = content.querySelector(`.item[data-id="${item.reportId}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);

	return true;
};

/**
 * Prettify zeny : 1000000 -> 1,000,000
 *
 * @param {number} zeny
 * @param {boolean} use color
 * @return {string}
 */
function prettyZeny(val, useStyle) {
	const list = val.toString().split('');
	const count = list.length;
	let i;
	let str = '';

	for (i = 0; i < count; i++) {
		str = list[count - i - 1] + (i && i % 3 === 0 ? ',' : '') + str;
	}

	if (useStyle) {
		const style = [
			'color:#000000; text-shadow:1px 0px #00ffff;', // 0 - 9
			'color:#0000ff; text-shadow:1px 0px #ce00ce;', // 10 - 99
			'color:#0000ff; text-shadow:1px 0px #00ffff;', // 100 - 999
			'color:#ff0000; text-shadow:1px 0px #ffff00;', // 1,000 - 9,999
			'color:#ff18ff;', // 10,000 - 99,999
			'color:#0000ff;', // 100,000 - 999,999
			'color:#000000; text-shadow:1px 0px #00ff00;', // 1,000,000 - 9,999,999
			'color:#ff0000;', // 10,000,000 - 99,999,999
			'color:#000000; text-shadow:1px 0px #cece63;', // 100,000,000 - 999,999,999
			'color:#ff0000; text-shadow:1px 0px #ff007b;' // 1,000,000,000 - 9,999,999,999
		];
		str = `<span style="${style[count - 1]}">${str}</span>`;
	}

	return str;
}

/**
 * Handle mouse wheel event to scroll the content.
 *
 * @param {Event} event The mouse wheel event.
 */
function onScrollWheel(event) {
	event.preventDefault();
	event.stopImmediatePropagation();

	const ROW_HEIGHT = 24;

	let delta = 0;
	if (event.wheelDelta) {
		delta = event.wheelDelta > 0 ? 1 : -1;
	} else if (event.deltaY) {
		delta = event.deltaY < 0 ? 1 : -1;
	}

	let target = this.scrollTop - delta * ROW_HEIGHT;

	const maxScroll = this.scrollHeight - this.clientHeight;
	target = Math.max(0, Math.min(target, maxScroll));

	target = Math.round(target / ROW_HEIGHT) * ROW_HEIGHT;

	this.scrollTop = target;
	this.style.backgroundPositionY = -target + 'px';
}

/**
 * Sync the background position to the scroll position.
 *
 * This function is called whenever the content is scrolled.
 */
function onScrollSync() {
	this.style.backgroundPositionY = -this.scrollTop + 'px';
}

/**
 * Show item name when mouse is over
 */
function onItemOver() {
	const idx = parseInt(this.getAttribute('data-id'), 10);
	const item = VendingReport.getItemByIndex(idx);

	if (!item) {
		return;
	}

	const root = _root();
	const overlay = root.querySelector('.overlay');
	const innerRoot = root.querySelector('#VendingReport');

	if (!overlay || !innerRoot) {
		return;
	}

	const itemRect = this.getBoundingClientRect();
	const rootRect = innerRoot.getBoundingClientRect();

	overlay.style.display = 'block';
	overlay.style.top = `${itemRect.top - rootRect.top}px`;
	overlay.style.left = `${itemRect.left - rootRect.left + 35}px`;
	overlay.textContent = DB.getItemName(item);

	if (item.IsIdentified) {
		overlay.classList.remove('grey');
	}
}

/**
 * Hide the item name
 */
function onItemOut() {
	const root = _root();
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
}

/**
 * Get item info (open description window)
 */
function onItemInfo(event) {
	event.stopImmediatePropagation();
	event.preventDefault();

	const index = parseInt(this.getAttribute('data-id'), 10);
	const item = VendingReport.getItemByIndex(index);

	if (!item) {
		return;
	}

	// Don't add the same UI twice, remove it
	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		return;
	}

	// Add ui to window
	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);
}

/**
 * Search in a list for an item by its index
 *
 * @param {number} index
 * @returns {Item}
 */
VendingReport.getItemByIndex = function getItemByIndex(index) {
	const list = VendingReportTable.list;

	for (let i = 0, count = list.length; i < count; ++i) {
		if (list[i].reportId === index) {
			return list[i];
		}
	}

	return null;
};

/**
 * Format a Unix timestamp (seconds) into MM/DD - HH:mm:sec
 *
 * @param {number} unixTimestamp - Unix time in seconds
 * @returns {string} Formatted date string (MM/DD HH:mm)
 */
function formatUnixDate(unixTimestamp) {
	const d = new Date(unixTimestamp * 1000);

	return (
		String(d.getMonth() + 1).padStart(2, '0') +
		'/' +
		String(d.getDate()).padStart(2, '0') +
		' - ' +
		String(d.getHours()).padStart(2, '0') +
		':' +
		String(d.getMinutes()).padStart(2, '0') +
		':' +
		String(d.getSeconds()).padStart(2, '0')
	);
}

/**
 * Close
 */
VendingReport.onClose = function onClose() {
	this.remove();
};

/**
 * Create component and export it
 */
export default UIManager.addComponent(VendingReport);
