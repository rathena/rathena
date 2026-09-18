/**
 * App/GrannyModelViewer.js
 *
 * Show Gravity Granny3D models (gr2 files)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 * @author Liam Mitchell
 */

import Configs from 'Core/Configs.js';
import Thread from 'Core/Thread.js';
import Client from 'Core/Client.js';
import GrannyModelViewer from 'UI/Components/GrannyModelViewer/GrannyModelViewer.js';

export default function init() {
	function onAPIMessage(event) {
		if (typeof event.data !== 'object') {
			return;
		}

		switch (event.data.type) {
			case 'init':
				Thread.delegate(event.source, event.origin);
				Thread.init();
				GrannyModelViewer.append();
				break;

			case 'load':
				GrannyModelViewer.loadModel(event.data.data);
				event.stopPropagation();
				break;

			case 'stop':
				GrannyModelViewer.stop();
				event.stopPropagation();
				break;
		}
	}

	// Resources sharing
	if (Configs.get('api')) {
		window.addEventListener('message', onAPIMessage, false);
		return;
	}

	// Wait for thread to be ready and run the modelviewer
	Thread.hook('THREAD_READY', function () {
		Client.onFilesLoaded = function () {
			GrannyModelViewer.append();
		};
		Client.init([]);
	});
	Thread.init();
}

init();
