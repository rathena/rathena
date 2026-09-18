/**
 * App/EffectViewer.js
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
import EffectViewer from 'UI/Components/EffectViewer/EffectViewer.js';

export default function init() {
	function onAPIMessage(event) {
		if (typeof event.data !== 'object') {
			return;
		}

		switch (event.data.type) {
			case 'init':
				Thread.delegate(event.source, event.origin);
				Thread.init();
				EffectViewer.append();
				break;

			case 'load':
				EffectViewer.loadEffect(event.data.data);
				event.stopPropagation();
				break;

			case 'stop':
				EffectViewer.stop();
				event.stopPropagation();
				break;
		}
	}

	// Resources sharing
	if (Configs.get('api')) {
		window.addEventListener('message', onAPIMessage, false);
		return;
	}

	// Wait for thread to be ready and run the effectviewer
	Thread.hook('THREAD_READY', function () {
		Client.onFilesLoaded = function () {
			EffectViewer.append();
		};
		Client.init([]);
	});
	Thread.init();
}

init();
