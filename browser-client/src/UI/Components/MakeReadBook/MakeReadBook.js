/**
 * UI/Components/MakeReadBook/MakeReadBook.js
 *
 * Chararacter MakeReadBook
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * In some cases the client will send packet twice.eg NORMAL_ITEMLIST4; fixit [skybook888]
 *
 */

import DB from 'DB/DBManager.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './MakeReadBook.html?raw';
import cssText from './MakeReadBook.css?raw';
import Sprite from 'Loaders/Sprite.js';
import Client from 'Core/Client.js';
import TextEncoding from 'Utils/CodepageManager.js';
import Announce from 'UI/Components/Announce/Announce.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';

const sleepNow = delay => new Promise(resolve => setTimeout(resolve, delay));

/**
 * Create Component
 */
const MakeReadBook = new GUIComponent('MakeReadBook', cssText);

MakeReadBook.render = () => htmlText;

/**
 * @var {Preferences} structure
 */
const _BOOK_INFORMATION = Preferences.get(
	'_BOOK_INFORMATION',
	{
		itid: 0,
		title: '',
		color: '',
		pagesize: 0,
		contents: [],
		bookmark_activated: false,
		bookmark_activated_page: 0,
		book_open: false
	},
	1.0
);

/**
 * Store MakeReadBook items
 */
MakeReadBook.list = [];

/**
 * @var {number} used to remember the window height
 */
const _realSize = 0;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'MakeReadBook',
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
		magnet_right: false
	},
	1.0
);

MakeReadBook.startBook = function startBook(inforBook, item) {
	const it = DB.getItemInfo(item.ITID);

	// Convert Uint8Array to raw-byte string (Client.loadFile returns Uint8Array for .txt).
	// Keep as raw bytes so TextEncoding.decodeString() in page() can decode the codepage.
	const bookText =
		inforBook instanceof Uint8Array ? Array.from(inforBook, b => String.fromCharCode(b)).join('') : inforBook;

	_BOOK_INFORMATION['title'] = it.identifiedDisplayName;
	const addColor = bookText.substr(1, 7);
	const validtext = bookText.substr(7);

	const lineValidtext = validtext.split('\n');
	const defoutValue = 15;
	let coutIndeNewText = 0;
	let coutNewIndex = 0;
	const contentsArray = [];

	for (let index = 0; index < lineValidtext.length; index++) {
		if (defoutValue + coutNewIndex > index) {
			const addText =
				typeof contentsArray[coutIndeNewText] === 'undefined'
					? '\n'
					: contentsArray[coutIndeNewText] + '\n' + lineValidtext[index];

			if (addText.length > 460) {
				contentsArray[coutIndeNewText + 1] =
					typeof contentsArray[coutIndeNewText + 1] === 'undefined' ? '\n' + lineValidtext[index] : '\n';

				coutNewIndex = defoutValue + coutNewIndex;
				coutIndeNewText++;
				continue;
			}

			contentsArray[coutIndeNewText] = addText;
			continue;
		}

		coutNewIndex = defoutValue + coutNewIndex;
		coutIndeNewText++;
	}

	const validNewBookOpen = _BOOK_INFORMATION['itid'] === item.ITID;
	_BOOK_INFORMATION['color'] = addColor;
	_BOOK_INFORMATION['contents'] = contentsArray;
	_BOOK_INFORMATION['pagesize'] = contentsArray.length;
	_BOOK_INFORMATION['page'] = validNewBookOpen ? _BOOK_INFORMATION['page'] : 0;
	_BOOK_INFORMATION['bookmark_activated'] = validNewBookOpen;
	_BOOK_INFORMATION['itid'] = item.ITID;
	_BOOK_INFORMATION['book_open'] = false;
	_BOOK_INFORMATION.save();
};

