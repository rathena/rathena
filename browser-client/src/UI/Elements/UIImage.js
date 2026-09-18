/**
 * UI/Elements/UIImage.js
 *
 * <ui-image src="basic_interface/titlebar_mid.bmp"></ui-image>
 *
 * Replaces: <div data-background="basic_interface/titlebar_mid.bmp">
 *
 * @author AoShinHo
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Targa from 'Loaders/Targa.js';

class UIImage extends HTMLElement {
	connectedCallback() {
		if (this._initialized) return;
		this._initialized = true;
		this.style.display = 'none';
		this._applyBackground();
	}

	static get observedAttributes() {
		return ['src'];
	}

	attributeChangedCallback(name, oldVal, newVal) {
		// Only reload on dynamic src changes after initial setup.
		// During innerHTML parsing, attributeChangedCallback fires before
		// connectedCallback — defer initial load to connectedCallback.
		if (name === 'src' && this._initialized) {
			this._applyBackground();
		}
	}

	_applyBackground() {
		const path = this.getAttribute('src');
		if (!path) return;

		const target = this.parentElement;
		if (!target) {
			// Parent may not be available yet (e.g. during innerHTML parsing).
			// Retry on next animation frame when the DOM tree is fully connected.
			requestAnimationFrame(() => {
				const retryTarget = this.parentElement;
				if (retryTarget) {
					this._loadImage(path, retryTarget);
				}
			});
			return;
		}
		this._loadImage(path, target);
	}

	_loadImage(path, target) {
		Client.loadFile(
			DB.INTERFACE_PATH + path,
			dataURI => {
				if (dataURI instanceof ArrayBuffer) {
					try {
						const tga = new Targa();
						tga.load(new Uint8Array(dataURI));
						target.style.backgroundImage = `url(${tga.getDataURL()})`;
					} catch (e) {
						console.error(`[ui-image] TGA decode error for "${path}":`, e.message);
					}
				} else {
					target.style.backgroundImage = `url(${dataURI})`;
				}
			},
			() => {
				console.warn(`[ui-image] Failed to load: ${path}`);
			}
		);
	}
}

customElements.define('ui-image', UIImage);
export default UIImage;
