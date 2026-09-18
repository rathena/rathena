/**
 * UI/UIManager.js
 *
 * Manage Interface
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import GUIComponent from 'UI/GUIComponent.js';
import CommonCSS from 'UI/Common.css?raw';
import UIVersionManager from 'UI/UIVersionManager.js';
import KEYS from 'Controls/KeyEventHandler.js';
import ClampToViewport from 'UI/ClampToViewport.js';

/**
 * Centralize popup position
 * @returns {{top: string, left: string, zIndex: string}}
 */
function _popupPosition() {
	return {
		top: `${(window.innerHeight - 120) / 1.5 - 120}px`,
		left: `${(window.innerWidth - 280) / 2.0}px`,
		zIndex: '100'
	};
}

/**
 * Create a button with data-attributes for parseHTML to process
 * @param {string} name - button name (ex: 'ok', 'cancel')
 * @param {function} onClick - click callback (fires once)
 * @param {function} parseHTML - reference to GUIComponent.processDataAttrs
 * @returns {HTMLButtonElement}
 */
function _createButton(name, onClick) {
	const btn = document.createElement('button');
	btn.className = 'btn';
	btn.dataset.background = `btn_${name}.bmp`;
	btn.dataset.hover = `btn_${name}_a.bmp`;
	btn.dataset.down = `btn_${name}_b.bmp`;

	let clicked = false;
	btn.addEventListener('click', () => {
		if (clicked) return;
		clicked = true;
		onClick();
	});

	GUIComponent.processDataAttrs(btn);

	return btn;
}

/**
 * Create overlay that blocks interaction with the game
 * @returns {HTMLDivElement}
 */
function _createOverlay() {
	const overlay = document.createElement('div');
	overlay.className = 'win_popup_overlay';
	document.body.appendChild(overlay);
	return overlay;
}

// Common CSS must live in a global <style> tag so document-level rules
// (body font-size/family, focus reset) apply to light DOM and are inherited
// by every component's Shadow DOM. UIComponent.js used to inject this at load;
// it now lives here since UIManager is always loaded.
(function injectCommonCSS() {
	let style = document.querySelector('style[data-common]');
	if (!style) {
		style = document.createElement('style');
		style.setAttribute('data-common', '');
		style.textContent = CommonCSS;
		document.head.appendChild(style);
	}
})();

// Overlay CSS must live in the global <style> tag because overlay divs
// are appended to document.body (light DOM), not inside any Shadow DOM.
(function injectOverlayCSS() {
	let style = document.querySelector('style[data-overlay]');
	if (!style) {
		style = document.createElement('style');
		style.setAttribute('data-overlay', '');
		style.textContent = `  
			.win_popup_overlay {
				position: fixed;
				top: 0px;
				left: 0px;
				width: 100%;
				height: 100%;
				z-index: 99;
			}`;
		document.head.appendChild(style);
	}
})();

/**
 * Reorder keydown handlers so the popup captures first.
 * Moves the component's keydown handler to the capture phase so it fires before all other listeners.
 */
function _prioritizeKeyDown() {
	if (this._keyHandler) {
		window.removeEventListener('keydown', this._keyHandler);
		window.addEventListener('keydown', this._keyHandler, true);
	}
}

/**
 * User Interface Manager
 */
class UIManager {
	/**
	 * Components cache
	 * @var {array} Components List
	 */
	static components = {};

	/**
	 * Store a component in the manager
	 *
	 * @param {GUIComponent} component object
	 */
	static addComponent(component) {
		if (!(component instanceof GUIComponent)) {
			throw new Error('UIManager::addComponent() - Invalid type of component');
		}

		component.manager = this;
		this.components[component.name] = component;
		return component;
	}

	/**
	 * Get component stored in manager
	 *
	 * @param {string} component name
	 * @return {GUIComponent} object
	 */
	static getComponent(name) {
		const versionAlias = UIVersionManager.getUIAlias(name);
		if (versionAlias) {
			name = versionAlias;
		}

		if (!(name in this.components)) {
			throw new Error('UIManager.getComponent() - Component "' + name + '" not found');
		}

		return this.components[name];
	}

