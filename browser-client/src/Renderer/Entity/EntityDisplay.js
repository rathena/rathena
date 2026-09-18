/**
 * Renderer/EntityDisplay.js
 *
 * Manage Entity Display (pseudo + guild + party)
 * Writing to canvas is very ugly, this file contain some hack to get some best results on Firefox and Chrome.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Utils/gl-matrix.js';
import MapPreferences from 'Preferences/Map.js';
import EntityOverlay from 'Renderer/Entity/EntityOverlay.js';

/**
 * Global methods
 */
const vec4 = glMatrix.vec4;
const _pos = new Float32Array(4);
const _size = new Float32Array(2);
const dpr = window.devicePixelRatio || 1;

const procCanvas = document.createElement('canvas');
const procCtx = procCanvas.getContext('2d', { willReadFrequently: true });

// Some helper for Firefox to render text-border
if (typeof CanvasRenderingContext2D !== 'undefined') {
	CanvasRenderingContext2D.prototype.outlineText = function outlineText(txt, x, y) {
		this.fillText(txt, x - 1, y);
		this.fillText(txt, x, y - 1);
		this.fillText(txt, x + 1, y);
		this.fillText(txt, x, y + 1);
	};
}

// Some helper for Chrome to render text-border
function multiShadow(ctx, text, x, y, offsetX, offsetY, blur) {
	ctx.textBaseline = 'top';
	ctx.lineWidth = 1;
	ctx.shadowColor = '#000';
	ctx.shadowBlur = blur;
	ctx.shadowOffsetX = offsetX;
	ctx.shadowOffsetY = offsetY;
	ctx.fillStyle = 'black';
	ctx.fillText(text, x, y);
}

/**
 * @var {boolean} is the shadow ugly in the GPU ?
 * Used to fallback to another renderer.
 *
 * For more informations, check :
 * http://forum.robrowser.com/index.php?topic=32200
 */
const _isUglyShadow = (function isUglyGPUShadow() {
	const fontSize = 12 * dpr;
	const text = 'Testing';

	// Create canvas
	procCtx.font = fontSize + 'px Arial';

	const width = procCtx.measureText(text).width + 10;
	const height = fontSize * 3 * (text.length ? 2 : 1);

	procCanvas.width = width;
	procCanvas.height = height;

	procCtx.font = fontSize + 'px Arial';
	procCtx.textBaseline = 'top';

	// Render text and shadows
	multiShadow(procCtx, text, 5, 0, 0, -1, 0);
	multiShadow(procCtx, text, 5, 0, 0, 1, 0);
	multiShadow(procCtx, text, 5, 0, -1, 0, 0);
	multiShadow(procCtx, text, 5, 0, 1, 0, 0);
	procCtx.fillStyle = 'white';
	procCtx.strokeStyle = 'black';
	procCtx.strokeText(text, 5, 0);
	procCtx.fillText(text, 5, 0);

	// Read canvas pixels and get the average black
	const imageData = procCtx.getImageData(0, 0, width, height);
	const pixels = imageData.data;
	let total = 0;

	for (let i = 0; i < pixels.length; i += 4) {
		total += ((255 - pixels[i]) / 255) * pixels[i + 3];
	}

	const percent = total / (pixels.length / 4) / 2.55;

	procCanvas.width = procCanvas.height = 0;

	// 6.1% seems for the moment a good value
	// to check if there is too much black.
	return !window.chrome || percent > 6.15;
})();

/**
/**
 * Display class — renders entity nameplate (name, party, guild, title, emblem)
 *
 * @class Display
 * @property {string} name Entity character/monster/NPC name
 * @property {string} fakename Fake name override
 * @property {string} party_name Party name string
 * @property {string} guild_name Guild name string
 * @property {string} guild_rank Guild rank title
 * @property {string} title_name Achievement / player title
 * @property {Image|null} emblem Static guild emblem image
 * @property {HTMLCanvasElement|null} gifEmblem Animated guild emblem canvas
 * @property {boolean} display Whether nameplate is currently visible
 * @property {HTMLCanvasElement} canvas Nameplate canvas element
 * @property {CanvasRenderingContext2D} ctx 2d context for nameplate
 */
class Display {
	constructor() {
		this.TYPE = {
			NONE: 0,
			LOADING: 1,
			COMPLETE: 2
		};

		this.STYLE = {
			DEFAULT: 0,
			MOB: 1,
			NPC: 2,
			ITEM: 3,
			ADMIN: 4
		};

		this.load = this.TYPE.NONE;
		this.name = '';
		this.fakename = '';
		this.party_name = '';
		this.guild_name = '';
		this.guild_rank = '';
		this.title_name = '';
		this.emblem = null;
		this.gifEmblem = null;
		this.display = false;
		this.canvas = document.createElement('canvas');
		this.canvas.className = 'entity-display';
		this.ctx = this.canvas.getContext('2d');
		this.canvas.style.position = 'absolute';
		this.canvas.style.zIndex = 1;
	}

