/**
 * UI/Components/Mail/Mail.js
 *
 * Chararacter Mail
 *
 * @author Francisco Wallison
 *
 */

import DB from 'DB/DBManager.js';
import ItemType from 'DB/Items/ItemType.js';
import Preferences from 'Core/Preferences.js';
import Client from 'Core/Client.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './Mail.html?raw';
import cssText from './Mail.css?raw';

/**
 * Create Component
 */
const Mail = new GUIComponent('Mail', cssText);

/**
 * Store Mail items
 */
Mail.list = [];
/**
 * Determine data page size
 */
Mail.pageSize = 7;
/**
 * know which page is current
 */
Mail.page = 0;

/**
 * @var {number} used to remember the window height
 */
const _realSize = 0;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'Mail',
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
	2.0
);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return Mail._shadow || Mail._host;
}

/**
 * Render HTML
 */
Mail.render = () => htmlText;

/**
 * Has input fields, protect key events
 */
Mail.captureKeyEvents = true;

/**
 * Apply preferences once append to body
 */
Mail.onAppend = function OnAppend() {
	const root = _root();

	const closeBtn = root.querySelector('.close');
	if (closeBtn) closeBtn.addEventListener('click', this.onClosePressed.bind(this));

	const inboxBtn = root.querySelector('#inbox');
	if (inboxBtn) inboxBtn.addEventListener('click', offCreateMessagesOnWindowMailbox);

	const writeBtn = root.querySelector('#write');
	if (writeBtn) writeBtn.addEventListener('click', openWindowCreateMessages);

	const cancelBtn = root.querySelector('#create_mail_cancel');
	if (cancelBtn) cancelBtn.addEventListener('click', offCreateMessagesOnWindowMailbox);

	const sendBtn = root.querySelector('#create_mail_send');
	if (sendBtn) sendBtn.addEventListener('click', sendCreateMessagesMail);

	updatePageMailItems();

	const containerItem = root.querySelector('.container_item');
	if (containerItem) {
		containerItem.addEventListener('drop', onDrop);
		containerItem.addEventListener('dragover', stopPropagation);

		containerItem.addEventListener('mouseover', event => {
			const item = event.target.closest('.item');
			if (item) onItemOver.call(item, event);
		});
		containerItem.addEventListener('mouseout', event => {
			const item = event.target.closest('.item');
			if (item) onItemOut.call(item, event);
		});
		containerItem.addEventListener('dragstart', event => {
			const item = event.target.closest('.item');
			if (item) onItemDragStart.call(item, event);
		});
		containerItem.addEventListener('dragend', event => {
			const item = event.target.closest('.item');
			if (item) onItemDragEnd.call(item, event);
		});
		containerItem.addEventListener('contextmenu', event => {
			const item = event.target.closest('.item');
			if (item) onItemInfo.call(item, event);
		});
	}

	// Validate information dragged into text field
	root.querySelectorAll('input[type=text]').forEach(input => {
		input.addEventListener('drop', onDropText);
		input.addEventListener('dragover', stopPropagation);
	});

	const textarea = root.querySelector('textarea');
	if (textarea) {
		textarea.addEventListener('drop', onDropText);
		textarea.addEventListener('dragover', stopPropagation);
	}

	const zenyAmtBtn = root.querySelector('#zeny_amt');
	if (zenyAmtBtn) zenyAmtBtn.addEventListener('click', onAddZenyInput);

	const zenyOkBtn = root.querySelector('#zeny_ok');
	if (zenyOkBtn) zenyOkBtn.addEventListener('click', onValidZenyInput);

	onWindowMailbox();

	// Apply preferences
	const hostHeight = this._host.offsetHeight || 0;
	const hostWidth = this._host.offsetWidth || 0;
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - hostHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - hostWidth)}px`;

	this.draggable('.titlebar');
};

/**
 * Add item to inventory
 */
Mail.addItemSub = function AddItemSub(Index) {
	const root = _root();
	const item = _preferences.item_add_email;
	if (item.index !== Index) {
		return false;
	}
	if (item.WearState && item.type !== ItemType.AMMO && item.type !== ItemType.CARD) {
		return false;
	}

	const it = DB.getItemInfo(item.ITID);
	const content = root.querySelector('.container_item');

	const oldItem = root.querySelector('.item');
	if (oldItem) oldItem.remove();

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
			const icon = content ? content.querySelector(`.item[data-index="${item.index}"] .icon`) : null;
			if (icon) icon.style.backgroundImage = `url(${data})`;
		}
	);
	return true;
};

/**
 * Send from mail to inventory - Remove item
 */
Mail.removeItem = function removeItem() {
	const root = _root();
	const item = root.querySelector('.item');
	if (item) item.remove();
};

/**
 * Send from mail to inventory - Remove zenys
 */
Mail.removeZeny = function removeZeny() {
	const root = _root();
	const input = root.querySelector('.input_zeny_amt');
	if (input) input.value = '0';
};

/**
 * Remove Mail from window (and so clean up items)
 */
Mail.onRemove = function OnRemove() {
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

/**
 * Extend Mail window size
 */
Mail.resize = function Resize(width, height) {
	const root = _root();
	width = Math.min(Math.max(width, 6), 9);
	height = Math.min(Math.max(height, 2), 6);

	const mailEl = root.querySelector('#Mail');
	if (mailEl) {
		mailEl.style.width = `${23 + 16 + 16 + width * 32}px`;
		mailEl.style.height = `${31 + 19 + height * 32}px`;
	}
};

/**
 * Extend Mail window size
 */
Mail.mailList = function mailList(read) {
	Mail.list = read;
	updatePageMailItems();
};

/**
 * Mail receive
 */
Mail.mailReceiveUpdate = function mailReceiveUpdate(newMail) {
	if (Mail.list.mailList === undefined) {
		return;
	}
	Mail.list.MailNumber = Mail.MailNumber + 1;
	let validIsOpen = 0;
	Mail.list.mailList = Mail.list.mailList.map(el => {
		let newElement = {};

		if (el.MailID == newMail.MailID) {
			newElement = {
				DeleteTime: el.DeleteTime,
				FromName: el.FromName,
				HEADER: el.HEADER,
				MailID: el.MailID,
				isOpen: 1
			};
			validIsOpen = 1;
		} else {
			newElement = el;
		}

		return newElement;
	});
	if (!validIsOpen) {
		Mail.list.mailList.push(newMail);
	}
	Mail.parseMailrefreshinbox();
};

/**
 * Search in a list for an item by its index
 */
Mail.getItemByIndex = function getItemByIndex(index) {
	const list = _preferences.item_add_email;

	if (list.index == index) {
		return list;
	}

	return null;
};

/**
 * Responder to a mail.
 */
Mail.replyNewMail = function replyNewMail(fromName) {
	const root = _root();
	onWindowCreateMessages();
	const textTo = root.querySelector('.text_to');
	if (textTo) textTo.value = fromName.replace(/^(\$|\%)/, '').replace(/\t/g, '');
};

/**
 * Responder to a mail from friends.
 */
Mail.replyNewMailFriends = async function replyNewMailFriends(fromName) {
	const root = _root();
	Mail.append();
	sleep(1).then(() => {
		onWindowCreateMessages();
		offWindowListMail();
		const textTo = root.querySelector('.text_to');
		if (textTo) textTo.value = fromName.replace(/^(\$|\%)/, '').replace(/\t/g, '');

		const inboxBtn = root.querySelector('#inbox');
		if (inboxBtn) {
			inboxBtn.replaceWith(inboxBtn.cloneNode(true));
			const newInbox = root.querySelector('#inbox');
			if (newInbox) {
				newInbox.disabled = false;
				newInbox.addEventListener('click', () => {});
			}
		}

		const cancelBtn = root.querySelector('#create_mail_cancel');
		if (cancelBtn) {
			cancelBtn.replaceWith(cancelBtn.cloneNode(true));
			const newCancel = root.querySelector('#create_mail_cancel');
			if (newCancel) newCancel.addEventListener('click', this.onClosePressed.bind(this));
		}
	});
};

Mail.clearFieldsItemZeny = function clearFieldsItemZeny() {
	const root = _root();
	const item = root.querySelector('.item');
	if (item) item.remove();

	const zenyInput = root.querySelector('.input_zeny_amt');
	if (zenyInput) zenyInput.value = '0';

	const sendBtn = root.querySelector('#create_mail_send');
	if (sendBtn) sendBtn.disabled = false;
};

Mail.onKeyDown = function onKeyDown(event) {
	const shadow = this._shadow || this._host;
	const focused = shadow ? shadow.activeElement : null;

	if (focused && focused.tagName && focused.tagName.match(/input|select|textarea/i)) {
		if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
			this.remove();
			event.stopImmediatePropagation();
			return false;
		}
		event.stopImmediatePropagation();
		return true;
	}

	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		this.remove();
		event.stopImmediatePropagation();
		return false;
	}

	return true;
};

/*
 * Update item pagination
 */
function updatePageMailItems() {
	const root = _root();

	const nextBtn = root.querySelector('.next');
	if (nextBtn) {
		nextBtn.addEventListener('click', e => {
			e.stopImmediatePropagation();
			if (Mail.page < Mail.list.mailList.length / Mail.pageSize - 1) {
				Mail.page++;
				createMailList();
				adjustButtons();
			}
		});
	}

	const prevBtn = root.querySelector('.prev');
	if (prevBtn) {
		prevBtn.addEventListener('click', e => {
			e.stopImmediatePropagation();
			if (Mail.page > 0) {
				Mail.page--;
				createMailList();
				adjustButtons();
			}
		});
	}

	createMailList();
}

/**
 * Create messages window size
 */
function onWindowMailbox() {
	const root = _root();

	Mail.parseMailrefreshinbox();

	const sendBtn = root.querySelector('#create_mail_send');
	if (sendBtn) sendBtn.disabled = false;

	const createMail = root.querySelector('.block_create_mail');
	if (createMail) createMail.style.display = 'none';

	const textTo = root.querySelector('.text_to');
	if (textTo) textTo.value = '';

	const inputTitle = root.querySelector('.input_title');
	if (inputTitle) inputTitle.value = '';

	const textareaMail = root.querySelector('.textarea_mail');
	if (textareaMail) textareaMail.value = '';

	const zenyInput = root.querySelector('.input_zeny_amt');
	if (zenyInput) zenyInput.value = '0';

	const addItemInput = root.querySelector('.input_add_item');
	if (addItemInput) addItemInput.value = '';

	const prevNext = root.querySelector('.prev_next');
	if (prevNext) prevNext.style.display = 'flex';

	const blockMail = root.querySelector('.block_mail');
	if (blockMail) blockMail.style.display = 'block';

	Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/maillist1_bg.bmp', url => {
		const body = root.querySelector('.body');
		if (body) body.style.backgroundImage = `url(${url})`;
	});

	const title = root.querySelector('#title');
	if (title) title.textContent = DB.getMessage(1025);
}

function createMailList() {
	const root = _root();
	const content = root.querySelector('.list_item_mail');

	root.querySelectorAll('.item_mail').forEach(el => el.remove());

	if (Mail.list.length == 0) {
		return;
	}

	for (let i = Mail.page * Mail.pageSize; i < Mail.list.mailList.length && i < (Mail.page + 1) * Mail.pageSize; i++) {
		const from_name =
			Mail.list.mailList[i].FromName.length > 15
				? Mail.list.mailList[i].FromName.substring(0, 15) + '...'
				: Mail.list.mailList[i].FromName;
		const header =
			Mail.list.mailList[i].HEADER.length > 23
				? Mail.list.mailList[i].HEADER.substring(0, 23) + '...'
				: Mail.list.mailList[i].HEADER;
		const mailId = Mail.list.mailList[i].MailID;
		const isOpen = Mail.list.mailList[i].isOpen;

		if (content) {
			content.insertAdjacentHTML(
				'beforeend',
				`<div class="item_mail">
					<div class="envelop" style="flex: 1;">
						<div class="btn_envelop" id="envelop_${mailId}"></div>
					</div>
					<div class="to_title" style="flex: 3;">
						<div class="flex">
							<div style="flex: 3;">
								<span id="from_name_${mailId}" class="event_add_cursor tooltip name_data"> ${from_name}
									<span class="tooltiptext to">${Mail.list.mailList[i].FromName}</span>
								</span>
							</div>
							<div style="flex: 3;">
								<span class="name_data">${formateDeleteTime(Mail.list.mailList[i].DeleteTime)}</span>
							</div>
						</div>
						<div>
							<span id="from_header_${mailId}" data-id="${mailId}" class="event_add_cursor tooltip"> ${header}
								<span class="tooltiptext title">${Mail.list.mailList[i].HEADER}</span>
							</span>
						</div>
					</div>
				</div>`
			);
		}

		if (!isOpen) {
			Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/envelop.bmp', data => {
				const envelop = root.querySelector(`#envelop_${mailId}`);
				if (envelop) envelop.style.backgroundImage = `url(${data})`;
			});
		}

		const fromNameEl = root.querySelector(`#from_name_${mailId}`);
		if (fromNameEl) {
			fromNameEl.addEventListener('click', () => {
				const toEl = root.querySelector(`#from_name_${mailId} .to`);
				if (toEl) Mail.replyNewMail(toEl.textContent);
			});
		}

		const fromHeaderEl = root.querySelector(`#from_header_${mailId}`);
		if (fromHeaderEl) {
			fromHeaderEl.addEventListener('click', event => {
				Mail.openMail(parseInt(event.currentTarget.dataset.id, 10));
			});
		}
	}

	if (Mail.list.mailList.length === 0) {
		const prevNext = root.querySelector('.prev_next');
		if (prevNext) prevNext.style.display = 'none';
	} else {
		const inforPage = root.querySelector('#infor_page');
		if (inforPage) {
			inforPage.textContent = `${Mail.page + 1}/${Math.ceil(Mail.list.mailList.length / Mail.pageSize)}`;
		}
	}

	adjustButtons();
}

