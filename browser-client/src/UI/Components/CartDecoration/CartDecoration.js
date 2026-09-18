/**
 * UI/Components/CartDecoration/CartDecoration.js
 *
 * Cart Decoration (MC_CARTDECORATE) skill UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */

import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Renderer from 'Renderer/Renderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Client from 'Core/Client.js';
import DB from 'DB/DBManager.js';
import htmlText from './CartDecoration.html?raw';
import cssText from './CartDecoration.css?raw';

/**
 * Create Component
 */
const CartDecoration = new GUIComponent('CartDecoration', cssText);

/**
 * Render HTML
 */
CartDecoration.render = () => htmlText;

/**
 * @var {object} loaded cart sprite data keyed by cart type id
 */
const _carts = {};
const _layerEntity = new Entity();

/**
 * @var {number} identity sent by server
 */
let _identity = 0;

/**
 * Decorative cart type IDs
 */
const CART_TYPES = [10, 11, 12];

/**
 * Initialize UI
 */
CartDecoration.init = function init() {
	const root = this.getRoot();

	this._host.style.top = (Renderer.height - 100) / 2.0 + 'px';
	this._host.style.left = (Renderer.width - 255) / 2.0 + 'px';

	root.querySelector('.titlebar .close').addEventListener('click', () => {
		CartDecoration._host.style.display = 'none';
		Renderer.stop(render);
	});
	this.draggable('.titlebar');

	const carts = root.querySelectorAll('.cart');
	carts.forEach(el => {
		el.classList.add('event_add_cursor');
		el.addEventListener('click', event => {
			onCartSelected(parseInt(event.currentTarget.getAttribute('data-id'), 10));
		});
	});

	preloadCartData();
};

/**
 * Pre-load sprite & action data for decorative carts
 */
function preloadCartData() {
	for (const id of CART_TYPES) {
		const path = DB.getCartPath(id);
		Client.loadFiles([path + '.spr', path + '.act'], (spr, act) => {
			_carts[id] = { spr, act };
		});
	}
}

/**
 * @param {number} type - cart type id
 */
function onCartSelected(type) {
	if (!CART_TYPES.includes(type)) {
		return;
	}

	const pkt = new PACKET.CZ.SELECTCART();
	pkt.identity = _identity;
	pkt.type = type;
	Network.sendPacket(pkt);

	CartDecoration._host.style.display = 'none';
	Renderer.stop(render);
}

/**
 * Append to body
 */
CartDecoration.onAppend = function onAppend() {
	this._host.style.display = 'none';
	preloadCartData();
};

/**
 * @param {object} pkt - parsed ZC_SELECTCART packet
 */
CartDecoration.onSelectCart = function onSelectCart(pkt) {
	const root = CartDecoration.getRoot();
	_identity = pkt.identity;

	root.querySelectorAll('.cart').forEach(el => {
		el.style.display = 'none';
	});

	const advertised = pkt.typeList && pkt.typeList.length ? pkt.typeList : CART_TYPES;
	for (const type of advertised) {
		const el = root.querySelector('.cart[data-id="' + type + '"]');
		if (el) {
			el.style.display = '';
		}
	}

	CartDecoration.ui.show();
	Renderer.stop(render);
	Renderer.render(render);
};

/**
 * Remove component
 */
CartDecoration.onRemove = function onRemove() {
	Renderer.stop(render);
};

CartDecoration.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		this._host.style.display = 'none';
		Renderer.stop(render);
	}
};

/**
 * Pick layers from action file
 */
function pickLayers(act, actionId) {
	const a = act.actions[actionId];
	if (!a || !a.animations || !a.animations.length) {
		return null;
	}
	return a.animations[(a.animations.length / 2) | 0].layers;
}

/**
 * Draw a cart action to a canvas context
 */
function drawActionToCanvas(ctx, act, spr, actionId, x, y) {
	const layers = pickLayers(act, actionId);
	if (!layers) {
		return;
	}
	SpriteRenderer.bind2DContext(ctx, x, y);
	for (let i = 0; i < layers.length; i++) {
		_layerEntity.renderLayer(layers[i], spr, spr, 1.0, [0, 0], false);
	}
}

/**
 * Render loop
 */
function render() {
	const root = CartDecoration.getRoot();
	const canvases = root.querySelectorAll('.canvas');

	canvases.forEach(el => {
		if (el.offsetParent === null) return; // not visible

		const id = parseInt(el.getAttribute('data-id'), 10);
		const data = _carts[id];

		if (!data || !data.spr || !data.act) {
			return;
		}

		const ctx = el.getContext('2d');
		ctx.clearRect(0, 0, el.width, el.height);
		drawActionToCanvas(ctx, data.act, data.spr, 0, el.width / 2, el.height + 10);
	});
}

CartDecoration.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Create component and export it
 */
export default UIManager.addComponent(CartDecoration);
