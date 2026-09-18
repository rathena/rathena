/**
 * UI/Components/ChatBox/ChatBox.js
 *
 * ChatBox windows
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Renderer from 'Renderer/Renderer.js';
import Client from 'Core/Client.js';
import Events from 'Core/Events.js';
import Preferences from 'Core/Preferences.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Mouse from 'Controls/MouseEventHandler.js';
import Cursor from 'UI/CursorManager.js';
import BattleMode from 'Controls/BattleMode.js';
import History from './History.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import ContextMenu from 'UI/Components/ContextMenu/ContextMenu.js';
import htmlText from './ChatBox.html?raw';
import cssText from './ChatBox.css?raw';
import Commands from 'Controls/ProcessCommand.js';
import ChatBoxSettings from 'UI/Components/ChatBoxSettings/ChatBoxSettings.js';
import Configs from 'Core/Configs.js';
import EntityManager from 'Renderer/EntityManager.js';

/**
 * @var {number} max message in the chatbox
 */
const MAX_MSG = 400;
const MAX_LENGTH = 100;
const MAGIC_NUMBER = 3 * 14;

/**
 * @var {History} message cached in history
 */
const _historyMessage = new History();

/**
 * @var {History} nickname cached in history
 */
const _historyNickName = new History(true);

/**
 * @var {number} Chatbox position's index
 */
let _heightIndex = 2;

/**
 * Buffer para acumular mensagens antes de adicionar ao DOM.
 * @private
 * @type {ChatMessage[]}
 */
let _messageBuffer = [];

/**
 * Flag que indica se um requestAnimationFrame foi agendado para processar o buffer.
 * @private
 * @type {boolean}
 */
let _rafScheduled = false;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'ChatBox',
	{
		x: 0,
		y: Infinity,
		height: 2,
		fontScale: 1.0,
		magnet_top: false,
		magnet_bottom: true,
		magnet_left: true,
		magnet_right: false,
		tabs: [],
		tabOption: [],
		activeTab: 0
	},
	1.0
);

/**
 * Create Basic Info component
 */
const ChatBox = new GUIComponent('ChatBox', cssText);

/**
 * Helper: query inside shadow root
 */
function _root() {
	return ChatBox._shadow || ChatBox._host;
}

/**
 * Render HTML
 */
ChatBox.render = () => htmlText;

/**
 * Has input fields, protect key events
 */
ChatBox.captureKeyEvents = true;

/**
 * Constants
 */
ChatBox.TYPE = {
	SELF: 1 << 0,
	PUBLIC: 1 << 1,
	PRIVATE: 1 << 2,
	PARTY: 1 << 3,
	GUILD: 1 << 4,
	ANNOUNCE: 1 << 5,
	ERROR: 1 << 6,
	INFO: 1 << 7,
	BLUE: 1 << 8,
	ADMIN: 1 << 9,
	MAIL: 1 << 10,
	CLAN: 1 << 11
};

ChatBox.FILTER = {
	PUBLIC_LOG: 0,
	PUBLIC_CHAT: 1,
	WHISPER: 2,
	PARTY: 3,
	GUILD: 4,
	ITEM: 5,
	EQUIP: 6,
	STATUS: 7,
	PARTY_ITEM: 8,
	PARTY_STATUS: 9,
	SKILL_FAIL: 10,
	PARTY_SETUP: 11,
	EQUIP_DAMAGE: 12,
	WOE: 13,
	PARTY_SEARCH: 14,
	BATTLE: 15,
	PARTY_BATTLE: 16,
	EXP: 17,
	PARTY_EXP: 18,
	QUEST: 19,
	BATTLEFIELD: 20,
	CLAN: 21
};

/**
 * @var {number} target message ?
 */
ChatBox.sendTo = ChatBox.TYPE.PUBLIC;

/**
 * Storage to cache the private messages
 */
ChatBox.PrivateMessageStorage = {
	nick: '',
	msg: ''
};

ChatBox.lastTabID = -1;
ChatBox.tabCount = 0;
ChatBox.activeTab = 0;

ChatBox.tabs = [];

/**
 * Initialize UI
 */
