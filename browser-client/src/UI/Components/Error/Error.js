/**
 * UI/Components/Error/Error.js
 *
 * Error screen
 * Don't use components class, if there is an error on this module will never be used...
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import _htmlText from './Error.html?raw';
import _cssText from './Error.css?raw';

/**
 * Error Namespace
 */
class Error {
	/**
	 * Initialize Metaling
	 */
	static init() {
		const wrapper = document.createElement('div');
		wrapper.innerHTML = _htmlText;
		this.ui = wrapper.firstElementChild;

		let style = document.querySelector('style');
		if (!style) {
			style = document.createElement('style');
			style.setAttribute('type', 'text/css');
			document.head.appendChild(style);
		}
		style.appendChild(document.createTextNode('\n' + _cssText));
		document.body.innerHTML = '';
		document.body.appendChild(this.ui);

		this.ui.style.backgroundImage = `url(${new URL('./error.png', import.meta.url).href})`;
	}

	/**
	 * Add trace info to UI
	 *
	 * @param {Error} error
	 */
	static addTrace(error) {
		console.error(error);
		let url = new URL('../../../', import.meta.url).href;
		error = error.stack || error;

		url = url.replace(/\/([^/]+)$/g, '/');
		error = error.replace(/\n/g, '<br/>');
		error = error.replace(new RegExp(url, 'g'), '');
		error = error.replace(/\?[^:]+/g, '');

		if (!this.ui) {
			this.init();
		}

		const trace = this.ui.querySelector('.trace');
		if (trace) {
			trace.insertAdjacentHTML('beforeend', `${error}<br />`);
		}
	}
}

/**
 * Stored component and return it
 */
export default Error;
