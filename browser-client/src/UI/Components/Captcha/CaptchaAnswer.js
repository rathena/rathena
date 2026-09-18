/**
 * UI/Components/Captcha/CaptchaAnswer.js
 *
 * Captcha Answer Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import DB from 'DB/DBManager.js';
import 'UI/Elements/Elements.js';
import htmlText from './CaptchaAnswer.html?raw';
import cssText from './CaptchaAnswer.css?raw';

/**
 * Create Component
 */
const CaptchaAnswer = new GUIComponent('CaptchaAnswer', cssText);

/**
 * Preferences
 */
const _preferences = Preferences.get(
	'CaptchaAnswer',
	{
		x: 230,
		y: 295
	},
	2.0
);

let timer = null;

CaptchaAnswer.render = () => htmlText;

CaptchaAnswer.captureKeyEvents = true;

/**
 * Initialize GUI
 */
CaptchaAnswer.init = function init() {
	this.draggable('.titlebar');

	const root = this.getRoot();
	const okBtn = root.querySelector('.ok');
	if (okBtn) {
		okBtn.addEventListener('click', () => {
			const answerInput = root.querySelector('.answer_input');
			const answer = answerInput ? answerInput.value : '';
			if (answer && answer.length > 0 && this.onSend) {
				this.onSend(answer);
			}
		});
	}
};

CaptchaAnswer.onKeyDown = function onKeyDown(event) {
	if (CaptchaAnswer.isEditableFocused()) {
		event.stopImmediatePropagation();
		return true;
	}
	return true;
};

/**
 * Append to DOM
 */
CaptchaAnswer.onAppend = function onAppend() {
	this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight)}px`;
	this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth)}px`;
};

/**
 * Set Image
 */
CaptchaAnswer.setImage = function setImage(imageData) {
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
 * Set Metadata
 */
CaptchaAnswer.setData = function setData(retryCount, timeout) {
	const root = this.getRoot();
	const retryEl = root.querySelector('.retry_count');
	if (retryEl) {
		retryEl.textContent = DB.getMessage(2886).replace('%d', retryCount);
	}

	if (timer) {
		clearInterval(timer);
		timer = null;
	}

	timer = setInterval(() => {
		timeout--;

		const minutes = Math.floor(timeout / 60);
		const seconds = timeout % 60;

		const timerText = root.querySelector('.timer_text');
		if (timerText) {
			timerText.textContent = `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
		}

		const timerBarFill = root.querySelector('.timer_bar_fill');
		if (timerBarFill) {
			timerBarFill.style.width = `${(timeout / 60) * 100}%`;
		}

		if (timeout <= 0) {
			clearInterval(timer);
			timer = null;
		}
	}, 1000);
};

CaptchaAnswer.setError = function setError(retryCount) {
	const root = this.getRoot();
	const errorText = root.querySelector('.error_text');
	if (errorText) {
		errorText.textContent = DB.getMessage(2875).replace('%d', retryCount);
	}
};

CaptchaAnswer.showSuccessMessage = function showSuccessMessage() {
	UIManager.showMessageBox(
		DB.getMessage(2871),
		'ok',
		() => {
			CaptchaAnswer.remove();
		},
		true
	);
};

/**
 * Remove data from UI
 */
CaptchaAnswer.onRemove = function onRemove() {
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.save();

	const root = this.getRoot();
	const imageContainer = root.querySelector('.image_container');
	if (imageContainer) {
		imageContainer.innerHTML = '';
	}
	const retryCount = root.querySelector('.retry_count');
	if (retryCount) {
		retryCount.textContent = 'Remaining chance: 0';
	}
	const timerText = root.querySelector('.timer_text');
	if (timerText) {
		timerText.textContent = '0';
	}
	const errorText = root.querySelector('.error_text');
	if (errorText) {
		errorText.textContent = '';
	}

	if (timer) {
		clearInterval(timer);
		timer = null;
	}
};

/**
 * Callback
 */
CaptchaAnswer.onSend = null;

/**
 * Stored component and return it
 */
export default UIManager.addComponent(CaptchaAnswer);
