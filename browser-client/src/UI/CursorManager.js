/**
 * UI/CursorManager.js
 *
 * Display Cursor
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Client from 'Core/Client.js';
import MemoryManager from 'Core/MemoryManager.js';
import Graphics from 'Preferences/Graphics.js';
import Sprite from 'Loaders/Sprite.js';
import Action from 'Loaders/Action.js';
import Preferences from 'Preferences/Controls.js';
import EntityManager from 'Renderer/EntityManager.js';
import Entity from 'Renderer/Entity/Entity.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Mouse from 'Controls/MouseEventHandler.js';

/**
 * @type {integer} tick
 */
let _tick = 0;

/**
 * @type {boolean} repeat animation ?
 */
let _norepeat = false;

/**
 * @type {integer} animation frame
 */
let _animation = 0;

/**
 * @type {boolean} play animation ?
 */
let _play = true;

/**
 * @type {number} last style id rendered
 */
let _lastStyleId = -1;

/**
 * @type {number} last rendered position x
 */
let _lastX = 0;

/**
 * @type {number} last renderer position y
 */
let _lastY = 0;

/**
 * @type {Array} css style list
 */
let _compiledStyle = [];

/**
 * @type {Sprite} sprite
 */
let _sprite;

/**
 * @type {Action} action
 */
let _action;

/**
 * @type {reference} selector
 */
let _selector;

/**
 * Cursor Constructor
 */
class Cursor {
	/**
	 * Cursor animation Constant
	 */
	static ACTION = {
		DEFAULT: 0,
		TALK: 1,
		CLICK: 2,
		LOCK: 3,
		ROTATE: 4,
		ATTACK: 5,
		WARP: 7,
		PICK: 9,
		TARGET: 10,
		NOWALK: 13
	};

	/**
	 * @type {boolean} block change ?
	 */
	static freeze = false;

	/**
	 * @type {integer} left in px
	 */
	static x = 0;

	/**
	 * @type {integer} top in px
	 */
	static y = 0;

	/**
	 * @type {boolean} magnetism while picking entites ?
	 */
	static magnetism = true;

	/**
	 * @type {boolean} force disabled magnetism
	 * Used to cast zone skill to ground
	 */
	static blockMagnetism = false;

	/**
	 * Load cursor data (action, sprite)
	 */
	static init(fn) {
		// Already loaded
		if (_sprite) {
			fn();
			return;
		}

		Client.getFiles(['data/sprite/cursors.spr', 'data/sprite/cursors.act'], (spr, act) => {
			try {
				_sprite = new Sprite(spr);
				_action = new Action(act);
			} catch (e) {
				console.error('Cursor::init() - ' + e.message);
				return;
			}

			// Load it properly later using webgl
			MemoryManager.remove(null, 'data/sprite/cursors.spr');
			MemoryManager.remove(null, 'data/sprite/cursors.act');

			bindMouseEvents();
			preCompiledAnimations();
			createSpriteSheet();
			fn();
		});
	}

	/**
	 * Change cursor action
	 *
	 * @param {number} type - Cursor.ACTION.*
	 * @param {boolean} norepeat - repeat animation ?
	 * @param {number} animation numero (optional)
	 */
	static setType(type, norepeat, animation) {
		if (Cursor.freeze) {
			return;
		}

		_type = type;
		_tick = Date.now();
		_norepeat = !!norepeat;

		if (typeof animation !== 'undefined') {
			_animation = animation;
			_play = false;
		} else {
			_animation = 0;
			_play = true;
		}
	}

	/**
	 * Simple method to get the current cursor type
	 *
	 * @return {number} Cursor.ACTION.*
	 */
	static getActualType() {
		return _type;
	}

