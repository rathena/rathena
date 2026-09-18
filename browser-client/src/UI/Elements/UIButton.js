/**
 * UI/Elements/UIButton.js
 *
 * <ui-button bg="btn_ok.bmp" hover="btn_ok_a.bmp" down="btn_ok_b.bmp">OK</ui-button>
 *
 * Replaces: <button data-background="btn_ok.bmp" data-hover="btn_ok_a.bmp" data-down="btn_ok_b.bmp">
 *
 * @author AoShinHo
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Targa from 'Loaders/Targa.js';

class UIButton extends HTMLElement {
	connectedCallback() {
		if (this._initialized) return;
		this._initialized = true;
		const bg = this.getAttribute('bg');
		const hover = this.getAttribute('hover');
		const down = this.getAttribute('down');

		let bgUri = null,
			hoverUri = null,
			downUri = null;
		const state = { hover: false, down: false };

		const update = () => {
			if (this.disabled) {
				if (bgUri) {
					this.style.backgroundImage = `url(${bgUri})`;
				}
				this.style.opacity = '0.5';
				this.style.cursor = 'default';
				return;
			}
			this.style.opacity = '';
			this.style.cursor = '';
			if (state.down && downUri) {
				this.style.backgroundImage = `url(${downUri})`;
			} else if (state.hover && hoverUri) {
				this.style.backgroundImage = `url(${hoverUri})`;
			} else if (bgUri) {
				this.style.backgroundImage = `url(${bgUri})`;
			} else {
				this.style.backgroundImage = '';
			}
		};
		this._update = update;
		const loadBmp = (path, cb) => {
			if (!path) return;
			Client.loadFile(DB.INTERFACE_PATH + path, dataURI => {
				if (dataURI instanceof ArrayBuffer) {
					try {
						const tga = new Targa();
						tga.load(new Uint8Array(dataURI));
						cb(tga.getDataURL());
					} catch (e) {
						console.error(e.message);
					}
				} else {
					cb(dataURI);
				}
			});
		};

		loadBmp(bg, uri => {
			bgUri = uri;
			update();
		});
		loadBmp(hover, uri => {
			hoverUri = uri;
		});
		loadBmp(down, uri => {
			downUri = uri;
		});

		this.addEventListener('mouseover', () => {
			if (this.disabled) return;
			state.hover = true;
			update();
		});
		this.addEventListener('mouseout', () => {
			state.hover = false;
			state.down = false;
			update();
		});
		this.addEventListener('mousedown', () => {
			if (this.disabled) return;
			state.down = true;
			update();
		});
		this.addEventListener('mouseup', () => {
			state.down = false;
			update();
		});
		this.addEventListener(
			'click',
			e => {
				if (this.disabled) {
					e.stopImmediatePropagation();
					e.preventDefault();
				}
			},
			true
		);
	}
	get disabled() {
		return this.hasAttribute('disabled');
	}

	set disabled(val) {
		if (val) {
			this.setAttribute('disabled', '');
		} else {
			this.removeAttribute('disabled');
		}
	}
	static get observedAttributes() {
		return ['disabled'];
	}

	attributeChangedCallback(name) {
		if (name === 'disabled' && this._initialized) {
			// Reset hover/down state when becoming disabled
			// The update() closure is inside connectedCallback, so we need a reference
			if (this._update) this._update();
		}
	}
}

customElements.define('ui-button', UIButton);
export default UIButton;