	/**
	 * Set emblem image and optional animated canvas (GIF)
	 * @param {Image} image - static emblem image
	 * @param {Canvas|null} animatedCanvas - GIF spritesheet with metadata
	 */
	setEmblem(image, animatedCanvas) {
		this.emblem = image;
		this.gifEmblem = animatedCanvas || null;
	}

	/**
	 * Add GUI to html
	 */
	add() {
		this.display = true;
	}

	/**
	 * Remove GUI from html
	 */
	remove() {
		this.canvas.remove();
		this.display = false;
	}

	/**
	 * Clean it (remove informations)
	 */
	clean() {
		this.remove();
	}

	/**
	 * Update the display
	 * @param {string} color
	 */
	update(style) {
		style = style || this.STYLE.DEFAULT;

		// Setup variables
		const lines = new Array(2);
		const fontSize = 12 * dpr;
		const ctx = this.ctx;
		const start_x =
			(this.emblem &&
			(style === this.STYLE.DEFAULT ||
				style === this.STYLE.ADMIN ||
				style === this.STYLE.MOB ||
				style === this.STYLE.NPC)
				? 26
				: 0) + 5;
		const paddingTop = 5;

		// Skip the "#" in the pseudo
		lines[0] = this.fakename ? this.fakename.split('#')[0] : this.name.split('#')[0];
		lines[1] = '';

		// Add the party name
		if (
			this.party_name.length &&
			(style === this.STYLE.DEFAULT ||
				style === this.STYLE.ADMIN ||
				style === this.STYLE.MOB ||
				style === this.STYLE.NPC)
		) {
			lines[0] += ' (' + this.party_name + ')';
		}

		// Add guild name
		if (
			this.guild_name.length &&
			(style === this.STYLE.DEFAULT ||
				style === this.STYLE.ADMIN ||
				style === this.STYLE.MOB ||
				style === this.STYLE.NPC)
		) {
			lines[1] = this.guild_name;

			// Add guild rank
			if (this.guild_rank.length && MapPreferences.showname) {
				lines[1] += ' [' + this.guild_rank + ']';
			}
		} else if (
			this.guild_rank.length &&
			(style === this.STYLE.DEFAULT ||
				style === this.STYLE.ADMIN ||
				style === this.STYLE.MOB ||
				style === this.STYLE.NPC)
		) {
			// showname
			lines[1] = this.guild_rank;
		}

		// Add Title name
		// when title is set client render title in line[0] and name/fakename in line[1]
		if (
			MapPreferences.showname &&
			this.title_name.length &&
			(style === this.STYLE.DEFAULT ||
				style === this.STYLE.ADMIN ||
				style === this.STYLE.MOB ||
				style === this.STYLE.NPC)
		) {
			lines[0] = '[' + this.title_name + '] ' + lines[0];
		}

		// Setup the canvas
		const fontBold = MapPreferences.showname ? 'bold ' : '';
		ctx.font = fontBold + fontSize + 'px Arial';

		const width = Math.max(ctx.measureText(lines[0]).width, ctx.measureText(lines[1]).width) + start_x + 5;
		const height = fontSize * 3 * (lines[1].length ? 2 : 1) + paddingTop;
		ctx.canvas.width = width;
		ctx.canvas.height = height;

		// Draw emblem
		if (
			this.emblem &&
			(style === this.STYLE.DEFAULT ||
				style === this.STYLE.ADMIN ||
				style === this.STYLE.MOB ||
				style === this.STYLE.NPC)
		) {
			if (this.gifEmblem) {
				const fw = this.gifEmblem.frameWidth;
				const fh = this.gifEmblem.frameHeight;
				ctx.drawImage(this.gifEmblem, 0, 0, fw, fh, 0, paddingTop, 24, 24);
			} else {
				ctx.drawImage(this.emblem, 0, paddingTop, 24, 24);
			}
		}

		// TODO: complete the color list in the Entity display
		let color = 'white';
		switch (style) {
			case this.STYLE.MOB:
				color = '#ffc6c6';
				break;
			case this.STYLE.NPC:
				color = '#94bdf7';
				break;
			case this.STYLE.ITEM:
				color = '#FFEF94';
				break;
			case this.STYLE.ADMIN:
				color = '#ffff00';
				break;
		}

		const fontBold2 = MapPreferences.showname ? 'bold ' : '';
		ctx.font = fontBold2 + fontSize + 'px Arial';
		ctx.textBaseline = 'top';

		// Shadow renderer
		if (!_isUglyShadow) {
			multiShadow(ctx, lines[0], start_x, paddingTop, 0, -1, 0);
			multiShadow(ctx, lines[0], start_x, paddingTop, 0, 1, 0);
			multiShadow(ctx, lines[0], start_x, paddingTop, -1, 0, 0);
			multiShadow(ctx, lines[0], start_x, paddingTop, 1, 0, 0);
			multiShadow(ctx, lines[1], start_x, fontSize * 1.2 + paddingTop, 0, -1, 0);
			multiShadow(ctx, lines[1], start_x, fontSize * 1.2 + paddingTop, 0, 1, 0);
			multiShadow(ctx, lines[1], start_x, fontSize * 1.2 + paddingTop, -1, 0, 0);
			multiShadow(ctx, lines[1], start_x, fontSize * 1.2 + paddingTop, 1, 0, 0);
			ctx.fillStyle = color;
			ctx.strokeStyle = 'black';
			ctx.strokeText(lines[0], start_x, 0 + paddingTop);
			ctx.fillText(lines[0], start_x, paddingTop);
			ctx.strokeText(lines[1], start_x, fontSize * 1.2 + paddingTop);
			ctx.fillText(lines[1], start_x, fontSize * 1.2 + paddingTop);
		}

		// fillText renderer
		else {
			ctx.translate(0.5, 0.5);
			ctx.fillStyle = 'black';
			ctx.outlineText(lines[0], start_x, paddingTop);
			ctx.outlineText(lines[1], start_x, fontSize * 1.2 + paddingTop);
			ctx.fillStyle = color;
			ctx.fillText(lines[0], start_x, paddingTop);
			ctx.fillText(lines[1], start_x, fontSize * 1.2 + paddingTop);
		}
	}