	/**
	 * Render the cursor (update)
	 */
	static render(tick) {
		if (!Graphics.cursor || !_compiledStyle.length) {
			if (_selector) {
				// Pre-rework it used 'hidden' css
				_selector.style.display = 'none';
			}
			return;
		}
		// Pre-rework it used 'show' css, need to check if it exist
		if (_selector && _selector.style.display === 'none') {
			_selector.style.display = 'block';
		}

		const info = ActionInformations[_type] || ActionInformations[Cursor.ACTION.DEFAULT];
		const action = _action.actions[_type] || _action.actions[Cursor.ACTION.DEFAULT];
		let anim = _animation;
		const delay = action.delay * info.delayMult;
		let x = info.startX;
		let y = info.startY;

		if (_play) {
			const frame = Math.floor((tick - _tick) / delay);
			anim = _norepeat ? Math.min(frame, action.animations.length - 1) : frame % action.animations.length;
		}

		if (Graphics.cursor) {
			document.body.classList.add('custom-cursor');
		}

		const animation = action.animations[anim];

		if (!animation) {
			return;
		}

		if (Cursor.magnetism && !Cursor.blockMagnetism) {
			const entity = EntityManager.getOverEntity();
			if (entity) {
				switch (entity.objecttype) {
					case Entity.TYPE_MOB:
					case Entity.TYPE_NPC_ABR:
					case Entity.TYPE_NPC_BIONIC:
						if (!Preferences.snap) {
							break;
						}
						x += Math.floor(
							Mouse.screen.x -
								(entity.boundingRect.x1 + (entity.boundingRect.x2 - entity.boundingRect.x1) / 2)
						);
						y += Math.floor(
							Mouse.screen.y -
								(entity.boundingRect.y1 + (entity.boundingRect.y2 - entity.boundingRect.y1) / 2)
						);
						break;
					case Entity.TYPE_ITEM:
						if (!Preferences.itemsnap) {
							break;
						}
						x += Math.floor(
							Mouse.screen.x -
								(entity.boundingRect.x1 + (entity.boundingRect.x2 - entity.boundingRect.x1) / 2)
						);
						y += Math.floor(
							Mouse.screen.y -
								(entity.boundingRect.y1 + (entity.boundingRect.y2 - entity.boundingRect.y1) / 2)
						);
						break;
					default:
						break;
				}
			}
		}

		if (animation.compiledStyleIndex !== _lastStyleId || x !== _lastX || y !== _lastY) {
			_lastStyleId = animation.compiledStyleIndex;
			_lastX = x;
			_lastY = y;

			const cursorSprite = document.querySelector('.cursor__sprite');
			if (cursorSprite) {
				cursorSprite.style.left = `${-_lastStyleId * 50}px`;
			}

			const cursor = document.querySelector('.cursor');
			if (cursor) {
				cursor.style.transform = `translate(-${_lastX}px, -${_lastY}px)`;
			}
		}
	}
}

/**
 * @type {integer} Cursor.ACTION.* constant
 */
let _type = Cursor.ACTION.DEFAULT;

/**
 * Define sprite informations (hardcoded)
 */
const ActionInformations = {};
ActionInformations[Cursor.ACTION.DEFAULT] = { drawX: 1, drawY: 19, startX: 0, startY: 0, delayMult: 2.0 };
ActionInformations[Cursor.ACTION.TALK] = { drawX: 20, drawY: 40, startX: 20, startY: 20, delayMult: 1.0 };
ActionInformations[Cursor.ACTION.WARP] = { drawX: 10, drawY: 32, startX: 0, startY: 0, delayMult: 1.0 };
ActionInformations[Cursor.ACTION.ROTATE] = { drawX: 18, drawY: 26, startX: 10, startY: 0, delayMult: 1.0 };
ActionInformations[Cursor.ACTION.PICK] = { drawX: 20, drawY: 40, startX: 15, startY: 15, delayMult: 1.0 };
ActionInformations[Cursor.ACTION.TARGET] = { drawX: 20, drawY: 50, startX: 20, startY: 28, delayMult: 0.5 };
ActionInformations[Cursor.ACTION.NOWALK] = { drawX: 13, drawY: 25, startX: 14, startY: 6, delayMult: 1.0 };

/**
 * Change the cursor for the button click event
 */