MakeReadBook.openBook = function openBook() {
	MakeReadBook.append();

	const root = MakeReadBook.getRoot();

	const panel = root.querySelector('.panel');
	if (panel) {
		panel.style.backgroundColor = `#${_BOOK_INFORMATION['color']}`;
	}
	const footer = root.querySelector('.footer');
	if (footer) {
		footer.style.backgroundColor = `#${_BOOK_INFORMATION['color']}`;
	}

	const titleBook = root.querySelector('#titleBook');
	if (titleBook) {
		titleBook.textContent = _BOOK_INFORMATION['title'];
	}

	Client.getFiles(
		[
			'data/sprite/book/\xc3\xa5\xb4\xdd\xb1\xe2.spr',
			'data/sprite/book/\xc3\xa5\xb0\xa5\xc7\xc7.spr',
			'data/sprite/book/\xc3\xa5\xbf\xde\xc2\xca.spr',
			'data/sprite/book/\xc3\xa5\xbf\xc0\xb8\xa5\xc2\xca.spr'
		],
		(spr_close, spr_highlighter, spr_previous, spr_next) => {
			const innerRoot = MakeReadBook.getRoot();

			// close
			const sprite_close = new Sprite(spr_close);
			const canvas = sprite_close.getCanvasFromFrame(0);
			canvas.className = 'clone_book event_add_cursor';
			const footerEl = innerRoot.querySelector('.footer');
			footerEl.querySelectorAll('canvas').forEach(c => c.remove());
			footerEl.appendChild(canvas);
			canvas.addEventListener('click', onClose);

			// highlighter
			const sprite_highlighter = new Sprite(spr_highlighter);
			const canvas2 = sprite_highlighter.getCanvasFromFrame(0);
			canvas2.className = 'highlighter event_add_cursor';
			const highlighterEl = innerRoot.querySelector('#highlighter');
			highlighterEl.querySelectorAll('canvas').forEach(c => c.remove());
			highlighterEl.appendChild(canvas2);
			canvas2.addEventListener('mouseover', e => {
				e.stopImmediatePropagation();
				const bookmark = innerRoot.querySelector('.bookmark');
				if (bookmark) {
					bookmark.style.display = 'block';
				}
			});
			canvas2.addEventListener('mouseout', e => {
				e.stopImmediatePropagation();
				const bookmark = innerRoot.querySelector('.bookmark');
				if (bookmark) {
					bookmark.style.display = 'none';
				}
			});
			canvas2.addEventListener('click', e => {
				e.stopImmediatePropagation();
				_BOOK_INFORMATION['bookmark_activated'] = true;
				_BOOK_INFORMATION['bookmark_activated_page'] = _BOOK_INFORMATION['page'];
				_BOOK_INFORMATION.save();
			});

			// remove canvas next and previous
			const nextPrevEl = innerRoot.querySelector('#next_previous');
			nextPrevEl.querySelectorAll('canvas').forEach(c => c.remove());

			// previous
			const sprite_previous = new Sprite(spr_previous);
			const canvas3 = sprite_previous.getCanvasFromFrame(0);
			canvas3.className = 'previous_btn event_add_cursor';
			nextPrevEl.appendChild(canvas3);
			canvas3.addEventListener('mouseover', e => {
				e.stopImmediatePropagation();
				const prev = innerRoot.querySelector('.previous');
				if (prev) {
					prev.style.display = 'block';
				}
			});
			canvas3.addEventListener('mouseout', e => {
				e.stopImmediatePropagation();
				const prev = innerRoot.querySelector('.previous');
				if (prev) {
					prev.style.display = 'none';
				}
			});

			// next
			const sprite_next = new Sprite(spr_next);
			const canvas4 = sprite_next.getCanvasFromFrame(0);
			canvas4.className = 'next_btn event_add_cursor';
			nextPrevEl.appendChild(canvas4);
			canvas4.addEventListener('mouseover', e => {
				e.stopImmediatePropagation();
				const next = innerRoot.querySelector('.next');
				if (next) {
					next.style.display = 'block';
				}
			});
			canvas4.addEventListener('mouseout', e => {
				e.stopImmediatePropagation();
				const next = innerRoot.querySelector('.next');
				if (next) {
					next.style.display = 'none';
				}
			});

			// pagination
			canvas4.addEventListener('click', e => {
				e.stopImmediatePropagation();
				if (_BOOK_INFORMATION['page'] < _BOOK_INFORMATION['pagesize'] / 1 - 1) {
					_BOOK_INFORMATION['page']++;
					page();
					adjustButtons();
					_BOOK_INFORMATION.save();
				}
			});
			canvas3.addEventListener('click', e => {
				e.stopImmediatePropagation();
				if (_BOOK_INFORMATION['page'] > 0) {
					_BOOK_INFORMATION['page']--;
					page();
					adjustButtons();
					_BOOK_INFORMATION.save();
				}
			});
			page();
			adjustButtons();
		}
	);
};

MakeReadBook.highlighter = async function highlighter() {
	if (_preferences.show) {
		onClose();
	}

	let index = _BOOK_INFORMATION['bookmark_activated'] ? _BOOK_INFORMATION['bookmark_activated_page'] : 0;
	let newText = '';
	for (index; index < _BOOK_INFORMATION['contents'].length; index++) {
		newText = newText + '\n' + _BOOK_INFORMATION['contents'][index];
	}
	await repeatedGreetingsLoop(newText);
};