	/**
	 * Refreshes the display (when player uses /showname)
	 */
	refresh(entity) {
		this.update(
			entity.objecttype === entity.constructor.TYPE_MOB
				? entity.display.STYLE.MOB
				: entity.objecttype === entity.constructor.TYPE_NPC_ABR
					? entity.display.STYLE.MOB
					: entity.objecttype === entity.constructor.TYPE_NPC_BIONIC
						? entity.display.STYLE.MOB
						: entity.objecttype === entity.constructor.TYPE_DISGUISED
							? entity.display.STYLE.MOB
							: entity.objecttype === entity.constructor.TYPE_NPC
								? entity.display.STYLE.NPC
								: entity.objecttype === entity.constructor.TYPE_NPC2
									? entity.display.STYLE.NPC
									: entity.objecttype === entity.constructor.TYPE_ITEM
										? entity.display.STYLE.ITEM
										: entity.objecttype === entity.constructor.TYPE_PC && entity.isAdmin
											? entity.display.STYLE.ADMIN
											: entity.display.STYLE.DEFAULT
		);
	}

	/**
	 * Rendering GUI
	 */
	render(matrix) {
		if (this.gifEmblem) {
			const paddingTop = 5;
			const now = Date.now();

			const currentFrameIndex = this.gifEmblem.currentFrame || 0;
			const frameDelay = this.gifEmblem.frameDelays ? this.gifEmblem.frameDelays[currentFrameIndex] : 100;

			if (now - this.gifEmblem.lastFrameChange >= frameDelay) {
				this.gifEmblem.lastFrameChange = now;

				const fw = this.gifEmblem.frameWidth;
				const fh = this.gifEmblem.frameHeight;
				const fpr = this.gifEmblem.framesPerRow || Math.floor(this.gifEmblem.width / fw);
				const total = this.gifEmblem.frameCount || fpr * Math.floor(this.gifEmblem.height / fh);

				this.gifEmblem.currentFrame = (this.gifEmblem.currentFrame + 1) % total;

				const col = this.gifEmblem.currentFrame % fpr;
				const row = Math.floor(this.gifEmblem.currentFrame / fpr);

				this.ctx.save();
				this.ctx.setTransform(1, 0, 0, 1, 0, 0);
				this.ctx.clearRect(0, paddingTop * dpr, 24 * dpr, 24 * dpr);
				this.ctx.restore();

				// updates gif image only when needed
				this.ctx.drawImage(this.gifEmblem, col * fw, row * fh, fw, fh, 0, paddingTop, 24, 24);
			}
		}

		const canvas = this.canvas;
		// Cast position
		_pos[0] = 0.0;
		_pos[1] = -0.5;
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

		canvas.style.top = ((_pos[1] + 13) | 0) + 'px';
		canvas.style.left = ((_pos[0] - canvas.width / dpr / 2) | 0) + 'px';
		canvas.style.width = canvas.width / dpr + 'px';
		canvas.style.height = canvas.height / dpr + 'px';

		// Append to the clipped overlay layer
		EntityOverlay.append(canvas);
	}
}

/**
 * Export ing
 */
export default function Init() {
	this.display = new Display();
}
