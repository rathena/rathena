/**
 * main.js
 * Entry point for roBrowserLegacy (ES6 version)
 * Centralized loader for all applications.
 */
// eslint-disable-next-line
import Online from 'App/Online.js';
import { roInitSpinner } from 'App/PreLoader.js';
import Configs from 'Core/Configs.js';

const APP = {
	ONLINE: 1,
	MAPVIEWER: 2,
	GRFVIEWER: 3,
	MODELVIEWER: 4,
	STRVIEWER: 5,
	GRANNYMODELVIEWER: 6,
	EFFECTVIEWER: 7
};

/**
 * Launch the appropriate application based on config
 */
async function launch(config) {
	const appId = parseInt(config.application, 10) || APP.ONLINE;

	switch (appId) {
		case APP.ONLINE:
			break;

		case APP.MAPVIEWER:
			await import('App/MapViewer.js');
			break;

		case APP.GRFVIEWER:
			await import('App/GrfViewer.js');
			break;

		case APP.MODELVIEWER:
			await import('App/ModelViewer.js');
			break;

		case APP.STRVIEWER:
			await import('App/StrViewer.js');
			break;

		case APP.GRANNYMODELVIEWER:
			await import('App/GrannyModelViewer.js');
			break;

		case APP.EFFECTVIEWER:
			await import('App/EffectViewer.js');
			break;

		default:
			console.error('Unknown application ID:', appId);
			break;
	}

	window.dispatchEvent(new Event('robrowser-ready'));
}

// Global initialization
console.log('roBrowser ES6 Entry Point Initialized');

// If ROConfig is already in window (direct access)
if (window.ROConfig) {
	launch(window.ROConfig);
} else {
	// Wait for configuration via postMessage (API mode)
	const onMessage = event => {
		// Only accept messages from the parent window or opener (the page that loaded us)
		if (event.source !== window.parent && event.source !== window.opener) {
			return;
		}
		if (event.data && (event.data.application || event.data.servers)) {
			window.ROConfig = event.data;
			// Configs is populated by an IIFE at import time, which runs before this
			// config arrives via postMessage; apply the received config so options such
			// as 'api' are available (frame/popup API mode).
			Object.keys(window.ROConfig).forEach(key => Configs.set(key, window.ROConfig[key]));
			launch(window.ROConfig);
			window.removeEventListener('message', onMessage);
		}
	};
	window.addEventListener('message', onMessage, false);

	// Fallback/Spinner
	roInitSpinner.add();
}
