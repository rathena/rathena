/**
 * UI/Components/Rodex/ReadRodex.js
 *
 * Chararacter ReadRodex
 *
 * @author Alisonrag
 *
 */

import DB from 'DB/DBManager.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import Client from 'Core/Client.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './ReadRodex.html?raw';
import cssText from './ReadRodex.css?raw';
import Rodex from 'UI/Components/Rodex/Rodex.js';

/**
 * Create Component
 */
const ReadRodex = new GUIComponent('ReadRodex', cssText);

ReadRodex.MailID = 0;
ReadRodex.openType = 0;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'ReadRodex',
	{
		show: false
	},
	1.0
);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return ReadRodex._shadow || ReadRodex._host;
}

/**
 * Render HTML
 */
ReadRodex.render = () => htmlText;

/**
 * Initialize Component
 */
ReadRodex.onAppend = function onAppend() {
	const root = _root();

	// Bind buttons
	root.querySelector('.right .close').addEventListener('click', onClickClose);

	const rodexTop = Rodex._host ? parseInt(Rodex._host.style.top, 10) || 0 : 0;
	const rodexLeft = Rodex._host ? parseInt(Rodex._host.style.left, 10) || 0 : 0;

	this._host.style.top = `${Math.min(Math.max(0, rodexTop), Renderer.height - this._host.offsetHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, rodexLeft) + 310, Renderer.width - this._host.offsetWidth)}px`;

	this.draggable(root.querySelector('.titlebar'));
};

/**
 * Remove Mail from window (and so clean up items)
 */
ReadRodex.onRemove = function OnRemove() {
	_preferences.show = this._host.style.display !== 'none';
	_preferences.save();
};

ReadRodex.initData = function initData(data, mail) {
	const root = _root();

	ReadRodex.MailID = mail.MailID;
	ReadRodex.openType = mail.openType;
	ReadRodex.SenderName = mail.SenderName;

	const nameEl = root.querySelector('.name');
	if (nameEl) {
		nameEl.textContent = mail.SenderName;
	}
	const titleEl = root.querySelector('.title-text');
	if (titleEl) {
		titleEl.textContent = mail.title;
	}
	const contentEl = root.querySelector('.content-text');
	if (contentEl) {
		contentEl.textContent = data.Textcontent;
	}
	const valueEl = root.querySelector('.value');
	if (valueEl) {
		valueEl.textContent = prettifyZeny(data.zeny);
	}

	root.querySelector('.get-content').addEventListener('click', onClickGetItems);
	root.querySelector('.get-zeny').addEventListener('click', onClickGetZeny);
	root.querySelector('.delete').addEventListener('click', onClickDelete);
	root.querySelector('.reply').addEventListener('click', onClickReply);

	const content = root.querySelector('.item-list');
	content.innerHTML = '';

	for (let i = 0; i < data.ItemList.length; i++) {
		const item = data.ItemList[i];
		const it = DB.getItemInfo(item.ITID);
		content.insertAdjacentHTML(
			'beforeend',
			`<div class="item" data-index="${i}">` +
				'<div class="icon"></div>' +
				`<div class="amount"><span class="count">${item.count || 1}</span></div>` +
				'</div>'
		);
		Client.loadFile(`${DB.INTERFACE_PATH}item/${it.identifiedResourceName}.bmp`, url => {
			const icon = root.querySelector(`.item[data-index="${i}"] .icon`);
			if (icon) {
				icon.style.backgroundImage = `url(${url})`;
			}
		});
	}

	const getContentBtn = root.querySelector('.get-content');
	if (data.ItemList.length > 0) {
		getContentBtn.style.display = '';
	} else {
		getContentBtn.style.display = 'none';
	}

	const getZenyBtn = root.querySelector('.get-zeny');
	if (data.zeny > 0) {
		getZenyBtn.style.display = '';
	} else {
		getZenyBtn.style.display = 'none';
	}

	this._host.style.display = '';
	this.focus();
};

function onClickClose(e) {
	e.stopImmediatePropagation();
	ReadRodex.MailID = 0;
	ReadRodex.openType = 0;
	ReadRodex.SenderName = '';
	ReadRodex._host.style.display = 'none';
}

function onClickGetItems(e) {
	e.stopImmediatePropagation();
	Rodex.requestItemsFromRodex(ReadRodex.openType, ReadRodex.MailID);
}

function onClickGetZeny(e) {
	e.stopImmediatePropagation();
	Rodex.requestZenyFromRodex(ReadRodex.openType, ReadRodex.MailID);
}

function onClickDelete(e) {
	e.stopImmediatePropagation();
	UIManager.showPromptBox(DB.getMessage(356), 'ok', 'cancel', () => {
		Rodex.requestDeleteRodex(ReadRodex.openType, ReadRodex.MailID);
	});
}

function onClickReply(e) {
	e.stopImmediatePropagation();
	Rodex.requestOpenWriteRodex(ReadRodex.SenderName);
}

ReadRodex.clearItemList = function clearItemList() {
	const root = _root();
	const itemList = root.querySelector('.item-list');
	if (itemList) {
		itemList.innerHTML = '';
	}
};

ReadRodex.clearZeny = function clearZeny() {
	const root = _root();
	const valueEl = root.querySelector('.value');
	if (valueEl) {
		valueEl.textContent = '';
	}
};

ReadRodex.close = function close() {
	ReadRodex.MailID = 0;
	ReadRodex.openType = 0;
	ReadRodex.SenderName = '';
	if (ReadRodex._host) {
		ReadRodex._host.style.display = 'none';
	}
};

/**
 * Prettify number (15000 -> 15,000)
 *
 * @param {number}
 * @return {string}
 */
function prettifyZeny(value) {
	const num = String(value);
	let i = 0;
	const len = num.length;
	let out = '';

	while (i < len) {
		out = num[len - i - 1] + out;
		if ((i + 1) % 3 === 0 && i + 1 !== len) {
			out = ',' + out;
		}
		++i;
	}

	return out;
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(ReadRodex);
