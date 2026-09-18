/**
 * UI/Components/Announce/Announce.js
 *
 * Announce GUI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Events from 'Core/Events.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './Announce.html?raw';
import cssText from './Announce.css?raw';

/**
 * Create Announce component
 */
const Announce = new GUIComponent('Announce', cssText);

/**
 * Mouse can cross this UI
 */
Announce.mouseMode = GUIComponent.MouseMode.CROSS;

/**
 * @var {boolean} do not focus this UI
 */
Announce.needFocus = false;

/**
 * @var {TimeOut} timer
 */
let _timer = 0;

/**
 * @var {number} how many time the announce is display (20secs)
 */
const _life = 20 * 1000;

Announce.render = () => htmlText;

/**
 * Initialize component
 */
Announce.init = function init() {
	const root = this.getRoot();
	this.canvas = root.querySelector('canvas');
	this.ctx = this.canvas.getContext('2d');
};

/**
 * Once removed from HTML, clean timer
 */
Announce.onRemove = function onRemove() {
	if (_timer) {
		Events.clearTimeout(_timer);
		_timer = 0;
	}
};

/**
 * Timer end, cleaning announce
 */
Announce.timeEnd = function timeEnd() {
	this.remove();
};

/**
 * Add an announce with text and color
 *
 * @param {string} text to display
 * @param {string} color
 */
Announce.set = function set(text, color, options = {}) {
	const allowNewlines = typeof options === 'boolean' ? options : !!options.allowNewlines;
	const opts = typeof options === 'object' ? options : {};
	const fontSize = opts.fontSize || 12;
	const life = opts.life || _life;

	let targetWidth = null;
	if (opts.width === '100%') {
		targetWidth = Renderer.width;
	} else if (opts.width) {
		targetWidth = opts.width;
	}

	const maxWidth = targetWidth ? targetWidth - 20 : 500;
	const lines = [];

	this.ctx.font = `${fontSize}px Arial`;

	if (allowNewlines) {
		text.split('\n').forEach(line => {
			const words = line.split(' ');
			let currentLine = '';

			words.forEach(word => {
				const testLine = `${currentLine}${word} `;
				if (this.ctx.measureText(testLine).width > maxWidth) {
					lines.push(currentLine.trim());
					currentLine = `${word} `;
				} else {
					currentLine = testLine;
				}
			});

			if (currentLine.trim()) {
				lines.push(currentLine.trim());
			}
		});
	} else {
		let currentLine = '';
		text.split(' ').forEach(word => {
			const testLine = `${currentLine}${word} `;
			if (this.ctx.measureText(testLine).width > maxWidth) {
				lines.push(currentLine.trim());
				currentLine = `${word} `;
			} else {
				currentLine = testLine;
			}
		});

		if (currentLine.trim()) {
			lines.push(currentLine.trim());
		}
	}

	this.canvas.width = targetWidth || 20 + Math.max(...lines.map(line => this.ctx.measureText(line).width));
	this.canvas.height = opts.height || 10 + (fontSize + 5) * lines.length;

	if (opts.width === '100%') {
		this._host.style.left = '0px';
	} else {
		this._host.style.left = `${(Renderer.width - this.canvas.width) >> 1}px`;
	}

	this.ctx.font = `${fontSize}px Arial`;

	if (!opts.noBackground) {
		this.ctx.fillStyle = 'rgba(0,0,0,0.5)';
		this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
	}

	this.ctx.fillStyle = color || '#FFFF00';

	if (targetWidth || opts.height) {
		this.ctx.textAlign = 'center';
		this.ctx.textBaseline = 'middle';
		lines.forEach((line, index) => {
			const y = this.canvas.height / 2 + (index - (lines.length - 1) / 2) * (fontSize + 5);
			this.ctx.fillText(line, this.canvas.width / 2, y);
		});
	} else {
		this.ctx.textAlign = 'left';
		this.ctx.textBaseline = 'alphabetic';
		lines.forEach((line, index) => {
			this.ctx.fillText(line, 10, 5 + fontSize + (fontSize + 5) * index);
		});
	}

	if (_timer) {
		Events.clearTimeout(_timer);
	}

	_timer = Events.setTimeout(this.timeEnd.bind(this), life);
};

/**
 * Create component and return it
 */
export default UIManager.addComponent(Announce);
