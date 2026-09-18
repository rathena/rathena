/**
 * UI/Components/Mail/ReadMail.js
 *
 * Chararacter ReadMail
 *
 * @author Francisco Wallison
 *
 */

import DB from 'DB/DBManager.js';
import Preferences from 'Core/Preferences.js';
import Client from 'Core/Client.js';
import Renderer from 'Renderer/Renderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import Mail from 'UI/Components/Mail/Mail.js';
import htmlText from './ReadMail.html?raw';
import cssText from './ReadMail.css?raw';

/**
 * Create Component
 */
const ReadMail = new GUIComponent('ReadMail', cssText);

/**
 * Store ReadMail items
 */
ReadMail.list = [];

/**
 * @var {number} used to remember the window height
 */
const _realSize = 0;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'ReadMail',
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
		magnet_right: false,
		item_add_email: {}
	},
	1.0
);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return ReadMail._shadow || ReadMail._host;
}

/**
 * Render HTML
 */
ReadMail.render = () => htmlText;

/**
 * Initialize Component
 */
ReadMail.onAppend = function onAppend() {
	const root = _root();

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', event => {
			event.stopImmediatePropagation();
			ReadMail.remove();
		});
	}

	const delBtn = root.querySelector('#read_mail_del');
	if (delBtn) delBtn.addEventListener('click', deleteMail);

	const remailBtn = root.querySelector('#read_mail_remail');
	if (remailBtn) remailBtn.addEventListener('click', replyMail);

	const returnBtn = root.querySelector('#read_mail_return');
	if (returnBtn) returnBtn.addEventListener('click', returnMail);

	// Position relative to Mail window
	const mailHost = Mail._host;
	const mailTop = mailHost ? parseInt(mailHost.style.top, 10) || 0 : 0;
	const mailLeft = mailHost ? parseInt(mailHost.style.left, 10) || 0 : 0;
	const hostHeight = this._host.offsetHeight || 0;
	const hostWidth = this._host.offsetWidth || 0;

	this._host.style.top = `${Math.min(Math.max(0, mailTop), Renderer.height - hostHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, mailLeft + 300), Renderer.width - hostWidth)}px`;

	this.draggable('.titlebar');
};

/**
 * Remove Mail from window (and so clean up items)
 */
ReadMail.onRemove = function OnRemove() {
	this.list.length = 0;
	_preferences.show = this._host.style.display !== 'none';
	_preferences.reduce = !!_realSize;
	_preferences.y = parseInt(this._host.style.top, 10) || 0;
	_preferences.x = parseInt(this._host.style.left, 10) || 0;
	_preferences.magnet_top = this.magnet.TOP;
	_preferences.magnet_bottom = this.magnet.BOTTOM;
	_preferences.magnet_left = this.magnet.LEFT;
	_preferences.magnet_right = this.magnet.RIGHT;
	_preferences.save();
};

ReadMail.openEmail = function openEmail(inforMail) {
	ReadMail.remove();
	ReadMail.append();

	// Re-query after re-append
	const freshRoot = _root();
	const textSender = inforMail.FromName;
	const textTitle = inforMail.Header;
	const textMessage = inforMail.msg === '(no message)' ? '' : inforMail.msg;

	const senderEl = freshRoot.querySelector('.text_sender');
	if (senderEl) senderEl.textContent = textSender;

	const titleEl = freshRoot.querySelector('.text_title');
	if (titleEl) titleEl.textContent = textTitle;

	const textarea = freshRoot.querySelector('.textarea_mail');
	if (textarea) textarea.value = textMessage;

	const btnContainer = freshRoot.querySelector('.btn_return_reply_remove');
	if (btnContainer) btnContainer.dataset.mailID = inforMail.MailID;

	if (inforMail.ITID != 0 || inforMail.Money != 0) {
		addItemSub(inforMail);
	} else {
		this.resetItemZeny();
	}
};

ReadMail.resetItemZeny = function resetItemZeny() {
	const root = _root();
	_preferences.item_add_email = {};
	_preferences.save();
	removeValueItemZeny();
	const infoBox = root.querySelector('.zeny_item_infor_box');
	if (infoBox) infoBox.remove();
};

ReadMail.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		this.remove();
	}
};

