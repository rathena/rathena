// applications/shared/importmap.js
(function () {
	var script = document.currentScript || document.querySelector('script[src*="shared/importmap.js"]');
	if (!script) {
		return;
	}
	var projectRoot = script.dataset.projectRoot;
	if (!projectRoot) {
		var splitPoint = script.dataset.projectRootSplit || '/applications/';
		var path = new URL(window.location.href).pathname;
		projectRoot = path.split(splitPoint)[0] + '/';
	}
	var importMap = {
		imports: {
			bson: projectRoot + 'node_modules/bson/lib/bson.mjs',
			lodash: projectRoot + 'node_modules/lodash-es/lodash.default.js',
			'src/': projectRoot + 'src/',
			'App/': projectRoot + 'src/App/',
			'Audio/': projectRoot + 'src/Audio/',
			'Controls/': projectRoot + 'src/Controls/',
			'Core/': projectRoot + 'src/Core/',
			'DB/': projectRoot + 'src/DB/',
			'Engine/': projectRoot + 'src/Engine/',
			'Loaders/': projectRoot + 'src/Loaders/',
			'Network/': projectRoot + 'src/Network/',
			'Plugins/': projectRoot + 'src/Plugins/',
			'Preferences/': projectRoot + 'src/Preferences/',
			'Renderer/': projectRoot + 'src/Renderer/',
			'UI/': projectRoot + 'src/UI/',
			'Utils/': projectRoot + 'src/Utils/',
			'Vendors/': projectRoot + 'src/Vendors/'
		}
	};
	var s = document.createElement('script');
	s.type = 'importmap';
	s.textContent = JSON.stringify(importMap);
	script.after(s);
})();
