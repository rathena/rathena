/**
 * UI/Components/Captcha/CaptchaPreview.js
 *
 * Captcha Upload Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import 'UI/Elements/Elements.js';
import htmlText from './CaptchaPreview.html?raw';
import cssText from './CaptchaPreview.css?raw';

/**
 * Create Component
 */
const CaptchaPreview = new GUIComponent('CaptchaPreview', cssText);

/**
 * Preferences
 */
const _preferences = Preferences.get(
	'CaptchaPreview',
	{
		x: 230,
		y: 295
	},
	2.0
);

CaptchaPreview.render = () => htmlText;

/**
 * Initialize GUI
 */
CaptchaPreview.init = function init() {
	const root = this.getRoot();
	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', () => this.remove());
	}
	this.draggable('.titlebar');
};

/**
 * Append to DOM
 */
CaptchaPreview.onAppend = function onAppend() {
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth)}px`;
};

/**
 * Remove data from UI
 */
CaptchaPreview.onRemove = function onRemove() {
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();

	const root = this.getRoot();
	const previewBox = root.querySelector('.preview_box');
	if (previewBox) {
		previewBox.innerHTML = '';
	}
};

/**
 * Set Image
 */
CaptchaPreview.setImage = function setImage(imageData) {
	const blob = new Blob([imageData], { type: 'image/bmp' });
	const url = URL.createObjectURL(blob);

	const root = this.getRoot();
	const previewBox = root.querySelector('.preview_box');
	if (previewBox) {
		previewBox.innerHTML = '';
		const img = document.createElement('img');
		img.src = url;
		previewBox.appendChild(img);
	}
};

/**
 * Stored component and return it
 */
export default UIManager.addComponent(CaptchaPreview);
