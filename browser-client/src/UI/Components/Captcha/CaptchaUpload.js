/**
 * UI/Components/Captcha/CaptchaUpload.js
 *
 * Captcha Upload Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import DB from 'DB/DBManager.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import 'UI/Elements/Elements.js';
import htmlText from './CaptchaUpload.html?raw';
import cssText from './CaptchaUpload.css?raw';

/**
 * Create Component
 */
const CaptchaUpload = new GUIComponent('CaptchaUpload', cssText);

/**
 * Preferences
 */
const _preferences = Preferences.get(
	'CaptchaUpload',
	{
		x: 230,
		y: 295
	},
	2.0
);

CaptchaUpload.render = () => htmlText;

CaptchaUpload.captureKeyEvents = true;

/**
 * Initialize GUI
 */
CaptchaUpload.init = function init() {
	const root = this.getRoot();
	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', () => this.remove());
	}
	this.draggable('.titlebar');

	const browseBtn = root.querySelector('.btn_browse');
	const fileInput = root.querySelector('.captcha_file_input');

	if (browseBtn && fileInput) {
		browseBtn.addEventListener('click', () => {
			fileInput.click();
		});
	}

	if (fileInput) {
		fileInput.addEventListener('change', evt => {
			const file = evt.target.files[0];
			if (file) {
				const fileText = root.querySelector('.captcha_file_text');
				if (fileText) {
					fileText.value = file.name;
				}

				const reader = new FileReader();
				reader.onload = e => {
					const previewBox = root.querySelector('.preview_box');
					if (previewBox) {
						previewBox.innerHTML = '';
						const img = document.createElement('img');
						img.src = e.target.result;
						previewBox.appendChild(img);
					}
				};
				reader.readAsDataURL(file);

				Promise.resolve(this.compressImage(file)).then(imageData => {
					this.imageData = imageData;
				});
			}
		});
	}

	const answerInput = root.querySelector('.answer_input');
	if (answerInput) {
		answerInput.addEventListener('change', () => {
			if (answerInput.value.length > 16) {
				answerInput.value = answerInput.value.slice(0, 16);
			}
			this.answer = answerInput.value;
		});
	}

	const okBtn = root.querySelector('.ok');
	if (okBtn) {
		okBtn.addEventListener('click', () => {
			if (!this.answer || this.answer.length === 0) {
				UIManager.showMessageBox(DB.getMessage(2872), 'ok', () => {}, true);
				return;
			}

			if (!this.imageData) {
				UIManager.showMessageBox(DB.getMessage(2874), 'ok', () => {}, true);
				return;
			}

			UIManager.showPromptBox(DB.getMessage(2873).replace('%s', this.answer), 'ok', 'cancel', () => {
				if (this.requestUploadCaptcha && this.imageData) {
					this.requestUploadCaptcha(this.imageData.length, this.answer);
				}
			});
		});
	}
};

CaptchaUpload.onKeyDown = function onKeyDown(event) {
	if (CaptchaUpload.isEditableFocused()) {
		event.stopImmediatePropagation();
		return true;
	}
	return true;
};

/**
 * Append to DOM
 */
CaptchaUpload.onAppend = function onAppend() {
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth)}px`;
};

/**
 * Remove data from UI
 */
CaptchaUpload.onRemove = function onRemove() {
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();

	const root = this.getRoot();
	const previewBox = root.querySelector('.preview_box');
	if (previewBox) {
		previewBox.innerHTML = '';
	}
	root.querySelectorAll('input').forEach(input => {
		input.value = '';
	});

	this.answer = null;
	this.imageData = null;
};

CaptchaUpload.uploadError = function uploadError() {
	UIManager.showMessageBox(DB.getMessage(2881), 'ok', () => {}, true);
};

CaptchaUpload.uploadSuccess = function uploadSuccess() {
	UIManager.showMessageBox(DB.getMessage(2880), 'ok', () => {}, true);
};

/**
 * Callbacks
 */
CaptchaUpload.requestUploadCaptcha = function () {};
CaptchaUpload.uploadCaptcha = function () {};
CaptchaUpload.compressImage = function () {};

/**
 * Stored component and return it
 */
export default UIManager.addComponent(CaptchaUpload);
