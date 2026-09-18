/**
 * UI/Components/Rodex/Rodex.js
 *
 * Rodex Box
 *
 * @author Alisonrag
 *
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';

import htmlText from './Rodex.html?raw';
import cssText from './Rodex.css?raw';

/**
 * Create Component
 */
const Rodex = new GUIComponent('Rodex', cssText);

/**
 * Store Rodex items
 */
Rodex.list = [];
/**
 * Determine data page size
 */
Rodex.pageSize = 6;
/**
 * know which page is current
 */
Rodex.page = 0;

/**
 * know which tab is current
 */
Rodex.openType = 0;

/**
 * know which tab is current
 */
Rodex.currentTab = 0;

/**
 * know what to search
 */
Rodex.searchType = 1;

Rodex.attachmentType = {
	0: '', // none
	2: 'basic_interface/rodexsystem/renewal/icon_zeny.bmp', // zeny
	4: 'basic_interface/rodexsystem/renewal/icon_item.bmp', // item
	6: 'basic_interface/rodexsystem/renewal/icon_zeny_n_item.bmp', // zeny + item
	12: 'basic_interface/rodexsystem/renewal/icon_zeny_n_item.bmp' // gift??
};

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'Rodex',
	{
		x: 350,
		y: 350,
		show: false
	},
	2.0
);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return Rodex._shadow || Rodex._host;
}

/**
 * Render HTML
 */
Rodex.render = () => htmlText;

/**
 * Apply preferences once append to body
 */
Rodex.onAppend = function OnAppend() {
	const root = _root();

	// Apply preferences
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth)}px`;

	this.draggable(root.querySelector('.titlebar'));

	root.querySelector('.close').addEventListener('click', onClickClose);
	root.querySelector('.refresh').addEventListener('click', onClickRefresh);
	root.querySelector('.write').addEventListener('click', onClickWriteMail);
	root.querySelector('.delete-all').addEventListener('click', onClickDeleteAll);
	root.querySelector('.retrieve-all').addEventListener('click', onClickRetrieveAll);
	root.querySelector('.previous-page').addEventListener('click', onClickPreviousPage);
	root.querySelector('.next-page').addEventListener('click', onClickNexPage);
	root.querySelectorAll('.nav-item').forEach(el => el.addEventListener('click', onClickTab));
	root.querySelector('.search-title').addEventListener('click', onClickSearchTitle);
	root.querySelector('.search-sender').addEventListener('click', onClickSearchSender);
	root.querySelector('.search').value = '';
	root.querySelector('.search-btn').addEventListener('click', onClickSearchButton);

	Rodex.openType = 0;
	root.querySelectorAll('.nav-item.active').forEach(el => el.classList.remove('active'));
	root.querySelector('#tab_0').classList.add('active');
	Rodex.searchType = 1;
	Rodex.page = 0;
};

/**
 * Remove Rodex from window (and so clean up items)
 */
Rodex.onRemove = function OnRemove() {
	this.list.length = 0;
	// Save preferences
	_preferences.show = this._host.style.display !== 'none';
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();

	Rodex.openType = 0;
};

/**
 * Extend Rodex window size
 *
 * @param {object} read
 */
Rodex.initData = function initData(pkt) {
	Rodex.list = pkt.MailList;
	Rodex.isEnd = pkt.isEnd;
	Rodex.openType = typeof pkt.openType !== 'undefined' ? pkt.openType : 0;
	Rodex.createRodexList();
	this._host.style.display = '';
	this.focus();
};

Rodex.createRodexList = function createRodexList(tabID = 0, search = false, term = '') {
	const root = _root();
	const content = root.querySelector('.mail-list');
	content.innerHTML = '';
	let mail_list = [];
	if (search) {
		Rodex.page = 0;
		if (Rodex.searchType === 1) {
			mail_list = Rodex.getMailsByTitle(term);
		} else {
			mail_list = Rodex.getMailsBySender(term);
		}
	} else {
		mail_list = Rodex.getMailsByTabID(tabID);
	}
	if (mail_list.length === 0) {
		return;
	}
	const start = Rodex.pageSize * Rodex.page;

	let total = 0;
	for (let i = start; total < Rodex.pageSize && i < mail_list.length; i++) {
		const mail = mail_list[i];
		const mailID = mail.MailID;
		const title = mail.title.length > 18 ? mail.title.substring(0, 18) + '...' : mail.title;
		const sender = mail.SenderName.length > 18 ? mail.SenderName.substring(0, 18) + '...' : mail.SenderName;
		const mail_image = mail.Isread ? 'icon_status_mail_read' : 'icon_status_mail_received';
		const mail_content = Rodex.attachmentType[mail.type];
		const remaining_days = parseInt(mail.expireDateTime / 60 / 60 / 24);
		const openType = typeof mail.openType !== 'undefined' ? mail.openType : 0;
		const mail_html = `<li class="mail-item">
				<div class="mail-checkbox" data-background="basic_interface/rodexsystem/renewal/checkbox_off.bmp">
				</div>
				<div class="mail-image" data-background="basic_interface/rodexsystem/renewal/${mail_image}.bmp">
				</div>
				<div class="mail-text">
					<div class="title"><div id="mail_${mailID}" openType="${openType}" class="text event_add_cursor"><span data-text="2702"></span>${title}</div></div>
					<div class="sender"><div id="sender_${mailID}" sender="${sender}" class="text event_add_cursor"><span data-text="2701"></span>${sender}</div></div>
				</div>
				<div class="mail-content" data-background="${mail_content}"></div>
				<div class="expire-days">${remaining_days} days</div>
			</li>`;
		content.insertAdjacentHTML('beforeend', mail_html);

		const mailEl = root.querySelector(`#mail_${mailID}`);
		if (mailEl) {
			mailEl.addEventListener('click', onClickReadMail);
		}
		const senderEl = root.querySelector(`#sender_${mailID}`);
		if (senderEl) {
			senderEl.addEventListener('click', onClickReplyMail);
		}
		total++;
	}

	// Process data-* attributes on dynamically created content
	const selector = '[data-background],[data-hover],[data-down],[data-active],[data-text],[data-preload]';
	content.querySelectorAll(selector).forEach(node => {
		GUIComponent.processDataAttrs(node);
	});
};