function bindMouseEvents() {
	const cursorCSS = `
		.custom-cursor * { cursor: none!important; }
		.custom-cursor .cursor { display: block; }
		.cursor { pointer-events: none; z-index: 9999; position: fixed; width: 50px; height: 50px; overflow: hidden; display: none; }
		.cursor__sprite { position: absolute; top: 0; left: 0; }
	`;
	const styleEl = document.createElement('style');
	styleEl.type = 'text/css';
	styleEl.textContent = cursorCSS;
	document.head.appendChild(styleEl);

	const cursorDiv = document.createElement('div');
	cursorDiv.className = 'cursor';
	document.body.appendChild(cursorDiv);
	_selector = document.querySelector('.cursor');

	const CLICKABLE_SELECTOR = [
		'a',
		'button',
		'ui-button',
		'input',
		'label',
		'select',
		'textarea',
		'.item-link',
		'.draggable',
		'.ro-custom-scrollbar',
		'.ro-custom-scrollbar *'
	].join(',');

	let _hasClickableHover = false;
	let _restoreType = Cursor.ACTION.DEFAULT;

	const findClickableTarget = target => {
		if (!target) {
			return null;
		}

		// Text node → element
		if (target.nodeType && target.nodeType !== 1) {
			target = target.parentElement;
		}

		if (!target) {
			return null;
		}

		// Tabs are clickable UI, but should not override the game cursor to "click".
		if (target.closest && target.closest('#chatbox td.tab')) {
			return null;
		}

		if (target.closest) {
			return target.closest(CLICKABLE_SELECTOR);
		}

		while (target && target !== document.body) {
			if (target.matches && target.matches(CLICKABLE_SELECTOR)) {
				return target;
			}
			target = target.parentElement;
		}

		return null;
	};

	const setCursorClick = (norepeat, animation) => {
		if (norepeat) {
			Cursor.setType(Cursor.ACTION.CLICK, true, animation);
			return;
		}

		if (Cursor.getActualType() !== Cursor.ACTION.CLICK) {
			Cursor.setType(Cursor.ACTION.CLICK);
		}
	};

	document.body.addEventListener('mouseover', e => {
		if (findClickableTarget(e.target)) {
			if (!_hasClickableHover) {
				_restoreType = Cursor.getActualType();
				_hasClickableHover = true;
			}
			setCursorClick(false, 0);
		}
	});

	document.body.addEventListener('mouseout', e => {
		if (!_hasClickableHover) {
			return;
		}

		// Ignore transitions between clickable elements.
		if (findClickableTarget(e.relatedTarget)) {
			return;
		}

		_hasClickableHover = false;
		Cursor.setType(_restoreType);
	});

	document.body.addEventListener(
		'mousedown',
		e => {
			if (findClickableTarget(e.target)) {
				if (!_hasClickableHover) {
					_restoreType = Cursor.getActualType();
					_hasClickableHover = true;
				}
				setCursorClick(true, 1);
			}
		},
		true
	);

	document.body.addEventListener('mouseup', e => {
		if (findClickableTarget(e.target)) {
			setCursorClick(false, 0);
			return;
		}

		if (_hasClickableHover) {
			_hasClickableHover = false;
			Cursor.setType(_restoreType);
		}
	});

	window.addEventListener(
		'pointermove',
		e => {
			Cursor.x = e.pageX;
			Cursor.y = e.pageY;
			_selector.style.left = `${e.pageX}px`;
			_selector.style.top = `${e.pageY}px`;
		},
		true
	);
}

/**
 * Start pre-compiling animation to avoid building sprites
 * during the rendering loop
 */