ChatBox.init = function init() {
	const root = _root();

	if (!ContextMenu.__loaded) ContextMenu.prepare();
	_heightIndex = _preferences.height - 1;
	ChatBox.updateHeight();
	ChatBox.applyFontScale();

	this._host.style.top = `${Math.min(Math.max(0, _preferences.y - (this._host.offsetHeight || 0)), Renderer.height - (this._host.offsetHeight || 0))}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - (this._host.offsetWidth || 0))}px`;

	this.magnet.TOP = _preferences.magnet_top;
	this.magnet.BOTTOM = _preferences.magnet_bottom;
	this.magnet.LEFT = _preferences.magnet_left;
	this.magnet.RIGHT = _preferences.magnet_right;

	this.draggable('.input');
	this.draggable('.battlemode');

	// Keep chat log area click-through for walking; only block over interactive UI parts.
	// For GUIComponent, set up manual mouse intersection blocking on interactive elements.
	const interactiveSelector = '.input, .chat-function, .battlemode, .event_add_cursor';
	const interactiveEls = root.querySelectorAll(interactiveSelector);
	interactiveEls.forEach(el => {
		let _intersect;
		let _enter = 0;
		el.addEventListener('mouseenter', () => {
			if (_enter === 0) {
				_intersect = Mouse.intersect;
				_enter++;
				if (_intersect) {
					Mouse.intersect = false;
					Cursor.setType(Cursor.ACTION.DEFAULT);
					EntityManager.setOverEntity(null);
				}
			}
		});
		el.addEventListener('mouseleave', () => {
			if (_enter > 0) {
				_enter--;
				if (_enter === 0 && _intersect) {
					Mouse.intersect = true;
					EntityManager.setOverEntity(null);
				}
			}
		});
	});

	// Setting chatbox scrollbar (inject into shadow style)
	Client.loadFiles(
		[DB.INTERFACE_PATH + 'basic_interface/dialscr_down.bmp', DB.INTERFACE_PATH + 'basic_interface/dialscr_up.bmp'],
		(down, up) => {
			const scrollbarCSS = [
				'#chatbox .content::-webkit-scrollbar { width: 10px; height: 10px;}',
				'#chatbox .content::-webkit-scrollbar-button:vertical:start:increment,',
				'#chatbox .content::-webkit-scrollbar-button:vertical:end:decrement { display: none;}',
				'#chatbox .content::-webkit-scrollbar-corner:vertical { display:none;}',
				'#chatbox .content::-webkit-scrollbar-resizer:vertical { display:none;}',
				'#chatbox .content::-webkit-scrollbar-button:start:decrement,',
				'#chatbox .content::-webkit-scrollbar-button:end:increment { display: block; border:none;}',
				`#chatbox .content::-webkit-scrollbar-button:vertical:increment { background: url(${down}) no-repeat; height:10px;}`,
				`#chatbox .content::-webkit-scrollbar-button:vertical:decrement { background: url(${up}) no-repeat; height:10px;}`,
				'#chatbox .content::-webkit-scrollbar-track-piece:vertical { background:black; border:none;}',
				'#chatbox .content::-webkit-scrollbar-thumb:vertical { background:grey; -webkit-border-image:none; border-color:transparent;border-width: 0px 0; }'
			].join('\n');

			const style = document.createElement('style');
			style.textContent = scrollbarCSS;
			if (ChatBox._shadow) {
				ChatBox._shadow.appendChild(style);
			}
		}
	);

	// Input selection
	const usernameInput = root.querySelector('.input input');
	if (usernameInput) {
		usernameInput.addEventListener('mousedown', function (event) {
			this.select();
			event.stopImmediatePropagation();
			event.preventDefault();
		});
	}

	const inputChatbox = root.querySelector('.input-chatbox');

	if (Configs.get('restoreChatFocus', false) && inputChatbox) {
		inputChatbox.addEventListener('blur', () => {
			Events.setTimeout(() => {
				const active = KEYS.getDeepActiveElement();
				const movedInsideChatbox = active && root.querySelector('#chatbox').contains(active);
				const isTextInput = active && active.tagName && active.tagName.match(/input|select|textarea/i);
				if (!movedInsideChatbox && !isTextInput) {
					inputChatbox.focus();
				}
			}, 1000);
		});
	}

	// Move caret to end of text
	if (inputChatbox) {
		inputChatbox.addEventListener('click', function () {
			const range = document.createRange();
			const selection = window.getSelection();
			range.selectNodeContents(this);
			range.collapse(false);
			selection.removeAllRanges();
			selection.addRange(range);
		});

		inputChatbox.addEventListener('focus', function () {
			const range = document.createRange();
			const selection = window.getSelection();
			range.selectNodeContents(this);
			range.collapse(false);
			selection.removeAllRanges();
			selection.addRange(range);
		});

		inputChatbox.maxLength = MAX_LENGTH;

		inputChatbox.addEventListener('input', event => {
			const currentText = extractChatMessage(inputChatbox);
			if (currentText.length >= MAX_LENGTH) {
				event.preventDefault();
			}
		});

		inputChatbox.addEventListener('keydown', event => {
			const currentText = extractChatMessage(inputChatbox);
			if (currentText.length >= MAX_LENGTH) {
				const allowedKeys = [
					'ArrowLeft',
					'ArrowUp',
					'ArrowRight',
					'ArrowDown',
					'Backspace',
					'Delete',
					'Enter',
					'Insert'
				];

				if (allowedKeys.includes(event.key)) {
					return true;
				}

				if (event.ctrlKey || event.altKey) {
					return true;
				}
				event.preventDefault();
				return false;
			}
		});

		inputChatbox.addEventListener('paste', event => {
			event.preventDefault();

			const clipboard = (event.originalEvent || event).clipboardData || event.clipboardData;
			let pastedText = clipboard ? clipboard.getData('text/plain') : '';
			if (!pastedText) {
				return;
			}

			pastedText = pastedText.replace(/\u00A0/g, ' ');

			const currentText = extractChatMessage(inputChatbox);
			const remaining = MAX_LENGTH - currentText.length;
			if (remaining <= 0) {
				return;
			}

			const toInsert = pastedText.substr(0, remaining);

			if (document.queryCommandSupported && document.queryCommandSupported('insertText')) {
				document.execCommand('insertText', false, toInsert);
			} else {
				const node = document.createTextNode(toInsert);
				inputChatbox.appendChild(node);
			}
		});
	}

	const nickBox = root.querySelector('.input .username');
	if (nickBox) {
		nickBox.addEventListener('blur', () => {
			Events.setTimeout(() => {
				const active = KEYS.getDeepActiveElement();
				const movedInsideChatbox = active && root.querySelector('#chatbox').contains(active);
				const isTextInput = active && active.tagName && active.tagName.match(/input|select|textarea/i);
				const isChatMessage = active === inputChatbox;
				if (!movedInsideChatbox && !isTextInput && !isChatMessage) {
					nickBox.focus();
				}
			}, 1000);
		});
	}

	// Validate information dragged into text field
	root.querySelectorAll('input[type=text]').forEach(input => {
		input.addEventListener('drop', onDropText);
		input.addEventListener('dragover', stopPropagation);
	});

	// Button change name (tab inputs)
	root.querySelectorAll('.header input').forEach(input => {
		input.addEventListener('dblclick', function () {
			this.type = 'text';
			this.select();
		});
		input.addEventListener('blur', function () {
			this.type = 'button';
		});
	});

	// Private message selection
	const listBtn = root.querySelector('.input .list');
	if (listBtn) {
		listBtn.addEventListener('click', function () {
			const names = _historyNickName.list;
			const count = names.length;

			if (!count) {
				ChatBox.addText(DB.getMessage(192), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
				return;
			}

			ContextMenu.remove();
			ContextMenu.append();

			for (let i = 0; i < count; ++i) {
				ContextMenu.addElement(names[i], onPrivateMessageUserSelection(names[i]));
			}

			ContextMenu.addElement('', onPrivateMessageUserSelection(''));

			const pos = this.getBoundingClientRect();
			const ui = ContextMenu.ui.find('.menu');
			ui.css({
				top: pos.top - ui.height() - 5,
				left: pos.left - ui.width() - 5
			});
		});
		listBtn.addEventListener('mousedown', event => {
			event.stopImmediatePropagation();
			event.preventDefault();
		});
	}

	const draggableEl = root.querySelector('.draggable');
	if (draggableEl) {
		draggableEl.addEventListener('mousedown', event => {
			event.stopPropagation();
		});
	}

	// Send message to...
	const filterBtn = root.querySelector('.input .filter');
	if (filterBtn) {
		filterBtn.addEventListener('click', function () {
			const pos = this.getBoundingClientRect();
			const ui = ContextMenu.ui.find('.menu');

			ContextMenu.remove();
			ContextMenu.append();

			ContextMenu.addElement(DB.getMessage(85), onChangeTargetMessage(ChatBox.TYPE.PUBLIC));
			ContextMenu.addElement(DB.getMessage(86), onChangeTargetMessage(ChatBox.TYPE.PARTY));
			ContextMenu.addElement(DB.getMessage(437), onChangeTargetMessage(ChatBox.TYPE.GUILD));
			ContextMenu.addElement(DB.getMessage(2361), onChangeTargetMessage(ChatBox.TYPE.CLAN));

			ui.css({
				top: pos.top - ui.height() - 5,
				left: pos.left - ui.width() + 25
			});
		});
		filterBtn.addEventListener('mousedown', event => {
			event.stopImmediatePropagation();
			event.preventDefault();
		});
	}

	// Change size
	const sizeBtn = root.querySelector('.input .size');
	if (sizeBtn) {
		sizeBtn.addEventListener('click', event => {
			ChatBox.updateHeight(true);
			event.stopImmediatePropagation();
			event.preventDefault();
		});
	}

	// Scroll feature should block at each line
	root.querySelectorAll('.content').forEach(el => {
		el.addEventListener('wheel', onScroll);
	});

	// Tabs should behave like "UI" (no entity hover / map cursor changes)
	(function initTabHoverBlock() {
		let _tabIntersect;
		let _tabEnter = 0;

		root.querySelector('#chatbox').addEventListener(
			'mouseenter',
			event => {
				if (event.target.closest('table.header tr td.tab')) {
					if (_tabEnter === 0) {
						_tabIntersect = Mouse.intersect;
						_tabEnter++;
						if (_tabIntersect) {
							Mouse.intersect = false;
							Cursor.setType(Cursor.ACTION.DEFAULT);
							EntityManager.setOverEntity(null);
						}
					}
				}
			},
			true
		);

		root.querySelector('#chatbox').addEventListener(
			'mouseleave',
			event => {
				if (event.target.closest('table.header tr td.tab')) {
					if (_tabEnter > 0) {
						_tabEnter--;
						if (_tabEnter === 0 && _tabIntersect) {
							Mouse.intersect = true;
							EntityManager.setOverEntity(null);
						}
					}
				}
			},
			true
		);

		// Prevent walking when clicking tabs
		root.querySelector('#chatbox').addEventListener(
			'mousedown',
			event => {
				if (event.target.closest('table.header tr td.tab')) {
					event.stopPropagation();
				}
			},
			true
		);
	})();

	// Prevent map right-click (camera rotate) when using chat right-click features
	const chatBody = root.querySelector('.body');
	if (chatBody) {
		chatBody.addEventListener('mousedown', event => {
			if (event.which !== 3) {
				return;
			}
			event.preventDefault();
			event.stopPropagation();
		});
	}

	// Chat font scale context menu (right click)
	const chatboxEl = root.querySelector('#chatbox');
	if (chatboxEl) {
		chatboxEl.addEventListener('contextmenu', event => {
			const target = event.target;
			if (target.closest('.body, .contentwrapper, .content')) {
				if (target.closest('a, .item-link, .event_add_cursor, .chat-function, td.tab')) {
					return;
				}

				event.preventDefault();
				event.stopPropagation();

				Mouse.screen.x = event.pageX;
				Mouse.screen.y = event.pageY;

				ContextMenu.remove();
				ContextMenu.append();
				ContextMenu.addElement('Chat font x1.0', setChatFontScale(1.0));
				ContextMenu.addElement('Chat font x1.2', setChatFontScale(1.2));
				ContextMenu.addElement('Chat font x1.4', setChatFontScale(1.4));
			}
		});
	}

	// Clicking interactive elements in chat should not trigger map movement
	if (chatboxEl) {
		chatboxEl.addEventListener('mousedown', event => {
			if (event.target.closest('.content a, .content .item-link')) {
				event.stopPropagation();
			}
		});
	}

	const bmtoggle = root.querySelector('.battlemode .bmtoggle');
	if (bmtoggle) {
		bmtoggle.addEventListener('click', () => {
			const inputEl = root.querySelector('.input');
			const bmEl = root.querySelector('.battlemode');
			if (inputEl) inputEl.style.display = inputEl.style.display === 'none' ? 'block' : 'none';
			if (bmEl) bmEl.style.display = bmEl.style.display === 'none' ? 'block' : 'none';
		});
	}

	const battleopt2 = root.querySelector('.chat-function .battleopt2');
	if (battleopt2) {
		battleopt2.addEventListener('click', () => {
			if (ChatBox.tabCount <= 5) {
				ChatBox.addNewTab();
				ChatBox.onAppend();
			}
		});
	}

	// Tab click handler (delegated)
	if (chatboxEl) {
		chatboxEl.addEventListener('click', event => {
			const tab = event.target.closest('table.header tr td.tab');
			if (tab) {
				event.stopImmediatePropagation();
				if (ChatBox.activeTab != tab.dataset.tab) {
					ChatBox.switchTab(tab.dataset.tab);
				}
			}
		});
	}

	const wndminib = root.querySelector('.chat-function .wndminib');
	if (wndminib) {
		wndminib.addEventListener('click', () => {
			if (ChatBox.tabCount > 1) {
				ChatBox.removeTab();
			}
		});
	}

	const chatmodeBtn = root.querySelector('.chat-function .chatmode');
	if (chatmodeBtn) {
		chatmodeBtn.addEventListener('click', () => {
			ChatBox.toggleChat();
		});
	}

	const battleoptBtn = root.querySelector('.chat-function .battleopt');
	if (battleoptBtn) {
		battleoptBtn.addEventListener('click', () => {
			ChatBox.toggleChatBattleOption();
		});
	}

	// Init settings window as well
	ChatBoxSettings.append();

	if (_preferences.tabs.length > 0 && _preferences.tabs.length == _preferences.tabOption.length) {
		// Load saved tabs
		for (let i = 0; i < _preferences.tabs.length; i++) {
			if (_preferences.tabs[i]) {
				const tabName = _preferences.tabs[i].name;
				const tabSettings = _preferences.tabOption[i];
				ChatBox.addNewTab(tabName, tabSettings);
			}
		}

		// switch to active tab
		if (_preferences.activeTab !== undefined && ChatBox.tabs[_preferences.activeTab]) {
			ChatBox.switchTab(_preferences.activeTab);
		}
	} else {
		// Create default tabs
		const firstTab = ChatBox.addNewTab(DB.getMessage(1291), [
			ChatBox.FILTER.PUBLIC_LOG,
			ChatBox.FILTER.PUBLIC_CHAT,
			ChatBox.FILTER.WHISPER,
			ChatBox.FILTER.PARTY,
			ChatBox.FILTER.GUILD,
			ChatBox.FILTER.ITEM,
			ChatBox.FILTER.EQUIP,
			ChatBox.FILTER.STATUS,
			ChatBox.FILTER.PARTY_ITEM,
			ChatBox.FILTER.PARTY_STATUS,
			ChatBox.FILTER.SKILL_FAIL,
			ChatBox.FILTER.PARTY_SETUP,
			ChatBox.FILTER.EQUIP_DAMAGE,
			ChatBox.FILTER.WOE,
			ChatBox.FILTER.PARTY_SEARCH,
			ChatBox.FILTER.QUEST,
			ChatBox.FILTER.BATTLEFIELD,
			ChatBox.FILTER.CLAN
		]); // Public Log

		ChatBox.addNewTab(DB.getMessage(1292)); // Battle Log

		// switch to first
		ChatBox.switchTab(firstTab);
	}

	// dialog box size
	makeResizableDiv();

	Commands.add(
		'savechat',
		'Saves current chat tab to txt file.',
		() => {
			ChatBox.saveCurrentTabChat();
			return;
		},
		['sc'],
		false
	);

	// Set up item link click handler inside shadow DOM
	ChatBox._setupItemLinkHandler();
};

/**
 * Clean up the box
 */
ChatBox.clean = function Clean() {
	const root = _root();

	const contents = root.querySelectorAll('.content');
	contents.forEach(content => {
		const matches = content.innerHTML.match(/(blob:[^"]+)/g);
		if (matches) {
			for (let i = 0, count = matches.length; i < count; ++i) {
				window.URL.revokeObjectURL(matches[i]);
			}
		}
		content.innerHTML = '';
	});

	const inputChatbox = root.querySelector('.input-chatbox');
	if (inputChatbox) inputChatbox.innerHTML = '';

	const nickBox = root.querySelector('.input .username');
	if (nickBox) nickBox.value = '';

	_historyMessage.clear();
	_historyNickName.clear();
};

ChatBox.toggleChatBattleOption = function toggleChatBattleOption() {
	const root = _root();
	const onInput = root.querySelector('.header tr td div.on input');
	const tabName = onInput ? onInput.value : '';
	ChatBoxSettings.toggle();
	ChatBoxSettings.updateTab(this.activeTab, tabName);
};

ChatBox.removeTab = function removeTab() {
	const root = _root();
	const tabEl = root.querySelector(`table.header tr td.tab[data-tab="${this.activeTab}"]`);
	if (tabEl) tabEl.remove();

	const contentEl = root.querySelector(`.body .content[data-content="${this.activeTab}"]`);
	if (contentEl) contentEl.remove();

	const _elem = root.querySelectorAll('table.header tr td.tab');
	const lastElem = _elem[_elem.length - 1];

	delete ChatBoxSettings.tabOption[this.activeTab];
	delete this.tabs[this.activeTab];
	this.tabCount--;

	ChatBox.switchTab(lastElem.dataset.tab);

	const onInput = root.querySelector('.header tr td div.on input');
	const tabName = onInput ? onInput.value : '';
	ChatBoxSettings.updateTab(this.activeTab, tabName);
};

ChatBox.addNewTab = function addNewTab(name, settings) {
	const root = _root();

	if (!name) {
		name = 'New Tab';
	}
	if (!settings) {
		settings = [
			ChatBox.FILTER.PUBLIC_LOG,
			ChatBox.FILTER.PUBLIC_CHAT,
			ChatBox.FILTER.WHISPER,
			ChatBox.FILTER.PARTY,
			ChatBox.FILTER.GUILD,
			ChatBox.FILTER.ITEM,
			ChatBox.FILTER.EQUIP,
			ChatBox.FILTER.STATUS,
			ChatBox.FILTER.PARTY_ITEM,
			ChatBox.FILTER.PARTY_STATUS,
			ChatBox.FILTER.SKILL_FAIL,
			ChatBox.FILTER.PARTY_SETUP,
			ChatBox.FILTER.EQUIP_DAMAGE,
			ChatBox.FILTER.WOE,
			ChatBox.FILTER.PARTY_SEARCH,
			ChatBox.FILTER.BATTLE,
			ChatBox.FILTER.PARTY_BATTLE,
			ChatBox.FILTER.EXP,
			ChatBox.FILTER.PARTY_EXP,
			ChatBox.FILTER.QUEST,
			ChatBox.FILTER.BATTLEFIELD,
			ChatBox.FILTER.CLAN
		];
	}

	const tabName = name;
	const tabID = ++this.lastTabID;

	const tab = {};
	tab.id = tabID;
	tab.name = tabName;

	// Remove current active state
	root.querySelectorAll('table.header tr td.tab div').forEach(el => el.classList.remove('on'));
	root.querySelectorAll('.body .content').forEach(el => el.classList.remove('active'));

	// Add new elements as active
	const opttab = root.querySelector('table.header tr .opttab');
	if (opttab) {
		opttab.insertAdjacentHTML(
			'beforebegin',
			`
			<td class="tab" data-tab="${tabID}">
				<div class="on">
					<input type="text" value="${tabName}"/>
				</div>
			</td>
		`
		);
	}

	const newTabInput = root.querySelector(`table.header tr td.tab[data-tab="${tabID}"] div input`);
	if (newTabInput) {
		newTabInput.addEventListener('change', function () {
			ChatBox.tabs[tabID].name = this.value;
		});
	}

	const contentWrapper = root.querySelector('.body .contentwrapper');
	if (contentWrapper) {
		contentWrapper.insertAdjacentHTML(
			'beforeend',
			`<div class="content active" data-content="${tabID}" data-scrollbar-skin="chatbox"></div>`
		);
	}

	// Bind scroll handler on new content
	const newContent = root.querySelector(`.body .content[data-content="${tabID}"]`);
	if (newContent) {
		newContent.addEventListener('wheel', onScroll);
	}

	ChatBoxSettings.tabOption[tabID] = settings;

	this.tabs[tabID] = tab;
	this.activeTab = tabID;

	this.tabCount++;

	ChatBoxSettings.updateTab(this.activeTab, tabName);

	return tabID;
};

ChatBox.switchTab = function switchTab(tabID) {
	const root = _root();

	root.querySelectorAll('table.header tr td.tab div').forEach(el => el.classList.remove('on'));
	root.querySelectorAll('.body .content').forEach(el => el.classList.remove('active'));

	this.activeTab = tabID;

	const tabDiv = root.querySelector(`table.header tr td.tab[data-tab="${this.activeTab}"] div`);
	if (tabDiv) tabDiv.classList.add('on');

	const contentDiv = root.querySelector(`.body .content[data-content="${this.activeTab}"]`);
	if (contentDiv) {
		contentDiv.classList.add('active');
		contentDiv.scrollTop = contentDiv.scrollHeight;
	}

	const onInput = root.querySelector('.header tr td div.on input');
	const tabName = onInput ? onInput.value : '';

	ChatBoxSettings.updateTab(this.activeTab, tabName);
};

/**
 * Once append to HTML
 */
ChatBox.onAppend = function OnAppend() {
	const root = _root();
	const inputEl = root.querySelector('.input');
	if (inputEl) inputEl.style.display = 'none';

	const bmEl = root.querySelector('.battlemode');
	if (bmEl) bmEl.style.display = 'block';

	const content = root.querySelector('.content.active');
	if (content) content.scrollTop = content.scrollHeight;
};

/**
 * Stop custom scroll
 */
ChatBox.onRemove = function OnRemove() {
	_preferences.y = (parseInt(this._host.style.top, 10) || 0) + (this._host.offsetHeight || 0);
	_preferences.x = parseInt(this._host.style.left, 10) || 0;
	_preferences.height = _heightIndex;
	_preferences.magnet_top = this.magnet.TOP;
	_preferences.magnet_bottom = this.magnet.BOTTOM;
	_preferences.magnet_left = this.magnet.LEFT;
	_preferences.magnet_right = this.magnet.RIGHT;

	_preferences.tabs = this.tabs;
	_preferences.tabOption = ChatBoxSettings.tabOption;
	_preferences.activeTab = this.activeTab;

	_preferences.save();

	this.lastTabID = -1;
	this.activeTab = 0;
};

/**
 * @param {number} key id to check
 * @return {boolean} found a shortcut ?
 */
ChatBox.processBattleMode = function processBattleMode(keyId) {
	const root = _root();
	const bmEl = root.querySelector('.battlemode');
	if (
		(bmEl && bmEl.style.display !== 'none') ||
		KEYS.ALT ||
		KEYS.SHIFT ||
		KEYS.CTRL ||
		(keyId >= KEYS.F1 && keyId <= KEYS.F24) ||
		KEYS.INSERT
	) {
		return BattleMode.process(keyId);
	}

	return false;
};

/**
 * Key Event Handler
 */
ChatBox.onKeyDown = function OnKeyDown(event) {
	const root = _root();
	const messageBox = root.querySelector('.input-chatbox');
	const nickBox = root.querySelector('.input .username');

	const onInput = root.querySelector('.header tr td div.on input');
	if (onInput) {
		onInput.addEventListener('keyup', function () {
			ChatBoxSettings.updateTab(ChatBox.activeTab, this.value);
		});
	}

	const activeElement = KEYS.getDeepActiveElement();
	const isChatInputFocused = activeElement === messageBox || activeElement === nickBox;
	const isOtherTextInputFocused =
		activeElement &&
		!isChatInputFocused &&
		((activeElement.tagName && activeElement.tagName.match(/input|select|textarea/i)) ||
			activeElement.isContentEditable);

	if (isOtherTextInputFocused) {
		return true;
	}

	switch (event.which) {
		default:
			if (isChatInputFocused) {
				if (event.which >= KEYS.F1 && event.which <= KEYS.F24) {
					if (event.which === KEYS.F11 || event.which === KEYS.F12) {
						event.preventDefault();
						return true;
					}

					if (ChatBox.processBattleMode(event.which)) {
						event.preventDefault();
						event.stopImmediatePropagation();
						return false;
					}
					event.preventDefault();
					event.stopImmediatePropagation();
					return false;
				}

				if (event.getModifierState && event.getModifierState('AltGraph')) {
					event.stopImmediatePropagation();
					return true;
				}

				if (event.ctrlKey || KEYS.CTRL) {
					const isEditingCombo =
						event.which === KEYS.C ||
						event.which === KEYS.V ||
						event.which === KEYS.X ||
						event.which === KEYS.A ||
						event.which === KEYS.Z ||
						event.which === KEYS.Y;

					if (!isEditingCombo) {
						if (ChatBox.processBattleMode(event.which)) {
							event.preventDefault();
							event.stopImmediatePropagation();
							return false;
						}
						event.preventDefault();
						return true;
					}

					event.stopImmediatePropagation();
					return true;
				}

				if (event.altKey || KEYS.ALT) {
					const isAltEditingCombo =
						event.which === KEYS.LEFT ||
						event.which === KEYS.RIGHT ||
						event.which === KEYS.UP ||
						event.which === KEYS.DOWN ||
						event.which === KEYS.BACKSPACE ||
						event.which === KEYS.DELETE ||
						event.which === KEYS.HOME ||
						event.which === KEYS.END;

					if (!isAltEditingCombo) {
						if (ChatBox.processBattleMode(event.which)) {
							event.preventDefault();
							event.stopImmediatePropagation();
							return false;
						}
						event.preventDefault();
						return true;
					}

					event.stopImmediatePropagation();
					return true;
				}

				if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
					return true;
				}

				event.stopImmediatePropagation();
				return true;
			}

			if (
				(event.target.tagName && !event.target.tagName.match(/input|select|textarea/i)) ||
				(event.which >= KEYS.F1 && event.which <= KEYS.F24) ||
				KEYS.ALT ||
				KEYS.SHIFT ||
				KEYS.CTRL
			) {
				if (ChatBox.processBattleMode(event.which)) {
					event.stopImmediatePropagation();
					return false;
				}
			}
			return true;

		// Message from history
		case KEYS.UP:
			if (!document.querySelector('#NpcMenu')) {
				if (activeElement === messageBox) {
					if (shouldLetChatInputHandleVerticalArrows(messageBox, 'up')) {
						event.stopImmediatePropagation();
						return true;
					}
					messageBox.innerHTML = _historyMessage.previous();
					break;
				}

				if (activeElement === nickBox) {
					nickBox.value = _historyNickName.previous();
					nickBox.select();
					break;
				}
			}
			return true;

		case KEYS.DOWN:
			if (!document.querySelector('#NpcMenu')) {
				if (activeElement === messageBox) {
					if (shouldLetChatInputHandleVerticalArrows(messageBox, 'down')) {
						event.stopImmediatePropagation();
						return true;
					}
					messageBox.innerHTML = _historyMessage.next();
					break;
				}

				if (activeElement === nickBox) {
					nickBox.value = _historyNickName.next();
					nickBox.select();
					break;
				}
			}
			return true;

		// Update chat height
		case KEYS.F10:
			this.updateHeight(false);
			{
				const activeContent = root.querySelector(`.content[data-content="${this.activeTab}"]`);
				if (activeContent) activeContent.scrollTop = activeContent.scrollHeight;
			}
			break;

		// Send message
		case KEYS.ENTER: {
			if (document.activeElement.className === 'message input-chatbox' && document.activeElement !== messageBox) {
				return true;
			}

			if (document.querySelector('#NpcMenu, #NpcBox')) {
				return true;
			}

			if (activeElement === messageBox) {
				this.submit();
				event.stopImmediatePropagation();
				return false;
			}

			const input = root.querySelector('.input');
			if (input && input.style.display === 'none') {
				input.style.display = 'block';
				const bmEl = root.querySelector('.battlemode');
				if (bmEl) bmEl.style.display = 'none';
			}

			messageBox.focus();
			const range = document.createRange();
			const sel = window.getSelection();
			range.selectNodeContents(messageBox);
			range.collapse(false);
			sel.removeAllRanges();
			sel.addRange(range);
			event.stopImmediatePropagation();
			return false;
		}
	}

	event.stopImmediatePropagation();
	return false;
};

ChatBox.toggleChat = function toggleChat() {
	const root = _root();
	const messageBox = root.querySelector('.input-chatbox');

	const activeElement = KEYS.getDeepActiveElement();
	if (activeElement.tagName === 'INPUT' && activeElement !== messageBox) {
		return true;
	}

	if (document.querySelector('#NpcMenu, #NpcBox')) {
		return true;
	}

	messageBox.focus();
	this.submit();
};

/**
 * Process ChatBox message
 */
ChatBox.submit = function Submit() {
	const root = _root();
	const inputEl = root.querySelector('.input');
	const $user = root.querySelector('.input .username');
	const $text = root.querySelector('.input-chatbox');

	const user = $user ? $user.value : '';
	const text = extractChatMessage($text);
	const trimmedText = text.replace(/\u00A0/g, ' ').trim();
	let isChatOn = false;

	// Battle mode
	if (!trimmedText.length) {
		const bmEl = root.querySelector('.battlemode');
		if (inputEl) inputEl.style.display = inputEl.style.display === 'none' ? 'block' : 'none';
		if (bmEl) bmEl.style.display = bmEl.style.display === 'none' ? 'block' : 'none';

		if (inputEl && inputEl.style.display !== 'none') {
			isChatOn = true;
			$text.focus();
		}
		const chatmode = isChatOn ? 'on' : 'off';
		Client.loadFile(`${DB.INTERFACE_PATH}basic_interface/chatmode_${chatmode}.bmp`, data => {
			const chatmodeBtn = root.querySelector('.chat-function .chatmode');
			if (chatmodeBtn) chatmodeBtn.style.backgroundImage = `url(${data})`;
		});

		return;
	}

	// Private message
	if (user.length && trimmedText[0] !== '/') {
		this.PrivateMessageStorage.nick = user;
		this.PrivateMessageStorage.msg = trimmedText;
		_historyNickName.push(user);
		_historyNickName.previous();
	}

	// Save in history
	_historyMessage.push(trimmedText);

	$text.innerHTML = '';

	// Command
	if (trimmedText[0] === '/') {
		Commands.processCommand.call(this, trimmedText.substr(1));
		return;
	}

	this.onRequestTalk(user, trimmedText, ChatBox.sendTo);
};

/**
 * Extract plain chat text from the contenteditable input while preserving item links.
 */
function extractChatMessage(inputEl) {
	if (!inputEl) return '';
	const clone = inputEl.cloneNode(true);

	clone.querySelectorAll('span.item-link').forEach(el => {
		const itemData = el.getAttribute('data-item') || el.dataset.item || '';
		el.replaceWith(document.createTextNode(itemData));
	});

	let result = clone.textContent;
	result = result.replace(/\u00A0/g, ' ');
	return result;
}

/**
 * Add text to chatbox
 */
ChatBox.addText = function addText(text, colorType, filterType, color, override) {
	text = text.replace(/<ITEMLINK>.*?<\/ITEMLINK>|<ITEML>.*?<\/ITEML>|<ITEM>.*?<\/ITEM>/gi, function (match) {
		const item = DB.parseItemLink(match);
		const span = `<span data-item="${match}" class="item-link" style="color:#FFFF63;">&lt;${item.name}&gt;</span>`;
		override = true;
		return span;
	});

	// Auto-detect client-generated HTML (nickname links in whispers)
	if (!override && /<span\s+class="nickname-link"/.test(text)) {
		override = true;
	}

	if (isNaN(filterType)) {
		filterType = ChatBox.FILTER.PUBLIC_LOG;
	}

	_messageBuffer.push({
		text: text,
		colorType: colorType,
		filterType: filterType,
		color: color,
		override: override
	});

	if (!_rafScheduled) {
		_rafScheduled = true;
		requestAnimationFrame(() => {
			_rafScheduled = false;
			flushMessageBuffer();
		});
	}
};

/**
 * Process all messages in the buffer at once
 */
function flushMessageBuffer() {
	if (_messageBuffer.length === 0) {
		return;
	}

	const root = _root();
	const messages = _messageBuffer.slice();
	_messageBuffer = [];

	const messagesByTab = {};

	messages.forEach(msg => {
		ChatBox.tabs.forEach((tab, TabNum) => {
			const chatTabOption = ChatBoxSettings.tabOption[TabNum];

			if (!chatTabOption || !chatTabOption.includes(msg.filterType)) {
				return;
			}

			if (!messagesByTab[TabNum]) {
				messagesByTab[TabNum] = [];
			}

			messagesByTab[TabNum].push(msg);
		});
	});

	Object.keys(messagesByTab).forEach(TabNum => {
		const content = root.querySelector(`.content[data-content="${TabNum}"]`);
		if (!content) return;

		const fragment = document.createDocumentFragment();
		const wasAtBottom = shouldScrollDownBeforeAdd(content, content.offsetHeight);

		messagesByTab[TabNum].forEach(msg => {
			const color = msg.color || getColorForType(msg.colorType);
			const div = document.createElement('div');
			div.style.color = color;
			if (!msg.override) {
				div.textContent = msg.text;
			} else {
				div.innerHTML = msg.text;
			}
			fragment.appendChild(div);
		});

		content.appendChild(fragment);

		while (content.childElementCount > MAX_MSG) {
			const element = content.firstElementChild;
			const matches = element.innerHTML.match(/(blob:[^"]+)/g);
			if (matches) {
				for (let i = 0; i < matches.length; i++) {
					window.URL.revokeObjectURL(matches[i]);
				}
			}
			element.remove();
		}

		if (wasAtBottom) {
			content.scrollTop = content.scrollHeight;
		}
	});
}

function getColorForType(colorType) {
	if (colorType & ChatBox.TYPE.PUBLIC && colorType & ChatBox.TYPE.SELF) {
		return '#00FF00';
	} else if (colorType & ChatBox.TYPE.PARTY) {
		return colorType & ChatBox.TYPE.SELF ? 'rgb(200, 200, 100)' : 'rgb(230,215,200)';
	} else if (colorType & ChatBox.TYPE.GUILD) {
		return 'rgb(180, 255, 180)';
	} else if (colorType & ChatBox.TYPE.PRIVATE) {
		return '#FFFF00';
	} else if (colorType & ChatBox.TYPE.ERROR) {
		return '#FF0000';
	} else if (colorType & ChatBox.TYPE.INFO) {
		return '#FFFF63';
	} else if (colorType & ChatBox.TYPE.BLUE) {
		return '#00FFFF';
	} else if (colorType & ChatBox.TYPE.ADMIN) {
		return '#FFFF00';
	} else if (colorType & ChatBox.TYPE.MAIL) {
		return '#F4D293';
	}
	return 'white';
}

function shouldScrollDownBeforeAdd(container, height) {
	const tolerance = 5;
	const atBottom = container.scrollTop + height >= container.scrollHeight - tolerance;

	if (height >= container.scrollHeight || atBottom) {
		return true;
	}

	return false;
}

/**
 * Change chatbox's height
 */
ChatBox.updateHeight = function changeHeight(AlwaysVisible) {
	const root = _root();
	const HeightList = [0, 0, MAGIC_NUMBER, MAGIC_NUMBER * 2, MAGIC_NUMBER * 3, MAGIC_NUMBER * 4, MAGIC_NUMBER * 5];
	const content = root.querySelector('.contentwrapper');
	const bottomBefore = getChatBottomAnchorPx(root, this.__lastBottomY);

	_heightIndex = (_heightIndex + 1) % HeightList.length;

	if (_heightIndex === 0 && AlwaysVisible) {
		_heightIndex = 1;
	}

	if (content) content.style.height = `${HeightList[_heightIndex]}px`;

	const header = root.querySelector('.header');
	const body = root.querySelector('.body');
	const inputEl = root.querySelector('.input');

	switch (_heightIndex) {
		case 0:
			this.__lastBottomY = bottomBefore;
			this._host.style.display = 'none';
			break;

		case 1:
			this._host.style.display = 'block';
			if (header) header.style.display = 'none';
			if (body) body.style.display = 'none';
			if (inputEl) inputEl.classList.add('fix');
			break;

		default:
			if (inputEl) inputEl.classList.remove('fix');
			if (header) header.style.display = '';
			if (body) body.style.display = '';
			break;
	}

	if (_heightIndex !== 0 && isFinite(bottomBefore)) {
		const bottomAfter = getChatBottomAnchorPx(root, bottomBefore);
		if (isFinite(bottomAfter)) {
			let top = parseInt(this._host.style.top, 10);
			top = isFinite(top) ? top : 0;
			this._host.style.top = `${top + (bottomBefore - bottomAfter)}px`;
			this.__lastBottomY = bottomBefore;
		}
	}

	const active = root.querySelector(`.content[data-content="${this.activeTab}"]`);
	if (active) {
		active.scrollTop = active.scrollHeight;
	}
};

function getChatBottomAnchorPx(root, fallback) {
	const inputEl = root.querySelector('.input');
	if (inputEl && inputEl.style.display !== 'none') {
		const rect = inputEl.getBoundingClientRect();
		return rect.bottom;
	}

	const bmEl = root.querySelector('.battlemode');
	if (bmEl && bmEl.style.display !== 'none') {
		const rect = bmEl.getBoundingClientRect();
		return rect.bottom;
	}

	return fallback;
}

/**
 * Determine if vertical arrow should scroll the contenteditable rather than navigate history
 */
function shouldLetChatInputHandleVerticalArrows(inputEl, direction) {
	if (!inputEl) {
		return false;
	}

	const sel = window.getSelection();
	if (!sel || sel.rangeCount < 1) {
		return false;
	}

	const range = sel.getRangeAt(0);
	if (!range) {
		return false;
	}

	const anchorNode = sel.anchorNode || range.startContainer;
	if (!anchorNode) {
		return false;
	}

	if (anchorNode !== inputEl && !(inputEl.contains && inputEl.contains(anchorNode))) {
		return false;
	}

	if (!sel.isCollapsed) {
		return true;
	}

	const text = extractChatMessage(inputEl);
	const hasNewline = text.indexOf('\n') > -1;
	const hasOverflow = inputEl.scrollHeight > inputEl.clientHeight + 1;
	if (!hasNewline && !hasOverflow) {
		return false;
	}

	let caretRect;
	try {
		caretRect =
			range.getClientRects && range.getClientRects().length
				? range.getClientRects()[0]
				: range.getBoundingClientRect();
	} catch (_e) {
		return true;
	}

	if (!caretRect) {
		return true;
	}

	const inputRect = inputEl.getBoundingClientRect ? inputEl.getBoundingClientRect() : null;
	if (!inputRect) {
		return true;
	}

	if (direction === 'up') {
		return caretRect.top > inputRect.top + 2;
	}

	if (direction === 'down') {
		return caretRect.bottom < inputRect.bottom - 2;
	}

	return false;
}

/**
 * Save user name to nick name history
 */
ChatBox.saveNickName = function saveNickName(pseudo) {
	_historyNickName.push(pseudo);
};

/**
 * Save chat from current tab into a file.
 */
ChatBox.saveCurrentTabChat = function saveCurrentTabChat() {
	const root = _root();
	let data;

	const tzoffset = new Date().getTimezoneOffset() * 60000;
	let localISOTime = new Date(Date.now() - tzoffset).toISOString().slice(0, -1);
	localISOTime = localISOTime.replace('T', ' ');
	const timezone = new Date().getTimezoneOffset() / 60;
	const date = `${localISOTime} (GMT ${timezone > 0 ? '-' : '+'}${Math.abs(timezone).toString()})`;

	const contentEl = root.querySelector(`.content[data-content="${ChatBox.activeTab}"]`);

	data =
		'<html><head><title>Chat History</title><style> body { background-color: DarkSlateGray; } </style></head><body>';
	data += contentEl ? contentEl.outerHTML : '';
	data += '</body></html>';

	const url = window.URL.createObjectURL(new Blob([data], { type: 'text/plain' }));

	ChatBox.addText(
		`Chat History [${ChatBox.tabs[ChatBox.activeTab].name}] ${date} can be saved by <a style="color:#F88" download="ChatHistory [${ChatBox.tabs[ChatBox.activeTab].name}] (${date.replace('/', '-')}).html" href="${url}" target="_blank">clicking here</a>.`,
		ChatBox.TYPE.PUBLIC,
		ChatBox.FILTER.PUBLIC_LOG,
		null,
		true
	);
};

/**
 * Update scroll by block (14px)
 */
function onScroll(event) {
	let delta;

	if (event.wheelDelta) {
		delta = event.wheelDelta / 120;
		if (window.opera) {
			delta = -delta;
		}
	} else if (event.detail) {
		delta = -event.detail;
	} else if (event.deltaY) {
		delta = -event.deltaY / Math.abs(event.deltaY);
	}

	const lineHeight = getScrollLineHeightPx(this);
	this.scrollTop = Math.floor(this.scrollTop / lineHeight) * lineHeight - (delta || 0) * lineHeight;
	event.preventDefault();
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

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	event.stopImmediatePropagation();
	event.preventDefault();
}

/**
 * Change private message nick name
 */
function onPrivateMessageUserSelection(name) {
	return function onPrivateMessageUserSelectionClosure() {
		const root = _root();
		const nickBox = root.querySelector('.input .username');
		if (nickBox) nickBox.value = name;
	};
}

/**
 * Change target of global chat (party, guild)
 */
function onChangeTargetMessage(type) {
	return function onChangeTargetMessageClosure() {
		const root = _root();
		const $input = root.querySelector('.input-chatbox');

		if ($input) {
			$input.classList.remove('guild', 'party', 'clan');

			if (type & ChatBox.TYPE.PARTY) {
				$input.classList.add('party');
			} else if (type & ChatBox.TYPE.GUILD) {
				$input.classList.add('guild');
			} else if (type & ChatBox.TYPE.CLAN) {
				$input.classList.add('clan');
			}
		}

		ChatBox.sendTo = type;
	};
}

function setChatFontScale(scale) {
	return function setChatFontScaleClosure() {
		_preferences.fontScale = clampChatFontScale(scale);
		_preferences.save();
		ChatBox.applyFontScale();
	};
}

function clampChatFontScale(scale) {
	const allowed = [1.0, 1.2, 1.4];
	let best = allowed[0];
	let bestDist = Infinity;

	scale = parseFloat(scale);
	if (!isFinite(scale) || scale <= 0) {
		return 1.0;
	}

	for (let i = 0; i < allowed.length; ++i) {
		const dist = Math.abs(allowed[i] - scale);
		if (dist < bestDist) {
			bestDist = dist;
			best = allowed[i];
		}
	}

	return best;
}

function getScrollLineHeightPx(element) {
	let style, lh;

	try {
		style = window.getComputedStyle(element);
		lh = parseFloat(style.lineHeight);
		if (isFinite(lh) && lh > 0) {
			return Math.round(lh);
		}
	} catch (_e) {
		// Ignore
	}

	return 14;
}

ChatBox.applyFontScale = function applyFontScale() {
	const root = _root();
	const scale = clampChatFontScale(_preferences.fontScale || 1.0);
	const baseFont = 12;
	const baseLineHeight = 14;
	const baseInputLineHeight = 18;

	const fontSize = Math.max(10, Math.round(baseFont * scale));
	const lineHeight = Math.max(12, Math.round(baseLineHeight * scale));
	const inputLineHeight = Math.max(14, Math.round(baseInputLineHeight * scale));

	_preferences.fontScale = scale;

	// Chat log
	root.querySelectorAll('.content').forEach(el => {
		el.style.fontSize = `${fontSize}px`;
		el.style.lineHeight = `${lineHeight}px`;
	});

	// Chat input
	root.querySelectorAll('.input input, .input .message').forEach(el => {
		el.style.fontFamily = 'Arial';
		el.style.fontSize = `${fontSize}px`;
	});

	const message = root.querySelector('.input .message');
	if (message) {
		message.style.lineHeight = `${inputLineHeight}px`;
	}
};

function makeResizableDiv() {
	const root = _root();
	const resizer = root.querySelector('.event_add_cursor');
	if (!resizer) {
		return;
	}

	let originalHeight = 0;
	let originalAnchorY = 0;
	let originalMouseY = 0;

	const fixHeight = height => Math.floor(height / MAGIC_NUMBER) * MAGIC_NUMBER;

	const resize = e => {
		let height = fixHeight(originalHeight - (e.pageY - originalMouseY));
		height = Math.max(MAGIC_NUMBER, Math.min(MAGIC_NUMBER * 5, height));

		ChatBox._host.style.top = `${originalAnchorY - height}px`;
		const contentWrapper = root.querySelector('.contentwrapper');
		if (contentWrapper) contentWrapper.style.height = `${height}px`;
		_heightIndex = Math.max(2, Math.min(6, height / MAGIC_NUMBER + 1));

		const active = root.querySelector(`.content[data-content="${ChatBox.activeTab}"]`);
		if (active) {
			active.scrollTop = active.scrollHeight;
		}
	};

	const stopResize = () => {
		window.removeEventListener('mousemove', resize);
		window.removeEventListener('mouseup', stopResize);
	};

	resizer.addEventListener('mousedown', e => {
		e.preventDefault();
		const contentWrapper = root.querySelector('.contentwrapper');
		originalHeight = contentWrapper ? contentWrapper.offsetHeight : 0;
		originalAnchorY = (parseInt(ChatBox._host.style.top, 10) || 0) + originalHeight;
		originalMouseY = e.pageY;

		window.addEventListener('mousemove', resize);
		window.addEventListener('mouseup', stopResize);
	});
}

// CLICKABLE ITEM → OPEN ITEMINFO (handle inside shadow)
ChatBox._setupItemLinkHandler = function _setupItemLinkHandler() {
	const root = _root();
	if (!root) return;

	root.addEventListener('click', event => {
		const link = event.target.closest('.item-link');
		if (!link) return;

		// If the link is inside the chat input, keep focus there
		if (link.closest('.input-chatbox')) {
			event.stopImmediatePropagation();
			return;
		}

		const item = DB.parseItemLink(link.dataset.item || link.getAttribute('data-item'));
		if (!item) return;

		const ItemInfo = UIManager.getComponent('ItemInfo');
		ItemInfo.append();
		ItemInfo.setItem(item);
	});
};

// Also keep global handler for item links outside shadow (backwards compatibility)
document.addEventListener('click', event => {
	const link = event.target.closest('.item-link');
	if (!link) {
		return;
	}

	if (link.closest('#chatbox .input-chatbox')) {
		event.stopImmediatePropagation();
		event.preventDefault();
		return;
	}

	const item = DB.parseItemLink(link.dataset.item);
	if (!item) {
		return;
	}

	const ItemInfo = UIManager.getComponent('ItemInfo');
	ItemInfo.append();
	ItemInfo.setItem(item);
});

ChatBox.insertText = function (text) {
	const root = _root();
	const input = root.querySelector('.input-chatbox');
	if (!input) return;

	input.appendChild(document.createTextNode(text));
	input.focus();
};

// Override mouseMode to CROSS since chatbox body is click-through
ChatBox.mouseMode = GUIComponent.MouseMode.CROSS;

/**
 * Create component and export it
 */
export default UIManager.addComponent(ChatBox);