function adjustButtons() {
	const root = _root();
	if (Mail.list.length == 0) {
		return;
	}
	const mailLength = Mail.list.mailList.length;

	if (!(Mail.page > mailLength / Mail.pageSize - 1)) {
		addEventNextAndPrevAdd('next');
	} else {
		addEventNextAndPrevRemove('next');
	}
	if (!(Mail.page == 0)) {
		addEventNextAndPrevAdd('prev');
	} else {
		addEventNextAndPrevRemove('prev');
	}

	const nextSpan = root.querySelector('.next span');
	if (nextSpan) nextSpan.disabled = mailLength <= Mail.pageSize || Mail.page > mailLength / Mail.pageSize - 1;

	const prevSpan = root.querySelector('.prev span');
	if (prevSpan) prevSpan.disabled = mailLength <= Mail.pageSize || Mail.page == 0;
}

function addEventNextAndPrevAdd(eventName) {
	const root = _root();
	const overlay = root.querySelector(`.prev_next .overlay_${eventName}`);
	const text = root.querySelector(`.prev_next .${eventName} span`);
	if (text) text.classList.add('event_add_cursor');
	if (overlay && text) overlay.textContent = text.textContent;

	const cursor = root.querySelector(`.${eventName} .event_add_cursor`);
	if (cursor) {
		cursor.addEventListener('mouseover', () => {
			if (text && text.classList.contains('event_add_cursor') && overlay) {
				overlay.style.display = 'block';
			}
		});
		cursor.addEventListener('mouseout', () => {
			if (overlay) overlay.style.display = 'none';
		});
	}
}

