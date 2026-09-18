/**
 * UI/Components/Enchant/Enchant.js
 *
 * Enchant UI Window
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import EffectDB from 'DB/Effects/EffectTable.js';
import EffectConst from 'DB/Effects/EffectConst.js';
import Camera from 'Renderer/Camera.js';
import Renderer from 'Renderer/Renderer.js';
import StrEffect from 'Renderer/Effects/StrEffect.js';
import WebGL from 'Utils/WebGL.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import Session from 'Engine/SessionStorage.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import KEYS from 'Controls/KeyEventHandler.js';
import htmlText from './Enchant.html?raw';
import cssText from './Enchant.css?raw';

const Enchant = new GUIComponent('Enchant', cssText);

Enchant.render = () => htmlText;

Enchant.captureKeyEvents = true;

function _root() {
	return Enchant._shadow || Enchant._host;
}

const EnchantState = {
	groupId: 0,
	group: null,
	item: null,
	itemIndex: 0,
	action: 'random',
	selectedSlot: null,
	selectedPerfect: null,
	selectedUpgradeSlot: null,
	pending: null,
	pendingLock: false
};

const EnchantAssets = {
	subpage: {
		normal: null,
		upgrade: null,
		reset: null
	},
	menuTabs: {
		random: null,
		perfect: null,
		upgrade: null,
		reset: null
	},
	actionButton: {
		normal: null,
		hover: null,
		down: null,
		disabled: null
	}
};

const DEFAULT_INTRO_DURATION_MS = 2000;

const EnchantEffectState = {
	introEndAt: 0,
	resultTimer: null,
	overlayTimer: null,
	overlayActive: false,
	overlayNode: null,
	canvasStyles: null,
	activeEffects: [],
	renderCallback: null,
	renderActive: false,
	overlayProgram: null,
	overlayBuffer: null,
	fallbackPosition: [0, 0, 0]
};

const EnchantEffectDurations = {};

const EnchantEffectGroups = {
	enchant: {
		intro: EffectConst.EF_UI_ENCHANT_INTRO_YELLOW,
		success: EffectConst.EF_UI_ENCHANT_SUCCESS,
		fail: EffectConst.EF_UI_ENCHANT_FAIL
	},
	upgrade: {
		intro: EffectConst.EF_UI_ENCHANT_INTRO_BLUE,
		success: EffectConst.EF_UI_ENCHANT_UP_SUCCESS,
		fail: EffectConst.EF_UI_ENCHANT_UP_FAIL
	},
	reset: {
		intro: EffectConst.EF_UI_ENCHANT_INTRO_GREEN,
		success: EffectConst.EF_UI_ENCHANT_RESET_SUCCESS,
		fail: EffectConst.EF_UI_ENCHANT_RESET_FAIL
	}
};

const ENCHANT_OVERLAY_ALPHA = 0.5;
const ENCHANT_OVERLAY_COLOR = new Float32Array([0, 0, 0, ENCHANT_OVERLAY_ALPHA]);

const EnchantEffectFog = {
	use: false,
	exist: true,
	far: 30,
	near: 180,
	factor: 1.0,
	color: new Float32Array([1, 1, 1])
};

let _effectStyleNode = null;

function clearState() {
	EnchantState.groupId = 0;
	EnchantState.group = null;
	EnchantState.item = null;
	EnchantState.itemIndex = 0;
	EnchantState.action = 'random';
	EnchantState.selectedSlot = null;
	EnchantState.selectedPerfect = null;
	EnchantState.selectedUpgradeSlot = null;
	EnchantState.pending = null;
	EnchantState.pendingLock = false;
}

function formatZeny(zeny) {
	return String(zeny || 0).replace(/\B(?=(\d{3})+(?!\d))/g, ',');
}

function getSlotKey(slotNum) {
	return 'card' + (slotNum + 1);
}

function getSlotValue(item, slotNum) {
	if (!item || !item.slot) {
		return 0;
	}
	return item.slot[getSlotKey(slotNum)] || 0;
}

function setSlotValue(item, slotNum, value) {
	if (!item) {
		return;
	}
	if (!item.slot) {
		item.slot = {};
	}
	item.slot[getSlotKey(slotNum)] = value;
}

function getBaseSlotCount(item) {
	const it = DB.getItemInfo(item.ITID);
	const slotCount = it && it.slotCount ? parseInt(it.slotCount, 10) : 0;
	return isNaN(slotCount) ? 0 : slotCount;
}

function getItemGrade(item) {
	return item.enchantgrade || item.grade || 0;
}

function hasRandomOptions(item) {
	if (!item || !item.Options) {
		return false;
	}
	for (let i = 1; i < item.Options.length; i++) {
		if (item.Options[i] && item.Options[i].index > 0) {
			return true;
		}
	}
	return false;
}

function isTargetItem(item, group) {
	if (!item || !group || !group.targetItems) {
		return false;
	}
	const baseName = DB.getBasefromItemID(item.ITID);
	for (let i = 0; i < group.targetItems.length; i++) {
		const target = group.targetItems[i];
		if (target.id && target.id === item.ITID) {
			return true;
		}
		if (!target.id && baseName && target.base === baseName) {
			return true;
		}
	}
	return false;
}

function getNextEnchantSlot(item, group) {
	const slotOrder = group && group.slotOrder && group.slotOrder.length ? group.slotOrder : [0, 1, 2, 3];
	const baseSlots = getBaseSlotCount(item);
	for (let i = 0; i < slotOrder.length; i++) {
		const slotNum = slotOrder[i];
		if (slotNum < baseSlots) {
			continue;
		}
		if (!getSlotValue(item, slotNum)) {
			return slotNum;
		}
	}
	return null;
}

function getUpgradeCandidates(item, group) {
	const candidates = [];
	const slotOrder = group.slotOrder && group.slotOrder.length ? group.slotOrder : [0, 1, 2, 3];
	const baseSlots = getBaseSlotCount(item);
	for (let i = 0; i < slotOrder.length; i++) {
		const slotNum = slotOrder[i];
		if (slotNum < baseSlots) {
			continue;
		}
		const slotData = group.slots[slotNum];
		if (!slotData || !slotData.upgrade) {
			continue;
		}
		const currentId = getSlotValue(item, slotNum);
		if (!currentId) {
			continue;
		}
		const baseName = DB.getBasefromItemID(currentId);
		const entry = baseName ? slotData.upgrade[baseName] : null;
		if (!entry) {
			continue;
		}
		candidates.push({
			slotNum: slotNum,
			baseName: baseName,
			entry: entry,
			currentId: currentId
		});
	}
	return candidates;
}

function setStatus(message, isError) {
	const root = _root();
	if (!root) {
		return;
	}
	const status = root.querySelector('.status');
	if (!status) {
		return;
	}
	status.textContent = message || '';
	status.classList.toggle('error', !!isError);
}

function setCaution(message) {
	const root = _root();
	if (!root) {
		return;
	}
	const caution = root.querySelector('.caution');
	if (caution) {
		caution.textContent = message || '';
	}
}

function showHoverOverlay(text, identified, target) {
	const root = _root();
	if (!root || !text) {
		return;
	}
	const overlay = root.querySelector('.overlay');
	if (!overlay) {
		return;
	}
	const hostRect = Enchant._host.getBoundingClientRect();
	const targetRect = target.getBoundingClientRect();
	overlay.textContent = text;
	Object.assign(overlay.style, {
		top: targetRect.top - hostRect.top + 'px',
		left: targetRect.left - hostRect.left + 35 + 'px'
	});
	overlay.classList.toggle('grey', !identified);
	overlay.style.display = 'block';
}

function hideHoverOverlay() {
	const root = _root();
	if (!root) {
		return;
	}
	const overlay = root.querySelector('.overlay');
	if (overlay) {
		overlay.style.display = 'none';
	}
}

function showItemInfo(item) {
	if (!item) {
		return false;
	}
	if (ItemInfo.uid === item.ITID) {
		ItemInfo.remove();
		return false;
	}
	ItemInfo.append();
	ItemInfo.uid = item.ITID;
	ItemInfo.setItem(item);
	return false;
}

function updateSubpageSkin() {
	const root = _root();
	if (!root) {
		return;
	}
	const target = root.querySelector('.subpage');
	if (!target) {
		return;
	}
	let skin = EnchantAssets.subpage.normal;
	if (EnchantState.action === 'upgrade' && EnchantAssets.subpage.upgrade) {
		skin = EnchantAssets.subpage.upgrade;
	} else if (EnchantState.action === 'reset' && EnchantAssets.subpage.reset) {
		skin = EnchantAssets.subpage.reset;
	}
	if (skin) {
		target.style.backgroundImage = `url(${skin})`;
	}
}

function updateMenuSkin() {
	const root = _root();
	if (!root) {
		return;
	}
	const target = root.querySelector('.action_tabs');
	if (!target) {
		return;
	}
	const skin = EnchantAssets.menuTabs[EnchantState.action] || EnchantAssets.menuTabs.random;
	if (skin) {
		target.style.backgroundImage = `url(${skin})`;
	}
}

function updateActionButtonSkin(button, enabled) {
	if (!button) {
		return;
	}
	if (!EnchantAssets.actionButton.normal) {
		return;
	}
	if (!enabled && EnchantAssets.actionButton.disabled) {
		button.style.backgroundImage = `url(${EnchantAssets.actionButton.disabled})`;
		return;
	}
	button.style.backgroundImage = `url(${EnchantAssets.actionButton.normal})`;
}

function bindActionButtonSkin() {
	const root = _root();
	if (!root) {
		return;
	}
	const button = root.querySelector('.action_btn');
	if (!button) {
		return;
	}
	button.addEventListener('mouseover', () => {
		if (!EnchantAssets.actionButton.hover || button.classList.contains('disabled')) {
			return;
		}
		button.style.backgroundImage = `url(${EnchantAssets.actionButton.hover})`;
	});
	button.addEventListener('mouseout', () => {
		if (button.classList.contains('disabled')) {
			return;
		}
		button.style.backgroundImage = `url(${EnchantAssets.actionButton.normal})`;
	});
	button.addEventListener('mousedown', () => {
		if (!EnchantAssets.actionButton.down || button.classList.contains('disabled')) {
			return;
		}
		button.style.backgroundImage = `url(${EnchantAssets.actionButton.down})`;
	});
	button.addEventListener('mouseup', () => {
		if (button.classList.contains('disabled')) {
			return;
		}
		const skin = EnchantAssets.actionButton.hover || EnchantAssets.actionButton.normal;
		button.style.backgroundImage = `url(${skin})`;
	});
}

function applyScrollSkin(up, down, bg, thumb) {
	const root = _root();
	if (!root) {
		return;
	}
	const inner = root.querySelector('#Enchant');
	if (!inner) {
		return;
	}
	inner.style.setProperty('--enchant-scroll-up', `url(${up})`);
	inner.style.setProperty('--enchant-scroll-down', `url(${down})`);
	inner.style.setProperty('--enchant-scroll-track', `url(${bg})`);
	inner.style.setProperty('--enchant-scroll-thumb', `url(${thumb})`);
}

function ensureEffectOverlay() {
	if (EnchantEffectState.overlayNode) {
		return EnchantEffectState.overlayNode;
	}
	const overlay = document.createElement('div');
	overlay.id = 'EnchantEffectOverlay';
	document.body.appendChild(overlay);
	EnchantEffectState.overlayNode = overlay;
	return overlay;
}

function getEffectAnchorPosition() {
	if (Camera && Camera.target && Camera.target.position) {
		return Camera.target.position;
	}
	if (Session.Entity && Session.Entity.position) {
		return Session.Entity.position;
	}
	return EnchantEffectState.fallbackPosition;
}

function ensureEffectRenderCallback() {
	if (EnchantEffectState.renderCallback) {
		return;
	}
	EnchantEffectState.renderCallback = function (tick, gl) {
		if (!EnchantEffectState.overlayActive && !EnchantEffectState.activeEffects.length) {
			return;
		}
		if (!gl) {
			return;
		}
		gl.bindFramebuffer(gl.FRAMEBUFFER, null);
		gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);

		if (EnchantEffectState.overlayActive) {
			renderEffectOverlay(gl);
		}
		if (EnchantEffectState.activeEffects.length) {
			renderEnchantEffects(tick, gl);
		} else {
			stopEffectRenderIfIdle();
		}
	};
}

function ensureEffectRenderActive() {
	ensureEffectRenderCallback();
	if (EnchantEffectState.renderActive) {
		return;
	}
	Renderer.render(EnchantEffectState.renderCallback);
	EnchantEffectState.renderActive = true;
}

function stopEffectRenderIfIdle() {
	if (!EnchantEffectState.renderActive) {
		return;
	}
	if (EnchantEffectState.overlayActive || EnchantEffectState.activeEffects.length) {
		return;
	}
	Renderer.stop(EnchantEffectState.renderCallback);
	EnchantEffectState.renderActive = false;
}

function ensureOverlayProgram(gl) {
	if (EnchantEffectState.overlayProgram && EnchantEffectState.overlayBuffer) {
		return;
	}
	const vertexShader =
		'\
#version 300 es\n\
#pragma vscode_glsllint_stage : vert\n\
precision highp float;\n\
in vec2 aPosition;\n\
void main() {\n\
	gl_Position = vec4(aPosition, 0.0, 1.0);\n\
}\n';
	const fragmentShader =
		'\
#version 300 es\n\
#pragma vscode_glsllint_stage : frag\n\
precision highp float;\n\
uniform vec4 uColor;\n\
out vec4 fragColor;\n\
void main() {\n\
	fragColor = uColor;\n\
}\n';
	EnchantEffectState.overlayProgram = WebGL.createShaderProgram(gl, vertexShader, fragmentShader);
	EnchantEffectState.overlayBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, EnchantEffectState.overlayBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, 1, -1, -1, 1, -1, 1, 1, -1, 1, 1]), gl.STATIC_DRAW);
}

function renderEffectOverlay(gl) {
	ensureOverlayProgram(gl);
	const program = EnchantEffectState.overlayProgram;
	if (!program) {
		return;
	}
	gl.useProgram(program);
	gl.bindBuffer(gl.ARRAY_BUFFER, EnchantEffectState.overlayBuffer);
	gl.enableVertexAttribArray(program.attribute.aPosition);
	gl.vertexAttribPointer(program.attribute.aPosition, 2, gl.FLOAT, false, 0, 0);
	gl.uniform4fv(program.uniform.uColor, ENCHANT_OVERLAY_COLOR);
	gl.disable(gl.DEPTH_TEST);
	gl.enable(gl.BLEND);
	gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	gl.drawArrays(gl.TRIANGLES, 0, 6);
}

function renderEnchantEffects(tick, gl) {
	if (!EnchantEffectState.activeEffects.length) {
		return;
	}
	if (!StrEffect.ready) {
		StrEffect.init(gl);
	}
	gl.disable(gl.DEPTH_TEST);
	StrEffect.beforeRender(gl, Camera.modelView, Camera.projection, EnchantEffectFog, tick);
	const anchor = getEffectAnchorPosition();
	const remaining = [];
	EnchantEffectState.activeEffects.forEach(function (effect) {
		if (!effect) {
			return;
		}
		if (!effect.ready) {
			effect.init(gl);
		}
		effect.position = anchor;
		effect.render(gl, tick);
		if (!effect.needCleanUp) {
			remaining.push(effect);
		}
	});
	EnchantEffectState.activeEffects = remaining;
	StrEffect.afterRender(gl);
	gl.enable(gl.DEPTH_TEST);
	stopEffectRenderIfIdle();
}

function spawnEnchantEffect(effectId) {
	if (!effectId) {
		return;
	}
	const effectList = EffectDB[effectId];
	if (!effectList || !effectList.length) {
		return;
	}
	const anchor = getEffectAnchorPosition();
	const startTick = Renderer && Renderer.tick ? Renderer.tick : Date.now();
	effectList.forEach(function (effect) {
		if (!effect || effect.type !== 'STR' || !effect.file) {
			return;
		}
		const filename = 'data/texture/effect/' + effect.file + '.str';
		const texturePath = effect.texturePath || '';
		const strEffect = new StrEffect(filename, anchor, startTick, texturePath);
		EnchantEffectState.activeEffects.push(strEffect);
	});
	ensureEffectRenderActive();
}

function ensureEffectStyle() {
	if (_effectStyleNode) {
		return;
	}
	_effectStyleNode = document.createElement('style');
	_effectStyleNode.textContent =
		'#EnchantEffectOverlay{position:fixed;top:0;left:0;width:100%;height:100%;background:transparent;z-index:1000;display:none;}' +
		'body.enchant-effect-active>*:not(canvas):not(.cursor):not(#EnchantEffectOverlay){filter:brightness(0.6);}';
	document.head.appendChild(_effectStyleNode);
}

function showEffectOverlay() {
	ensureEffectStyle();
	const overlay = ensureEffectOverlay();
	if (EnchantEffectState.overlayActive) {
		overlay.style.display = 'block';
		return;
	}
	overlay.style.display = 'block';
	EnchantEffectState.overlayActive = true;
	document.body.classList.add('enchant-effect-active');
	ensureEffectRenderActive();
}

function hideEffectOverlay() {
	if (EnchantEffectState.overlayTimer) {
		clearTimeout(EnchantEffectState.overlayTimer);
		EnchantEffectState.overlayTimer = null;
	}
	if (!EnchantEffectState.overlayActive) {
		return;
	}
	if (Renderer && Renderer.canvas && EnchantEffectState.canvasStyles) {
		Renderer.canvas.style.zIndex = EnchantEffectState.canvasStyles.zIndex;
		Renderer.canvas.style.opacity = EnchantEffectState.canvasStyles.opacity;
		Renderer.canvas.style.pointerEvents = EnchantEffectState.canvasStyles.pointerEvents;
	}
	if (EnchantEffectState.overlayNode) {
		EnchantEffectState.overlayNode.style.display = 'none';
	}
	EnchantEffectState.overlayActive = false;
	document.body.classList.remove('enchant-effect-active');
	stopEffectRenderIfIdle();
}

function scheduleEffectOverlayHide(delay) {
	if (EnchantEffectState.overlayTimer) {
		clearTimeout(EnchantEffectState.overlayTimer);
	}
	const timeout = Math.max(Number(delay) || 0, 0);
	EnchantEffectState.overlayTimer = setTimeout(() => {
		hideEffectOverlay();
	}, timeout);
}

function getEffectGroup(action) {
	if (action === 'upgrade') {
		return 'upgrade';
	}
	if (action === 'reset') {
		return 'reset';
	}
	return 'enchant';
}

function cacheEffectDuration(effectId) {
	if (Object.prototype.hasOwnProperty.call(EnchantEffectDurations, effectId)) {
		return;
	}
	const effectList = EffectDB[effectId];
	if (!effectList || !effectList.length) {
		EnchantEffectDurations[effectId] = DEFAULT_INTRO_DURATION_MS;
		return;
	}
	const effect = effectList[0];
	if (!effect || effect.type !== 'STR' || !effect.file) {
		EnchantEffectDurations[effectId] = DEFAULT_INTRO_DURATION_MS;
		return;
	}
	const filename = 'data/texture/effect/' + effect.file + '.str';
	const texturePath = effect.texturePath || '';
	Client.loadFile(
		filename,
		function (strFile) {
			let duration = DEFAULT_INTRO_DURATION_MS;
			if (strFile && strFile.fps) {
				duration = Math.ceil((strFile.maxKey / strFile.fps) * 1000);
			}
			EnchantEffectDurations[effectId] = duration;
		},
		null,
		{ texturePath: texturePath }
	);
}

function preloadEnchantEffectDurations() {
	Object.keys(EnchantEffectGroups).forEach(key => {
		const group = EnchantEffectGroups[key];
		cacheEffectDuration(group.intro);
		cacheEffectDuration(group.success);
		cacheEffectDuration(group.fail);
	});
}

function getEffectDuration(effectId) {
	return EnchantEffectDurations[effectId] || DEFAULT_INTRO_DURATION_MS;
}

function playEffect(effectId) {
	if (!effectId) {
		return;
	}
	spawnEnchantEffect(effectId);
}

function playIntroEffect(action) {
	const group = getEffectGroup(action);
	const effects = EnchantEffectGroups[group];
	if (!effects || !effects.intro) {
		return;
	}
	if (EnchantEffectState.resultTimer) {
		clearTimeout(EnchantEffectState.resultTimer);
		EnchantEffectState.resultTimer = null;
	}
	showEffectOverlay();
	cacheEffectDuration(effects.intro);
	playEffect(effects.intro);
	EnchantEffectState.introEndAt = Date.now() + getEffectDuration(effects.intro);
	scheduleEffectOverlayHide(getEffectDuration(effects.intro) + DEFAULT_INTRO_DURATION_MS);
}

function playResultEffect(action, success) {
	const group = getEffectGroup(action);
	const effects = EnchantEffectGroups[group];
	if (!effects) {
		return;
	}
	const effectId = success ? effects.success : effects.fail;
	if (!effectId) {
		return;
	}
	cacheEffectDuration(effectId);
	const delay = Math.max(EnchantEffectState.introEndAt - Date.now(), 0);
	if (EnchantEffectState.resultTimer) {
		clearTimeout(EnchantEffectState.resultTimer);
	}
	if (delay > 0) {
		EnchantEffectState.resultTimer = setTimeout(() => {
			playEffect(effectId);
		}, delay);
	} else {
		playEffect(effectId);
		EnchantEffectState.resultTimer = null;
	}
	scheduleEffectOverlayHide(delay + getEffectDuration(effectId));
	EnchantEffectState.introEndAt = 0;
}

function loadItemIcon(target, itemId, isIdentified) {
	const it = DB.getItemInfo(itemId);
	if (!it) {
		return;
	}
	const name = isIdentified !== false ? it.identifiedResourceName : it.unidentifiedResourceName;
	if (!name) {
		return;
	}
	Client.loadFile(DB.INTERFACE_PATH + 'item/' + name + '.bmp', data => {
		target.style.backgroundImage = `url(${data})`;
	});
}

function loadItemCollection(target, itemId, isIdentified) {
	const it = DB.getItemInfo(itemId);
	if (!it) {
		return;
	}
	const name = isIdentified !== false ? it.identifiedResourceName : it.unidentifiedResourceName;
	if (!name) {
		return;
	}
	Client.loadFile(
		DB.INTERFACE_PATH + 'collection/' + name + '.bmp',
		data => {
			target.style.backgroundImage = `url(${data})`;
		},
		() => {
			loadItemIcon(target, itemId, isIdentified);
		}
	);
}

function loadGradeIcon(target, grade) {
	if (!target) {
		return;
	}
	if (!grade || grade <= 0) {
		target.style.backgroundImage = '';
		return;
	}
	Client.loadFile(DB.INTERFACE_PATH + 'grade_enchant/grade_icon' + grade + '.bmp', data => {
		target.style.backgroundImage = `url(${data})`;
	});
}

function getItemDisplayName(itemId, fallback) {
	const info = DB.getItemInfo(itemId);
	if (info && info.identifiedDisplayName) {
		return info.identifiedDisplayName;
	}
	return fallback || 'Unknown';
}

function resolveMaterialId(material) {
	if (material && material.id) {
		return material.id;
	}
	if (material && material.base) {
		return DB.getItemIdfromBase(material.base) || 0;
	}
	return 0;
}

function hasEnoughZeny(zeny) {
	const cost = Number(zeny) || 0;
	if (cost <= 0) {
		return true;
	}
	return Session.zeny >= cost;
}

function hasEnoughMaterials(materials) {
	if (!materials || !materials.length) {
		return true;
	}
	const inventoryUI = Inventory.getUI && Inventory.getUI();
	if (!inventoryUI || !inventoryUI.getItemById) {
		return false;
	}
	for (let i = 0; i < materials.length; i++) {
		const mat = materials[i];
		const required = Number(mat.count) || 0;
		if (required <= 0) {
			continue;
		}
		const matId = resolveMaterialId(mat);
		if (!matId) {
			return false;
		}
		const inventoryItem = inventoryUI.getItemById(matId);
		const current = inventoryItem ? inventoryItem.count : 0;
		if (current < required) {
			return false;
		}
	}
	return true;
}

function canAffordCost(zeny, materials) {
	return hasEnoughZeny(zeny) && hasEnoughMaterials(materials);
}

function renderMaterials(materials) {
	const root = _root();
	if (!root) {
		return;
	}
	const list = root.querySelector('.material_list');
	if (!list) {
		return;
	}
	list.innerHTML = '';
	if (!materials || !materials.length) {
		return;
	}
	const inventoryUI = Inventory.getUI && Inventory.getUI();
	materials.forEach(mat => {
		const entry = document.createElement('div');
		entry.className = 'material';
		const icon = document.createElement('div');
		icon.className = 'icon';
		const nameEl = document.createElement('div');
		nameEl.className = 'name';
		const count = document.createElement('div');
		count.className = 'count';
		const matId = resolveMaterialId(mat);
		const label = matId ? getItemDisplayName(matId, mat.base) : mat.base || 'Unknown';
		let current = 0;
		const required = Number(mat.count) || 0;
		if (matId && inventoryUI && inventoryUI.getItemById) {
			const inventoryItem = inventoryUI.getItemById(matId);
			current = inventoryItem ? inventoryItem.count : 0;
		}
		nameEl.textContent = label;
		if (mat.count != null) {
			count.textContent = matId ? current + '/' + required : 'x' + required;
		}
		entry.dataset.background =
			matId && current >= required ? 'enchantui/bg_enc_on.bmp' : 'enchantui/bg_enc_off.bmp';
		entry.appendChild(icon);
		entry.appendChild(nameEl);
		entry.appendChild(count);
		list.appendChild(entry);
		GUIComponent.processDataAttrs(entry);
		icon.dataset.name = label;
		if (matId) {
			icon.dataset.itid = matId;
			loadItemIcon(icon, matId, true);
		}
	});
}

function renderEnchantList(entries, selectedKey) {
	const root = _root();
	if (!root) {
		return;
	}
	const list = root.querySelector('.enchant_list');
	if (!list) {
		return;
	}
	list.innerHTML = '';
	if (!entries || !entries.length) {
		return;
	}
	entries.forEach(entry => {
		const row = document.createElement('div');
		row.className = 'enchant_entry';
		const icon = document.createElement('div');
		icon.className = 'entry_icon';
		const text = document.createElement('div');
		text.className = 'entry_text';
		const name = document.createElement('div');
		name.className = 'entry_name';
		const itid = entry.id || (entry.base ? DB.getItemIdfromBase(entry.base) : 0);
		name.textContent = entry.label || '';
		text.appendChild(name);
		if (entry.subLabel) {
			const sub = document.createElement('div');
			sub.className = 'entry_sub';
			sub.textContent = entry.subLabel;
			text.appendChild(sub);
		}
		row.dataset.key = entry.key;
		row.dataset.itid = itid || 0;
		row.dataset.name = entry.label || '';
		row.dataset.background = 'enchantui/btn_enc_item.bmp';
		row.dataset.hover = 'enchantui/btn_enc_item_over.bmp';
		row.dataset.down = 'enchantui/btn_enc_item_press.bmp';
		row.dataset.active = 'enchantui/btn_enc_item_press.bmp';
		row.classList.toggle('disabled', !!entry.disabled);
		row.classList.toggle('active', selectedKey != null && String(entry.key) === String(selectedKey));
		row.appendChild(icon);
		row.appendChild(text);
		list.appendChild(row);
		GUIComponent.processDataAttrs(row);
		if (itid) {
			loadItemIcon(icon, itid, true);
		}
	});
}

function renderItemList() {
	const root = _root();
	if (!root) {
		return;
	}
	const list = root.querySelector('.item_list');
	if (!list) {
		return;
	}
	list.innerHTML = '';

	if (!EnchantState.group) {
		const empty = document.createElement('div');
		empty.className = 'item_list_empty';
		empty.textContent = 'Enchant data missing.';
		list.appendChild(empty);
		return;
	}

	const inventoryUI = Inventory.getUI && Inventory.getUI();
	const items = inventoryUI && inventoryUI.list ? inventoryUI.list : [];
	let selectedIndex = EnchantState.item ? EnchantState.item.index : 0;
	let selectedValid = false;
	const candidates = [];

	items.forEach(item => {
		const result = validateItem(item);
		if (!result.ok) {
			return;
		}
		candidates.push(item);
		if (selectedIndex && item.index === selectedIndex) {
			selectedValid = true;
		}
	});

	if (selectedIndex && !selectedValid) {
		EnchantState.item = null;
		EnchantState.itemIndex = 0;
		EnchantState.selectedPerfect = null;
		EnchantState.selectedSlot = null;
		EnchantState.selectedUpgradeSlot = null;
		selectedIndex = 0;
	}

	if (!candidates.length) {
		const empty = document.createElement('div');
		empty.className = 'item_list_empty';
		empty.textContent = 'No enchantable items.';
		list.appendChild(empty);
		return;
	}

	candidates.forEach(item => {
		const entry = document.createElement('button');
		entry.className = 'item_entry';
		const slot = document.createElement('div');
		slot.className = 'item_slot';
		const info = document.createElement('div');
		info.className = 'item_info';
		const name = document.createElement('div');
		name.className = 'item_name';
		const grade = document.createElement('div');
		grade.className = 'item_grade';
		const itemName = DB.getItemName(item, {
			showItemRefine: true,
			showItemGrade: false,
			showItemOptions: false
		});

		name.textContent = itemName;
		info.appendChild(name);
		info.appendChild(grade);

		entry.dataset.index = item.index;
		entry.dataset.name = itemName;
		entry.dataset.background = 'enchantui/bg_enc_gear.bmp';
		entry.dataset.hover = 'enchantui/bg_enc_gear_over.bmp';
		entry.dataset.down = 'enchantui/bg_enc_gear_press.bmp';
		entry.dataset.active = 'enchantui/bg_enc_gear_press.bmp';
		entry.classList.toggle('active', !!(selectedIndex && item.index === selectedIndex));
		entry.appendChild(slot);
		entry.appendChild(info);
		list.appendChild(entry);
		GUIComponent.processDataAttrs(entry);
		slot.dataset.itid = item.ITID;
		slot.dataset.name = itemName;
		loadItemIcon(slot, item.ITID, item.IsIdentified);
		loadGradeIcon(grade, getItemGrade(item));
	});
}

function renderItemPreview() {
	const root = _root();
	if (!root) {
		return;
	}
	const preview = root.querySelector('.preview_item');
	if (!preview) {
		return;
	}
	preview.style.backgroundImage = '';
	if (!EnchantState.item) {
		return;
	}
	loadItemCollection(preview, EnchantState.item.ITID, EnchantState.item.IsIdentified);
}

function renderSlots() {
	const root = _root();
	if (!root) {
		return;
	}
	const list = root.querySelector('.slot_list');
	if (!list) {
		return;
	}
	let empty = list.querySelector('.slot_empty');
	if (!empty) {
		empty = document.createElement('div');
		empty.className = 'slot_empty';
		empty.textContent = 'Select an item';
		list.appendChild(empty);
	}
	const hasItem = !!(EnchantState.item && EnchantState.group);
	empty.style.display = hasItem ? 'none' : 'flex';

	const baseSlots = hasItem ? getBaseSlotCount(EnchantState.item) : 0;
	list.querySelectorAll('.slot_entry').forEach(entry => {
		const slotNum = parseInt(entry.dataset.slot, 10);
		const icon = entry.querySelector('.slot_icon');
		const slotItem = hasItem ? getSlotValue(EnchantState.item, slotNum) : 0;
		entry.classList.toggle('locked', hasItem && slotNum < baseSlots);
		entry.classList.toggle('active', hasItem && slotNum === EnchantState.selectedSlot);
		entry.classList.toggle('empty', !slotItem);
		entry.dataset.itid = slotItem || 0;
		if (icon) {
			icon.style.backgroundImage = '';
		}
		if (slotItem && icon) {
			loadItemIcon(icon, slotItem, true);
		}
	});
}

function updateTabs(availability) {
	const root = _root();
	if (!root) {
		return;
	}
	root.querySelectorAll('.action_tabs .tab').forEach(tab => {
		const action = tab.dataset.action;
		tab.classList.toggle('active', EnchantState.action === action);
		tab.classList.toggle('disabled', availability && availability[action] === false);
	});
	updateSubpageSkin();
	updateMenuSkin();
}

function updateActionSections() {
	const root = _root();
	if (!root) {
		return;
	}
	root.querySelectorAll('.action_section').forEach(section => {
		section.classList.remove('active');
	});
	const activeSection = root.querySelector('.' + EnchantState.action + '_section');
	if (activeSection) {
		activeSection.classList.add('active');
	}
}

function updateActionButton(enabled) {
	const root = _root();
	if (!root) {
		return;
	}
	const button = root.querySelector('.action_btn');
	if (!button) {
		return;
	}
	const label = EnchantState.action === 'reset' ? 'Reset' : 'Enchant';
	button.textContent = '';
	button.title = label;
	button.classList.toggle('disabled', !enabled);
	updateActionButtonSkin(button, enabled);
}

function renderCosts(rate, zeny, materials) {
	const root = _root();
	if (!root) {
		return;
	}
	const zenyText = zeny != null ? formatZeny(zeny) : '';
	const zenyCost = root.querySelector('.zeny_cost');
	if (zenyCost) {
		zenyCost.textContent = zenyText;
	}
	renderMaterials(materials);
}

function refreshActionContent() {
	const root = _root();
	if (!root) {
		return;
	}
	const group = EnchantState.group;
	const item = EnchantState.item;
	let actionReady = false;
	const availability = {
		random: false,
		perfect: false,
		upgrade: false,
		reset: false
	};
	let listEntries = [];
	let selectedKey = null;

	const upgradeResult = root.querySelector('.upgrade_result');
	if (upgradeResult) {
		upgradeResult.textContent = '';
	}

	if (group && item) {
		const slotNum = getNextEnchantSlot(item, group);
		const slotData = slotNum !== null ? group.slots[slotNum] : null;
		const baseSlots = getBaseSlotCount(item);

		if (slotData && slotData.random && Object.keys(slotData.random).length) {
			availability.random = true;
		}
		if (slotData && slotData.perfect && Object.keys(slotData.perfect).length) {
			availability.perfect = true;
		}

		const upgradeCandidates = getUpgradeCandidates(item, group);
		if (upgradeCandidates.length) {
			availability.upgrade = true;
		}

		let hasEnchant = false;
		const slotOrder = group.slotOrder && group.slotOrder.length ? group.slotOrder : [0, 1, 2, 3];
		slotOrder.forEach(orderSlot => {
			if (orderSlot >= baseSlots && getSlotValue(item, orderSlot)) {
				hasEnchant = true;
			}
		});
		if (group.reset && group.reset.enabled && hasEnchant) {
			availability.reset = true;
		}

		if (!availability[EnchantState.action]) {
			const nextAction = Object.keys(availability).find(key => availability[key]);
			EnchantState.action = nextAction || EnchantState.action;
		}

		updateTabs(availability);
		updateActionSections();

		if (EnchantState.action === 'random') {
			EnchantState.selectedSlot = slotNum;
			if (slotData) {
				const grade = getItemGrade(item);
				const bonus = slotData.gradeBonus ? slotData.gradeBonus[grade] || 0 : 0;
				const baseRate = slotData.successRate || 0;
				const totalRate = Math.min(baseRate + bonus, 100000);
				const require = slotData.require || { zeny: 0, materials: [] };
				renderCosts(totalRate, require.zeny, require.materials);
				actionReady = availability.random && canAffordCost(require.zeny, require.materials);
				if (slotData.random) {
					const randomList = slotData.random[grade] || slotData.random[0] || [];
					listEntries = randomList.map(entry => ({
						key:
							entry.id ||
							entry.base ||
							(entry.id ? getItemDisplayName(entry.id, entry.base) : entry.base),
						id: entry.id,
						base: entry.base,
						label: entry.id ? getItemDisplayName(entry.id, entry.base) : entry.base,
						disabled: true
					}));
				}
			} else {
				renderCosts(null, 0, []);
			}
		}

		if (EnchantState.action === 'perfect') {
			EnchantState.selectedSlot = slotNum;
			const select = root.querySelector('.perfect_select');
			const perfectEntries = [];
			if (select) {
				select.innerHTML = '';
			}
			if (slotData && slotData.perfect) {
				const perfectList = Object.keys(slotData.perfect);
				perfectList.forEach(key => {
					const entry = slotData.perfect[key];
					const label = entry.id ? getItemDisplayName(entry.id, entry.base) : entry.base;
					if (select) {
						const option = document.createElement('option');
						option.value = key;
						option.textContent = label;
						select.appendChild(option);
					}
					perfectEntries.push({
						key: key,
						id: entry.id,
						base: entry.base,
						label: label
					});
				});
				if (perfectList.length) {
					if (EnchantState.selectedPerfect && slotData.perfect[EnchantState.selectedPerfect]) {
						if (select) {
							select.value = EnchantState.selectedPerfect;
						}
						const selected = slotData.perfect[EnchantState.selectedPerfect];
						renderCosts(100000, selected.zeny, selected.materials);
						actionReady = availability.perfect && canAffordCost(selected.zeny, selected.materials);
						selectedKey = EnchantState.selectedPerfect;
					} else {
						EnchantState.selectedPerfect = null;
						if (select) {
							select.selectedIndex = -1;
						}
						renderCosts(null, 0, []);
					}
				} else {
					renderCosts(null, 0, []);
				}
			} else {
				renderCosts(null, 0, []);
			}
			listEntries = perfectEntries;
		}

		if (EnchantState.action === 'upgrade') {
			const upgradeSelect = root.querySelector('.upgrade_select');
			const upgradeEntries = [];
			if (upgradeSelect) {
				upgradeSelect.innerHTML = '';
			}
			if (upgradeCandidates.length) {
				upgradeCandidates.forEach(candidate => {
					const currentName = getItemDisplayName(candidate.currentId, candidate.baseName);
					const resultId = candidate.entry.result
						? candidate.entry.result.id || DB.getItemIdfromBase(candidate.entry.result.base)
						: 0;
					const resultName = resultId
						? getItemDisplayName(resultId, candidate.entry.result.base)
						: candidate.entry.result
							? candidate.entry.result.base
							: '';
					if (upgradeSelect) {
						const option = document.createElement('option');
						option.value = candidate.slotNum;
						option.textContent = `Slot ${candidate.slotNum + 1}: ${currentName} -> ${resultName}`;
						upgradeSelect.appendChild(option);
					}
					upgradeEntries.push({
						key: String(candidate.slotNum),
						id: resultId || 0,
						base: candidate.entry.result ? candidate.entry.result.base : null,
						label: resultName || currentName
					});
				});
				if (
					!EnchantState.selectedUpgradeSlot ||
					!upgradeCandidates.find(candidate => candidate.slotNum === EnchantState.selectedUpgradeSlot)
				) {
					EnchantState.selectedUpgradeSlot = upgradeCandidates[0].slotNum;
				}
				if (upgradeSelect) {
					upgradeSelect.value = EnchantState.selectedUpgradeSlot;
				}
				const selectedEntry = upgradeCandidates.find(
					candidate => candidate.slotNum === EnchantState.selectedUpgradeSlot
				);
				if (selectedEntry) {
					EnchantState.selectedSlot = selectedEntry.slotNum;
					const ur = root.querySelector('.upgrade_result');
					if (ur) {
						ur.textContent =
							'Result: ' + (selectedEntry.entry.result ? selectedEntry.entry.result.base : '');
					}
					renderCosts(100000, selectedEntry.entry.zeny, selectedEntry.entry.materials);
					actionReady =
						availability.upgrade && canAffordCost(selectedEntry.entry.zeny, selectedEntry.entry.materials);
					selectedKey = String(EnchantState.selectedUpgradeSlot);
				}
			} else {
				renderCosts(null, 0, []);
			}
			listEntries = upgradeEntries;
		}

		if (EnchantState.action === 'reset') {
			EnchantState.selectedSlot = null;
			if (availability.reset) {
				renderCosts(group.reset.rate, group.reset.zeny, group.reset.materials);
				actionReady = canAffordCost(group.reset.zeny, group.reset.materials);
			} else {
				renderCosts(null, 0, []);
			}
		}
	} else {
		updateTabs(availability);
		updateActionSections();
		renderCosts(null, 0, []);
	}

	renderEnchantList(listEntries, selectedKey);
	updateActionButton(actionReady);
}

function refreshUI() {
	renderItemList();
	renderItemPreview();
	refreshActionContent();
	renderSlots();
	setCaution(EnchantState.group ? EnchantState.group.caution : '');
}

function validateItem(item) {
	if (!EnchantState.group) {
		return { ok: false, message: 'Enchant data missing for this group.' };
	}
	if (!item) {
		return { ok: false, message: 'Invalid item.' };
	}
	if (item.WearState) {
		return { ok: false, message: 'Item must be in inventory.' };
	}
	if ((item.equipSwitch && item.equipSwitch > 0) || (item.EquipSwitch && item.EquipSwitch > 0)) {
		return { ok: false, message: 'Item cannot be in equipment switch.' };
	}
	if (item.attribute && item.attribute !== 0) {
		return { ok: false, message: 'Item attribute must be normal.' };
	}
	if (!isTargetItem(item, EnchantState.group)) {
		return { ok: false, message: 'Item is not valid for this enchant group.' };
	}
	const refine = item.RefiningLevel || item.refiningLevel || 0;
	if (refine < EnchantState.group.condition.minRefine) {
		return { ok: false, message: 'Refine level too low.' };
	}
	const grade = getItemGrade(item);
	if (grade < EnchantState.group.condition.minGrade) {
		return { ok: false, message: 'Enchant grade too low.' };
	}
	if (!EnchantState.group.allowRandomOption && hasRandomOptions(item)) {
		return { ok: false, message: 'Random options not allowed.' };
	}
	return { ok: true };
}

function getInventoryItemByIndex(index) {
	const inventoryUI = Inventory.getUI && Inventory.getUI();
	return inventoryUI && inventoryUI.getItemByIndex ? inventoryUI.getItemByIndex(index) : null;
}

function resolveItemContext(target) {
	const slotEntry = target.closest('.slot_entry');
	if (slotEntry) {
		if (!EnchantState.item) {
			return null;
		}
		const slotNum = parseInt(slotEntry.dataset.slot, 10);
		if (isNaN(slotNum)) {
			return null;
		}
		const slotItemId = getSlotValue(EnchantState.item, slotNum);
		if (!slotItemId) {
			return null;
		}
		return {
			item: { ITID: slotItemId, IsIdentified: 1 },
			label: '',
			identified: true,
			anchor: slotEntry
		};
	}

	const itemEntry = target.closest('.item_entry');
	if (itemEntry) {
		const index = parseInt(itemEntry.dataset.index, 10);
		let invItem = !isNaN(index) ? getInventoryItemByIndex(index) : null;
		if (!invItem) {
			const itemId = parseInt(target.dataset.itid, 10);
			if (!isNaN(itemId) && itemId) {
				invItem = { ITID: itemId, IsIdentified: 1 };
			}
		}
		return {
			item: invItem,
			label: target.dataset.name || itemEntry.dataset.name || '',
			identified: invItem ? !!invItem.IsIdentified : true,
			anchor: target
		};
	}

	const materialIcon = target.closest('.material .icon');
	if (materialIcon) {
		const matId = parseInt(materialIcon.dataset.itid, 10);
		const matItem = !isNaN(matId) && matId ? { ITID: matId, IsIdentified: 1 } : null;
		return {
			item: matItem,
			label: materialIcon.dataset.name || '',
			identified: true,
			anchor: materialIcon
		};
	}

	const enchantEntry = target.closest('.enchant_entry');
	if (enchantEntry) {
		const enchantId = parseInt(enchantEntry.dataset.itid, 10);
		const enchantItem = !isNaN(enchantId) && enchantId ? { ITID: enchantId, IsIdentified: 1 } : null;
		return {
			item: enchantItem,
			label: enchantEntry.dataset.name || '',
			identified: true,
			anchor: target
		};
	}

	return null;
}

function onIconOver(event) {
	const context = resolveItemContext(event.currentTarget);
	if (!context) {
		return;
	}
	let label = context.label;
	if (context.item) {
		label = DB.getItemName(context.item, { showItemOptions: false });
	}
	if (!label) {
		return;
	}
	showHoverOverlay(label, context.identified, context.anchor || event.currentTarget);
}

function onIconOut() {
	hideHoverOverlay();
}

function onIconInfo(event) {
	event.preventDefault();
	event.stopImmediatePropagation();
	const context = resolveItemContext(event.currentTarget);
	if (context && context.item) {
		showItemInfo(context.item);
	}
}

function onActionSelect(event) {
	const action = event.currentTarget.dataset.action;
	if (event.currentTarget.classList.contains('disabled')) {
		return;
	}
	EnchantState.action = action;
	refreshUI();
}

function onPerfectChange(event) {
	EnchantState.selectedPerfect = event.currentTarget.value;
	refreshUI();
}

function onUpgradeChange(event) {
	EnchantState.selectedUpgradeSlot = parseInt(event.currentTarget.value, 10);
	refreshUI();
}

function onEnchantListSelect(event) {
	const entry = event.target.closest('.enchant_entry');
	if (!entry) {
		return;
	}
	if (entry.classList.contains('disabled')) {
		return;
	}
	const key = entry.dataset.key;
	if (EnchantState.action === 'perfect') {
		EnchantState.selectedPerfect = key;
		refreshUI();
		return;
	}
	if (EnchantState.action === 'upgrade') {
		const slotNum = parseInt(key, 10);
		if (!isNaN(slotNum)) {
			EnchantState.selectedUpgradeSlot = slotNum;
			refreshUI();
		}
	}
}

function onItemSelect(event) {
	const entry = event.target.closest('.item_entry');
	if (!entry) {
		return;
	}
	const index = parseInt(entry.dataset.index, 10);
	if (isNaN(index)) {
		return;
	}
	const inventoryUI = Inventory.getUI && Inventory.getUI();
	const item = inventoryUI && inventoryUI.getItemByIndex ? inventoryUI.getItemByIndex(index) : null;
	if (!item) {
		return;
	}
	Enchant.onRequestItemEnchant(item);
}

function onRequestAction() {
	const root = _root();
	if (!root) {
		return;
	}
	const actionBtn = root.querySelector('.action_btn');
	if (actionBtn && actionBtn.classList.contains('disabled')) {
		return;
	}
	if (!EnchantState.group || !EnchantState.item) {
		setStatus('Select an item first.', true);
		return;
	}
	let pkt;
	const slotNum = EnchantState.selectedSlot;
	const itemIndex = EnchantState.itemIndex;

	if (EnchantState.action === 'random') {
		if (slotNum === null) {
			setStatus('No available enchant slot.', true);
			return;
		}
		pkt = new PACKET.CZ.REQUEST_RANDOM_ENCHANT();
		pkt.enchant_group = EnchantState.groupId;
		pkt.index = itemIndex;
	} else if (EnchantState.action === 'perfect') {
		if (slotNum === null || !EnchantState.selectedPerfect) {
			setStatus('Select a perfect enchant.', true);
			return;
		}
		const slotData = EnchantState.group.slots[slotNum];
		const perfectEntry = slotData ? slotData.perfect[EnchantState.selectedPerfect] : null;
		const perfectId = perfectEntry ? perfectEntry.id || DB.getItemIdfromBase(perfectEntry.base) : 0;
		if (!perfectEntry || !perfectId) {
			setStatus('Invalid perfect enchant selection.', true);
			return;
		}
		pkt = new PACKET.CZ.REQUEST_PERFECT_ENCHANT();
		pkt.enchant_group = EnchantState.groupId;
		pkt.index = itemIndex;
		pkt.ITID = perfectId;
	} else if (EnchantState.action === 'upgrade') {
		if (slotNum === null) {
			setStatus('Select an upgrade slot.', true);
			return;
		}
		pkt = new PACKET.CZ.REQUEST_UPGRADE_ENCHANT();
		pkt.enchant_group = EnchantState.groupId;
		pkt.index = itemIndex;
		pkt.slot = slotNum;
	} else if (EnchantState.action === 'reset') {
		pkt = new PACKET.CZ.REQUEST_RESET_ENCHANT();
		pkt.enchant_group = EnchantState.groupId;
		pkt.index = itemIndex;
	}

	if (!pkt) {
		return;
	}

	playIntroEffect(EnchantState.action);

	EnchantState.pending = {
		action: EnchantState.action,
		slot: slotNum,
		itemIndex: itemIndex,
		item: EnchantState.item
	};

	EnchantState.pendingLock = true;
	root.querySelectorAll('.close, .close_btn').forEach(btn => {
		btn.disabled = true;
	});
	Network.sendPacket(pkt);
	setStatus('Request sent...', false);
}

function applyEnchantResult(item, action, slotNum, itid) {
	if (!item) {
		return;
	}
	if (action === 'reset') {
		const baseSlots = getBaseSlotCount(item);
		const slotOrder =
			EnchantState.group && EnchantState.group.slotOrder && EnchantState.group.slotOrder.length
				? EnchantState.group.slotOrder
				: [0, 1, 2, 3];
		slotOrder.forEach(orderSlot => {
			if (orderSlot >= baseSlots) {
				setSlotValue(item, orderSlot, 0);
			}
		});
		return;
	}
	if (slotNum !== null && itid) {
		setSlotValue(item, slotNum, itid);
	}
}

function onEnchantResult(pkt) {
	if (!EnchantState.pending) {
		return;
	}
	const message = DB.getMessage(pkt.msgId) || 'Enchant result: ' + pkt.msgId;
	const isSuccess = pkt.msgId === 3857;
	ChatBox.addText(message, ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
	playResultEffect(EnchantState.pending.action, isSuccess);
	if (isSuccess) {
		applyEnchantResult(EnchantState.pending.item, EnchantState.pending.action, EnchantState.pending.slot, pkt.ITID);
		if (ItemInfo.ui && ItemInfo.ui.is(':visible') && ItemInfo.uid === EnchantState.pending.item.ITID) {
			ItemInfo.setItem(EnchantState.pending.item);
		}
	}
	Enchant.remove();
}

function onRequestClose() {
	if (EnchantState.pendingLock) {
		setStatus('Request in progress...', false);
		return;
	}
	if (PACKETVER.value >= 20211103) {
		const pkt = new PACKET.CZ.CLOSE_UI_ENCHANT();
		Network.sendPacket(pkt);
	}
	Enchant.remove();
}

Enchant.onKeyDown = function onKeyDown(event) {
	if (Enchant.isEditableFocused()) {
		return;
	}
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && Enchant.ui.is(':visible')) {
		onRequestClose();
	}
};

Enchant.init = function init() {
	const root = _root();
	this.ui.css({ top: 200, left: 360 });
	this.draggable(root.querySelector('.titlebar'));
	preloadEnchantEffectDurations();

	Client.loadFiles(
		[
			DB.INTERFACE_PATH + 'enchantui/bg_subpage.bmp',
			DB.INTERFACE_PATH + 'enchantui/bg_subpageb.bmp',
			DB.INTERFACE_PATH + 'enchantui/bg_subpagey.bmp'
		],
		function (normal, upgrade, reset) {
			EnchantAssets.subpage.normal = normal;
			EnchantAssets.subpage.upgrade = upgrade;
			EnchantAssets.subpage.reset = reset;
			updateSubpageSkin();
		}
	);

	Client.loadFiles(
		[
			DB.INTERFACE_PATH + 'enchantui/bg_enc_menu0.bmp',
			DB.INTERFACE_PATH + 'enchantui/bg_enc_menu1.bmp',
			DB.INTERFACE_PATH + 'enchantui/bg_enc_menu2.bmp',
			DB.INTERFACE_PATH + 'enchantui/bg_enc_menu3.bmp'
		],
		function (menu0, menu1, menu2, menu3) {
			EnchantAssets.menuTabs.random = menu0;
			EnchantAssets.menuTabs.perfect = menu1;
			EnchantAssets.menuTabs.upgrade = menu2;
			EnchantAssets.menuTabs.reset = menu3;
			updateMenuSkin();
		}
	);

	Client.loadFiles(
		[
			DB.INTERFACE_PATH + 'enchantui/btn_enc.bmp',
			DB.INTERFACE_PATH + 'enchantui/btn_enc_over.bmp',
			DB.INTERFACE_PATH + 'enchantui/btn_enc_press.bmp',
			DB.INTERFACE_PATH + 'enchantui/btn_enc_dis.bmp'
		],
		function (normal, hover, down, disabled) {
			EnchantAssets.actionButton.normal = normal;
			EnchantAssets.actionButton.hover = hover;
			EnchantAssets.actionButton.down = down;
			EnchantAssets.actionButton.disabled = disabled;
			bindActionButtonSkin();
			const actionBtn = _root() ? _root().querySelector('.action_btn') : null;
			if (actionBtn) {
				updateActionButtonSkin(actionBtn, !actionBtn.classList.contains('disabled'));
			}
		}
	);

	Client.loadFiles(
		[
			DB.INTERFACE_PATH + 'enchantui/scroll_arrow_up.bmp',
			DB.INTERFACE_PATH + 'enchantui/scroll_arrow_dowm.bmp',
			DB.INTERFACE_PATH + 'enchantui/scroll_bg.bmp',
			DB.INTERFACE_PATH + 'enchantui/scroll_btn.bmp'
		],
		applyScrollSkin
	);

	root.querySelectorAll('.close, .close_btn').forEach(btn => {
		btn.addEventListener('click', onRequestClose);
	});

	const itemList = root.querySelector('.item_list');
	if (itemList) {
		itemList.addEventListener('click', onItemSelect);
		itemList.addEventListener('mouseover', e => {
			const slot = e.target.closest('.item_slot');
			if (slot) {
				onIconOver({ currentTarget: slot });
			}
		});
		itemList.addEventListener('mouseout', e => {
			const slot = e.target.closest('.item_slot');
			if (slot) {
				onIconOut();
			}
		});
		itemList.addEventListener('contextmenu', e => {
			const slot = e.target.closest('.item_slot');
			if (slot) {
				onIconInfo({
					currentTarget: slot,
					preventDefault: () => e.preventDefault(),
					stopImmediatePropagation: () => e.stopImmediatePropagation()
				});
			}
		});
	}

	const actionTabs = root.querySelector('.action_tabs');
	if (actionTabs) {
		actionTabs.addEventListener('click', e => {
			const tab = e.target.closest('.tab');
			if (tab) {
				onActionSelect({ currentTarget: tab });
			}
		});
	}

	const perfectSelect = root.querySelector('.perfect_select');
	if (perfectSelect) {
		perfectSelect.addEventListener('change', onPerfectChange);
	}

	const upgradeSelect = root.querySelector('.upgrade_select');
	if (upgradeSelect) {
		upgradeSelect.addEventListener('change', onUpgradeChange);
	}

	const materialList = root.querySelector('.material_list');
	if (materialList) {
		materialList.addEventListener('mouseover', e => {
			const icon = e.target.closest('.icon');
			if (icon) {
				onIconOver({ currentTarget: icon });
			}
		});
		materialList.addEventListener('mouseout', e => {
			const icon = e.target.closest('.icon');
			if (icon) {
				onIconOut();
			}
		});
		materialList.addEventListener('contextmenu', e => {
			const icon = e.target.closest('.icon');
			if (icon) {
				onIconInfo({
					currentTarget: icon,
					preventDefault: () => e.preventDefault(),
					stopImmediatePropagation: () => e.stopImmediatePropagation()
				});
			}
		});
	}

	const slotList = root.querySelector('.slot_list');
	if (slotList) {
		slotList.addEventListener('mouseover', e => {
			const icon = e.target.closest('.slot_icon');
			if (icon) {
				onIconOver({ currentTarget: icon });
			}
		});
		slotList.addEventListener('mouseout', e => {
			const icon = e.target.closest('.slot_icon');
			if (icon) {
				onIconOut();
			}
		});
		slotList.addEventListener('contextmenu', e => {
			const icon = e.target.closest('.slot_icon');
			if (icon) {
				onIconInfo({
					currentTarget: icon,
					preventDefault: () => e.preventDefault(),
					stopImmediatePropagation: () => e.stopImmediatePropagation()
				});
			}
		});
	}

	const enchantList = root.querySelector('.enchant_list');
	if (enchantList) {
		enchantList.addEventListener('click', onEnchantListSelect);
		enchantList.addEventListener('mouseover', e => {
			const icon = e.target.closest('.entry_icon');
			if (icon) {
				onIconOver({ currentTarget: icon });
			}
		});
		enchantList.addEventListener('mouseout', e => {
			const icon = e.target.closest('.entry_icon');
			if (icon) {
				onIconOut();
			}
		});
		enchantList.addEventListener('contextmenu', e => {
			const icon = e.target.closest('.entry_icon');
			if (icon) {
				onIconInfo({
					currentTarget: icon,
					preventDefault: () => e.preventDefault(),
					stopImmediatePropagation: () => e.stopImmediatePropagation()
				});
			}
		});
	}

	const actionBtn = root.querySelector('.action_btn');
	if (actionBtn) {
		actionBtn.addEventListener('click', onRequestAction);
	}
};

Enchant.onAppend = function onAppend() {
	setStatus('', false);
	refreshUI();
};

Enchant.onRemove = function onRemove() {
	const root = _root();
	clearState();
	if (root) {
		root.querySelectorAll('.close, .close_btn').forEach(btn => {
			btn.disabled = false;
		});
		root.querySelectorAll('.slot_entry').forEach(entry => {
			entry.classList.remove('locked', 'active');
			entry.title = '';
		});
		root.querySelectorAll('.slot_icon').forEach(icon => {
			icon.style.backgroundImage = '';
		});
		const materialList = root.querySelector('.material_list');
		if (materialList) {
			materialList.innerHTML = '';
		}
		const enchantList = root.querySelector('.enchant_list');
		if (enchantList) {
			enchantList.innerHTML = '';
		}
		const perfectSelect = root.querySelector('.perfect_select');
		if (perfectSelect) {
			perfectSelect.innerHTML = '';
		}
		const upgradeSelect = root.querySelector('.upgrade_select');
		if (upgradeSelect) {
			upgradeSelect.innerHTML = '';
		}
		const upgradeResult = root.querySelector('.upgrade_result');
		if (upgradeResult) {
			upgradeResult.textContent = '';
		}
		const zenyCost = root.querySelector('.zeny_cost');
		if (zenyCost) {
			zenyCost.textContent = '';
		}
		const itemList = root.querySelector('.item_list');
		if (itemList) {
			itemList.innerHTML = '';
		}
		const previewItem = root.querySelector('.preview_item');
		if (previewItem) {
			previewItem.style.backgroundImage = '';
		}
	}
	hideHoverOverlay();
	setStatus('', false);
};

Enchant.onOpenEnchantUI = function onOpenEnchantUI(groupId) {
	clearState();
	EnchantState.groupId = Number(groupId);
	EnchantState.group = DB.getEnchantGroup(EnchantState.groupId);
	if (!EnchantState.group) {
		UIManager.showErrorBox('Enchant data missing for group ' + groupId + '.');
	}
	Enchant.append();
	Enchant.ui.show();
	Enchant.focus();
	refreshUI();
};

Enchant.onRequestItemEnchant = function onRequestItemEnchant(item) {
	const result = validateItem(item);
	if (!result.ok) {
		setStatus(result.message, true);
		return;
	}
	EnchantState.item = item;
	EnchantState.itemIndex = item.index;
	EnchantState.selectedPerfect = null;
	EnchantState.selectedUpgradeSlot = null;
	EnchantState.selectedSlot = null;
	setStatus('', false);
	refreshUI();
};

Enchant.isEnchantOpen = function isEnchantOpen() {
	return Enchant.ui && Enchant.ui.is(':visible');
};

Network.hookPacket(PACKET.ZC.RESPONSE_ENCHANT, onEnchantResult);
export default UIManager.addComponent(Enchant);
