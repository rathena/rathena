/**
 * UI/Components/PlayerViewEquip/PlayerViewEquipCommon.js
 *
 * Shared factory for all PlayerViewEquip versions (V0, V1, V2).
 * Migrated to GUIComponent with native DOM APIs.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */
import DB from 'DB/DBManager.js';
import EquipLocation from 'DB/Items/EquipmentLocation.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import Camera from 'Renderer/Camera.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Entity from 'Renderer/Entity/Entity.js';

/**
 * Escape HTML special characters
 */
function escapeHTML(str) {
	const div = document.createElement('div');
	div.appendChild(document.createTextNode(str));
	return div.innerHTML;
}

/**
 * Truncate string with ellipsis
 */
function add3Dots(string, limit) {
	if (string.length > limit) {
		return string.substring(0, limit) + '...';
	}
	return string;
}

/**
 * Generate the general equipment table HTML
 */
function generateGeneralTable() {
	return `  
		<table class="vieweqcontent" id="vieweqgeneral" data-background="basic_interface/equipwin_bg.bmp">  
			<tr>  
				<td class="head_top col1"></td>  
				<td rowspan="6">  
					<div class="col2 ammo_container">  
						<canvas width="55" height="125"></canvas>  
					</div>  
				</td>  
				<td class="head_mid col3"></td>  
			</tr>  
			<tr>  
				<td class="head_bottom col1"></td>  
				<td class="armor col3"></td>  
			</tr>  
			<tr>  
				<td class="weapon col1"></td>  
				<td class="shield col3"></td>  
			</tr>  
			<tr>  
				<td class="garment col1"></td>  
				<td class="shoes col3"></td>  
			</tr>  
			<tr>  
				<td class="accessory1 col1"></td>  
				<td class="accessory2 col3"></td>  
			</tr>  
		</table>`;
}

/**
 * Generate the costume equipment table HTML
 */
function generateCostumeTable(costumeRows, costumeTableBg) {
	let rows = '';
	for (let i = 0; i < costumeRows.length; i++) {
		const { left: col1, right: col3 } = costumeRows[i];
		if (i === 0) {
			rows += `  
			<tr>  
				<td class="${col1} col1"></td>  
				<td rowspan="6">  
					<div class="col2 ammo_container">  
						<canvas width="55" height="125"></canvas>  
					</div>  
				</td>  
				<td class="${col3} col3"></td>  
			</tr>`;
		} else {
			rows += `  
			<tr>  
				<td class="${col1} col1"></td>  
				<td class="${col3} col3"></td>  
			</tr>`;
		}
	}

	return `  
		<table class="vieweqcontent" id="vieweqcostume" data-background="${costumeTableBg}">  
			${rows}  
		</table>`;
}

/**
 * Generate the full component HTML
 */
function generateHTML(hasTabs, costumeRows, costumeTableBg) {
	const tabsHTML = hasTabs
		? `  
	<div class="vieweqtab-manager" id="vieweqtabs">  
		<div class="vieweqtab" id="vieweqgentab">  
			<a href="#vieweqgeneral"><span data-text="3158">General</span></a>  
		</div>  
		<div class="vieweqtab" id="vieweqcostab">  
			<a href="#vieweqcostume"><span data-text="3159">Costume</span></a>  
		</div>  
	</div>`
		: '';

	const costumeHTML = hasTabs ? generateCostumeTable(costumeRows, costumeTableBg) : '';

	return `<div id="PlayerViewEquip" data-repload="basic_interface/item_invert.bmp">  
	<div class="titlebar" data-background="basic_interface/titlebar_mid.bmp">  
		<div class="left">  
			<button  
				class="base"  
				data-background="basic_interface/sys_base_off.bmp"  
				data-hover="basic_interface/sys_base_on.bmp"  
			></button>  
			<span class="PlayerName"></span>  
		</div>  
		<div class="right">  
			<button  
				class="base mini"  
				data-background="basic_interface/sys_mini_off.bmp"  
				data-hover="basic_interface/sys_mini_on.bmp"  
			></button>  
			<button  
				class="base close"  
				data-background="basic_interface/sys_close_off.bmp"  
				data-hover="basic_interface/sys_close_on.bmp"  
			></button>  
		</div>  
		<div class="clear"></div>  
	</div>  
	<div class="overlay"></div>  
	${tabsHTML}  
	<div class="panel">  
		${generateGeneralTable()}  
		${costumeHTML}  
	</div>  
</div>`;
}