function preCompiledAnimations() {
	let i, j, k, count, size, total, pos;
	let action, animation, info;
	let binary, data, dataURI;

	// Start initializing variables
	const canvas = document.createElement('canvas');
	canvas.width = 50;
	canvas.height = 50;
	const ctx = canvas.getContext('2d');
	const entity = new Entity();
	const dataURIList = [];
	_compiledStyle = [];
	const position = [0, 0];

	// Start compiling animation
	for (i = 0, count = _action.actions.length; i < count; ++i) {
		action = _action.actions[i];
		info = ActionInformations[i] || ActionInformations[Cursor.ACTION.DEFAULT];

		for (j = 0, size = action.animations.length; j < size; ++j) {
			animation = action.animations[j];

			// Initialize context
			SpriteRenderer.bind2DContext(ctx, info.drawX, info.drawY);
			ctx.clearRect(0, 0, 50, 50);

			// // add borders to debug
			// ctx.strokeStyle = 'red';
			// ctx.strokeRect(0, 0, 50, 50);

			// Render layers
			for (k = 0, total = animation.layers.length; k < total; ++k) {
				entity.renderLayer(animation.layers[k], _sprite, _sprite, 1.0, position, false);
			}

			dataURI = canvas.toDataURL('image/png');
			pos = dataURIList.indexOf(dataURI);

			// Already build
			if (pos > -1) {
				animation.compiledStyleIndex = pos;
				continue;
			}

			// Modify the canvas to a file object
			binary = atob(dataURI.replace(/^data[^,]+,/, ''));
			total = binary.length;
			data = new Uint8Array(total);

			for (k = 0; k < total; ++k) {
				data[k] = binary.charCodeAt(k);
			}

			// Store it.
			animation.compiledStyleIndex = _compiledStyle.length;

			dataURIList.push(dataURI);

			const blobURL = URL.createObjectURL(new Blob([data.buffer], { type: 'image/png' }));

			_compiledStyle.push(blobURL);
		}
	}
}

/**
 * Creates a sprite sheet from a list of image URLs and appends it to the DOM.
 *
 * Creates a canvas, draws each sprite in a row, converts the canvas to a data URL,
 * and then creates a Blob URL to display the sprite sheet.
 */
function createSpriteSheet() {
	const spriteWidth = 50,
		spriteHeight = 50;
	const totalSprites = _compiledStyle.length;
	const spriteSheetWidth = totalSprites * spriteWidth;
	const spriteSheetHeight = spriteHeight;

	// Create a canvas to hold the sprite sheet
	const spriteSheetCanvas = document.createElement('canvas');
	spriteSheetCanvas.width = spriteSheetWidth;
	spriteSheetCanvas.height = spriteSheetHeight;
	const ctx = spriteSheetCanvas.getContext('2d');

	let imagesLoaded = 0;

	function drawSprite(n) {
		const img = new Image();
		img.decoding = 'async';
		img.onload = function () {
			ctx.drawImage(img, n * spriteWidth, 0, spriteWidth, spriteHeight);
			imagesLoaded++;
			if (imagesLoaded === totalSprites) {
				finalizeSpriteSheet();
			}
		};
		img.src = _compiledStyle[n];
	}

	function finalizeSpriteSheet() {
		// Convert the sprite sheet canvas to a data URL
		const spriteSheetDataURL = spriteSheetCanvas.toDataURL('image/png');

		// Create a Blob from the sprite sheet data URL
		const binary = atob(spriteSheetDataURL.split(',')[1]);
		const array = [];
		for (let i = 0; i < binary.length; i++) {
			array.push(binary.charCodeAt(i));
		}
		const blob = new Blob([new Uint8Array(array)], { type: 'image/png' });
		const spriteSheetBlobURL = URL.createObjectURL(blob);

		// Append the sprite sheet to the DOM
		const imageElem = document.createElement('img');
		imageElem.className = 'cursor__sprite';
		imageElem.src = spriteSheetBlobURL;
		imageElem.addEventListener(
			'load',
			e => {
				URL.revokeObjectURL(e.target.src);
			},
			{ once: true }
		);
		const cursorEl = document.querySelector('.cursor');
		if (cursorEl) {
			cursorEl.appendChild(imageElem);
		}
	}

	for (let i = 0; i < totalSprites; i++) {
		drawSprite(i);
	}
}

/**
 * Export
 */
export default Cursor;
