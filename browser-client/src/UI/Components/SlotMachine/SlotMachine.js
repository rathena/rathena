/**
 * UI/Components/SlotMachine/SlotMachine.js
 *
 * SlotMachine window
 * Used to catch pet
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import GUIComponent from 'UI/GUIComponent.js';
import UIManager from 'UI/UIManager.js';
import Client from 'Core/Client.js';
import Events from 'Core/Events.js';
import Renderer from 'Renderer/Renderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import htmlText from './SlotMachine.html?raw';
import cssText from './SlotMachine.css?raw';

/**
 * Create SlotMachine UI
 */
const SlotMachine = new GUIComponent('SlotMachine', cssText);

SlotMachine.render = () => htmlText;

/**
 * @var {Sprite,Action} objects
 */
let _sprite, _action;

/**
 * @var {number} type
 */
let _type = 0;

/**
 * @var {boolean} result
 */
let _result = false;

/**
 * @var {CanvasRenderingContext2D}
 */
let _ctx;

/**
 * @var {Entity}
 */
const _entity = new Entity();

/**
 * @var {number} start tick
 */
let _start = 0;

/**
 * Initialize UI
 */
SlotMachine.init = function init() {
	const root = this.getRoot();
	const canvas = root.querySelector('canvas');
	_ctx = canvas.getContext('2d');

	this._host.style.zIndex = '500';

	Client.loadFiles(['data/sprite/slotmachine.spr', 'data/sprite/slotmachine.act'], (spr, act) => {
		_sprite = spr;
		_action = act;
		_type = 0;
		_start = Renderer.tick;

		if (this._host.parentNode) {
			Renderer.render(rendering);
		}
	});

	canvas.addEventListener('mousedown', event => {
		if (_type === 0) {
			SlotMachine.onTry();
			event.stopImmediatePropagation();
			event.preventDefault();
		}
	});
};

/**
 * Once append to body
 */
SlotMachine.onAppend = function onAppend() {
	_type = 0;
	_start = Renderer.tick;

	this._host.style.top = `${Renderer.height / 2 - 130}px`;
	this._host.style.left = `${Renderer.width / 2 - 135}px`;

	if (_sprite && _action) {
		Renderer.render(rendering);
	}
};

/**
 * Once removed from body
 */
SlotMachine.onRemove = function onRemove() {
	Renderer.stop(rendering);
};

/**
 * Set the result
 *
 * @param {boolean} result
 */
SlotMachine.setResult = function setResult(result) {
	_type = 1;
	_result = result;
	_start = Renderer.tick;
};

/**
 * Rendering animation
 */
const rendering = (() => {
	const position = new Uint16Array([0, 0]);

	return () => {
		let action, anim;

		switch (_type) {
			case 0: // waiting
				action = _action.actions[0];
				anim = Math.floor(((Renderer.tick - _start) / action.delay) * 2);
				break;

			case 1: {
				// pending
				action = _action.actions[1];
				const max = action.animations.length + (_result ? 7 : 3);
				anim = Renderer.tick - _start;
				anim = Math.floor(anim / action.delay);
				anim = Math.min(anim, max);

				if (anim === max) {
					_type = _result ? 2 : 3;
					_start = Renderer.tick;
				}
				break;
			}

			case 2: // success
			case 3: {
				// fail
				action = _action.actions[_type];
				const max = action.animations.length;
				anim = Renderer.tick - _start;
				anim = Math.floor(anim / action.delay);

				if (anim >= max) {
					Renderer.stop(rendering);
					Events.setTimeout(() => {
						SlotMachine.remove();
					}, 500);
				}
				break;
			}
		}

		const animation = action.animations[anim % action.animations.length];

		SpriteRenderer.bind2DContext(_ctx, 140, 165);
		_ctx.clearRect(0, 0, _ctx.canvas.width, _ctx.canvas.height);

		for (let i = 0, count = animation.layers.length; i < count; ++i) {
			_entity.renderLayer(animation.layers[i], _sprite, _sprite, 1.0, position, false);
		}
	};
})();

/**
 * Functions defined in Engine/MapEngine/Pet.js
 */
SlotMachine.onTry = function onTry() {};

SlotMachine.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Export
 */
export default UIManager.addComponent(SlotMachine);
