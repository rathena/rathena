/**
 * UI/Components/Inventory/InventoryCommon.js
 *
 * Shared factory for the Inventory window versions.
 *
 * Each version (InventoryV0..V3) provides its own name, htmlText and cssText
 * plus a set of capability flags describing the behavioral differences that
 * appeared over time (favorite tab, equipment switch, inventory expansion,
 * enchant/refine routing, ...). The factory reproduces every version's legacy
 * behavior 1:1; the flags are the only mechanism used to express real
 * per-version differences.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * In some cases the client will send packet twice.eg NORMAL_ITEMLIST4; fixit [skybook888]
 *
 */

import DB from 'DB/DBManager.js';
import ItemType from 'DB/Items/ItemType.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import Mouse from 'Controls/MouseEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import CartItems from 'UI/Components/CartItems/CartItems.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import ItemCompare from 'UI/Components/ItemCompare/ItemCompare.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import Storage from 'UI/Components/Storage/Storage.js';
import SwitchEquip from 'UI/Components/SwitchEquip/SwitchEquip.js';
import UIVersionManager from 'UI/UIVersionManager.js';
import Configs from 'Core/Configs.js';
import PACKETVER from 'Network/PacketVerManager.js';
import BasicInfo from 'UI/Components/BasicInfo/BasicInfo.js';
import Refine from 'UI/Components/Refine/Refine.js';
import EnchantGrade from 'UI/Components/EnchantGrade/EnchantGrade.js';
import EnchantUI from 'UI/Components/Enchant/Enchant.js';
import Mail from 'UI/Components/Mail/Mail.js';
import WriteRodex from 'UI/Components/Rodex/WriteRodex.js';
import { transferInventoryItemStack } from './InventoryItemTransfer.js';

function _sanitizeHtml(str) {
	const whitelist = ['font', 'i', 'b'];
	const div = document.createElement('div');
	div.innerHTML = str;
	div.querySelectorAll('*').forEach(el => {
		if (!whitelist.includes(el.tagName.toLowerCase())) {
			el.replaceWith(...el.childNodes);
		}
	});
	return div.innerHTML;
}

/**
 * Create an Inventory component.
 *
 * @param {object} config
 * @param {string} config.name                     Component name (kept per version verbatim)
 * @param {string} config.htmlText                 Version HTML template
 * @param {string} config.cssText                  Version CSS
 * @param {number} config.defaultHeight            Default `height` preference value
 * @param {boolean} [config.resizableHeight]       V0: two-dimensional resize (width + height)
 * @param {boolean} [config.tabSprite]             V0: tab highlight via tab_itm sprite (no `selected` class)
 * @param {boolean} [config.favoriteTab]           V1+: favorite tab, footer tools, item compare, count separator
 * @param {boolean} [config.equipSwitch]           V2+: equipment switch list and icons
 * @param {boolean} [config.enchantGrade]          V3: enchant grade icon on items
 * @param {boolean} [config.inventoryExpansion]    V3: inventory expansion button and packets
 * @param {boolean} [config.refineEnchant]         V3: refine/enchant routing on item use
 * @param {boolean} [config.useHiddenClass]        V3: toggle footer tools via `.hidden` class (vs style.display)
 * @param {boolean} [config.hostDropPreventDefault] V2+: preventDefault on host dragover
 * @param {boolean} [config.tabDropPreventDefault] V3: preventDefault on tab dragover
 */