async function repeatedGreetingsLoop(book_information) {
	const text1 = book_information.split('\n');
	for (let i = 0; i < text1.length; i++) {
		if (_BOOK_INFORMATION['book_open']) {
			break;
		}

		if (text1[i] === '' && i === 0) {
			getText('   ');
			continue;
		}

		if (i === 1) {
			getText(text1[i]);
			continue;
		}
		await sleepNow(5000);

		if (_BOOK_INFORMATION['book_open']) {
			break;
		}

		getText(text1[i]);
	}
}

function getText(textbook) {
	let text = cleanTextColor(textbook);
	text = TextEncoding.decodeString(text);
	ChatBox.addText(text == '' ? '  ' : text, ChatBox.TYPE.ANNOUNCE, ChatBox.FILTER.PUBLIC_LOG, '#ffffff');
	Announce.append();
	Announce.set(text == '' ? '  ' : text, '#ffffff');
}

function cleanTextColor(text) {
	const cout = text.split('^').length;
	const array = text.split('^');
	let newMessage = '';

	for (let index = 0; index < cout; index++) {
		if (index === 0) {
			newMessage = array[index];
			continue;
		}

		newMessage = newMessage + array[index].substr(6);
	}

	if (newMessage.length === 1) {
		return newMessage[0];
	}

	return newMessage;
}

/**
 * Apply preferences once append to body
 */
MakeReadBook.onAppend = function OnAppend() {
	this._host.style.display = '';

	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - 455)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - 555)}px`;

	_BOOK_INFORMATION['book_open'] = true;
	_BOOK_INFORMATION.save();

	_preferences.show = true;
	_preferences.save();

	const root = MakeReadBook.getRoot();
	this.draggable(root.querySelector('.titlebar'));
};

/**
 * Remove MakeReadBook from window (and so clean up items)
 */
MakeReadBook.onRemove = function OnRemove() {
	try {
		if (_preferences.show) {
			this._host.style.display = 'none';
		}
		_preferences.show = this._host.style.display !== 'none';
		_preferences.reduce = !!_realSize;
		_preferences.y = parseInt(this._host.style.top, 10) || 0;
		_preferences.x = parseInt(this._host.style.left, 10) || 0;
		_preferences.magnet_top = this.magnet.TOP;
		_preferences.magnet_bottom = this.magnet.BOTTOM;
		_preferences.magnet_left = this.magnet.LEFT;
		_preferences.magnet_right = this.magnet.RIGHT;
		_preferences.save();
	} catch (_error) {
		_preferences.show = false;
		_preferences.save();
	}
};

/**
 * Extend MakeReadBook window size
 *
 * @param {number} width
 * @param {number} height
 */
MakeReadBook.resize = function Resize(width, height) {
	width = Math.min(Math.max(width, 6), 9);
	height = Math.min(Math.max(height, 2), 6);

	this._host.style.width = `${23 + 16 + 16 + width * 32}px`;
	this._host.style.height = `${31 + 19 + height * 32}px`;
};

/**
 * Exit window
 */
function onClose(e) {
	try {
		if (e) {
			e.stopImmediatePropagation();
		}
	} catch (_error) {
		// Ignore if event propagation has already stopped
	}

	_BOOK_INFORMATION['book_open'] = false;
	_BOOK_INFORMATION.save();
	if (_preferences.show) {
		MakeReadBook.onRemove();
	}
}

function page() {
	const root = MakeReadBook.getRoot();
	const textBook = root.querySelector('#textBook');
	if (textBook) {
		textBook.innerHTML = '';
	}

	for (
		let i = _BOOK_INFORMATION['page'] * 1;
		i < _BOOK_INFORMATION['pagesize'] && i < (_BOOK_INFORMATION['page'] + 1) * 1;
		i++
	) {
		if (textBook) {
			const decoded = TextEncoding.decodeString(_BOOK_INFORMATION['contents'][i]);
			textBook.innerHTML = DB.formatMsgToHtml(decoded);
		}
	}

	const pageBook = root.querySelector('#pageBook');
	if (pageBook) {
		pageBook.textContent = `(${_BOOK_INFORMATION['page'] + 1}/${Math.ceil(_BOOK_INFORMATION['pagesize'] / 1)})`;
	}
}

function adjustButtons() {
	const root = MakeReadBook.getRoot();
	const nextBtn = root.querySelector('.next_btn');
	if (nextBtn) {
		nextBtn.disabled =
			_BOOK_INFORMATION['pagesize'] <= 1 || _BOOK_INFORMATION['page'] > _BOOK_INFORMATION['pagesize'] / 1 - 1;
	}
	const prevBtn = root.querySelector('.previous_btn');
	if (prevBtn) {
		prevBtn.disabled = _BOOK_INFORMATION['pagesize'] <= 1 || _BOOK_INFORMATION['page'] == 0;
	}
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(MakeReadBook);
