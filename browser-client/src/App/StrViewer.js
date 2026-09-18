/**
 * App/StrViewer.js
 *
 * Show Str file effect
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Configs from 'Core/Configs.js';
import Thread from 'Core/Thread.js';
import Client from 'Core/Client.js';
import StrViewer from 'UI/Components/StrViewer/StrViewer.js';

export default function init() {
	function onAPIMessage(event) {
		if (typeof event.data !== 'object') {
			return;
		}

		switch (event.data.type) {
			case 'init':
				Thread.delegate(event.source, event.origin);
				Thread.init();
				StrViewer.append();
				break;

			case 'load':
				StrViewer.loadEffect(event.data.data);
				event.stopPropagation();
				break;

			case 'stop':
				StrViewer.stop();
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
			StrViewer.append();
		};
		Client.init([]);
	});
	Thread.init();
}

init();
