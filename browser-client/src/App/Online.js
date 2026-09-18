import GameEngine from 'Engine/GameEngine.js';
import Plugins from 'Plugins/PluginManager.js';
import { roInitSpinner } from 'App/PreLoader.js';

export { roInitSpinner };

export function init() {
	// Grab (or create) the preloader
	roInitSpinner.add();

	Plugins.init();
	GameEngine.init();

	window.onbeforeunload = function () {
		return 'Are you sure to exit roBrowser ?';
	};
}

export default {
	init: init,
	roInitSpinner: roInitSpinner
};

init();
