/**
 * UI/Components/Equipment/EquipmentCommon.js
 *
 * Shared factory for every Equipment window version (V0 - V4).
 *
 * Version differences are passed as capability flags; the WinStats status
 * window is anchored through the embed/unembed model for every version
 * (the legacy .status_component / WinStats._host path used by V3/V4 was dead
 * code after the WinStats refactor and is converged here).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import StatusConst from 'DB/Status/StatusState.js';
import EquipLocation from 'DB/Items/EquipmentLocation.js';
import Network from 'Network/NetworkManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import ItemType from 'DB/Items/ItemType.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import Camera from 'Renderer/Camera.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import UIVersionManager from 'UI/UIVersionManager.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import CartItems from 'UI/Components/CartItems/CartItems.js';
import SwitchEquip from 'UI/Components/SwitchEquip/SwitchEquip.js';
import WinStats from 'UI/Components/WinStats/WinStats.js';
import GraphicsSettings from 'Preferences/Graphics.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import Entity from 'Renderer/Entity/Entity.js';

function escapeHTML(str) {
	const div = document.createElement('div');
	div.textContent = str;
	return div.innerHTML;
}

function getFirstChildWithTagName(element, tagName) {
	for (let i = 0; i < element.childNodes.length; i++) {
		if (element.childNodes[i].nodeName === tagName.toUpperCase()) {
			return element.childNodes[i];
		}
	}
}

function getHash(url) {
	const hashPos = url.lastIndexOf('#');
	return url.substring(hashPos + 1);
}

function getSelectorFromLocation(location) {
	const selector = [];
	if (location & EquipLocation.HEAD_TOP) selector.push('.head_top');
	if (location & EquipLocation.HEAD_MID) selector.push('.head_mid');
	if (location & EquipLocation.HEAD_BOTTOM) selector.push('.head_bottom');
	if (location & EquipLocation.ARMOR) selector.push('.armor');
	if (location & EquipLocation.WEAPON) selector.push('.weapon');
	if (location & EquipLocation.SHIELD) selector.push('.shield');
	if (location & EquipLocation.GARMENT) selector.push('.garment');
	if (location & EquipLocation.SHOES) selector.push('.shoes');
	if (location & EquipLocation.ACCESSORY1) selector.push('.accessory1');
	if (location & EquipLocation.ACCESSORY2) selector.push('.accessory2');
	if (location & EquipLocation.AMMO) selector.push('.ammo');
	if (location & EquipLocation.COSTUME_HEAD_TOP) selector.push('.costume_head_top');
	if (location & EquipLocation.COSTUME_HEAD_MID) selector.push('.costume_head_mid');
	if (location & EquipLocation.COSTUME_HEAD_BOTTOM) selector.push('.costume_head_bottom');
	if (location & EquipLocation.SHADOW_ARMOR) selector.push('.shadow_armor');
	if (location & EquipLocation.SHADOW_WEAPON) selector.push('.shadow_weapon');
	if (location & EquipLocation.SHADOW_SHIELD) selector.push('.shadow_shield');
	if (location & EquipLocation.COSTUME_ROBE) selector.push('.shadow_garment');
	if (location & EquipLocation.SHADOW_SHOES) selector.push('.shadow_shoes');
	if (location & EquipLocation.SHADOW_R_ACCESSORY_SHADOW) selector.push('.shadow_accessory1');
	if (location & EquipLocation.SHADOW_L_ACCESSORY_SHADOW) selector.push('.shadow_accessory2');
	return selector.join(', ');
}

export function createEquipment({
	name,
	htmlText,
	cssText,
	entityRender = true,
	enchantGrade = false,
	switchEquip = false,
	titles = false,
	costumeConfig = false,
	damageSkin = false,
	statsDefault = true
}) {
	const Component = new GUIComponent(name, cssText);

	Component.render = () => htmlText;

	const _preferences = Preferences.get(
		name,
		{
			x: 480,
			y: 200,
			show: false,
			reduce: false,
			stats: statsDefault
		},
		1.0
	);

	let _list = {};
	const _ctx = [];
	let _showEquip = false;
	let _hideCostume = false;
	let _currentTitleId = 0;
	let _btnLevelUp;

	const tabLinks = {};
	const contentDivs = {};
	let currentTabId = 'general';

	let switchappend;
	let switchUIopen;

	Component.init = function init() {
		const root = Component.getRoot();
		const canvases = root.querySelectorAll('canvas');
		if (canvases[0]) _ctx.push(canvases[0].getContext('2d'));
		if (canvases[1]) _ctx.push(canvases[1].getContext('2d'));

		const tabsEl = root.querySelector('#tabs');
		if (tabsEl) {
			const tabListItems = tabsEl.childNodes;
			for (let i = 0; i < tabListItems.length; i++) {
				if (tabListItems[i].nodeName === 'DIV') {
					const tabLink = getFirstChildWithTagName(tabListItems[i], 'A');
					if (tabLink) {
						const id = getHash(tabLink.getAttribute('href'));
						tabLinks[id] = tabLink;
						contentDivs[id] = root.querySelector(`#${id}`);
					}
				}
			}
		}

		let idx = 0;
		for (const id in tabLinks) {
			tabLinks[id].onclick = showTab;
			tabLinks[id].onfocus = function () {
				this.blur();
			};
			if (idx === 0) tabLinks[id].className = 'tab selected';
			idx++;
		}

		idx = 0;
		for (const id in contentDivs) {
			if (contentDivs[id]) {
				if (idx !== 0) contentDivs[id].classList.add('content', 'hide');
				idx++;
			}
		}

		if (UIVersionManager.getEquipmentVersion() > 0) {
			const lvlupEl = root.querySelector('#lvlup_base');
			if (lvlupEl) {
				_btnLevelUp = lvlupEl;
				lvlupEl.remove();
				_btnLevelUp.addEventListener('mousedown', e => e.stopImmediatePropagation());
				_btnLevelUp.addEventListener('click', () => {
					if (_btnLevelUp.parentNode) _btnLevelUp.remove();
					Component._host.style.display = '';
					Component._host.parentNode.appendChild(Component._host);
					if (Component._host.style.display !== 'none') {
						Renderer.render(renderCharacter);
					}
				});
			}
		} else {
			const footer = root.querySelector('#equipment_footer');
			if (footer) footer.remove();
			const rootEl = root.querySelector('#' + name);
			if (rootEl) rootEl.classList.add('equipmentV0');
			const lvlup = root.querySelector('#lvlup_base');
			if (lvlup) lvlup.remove();
		}

		const baseBtn = root.querySelector('.titlebar .base');
		if (baseBtn) baseBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());

		const miniBtn = root.querySelector('.titlebar .mini');
		if (miniBtn) {
			miniBtn.addEventListener('click', () => {
				const panel = root.querySelector('.panel');
				if (panel) panel.style.display = panel.style.display === 'none' ? '' : 'none';
			});
		}

		const closeBtn = root.querySelector('.titlebar .close');
		if (closeBtn) {
			closeBtn.addEventListener('click', () => {
				Component._host.style.display = 'none';
				Renderer.stop(renderCharacter);
				hideStatus();
			});
		}

		const removeOptBtn = root.querySelector('.removeOption');
		if (removeOptBtn) removeOptBtn.addEventListener('mousedown', onRemoveOption);
		const viewStatusBtn = root.querySelector('.view_status');
		if (viewStatusBtn) viewStatusBtn.addEventListener('mousedown', toggleStatus);
		const showEquipBtn = root.querySelector('.show_equip');
		if (showEquipBtn) showEquipBtn.addEventListener('mousedown', toggleEquip);
		if (costumeConfig) {
			const showCostumeBtn = root.querySelector('.show_costume');
			if (showCostumeBtn) showCostumeBtn.addEventListener('mousedown', toggleCostume);
		}
		const cartBtn = root.querySelector('.cartitems');
		if (cartBtn) cartBtn.addEventListener('click', onCartItems);
		if (switchEquip) {
			const switchEquipBtn = root.querySelector('.switch_equip');
			if (switchEquipBtn) switchEquipBtn.addEventListener('click', onSwtichEquip);
		}

		if (titles) {
			this.loadTitles();
		}

		this._host.addEventListener('dragover', onDragOver);
		this._host.addEventListener('dragleave', onDragLeave);
		this._host.addEventListener('drop', onDrop);

		const content = root.querySelector('.content');
		if (content) {
			content.addEventListener('contextmenu', e => {
				e.preventDefault();
				const item = e.target.closest('.item');
				if (item) onEquipmentInfo.call(item, e);
			});
			content.addEventListener('dblclick', e => {
				const item = e.target.closest('.item');
				if (item) onEquipmentUnEquip.call(item, e);
			});
			content.addEventListener('mouseover', e => {
				const btn = e.target.closest('button');
				if (btn) onEquipmentOver.call(btn, e);
			});
			content.addEventListener('mouseout', e => {
				const btn = e.target.closest('button');
				if (btn) onEquipmentOut();
			});
		}

		this.draggable('.titlebar');

		if (switchEquip) {
			switchappend = root.querySelector('.footer');
		}

		if (costumeConfig) {
			const costumeBtn2 = root.querySelector('.show_costume');
			if (costumeBtn2) costumeBtn2.style.display = 'none';
			const costumeSpan = costumeBtn2 ? costumeBtn2.nextElementSibling : null;
			if (costumeSpan && costumeSpan.tagName === 'SPAN') costumeSpan.style.display = 'none';
		}

		if (damageSkin) {
			// Damage Skin Settings
			const skinButtons = root.querySelectorAll('#damageskin .skin-option');
			skinButtons.forEach(btn => {
				btn.setAttribute('data-background', 'showdamage/btn_damage.bmp');
				btn.setAttribute('data-hover', 'showdamage/btn_damage_press.bmp');
				btn.setAttribute('data-down', 'showdamage/btn_damage_pick.bmp');
			});
			if (this.parseHTML) {
				skinButtons.forEach(btn => {
					this.parseHTML.call(btn);
				});
			}
			skinButtons.forEach(btn => {
				btn.addEventListener('mousedown', function () {
					const skinId = parseInt(this.getAttribute('data-skin'), 10);
					Component.setDamageSkin(skinId);
				});
			});
			let savedSkin = GraphicsSettings.damageSkin;
			savedSkin = savedSkin !== undefined && savedSkin !== null ? savedSkin : 0;
			Component.setDamageSkin(savedSkin);

			// Damage Motion Settings
			const motionChecks = root.querySelectorAll('.motion-check');
			if (this.parseHTML) {
				motionChecks.forEach(btn => {
					this.parseHTML.call(btn);
				});
			}
			motionChecks.forEach(btn => {
				btn.addEventListener('mousedown', function () {
					const motionId = parseInt(this.getAttribute('data-motion'), 10);
					Component.setDamageMotion(motionId);
				});
			});
			let savedMotion = GraphicsSettings.damageMotion;
			savedMotion = savedMotion !== undefined && savedMotion !== null ? savedMotion : 0;
			Component.setDamageMotion(savedMotion);
		}
	};

	function showTab() {
		const selectedId = getHash(this.getAttribute('href'));
		const root = Component.getRoot();

		for (const id in contentDivs) {
			if (id === selectedId) {
				tabLinks[id].className = 'tab selected';
				if (contentDivs[id]) contentDivs[id].className = 'content';
			} else {
				tabLinks[id].className = 'tab';
				if (contentDivs[id]) contentDivs[id].classList.add('content', 'hide');
			}
		}

		currentTabId = selectedId;

		if (switchEquip) {
			if (SwitchEquip.ui) {
				SwitchEquip.showSwapTab(currentTabId);
			}

			const nonDefaultTab = costumeConfig ? currentTabId !== 'general' : currentTabId === 'title';
			if (nonDefaultTab) {
				if (SwitchEquip.ui) {
					const switchHost = SwitchEquip._host || SwitchEquip.ui;
					switchUIopen = switchHost.style ? switchHost.style.display !== 'none' : false;
					if (switchHost.style) switchHost.style.display = 'none';
				}
				const switchBtn = root.querySelector('.switch_equip');
				if (switchBtn) switchBtn.style.display = 'none';
			} else {
				if (SwitchEquip.ui && switchUIopen) {
					const switchHost = SwitchEquip._host || SwitchEquip.ui;
					if (switchHost.style) switchHost.style.display = '';
				}
				const switchBtn = root.querySelector('.switch_equip');
				if (switchBtn) switchBtn.style.display = '';
			}

			if (costumeConfig) {
				if (currentTabId !== 'general') {
					const showEquipEl = root.querySelector('.show_equip');
					if (showEquipEl) showEquipEl.style.display = 'none';
					const showEquipSpan = showEquipEl ? showEquipEl.nextElementSibling : null;
					if (showEquipSpan && showEquipSpan.tagName === 'SPAN') showEquipSpan.style.display = 'none';

					if (currentTabId === 'costume') {
						const costumeEl = root.querySelector('.show_costume');
						if (costumeEl) costumeEl.style.display = '';
						const costumeSpan = costumeEl ? costumeEl.nextElementSibling : null;
						if (costumeSpan && costumeSpan.tagName === 'SPAN') costumeSpan.style.display = '';
					} else {
						const costumeEl = root.querySelector('.show_costume');
						if (costumeEl) costumeEl.style.display = 'none';
						const costumeSpan = costumeEl ? costumeEl.nextElementSibling : null;
						if (costumeSpan && costumeSpan.tagName === 'SPAN') costumeSpan.style.display = 'none';
					}
				} else {
					const showEquipEl = root.querySelector('.show_equip');
					if (showEquipEl) showEquipEl.style.display = '';
					const showEquipSpan = showEquipEl ? showEquipEl.nextElementSibling : null;
					if (showEquipSpan && showEquipSpan.tagName === 'SPAN') showEquipSpan.style.display = '';

					const costumeEl = root.querySelector('.show_costume');
					if (costumeEl) costumeEl.style.display = 'none';
					const costumeSpan = costumeEl ? costumeEl.nextElementSibling : null;
					if (costumeSpan && costumeSpan.tagName === 'SPAN') costumeSpan.style.display = 'none';
				}
			}
		}

		return false;
	}

	Component.getCurrentTabId = function () {
		return currentTabId;
	};

	function onCartItems() {
		if (Session.Entity.hasCart === false) return;
		if (CartItems._host) {
			CartItems._host.style.display = CartItems._host.style.display === 'none' ? '' : 'none';
		}
	}

	function onRemoveOption() {
		const pkt = new PACKET.CZ.REQ_CARTOFF();
		Network.sendPacket(pkt);
	}

	Component.onAppend = function onAppend() {
		const hostRect = this._host.getBoundingClientRect();
		this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - hostRect.height)}px`;
		this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - hostRect.width)}px`;

		if (!_preferences.show) {
			this._host.style.display = 'none';
		}

		if (_preferences.reduce) {
			const root = Component.getRoot();
			const panel = root.querySelector('.panel');
			if (panel) panel.style.display = 'none';
		}

		if (UIVersionManager.getEquipmentVersion() > 0) {
			if (_preferences.stats && _preferences.show) {
				const winStats = WinStats.getUI();
				winStats.embed(Component._host);
			} else {
				Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/viewon.bmp', data => {
					const root = Component.getRoot();
					const btn = root.querySelector('.view_status');
					if (btn) btn.style.backgroundImage = `url(${data})`;
				});
			}
		}

		const root = Component.getRoot();
		const canvas = root.querySelector('canvas');
		if (canvas && this._host.style.display !== 'none') {
			Renderer.render(renderCharacter);
		}

		if (switchEquip) {
			SwitchEquip.append(switchappend);
			if (SwitchEquip.ui) {
				const switchHost = SwitchEquip._host || SwitchEquip.ui;
				if (switchHost.style) switchHost.style.display = 'none';
			}
		}
	};

	Component.onRemove = function onRemove() {
		if (UIVersionManager.getEquipmentVersion() > 0 && _btnLevelUp && _btnLevelUp.parentNode) {
			_btnLevelUp.remove();
		}

		Renderer.stop(renderCharacter);

		_list = {};
		const root = Component.getRoot();
		root.querySelectorAll('.col1, .col3, .ammo').forEach(el => {
			el.innerHTML = '';
		});

		_preferences.show = this._host.style.display !== 'none';
		const panel = root.querySelector('.panel');
		_preferences.reduce = panel ? panel.style.display === 'none' : false;
		const winStats = WinStats.getUI();
		_preferences.stats = winStats.isEmbedded();
		hideStatus();
		_preferences.y = parseInt(this._host.style.top, 10);
		_preferences.x = parseInt(this._host.style.left, 10);
		_preferences.save();
	};

	Component.toggle = function toggle() {
		if (this._host.style.display === 'none') {
			this._host.style.display = '';
			Renderer.render(renderCharacter);
			if (UIVersionManager.getEquipmentVersion() > 0) {
				if (_btnLevelUp && _btnLevelUp.parentNode) _btnLevelUp.remove();
				if (_preferences.stats) {
					WinStats.getUI().embed(Component._host);
				}
			}
			this.focus();
		} else {
			this._host.style.display = 'none';
			Renderer.stop(renderCharacter);
			hideStatus();
		}
	};

	Component.onShortCut = function onShurtCut(key) {
		switch (key.cmd) {
			case 'TOGGLE':
				this.toggle();
				break;
		}
	};

	Component.setEquipConfig = function setEquipConfig(on) {
		_showEquip = on;
		Client.loadFile(DB.INTERFACE_PATH + 'checkbox_' + (on ? '1' : '0') + '.bmp', data => {
			const root = Component.getRoot();
			const btn = root.querySelector('.show_equip');
			if (btn) btn.style.backgroundImage = `url(${data})`;
		});
	};

	if (costumeConfig) {
		Component.setCostumeConfig = function setCostumeConfig(on) {
			_hideCostume = on;
			Client.loadFile(DB.INTERFACE_PATH + 'checkbox_' + (on ? '0' : '1') + '.bmp', data => {
				const root = Component.getRoot();
				const btn = root.querySelector('.show_costume');
				if (btn) btn.style.backgroundImage = `url(${data})`;
			});
		};
	} else {
		Component.setCostumeConfig = function setCostumeConfig(_on) {};
	}

	Component.equip = function equip(item, location) {
		const it = DB.getItemInfo(item.ITID);
		if (entityRender) {
			item.equipped = location;
		}
		_list[item.index] = item;

		if (!entityRender && arguments.length === 1) {
			if ('WearState' in item) {
				location = item.WearState;
			} else if ('location' in item) {
				location = item.location;
			}
		}

		function add3Dots(string, limit) {
			function stripHTML(str) {
				const div = document.createElement('div');
				div.innerHTML = str;
				return div.textContent || div.innerText || '';
			}
			const text = stripHTML(string);
			if (text.length > limit) return text.substring(0, limit) + '...';
			return text;
		}

		const root = Component.getRoot();
		const selector = getSelectorFromLocation(location);
		const gradeInner = enchantGrade ? '<div class="grade"></div>' : '';
		root.querySelectorAll(selector).forEach(cell => {
			cell.innerHTML =
				'<div class="item" data-index="' +
				item.index +
				'">' +
				'<button>' +
				gradeInner +
				'</button>' +
				'<span class="itemName">' +
				escapeHTML(
					add3Dots(
						DB.getItemName(item, { showItemGrade: false, showItemSlots: false, showItemOptions: false }),
						25
					)
				) +
				'</span>' +
				'</div>';
		});

		Client.loadFile(DB.INTERFACE_PATH + 'item/' + it.identifiedResourceName + '.bmp', data => {
			const btns = root.querySelectorAll(`.item[data-index="${item.index}"] button`);
			btns.forEach(btn => {
				btn.style.backgroundImage = `url(${data})`;
			});
		});

		if (enchantGrade && item.enchantgrade) {
			Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/grade_icon' + item.enchantgrade + '.bmp', data => {
				root.querySelectorAll(`.item[data-index="${item.index}"] .grade`).forEach(el => {
					el.style.backgroundImage = `url(${data})`;
				});
			});
		}

		if (!Inventory.getUI().equippedItems.includes(item.index)) {
			Inventory.getUI().equippedItems.push(item.index);
		}

		if (switchEquip && PACKETVER.value >= 20170621) {
			if (!Inventory.getUI().isInEquipSwitchList(location)) {
				SwitchEquip.equip(item, location, false);
			}
		}
	};

	Component.unEquip = function unEquip(index, location) {
		const selector = getSelectorFromLocation(location);
		const root = Component.getRoot();
		const item = _list[index];
		if (entityRender) {
			item.equipped = 0;
		}

		root.querySelectorAll(selector).forEach(el => {
			el.innerHTML = '';
		});
		delete _list[index];

		return item;
	};

	Component.onLevelUp = function onLevelUp() {
		if (UIVersionManager.getEquipmentVersion() > 0 && _btnLevelUp) {
			document.body.appendChild(_btnLevelUp);
		}
	};

	Component.checkEquipLoc = function checkEquipLoc(location) {
		if (!entityRender) {
			return 0;
		}
		for (const key in _list) {
			const equipMask = switchEquip ? _list[key].location : _list[key].equipped;
			if (equipMask & location) {
				return _list[key].wItemSpriteNumber;
			}
		}
		return 0;
	};

	function hideStatus() {
		const winStats = WinStats.getUI();
		if (winStats.isEmbedded()) {
			winStats.unembed();
		}
	}

	function toggleStatus() {
		const root = Component.getRoot();
		const self = root.querySelector('.view_status');
		const winStats = WinStats.getUI();
		const isVisible = winStats.isEmbedded();
		const state = isVisible ? 'on' : 'off';

		if (isVisible) {
			winStats.unembed();
			_preferences.stats = false;
		} else {
			winStats.embed(Component._host);
			_preferences.stats = true;
		}

		Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/view' + state + '.bmp', data => {
			if (self) self.style.backgroundImage = `url(${data})`;
		});
	}

	function toggleEquip() {
		Component.onConfigUpdate(0, !_showEquip ? 1 : 0);
	}

	function toggleCostume() {
		Component.onConfigUpdate(5, !_hideCostume ? 1 : 0);
	}

	function onSwtichEquip() {
		SwitchEquip.toggle();

		if (SwitchEquip.ui) {
			const switchHost = SwitchEquip._host || SwitchEquip.ui;
			if (switchHost.style) {
				switchHost.style.position = 'absolute';
				switchHost.style.top = '0';
				switchHost.style.left = '0';
				switchHost.style.zIndex = '100';
			}
		}
	}

	const renderCharacter = (function renderCharacterClosure() {
		let _lastState = 0;
		let _hasCart = 0;

		const _cleanColor = new Float32Array([1.0, 1.0, 1.0, 1.0]);
		const _savedColor = new Float32Array(4);
		const _animation = {
			tick: 0,
			frame: 0,
			repeat: true,
			play: true,
			next: false,
			delay: 0,
			save: false
		};

		const HasAttachmentState =
			StatusConst.EffectState.FALCON |
			StatusConst.EffectState.RIDING |
			StatusConst.EffectState.DRAGON1 |
			StatusConst.EffectState.DRAGON2 |
			StatusConst.EffectState.DRAGON3 |
			StatusConst.EffectState.DRAGON4 |
			StatusConst.EffectState.DRAGON5 |
			StatusConst.EffectState.MADOGEAR |
			StatusConst.EffectState.CART1 |
			StatusConst.EffectState.CART2 |
			StatusConst.EffectState.CART3 |
			StatusConst.EffectState.CART4 |
			StatusConst.EffectState.CART5;

		const HasCartState =
			StatusConst.EffectState.CART1 |
			StatusConst.EffectState.CART2 |
			StatusConst.EffectState.CART3 |
			StatusConst.EffectState.CART4 |
			StatusConst.EffectState.CART5;

		function updateAttachmentButtons() {
			if (Session.Entity.effectState !== _lastState || _hasCart !== Session.Entity.hasCart) {
				_lastState = Session.Entity.effectState;
				_hasCart = Session.Entity.hasCart;

				const root = Component.getRoot();
				const removeOpt = root.querySelector('.removeOption');
				const cartBtn = root.querySelector('.cartitems');

				if (_lastState & HasAttachmentState || _hasCart) {
					if (removeOpt) removeOpt.style.display = '';
				} else {
					if (removeOpt) removeOpt.style.display = 'none';
				}

				if (_lastState & HasCartState || _hasCart) {
					if (cartBtn) cartBtn.style.display = '';
				} else {
					if (cartBtn) cartBtn.style.display = 'none';
				}
			}
		}

		function renderLegacy() {
			const character = Session.Entity;
			const direction = character.direction;
			const headDir = character.headDir;
			const action = character.action;
			const animation = character.animation;

			updateAttachmentButtons();

			Camera.direction = 4;
			character.direction = 4;
			character.headDir = 0;
			character.action = character.ACTION.IDLE;
			character.animation = _animation;

			_savedColor.set(character.effectColor);
			character.effectColor.set(_cleanColor);

			SpriteRenderer.bind2DContext(_ctx[0], 30, 130);
			_ctx[0].clearRect(0, 0, _ctx[0].canvas.width, _ctx[0].canvas.height);
			character.renderEntity();

			character.direction = direction;
			character.headDir = headDir;
			character.action = action;
			character.animation = animation;
			character.effectColor.set(_savedColor);
		}

		function renderEntity() {
			const equip_character = new Entity();
			equip_character.set({
				GID: Session.Entity.GID + '_EQUIP',
				objecttype: equip_character.constructor.TYPE_PC,
				job: Session.Entity.job,
				sex: Session.Entity.sex,
				name: '',
				hideShadow: true,
				head: Session.Entity.head,
				headpalette: Session.Entity.headpalette,
				bodypalette: Session.Entity.bodypalette
			});

			updateAttachmentButtons();

			if (currentTabId === 'general') {
				equip_character.accessory = Component.checkEquipLoc(EquipLocation.HEAD_BOTTOM);
				equip_character.accessory2 = Component.checkEquipLoc(EquipLocation.HEAD_TOP);
				equip_character.accessory3 = Component.checkEquipLoc(EquipLocation.HEAD_MID);
				equip_character.robe = Component.checkEquipLoc(EquipLocation.GARMENT);
			} else if (currentTabId === 'costume') {
				equip_character.accessory = Component.checkEquipLoc(EquipLocation.COSTUME_HEAD_BOTTOM);
				equip_character.accessory2 = Component.checkEquipLoc(EquipLocation.COSTUME_HEAD_TOP);
				equip_character.accessory3 = Component.checkEquipLoc(EquipLocation.COSTUME_HEAD_MID);
				equip_character.robe = Component.checkEquipLoc(EquipLocation.COSTUME_ROBE);
			}

			_savedColor.set(equip_character.effectColor);
			equip_character.effectColor.set(_cleanColor);

			Camera.direction = 0;
			equip_character.direction = 0;
			equip_character.headDir = 0;
			equip_character.action = equip_character.ACTION.IDLE;
			equip_character.animation = _animation;

			for (let i = 0; i < _ctx.length; i++) {
				const ctx = _ctx[i];
				SpriteRenderer.bind2DContext(ctx, 30, 130);
				ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
				equip_character.renderEntity(ctx);
			}
		}

		return function renderChar() {
			if (entityRender) {
				renderEntity();
			} else {
				renderLegacy();
			}
		};
	})();

	function onDragOver(event) {
		if (window._OBJ_DRAG_) {
			const data = window._OBJ_DRAG_;
			if (data.type === 'item') {
				const item = data.data;
				if (
					(item.type === ItemType.WEAPON ||
						item.type === ItemType.ARMOR ||
						item.type === ItemType.SHADOWGEAR) &&
					item.IsIdentified &&
					!item.IsDamaged
				) {
					const selector = getSelectorFromLocation('location' in item ? item.location : item.WearLocation);
					const root = Component.getRoot();
					const cells = root.querySelectorAll(selector);
					Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/item_invert.bmp', _data => {
						cells.forEach(c => {
							c.style.backgroundImage = `url(${_data})`;
						});
					});
				}
			}
		}
		event.stopImmediatePropagation();
		return false;
	}

	function onDragLeave(event) {
		const root = Component.getRoot();
		root.querySelectorAll('td').forEach(td => {
			td.style.backgroundImage = 'none';
		});
		event.stopImmediatePropagation();
		return false;
	}

	function onDrop(event) {
		let item, data;
		event.stopImmediatePropagation();

		try {
			data = JSON.parse(event.dataTransfer.getData('Text'));
		} catch (_e) {
			return false;
		}

		if (data && data.type === 'item') {
			item = data.data;
			if (
				(item.type === ItemType.WEAPON ||
					item.type === ItemType.ARMOR ||
					item.type === ItemType.AMMO ||
					item.type === ItemType.SHADOWGEAR) &&
				item.IsIdentified &&
				!item.IsDamaged
			) {
				const root = Component.getRoot();
				root.querySelectorAll('td').forEach(td => {
					td.style.backgroundImage = 'none';
				});
				Component.onEquipItem(item.index, 'location' in item ? item.location : item.WearState);
			}
		}

		return false;
	}

	function onEquipmentInfo(event) {
		const index = parseInt(this.getAttribute('data-index'), 10);
		const item = _list[index];

		if (item) {
			if (ItemInfo.uid === item.ITID) {
				ItemInfo.remove();
			} else {
				ItemInfo.append();
				ItemInfo.uid = item.ITID;
				ItemInfo.setItem(item);
			}
		}

		event.stopImmediatePropagation();
		return false;
	}

	function onEquipmentUnEquip() {
		const index = parseInt(this.getAttribute('data-index'), 10);
		Component.onUnEquip(index);
		const root = Component.getRoot();
		const overlay = root.querySelector('.overlay');
		if (overlay) overlay.style.display = 'none';
	}

	function onEquipmentOver() {
		const idx = parseInt(this.parentNode.getAttribute('data-index'), 10);
		const item = _list[idx];
		if (!item) return;

		const root = Component.getRoot();
		const overlay = root.querySelector('.overlay');
		const rootEl = root.querySelector('#' + name) || root;
		const btnRect = this.getBoundingClientRect();
		const rootRect = rootEl.getBoundingClientRect();
		const top = btnRect.top - rootRect.top;
		const left = btnRect.left - rootRect.left;
		if (!top && !left) return;

		if (overlay) {
			overlay.style.display = 'block';
			overlay.style.top = `${top - 22}px`;
			overlay.style.left = `${left - 22}px`;
			overlay.textContent = DB.getItemName(item);
		}
	}

	function onEquipmentOut() {
		const root = Component.getRoot();
		const overlay = root.querySelector('.overlay');
		if (overlay) overlay.style.display = 'none';
	}

	Component.onUpdateOwnerName = function () {
		const root = Component.getRoot();
		for (const index in _list) {
			const item = _list[index];
			if (item.slot && [0x00ff, 0x00fe, 0xff00].includes(item.slot.card1)) {
				root.querySelectorAll(`.item[data-index="${index}"] .itemName`).forEach(nameEl => {
					nameEl.textContent = DB.getItemName(item);
				});
			}
		}
	};

	Component.getNumber = function () {
		let num = 0;
		for (const key in _list) {
			if (_list[key].location && _list[key].location !== EquipLocation.AMMO) {
				num++;
			}
		}
		return num;
	};

	if (switchEquip) {
		Component.isInEquipList = function (data) {
			for (const key in _list) {
				if (_list[key].location & data) {
					return _list[key];
				}
			}
			return 0;
		};

		Component.equipItemsToSwitch = function () {
			const equipmentKeys = Object.keys(_list);
			for (let i = 0; i < equipmentKeys.length; i++) {
				const key = equipmentKeys[i];
				const equipmentItem = _list[key];
				if (equipmentItem.location) {
					SwitchEquip.equip(equipmentItem, equipmentItem.location, false);
				}
			}
		};
	} else {
		Component.isInEquipList = function () {
			return 0;
		};
	}

	if (titles) {
		Component.loadTitles = function () {
			const root = Component.getRoot();
			const titleList = root.querySelector('#title_list');
			if (!titleList) return;
			titleList.innerHTML = '';

			const removeTitleText = DB.getMessage(2686) || 'Remove Title';
			const removeSelectedClass = _currentTitleId === 0 ? ' selected' : '';
			const removeEl = document.createElement('div');
			removeEl.className = `title-option${removeSelectedClass}`;
			removeEl.setAttribute('data-title', '0');
			removeEl.textContent = removeTitleText;
			titleList.appendChild(removeEl);

			const allTitles = DB.getAllTitles();
			for (const titleId in allTitles) {
				if (allTitles.hasOwnProperty(titleId)) {
					const titleName = allTitles[titleId];
					const selectedClass = parseInt(titleId) === _currentTitleId ? ' selected' : '';
					const titleEl = document.createElement('div');
					titleEl.className = `title-option${selectedClass}`;
					titleEl.setAttribute('data-title', titleId);
					titleEl.textContent = titleName;
					titleList.appendChild(titleEl);
				}
			}

			titleList.addEventListener('click', e => {
				const option = e.target.closest('.title-option');
				if (option) {
					e.preventDefault();
					e.stopPropagation();
					const titleId = parseInt(option.getAttribute('data-title'));
					Component.selectTitle(titleId);
				}
			});
		};

		Component.selectTitle = function (titleId) {
			const pkt = new PACKET.CZ.REQ_CHANGE_TITLE();
			pkt.title_id = titleId;
			Network.sendPacket(pkt);
		};

		Component.setTitle = function OnSetTitle(titleId) {
			_currentTitleId = titleId;
			Component.loadTitles();
		};
	}

	if (damageSkin) {
		Component.setDamageSkin = function setDamageSkin(skinId) {
			const root = Component.getRoot();
			const buttons = root.querySelectorAll('#damageskin .skin-option');
			const buttonSelected = root.querySelector(`#damageskin .skin-option[data-skin="${skinId}"]`);

			GraphicsSettings.damageSkin = skinId;
			GraphicsSettings.save();

			buttons.forEach(btn => {
				btn.setAttribute('data-background', 'showdamage/btn_damage.bmp');
				btn.setAttribute('data-hover', 'showdamage/btn_damage_press.bmp');
				btn.setAttribute('data-down', 'showdamage/btn_damage_pick.bmp');
			});
			if (this.parseHTML) {
				buttons.forEach(btn => {
					this.parseHTML.call(btn);
				});
			}

			Client.loadFile(DB.INTERFACE_PATH + 'showdamage/btn_damage.bmp', data => {
				buttons.forEach(btn => {
					btn.style.backgroundImage = `url(${data})`;
				});
			});

			if (buttonSelected) {
				Client.loadFile(DB.INTERFACE_PATH + 'showdamage/btn_damage_pick.bmp', data => {
					buttonSelected.style.backgroundImage = `url(${data})`;
				});

				buttonSelected.onmouseover = null;
				buttonSelected.onmouseout = null;
			}
		};

		Component.setDamageMotion = function setDamageMotion(motionId) {
			GraphicsSettings.damageMotion = motionId;
			GraphicsSettings.save();

			const root = Component.getRoot();
			const checkboxes = root.querySelectorAll('.motion-check');

			checkboxes.forEach(btn => {
				const btnId = parseInt(btn.getAttribute('data-motion'), 10);
				const bgImage = btnId === motionId ? 'checkbox_1.bmp' : 'checkbox_0.bmp';
				Client.loadFile(DB.INTERFACE_PATH + bgImage, data => {
					btn.style.backgroundImage = `url(${data})`;
				});
			});
		};
	}

	Component.onUnEquip = function onUnEquip(/* index */) {};
	Component.onConfigUpdate = function onConfigUpdate(/* type, value*/) {};
	Component.onEquipItem = function onEquipItem(/* index, location */) {};
	Component.onRemoveCart = function onRemoveCart() {};

	return UIManager.addComponent(Component);
}
