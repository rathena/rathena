/**
 * UI/Components/ItemCompare/ItemCompare.js
 *
 * Item Comparison Information
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import ItemType from 'DB/Items/ItemType.js';
import Client from 'Core/Client.js';
import CardIllustration from 'UI/Components/CardIllustration/CardIllustration.js';
import UIManager from 'UI/UIManager.js';
import Mouse from 'Controls/MouseEventHandler.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import MakeReadBook from 'UI/Components/MakeReadBook/MakeReadBook.js';
import Renderer from 'Renderer/Renderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Sprite from 'Loaders/Sprite.js';
import Action from 'Loaders/Action.js';
import htmlText from './ItemCompare.html?raw';
import cssText from './ItemCompare.css?raw';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Entity from 'Renderer/Entity/Entity.js';

/**
 * @let {Sprite,Action} objects
 */
let _sprite, _action;

/**
 * @let {CanvasRenderingContext2D}
 */
let _ctx;

/**
 * @let {number} type
 */
const _type = 0;

/**
 * @let {number} start tick
 */
const _start = 0;

/**
 * Helper: escape HTML
 */
function _escapeHTML(text) {
	const div = document.createElement('div');
	div.textContent = text;
	return div.innerHTML;
}

/**
 * Create Component
 */
const ItemCompare = new GUIComponent('ItemCompare', cssText);

ItemCompare.render = () => htmlText;

/**
 * @let {number} ItemCompare unique id
 */
ItemCompare.uid = -1;

/**
 * Once append
 */
ItemCompare.onAppend = function onAppend() {
	const root = ItemCompare.getRoot();
	const descInner = root.querySelector('.description-inner');
	if (descInner) {
		resize(descInner.offsetHeight + 45);
	}

	// Position ItemCompare next to ItemInfo
	if (ItemInfo._host) {
		const itemInfoRect = ItemInfo._host.getBoundingClientRect();
		const itemInfoWidth = itemInfoRect.width;
		this._host.style.top = `${itemInfoRect.top ? itemInfoRect.top : 200}px`;
		this._host.style.left = `${itemInfoRect.left ? itemInfoRect.left - itemInfoWidth : 200}px`;
	}
};

/**
 * Once removed from html
 */
ItemCompare.onRemove = function onRemove() {
	this.uid = -1;
};

/**
 * Initialize UI
 */
ItemCompare.init = function init() {
	const root = ItemCompare.getRoot();

	this._host.style.top = '200px';
	this._host.style.left = '200px';

	const extendBtn = root.querySelector('.extend');
	if (extendBtn) {
		extendBtn.addEventListener('mousedown', onResize);
	}

	// Ask to see card.
	const viewBtn = root.querySelector('.view');
	if (viewBtn) {
		viewBtn.addEventListener('click', () => {
			CardIllustration.append();
			CardIllustration.setCard(this.item);
		});
	}

	// Use ItemInfo's title as drag handle to preserve linked-drag behavior
	const itemInfoRoot = ItemInfo.getRoot();
	const itemInfoTitle = itemInfoRoot ? itemInfoRoot.querySelector('.title') : null;
	this.draggable(itemInfoTitle || '.title');
};

/**
 * Bind component
 *
 * @param {object} item
 */
