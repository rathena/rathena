/**
 * UI/Components/ItemInfo/ItemInfo.js
 *
 * Item Information
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import ItemType from 'DB/Items/ItemType.js';
import EquipLocation from 'DB/Items/EquipmentLocation.js';
import Client from 'Core/Client.js';
import KEYS from 'Controls/KeyEventHandler.js';
import CardIllustration from 'UI/Components/CardIllustration/CardIllustration.js';
import UIManager from 'UI/UIManager.js';
import Mouse from 'Controls/MouseEventHandler.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import Cursor from 'UI/CursorManager.js';
import ItemCompare from 'UI/Components/ItemCompare/ItemCompare.js';
import ItemPreview from 'UI/Components/ItemPreview/ItemPreview.js';
import MakeReadBook from 'UI/Components/MakeReadBook/MakeReadBook.js';
import Renderer from 'Renderer/Renderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Sprite from 'Loaders/Sprite.js';
import Action from 'Loaders/Action.js';
import htmlText from './ItemInfo.html?raw';
import cssText from './ItemInfo.css?raw';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Entity from 'Renderer/Entity/Entity.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';

/**
 * Create Component
 */
const ItemInfo = new GUIComponent('ItemInfo', cssText);

ItemInfo.render = () => htmlText;

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
 * @let {number} ItemInfo unique id
 */
ItemInfo.uid = -1;

/**
 * ItemMoveInfo messages mapping
 */
const MOVE_INFO_MESSAGES = [
	{ key: 'Drop', msgId: 2788 },
	{ key: 'Storage', msgId: 2789 },
	{ key: 'Cart', msgId: 2790 },
	{ key: 'Mail', msgId: 2791 },
	{ key: 'Exchange', msgId: 2792 },
	// 2793 Auction → intentionally skipped
	{ key: 'GuildStorage', msgId: 2794 },
	{ key: 'NPCSale', msgId: 2795 }
];

/**
 * Helper: escape HTML keeping whitelisted tags
 */
function _escapeHTML(text) {
	const div = document.createElement('div');
	div.textContent = text;
	return div.innerHTML;
}

/**
 * Once append to the DOM
 */
ItemInfo.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		// Cleanup moveInfo tooltip & cursor
		ItemInfo.hideMoveInfoTooltip();

		ItemInfo.remove();
		if (ItemCompare.ui) {
			ItemCompare.remove();
		}
		if (ItemPreview.ui) {
			ItemPreview.remove();
		}
	}
};

/**
 * Once append
 */
ItemInfo.onAppend = function onAppend() {
	const root = ItemInfo.getRoot();
	const descInner = root.querySelector('.description-inner');
	if (descInner) {
		resize(descInner.offsetHeight + 45);
	}
};

/**
 * Once removed from html
 */
ItemInfo.onRemove = function onRemove() {
	this.uid = -1;

	// Cleanup moveInfo tooltip & cursor
	ItemInfo.hideMoveInfoTooltip();

	// Remove existing compare UI if it's currently displayed
	if (ItemCompare.ui) {
		ItemCompare.remove();
	}
	if (ItemPreview.ui) {
		ItemPreview.remove();
	}
};

/**
 * Initialize UI
 */
ItemInfo.init = function init() {
	const root = ItemInfo.getRoot();

	this._host.style.top = '200px';
	this._host.style.left = '480px';

	const extendBtn = root.querySelector('.extend');
	if (extendBtn) {
		extendBtn.addEventListener('mousedown', onResize);
	}

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => {
			e.stopImmediatePropagation();
		});
		closeBtn.addEventListener('click', () => {
			this.remove();
			if (ItemCompare.ui) {
				ItemCompare.remove();
			}
		});
	}

	// Ask to see card.
	const viewBtn = root.querySelector('.view');
	if (viewBtn) {
		viewBtn.addEventListener('click', () => {
			CardIllustration.append();
			CardIllustration.setCard(this.item);
		});
	}

	this.draggable('.title');
};

/**
 * Bind component
 *
 * @param {object} item
 */