/**
 * Map EquipLocation bitmask to CSS selectors
 */
function getSelectorFromLocation(location) {
	const selector = [];

	// Basic equip
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

	// Costume / Shadow
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

/**
 * Factory function to create a PlayerViewEquip component
 *
 * @param {Object} config
 * @param {string} config.name - Component name
 * @param {string} config.cssText - CSS text
 * @param {boolean} config.hasTabs - Whether to show General/Costume tabs
 * @param {Array|null} config.costumeRows - [[col1Class, col3Class], ...] for costume table
 * @param {string|null} config.costumeTableBg - Costume table background image path
 */
export function createPlayerViewEquip({ name, cssText, hasTabs, costumeRows, costumeTableBg }) {
	const Component = new GUIComponent(name, cssText);

	Component.render = () => generateHTML(hasTabs, costumeRows, costumeTableBg);

	const _preferences = Preferences.get(name, { x: 480, y: 200 }, 1.0);

	let _list = {};
	const _vieweqctx = [];

	// Tab state
	const tabLinks = {};
	const contentDivs = {};
	let currentTabId = 'vieweqgeneral';

	// Character rendering data
	let charName, jobID, headID, sexID, bodypalID, headpalID;

	// Cached DOM references
	let _root, _overlay, _panel;

	// ─── Init ──────────────────────────────────────────────

	Component.init = function init() {
		_root = this.getRoot();
		_overlay = _root.querySelector('.overlay');
		_panel = _root.querySelector('.panel');

		// Canvas contexts (clear first to avoid accumulation on re-init)
		_vieweqctx.length = 0;
		const canvases = _root.querySelectorAll('canvas');
		for (const canvas of canvases) {
			_vieweqctx.push(canvas.getContext('2d'));
		}

		// Tab setup
		if (hasTabs) {
			const tabListEl = _root.querySelector('#vieweqtabs');
			if (tabListEl) {
				const tabDivs = tabListEl.querySelectorAll('.vieweqtab');
				let i = 0;
				for (const tabDiv of tabDivs) {
					const link = tabDiv.querySelector('a');
					if (!link) continue;
					const href = link.getAttribute('href');
					const id = href.substring(href.lastIndexOf('#') + 1);
					tabLinks[id] = link;
					contentDivs[id] = _root.querySelector('#' + id);

					link.addEventListener('click', e => {
						e.preventDefault();
						showTab(id);
					});
					link.addEventListener('focus', function () {
						this.blur();
					});

					if (i === 0) {
						link.className = 'vieweqtab selected';
					}
					i++;
				}

				// Hide all content divs except the first
				let j = 0;
				for (const id in contentDivs) {
					if (contentDivs[id] && j !== 0) {
						contentDivs[id].className = 'vieweqcontent hide';
					}
					j++;
				}
			}
		}

		// Titlebar buttons — stop propagation so drag doesn't activate
		const baseBtns = _root.querySelectorAll('.titlebar .base');
		for (const btn of baseBtns) {
			btn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		}

		const miniBtn = _root.querySelector('.titlebar .mini');
		if (miniBtn) {
			miniBtn.addEventListener('click', () => {
				if (_panel) {
					_panel.style.display = _panel.style.display === 'none' ? '' : 'none';
				}
			});
		}

		const closeBtn = _root.querySelector('.titlebar .close');
		if (closeBtn) {
			closeBtn.addEventListener('click', () => {
				Component.remove();
			});
		}

		// Drag leave
		this._host.addEventListener('dragleave', e => {
			const tds = _root.querySelectorAll('td');
			for (const td of tds) {
				td.style.backgroundImage = 'none';
			}
			e.stopImmediatePropagation();
		});

		// Equipment item events (contextmenu, mouseover, mouseout)
		const contentTables = _root.querySelectorAll('.vieweqcontent');
		for (const table of contentTables) {
			// Right-click on item → show ItemInfo
			table.addEventListener('contextmenu', e => {
				const itemEl = e.target.closest('.item');
				if (!itemEl) return;

				const index = parseInt(itemEl.getAttribute('data-index'), 10);
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

				e.stopImmediatePropagation();
				e.preventDefault();
			});

			// Mouseover on button → show item name overlay
			table.addEventListener('mouseover', e => {
				const btn = e.target.closest('button');
				if (!btn) return;

				const itemEl = btn.closest('.item');
				if (!itemEl) return;

				const idx = parseInt(itemEl.getAttribute('data-index'), 10);
				const item = _list[idx];
				if (!item) return;

				const top = btn.offsetTop;
				const left = btn.offsetLeft;
				if (!top && !left) return;

				_overlay.style.display = '';
				_overlay.style.top = top - 22 + 'px';
				_overlay.style.left = left - 22 + 'px';
				_overlay.textContent = DB.getItemName(item);
			});

			// Mouseout on button → hide overlay
			table.addEventListener('mouseout', e => {
				const btn = e.target.closest('button');
				if (!btn) return;
				_overlay.style.display = 'none';
			});
		}

		this.draggable('.titlebar');
	};

	// ─── Show Tab ──────────────────────────────────────────

	function showTab(selectedId) {
		for (const id in contentDivs) {
			if (id === selectedId) {
				tabLinks[id].className = 'vieweqtab selected';
				contentDivs[id].className = 'vieweqcontent';
			} else {
				tabLinks[id].className = 'vieweqtab';
				contentDivs[id].className = 'vieweqcontent hide';
			}
		}
		currentTabId = selectedId;
	}

	// ─── onAppend ──────────────────────────────────────────

	Component.onAppend = function onAppend() {
		const rect = this._host.getBoundingClientRect();
		this._host.style.top = Math.min(Math.max(0, _preferences.y), Renderer.height - rect.height) + 'px';
		this._host.style.left = Math.min(Math.max(0, _preferences.x), Renderer.width - rect.width) + 'px';

		// Check if any canvas is visible before starting render
		const canvases = _root.querySelectorAll('canvas');
		let anyVisible = false;
		for (const canvas of canvases) {
			if (canvas.offsetParent !== null) {
				anyVisible = true;
				break;
			}
		}
		if (anyVisible) {
			Renderer.render(renderCharacter);
		}
	};

	// ─── onRemove ──────────────────────────────────────────

	Component.onRemove = function onRemove() {
		Renderer.stop(renderCharacter);

		// Reset tab state so next open starts on General tab
		currentTabId = 'vieweqgeneral';

		_list = {};
		const cells = _root.querySelectorAll('.col1, .col3, .ammo');
		for (const cell of cells) {
			cell.innerHTML = '';
		}

		_preferences.show = this._host.style.display !== 'none';
		_preferences.reduce = _panel ? _panel.style.display === 'none' : false;
		_preferences.y = parseInt(this._host.style.top, 10) || 0;
		_preferences.x = parseInt(this._host.style.left, 10) || 0;
		_preferences.save();
	};

	// ─── Equip ─────────────────────────────────────────────

	Component.equip = function equip(item, location) {
		const it = DB.getItemInfo(item.ITID);

		if (arguments.length === 1) {
			if ('WearState' in item) {
				location = item.WearState;
			} else if ('location' in item) {
				location = item.location;
			}
		}

		item.equipped = location;
		_list[item.index] = item;

		const selector = getSelectorFromLocation(location);
		if (!selector) return;

		const cells = _root.querySelectorAll(selector);
		for (const cell of cells) {
			cell.innerHTML =
				'<div class="item" data-index="' +
				item.index +
				'">' +
				'<button></button>' +
				'<span class="itemName">' +
				add3Dots(escapeHTML(DB.getItemName(item)), 19) +
				'</span>' +
				'</div>';
		}

		Client.loadFile(DB.INTERFACE_PATH + 'item/' + it.identifiedResourceName + '.bmp', function (data) {
			const btn = _root.querySelector('.item[data-index="' + item.index + '"] button');
			if (btn) {
				btn.style.backgroundImage = 'url(' + data + ')';
			}
		});
	};

	// ─── setEquipmentData ──────────────────────────────────

	Component.setEquipmentData = function setEquipmentData(equipmentData) {
		_list = {};
		const cells = _root.querySelectorAll('.col1, .col3, .ammo');
		for (const cell of cells) {
			cell.innerHTML = '';
		}

		equipmentData.forEach(function (item) {
			Component.equip(item);
		});
	};

	// ─── setTitleBar ───────────────────────────────────────

	Component.setTitleBar = function setTitleBar(characterName) {
		const nameEl = _root.querySelector('.PlayerName');
		if (nameEl) {
			nameEl.textContent = DB.getMessage(1361).replace('%s', characterName);
		}
	};

	// ─── setChar2Render ────────────────────────────────────

	Component.setChar2Render = function setChar2Render(pkt) {
		if (pkt) {
			charName = pkt.characterName;
			jobID = pkt.job;
			headID = pkt.head;
			sexID = pkt.sex;
			bodypalID = pkt.bodypalette;
			headpalID = pkt.headpalette;
		}
	};

	// ─── renderCharacter ───────────────────────────────────

	const renderCharacter = (function renderCharacterClosure() {
		const _cleanColor = new Float32Array([1.0, 1.0, 1.0, 1.0]);
		const _animation = {
			tick: 0,
			frame: 0,
			repeat: true,
			play: true,
			next: false,
			delay: 0,
			save: false
		};
		const show_character = new Entity();

		return function renderChar() {
			show_character.set({
				GID: charName + '_EQUIP',
				objecttype: show_character.constructor.TYPE_PC,
				job: jobID,
				sex: sexID,
				name: '',
				hideShadow: true,
				head: headID,
				headpalette: headpalID,
				bodypalette: bodypalID
			});

			if (hasTabs && currentTabId === 'vieweqcostume') {
				// Costume tab — show costume headgears
				show_character.accessory = Component.checkEquipLoc(EquipLocation.COSTUME_HEAD_BOTTOM);
				show_character.accessory2 = Component.checkEquipLoc(EquipLocation.COSTUME_HEAD_TOP);
				show_character.accessory3 = Component.checkEquipLoc(EquipLocation.COSTUME_HEAD_MID);
				show_character.robe = Component.checkEquipLoc(EquipLocation.COSTUME_ROBE);
			} else {
				// General tab (or no tabs) — show normal headgears
				show_character.accessory = Component.checkEquipLoc(EquipLocation.HEAD_BOTTOM);
				show_character.accessory2 = Component.checkEquipLoc(EquipLocation.HEAD_TOP);
				show_character.accessory3 = Component.checkEquipLoc(EquipLocation.HEAD_MID);
				show_character.robe = Component.checkEquipLoc(EquipLocation.GARMENT);
			}

			show_character.effectColor.set(_cleanColor);

			Camera.direction = 0;
			show_character.direction = 0;
			show_character.headDir = 0;
			show_character.action = show_character.ACTION.IDLE;
			show_character.animation = _animation;

			for (let i = 0; i < _vieweqctx.length; i++) {
				const ctx = _vieweqctx[i];
				SpriteRenderer.bind2DContext(ctx, 30, 130);
				ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);
				show_character.renderEntity(ctx);
			}
		};
	})();

	// ─── onUpdateOwnerName ─────────────────────────────────

	Component.onUpdateOwnerName = function () {
		for (const index in _list) {
			const item = _list[index];
			if (item.slot && [0x00ff, 0x00fe, 0xff00].includes(item.slot.card1)) {
				const nameEl = _root.querySelector('.item[data-index="' + index + '"] .itemName');
				if (nameEl) {
					nameEl.textContent = DB.getItemName(item);
				}
			}
		}
	};

	// ─── getNumber ─────────────────────────────────────────

	Component.getNumber = function () {
		let num = 0;
		for (const key in _list) {
			if (_list[key].location && _list[key].location !== EquipLocation.AMMO) {
				num++;
			}
		}
		return num;
	};

	// ─── checkEquipLoc ─────────────────────────────────────

	Component.checkEquipLoc = function checkEquipLoc(location) {
		for (const key in _list) {
			if (_list[key].equipped & location) {
				return _list[key].wItemSpriteNumber;
			}
		}
		return 0;
	};

	// ─── Abstract callbacks ────────────────────────────────

	Component.onConnectionRequest = function onConnectionRequest() {};
	Component.onExitRequest = function onExitRequest() {};

	return UIManager.addComponent(Component);
}