Rodex.getMailsByTabID = function getMailsByTabID(tabID) {
	return Rodex.list.filter(mail => mail.openType == tabID);
};

Rodex.getMailsBySender = function getMailsBySender(term) {
	return Rodex.list.filter(mail => mail.SenderName.indexOf(term) > -1);
};

Rodex.getMailsByTitle = function getMailsByTitle(term) {
	return Rodex.list.filter(mail => mail.title.indexOf(term) > -1);
};

Rodex.getMailByID = function getMailByID(mailID) {
	return Rodex.list.find(mail => mail.MailID == mailID);
};

Rodex.getAll = function getAll() {
	for (let i = 0; i < Rodex.list.length; i++) {
		const mail = Rodex.list[i];
		if (mail.type > 0 && (mail.type === 4 || mail.type === 6)) {
			Rodex.requestItemsFromRodex(mail.openType, mail.MailID);
		}
		if (mail.type > 0 && (mail.type === 2 || mail.type === 6)) {
			Rodex.requestZenyFromRodex(mail.openType, mail.MailID);
		}
	}
};

Rodex.deleteAll = function deleteAll() {
	for (let i = 0; i < Rodex.list.length; i++) {
		const mail = Rodex.list[i];
		if (mail.type === 0) {
			Rodex.requestDeleteRodex(mail.openType, mail.MailID);
		} else {
			ChatBox.addText(DB.getMessage(2612), ChatBox.TYPE.INFO_MAIL, ChatBox.FILTER.PUBLIC_LOG);
		}
	}
};

Rodex.updateDeletedMailContent = function updateDeletedMailContent(openType, MailID) {
	const root = _root();
	const mailEl = root.querySelector(`#mail_${MailID}`);
	if (mailEl) {
		mailEl.textContent = DB.getMessage(2907);
		mailEl.classList.add('deleted');
	}
	const senderEl = root.querySelector(`#sender_${MailID}`);
	if (senderEl) {
		senderEl.textContent = '';
	}
};

/**
 * Show/Hide UI
 */
Rodex.toggle = function toggle() {
	if (this._host && this._host.style.display !== 'none') {
		Rodex.closeRodexBox();
		this._host.style.display = 'none';
	} else {
		Rodex.openRodexBox();
		Rodex.append();
		this._host.style.display = '';
		this.focus();
	}
};