ItemInfo.setItem = function setItem(item) {
	const it = DB.getItemInfo(item.ITID);
	const root = ItemInfo.getRoot();
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

	/* Grade System */
	const container = root.querySelector('.container');
	if (item.enchantgrade) {
		Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/collection_bg_g' + item.enchantgrade + '.bmp', data => {
			if (container) {
				container.style.backgroundImage = `url(${data})`;
			}
		});
	} else {
		Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/collection_bg.bmp', data => {
			if (container) {
				container.style.backgroundImage = `url(${data})`;
			}
		});
	}

	const descInner = root.querySelector('.description-inner');
	if (descInner) {
		const rawDesc = item.IsIdentified ? it.identifiedDescriptionName : it.unidentifiedDescriptionName;
		descInner.innerHTML = DB.formatMsgToHtml(_escapeHTML(rawDesc));
	}

	if (item.HireExpireDate) {
		const dateText = DB.formatUnixDate(item.HireExpireDate);

		// Get message and replace %s
		let msg = DB.getMessage(1255).replace('%s', dateText);
		msg = DB.formatMsgToHtml(msg);

		if (descInner) {
			const div = document.createElement('div');
			div.innerHTML = msg;
			descInner.insertBefore(div, descInner.firstChild);
		}
	}

	if (it.moveInfo) {
		const tooltipHtml = buildMoveInfoTooltip(it.moveInfo);
		if (tooltipHtml) {
			// Create the hoverable label
			const label = document.createElement('span');
			label.textContent = DB.getMessage(2796);
			label.className = 'moveinfo-label';

			// Append label to description
			if (descInner) {
				descInner.appendChild(label);
			}

			// Tooltip container (in light DOM for z-index)
			let tooltip = document.getElementById('moveinfo-tooltip');
			if (!tooltip) {
				tooltip = document.createElement('div');
				tooltip.id = 'moveinfo-tooltip';
				document.body.appendChild(tooltip);
			}

			// Mouse enter → show tooltip
			label.addEventListener('mouseenter', e => {
				Cursor.setType(Cursor.ACTION.CLICK);
				tooltip.innerHTML = tooltipHtml;
				tooltip.style.display = 'block';
				tooltip.style.left = `${e.pageX + 15}px`;
				tooltip.style.top = `${e.pageY + 2}px`;
			});

			// Mouse leave → hide tooltip
			label.addEventListener('mouseleave', () => {
				tooltip.style.display = 'none';
				Cursor.setType(Cursor.ACTION.DEFAULT);
			});

			// Mouse move → follow cursor
			label.addEventListener('mousemove', e => {
				tooltip.style.left = `${e.pageX + 20}px`;
				tooltip.style.top = `${e.pageY + 2}px`;
			});
		}
	}

	updatePreviewButton(item);

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

		case ItemType.ARMOR:
			// Pet Egg check for old versions (before ItemType.PETEGG existed)
			if (DB.isPetEgg(item.ITID)) {
				hideslots = true;
			}
		// falls through
		case ItemType.WEAPON:
		case ItemType.SHADOWGEAR: {
			if (hideslots) {
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
		const root = ItemInfo.getRoot();
		const element = root.querySelector(`.cardlist .item[data-index="${index}"] .icon`);
		if (element) {
			element.style.backgroundImage = `url(${data})`;

			if (itemId && card) {
				element.addEventListener('contextmenu', e => {
					e.preventDefault();
					e.stopImmediatePropagation();
					ItemInfo.setItem({
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
 * Extend ItemInfo window size
 */
function onResize() {
	const top = ItemInfo._host.offsetTop;
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
 * Extend ItemInfo window size
 *
 * @param {number} height
 */
function resize(height) {
	const root = ItemInfo.getRoot();
	const container = root.querySelector('.container');
	const description = root.querySelector('.description');
	const descriptionInner = root.querySelector('.description-inner');
	let containerHeight = height;
	const minHeight = 140;
	const innerH = descriptionInner ? descriptionInner.offsetHeight : 0;
	const maxHeight = innerH + 45 > 140 ? Math.min(innerH + 45, 448) : 140;

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
	const root = ItemInfo.getRoot();
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
					console.error('ItemInfo::addEvent() - startBook failed:', e.message);
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

function updatePreviewButton(item) {
	const root = ItemInfo.getRoot();
	const previewButton = root.querySelector('.btn_mounting');
	if (!previewButton) {
		return;
	}

	if (!canPreviewItem(item)) {
		previewButton.style.display = 'none';
		return;
	}

	previewButton.style.display = 'block';
	// Remove old listener by cloning
	const newBtn = previewButton.cloneNode(true);
	previewButton.parentNode.replaceChild(newBtn, previewButton);
	newBtn.addEventListener('click', e => {
		e.stopImmediatePropagation();
		toggleItemPreview(item);
	});
}

function canPreviewItem(item) {
	if (!item) {
		return false;
	}

	const location = getPreviewLocation(item);
	const previewLocations =
		EquipLocation.HEAD_BOTTOM |
		EquipLocation.HEAD_MID |
		EquipLocation.HEAD_TOP |
		EquipLocation.COSTUME_HEAD_BOTTOM |
		EquipLocation.COSTUME_HEAD_MID |
		EquipLocation.COSTUME_HEAD_TOP |
		EquipLocation.COSTUME_ROBE;

	if (!(location & previewLocations)) {
		return false;
	}

	const it = DB.getItemInfo(item.ITID);
	return getPreviewSpriteId(item, it) > 0;
}

function getPreviewLocation(item) {
	if ('location' in item) {
		return item.location;
	}

	if ('WearState' in item) {
		return item.WearState;
	}

	if ('WearLocation' in item) {
		return item.WearLocation;
	}

	return 0;
}

function getPreviewSpriteId(item, it) {
	if (item && item.wItemSpriteNumber) {
		return item.wItemSpriteNumber;
	}

	if (it && it.ClassNum) {
		return it.ClassNum;
	}

	return 0;
}

function toggleItemPreview(item) {
	if (
		ItemPreview.ui &&
		ItemPreview._host &&
		ItemPreview._host.style.display !== 'none' &&
		ItemPreview.uid === item.ITID
	) {
		ItemPreview.remove();
		return;
	}

	ItemPreview.append();
	ItemPreview.uid = item.ITID;
	ItemPreview.setItem(item);
}

function eventsBooks() {
	const root = ItemInfo.getRoot();
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
	const root = ItemInfo.getRoot();

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
 * A function that handles previewing an item.
 *
 * @param {object} pkt - The packet containing information about the item
 * @return {boolean} Indicates success or failure of the preview action
 */
function onItemPreview(pkt) {
	if (pkt) {
		const item = Inventory.getUI().getItemByIndex(pkt.index);

		if (!item) {
			return false;
		}

		// Remove existing compare UI if it's currently displayed
		if (ItemCompare.ui) {
			ItemCompare.remove();
		}

		// Don't add the same UI twice, remove it
		if (ItemInfo.uid === item.ITID) {
			ItemInfo.remove();
			if (ItemCompare.ui) {
				ItemCompare.remove();
			}
			return false;
		}

		// Add ui to window
		ItemInfo.append();
		ItemInfo.uid = item.ITID;
		ItemInfo.setItem(item);

		// Check if there is an equipped item in the same location
		const compareItem = Equipment.getUI().isInEquipList(item.location);

		// If a comparison item is found, display comparison
		if (compareItem && Inventory.getUI().itemcomp) {
			ItemCompare.prepare();
			ItemCompare.append();
			ItemCompare.uid = compareItem.ITID;
			ItemCompare.setItem(compareItem);
		}
	}
}

/**
 * Build moveInfo tooltip html content
 * @param {object} moveInfo
 * @returns {string} html content
 */
function buildMoveInfoTooltip(moveInfo) {
	const lines = [];

	for (const entry of MOVE_INFO_MESSAGES) {
		if (moveInfo[entry.key] === true) {
			lines.push(DB.getMessage(entry.msgId));
		}
	}
	return lines.map(l => `<div>${l}</div>`).join('');
}

/**
 * Cleanup moveInfo tooltip and restore cursor state.
 * Called when ItemInfo UI is removed.
 */
ItemInfo.hideMoveInfoTooltip = function hideMoveInfoTooltip() {
	const tooltip = document.getElementById('moveinfo-tooltip');
	if (tooltip) {
		tooltip.style.display = 'none';
	}
	Cursor.setType(Cursor.ACTION.DEFAULT);
};

/**
 * Packet Hooks to functions
 */
Network.hookPacket(PACKET.ZC.CHANGE_ITEM_OPTION, onItemPreview);

/**
 * Create component and export it
 */
export default UIManager.addComponent(ItemInfo);