	/**
	 * Remove all components in screen
	 */
	static removeComponents() {
		const keys = Object.keys(this.components);
		const count = keys.length;

		for (let i = 0; i < count; ++i) {
			this.components[keys[i]].remove();
		}
	}

	/**
	 * When resizing window, some components can be outside the screen size and
	 * it sucks a lot. Try to correct the problem.
	 *
	 * @param {number} Game screen width
	 * @param {number} Game screen height
	 */
	static fixResizeOverflow(WIDTH, HEIGHT) {
		const keys = Object.keys(this.components);
		for (let i = 0; i < keys.length; ++i) {
			const component = this.components[keys[i]];
			const el = component.ui ? component.ui[0] : null;
			if (!el) continue;
			ClampToViewport(el, WIDTH, HEIGHT, component.magnet);
			if (component.onResize) {
				component.onResize();
			}
		}
	}

	/**
	 * Display an error box component
	 * Will reload the game once selected
	 *
	 * @param {string} error message
	 */
	static showErrorBox(text) {
		const WinError = this.getComponent('WinPopup').clone('WinError');
		// eslint-disable-next-line
		let overlay;

		WinError.init = function Init() {
			const root = this._shadow;

			root.querySelector('.text').textContent = text;
			Object.assign(this._host.style, _popupPosition());

			root.querySelector('.btns').appendChild(
				_createButton('ok', () => {
					overlay.remove();
					WinError.remove();
					import('Engine/GameEngine.js').then(m => m.default.reload());
				})
			);
		};

		WinError.onKeyDown = function OnKeyDown(event) {
			event.stopImmediatePropagation();
			switch (event.which) {
				case KEYS.ENTER:
				case KEYS.ESCAPE:
					overlay.remove();
					this.remove();
					import('Engine/GameEngine.js').then(m => m.default.reload());
			}
		};

		overlay = _createOverlay();
		WinError.onAppend = _prioritizeKeyDown;
		WinError.append();

		return WinError;
	}

	/**
	 * Show a message box to the user
	 *
	 * @param {string} message to show
	 * @param {string} button name
	 * @param {function} callback once the button is pressed
	 */
	static showMessageBox(text, btn_name, callback, keydown) {
		const WinMSG = this.getComponent('WinPopup').clone('WinMSG');

		WinMSG.init = function Init() {
			this.draggable();
			const root = this._shadow;

			root.querySelector('.text').textContent = text;
			Object.assign(this._host.style, _popupPosition());

			if (btn_name) {
				root.querySelector('.btns').appendChild(
					_createButton(btn_name, () => {
						WinMSG.remove();
						if (callback) callback();
					})
				);
			}
		};

		if (keydown) {
			WinMSG.onKeyDown = function (event) {
				switch (event.which) {
					case KEYS.ENTER:
					case KEYS.ESCAPE:
						this.remove();
						if (callback) callback();
				}
				event.stopImmediatePropagation();
			};

			WinMSG.onAppend = _prioritizeKeyDown;
		}

		WinMSG.append();
		return WinMSG;
	}

	/**
	 * Prompt a message to the user
	 *
	 * @param {string} message to show
	 * @param {string} button ok
	 * @param {string} button cancel
	 * @param {function} callback when ok is pressed
	 * @param {function} callback when cancel is pressed
	 */
	static showPromptBox(text, btn_yes, btn_no, onYes, onNo) {
		const WinPrompt = this.getComponent('WinPopup').clone('WinPrompt');

		WinPrompt.init = function Init() {
			this.draggable();
			const root = this._shadow;

			root.querySelector('.text').textContent = text;
			Object.assign(this._host.style, _popupPosition());

			const btnsContainer = root.querySelector('.btns');

			btnsContainer.appendChild(
				_createButton(btn_yes, () => {
					WinPrompt.remove();
					if (onYes) onYes();
				})
			);

			btnsContainer.appendChild(
				_createButton(btn_no, () => {
					WinPrompt.remove();
					if (onNo) onNo();
				})
			);
		};

		WinPrompt.append();
		return WinPrompt;
	}

	/**
	 * Reload CSS of a component
	 * @param {string} componentName
	 * @param {string} newCssText
	 */
	static reloadCSS(componentName, newCssText) {
		GUIComponent.reloadCSS(componentName, newCssText);
	}
}
/**
 * Export
 */
export default UIManager;
