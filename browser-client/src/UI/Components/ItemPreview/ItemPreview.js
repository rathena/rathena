/**
 * UI/Components/ItemPreview/ItemPreview.js
 *
 * Item preview window (costume/hat)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import DB from 'DB/DBManager.js';
import EquipLocation from 'DB/Items/EquipmentLocation.js';
import Renderer from 'Renderer/Renderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Camera from 'Renderer/Camera.js';
import Session from 'Engine/SessionStorage.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './ItemPreview.html?raw';
import cssText from './ItemPreview.css?raw';
import ItemInfo from 'UI/Components/ItemInfo/ItemInfo.js';
import Entity from 'Renderer/Entity/Entity.js';

/**
 * Create Component
 */
const ItemPreview = new GUIComponent('ItemPreview', cssText);

/**
 * Render HTML
 */
ItemPreview.render = () => htmlText;

/**
 * @var {CanvasRenderingContext2D}
 */
let _ctx;

/**
 * @var {number} direction
 */
let _direction = 0;

/**
 * @var {number} preview location bitmask
 */
let _previewLocation = 0;

/**
 * @var {number} preview sprite id
 */
let _previewSpriteId = 0;

/**
 * @var {boolean} render state
 */
let _rendering = false;

/**
 * @var {number} ItemPreview unique id
 */
ItemPreview.uid = -1;

let _remove = false;

/**
 * Initialize UI
 */
ItemPreview.init = function init() {
	const root = this.getRoot();

	_ctx = root.querySelector('canvas').getContext('2d');

	root.querySelector('.close').addEventListener('click', () => {
		ItemPreview.remove();
	});

	root.querySelector('.rot_left').addEventListener('click', event => {
		event.stopImmediatePropagation();
		rotatePreview(1);
	});

	root.querySelector('.rot_right').addEventListener('click', event => {
		event.stopImmediatePropagation();
		rotatePreview(-1);
	});

	root.querySelector('.reset').addEventListener('click', event => {
		event.stopImmediatePropagation();
		resetPreview();
	});

	this.draggable('.titlebar');
};

/**
 * Once append
 */
ItemPreview.onAppend = function onAppend() {
	if (ItemInfo.ui) {
		const itemInfoPosition = ItemInfo.ui.offset();
		const itemInfoWidth = ItemInfo.ui.width();
		const hostWidth = this._host.offsetWidth;
		const hostHeight = this._host.offsetHeight;
		let left = itemInfoPosition.left + itemInfoWidth + 10;
		let top = itemInfoPosition.top;

		if (left + hostWidth > Renderer.width) {
			left = Math.max(0, itemInfoPosition.left - hostWidth - 10);
		}

		if (top + hostHeight > Renderer.height) {
			top = Math.max(0, Renderer.height - hostHeight);
		}

		Object.assign(this._host.style, {
			top: `${top || 200}px`,
			left: `${left || 200}px`
		});
	}

	if (!_rendering) {
		Renderer.render(renderPreview);
		_rendering = true;
	}
};

/**
 * Once removed from html
 */
ItemPreview.onRemove = function onRemove() {
	this.uid = -1;
	_previewLocation = 0;
	_previewSpriteId = 0;
	_direction = 0;

	if (_rendering) {
		Renderer.stop(renderPreview);
		_rendering = false;
	}
};

/**
 * Bind component
 *
 * @param {object} item
 */
ItemPreview.setItem = function setItem(item) {
	const it = DB.getItemInfo(item.ITID);

	this.item = item;
	_previewLocation = getItemLocation(item);
	_previewSpriteId = getPreviewSpriteId(item, it);
	_direction = 0;
};

/**
 * Rotate preview direction
 *
 * @param {number} delta
 */
function rotatePreview(delta) {
	_direction = (_direction + delta + 8) % 8;
}

/**
 * Reset preview direction
 */
function resetPreview() {
	_remove = !_remove;
	_direction = 0;
}

/**
 * Get preview item location
 *
 * @param {object} item
 * @returns {number}
 */
function getItemLocation(item) {
	if (!item) {
		return 0;
	}

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

/**
 * Get preview sprite id
 *
 * @param {object} item
 * @param {object} it
 * @returns {number}
 */
function getPreviewSpriteId(item, it) {
	if (item && item.wItemSpriteNumber) {
		return item.wItemSpriteNumber;
	}

	if (it && it.ClassNum) {
		return it.ClassNum;
	}

	return 0;
}

/**
 * Rendering character
 */
const renderPreview = (function renderPreviewClosure() {
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

	return function render() {
		if (!_ctx) {
			return;
		}

		_ctx.clearRect(0, 0, _ctx.canvas.width, _ctx.canvas.height);

		if (!_previewSpriteId || !_previewLocation || !Session.Entity) {
			return;
		}

		const previewCharacter = new Entity();
		previewCharacter.set({
			GID: Session.Entity.GID + '_PREVIEW',
			objecttype: previewCharacter.constructor.TYPE_PC,
			job: Session.Entity.job,
			sex: Session.Entity.sex,
			name: '',
			hideShadow: true,
			head: Session.Entity.head,
			headpalette: Session.Entity.headpalette,
			bodypalette: Session.Entity.bodypalette,
			accessory: Session.Entity.accessory,
			accessory2: Session.Entity.accessory2,
			accessory3: Session.Entity.accessory3,
			robe: Session.Entity.robe
		});

		if (!_remove) {
			applyPreviewItem(previewCharacter);
		}

		_savedColor.set(previewCharacter.effectColor);
		previewCharacter.effectColor.set(_cleanColor);

		// Set action
		Camera.direction = 0;
		previewCharacter.direction = _direction;
		previewCharacter.headDir = 0;
		previewCharacter.action = previewCharacter.ACTION.IDLE;
		previewCharacter.animation = _animation;

		SpriteRenderer.bind2DContext(_ctx, Math.floor(_ctx.canvas.width / 2), _ctx.canvas.height);
		previewCharacter.renderEntity(_ctx);
		previewCharacter.effectColor.set(_savedColor);
	};
})();

/**
 * Apply preview item to entity
 *
 * @param {object} entity
 */
function applyPreviewItem(entity) {
	if (_previewLocation & EquipLocation.HEAD_BOTTOM) {
		entity.accessory = _previewSpriteId;
	}
	if (_previewLocation & EquipLocation.HEAD_MID) {
		entity.accessory3 = _previewSpriteId;
	}
	if (_previewLocation & EquipLocation.HEAD_TOP) {
		entity.accessory2 = _previewSpriteId;
	}

	if (_previewLocation & EquipLocation.COSTUME_HEAD_BOTTOM) {
		entity.accessory = _previewSpriteId;
	}
	if (_previewLocation & EquipLocation.COSTUME_HEAD_MID) {
		entity.accessory3 = _previewSpriteId;
	}
	if (_previewLocation & EquipLocation.COSTUME_HEAD_TOP) {
		entity.accessory2 = _previewSpriteId;
	}

	if (_previewLocation & EquipLocation.COSTUME_ROBE) {
		entity.robe = _previewSpriteId;
	}
}

/**
 * Create component and export it
 */
export default UIManager.addComponent(ItemPreview);
