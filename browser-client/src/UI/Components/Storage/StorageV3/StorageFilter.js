/**
 * UI/Components/Storage/StorageFilter.js
 *
 * A simple window to show a filtered list of items from Storage.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import Mouse from 'Controls/MouseEventHandler.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import htmlText from './StorageFilter.html?raw';
import cssText from './StorageFilter.css?raw';

function StorageFilter(tabId) {
	const prefName = 'StorageFilter_' + tabId;
	GUIComponent.call(this, prefName, cssText);

	this.render = () => htmlText;

	this.onRemove = function () {
		const root = this.getRoot();
		const content = root.querySelector('.content');
		if (content) {
			content.innerHTML = '';
		}
		this._list.length = 0;
		this._currentTabId = -1;

		this._preferences.y = parseInt(this._host.style.top, 10);
		this._preferences.x = parseInt(this._host.style.left, 10);
		this._preferences.height = Math.floor(
			(root.querySelector('.content') ? root.querySelector('.content').offsetHeight : 128) / 32
		);
		this._preferences.save();

		if (typeof this.onCloseCallback === 'function') {
			this.onCloseCallback();
		}
	};

	this._list = [];
	this._currentTabId = -1;
	this._preferences = Preferences.get(
		prefName,
		{
			x: 300 + tabId * 20,
			y: 200 + tabId * 20,
			height: 4
		},
		1.0
	);
	this.onCloseCallback = null;
}

StorageFilter.prototype = Object.create(GUIComponent.prototype);
StorageFilter.prototype.constructor = StorageFilter;

StorageFilter.prototype.init = function init() {
	const self = this;
	const root = this.getRoot();

	const closeBtn = root.querySelector('.titlebar .right .close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		closeBtn.addEventListener('click', () => {
			self.remove();
		});
	}

	const extendBtn = root.querySelector('.footer .extend');
	if (extendBtn) {
		extendBtn.addEventListener('mousedown', () => this.onResize());
	}

	this.resizeHeight(this._preferences.height);

	const content = root.querySelector('.content');
	if (content) {
		content.addEventListener('mouseover', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				this.onItemOver(itemEl, root);
			}
		});
		content.addEventListener('mouseout', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				this.onItemOut(root);
			}
		});
		content.addEventListener('contextmenu', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				e.preventDefault();
				this.onItemInfo(e, itemEl);
			}
		});
		content.addEventListener('dragstart', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				this.onItemDragStart(e, itemEl);
				this.onItemOut(root);
			}
		});
		content.addEventListener('dragend', e => {
			const itemEl = e.target.closest('.item');
			if (itemEl) {
				this.onItemDragEnd();
			}
		});
	}

	this.draggable('.titlebar');
	this.ui.hide();
};

StorageFilter.prototype.onAppend = function onAppend() {
	this.ui.show();
	this._host.style.left = `${Math.min(Math.max(0, this._preferences.x), Renderer.width - this._host.getBoundingClientRect().width)}px`;
	this._host.style.top = `${Math.min(Math.max(0, this._preferences.y), Renderer.height - this._host.getBoundingClientRect().height)}px`;
};

StorageFilter.prototype.setItems = function setItems(title, items, tabId) {
	this._list = items.slice(0);
	this._currentTabId = tabId;
	const root = this.getRoot();
	const titleEl = root.querySelector('.titlebar .text');
	if (titleEl) {
		titleEl.textContent = title;
	}
	const content = root.querySelector('.content');
	if (content) {
		content.innerHTML = '';
	}

	for (let i = 0, count = this._list.length; i < count; ++i) {
		this.renderItem(this._list[i]);
	}
};

StorageFilter.prototype.renderItem = function renderItem(item) {
	const it = DB.getItemInfo(item.ITID);
	const root = this.getRoot();
	const content = root.querySelector('.content');

	const itemEl = document.createElement('div');
	itemEl.className = 'item';
	itemEl.setAttribute('data-index', item.index);
	itemEl.setAttribute('draggable', 'true');

	const iconDiv = document.createElement('div');
	iconDiv.className = 'icon';
	itemEl.appendChild(iconDiv);

	const amountDiv = document.createElement('div');
	amountDiv.className = 'amount';
	if (item.count) {
		const countSpan = document.createElement('span');
		countSpan.className = 'count';
		countSpan.textContent = item.count;
		amountDiv.appendChild(countSpan);
		amountDiv.appendChild(document.createTextNode(' '));
	}
	itemEl.appendChild(amountDiv);

	const nameSpan = document.createElement('span');
	nameSpan.className = 'name';
	nameSpan.innerHTML = DB.getItemName(item);
	itemEl.appendChild(nameSpan);

	if (content) {
		content.appendChild(itemEl);
	}

	Client.loadFile(
		`${DB.INTERFACE_PATH}item/${item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName}.bmp`,
		data => {
			const icon = root.querySelector(`.item[data-index="${item.index}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${data})`;
			}
		}
	);
};

StorageFilter.prototype.getItemFromIndex = function getItemFromIndex(index) {
	return this._list.filter(item => item.index === index)[0];
};

StorageFilter.prototype.onItemOver = function onItemOver(itemEl, root) {
	const index = parseInt(itemEl.getAttribute('data-index'), 10);
	const item = this.getItemFromIndex(index);
	if (!item) {
		return;
	}

	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = '';
		overlay.style.top = `${itemEl.offsetTop - 10}px`;
		overlay.style.left = `${itemEl.offsetLeft + 35}px`;
		overlay.innerHTML = `${DB.getItemName(item)} ${item.count || 1} ea`;

		if (item.IsIdentified) {
			overlay.classList.remove('grey');
		} else {
			overlay.classList.add('grey');
		}
	}
};

StorageFilter.prototype.onItemOut = function onItemOut(root) {
	if (!root) {
		root = this.getRoot();
	}
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
};

StorageFilter.prototype.onItemDragStart = function onItemDragStart(event, itemEl) {
	const index = parseInt(itemEl.getAttribute('data-index'), 10);
	const item = this.getItemFromIndex(index);
	if (!item) {
		return;
	}

	const img = new Image();
	let url = itemEl.firstChild.style.backgroundImage.match(/\(([^)]+)/)[1];
	url = url.replace(/^"/, '').replace(/"$/, '');
	img.src = url;

	event.dataTransfer.setDragImage(img, 12, 12);
	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'Storage',
				data: item
			})
		)
	);
};

StorageFilter.prototype.onItemDragEnd = function onItemDragEnd() {
	delete window._OBJ_DRAG_;
};

StorageFilter.prototype.onItemInfo = function onItemInfo(event, itemEl) {
	event.stopImmediatePropagation();
	const index = parseInt(itemEl.getAttribute('data-index'), 10);
	const item = this.getItemFromIndex(index);
	if (!item) {
		return false;
	}

	if (event.altKey && event.which === 3) {
		if (typeof this.onTransferItemToOtherUI === 'function') {
			this.onTransferItemToOtherUI(item);
		}
		return false;
	}

	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
	}

	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);
	return false;
};

StorageFilter.prototype.resizeHeight = function resizeHeight(height) {
	height = Math.min(Math.max(height, 4), 10);
	const root = this.getRoot();
	const content = root.querySelector('.content');
	if (content) {
		content.style.height = `${height * 32}px`;
	}
	this._host.style.height = `${height * 32 + 17 + 19}px`;
};

StorageFilter.prototype.onResize = function onResize() {
	const self = this;
	const top = this._host.offsetTop;
	let lastHeight = 0;
	const extraY = 17 + 19;

	function resizing() {
		let h = Math.floor((Mouse.screen.y - top - extraY) / 32);
		h = Math.min(Math.max(h, 4), 10);

		if (h === lastHeight) {
			return;
		}
		self.resizeHeight(h);
		lastHeight = h;
	}

	const _Interval = setInterval(resizing, 30);

	const onMouseUp = event => {
		if (event.which === 1) {
			clearInterval(_Interval);
			window.removeEventListener('mouseup', onMouseUp);
		}
	};
	window.addEventListener('mouseup', onMouseUp);
};

StorageFilter.prototype.getCurrentTab = function getCurrentTab() {
	return this._currentTabId;
};

StorageFilter.prototype.removeItem = function removeItem(index, count) {
	let i = -1;
	for (let j = 0, count_ = this._list.length; j < count_; ++j) {
		if (this._list[j].index === index) {
			i = j;
			break;
		}
	}
	if (i < 0) {
		return;
	}

	const item = this._list[i];
	const root = this.getRoot();
	if (item.count) {
		item.count -= count;
		if (item.count > 0) {
			const countEl = root.querySelector(`.item[data-index="${index}"] .count`);
			if (countEl) {
				countEl.textContent = item.count;
			}
			return;
		}
	}
	this._list.splice(i, 1);
	const el = root.querySelector(`.item[data-index="${index}"]`);
	if (el) {
		el.remove();
	}
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
};

StorageFilter.prototype.addItem = function addItem(item) {
	for (let j = 0, count_ = this._list.length; j < count_; ++j) {
		if (this._list[j].index === item.index) {
			this._list[j].count += item.count;
			const root = this.getRoot();
			const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
			if (countEl) {
				countEl.textContent = this._list[j].count;
			}
			return;
		}
	}
	this._list.push(JSON.parse(JSON.stringify(item)));
	this.renderItem(item);
};

StorageFilter.prototype.mouseMode = GUIComponent.MouseMode.STOP;

export default StorageFilter;
