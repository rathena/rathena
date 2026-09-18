/**
 * UI/Elements/UIText.js
 *
 * <ui-text msg="2355">Fallback text</ui-text>
 *
 * Replaces: <span data-text="2355">Clan Info</span>
 *
 * @author AoShinHo
 */

import DB from 'DB/DBManager.js';

class UIText extends HTMLElement {
	connectedCallback() {
		if (this._initialized) return;
		this._initialized = true;
		const msgId = this.getAttribute('msg');
		if (msgId) {
			const text = DB.getMessage(msgId, '');
			if (text) this.textContent = text;
		}
	}

	static get observedAttributes() {
		return ['msg'];
	}

	attributeChangedCallback(name, oldVal, newVal) {
		if (name === 'msg' && newVal) {
			const text = DB.getMessage(newVal, '');
			if (text) this.textContent = text;
		}
	}
}

customElements.define('ui-text', UIText);
export default UIText;
