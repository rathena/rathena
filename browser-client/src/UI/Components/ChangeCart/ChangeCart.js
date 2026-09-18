/**
 * UI/Components/ChangeCart/ChangeCart.js
 *
 * Change Cart UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */

import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import ChatRoom from 'UI/Components/ChatRoom/ChatRoom.js';
import Client from 'Core/Client.js';
import DB from 'DB/DBManager.js';
import htmlText from './ChangeCart.html?raw';
import cssText from './ChangeCart.css?raw';

// Config
const CART_LIMIT = 13;

/**
 * Create Component
 */
const ChangeCart = new GUIComponent('ChangeCart', cssText);

/**
 * Render HTML
 */
ChangeCart.render = () => htmlText;

/**
 * @var {object} data info
 */
const _carts = {};
const _layerEntity = new Entity();

/**
 * Initialize UI
 */
ChangeCart.init = function init() {
	const root = this.getRoot();
	const carts = root.querySelectorAll('.cart');

	this._host.style.top = (Renderer.height - 100) / 2.0 + 'px';
	this._host.style.left = (Renderer.width - 400) / 2.0 + 'px';

	root.querySelector('.titlebar .close').addEventListener('click', () => {
		ChangeCart._host.style.display = 'none';
		Renderer.stop(render);
	});
	this.draggable('.titlebar');

	carts.forEach(el => {
		el.style.display = 'none';
		el.classList.add('event_add_cursor');
		el.addEventListener('click', event => {
			onCart(parseInt(event.currentTarget.getAttribute('data-id'), 10));
		});
	});

	// Pre-load carts
	loadCartData();
};

/**
 * Load Carts assets
 */
function loadCartData() {
	for (let i = 0; i <= CART_LIMIT; ++i) {
		const path = DB.getCartPath(i);
		Client.loadFiles([path + '.spr', path + '.act'], (spr, act) => {
			_carts[i] = { spr: spr, act: act };
		});
	}
}

/**
 * Change cart (Change cart packet IDs are not the same as global cart IDs!!)
 */
function onCart(num) {
	if (Session.Entity.hasCart == false || num < 0 || num > 9) {
		return;
	}

	const pkt = new PACKET.CZ.REQ_CHANGECART();
	pkt.num = num;
	Network.sendPacket(pkt);
	ChangeCart._host.style.display = 'none';
	Renderer.stop(render);
}

/**
 * Append to body
 */
ChangeCart.onAppend = function onAppend() {
	this._host.style.display = 'none';
	loadCartData();
};

ChangeCart.onChangeCartSkill = function onChangeCartSkill() {
	if (Session.Entity.hasCart == false) {
		return;
	}
	let msg = 'Change Cart!!';
	if (ChatRoom.isOpen) {
		msg = 'Close your Room first!!';
		ChatRoom.message(msg);
		return;
	}
	ChatBox.addText(msg, ChatBox.TYPE.PUBLIC | ChatBox.TYPE.SELF, ChatBox.FILTER.PUBLIC_LOG);
	if (Session.Entity) {
		Session.Entity.dialog.set(msg);
	}
	ChangeCart.ui.show();
	updateList(Session.Entity.clevel);
	// Avoid stacking duplicate render callbacks if invoked while already open
	Renderer.stop(render);
	Renderer.render(render);
};

ChangeCart.onLevelUp = function onLevelUp(blvl) {
	updateList(blvl);
};

function updateList(blvl) {
	if (Session.Entity.hasCart == false) {
		return;
	}

	const root = ChangeCart.getRoot();
	root.querySelectorAll('.cart').forEach(el => {
		el.style.display = 'none';
	});

	if (blvl > 131) {
		root.querySelector(".cart[data-id='9']").style.display = '';
	}
	if (blvl > 121) {
		root.querySelector(".cart[data-id='8']").style.display = '';
	}
	if (blvl > 111) {
		root.querySelector(".cart[data-id='7']").style.display = '';
	}
	if (blvl > 100) {
		root.querySelector(".cart[data-id='6']").style.display = '';
	}
	if (blvl > 90) {
		root.querySelector(".cart[data-id='5']").style.display = '';
	}
	if (blvl > 80) {
		root.querySelector(".cart[data-id='4']").style.display = '';
	}
	if (blvl > 65) {
		root.querySelector(".cart[data-id='3']").style.display = '';
	}
	if (blvl > 40) {
		root.querySelector(".cart[data-id='2']").style.display = '';
	}

	root.querySelector(".cart[data-id='1']").style.display = '';
}

/**
 * Remove component from HTML
 * Stop rendering
 */
ChangeCart.onRemove = function onRemove() {
	Renderer.stop(render);
};

ChangeCart.onKeyDown = function onKeyDown(event) {
	if (this._host.style.display === 'none') {
		return true;
	}
	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		this._host.style.display = 'none';
		Renderer.stop(render);
		event.stopImmediatePropagation();
		return false;
	}
	return true;
};

/**
 * Pick layers from act
 */
function pickLayers(act, actionId) {
	const a = act.actions[actionId];
	if (!a || !a.animations || !a.animations.length) {
		return null;
	}
	return a.animations[(a.animations.length / 2) | 0].layers;
}

/**
 * Draw action to canvas
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
 * Rendering the Carts
 */
function render(tick) {
	const root = ChangeCart.getRoot();
	const canvases = root.querySelectorAll('.canvas');

	canvases.forEach(el => {
		// Skip hidden canvases
		if (el.offsetParent === null) {
			return;
		}

		const id = el.getAttribute('data-id');
		const data = _carts[id];

		if (!data || !data.spr || !data.act) {
			return;
		}

		const ctx = el.getContext('2d');
		ctx.clearRect(0, 0, el.width, el.height);
		drawActionToCanvas(ctx, data.act, data.spr, 0, el.width / 2, el.height + 10);
	});
}

ChangeCart.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Create component and export it
 */
export default UIManager.addComponent(ChangeCart);