ItemCompare.setItem = function setItem(item) {
	const it = DB.getItemInfo(item.ITID);
	const root = ItemCompare.getRoot();
	const cardList = root.querySelector('.cardlist .border');
	const optionContainer = root.querySelector('.option-container');

	this.item = it;
	Client.loadFile(
		DB.INTERFACE_PATH +
			'collection/' +
			(item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
			'.bmp',
		data => {
			const collection = root.querySelector('.collection');
			if (collection) {
				collection.style.backgroundImage = `url(${data})`;
			}
		}
	);

	const itemName = DB.getItemName(item, { showItemOptions: false });

	// Damaged status
	const title = root.querySelector('.title');
	if (title) {
		if (item.IsDamaged) {
			title.classList.add('damaged');
		} else {
			title.classList.remove('damaged');
		}
		title.textContent = itemName;
	}

	if (item.Options && item.IsIdentified) {
		//Clear all option list
		if (optionContainer) {
			optionContainer.innerHTML = '';
		}

		//Loop to Show Options
		for (let i = 1; i <= 5; i++) {
			if (item.Options[i].index > 0) {
				const randomOptionName = DB.getOptionName(item.Options[i].index);
				const optionList =
					'<div class="optionlist">' +
					'<div class="border">' +
					randomOptionName.replace('%d', item.Options[i].value).replace('%%', '%') +
					'</div>' +
					'</div>';
				if (optionContainer) {
					optionContainer.insertAdjacentHTML('beforeend', optionList);
				}
			}
		}
		if (optionContainer) {
			optionContainer.style.display = 'block';
		}
	} else {
		if (optionContainer) {
			optionContainer.style.display = 'none';
		}
	}

	const descInner = root.querySelector('.description-inner');
	if (descInner) {
		descInner.textContent = item.IsIdentified ? it.identifiedDescriptionName : it.unidentifiedDescriptionName;
	}

	// Add view button (for cards)
	addEvent(item);

	let hideslots = false;
	if (item.slot) {
		switch (item.slot['card1']) {
			case 0x00ff: // FORGE
			case 0x00fe: // CREATE
			case 0xff00: // PET
				hideslots = true;
				break;
		}
		switch (item.slot['card4']) {
			case 0x1: //BELOVED PET
				hideslots = true;
				break;
		}
	}

	const cardListParent = cardList ? cardList.parentElement : null;

	switch (item.type) {
		// Not an equipement = no card
		default:
			if (cardListParent) {
				cardListParent.style.display = 'none';
			}
			break;

		case ItemType.WEAPON:
		case ItemType.ARMOR:
		case ItemType.SHADOWGEAR: {
			if (hideslots || (item.type === ItemType.ARMOR && item.location === 0)) {
				if (cardListParent) {
					cardListParent.style.display = 'none';
				}
				break;
			}
			const slotCount = it.slotCount || 0;

			if (cardListParent) {
				cardListParent.style.display = 'block';
			}
			if (cardList) {
				cardList.innerHTML = '';
			}

			for (let i = 0; i < 4; ++i) {
				addCard(cardList, (item.slot && item.slot['card' + (i + 1)]) || 0, i, slotCount);
			}
			if (!item.IsIdentified && cardListParent) {
				cardListParent.style.display = 'none';
			}
			break;
		}

		case ItemType.PETEGG:
			if (cardListParent) {
				cardListParent.style.display = 'none';
			}
			break;
	}

	if (descInner) {
		resize(descInner.offsetHeight + 45);
	}
};

/**
 * Add a card into a slot
 *
 * @param {Element} cardList DOM element
 * @param {number} item id
 * @param {number} index
 * @param {number} slot count
 */
function addCard(cardList, itemId, index, slotCount) {
	let file,
		name = '';
	const card = DB.getItemInfo(itemId);

	if (itemId && card) {
		file = 'item/' + card.identifiedResourceName + '.bmp';
		name = `<div class="name">${_escapeHTML(card.identifiedDisplayName)}</div>`;
	} else if (index < slotCount) {
		file = 'empty_card_slot.bmp';
	} else {
		file = 'basic_interface/coparison_disable_card_slot.bmp';
	}

	if (!cardList) {
		return;
	}

	cardList.insertAdjacentHTML(
		'beforeend',
		`<div class="item" data-index="${index}"><div class="icon"></div>${name}</div>`
	);

	Client.loadFile(DB.INTERFACE_PATH + file, data => {
		const root = ItemCompare.getRoot();
		const element = root.querySelector(`.cardlist .item[data-index="${index}"] .icon`);
		if (element) {
			element.style.backgroundImage = `url(${data})`;

			if (itemId && card) {
				element.addEventListener('contextmenu', e => {
					e.preventDefault();
					e.stopImmediatePropagation();
					ItemCompare.setItem({
						ITID: itemId,
						IsIdentified: true,
						type: 6
					});
				});
			}
		}
	});
}

/**
 * Extend ItemCompare window size
 */
function onResize() {
	const top = ItemCompare._host.offsetTop;
	let lastHeight = 0;

	function resizing() {
		const h = Math.floor(Mouse.screen.y - top);
		if (h === lastHeight) {
			return;
		}
		resize(h);
		lastHeight = h;
	}

	// Start resizing
	const _Interval = setInterval(resizing, 30);

	// Stop resizing on left click
	const onMouseUp = event => {
		if (event.which === 1) {
			clearInterval(_Interval);
			window.removeEventListener('mouseup', onMouseUp);
		}
	};
	window.addEventListener('mouseup', onMouseUp);
}

/**
 * Extend ItemCompare window size
 *
 * @param {number} height
 */
function resize(height) {
	const root = ItemCompare.getRoot();
	const container = root.querySelector('.container');
	const description = root.querySelector('.description');
	const descriptionInner = root.querySelector('.description-inner');
	let containerHeight = height;
	const minHeight = 120;
	const innerH = descriptionInner ? descriptionInner.offsetHeight : 0;
	const maxHeight = innerH + 45 > 120 ? Math.min(innerH + 45, 448) : 120;

	if (containerHeight <= minHeight) {
		containerHeight = minHeight;
	}

	if (containerHeight >= maxHeight) {
		containerHeight = maxHeight;
	}

	if (container) {
		container.style.height = `${containerHeight}px`;
	}
	if (description) {
		description.style.height = `${containerHeight - 45}px`;
	}
}

function addEvent(item) {
	const root = ItemCompare.getRoot();
	let event = root.querySelector('.event_view');
	if (!event) {
		if (!validateFieldsExist(event)) {
			event = root.querySelector('.event_view');
		}
	}
	if (!event) {
		return;
	}

	const viewBtn = event.querySelector('.view');
	if (viewBtn) {
		viewBtn.style.display = 'none';
	}
	const canvases = event.querySelectorAll('canvas');
	canvases.forEach(c => c.remove());

	Renderer.stop(rendering);

	switch (item.type) {
		case ItemType.CARD:
			if (viewBtn) {
				viewBtn.style.display = 'block';
			}
			break;
		case ItemType.ETC: {
			const filenameBook = `data/book/${item.ITID}.txt`;
			Client.loadFile(filenameBook, data => {
				try {
					MakeReadBook.startBook(data, item);
				} catch (e) {
					console.error('ItemCompare::addEvent() - startBook failed:', e.message);
				}
				eventsBooks();
			});
			break;
		}
		default:
			if (viewBtn) {
				viewBtn.style.display = 'none';
			}
			break;
	}
}

function eventsBooks() {
	const root = ItemCompare.getRoot();
	const event = root.querySelector('.event_view');
	if (!event) {
		return;
	}

	Client.getFiles(
		['data/sprite/book/\xc3\xa5\xc0\xd0\xb1\xe2.spr', 'data/sprite/book/\xc3\xa5\xc0\xd0\xb1\xe2.act'],
		function (spr, act) {
			try {
				_sprite = new Sprite(spr);
				_action = new Action(act);
			} catch (e) {
				console.error('Book::init() - ' + e.message);
				return;
			}
			const canvas = _sprite.getCanvasFromFrame(0);
			canvas.className = 'book_open event_add_cursor';
			event.appendChild(canvas);

			const bookOpen = root.querySelector('.book_open');
			if (bookOpen) {
				bookOpen.addEventListener('mouseover', e => {
					e.stopImmediatePropagation();
					const overlayOpen = root.querySelector('.overlay_open');
					if (overlayOpen) {
						overlayOpen.style.display = 'block';
					}
				});
				bookOpen.addEventListener('mouseout', e => {
					e.stopImmediatePropagation();
					const overlayOpen = root.querySelector('.overlay_open');
					if (overlayOpen) {
						overlayOpen.style.display = 'none';
					}
				});
				bookOpen.addEventListener('click', e => {
					e.stopImmediatePropagation();
					MakeReadBook.openBook();
				});
			}

			// icon read book
			const readCanvas = document.createElement('canvas');
			readCanvas.width = 21;
			readCanvas.height = 15;
			readCanvas.className = 'book_read event_add_cursor';
			event.appendChild(readCanvas);
			_ctx = readCanvas.getContext('2d');

			const bookRead = root.querySelector('.book_read');
			if (bookRead) {
				bookRead.addEventListener('mouseover', e => {
					e.stopImmediatePropagation();
					const overlayRead = root.querySelector('.overlay_read');
					if (overlayRead) {
						overlayRead.style.display = 'block';
					}
				});
				bookRead.addEventListener('mouseout', e => {
					e.stopImmediatePropagation();
					const overlayRead = root.querySelector('.overlay_read');
					if (overlayRead) {
						overlayRead.style.display = 'none';
					}
				});
				bookRead.addEventListener('click', e => {
					e.stopImmediatePropagation();
					MakeReadBook.highlighter();
				});
			}
			Renderer.render(rendering);
		}
	);
}

/**
 * Rendering animation
 */
const rendering = (function renderingClosure() {
	const position = new Uint16Array([0, 0]);

	return function render() {
		const _entity = new Entity();
		const action = _action.actions[_type];
		const anim = Math.floor((Renderer.tick - _start) / action.delay);

		const animation = action.animations[anim % action.animations.length];

		let i, count;
		count = animation.layers.length;

		// Initialize context
		SpriteRenderer.bind2DContext(_ctx, 10, 25);
		_ctx.clearRect(0, 0, _ctx.canvas.width, _ctx.canvas.height);

		// Render layers
		for (i = 0, count = animation.layers.length; i < count; ++i) {
			_entity.renderLayer(animation.layers[i], _sprite, _sprite, 1.0, position, false);
		}
	};
})();

function validateFieldsExist(event) {
	const root = ItemCompare.getRoot();

	if (!event) {
		const validExitElement =
			'<div class="event_view">' +
			'<button class="view" data-background="btn_view.bmp" data-down="btn_view_a.bmp" data-hover="btn_view_b.bmp"></button>' +
			'<span class="overlay_open">' +
			DB.getMessage(1294) +
			'</span>' +
			'<span class="overlay_read">' +
			DB.getMessage(1295) +
			'</span>' +
			'</div>';
		const collection = root.querySelector('.collection');
		if (collection) {
			collection.insertAdjacentHTML('afterend', validExitElement);
		}
		return false;
	}

	if (!root.querySelector('.overlay_open') && !root.querySelector('.overlay_read')) {
		event.insertAdjacentHTML(
			'beforeend',
			'<span class="overlay_open">' +
				DB.getMessage(1294) +
				'</span>' +
				'<span class="overlay_read">' +
				DB.getMessage(1295) +
				'</span>'
		);
	}

	if (!event.querySelector('button')) {
		event.insertAdjacentHTML(
			'beforeend',
			'<button class="view" data-background="btn_view.bmp" data-down="btn_view_a.bmp" data-hover="btn_view_b.bmp"></button>'
		);
	}

	return true;
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(ItemCompare);