Rodex.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host && this._host.style.display !== 'none') {
		this.toggle();
	}
};

function onClickClose(e) {
	e.stopImmediatePropagation();
	Rodex.openType = 0;
	Rodex.closeRodexBox();
	// Save preferences
	_preferences.y = parseInt(Rodex._host.style.top, 10);
	_preferences.x = parseInt(Rodex._host.style.left, 10);
	_preferences.save();
	Rodex._host.style.display = 'none';
}

function onClickRefresh(e) {
	e.stopImmediatePropagation();
	Rodex.requestRefreshRodexPage();
}

function onClickWriteMail(e) {
	e.stopImmediatePropagation();
	Rodex.requestOpenWriteRodex();
}

function onClickDeleteAll(e) {
	e.stopImmediatePropagation();
	UIManager.showPromptBox(DB.getMessage(3590), 'ok', 'cancel', () => {
		Rodex.deleteAll();
	});
}

function onClickRetrieveAll(e) {
	e.stopImmediatePropagation();
	UIManager.showPromptBox(DB.getMessage(3594), 'ok', 'cancel', () => {
		Rodex.getAll();
	});
}

function onClickPreviousPage(e) {
	e.stopImmediatePropagation();
	if (Rodex.page - 1 < 0) {
		return;
	}
	Rodex.page--;
	Rodex.createRodexList(Rodex.openType);
}

function onClickNexPage(e) {
	e.stopImmediatePropagation();
	if ((Rodex.page + 1) * Rodex.pageSize < Rodex.getMailsByTabID(Rodex.openType).length) {
		Rodex.page++;
		Rodex.createRodexList(Rodex.openType);
	}
}

function onClickTab(e) {
	e.stopImmediatePropagation();
	Rodex.page = 0;
	const element = e.currentTarget;
	const tid = element.id;
	const id = tid.replace('tab_', '');
	const root = _root();
	root.querySelectorAll('.nav-item.active').forEach(el => el.classList.remove('active'));
	element.classList.add('active');
	if (id >= 0 && id <= 2) {
		Rodex.openType = id;
		Rodex.createRodexList(Rodex.openType);
	} else {
		root.querySelector('.mail-list').innerHTML = '';
	}
}

function onClickSearchTitle(e) {
	e.stopImmediatePropagation();
	Rodex.searchType = 1;
	const root = _root();
	Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/rodexsystem/renewal/checkbox_search_off.bmp', data => {
		const el = root.querySelector('.search-sender');
		if (el) {
			el.style.backgroundImage = `url(${data})`;
		}
	});
	Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/rodexsystem/renewal/checkbox_search_on.bmp', data => {
		const el = root.querySelector('.search-title');
		if (el) {
			el.style.backgroundImage = `url(${data})`;
		}
	});
}

function onClickSearchSender(e) {
	e.stopImmediatePropagation();
	Rodex.searchType = 2;
	const root = _root();
	Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/rodexsystem/renewal/checkbox_search_on.bmp', data => {
		const el = root.querySelector('.search-sender');
		if (el) {
			el.style.backgroundImage = `url(${data})`;
		}
	});
	Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/rodexsystem/renewal/checkbox_search_off.bmp', data => {
		const el = root.querySelector('.search-title');
		if (el) {
			el.style.backgroundImage = `url(${data})`;
		}
	});
}

function onClickSearchButton(e) {
	e.stopImmediatePropagation();
	const root = _root();
	const search = root.querySelector('.search').value;
	root.querySelectorAll('.nav-item.active').forEach(el => el.classList.remove('active'));
	root.querySelector('#tab_3').classList.add('active');
	if (search.length > 2) {
		Rodex.createRodexList(0, true, search);
	}
}

function onClickReadMail(e) {
	e.stopImmediatePropagation();
	const element = e.currentTarget;
	const mid = element.id;
	const openType = element.getAttribute('openType');
	const id = mid.replace('mail_', '');
	Rodex.requestReadRodex(openType, id);
}

function onClickReplyMail(e) {
	e.stopImmediatePropagation();
	const element = e.currentTarget;
	const sender = element.getAttribute('sender');
	Rodex.requestOpenWriteRodex(sender);
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(Rodex);
