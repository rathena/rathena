/**
 * Controls/ScreenShot.js
 *
 * ScreenShot Manager
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Client from 'Core/Client.js';
import html2canvas from 'Vendors/html2canvas.js';
import KEYS from 'Controls/KeyEventHandler.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';

/**
 * Initiate methods
 */
class ScreenShot {
	/**
	 * Take a ScreenShot
	 */
	static take() {
		if (!ChatBox.ui) {
			return; //UI not loaded yet, cant display screenshot
		}

		html2canvas([document.body], {
			onrendered: this.process
		});
	}

	/**
	 * Process ScreenShot
	 *
	 * @param {canvasElement} canvas
	 */
	static process(canvas) {
		let x, y;

		// Create a date to add to canvas
		const tzoffset = new Date().getTimezoneOffset() * 60000; //offset in milliseconds
		let localISOTime = new Date(Date.now() - tzoffset).toISOString().slice(0, -1);
		localISOTime = localISOTime.replace('T', ' ');
		const timezone = new Date().getTimezoneOffset() / 60;
		const date = `${localISOTime} (GMT ${timezone > 0 ? '-' : '+'}${Math.abs(timezone).toString()})`; //GMT

		const context = canvas.getContext('2d');

		// Input the timestamp on screenshot
		context.fillStyle = 'white';
		context.strokeStyle = 'black';

		x = 20;
		y = canvas.height - 5;

		context.font = 'bold 16px Arial';
		context.fillText(date, x, y);
		context.strokeText(date, x, y);

		// Get and draw src_logo to canvas
		Client.loadFile(
			'data/texture/scr_logo.bmp',
			url => {
				const img = new Image();
				img.decoding = 'async';
				img.src = url;
				img.onload = () => {
					x = canvas.width - img.width - 20;
					y = canvas.height - img.height - 5;
					context.drawImage(img, x, y);
					ScreenShot.display(canvas, date);
				};
			},
			() => {
				ScreenShot.display(canvas, date);
			}
		);
	}

	/**
	 * Display the ScreenShot, this method is ment to be replaced by plugins if wanted.
	 *
	 * @param {canvasElement} canvas
	 * @param {string} date
	 */
	static display(canvas, date) {
		let i;

		// We decode the base64 to get the binary of the png
		const binary = atob(canvas.toDataURL('image/png').replace(/^data[^,]+,/, ''));
		const count = binary.length;
		const data = new Uint8Array(count);

		// We store the content in a buffer
		for (i = 0; i < count; ++i) {
			data[i] = binary.charCodeAt(i);
		}

		// We create a local image with the buffer
		const url = window.URL.createObjectURL(new Blob([data], { type: 'image/png' }));

		ChatBox.addText(
			`Screenshot ${date} can be saved by <a style="color:#F88" download="ScreenShot (${date.replace('/', '-')}).png" href="${url}" target="_blank">clicking here</a>.`,
			ChatBox.TYPE.PUBLIC,
			ChatBox.FILTER.PUBLIC_LOG,
			null,
			true
		);
	}
}

/**
 * Key Listener
 */
window.addEventListener('keydown', event => {
	if (KEYS.ALT && event.which === KEYS.P) {
		ScreenShot.take();
		event.stopImmediatePropagation();
		event.preventDefault();
	}
});

/**
 * Export
 */
export default ScreenShot;
