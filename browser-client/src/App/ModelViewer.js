/**
 * App/ModelViewer.js
 *
 * Show Gravity models (rsm files)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Configs from 'Core/Configs.js';
import Thread from 'Core/Thread.js';
import Client from 'Core/Client.js';
import ModelViewer from 'UI/Components/ModelViewer/ModelViewer.js';

export default function init() {
	function onAPIMessage(event) {
		if (typeof event.data !== 'object') {
			return;
		}

		switch (event.data.type) {
			case 'init':
				Thread.delegate(event.source, event.origin);
				Thread.init();
				ModelViewer.append();
				break;

			case 'load':
				ModelViewer.loadModel(event.data.data);
				event.stopPropagation();
				break;

			case 'stop':
				ModelViewer.stop();
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
			ModelViewer.append();
		};
		Client.init([]);
	});
	Thread.init();
}

init();