function addItemSub(itemMail) {
	const root = _root();
	removeValueItemZeny();

	const zenyItemContainer = root.querySelector('.zeny_item_container');

	if (itemMail.ITID != 0 && itemMail.count > 0) {
		const item = itemMail;
		const it = DB.getItemInfo(item.ITID);
		const content = root.querySelector('.container_item');
		if (content) {
			content.insertAdjacentHTML(
				'beforeend',
				`<div class="item" data-index="${item.index}" draggable="true">` +
					`<div class="icon"></div>` +
					`<div class="amount"><span class="count">${item.count || 1}</span></div>` +
					`</div>`
			);
		}

		const hideEl = root.querySelector('.hide');
		if (hideEl) hideEl.style.display = 'block';

		Client.loadFile(
			`${DB.INTERFACE_PATH}item/${item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName}.bmp`,
			data => {
				if (content) {
					const icon = content.querySelector(`.item[data-index="${item.index}"] .icon`);
					if (icon) icon.style.backgroundImage = `url(${data})`;
				}
			}
		);

		if (content) {
			content.addEventListener('mouseover', event => {
				const el = event.target.closest('.item');
				if (el) onItemOver.call(el, event);
			});
			content.addEventListener('mouseout', event => {
				const el = event.target.closest('.item');
				if (el) onItemOut.call(el, event);
			});
			content.addEventListener('contextmenu', event => {
				const el = event.target.closest('.item');
				if (el) onItemInfo.call(el, event);
			});
		}
	}

	if (itemMail.Money > 0) {
		const zenyInput = root.querySelector('.input_zeny_amt');
		if (zenyInput) {
			zenyInput.value = prettifyZeny(itemMail.Money);
			zenyInput.disabled = true;
		}
	}

	const oldInfoBox = root.querySelector('.zeny_item_infor_box');
	if (oldInfoBox) oldInfoBox.remove();

	if (zenyItemContainer) {
		zenyItemContainer.insertAdjacentHTML('beforeend', '<div class="zeny_item_infor_box"></div>');
	}

	const inforEl = root.querySelector('.zeny_item_infor');
	if (inforEl) {
		inforEl.addEventListener('mouseover', () => {
			const box = root.querySelector('.zeny_item_infor_box');
			if (box) box.style.display = 'block';
		});
		inforEl.addEventListener('mouseout', () => {
			const box = root.querySelector('.zeny_item_infor_box');
			if (box) box.style.display = 'none';
		});
	}

	_preferences.item_add_email = itemMail;
	_preferences.save();

	if (inforEl) {
		inforEl.addEventListener('click', event => {
			event.stopImmediatePropagation();
			if (!validItemMoneyExists()) {
				const btnContainer = root.querySelector('.btn_return_reply_remove');
				const mailID = btnContainer ? btnContainer.dataset.mailID : null;
				if (mailID) Mail.parseMailgetattach(parseInt(mailID, 10));
			}
		});
	}
}

/**
 * Hide the item name
 */
function onItemOut(event) {
	const root = _root();
	event.stopImmediatePropagation();
	const overlay = root.querySelector('.container_item .overlay');
	if (overlay) overlay.style.display = 'none';
}

/**
 * Show item name when mouse is over
 */
function onItemOver(event) {
	const root = _root();
	event.stopImmediatePropagation();
	const item = _preferences.item_add_email;

	if (!item) {
		return;
	}

	const overlay = root.querySelector('.container_item .overlay');
	if (overlay) {
		overlay.style.display = 'block';
		overlay.textContent = `${DB.getItemName(item)} ${item.count || 1} ea`;

		if (item.IsIdentified) {
			overlay.classList.remove('grey');
		} else {
			overlay.classList.add('grey');
		}
	}
}

/**
 * Get item info (open description window)
 */
function onItemInfo(event) {
	event.stopImmediatePropagation();

	const item = _preferences.item_add_email;
	if (!item) {
		return;
	}

	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		return;
	}

	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);
}

function replyMail(event) {
	const root = _root();
	event.stopImmediatePropagation();
	const senderEl = root.querySelector('.text_sender');
	const textSender = senderEl ? senderEl.textContent : '';
	Mail.replyNewMail(textSender);
}

function deleteMail(event) {
	const root = _root();
	event.stopImmediatePropagation();

	if (validItemMoneyExists()) {
		const btnContainer = root.querySelector('.btn_return_reply_remove');
		const mailID = btnContainer ? btnContainer.dataset.mailID : null;
		if (mailID) Mail.deleteMail(parseInt(mailID, 10));
	} else {
		ChatBox.addText(DB.getMessage(1105), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
	}
}

function validItemMoneyExists() {
	let validItem = _preferences.item_add_email.count === 0 && _preferences.item_add_email.ITID === 0;
	validItem = _preferences.item_add_email.ITID === undefined ? true : validItem;

	let validMoney = _preferences.item_add_email.Money === 0;
	validMoney = _preferences.item_add_email.Money === undefined ? true : validMoney;

	return validItem && validMoney;
}

function returnMail(event) {
	const root = _root();
	event.stopImmediatePropagation();
	const btnContainer = root.querySelector('.btn_return_reply_remove');
	const mailID = btnContainer ? btnContainer.dataset.mailID : null;
	const senderEl = root.querySelector('.text_sender');
	const textSender = senderEl ? senderEl.textContent : '';
	if (mailID) Mail.returnMail(parseInt(mailID, 10), textSender);
}

/**
 * Prettify number (15000 -> 15,000)
 */
function prettifyZeny(value) {
	const num = String(value);
	let i = 0;
	const len = num.length;
	let out = '';

	while (i < len) {
		out = num[len - i - 1] + out;
		if ((i + 1) % 3 === 0 && i + 1 !== len) {
			out = `,${out}`;
		}
		++i;
	}

	return out;
}

function removeValueItemZeny() {
	const root = _root();
	const item = root.querySelector('.item');
	if (item) item.remove();

	const zenyInput = root.querySelector('.input_zeny_amt');
	if (zenyInput) zenyInput.value = '';
}

/**
 * Callbacks
 */

/**
 * Create component and export it
 */
export default UIManager.addComponent(ReadMail);