export function createInventory(config) {
	const {
		name,
		htmlText,
		cssText,
		defaultHeight,
		resizableHeight = false,
		tabSprite = false,
		favoriteTab = false,
		equipSwitch = false,
		enchantGrade = false,
		inventoryExpansion = false,
		refineEnchant = false,
		useHiddenClass = false,
		hostDropPreventDefault = false,
		tabDropPreventDefault = false
	} = config;

	const Component = new GUIComponent(name, cssText);
	Component.render = () => htmlText;

	Component.TAB = {
		USABLE: 0,
		EQUIP: 1,
		ETC: 2,
		FAV: 3
	};

	Component.list = [];
	Component.newItems = [];
	Component.equippedItems = [];
	if (equipSwitch) {
		Component.equipswitchlist = [];
	}

	let _realSize = 0;
	let lockOverlayTimeout;

	const prefDefaults = {
		x: 0,
		y: UIVersionManager.getInventoryVersion() > 0 ? 172 : 120,
		width: 7,
		height: defaultHeight,
		show: false,
		reduce: false,
		tab: Component.TAB.USABLE,
		magnet_top: false,
		magnet_bottom: false,
		magnet_left: true,
		magnet_right: false
	};
	if (favoriteTab) {
		prefDefaults.itemlock = false;
		prefDefaults.itemcomp = true;
		prefDefaults.npcsalelock = false;
	}

	const _preferences = Preferences.get(name, prefDefaults, 1.0);

	if (favoriteTab) {
		Component.itemlock = _preferences.itemlock;
		Component.itemcomp = _preferences.itemcomp;
		Component.npcsalelock = _preferences.npcsalelock;
	}

	// Toggle the visibility of a footer element, matching the legacy mechanism
	// of each version: V3 uses the `.hidden` CSS class, earlier versions use
	// inline display. NOTE: on V1/V2 the elements keep their initial `hidden`
	// class, so setting display alone never actually reveals them — that legacy
	// quirk is preserved here on purpose.
	// TODO: V1/V2 footer tools (deal lock / sort) never become visible because
	// the inline-display path cannot override the `.hidden` class in the markup.
	function setElHidden(el, hidden) {
		if (!el) {
			return;
		}
		if (useHiddenClass) {
			el.classList.toggle('hidden', hidden);
		} else {
			el.style.display = hidden ? 'none' : '';
		}
	}

	/**
	 * Initialize UI
	 */
	Component.init = function Init() {
		const root = Component.getRoot();

		const baseBtn = root.querySelector('.titlebar .base');
		if (baseBtn) {
			baseBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		}

		const miniBtn = root.querySelector('.titlebar .mini');
		if (miniBtn) {
			miniBtn.addEventListener('click', onToggleReduction);
		}

		const tabButtons = root.querySelectorAll('.tabs button');
		tabButtons.forEach(btn => {
			btn.addEventListener('mousedown', onSwitchTab);
			if (favoriteTab) {
				btn.addEventListener('dragover', e => {
					if (tabDropPreventDefault) {
						// preventDefault is required to mark the tab as a valid drop target;
						// without it the browser never fires the `drop` event (so dragging an
						// item onto a tab — e.g. to favorite it — silently did nothing).
						e.preventDefault();
					}
					e.stopImmediatePropagation();
				});
				btn.addEventListener('drop', onTabDrop);
			}
		});

		const extendBtn = root.querySelector('.footer .extend');
		if (extendBtn) {
			extendBtn.addEventListener('mousedown', onResize);
		}

		const closeBtn = root.querySelector('.titlebar .close');
		if (closeBtn) {
			closeBtn.addEventListener('click', () => {
				Component._host.style.display = 'none';
			});
		}

		this._host.addEventListener('drop', onDrop);
		this._host.addEventListener('dragover', e => {
			e.stopImmediatePropagation();
			if (hostDropPreventDefault) {
				e.preventDefault();
			}
		});

		const content = root.querySelector('.container .content');
		if (content) {
			content.addEventListener('mouseover', e => {
				const item = e.target.closest('.item');
				if (item) {
					onItemOver.call(item, e);
				}
			});
			content.addEventListener('mouseout', e => {
				const item = e.target.closest('.item');
				if (item) {
					onItemOut();
				}
			});
			content.addEventListener('dragstart', e => {
				const item = e.target.closest('.item');
				if (item) {
					onItemDragStart.call(item, e);
				}
			});
			content.addEventListener('dragend', e => {
				const item = e.target.closest('.item');
				if (item) {
					onItemDragEnd();
				}
			});
			content.addEventListener('contextmenu', e => {
				e.preventDefault();
				const item = e.target.closest('.item');
				if (item) {
					onItemInfo.call(item, e);
				}
			});
			content.addEventListener('dblclick', e => {
				const item = e.target.closest('.item');
				if (item) {
					onItemUsed.call(item, e);
				}
			});
			content.addEventListener('click', e => {
				const item = e.target.closest('.item');
				if (item) {
					onItemClick.call(item, e);
				}
			});
		}

		const ncnt = root.querySelector('.ncnt');
		if (ncnt) {
			ncnt.textContent = favoriteTab ? '0 / ' : '0';
		}
		const mcnt = root.querySelector('.mcnt');
		if (mcnt) {
			mcnt.textContent = '100';
		}

		this.draggable('.titlebar');

		if (!tabSprite) {
			root.querySelectorAll('.tabs button').forEach(b => b.classList.remove('selected'));
			const allTabs = root.querySelectorAll('.tabs button');
			if (allTabs[_preferences.tab]) {
				allTabs[_preferences.tab].classList.add('selected');
			}
		}

		if (favoriteTab) {
			const lockImg = _preferences.itemlock
				? 'inventory/item_drop_lock_on.bmp'
				: 'inventory/item_drop_lock_off.bmp';
			Client.loadFile(DB.INTERFACE_PATH + lockImg, data => {
				const btn = root.querySelector('.item_drop_lock');
				if (btn) {
					btn.style.backgroundImage = `url(${data})`;
				}
			});

			const compImg = _preferences.itemcomp ? 'inventory/item_compare_on.bmp' : 'inventory/item_compare_off.bmp';
			Client.loadFile(DB.INTERFACE_PATH + compImg, data => {
				const btn = root.querySelector('.item_compare');
				if (btn) {
					btn.style.backgroundImage = `url(${data})`;
				}
			});

			const dealOn = root.querySelector('.deallock_on');
			const dealOff = root.querySelector('.deallock_off');
			const onFavTab = _preferences.tab === Component.TAB.FAV;
			setElHidden(dealOn, !(onFavTab && _preferences.npcsalelock));
			setElHidden(dealOff, !(onFavTab && !_preferences.npcsalelock));
			setElHidden(root.querySelector('.sort'), !onFavTab);

			const itemDropLock = root.querySelector('.item_drop_lock');
			if (itemDropLock) {
				itemDropLock.addEventListener('click', onItemLock);
			}
			const itemCompare = root.querySelector('.item_compare');
			if (itemCompare) {
				itemCompare.addEventListener('click', onItemCompare);
			}
			root.querySelectorAll('.deal_lock').forEach(btn => btn.addEventListener('click', onNPCLock));

			const overlayClose = root.querySelector('.lockoverlayclose');
			if (overlayClose) {
				overlayClose.addEventListener('click', () => {
					setElHidden(root.querySelector('.lockoverlaymsg'), true);
					clearTimeout(lockOverlayTimeout);
				});
			}

			const sortBtn = root.querySelector('.sort');
			if (sortBtn) {
				sortBtn.addEventListener('click', () => requestFilter());
			}
		}

		if (inventoryExpansion) {
			const itemExpansion = root.querySelector('.item_expansion');
			if (itemExpansion) {
				itemExpansion.addEventListener('click', onInventoryExpand);
			}
		}
	};

	/**
	 * Apply preferences once append to body
	 */
	Component.onAppend = function OnAppend() {
		const root = Component.getRoot();

		if (!_preferences.show) {
			this._host.style.display = 'none';
		}

		if (tabSprite) {
			Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/tab_itm_0' + (_preferences.tab + 1) + '.bmp', data => {
				const tabSpriteEl = root.querySelector('.tab-sprite');
				if (tabSpriteEl) {
					tabSpriteEl.style.backgroundImage = `url("${data}")`;
				}
			});
		}

		this.resize(_preferences.width, _preferences.height);

		const hostRect = this._host.getBoundingClientRect();
		this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - hostRect.height)}px`;
		this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - hostRect.width)}px`;

		this.magnet.TOP = _preferences.magnet_top;
		this.magnet.BOTTOM = _preferences.magnet_bottom;
		this.magnet.LEFT = _preferences.magnet_left;
		this.magnet.RIGHT = _preferences.magnet_right;

		_realSize = _preferences.reduce ? 0 : this._host.getBoundingClientRect().height;
		const miniBtnAppend = root.querySelector('.titlebar .mini');
		if (miniBtnAppend) {
			miniBtnAppend.dispatchEvent(new Event('mousedown'));
		}
	};

	/**
	 * Remove Inventory from window (and so clean up items)
	 */
	Component.onRemove = function OnRemove() {
		const root = Component.getRoot();
		const content = root.querySelector('.container .content');
		if (content) {
			content.innerHTML = '';
		}
		this.list.length = 0;
		if (equipSwitch) {
			this.equipswitchlist.length = 0;
		}
		Component.newItems.length = 0;
		document.querySelectorAll('.ItemInfo').forEach(el => el.remove());

		// Save preferences
		_preferences.show = this._host.style.display !== 'none';
		_preferences.reduce = !!_realSize;
		_preferences.y = parseInt(this._host.style.top, 10);
		_preferences.x = parseInt(this._host.style.left, 10);
		const hostRect = this._host.getBoundingClientRect();
		_preferences.width = Math.floor((hostRect.width - (23 + 16 + 16 - 30)) / 32);
		if (resizableHeight) {
			_preferences.height = Math.floor((hostRect.height - (31 + 19 - 30)) / 32);
		}
		_preferences.magnet_top = this.magnet.TOP;
		_preferences.magnet_bottom = this.magnet.BOTTOM;
		_preferences.magnet_left = this.magnet.LEFT;
		_preferences.magnet_right = this.magnet.RIGHT;
		_preferences.save();
	};

	/**
	 * Process shortcut
	 *
	 * @param {object} key
	 */
	Component.onShortCut = function onShurtCut(key) {
		switch (key.cmd) {
			case 'TOGGLE':
				if (this._host.style.display === 'none') {
					this._host.style.display = '';
					this.focus();
				} else {
					this._host.dispatchEvent(new Event('mouseleave'));
					this.clearNewItems();
					const root = Component.getRoot();
					root.querySelectorAll('.new_item').forEach(el => {
						el.style.backgroundImage = '';
					});
					this._host.style.display = 'none';
				}
				break;
		}

		const basicInfoUI = BasicInfo.getUI();
		if (basicInfoUI._host) {
			const changeUI = basicInfoUI.getRoot().querySelector('#item .btn_overlay');
			if (changeUI) {
				changeUI.style.display = 'none';
			}
		}
	};

	/**
	 * Show/Hide UI
	 */
	Component.toggle = function toggle() {
		if (this._host.style.display === 'none') {
			this._host.style.display = '';
			this.focus();
		} else {
			this._host.dispatchEvent(new Event('mouseleave'));
			this.clearNewItems();
			const root = Component.getRoot();
			root.querySelectorAll('.new_item').forEach(el => {
				el.style.backgroundImage = '';
			});
			this._host.style.display = 'none';
		}

		const basicInfoUI = BasicInfo.getUI();
		if (basicInfoUI._host) {
			const changeUI = basicInfoUI.getRoot().querySelector('#item .btn_overlay');
			if (changeUI) {
				changeUI.style.display = 'none';
			}
		}
	};

	/**
	 * Clear newItems array
	 */
	Component.clearNewItems = function clearNewItems() {
		this.newItems = [];
	};

	/**
	 * Extend inventory window size
	 *
	 * @param {number} width
	 * @param {number} height
	 */
	Component.resize = function Resize(width, height) {
		width = Math.min(Math.max(width, 6), 8);

		const root = Component.getRoot();
		const content = root.querySelector('.container .content');
		if (content) {
			content.style.width = `${width * 32}px`;
		}

		if (resizableHeight) {
			height = Math.min(Math.max(height, 2), 5);
			this._host.style.width = `${27 + 16 + 16 + width * 32}px`;
			this._host.style.height = `${31 + 4 + 27 + height * 32}px`;
		} else {
			this._host.style.width = `${23 + 16 + 16 + width * 32}px`;
		}

		this.updateScroll();
	};

	/**
	 * Force scroll clamping
	 */
	Component.updateScroll = function updateScroll() {
		const root = Component.getRoot();
		const hostEl = root.querySelector('.scroll-host');
		if (!hostEl) {
			return;
		}

		const contentEl = root.querySelector('.content');
		let ticker = 0;

		const clamp = () => {
			const maxScroll = Math.max(0, hostEl.scrollHeight - hostEl.clientHeight);

			const lastItem = contentEl ? contentEl.querySelector('.item:last-child') : null;
			if (lastItem) {
				const itemRect = lastItem.getBoundingClientRect();
				const hostRect = hostEl.getBoundingClientRect();

				if (itemRect.bottom < hostRect.bottom && hostEl.scrollTop > 0) {
					hostEl.scrollTop = Math.max(0, hostEl.scrollTop - (hostRect.bottom - itemRect.bottom));
				}
			}

			if (hostEl.scrollTop > maxScroll) {
				hostEl.scrollTop = maxScroll;
			}

			if (hostEl._roScrollbarRestart) {
				hostEl._roScrollbarRestart();
			}

			if (ticker++ < 20) {
				requestAnimationFrame(clamp);
			}
		};
		clamp();
	};

	/**
	 * Get item object
	 *
	 * @param {number} id
	 * @returns {Item}
	 */
	Component.getItemById = function GetItemById(id) {
		const list = Component.list;
		for (let i = 0, count = list.length; i < count; ++i) {
			if (list[i].ITID === id) {
				return list[i];
			}
		}
		return null;
	};

	/**
	 * Search in a list for an item by its index
	 *
	 * @param {number} index
	 * @returns {Item}
	 */
	Component.getItemByIndex = function getItemByIndex(index) {
		const list = Component.list;
		for (let i = 0, count = list.length; i < count; ++i) {
			if (list[i].index === index) {
				return list[i];
			}
		}
		return null;
	};

	/**
	 * Get the TAB constant for a given item based on its type.
	 *
	 * @param {object} item
	 * @returns {number} TAB constant
	 */
	function getItemTab(item) {
		switch (item.type) {
			case ItemType.HEALING:
			case ItemType.USABLE:
			case ItemType.DELAYCONSUME:
			case ItemType.CASH:
				return Component.TAB.USABLE;

			case ItemType.WEAPON:
			case ItemType.ARMOR:
			case ItemType.SHADOWGEAR:
			case ItemType.PETEGG:
			case ItemType.PETARMOR:
				return Component.TAB.EQUIP;

			default:
			case ItemType.ETC:
			case ItemType.CARD:
			case ItemType.AMMO:
				return Component.TAB.ETC;
		}
	}

	/**
	 * Add items to the list
	 * if the item index is exist you should clear it;[skybook888]
	 */
	Component.setItems = function SetItems(items) {
		const root = Component.getRoot();
		for (let i = 0, count = items.length; i < count; ++i) {
			const object = this.getItemByIndex(items[i].index);
			if (object) {
				this.removeItem(object.index, object.count);
			}
			if (this.addItemSub(items[i])) {
				this.list.push(items[i]);
				const ncnt = root.querySelector('.ncnt');
				if (ncnt) {
					ncnt.textContent = countLabel();
				}
				this.onUpdateItem(items[i].ITID, items[i].count ? items[i].count : 1);
			}
		}
	};

	function countLabel() {
		return Component.list.length + Equipment.getUI().getNumber() + (favoriteTab ? ' / ' : '');
	}

	/**
	 * Insert Item to inventory
	 *
	 * @param {object} item
	 */
	Component.addItem = function AddItem(item) {
		let object = this.getItemByIndex(item.index);
		const root = Component.getRoot();

		// Check if the item was equipped
		const equippedIndex = Component.equippedItems.indexOf(item.index);
		if (equippedIndex !== -1) {
			Component.equippedItems.splice(equippedIndex, 1);
		} else {
			// Mark as new item
			Component.newItems.push(item.index);

			const basicInfoUI = BasicInfo.getUI();
			if (basicInfoUI._host) {
				const changeUI = basicInfoUI.getRoot().querySelector('#item .btn_overlay');
				if (changeUI) {
					changeUI.style.display = 'block';
				}
			}
		}

		if (refineEnchant) {
			const hasRefineFlag = Configs.get('enableRefineUI') || PACKETVER.value >= 20161012;
			if (hasRefineFlag && Refine.isRefineOpen()) {
				const refineRoot = Refine._shadow || Refine._host;
				if (refineRoot) {
					const refinecount = refineRoot.querySelector(
						`.materials .item[data-index="${item.ITID}"] .mat_count`
					);
					if (refinecount) {
						const previousData = parseInt(refinecount.textContent.split('/')[0], 10);
						const newCount = previousData + item.count;
						refinecount.textContent = `${newCount}/1`;
					}
				}
			}
		}

		if (object) {
			if (isNaN(object.count)) {
				object.count = 1;
			}
			if (isNaN(item.count)) {
				item.count = 1;
			}
			object.count += item.count;
			const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
			if (countEl) {
				countEl.textContent = object.count;
			}
			this.onUpdateItem(object.ITID, object.count);
			if (Component.newItems.indexOf(item.index) === -1) {
				Component.newItems.push(item.index);
			}
			if (getItemTab(item) === _preferences.tab) {
				Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/new_item.bmp', data => {
					const el = root.querySelector(`.item[data-index="${item.index}"] .new_item`);
					if (el) {
						el.style.backgroundImage = `url(${data})`;
					}
				});
			}
			return;
		}

		object = Object.assign({}, item);
		if (this.addItemSub(object)) {
			this.list.push(object);
			const ncnt = root.querySelector('.ncnt');
			if (ncnt) {
				ncnt.textContent = countLabel();
			}
			this.onUpdateItem(object.ITID, object.count);
		}
	};

	/**
	 * Check if item index is in newItems list
	 */
	Component.isNewItem = function isNewItem(index) {
		return Component.newItems.includes(index);
	};

	/**
	 * Add item to inventory
	 *
	 * @param {object} item
	 */
	Component.addItemSub = function AddItemSub(item) {
		let tab = getItemTab(item);

		if (favoriteTab && item.PlaceETCTab) {
			tab = Component.TAB.FAV;
		}

		if (item.WearState && item.type !== ItemType.AMMO && item.type !== ItemType.CARD) {
			Equipment.getUI().equip(item, item.WearState);
			return false;
		}

		const isInSwitchList = equipSwitch
			? Component.equipswitchlist.some(equipItem => equipItem.index === item.index)
			: false;

		if (isInSwitchList) {
			SwitchEquip.equip(item, item.location, true);
		}

		if (tab === _preferences.tab) {
			const it = DB.getItemInfo(item.ITID);
			const root = Component.getRoot();
			const content = root.querySelector('.container .content');
			if (!content) {
				return true;
			}

			content.insertAdjacentHTML(
				'beforeend',
				`<div class="item" data-index="${item.index}" draggable="true">` +
					'<div class="new_item"></div>' +
					'<div class="icon"></div>' +
					(equipSwitch ? '<div class="switch1"></div>' + '<div class="switch2"></div>' : '') +
					(enchantGrade ? '<div class="grade"></div>' : '') +
					`<div class="amount"><span class="count">${item.count || 1}</span></div>` +
					'</div>'
			);

			Client.loadFile(
				DB.INTERFACE_PATH +
					'item/' +
					(item.IsIdentified ? it.identifiedResourceName : it.unidentifiedResourceName) +
					'.bmp',
				data => {
					const icon = root.querySelector(`.item[data-index="${item.index}"] .icon`);
					if (icon) {
						icon.style.backgroundImage = `url(${data})`;
					}
				}
			);

			if (Component.isNewItem(item.index)) {
				Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/new_item.bmp', data => {
					const el = root.querySelector(`.item[data-index="${item.index}"] .new_item`);
					if (el) {
						el.style.backgroundImage = `url(${data})`;
					}
				});
			} else {
				const el = root.querySelector(`.item[data-index="${item.index}"] .new_item`);
				if (el) {
					el.style.backgroundImage = '';
				}
			}

			if (isInSwitchList) {
				Client.loadFile(DB.INTERFACE_PATH + 'swap_equipment/bg_change.bmp', data => {
					const el = root.querySelector(`.item[data-index="${item.index}"] .switch1`);
					if (el) {
						el.style.backgroundImage = `url(${data})`;
					}
				});
				Client.loadFile(DB.INTERFACE_PATH + 'swap_equipment/ico_change.bmp', data => {
					const el = root.querySelector(`.item[data-index="${item.index}"] .switch2`);
					if (el) {
						el.style.backgroundImage = `url(${data})`;
					}
				});
			}

			if (enchantGrade && item.enchantgrade) {
				Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/grade_icon' + item.enchantgrade + '.bmp', data => {
					const el = root.querySelector(`.item[data-index="${item.index}"] .grade`);
					if (el) {
						el.style.backgroundImage = `url(${data})`;
					}
				});
			}
		}

		return true;
	};

	if (equipSwitch) {
		Component.isInEquipSwitchList = function (location) {
			return this.equipswitchlist.some(existingItem => (existingItem.location & location) !== 0);
		};
	}

	/**
	 * Remove item from inventory
	 *
	 * @param {number} index in inventory
	 * @param {number} count
	 */
	Component.removeItem = function RemoveItem(index, count) {
		const item = this.getItemByIndex(index);
		const root = Component.getRoot();

		if (!item || count <= 0) {
			return null;
		}

		if (item.count) {
			item.count -= count;
			if (item.count > 0) {
				const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
				if (countEl) {
					countEl.textContent = item.count;
				}
				this.onUpdateItem(item.ITID, item.count);
				return item;
			}
		}

		this.list.splice(this.list.indexOf(item), 1);
		const el = root.querySelector(`.item[data-index="${item.index}"]`);
		if (el) {
			el.remove();
		}
		const ncnt = root.querySelector('.ncnt');
		if (ncnt) {
			ncnt.textContent = countLabel();
		}
		this.onUpdateItem(item.ITID, 0);

		const overlay = root.querySelector('.overlay');
		if (overlay) {
			overlay.style.display = 'none';
		}

		return item;
	};

	/**
	 * Update item in inventory
	 *
	 * @param {number} index in inventory
	 * @param {number} count
	 */
	Component.updateItem = function UpdateItem(index, count) {
		const item = this.getItemByIndex(index);
		if (!item) {
			return;
		}

		item.count = count;
		const root = Component.getRoot();

		if (item.count > 0) {
			const countEl = root.querySelector(`.item[data-index="${item.index}"] .count`);
			if (countEl) {
				countEl.textContent = item.count;
			}
			this.onUpdateItem(item.ITID, item.count);
			return;
		}

		this.list.splice(this.list.indexOf(item), 1);
		const el = root.querySelector(`.item[data-index="${item.index}"]`);
		if (el) {
			el.remove();
		}
		const ncnt = root.querySelector('.ncnt');
		if (ncnt) {
			ncnt.textContent = countLabel();
		}
		this.onUpdateItem(item.ITID, 0);

		this.updateScroll();
	};

	/**
	 * Use an item
	 *
	 * @param {Item} item
	 */
	Component.useItem = function UseItem(item) {
		switch (item.type) {
			case ItemType.HEALING:
			case ItemType.USABLE:
			case ItemType.CASH:
				Component.onUseItem(item.index);
				break;
			case ItemType.CARD:
				Component.onUseCard(item.index);
				break;
			case ItemType.DELAYCONSUME:
				break;
			case ItemType.WEAPON:
			case ItemType.ARMOR:
			case ItemType.SHADOWGEAR:
				if (refineEnchant) {
					const hasRefineFlag = Configs.get('enableRefineUI') || PACKETVER.value >= 20161012;
					const hasEnchantGradeFlag = PACKETVER.value >= 20200916;
					const hasEnchantFlag = PACKETVER.value >= 20211103;

					if (hasRefineFlag && Refine.isRefineOpen()) {
						Refine.onRequestItemRefine(item);
						break;
					}
					if (hasEnchantGradeFlag && EnchantGrade.isEnchantGradeOpen()) {
						EnchantGrade.onRequestItemEnchantGrade(item);
						break;
					}
					if (hasEnchantFlag && EnchantUI.isEnchantOpen()) {
						EnchantUI.onRequestItemEnchant(item);
						break;
					}
				}
			// falls through
			case ItemType.PETARMOR:
			case ItemType.AMMO:
				if (item.IsIdentified && !item.IsDamaged) {
					Component.onEquipItem(item.index, item.location);
				}
				break;
		}
	};

	/**
	 * Extend inventory window size
	 */
	function onResize() {
		const top = Component._host.offsetTop;
		const left = Component._host.offsetLeft;
		let lastWidth = 0;
		let lastHeight = 0;

		function resizing() {
			const extraX = 23 + 16 + 16 - 30;

			let w = Math.floor((Mouse.screen.x - left - extraX) / 32);
			w = Math.min(Math.max(w, 6), resizableHeight ? 8 : 9);

			if (resizableHeight) {
				const extraY = 31 + 19 - 30;
				let h = Math.floor((Mouse.screen.y - top - extraY) / 32);
				h = Math.min(Math.max(h, 2), 5);

				if (w === lastWidth && h === lastHeight) {
					return;
				}

				Component.resize(w, h);
				lastWidth = w;
				lastHeight = h;
			} else {
				if (w === lastWidth) {
					return;
				}

				Component.resize(w);
				lastWidth = w;
			}
		}

		const _Interval = setInterval(resizing, 30);

		const onMouseUp = event => {
			if (event.which === 1) {
				clearInterval(_Interval);
				window.removeEventListener('mouseup', onMouseUp);
			}
		};
		window.addEventListener('mouseup', onMouseUp);
	}

	/**
	 * Modify tab, filter display entries
	 */
	function onSwitchTab() {
		const root = Component.getRoot();
		const buttons = root.querySelectorAll('.tabs button');
		const idx = Array.from(buttons).indexOf(this);
		_preferences.tab = parseInt(idx, 10);

		if (tabSprite) {
			Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/tab_itm_0' + (idx + 1) + '.bmp', data => {
				const tabSpriteEl = root.querySelector('.tab-sprite');
				if (tabSpriteEl) {
					tabSpriteEl.style.backgroundImage = `url(${data})`;
				}
				requestFilter();
			});
			return;
		}

		requestFilter();

		buttons.forEach(b => b.classList.remove('selected'));
		this.classList.add('selected');

		if (favoriteTab) {
			const dealOn = root.querySelector('.deallock_on');
			const dealOff = root.querySelector('.deallock_off');
			const lockOverlay = root.querySelector('.lockoverlay');
			const lockMsg = root.querySelector('.lockoverlaymsg');
			const sort = root.querySelector('.sort');
			const onFavTab = _preferences.tab === Component.TAB.FAV;

			setElHidden(dealOn, !(onFavTab && _preferences.npcsalelock));
			setElHidden(dealOff, !(onFavTab && !_preferences.npcsalelock));
			setElHidden(lockOverlay, !(onFavTab && _preferences.npcsalelock));
			setElHidden(lockMsg, true);
			setElHidden(sort, !onFavTab);
		}
	}

	/**
	 * Hide/show inventory's content
	 */
	function onToggleReduction() {
		const root = Component.getRoot();
		const panel = root.querySelector('.panel');

		if (_realSize) {
			if (panel) {
				panel.style.display = 'flex';
			}
			Component._host.style.height = `${_realSize}px`;
			_realSize = 0;
		} else {
			_realSize = Component._host.getBoundingClientRect().height;
			Component._host.style.height = '17px';
			if (panel) {
				panel.style.display = 'none';
			}
		}
	}

	/**
	 * Update tab, reset inventory content
	 */
	function requestFilter() {
		const root = Component.getRoot();
		const host = root.querySelector('.scroll-host');
		if (host) {
			host.scrollTop = 0;
		}

		const content = root.querySelector('.container .content');
		if (content) {
			content.innerHTML = '';
		}

		const list = Component.list;
		for (let i = 0, count = list.length; i < count; ++i) {
			Component.addItemSub(list[i]);
		}

		Component.updateScroll();
	}

	/**
	 * Drop an item from storage to inventory
	 *
	 * @param {event} event
	 */
	function onDrop(event) {
		let item, data;
		event.stopImmediatePropagation();

		try {
			data = JSON.parse(event.dataTransfer.getData('Text'));
			item = data.data;
		} catch (_e) {
			return false;
		}

		if (
			data.type !== 'item' ||
			(data.from !== 'Storage' && data.from !== 'CartItems' && data.from !== 'Mail' && data.from !== 'WriteRodex')
		) {
			return false;
		}

		if (item.count > 1) {
			InputBox.append();
			InputBox.setType('number', false, item.count);

			InputBox.onSubmitRequest = function OnSubmitRequest(count) {
				InputBox.remove();

				switch (data.from) {
					case 'Storage':
						Storage.reqRemoveItem(item.index, parseInt(count, 10));
						break;
					case 'CartItems':
						CartItems.reqRemoveItem(item.index, parseInt(count, 10));
						break;
					case 'Mail':
						Mail.reqRemoveItem(item.index, parseInt(count, 10));
						break;
					case 'WriteRodex':
						WriteRodex.requestRemoveItemRodex(item.index, parseInt(count, 10));
						break;
				}
			};
			return false;
		}

		switch (data.from) {
			case 'Storage':
				Storage.reqRemoveItem(item.index, 1);
				break;
			case 'CartItems':
				CartItems.reqRemoveItem(item.index, 1);
				break;
			case 'Mail':
				Mail.reqRemoveItem(item.index, 1);
				break;
			case 'WriteRodex':
				WriteRodex.requestRemoveItemRodex(item.index, 1);
				break;
		}

		return false;
	}

	/**
	 * Show item name when mouse is over
	 */
	function onItemOver(_e) {
		const idx = parseInt(this.getAttribute('data-index'), 10);
		const item = Component.getItemByIndex(idx);

		if (!item) {
			return;
		}

		let quantity = ' ea';
		if (
			item.Options &&
			(item.type === ItemType.WEAPON || item.type === ItemType.ARMOR || item.type === ItemType.SHADOWGEAR) &&
			item.Options.filter(Option => Option.index !== 0).length > 0
		) {
			quantity = ' Quantity';
		}

		const root = Component.getRoot();
		const overlay = root.querySelector('.overlay');
		const rootEl = root.querySelector(`#${name}`) || root;
		const itemRect = this.getBoundingClientRect();
		const rootRect = rootEl.getBoundingClientRect();

		if (overlay) {
			overlay.style.display = 'block';
			overlay.style.top = `${itemRect.top - rootRect.top}px`;
			overlay.style.left = `${itemRect.left - rootRect.left + 35}px`;
			overlay.innerHTML = _sanitizeHtml(`${DB.getItemName(item)}: ${item.count || 1}${quantity}`);

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
		const root = Component.getRoot();
		const overlay = root.querySelector('.overlay');
		if (overlay) {
			overlay.style.display = 'none';
		}
	}

	/**
	 * Start dragging an item
	 */
	function onItemDragStart(event) {
		const index = parseInt(this.getAttribute('data-index'), 10);
		const item = Component.getItemByIndex(index);

		if (!item) {
			return;
		}

		const img = new Image();
		const iconEl = this.querySelector('.icon');
		const url = iconEl ? iconEl.style.backgroundImage.match(/\((.*?)\)/)?.[1]?.replace(/('|")/g, '') : '';
		img.decoding = 'async';
		img.src = url || '';

		event.dataTransfer.setDragImage(img, 12, 12);
		event.dataTransfer.setData(
			'Text',
			JSON.stringify(
				(window._OBJ_DRAG_ = {
					type: 'item',
					from: 'Inventory',
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
		const item = Component.getItemByIndex(index);

		if (!item) {
			return false;
		}

		if (event.altKey && event.which === 3) {
			event.stopImmediatePropagation();
			transferItemToOtherUI(item);
			return false;
		}

		if (ItemInfo.uid === item.ITID) {
			ItemInfo.remove();
			if (favoriteTab && ItemCompare.ui) {
				ItemCompare.remove();
			}
			return false;
		}

		if (favoriteTab && ItemCompare.ui) {
			ItemCompare.remove();
		}

		ItemInfo.append();
		ItemInfo.uid = item.ITID;
		ItemInfo.setItem(item);

		if (favoriteTab) {
			const compareItem = Equipment.getUI().isInEquipList(item.location);

			if (compareItem && Component.itemcomp) {
				ItemCompare.prepare();
				ItemCompare.append();
				ItemCompare.uid = compareItem.ITID;
				ItemCompare.setItem(compareItem);
			}
		}

		return false;
	}

	/**
	 * Alt Right Click Request Transfer
	 */
	function transferItemToOtherUI(item) {
		if (transferInventoryItemStack(item, UIManager.components)) {
			return true;
		}

		const storageUI = Storage.getUI();
		const isStorageOpen = storageUI._host ? storageUI._host.style.display !== 'none' : false;
		const isCartOpen = CartItems._host ? CartItems._host.style.display !== 'none' : false;

		if (!item) {
			return false;
		}

		const count = item.count || 1;

		if (isStorageOpen) {
			Storage.reqAddItem(item.index, count);
		} else if (isCartOpen) {
			Component.reqMoveItemToCart(item.index, count);
		}

		return true;
	}

	/**
	 * Ask to use an item
	 */
	function onItemUsed(event) {
		const index = parseInt(this.getAttribute('data-index'), 10);
		const item = Component.getItemByIndex(index);

		if (item) {
			Component.useItem(item);
			onItemOut();
		}

		event.stopImmediatePropagation();
		event.preventDefault();
	}

	/**
	 * Handle click event on an item
	 */
	function onItemClick(event) {
		if (event.shiftKey && event.which === 1) {
			const idx = parseInt(this.getAttribute('data-index'), 10);
			const item = Component.getItemByIndex(idx);
			if (!item) {
				return false;
			}

			item.name = DB.getItemName(item);
			const link =
				'<span data-item="' +
				DB.createItemLink(item) +
				'" class="item-link" style="color:#A9B95F;">&lt;' +
				item.name +
				'&gt;</span>';

			const chatRoot = ChatBox._shadow || ChatBox._host || document;
			const msgBox = chatRoot.querySelector('.input-chatbox');
			if (msgBox) {
				msgBox.innerHTML += link + ' ';
				msgBox.focus();
			}

			event.stopImmediatePropagation();
		}

		return false;
	}

	/**
	 * Handle drop event on tabs
	 */
	function onTabDrop(event) {
		let item, data;
		event.stopImmediatePropagation();

		try {
			data = JSON.parse(event.dataTransfer.getData('Text'));
			item = data.data;
		} catch (_e) {
			return false;
		}

		if (data.type !== 'item') {
			return false;
		}
		if (!item) {
			return false;
		}

		const targetTab = event.target.getAttribute('data-tab');
		const itemfav = targetTab === 'fav' ? 0 : 1;

		const pkt = new PACKET.CZ.INVENTORY_TAB();
		pkt.item_index = item.index;
		pkt.favorite = itemfav;
		Network.sendPacket(pkt);
	}

	if (favoriteTab) {
		/**
		 * Update PlaceETCTab of an item in the inventory
		 */
		Component.updatePlaceETCTab = function (itemIndex, newValue) {
			const item = Component.getItemByIndex(itemIndex);

			if (!item) {
				return;
			}

			if (newValue) {
				let favoriteval;
				switch (item.type) {
					case ItemType.HEALING:
					case ItemType.USABLE:
					case ItemType.DELAYCONSUME:
					case ItemType.CASH:
					case ItemType.ETC:
					case ItemType.CARD:
					case ItemType.AMMO:
						favoriteval = 2;
						break;
					case ItemType.WEAPON:
					case ItemType.ARMOR:
					case ItemType.SHADOWGEAR:
					case ItemType.PETEGG:
					case ItemType.PETARMOR:
						favoriteval = 4;
						break;
					default:
						break;
				}
				item.PlaceETCTab = favoriteval;
			} else {
				item.PlaceETCTab = newValue;
			}

			requestFilter();
		};
	}

	/**
	 * Toggle the item drop lock preference
	 */
	function onItemLock() {
		_preferences.itemlock = !_preferences.itemlock;
		Component.itemlock = _preferences.itemlock;

		const lockImg = _preferences.itemlock ? 'inventory/item_drop_lock_on.bmp' : 'inventory/item_drop_lock_off.bmp';
		Client.loadFile(DB.INTERFACE_PATH + lockImg, data => {
			const root = Component.getRoot();
			const btn = root.querySelector('.item_drop_lock');
			if (btn) {
				btn.style.backgroundImage = `url(${data})`;
			}
		});
	}

	/**
	 * Toggles the value of Item Compare
	 */
	function onItemCompare() {
		_preferences.itemcomp = !_preferences.itemcomp;
		Component.itemcomp = _preferences.itemcomp;

		const compImg = _preferences.itemcomp ? 'inventory/item_compare_on.bmp' : 'inventory/item_compare_off.bmp';
		Client.loadFile(DB.INTERFACE_PATH + compImg, data => {
			const root = Component.getRoot();
			const btn = root.querySelector('.item_compare');
			if (btn) {
				btn.style.backgroundImage = `url(${data})`;
			}
		});
	}

	/**
	 * Toggles the value of Item Lock NPCSale
	 */
	function onNPCLock() {
		_preferences.npcsalelock = !_preferences.npcsalelock;
		Component.npcsalelock = _preferences.npcsalelock;

		const root = Component.getRoot();
		const dealOn = root.querySelector('.deallock_on');
		const dealOff = root.querySelector('.deallock_off');
		const lockOverlay = root.querySelector('.lockoverlay');
		const lockMsg = root.querySelector('.lockoverlaymsg');

		setElHidden(dealOn, !_preferences.npcsalelock);
		setElHidden(dealOff, _preferences.npcsalelock);

		if (_preferences.npcsalelock) {
			setElHidden(lockOverlay, false);
			setElHidden(lockMsg, false);

			lockOverlayTimeout = setTimeout(() => {
				setElHidden(root.querySelector('.lockoverlaymsg'), true);
			}, 3000);
		} else {
			setElHidden(lockOverlay, true);
			setElHidden(lockMsg, true);
		}
	}

	if (equipSwitch) {
		Component.addItemtoSwitch = function (index) {
			const item = this.getItemByIndex(index);
			if (!item) {
				console.warn(`Item with index ${index} not found in inventory.`);
				return;
			}

			const existingItemIndex = this.equipswitchlist.findIndex(
				existingItem => existingItem.location === item.location
			);

			if (existingItemIndex > -1) {
				const existingItem = this.equipswitchlist[existingItemIndex];
				SwitchEquip.unEquip(existingItem.index, existingItem.location);
				this.equipswitchlist.splice(existingItemIndex, 1);
			}

			this.equipswitchlist.push(item);

			const root = Component.getRoot();
			Client.loadFile(DB.INTERFACE_PATH + 'swap_equipment/bg_change.bmp', data => {
				const el = root.querySelector(`.item[data-index="${item.index}"] .switch1`);
				if (el) {
					el.style.backgroundImage = `url(${data})`;
				}
			});
			Client.loadFile(DB.INTERFACE_PATH + 'swap_equipment/ico_change.bmp', data => {
				const el = root.querySelector(`.item[data-index="${item.index}"] .switch2`);
				if (el) {
					el.style.backgroundImage = `url(${data})`;
				}
			});

			SwitchEquip.equip(item, item.location, true);
			ChatBox.addText(DB.getItemName(item) + ' ' + DB.getMessage(3143), ChatBox.TYPE.BLUE, ChatBox.FILTER.ITEM);
		};

		Component.removeItemFromSwitch = function (index) {
			const item = this.getItemByIndex(index);
			if (!item) {
				console.warn(`Item with index ${index} not found in inventory.`);
				return;
			}

			const existingItemIndex = this.equipswitchlist.findIndex(existingItem => existingItem.index === item.index);

			if (existingItemIndex > -1) {
				const root = Component.getRoot();
				const sw1 = root.querySelector(`.item[data-index="${item.index}"] .switch1`);
				if (sw1) {
					sw1.style.backgroundImage = 'none';
				}
				const sw2 = root.querySelector(`.item[data-index="${item.index}"] .switch2`);
				if (sw2) {
					sw2.style.backgroundImage = 'none';
				}

				SwitchEquip.unEquip(item.index, item.location);
				this.equipswitchlist.splice(existingItemIndex, 1);

				ChatBox.addText(
					DB.getItemName(item) + ' ' + DB.getMessage(3144),
					ChatBox.TYPE.BLUE,
					ChatBox.FILTER.ITEM
				);

				Equipment.getUI().equipItemsToSwitch();
				Component.equipAllFromSwitchList();
			}
		};

		Component.equipAllFromSwitchList = function equipAllFromSwitchList() {
			const equipSwitchList = Component.equipswitchlist;
			for (let i = 0; i < equipSwitchList.length; i++) {
				const item = equipSwitchList[i];
				if (item) {
					SwitchEquip.equip(item, item.location, true);
				}
			}
		};
	}

	/**
	 * Request inventory expansion using an expansion item
	 */
	function onInventoryExpand() {
		const itemIds = [25791, 25792, 25793];
		let itemforexpand = null;

		for (let i = 0; i < itemIds.length; i++) {
			itemforexpand = Component.getItemById(itemIds[i]);
			if (itemforexpand) {
				break;
			}
		}

		if (!itemforexpand) {
			ChatBox.addText(DB.getMessage(3564), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			return false;
		}

		const pkt = new PACKET.CZ.REQ_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE();
		Network.sendPacket(pkt);
	}

	if (inventoryExpansion) {
		const onRequestInventoryExpandResult = function onRequestInventoryExpandResult(pkt) {
			if (pkt) {
				switch (pkt.result) {
					case 0: {
						const item = Component.getItemById(pkt.itemId);
						if (!item) {
							return false;
						}

						const itemname = DB.getItemName(item);
						const root = Component.getRoot();
						const mcntEl = root.querySelector('.mcnt');
						const currentlimit = mcntEl ? parseInt(mcntEl.textContent, 10) : 100;
						const newlimit = currentlimit + 10;

						UIManager.showPromptBox(
							DB.getMessage(3561)
								.replace('%s', itemname)
								.replace('%d', currentlimit)
								.replace('%d', newlimit),
							'ok',
							'cancel',
							InventoryExpandReq,
							InventoryExpandCancel
						);
						break;
					}
					case 1:
						ChatBox.addText(DB.getMessage(3562), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						break;
					case 2:
						ChatBox.addText(DB.getMessage(3563), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						break;
					case 3:
						ChatBox.addText(DB.getMessage(3564), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						break;
					case 4:
						ChatBox.addText(DB.getMessage(3565), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						break;
					default:
						break;
				}
			}
		};

		function InventoryExpandReq() {
			const pkt = new PACKET.CZ.REQ_EXTEND_BODYITEM_SIZE();
			Network.sendPacket(pkt);
		}

		function InventoryExpandCancel() {
			const pkt = new PACKET.CZ.CLOSE_MSGBOX_EXTEND_BODYITEM_SIZE();
			Network.sendPacket(pkt);
		}

		const onFinalReqInventoryExpandResult = function onFinalReqInventoryExpandResult(pkt) {
			if (pkt) {
				switch (pkt.result) {
					case 0:
						ChatBox.addText(DB.getMessage(3566), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
						break;
					case 1:
						ChatBox.addText(DB.getMessage(3562), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						break;
					case 2:
						ChatBox.addText(DB.getMessage(3563), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						break;
					case 3:
						ChatBox.addText(DB.getMessage(3564), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						break;
					case 4:
						ChatBox.addText(DB.getMessage(3565), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						break;
					default:
						break;
				}
			}
		};

		Network.hookPacket(PACKET.ZC.ACK_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE, onRequestInventoryExpandResult);
		Network.hookPacket(PACKET.ZC.ACK_EXTEND_BODYITEM_SIZE, onFinalReqInventoryExpandResult);
	}

	/**
	 * functions to define
	 */
	Component.onUseItem = function OnUseItem(/* index */) {};
	Component.onUseCard = function onUseCard(/* index */) {};
	Component.onEquipItem = function OnEquipItem(/* index, location */) {};
	Component.onUpdateItem = function OnUpdateItem(/* index, amount */) {};
	Component.reqMoveItemToCart = function reqMoveItemToCart(/* index, amount */) {};

	return UIManager.addComponent(Component);
}