function addEventNextAndPrevRemove(eventName) {
	const root = _root();
	const overlay = root.querySelector(`.prev_next .overlay_${eventName}`);
	const text = root.querySelector(`.prev_next .${eventName} span`);
	if (overlay) overlay.style.display = 'none';
	if (text) text.classList.remove('event_add_cursor');
}

function offCreateMessagesOnWindowMailbox(event) {
	event.stopImmediatePropagation();
	onWindowMailbox();
	removeCreateAllItem();
}

function sendCreateMessagesMail(event) {
	const root = _root();
	event.stopImmediatePropagation();

	const zenyOk = root.querySelector('#zeny_ok');
	if (zenyOk && window.getComputedStyle(zenyOk).display !== 'none') {
		ChatBox.addText(DB.getMessage(1110), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}

	let to = root.querySelector('.text_to')?.value || '';
	to = to.length > 50 ? to.substring(0, 50) : to;
	let title = root.querySelector('.input_title')?.value || '';
	title = title.length > 50 ? title.substring(0, 50) : title;
	let message = root.querySelector('.textarea_mail')?.value || '';
	message = message.length > 198 ? message.substring(0, 198) : message;

	if (title === '') {
		ChatBox.addText(DB.getMessage(1106), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
		return;
	}

	const send_message = {
		ReceiveName: to,
		Header: title,
		msg_len: message.length,
		msg: message
	};

	const sendBtn = root.querySelector('#create_mail_send');
	if (sendBtn) sendBtn.disabled = true;

	Mail.parseMailSend(send_message);
}

function openWindowCreateMessages(event) {
	event.stopImmediatePropagation();
	onWindowCreateMessages();
}

/**
 * Open Create messages window size
 */
function onWindowCreateMessages() {
	const root = _root();
	removeCreateAllItem();
	offWindowListMail();

	Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/maillist2_bg.bmp', url => {
		const body = root.querySelector('.body');
		if (body) body.style.backgroundImage = `url(${url})`;
	});

	const title = root.querySelector('#title');
	if (title) title.textContent = DB.getMessage(1026);
}

