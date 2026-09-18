/**
 * Renderer/EntityAttachments.js
 *
 * Helper to manage entity's attachment
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Client from 'Core/Client.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Camera from 'Renderer/Camera.js';
import Ground from 'Renderer/Map/Ground.js';
import StrEffect from 'Renderer/Effects/StrEffect.js';
import Renderer from 'Renderer/Renderer.js';

// Default fog for STR attachments
const _defaultFog = {
	use: false,
	exist: false,
	near: 30,
	far: 180,
	color: new Float32Array([1, 1, 1])
};

const _effectColor = new Float32Array(4);
const _position = new Int32Array(2);

/**
 * AttachmentManager class
 *
 * @class AttachmentManager
 * @property {Array<Object>} list List of active entity attachments (sprites, str effects)
 * @property {Entity} entity Target attached entity
 */
class AttachmentManager {
	constructor(entity) {
		this.list = [];
		this.entity = entity;
	}

	/**
	 * Add an attachment
	 *
	 * @param {object} attachment options
	 */
	add(attachment) {
		if (attachment.uid && !attachment.stackable) {
			this.remove(attachment.uid);
		}

		attachment.startTick = Date.now();
		attachment.opacity = !isNaN(attachment.opacity) ? attachment.opacity : 1.0;
		attachment.direction = attachment.hasOwnProperty('frame') ? false : true;
		attachment.frame = attachment.frame || 0;
		attachment.depth = attachment.depth || 0.0;
		attachment.head = attachment.head || false;

		attachment.position = false;

		if (attachment.yOffset || attachment.xOffset) {
			attachment.position = new Int16Array(2);
			if (attachment.xOffset) {
				attachment.position[0] = attachment.xOffset;
			}
			if (attachment.yOffset) {
				attachment.position[1] = attachment.yOffset;
			}
		}

		attachment.repeat = attachment.repeat || false;
		attachment.duration = attachment.duration || 0;
		attachment.stopAtEnd = attachment.stopAtEnd || false;
		attachment.delay = attachment.delay || false;
		attachment.renderBefore = attachment.renderBefore || false;

		if (attachment.completeFile) {
			attachment.spr = attachment.completeFile + '.spr';
			attachment.act = attachment.completeFile + '.act';
		} else if (attachment.file) {
			attachment.spr = 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/' + attachment.file + '.spr';
			attachment.act = 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/' + attachment.file + '.act';
		}

		// STR file attachment
		if (attachment.strFile) {
			attachment.isStr = true;
			const strNormalized = attachment.strFile.replace(/\\/g, '/');
			const lastSlash = strNormalized.lastIndexOf('/');
			let strTexturePath = lastSlash >= 0 ? strNormalized.substring(0, lastSlash + 1) : '';

			// STRLayer (Str.js) always prepends "data\texture\effect\" to texture paths because
			// other game effects (spamSTR, enchant, etc.) pass a relative texturePath and rely on
			// that prefix. Since hat effect strFile needs the full path for Client.loadFile, the
			// extracted strTexturePath ends up including the prefix — strip it to avoid duplication.
			strTexturePath = strTexturePath.replace(/^data\/texture\/effect\//, '');

			const strEffect = new StrEffect(attachment.strFile, this.entity.position, Renderer.tick, strTexturePath);
			strEffect.ownerEntity = this.entity;
			strEffect.persistent = attachment.repeat !== false;
			strEffect.xOffset = attachment.xOffset || 0;
			strEffect.yOffset = attachment.yOffset || 0;

			attachment.strEffect = strEffect;
			this.list.push(attachment);
			return;
		}

		// Start rendering once sprite is loaded
		Client.loadFile(
			attachment.spr,
			function onLoad() {
				this.list.push(attachment);
			}.bind(this),
			null,
			{ to_rgba: true }
		);
	}

	/**
	 * Get an attachment
	 *
	 * @param {mixed} unique id
	 */
	get(uid) {
		const length = this.list.length;
		for (let i = 0; i < length; ++i) {
			if (this.list[i].uid == uid) {
				return this.list[i];
			}
		}
		return null;
	}

	/**
	 * Remove an attachment
	 *
	 * @param {mixed} unique id
	 */
	remove(uid) {
		const list = this.list;
		let count = list.length;

		for (let i = 0; i < count; ++i) {
			if (list[i].uid === uid) {
				this.removeIndex(i);
				i--;
				count--;
			}
		}
	}

	/**
	 * Remove attachment at index
	 *
	 * @param {number} index
	 */
	removeIndex(index) {
		this.list.splice(index, 1);

		// Is effect and no attachment, clean up
		if (this.list.length === 0 && this.entity.objecttype === this.entity.constructor.TYPE_EFFECT) {
			this.entity.remove();
		}
	}

	/**
	 * Render attachments filtered by renderBefore flag
	 */
	_renderFiltered(tick, renderBeforeEntity) {
		if (!this.list.some(item => item.renderBefore === renderBeforeEntity)) {
			return;
		}

		SpriteRenderer.shadow = Ground.getShadowFactor(this.entity.position[0], this.entity.position[1]);

		_effectColor[0] = this.entity.effectColor[0];
		_effectColor[1] = this.entity.effectColor[1];
		_effectColor[2] = this.entity.effectColor[2];
		_effectColor[3] = this.entity.effectColor[3];

		// Reset effectColor to white for attachments
		this.entity.effectColor[0] = 1.0;
		this.entity.effectColor[1] = 1.0;
		this.entity.effectColor[2] = 1.0;
		this.entity.effectColor[3] = 1.0;

		let count = this.list.length;

		for (let i = 0; i < count; ++i) {
			if (!!this.list[i].renderBefore !== renderBeforeEntity) {
				continue;
			}

			// Effects ignore ground shadow
			if (this.list[i].ignoreShadow || this.list[i].renderBefore) {
				SpriteRenderer.shadow = 1.0;
			}

			if (this.renderAttachment(this.list[i], tick)) {
				this.removeIndex(i);
				i--;
				count--;
			}
		}

		SpriteRenderer.depth = 0.0;

		// Restore original effectColor
		this.entity.effectColor[0] = _effectColor[0];
		this.entity.effectColor[1] = _effectColor[1];
		this.entity.effectColor[2] = _effectColor[2];
		this.entity.effectColor[3] = _effectColor[3];
	}

	/**
	 * Render attachments before (behind) the entity
	 */
	renderBefore(tick) {
		this._renderFiltered(tick, true);
	}

	/**
	 * Render attachments after (in front of) the entity
	 */
	render(tick) {
		this._renderFiltered(tick, false);
	}

	/**
	 * Render an attachment
	 *
	 * @param {object} attachment options
	 * @param {number} game tick
	 * @return {boolean} remove from the list
	 */
	renderAttachment(attachment, tick) {
		// Nothing to render yet
		if (attachment.startTick > tick) {
			return;
		}

		// Render STR attachment
		if (attachment.isStr && attachment.strEffect) {
			const strEffect = attachment.strEffect;
			// dynamic access to Renderer to avoid cycle
			const gl = Renderer.gl;

			try {
				strEffect.position = this.entity.position;
				strEffect.ownerDirection = this.entity.direction;

				if (!StrEffect.ready && gl) {
					StrEffect.init(gl);
				}

				if (gl) {
					// Pass true for disableDepthTest since this is an entity attachment
					StrEffect.beforeRender(gl, Camera.modelView, Camera.projection, _defaultFog, tick, true);
					strEffect.render(gl, tick);
					StrEffect.afterRender(gl);
				}
			} catch (e) {
				console.error('STR attachment error:', e);
				// Ensure WebGL state is restored even on error
				if (gl) {
					StrEffect.afterRender(gl);
				}
			}

			if (gl) {
				SpriteRenderer.bind3DContext(gl, Camera.modelView, Camera.projection, _defaultFog);
			}

			if (!strEffect.persistent && strEffect.needCleanUp) {
				return true;
			}

			return false;
		}

		let i, count;
		let frame;
		let animation, layers;
		let clean = false;

		const spr = Client.loadFile(attachment.spr);
		const act = Client.loadFile(attachment.act);

		if (!spr || !act || !spr.frames) {
			return clean;
		}

		this.entity.effectColor[3] = attachment.opacity;
		if (!attachment.position) {
			_position[0] = 0;
			_position[1] = attachment.head ? -100 : 0;
		} else {
			_position[0] = attachment.position[0];
			_position[1] = attachment.position[1];
		}

		frame = attachment.direction ? (Camera.direction + this.entity.direction + 8) % 8 : attachment.frame;
		frame %= act.actions.length;
		const animations = act.actions[frame].animations;
		const delay = attachment.delay || act.actions[frame].delay;
		SpriteRenderer.depth = attachment.depth || 0;

		// pause
		if ('animationId' in attachment) {
			layers = animations[attachment.animationId].layers;
		}

		// repeat animation
		else if (attachment.repeat) {
			if (attachment.duration > 0 && tick - attachment.startTick >= attachment.duration) {
				return true; // duration expired, remove attachment
			}
			layers = animations[Math.floor((tick - attachment.startTick) / delay) % animations.length].layers;
		}

		// stop at end
		else {
			animation = Math.min(Math.floor((tick - attachment.startTick) / delay), animations.length - 1);
			layers = animations[animation].layers;

			if (animation === animations.length - 1 && !attachment.stopAtEnd) {
				clean = true;
			}
		}

		// Render layers with depth ordering (renderBefore behind, normal in front)
		const self = this;
		const zIdx = attachment.renderBefore ? 1 : 500;

		SpriteRenderer.runWithDepth(true, false, false, function () {
			SpriteRenderer.zIndex = zIdx;
			for (i = 0, count = layers.length; i < count; ++i) {
				self.entity.renderLayer(layers[i], spr, spr, 1.0, _position, false);
			}
		});

		return clean;
	}
}
/**
 * Export
 */
export default function init() {
	this.attachments = new AttachmentManager(this);
}
