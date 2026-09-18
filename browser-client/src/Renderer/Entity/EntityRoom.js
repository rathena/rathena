/**
 * Renderer/Entity/EntityRoom.js
 *
 * Shop / Chat room above entity's head
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import DB from 'DB/DBManager.js';
import EntityRoom from 'UI/Components/EntityRoom/EntityRoom.js';

/**
 * Private methods
 */
const vec4 = glMatrix.vec4;
const _pos = new Float32Array(4);
const _size = new Float32Array(2);

/**
 * Room class — vending shop / buying store / chatroom icon & title above entity head
 *
 * @class Room
 * @property {Entity} owner Owner entity reference
 * @property {string} text Room text content
 * @property {boolean} display Whether room sign is visible
 * @property {HTMLElement|null} node DOM element node for the sign
 * @property {number} type Room type (SELL_SHOP, BUY_SHOP, PUBLIC_CHAT, PRIVATE_CHAT)
 * @property {number} id Room ID / GID
 * @property {string} title Room title text
 * @property {number} count Member / item count
 * @property {number} maxcount Maximum allowed members / items
 */
class Room {
	/**
	 * Constants
	 */
	static Type = {
		SELL_SHOP: 0,
		BUY_SHOP: 1,
		PUBLIC_CHAT: 2,
		PRIVATE_CHAT: 3
	};

	/**
	 * @Constructor Room
	 */
	constructor(owner) {
		this.owner = owner;
		this.text = '';
		this.display = false;
		this.node = null;
		this.type = Room.Type.BUY_SHOP;
		this.id = 0;
		this.title = '';
		this.count = 0;
		this.limit = 0;
	}

	/**
	 * Create a room
	 *
	 * @param {string} room title
	 * @param {id} target id
	 * @param {const} Room.Type constant
	 * @param {bool} is clickable ?
	 */
	create(title, id, type, clickable) {
		const self = this;

		function init() {
			let filename = 'chat_open';

			switch (type) {
				case Room.Type.PUBLIC_CHAT:
					filename = 'chat_open';
					break;
				case Room.Type.SELL_SHOP:
					filename = 'buyingshop';
					break;
				case Room.Type.BUY_SHOP:
					filename = 'shop';
					break;
				case Room.Type.PRIVATE_CHAT:
					filename = 'chat_close';
					break;
			}

			self.type = type;
			self.id = id;
			self.node.onEnter = clickable ? self.owner.onRoomEnter.bind(self.owner) : null;

			Client.loadFile(DB.INTERFACE_PATH + filename + '.bmp', function (url) {
				self.display = true;

				if (self.node) {
					self.node.setTitle(title, url);
				}
			});
		}

		// Already exist
		if (this.node) {
			init();
			this.node.append();
			return;
		}

		this.node = EntityRoom.clone('EntityRoom', true);
		this.node.init = init;
		this.node.append();
	}

	/**
	 * Remove Room
	 */
	remove() {
		this.display = false;
		if (this.node) {
			this.node.remove();
		}
	}

	/**
	 * Clean Room
	 */
	clean() {
		this.remove();
		this.node = null;
	}

	/**
	 * Rendering Room
	 *
	 * @param {mat4} matrix
	 */
	render(matrix) {
		const ui = this.node.ui[0];

		// Cast position
		_pos[0] = 0.0;
		_pos[1] = 120 / 35;
		_pos[2] = 0.0;
		_pos[3] = 1.0;

		// Set the viewport
		_size[0] = window.innerWidth / 2;
		_size[1] = window.innerHeight / 2;

		// Project point to scene
		vec4.transformMat4(_pos, _pos, matrix);

		// Calculate position
		const z = _pos[3] === 0.0 ? 1.0 : 1.0 / _pos[3];
		_pos[0] = _size[0] + Math.round(_size[0] * (_pos[0] * z));
		_pos[1] = _size[1] - Math.round(_size[1] * (_pos[1] * z));

		ui.style.top = (_pos[1] | 0) + 'px';
		ui.style.left = ((_pos[0] - ui.clientWidth / 2) | 0) + 'px';
	}
}

/**
 * Export
 */
export default function Init() {
	this.room = new Room(this);
}