function offWindowListMail() {
	const root = _root();
	const prevNext = root.querySelector('.prev_next');
	if (prevNext) prevNext.style.display = 'none';

	const blockMail = root.querySelector('.block_mail');
	if (blockMail) blockMail.style.display = 'none';

	const createMail = root.querySelector('.block_create_mail');
	if (createMail) createMail.style.display = 'block';

	const textarea = root.querySelector('.textarea_mail');
	if (textarea) textarea.focus();
}

function onAddZenyInput(event) {
	const root = _root();
	event.stopImmediatePropagation();

	const zenyAmt = root.querySelector('#zeny_amt');
	if (zenyAmt) zenyAmt.style.display = 'none';

	const zenyOk = root.querySelector('#zeny_ok');
	if (zenyOk) zenyOk.style.display = 'inline-block';

	const input = root.querySelector('.input_zeny_amt');
	if (input) {
		input.disabled = false;
		input.focus();
		input.select();
	}

	Mail.parseMailWinopen(2);
}

function onValidZenyInput(event) {
	const root = _root();
	event.stopImmediatePropagation();

	const zenyAmt = root.querySelector('#zeny_amt');
	if (zenyAmt) zenyAmt.style.display = 'inline-block';

	const zenyOk = root.querySelector('#zeny_ok');
	if (zenyOk) zenyOk.style.display = 'none';

	const input = root.querySelector('.input_zeny_amt');
	let val_Zeny = input ? input.value.split(',').join('') : '0';
	val_Zeny = Math.min(Math.max(0, val_Zeny), Session.zeny);
	val_Zeny = isNaN(val_Zeny) ? 0 : val_Zeny;

	if (input) {
		input.value = prettifyZeny(val_Zeny);
		input.disabled = true;
	}

	Mail.parseMailSetattach(0, val_Zeny);
}

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Drop an item in the equipment, equip it if possible
 */
function onDrop(event) {
	let item, data;
	event.stopImmediatePropagation();
	event.preventDefault();

	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
		item = data.data;
	} catch (_e) {
		return;
	}

	if (data.type !== 'item' || data.from == 'Storage' || data.from == 'Mail') {
		return;
	}

	if (item.count > 1) {
		InputBox.append();
		InputBox.setType('number', false, item.count);
		InputBox.onSubmitRequest = function OnSubmitRequest(count) {
			InputBox.remove();
			Mail.parseMailWinopen(1);

			if (data.from == 'Inventory') {
				Inventory.getUI().removeItem(item.index, parseInt(count, 10));
			}

			Mail.parseMailSetattach(item.index, parseInt(count, 10));
			_preferences.item_add_email = item;
			_preferences.item_add_email.count = parseInt(count, 10);
			_preferences.save();
		};
		return;
	}

	if (data.from == 'Inventory') {
		Inventory.getUI().removeItem(item.index, 1);
	}
	Mail.parseMailWinopen(1);

	Mail.parseMailSetattach(item.index, 1);
	_preferences.item_add_email = item;
	_preferences.item_add_email.count = 1;
	_preferences.save();
}

/**
 * Show item name when mouse is over
 */
function onItemOver() {
	const root = _root();
	const idx = parseInt(this.getAttribute('data-index'), 10);
	const item = Mail.getItemByIndex(idx);

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
 * Hide the item name
 */
function onItemOut() {
	const root = _root();
	const overlay = root.querySelector('.container_item .overlay');
	if (overlay) overlay.style.display = 'none';
}

/**
 * Start dragging an item
 */
function onItemDragStart(event) {
	const index = parseInt(this.getAttribute('data-index'), 10);
	const item = Mail.getItemByIndex(index);

	if (!item) {
		return;
	}

	const img = new Image();
	const url = this.firstChild.style.backgroundImage.match(/\(([^)]+)/)[1];
	img.decoding = 'async';
	img.src = url.replace(/^"/, '').replace(/"$/, '');

	event.dataTransfer.setDragImage(img, 12, 12);
	event.dataTransfer.setData(
		'Text',
		JSON.stringify(
			(window._OBJ_DRAG_ = {
				type: 'item',
				from: 'Mail',
				data: item
			})
		)
	);

	onItemOut();
}

/**
 * Stop dragging an item
 */
function onItemDragEnd() {
	delete window._OBJ_DRAG_;
}

/**
 * Get item info (open description window)
 */
function onItemInfo(event) {
	event.stopImmediatePropagation();

	const index = parseInt(this.getAttribute('data-index'), 10);
	const item = Mail.getItemByIndex(index);

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

/**
 * Converte DeleteTime
 */
function formateDeleteTime(value) {
	const ts_ms = value * 1000;
	const date_ob = new Date(ts_ms);
	const year = date_ob.getFullYear();
	const month = `0${date_ob.getMonth() + 1}`.slice(-2);
	const date = `0${date_ob.getDate()}`.slice(-2);

	return `${month} ${date} ${String(year).substring(2, 4)}`;
}

function removeCreateAllItem() {
	Mail.parseMailWinopen(0);
	Mail.clearFieldsItemZeny();
}

/**
 * Validate the type of information being dropped into the text field
 */
function onDropText(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
	let data;
	try {
		data = JSON.parse(event.dataTransfer.getData('Text'));
	} catch (_e) {
		return;
	}

	if (data.type == 'item') {
		return;
	}

	event.currentTarget.value = data;
}

function sleep(time) {
	return new Promise(resolve => setTimeout(resolve, time));
}

/**
 * Callbacks
 */
Mail.onClosePressed = function onClosePressed() {};
Mail.parseMailWinopen = function parseMailWinopen(/*type*/) {};
Mail.parseMailrefreshinbox = function parseMailrefreshinbox() {};
Mail.parseMailSetattach = function parseMailSetattach(/*index, count*/) {};
Mail.reqRemoveItem = function reqRemoveItem(/*index, count*/) {};
Mail.parseMailSend = function parseMailSend(/*object*/) {};
Mail.openMail = function openMail(/*MailID*/) {};
Mail.replyMail = function replyMail(/*MailID*/) {};

/**
 * Create component and export it
 */
export default UIManager.addComponent(Mail);
