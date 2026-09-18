/**
 * UI/Components/GrfViewer/GrfViewer.js
 *
 * Game File Viewer
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Configs from 'Core/Configs.js';
import Client from 'Core/Client.js';
import Thread from 'Core/Thread.js';
import Memory from 'Core/MemoryManager.js';
import Events from 'Core/Events.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Sprite from 'Loaders/Sprite.js';
import Targa from 'Loaders/Targa.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './GrfViewer.html?raw';
import cssText from './GrfViewer.css?raw';
import History from './History.js';

/**
 * Create GRFViewer component
 */
const Viewer = new GUIComponent('GRFViewer', cssText);

Viewer.render = () => htmlText;

/**
 * @var {number} The display is done asynchronous, keep reference of the thread
 */
let _thread = 0;

/**
 * @var {number} action id, to stop doing things when the action change
 */
let _actionID = 0;

/**
 * Initialize Component
 */
Viewer.init = function init() {
	const root = Viewer.getRoot();

	Thread.hook('THREAD_READY', () => {
		const remoteClient = Configs.get('remoteClient');
		if (remoteClient) {
			Thread.send('SET_HOST', remoteClient);
			Client.init([]);
		}
	});
	Thread.init();

	// Inject icon styles into shadow DOM
	const extraStyle = document.createElement('style');
	extraStyle.textContent =
		`#previous { background-image: url(${new URL('./Icons/arw-left.png', import.meta.url).href}); }` +
		`#next { background-image: url(${new URL('./Icons/arw-right.png', import.meta.url).href}); }` +
		`#progress { background-image: url(${new URL('./Icons/load.gif', import.meta.url).href}); }`;
	this._shadow.appendChild(extraStyle);

	// Drag drop the GRF
	const info = root.querySelector('#info');
	info.addEventListener('dragover', e => {
		info.style.backgroundColor = '#DFD';
		e.preventDefault();
	});
	info.addEventListener('dragleave', () => {
		info.style.backgroundColor = '#EEE';
	});
	info.addEventListener('drop', e => {
		info.style.backgroundColor = '#EEE';
		processGRF(e);
	});

	// Load GRFs
	root.querySelector('#file').addEventListener('change', processGRF);
	root.querySelector('#info button').addEventListener('mousedown', () => {
		root.querySelector('#file').click();
	});

	// Bind icon events via delegation
	const grfviewerEl = root.querySelector('#grfviewer');
	grfviewerEl.addEventListener('click', e => {
		let el;
		if ((el = e.target.closest('.directory'))) {
			onDirectoryClick(el);
			return;
		}
		if ((el = e.target.closest('.audio'))) {
			onAudioClick(el);
			return;
		}
		if ((el = e.target.closest('.img, .thumb'))) {
			onImageClick(el);
			return;
		}
		if ((el = e.target.closest('.txt'))) {
			onTextClick(el);
			return;
		}
		if ((el = e.target.closest('.map'))) {
			onWorldClick(el);
			return;
		}
		if ((el = e.target.closest('.\\33 d'))) {
			onObjectClick(el);
			return;
		}
		if ((el = e.target.closest('.gr2'))) {
			onGrannyClick(el);
			return;
		}
		if ((el = e.target.closest('.fx'))) {
			onEffectClick(el);
			return;
		}
	});
	grfviewerEl.addEventListener('contextmenu', e => {
		const icon = e.target.closest('.icon');
		if (icon) {
			showContextMenu(icon, e);
			e.preventDefault();
		}
	});

	// Initialize toolbar
	initToolBar();

	// Initialize history
	History.init(root.querySelector('#previous'), root.querySelector('#next'));

	// Renderer is not rendering, causing issue in src/UI/UIComponents.js#212
	// Trigger manually the event.
	setTimeout(() => {
		Events.process(100);
	}, 10);
};

/**
 * Once append to body, initialize elements
 */
Viewer.onAppend = function onAppend() {
	document.body.style.backgroundColor = 'white';
	document.body.style.margin = '0';
	document.body.style.overflow = 'auto';
	moveToDirectory('/', true);
};

/**
 * Initialize tool bar
 */
function initToolBar() {
	const root = Viewer.getRoot();

	// Path submit
	root.querySelector('#path').addEventListener('keydown', function (event) {
		if (event.which === KEYS.ENTER) {
			let value = this.value.replace(/^\s+|\s+$/g, '');
			if (value.substr(-1) !== '/') {
				value += '/';
			}
			moveToDirectory(value, true);
		}
	});

	// History before
	root.querySelector('#previous').addEventListener('click', () => {
		const path = History.previous();
		if (path) {
			moveToDirectory(path, false);
		}
	});

	// History after
	root.querySelector('#next').addEventListener('click', () => {
		const path = History.next();
		if (path) {
			moveToDirectory(path, false);
		}
	});

	// Search toolbar
	const searchInput = root.querySelector('#search');
	searchInput.addEventListener('focus', function () {
		this.value = '';
	});
	searchInput.addEventListener('blur', function () {
		this.value = this.value || 'Search...';
	});
	searchInput.addEventListener('keydown', function (event) {
		if (event.which === KEYS.ENTER) {
			const value = this.value.replace(/^\s+|\s+$/g, '');
			if (value.length > 2) {
				moveToDirectory(`search/${value}`, true);
			}
		}
	});
}

/**
 * Context Menu feature
 *
 * @param {HTMLElement} iconElement
 * @param {object} event
 */
function showContextMenu(iconElement, event) {
	const root = Viewer.getRoot();
	const contextmenu = root.querySelector('#contextmenu');
	const overlay = root.querySelector('.overlay');
	const header = contextmenu.querySelector('.header');
	const openBtn = contextmenu.querySelector('.open');
	const saveLink = contextmenu.querySelector('.save');
	const infoBtn = contextmenu.querySelector('.info');
	const path = iconElement.getAttribute('data-path');

	contextmenu.style.left = `${event.pageX}px`;
	contextmenu.style.top = `${event.pageY}px`;
	contextmenu.style.display = 'block';

	overlay.onmousedown = () => {
		contextmenu.style.display = 'none';
		overlay.style.display = 'none';
	};
	overlay.style.display = 'block';

	// Clean up previous handlers
	openBtn.classList.remove('disable');
	openBtn.onmousedown = null;
	saveLink.classList.remove('disable');
	saveLink.onclick = null;
	saveLink.onmouseup = null;
	infoBtn.classList.remove('disable');
	infoBtn.onmousedown = null;

	// Header
	header.innerHTML = `Path: ${path}`;
	header.style.backgroundColor = '';
	header.onclick = () => {
		navigator.clipboard.writeText(path);
		header.innerHTML = `Copied: ${path}`;
		header.style.backgroundColor = '#AAFFAA';
	};

	// Open
	if (iconElement.classList.contains('file')) {
		openBtn.classList.add('disable');
	} else {
		openBtn.onmousedown = () => {
			iconElement.click();
			overlay.onmousedown();
		};
	}

	// Save
	if (iconElement.classList.contains('directory')) {
		saveLink.classList.add('disable');
		saveLink.onclick = e => e.preventDefault();
		saveLink.removeAttribute('download');
	} else {
		saveLink.onmouseup = () => {
			overlay.onmousedown();
		};

		Client.getFile(path, buffer => {
			const url = URL.createObjectURL(new Blob([buffer], { type: 'application/octet-stream' }));
			saveLink.href = url;
			saveLink.download = iconElement.textContent.trim();
		});
	}

	// Properties - not supported yet
	infoBtn.classList.add('disable');
}

/**
 * Move to a path
 *
 * @param {string} path to move
 * @param {boolean} save in history
 */
function moveToDirectory(path, save) {
	path = decodeURIComponent(path) || '/';
	path = path.replace(/\\/g, '/');

	if (path.substr(0, 1) === '/') {
		path = path.substr(1);
	}

	if (path.match(/^search\//)) {
		search(path.substr(7));
	} else {
		showDirectory(path);
	}

	// Update history
	if (save) {
		History.push(path);
	}
}

/**
 * Load GRF files
 *
 * @param {object} event
 */
function processGRF(event) {
	const root = Viewer.getRoot();
	root.querySelector('#progress').style.display = 'block';

	Client.onFilesLoaded = () => {
		moveToDirectory('data/', true);
	};
	Client.init((event.dataTransfer || event.target).files);

	event.preventDefault();
	event.stopPropagation();
}

/**
 * Move to a specify path
 *
 * @param {string} path
 */
function showDirectory(path) {
	clearTimeout(_thread);

	path = decodeURIComponent(path) || '/';
	path = path.replace(/\\/g, '/');

	if (path.substr(0, 1) === '/') {
		path = path.substr(1);
	}

	const root = Viewer.getRoot();
	const directory = path.replace(/\//g, '\\\\');
	const reg = `${directory}([^(\\0|\\\\)]+)`;

	root.querySelector('#path').value = path;
	root.querySelectorAll('.icon').forEach(el => el.remove());
	root.querySelector('#progress').style.display = 'block';
	root.querySelector('#msg').style.display = 'none';

	if (!path.length) {
		root.querySelector('#info').style.display = '';
		renderFiles(['data']);
		return;
	}

	root.querySelector('#info').style.display = 'none';

	const actionID = ++_actionID;

	Client.search(new RegExp(reg, 'gi'), list => {
		if (actionID === _actionID) {
			list.sort(sortFiles);
			renderFiles(list);
		}
	});
}

/**
 * Search files (apply a regex on fileList) and show the result
 *
 * @param {string} keyword
 */
function search(keyword) {
	const escapedSearch = keyword.replace(/(\.|\\|\+|\*|\?|\[|\^|\]|\$|\(|\)|\{|\}|\=|\!|<|>|\||\:|\-)/g, '\\$1');
	const reg = `data\\\\([^(\\0\\)]+)?${escapedSearch}([^(\\0|\\\\)]+)?`;
	const root = Viewer.getRoot();
	const actionID = ++_actionID;

	root.querySelectorAll('.icon').forEach(el => el.remove());
	root.querySelector('#progress').style.display = 'block';
	root.querySelector('#info').style.display = 'none';
	root.querySelector('#msg').style.display = 'none';

	Client.search(new RegExp(reg, 'gi'), list => {
		if (actionID === _actionID) {
			list.sort(sortFiles);
			renderFiles(list);
		}
	});
}

/**
 * Organize file listing
 * Directory first, and organized by alpha
 *
 * @param {string} a file's name
 * @param {string} b file's name
 * @return {number}
 */
function sortFiles(a, b) {
	const nameA = a.replace(/.*\\/, '');
	const nameB = b.replace(/.*\\/, '');
	const isFileA = nameA.indexOf('.') !== -1;
	const isFileB = nameB.indexOf('.') !== -1;

	if (isFileA === isFileB) {
		return nameA > nameB ? 1 : -1;
	}
	if (isFileA) {
		return 1;
	}
	return -1;
}

/**
 * Showing file list on the screen
 *
 * @param {Array} list of files and directories
 */
function renderFiles(list) {
	const root = Viewer.getRoot();
	root.querySelector('#progress').style.display = 'none';

	if (!list.length) {
		const msg = root.querySelector('#msg');
		msg.textContent = 'No file found.';
		msg.style.display = 'block';
		return;
	}

	let i;
	const reg = /(.*\\)/;
	i = 0;
	const count = list.length;
	const grfviewerEl = root.querySelector('#grfviewer');

	function streamExecute() {
		let j;
		let html = '';

		for (j = 0; j < 200 && i + j < count; ++j) {
			const type = getFileIcon(list[j + i]);
			html +=
				`<div class="icon ${type}" data-path="${list[j + i]}">` +
				`	<img src="${new URL(`./Icons/${type}.png`, import.meta.url).href}" width="48" height="48"/><br/>` +
				list[j + i].replace(reg, '') +
				'</div>';
		}

		grfviewerEl.insertAdjacentHTML('beforeend', html);

		i += j;

		if (i < count) {
			_thread = setTimeout(streamExecute, 4);
		}
	}

	streamExecute();
	displayImagesThumbnail();
}

/**
 * Get file thumbnail based on its extension
 *
 * @param {string} filename
 * @return {string} icon name
 */
function getFileIcon(filename) {
	const ext = filename.split(/\.([^.]+)$/)[1] || 'dir';
	let img = 'file';

	switch (ext.toLowerCase()) {
		case 'dir':
			img = 'directory';
			break;

		case 'xml':
		case 'txt':
		case 'lua':
			img = 'txt';
			break;

		case 'jpg':
		case 'bmp':
		case 'tga':
		case 'jpeg':
		case 'spr':
		case 'pal':
			img = 'img';
			break;

		case 'wav':
		case 'mp3':
			img = 'audio';
			break;

		case 'gr2':
			img = 'gr2';
			break;

		case 'rsm':
		case 'rsm2':
			img = '3d';
			break;

		case 'rsw':
			img = 'map';
			break;

		case 'str':
			img = 'fx';
			break;
	}

	return img;
}

/**
 * Display real thumbnails for each known file
 */
function displayImagesThumbnail() {
	const actionID = _actionID + 0;

	function cleanUp() {
		URL.revokeObjectURL(this.src);
	}

	function process() {
		if (actionID !== _actionID) {
			return;
		}

		const root = Viewer.getRoot();
		const nodes = Array.from(root.querySelectorAll('.img')).slice(0, 5);
		let load = 0;
		const total = nodes.length;

		if (!total) {
			return;
		}

		nodes.forEach(el => {
			const path = el.getAttribute('data-path');

			Client.getFile(path, data => {
				Memory.remove(path);
				el.classList.remove('img');
				el.classList.add('thumb');

				const url = getImageThumbnail(path, data);

				if (url) {
					const img = el.querySelector('img');
					if (url.match(/^blob:/)) {
						img.onload = img.onerror = img.onabort = cleanUp;
					}
					img.decoding = 'async';
					img.src = url;
				}

				if (++load >= total) {
					setTimeout(process, 4);
				}
			});
		});
	}

	process();
}

/**
 * Generate thumbnail for a file
 *
 * @param {string} filename
 * @param {ArrayBuffer} data
 * @return {string|null} url generated
 */
function getImageThumbnail(filename, data) {
	let canvas;
	const ext = filename.substr(-3).toLowerCase();

	switch (ext) {
		case 'spr': {
			const spr = new Sprite(data);
			canvas = spr.getCanvasFromFrame(0);
			return canvas.toDataURL();
		}

		case 'pal': {
			canvas = document.createElement('canvas');
			const ctx = canvas.getContext('2d');
			const palette = new Uint8Array(data);

			canvas.width = 16;
			canvas.height = 16;
			const imageData = ctx.createImageData(canvas.width, canvas.height);

			for (let i = 0, count = imageData.data.length; i < count; i += 4) {
				imageData.data[i + 0] = palette[i + 0];
				imageData.data[i + 1] = palette[i + 1];
				imageData.data[i + 2] = palette[i + 2];
				imageData.data[i + 3] = 255;
			}

			ctx.putImageData(imageData, 0, 0);
			return canvas.toDataURL();
		}

		case 'tga': {
			const tga = new Targa();
			tga.load(new Uint8Array(data));
			return tga.getDataURL();
		}

		default:
			return URL.createObjectURL(new Blob([data], { type: `image/${ext}` }));
	}
}

/**
 * User click on directory, open it
 */
function onDirectoryClick(iconEl) {
	moveToDirectory(`${iconEl.getAttribute('data-path')}/`, true);
}

/**
 * User click on an audio file, play it
 */
function onAudioClick(iconEl) {
	const root = Viewer.getRoot();
	const path = iconEl.getAttribute('data-path');
	const box = root.querySelector('#preview .box');
	const progress = root.querySelector('#progress');
	const preview = root.querySelector('#preview');

	progress.style.display = 'block';

	Client.loadFile(path, url => {
		const audio = document.createElement('audio');
		audio.src = url;
		audio.controls = true;
		audio.play();
		audio.addEventListener('click', e => e.stopPropagation());

		box.style.top = `${(window.innerHeight - 100) / 2}px`;
		box.appendChild(audio);

		progress.style.display = 'none';
		preview.style.display = 'block';
		preview.addEventListener(
			'click',
			() => {
				preview.style.display = 'none';
				const audioEl = box.querySelector('audio');
				if (audioEl) {
					audioEl.remove();
				}
			},
			{ once: true }
		);
	});
}

/**
 * User click on an image, render it
 */
function onImageClick(iconEl) {
	const root = Viewer.getRoot();
	const path = iconEl.getAttribute('data-path');
	const box = root.querySelector('#preview .box');
	const progress = root.querySelector('#progress');
	const preview = root.querySelector('#preview');

	progress.style.display = 'block';

	Client.getFile(path, data => {
		let i, count, canvas;

		switch (path.substr(-3)) {
			case 'spr': {
				const spr = new Sprite(data);
				box.style.top = '200px';

				for (i = 0, count = spr.frames.length; i < count; ++i) {
					canvas = spr.getCanvasFromFrame(i);
					if (canvas) {
						box.appendChild(canvas);
					}
				}
				break;
			}

			case 'pal': {
				const palette = new Uint8Array(data);
				canvas = document.createElement('canvas');
				const ctx = canvas.getContext('2d');

				canvas.width = 128;
				canvas.height = 128;

				for (i = 0, count = palette.length; i < count; i += 4) {
					ctx.fillStyle = `rgb(${palette[i + 0]},${palette[i + 1]},${palette[i + 2]})`;
					ctx.fillRect((((i / 4) | 0) % 16) * 8, ((((i / 4) | 0) / 16) | 0) * 8, 8, 8);
				}

				box.style.top = `${window.innerHeight / 2 - 64}px`;
				box.appendChild(canvas);
				break;
			}

			case 'tga': {
				const tga = new Targa();
				tga.load(new Uint8Array(data));
				box.style.top = `${window.innerHeight / 2 - 64}px`;
				box.appendChild(tga.getCanvas());
				break;
			}

			default: {
				const url = URL.createObjectURL(new Blob([data], { type: `image/${path.substr(-3)}` }));
				const img = new Image();
				img.decoding = 'async';
				img.src = url;
				img.onload = function () {
					box.style.top = `${(window.innerHeight - this.height) / 2}px`;
					box.appendChild(this);
					URL.revokeObjectURL(url);
				};
				break;
			}
		}

		preview.style.display = 'block';
		progress.style.display = 'none';
		preview.addEventListener(
			'click',
			() => {
				preview.style.display = 'none';
				box.querySelectorAll('img, canvas').forEach(el => el.remove());
			},
			{ once: true }
		);
	});
}

/**
 * User click on a model, render it using ModelViewer
 */
const onObjectClick = (() => {
	let ready = false;
	let App = null;
	const element = document.createElement('div');

	function initApp() {
		if (App) return true;
		if (typeof ROBrowser === 'undefined') return false;
		App = new ROBrowser({
			target: element,
			type: ROBrowser.TYPE.FRAME, // eslint-disable-line no-undef
			application: ROBrowser.APP.MODELVIEWER, // eslint-disable-line no-undef
			development: Configs.get('development', false),
			api: true,
			width: 500,
			height: 400,
			version: Configs.get('version', '')
		});
		return true;
	}

	function onMessage(event) {
		if (typeof event.data !== 'object') {
			return;
		}

		switch (event.data.type) {
			case 'SYNC':
				ready = true;
				App.onload();
				break;

			case 'SET_HOST':
			case 'CLEAN_GRF':
				return;

			default:
				Thread.send(event.data.type, event.data.data, function () {
					App._APP.postMessage(
						{
							arguments: Array.prototype.slice.call(arguments, 0),
							uid: event.data.uid
						},
						location.origin
					);
				});
		}
	}

	function synchronise() {
		if (!ready) {
			App._APP.postMessage({ type: 'init' }, location.origin);
			setTimeout(synchronise, 100);
		}
	}

	return iconEl => {
		if (!initApp()) return;
		const root = Viewer.getRoot();
		const path = iconEl.getAttribute('data-path').replace(/\\/g, '/');
		const box = root.querySelector('#preview .box');
		const preview = root.querySelector('#preview');

		box.style.top = `${(window.innerHeight - 400) * 0.5}px`;
		element.style.display = 'block';

		preview.style.display = 'block';

		preview.addEventListener(
			'click',
			() => {
				preview.style.display = 'none';
				element.style.display = 'none';
				App._APP.postMessage({ type: 'stop' }, location.origin);
				window.removeEventListener('message', onMessage, false);
			},
			{ once: true }
		);

		window.addEventListener('message', onMessage, false);

		if (!ready) {
			box.appendChild(element);

			App.start();
			App.onReady = () => {
				App._APP.frameElement.style.border = '1px solid grey';
				App._APP.frameElement.style.backgroundColor = '#45484d';
				synchronise();
			};
			App.onload = () => {
				App._APP.postMessage({ type: 'load', data: path }, location.origin);
			};
		} else {
			App._APP.postMessage({ type: 'load', data: path }, location.origin);
		}
	};
})();

/**
 * User click on an effect, render it using StrViewer
 */
const onEffectClick = (() => {
	let ready = false;
	let App = null;
	const element = document.createElement('div');

	function initApp() {
		if (App) return true;
		if (typeof ROBrowser === 'undefined') return false;
		App = new ROBrowser({
			target: element,
			type: ROBrowser.TYPE.FRAME, // eslint-disable-line no-undef
			application: ROBrowser.APP.STRVIEWER, // eslint-disable-line no-undef
			development: Configs.get('development', false),
			api: true,
			width: 400,
			height: 400,
			version: Configs.get('version', '')
		});
		return true;
	}

	function onMessage(event) {
		if (typeof event.data !== 'object') {
			return;
		}

		switch (event.data.type) {
			case 'SYNC':
				ready = true;
				App.onload();
				break;

			case 'SET_HOST':
			case 'CLEAN_GRF':
				return;

			default:
				Thread.send(event.data.type, event.data.data, function () {
					App._APP.postMessage(
						{
							arguments: Array.prototype.slice.call(arguments, 0),
							uid: event.data.uid
						},
						location.origin
					);
				});
		}
	}

	function synchronise() {
		if (!ready) {
			App._APP.postMessage({ type: 'init' }, location.origin);
			setTimeout(synchronise, 100);
		}
	}

	return iconEl => {
		if (!initApp()) return;
		const root = Viewer.getRoot();
		const path = iconEl.getAttribute('data-path').replace(/\\/g, '/');
		const box = root.querySelector('#preview .box');
		const preview = root.querySelector('#preview');

		box.style.top = `${(window.innerHeight - 400) * 0.5}px`;
		preview.style.display = 'block';
		element.style.display = 'block';

		preview.addEventListener(
			'click',
			() => {
				preview.style.display = 'none';
				element.style.display = 'none';
				App._APP.postMessage({ type: 'stop' }, location.origin);
				window.removeEventListener('message', onMessage, false);
			},
			{ once: true }
		);

		window.addEventListener('message', onMessage, false);

		if (!ready) {
			box.appendChild(element);

			App.start();
			App.onReady = () => {
				App._APP.frameElement.style.border = '1px solid grey';
				App._APP.frameElement.style.backgroundColor = 'black';
				synchronise();
			};
			App.onload = () => {
				App._APP.postMessage({ type: 'load', data: path }, location.origin);
			};
		} else {
			App._APP.postMessage({ type: 'load', data: path }, location.origin);
		}
	};
})();

/**
 * User click on a map, render it using MapViewer
 */
const onWorldClick = (() => {
	let ready = false;
	let App = null;
	const element = document.createElement('div');

	function initApp() {
		if (App) return true;
		if (typeof ROBrowser === 'undefined') return false;
		App = new ROBrowser({
			target: element,
			type: ROBrowser.TYPE.FRAME, // eslint-disable-line no-undef
			application: ROBrowser.APP.MAPVIEWER, // eslint-disable-line no-undef
			development: Configs.get('development', false),
			api: true,
			width: 600,
			height: 480,
			version: Configs.get('version', '')
		});
		return true;
	}

	function onMessage(event) {
		if (typeof event.data !== 'object') {
			return;
		}

		switch (event.data.type) {
			case 'SYNC':
				ready = true;
				App.onload();
				break;

			case 'SET_HOST':
			case 'CLEAN_GRF':
				return;

			default:
				Thread.send(event.data.type, event.data.data, function () {
					App._APP.postMessage(
						{
							arguments: Array.prototype.slice.call(arguments, 0),
							uid: event.data.uid
						},
						location.origin
					);
				});
		}
	}

	function threadRedirect(type) {
		Thread.hook(type, data => {
			App._APP.postMessage(
				{
					type: type,
					data: data
				},
				location.origin
			);
		});
	}

	function synchronise() {
		if (!ready) {
			App._APP.postMessage({ type: 'init' }, location.origin);
			setTimeout(synchronise, 100);
		}
	}

	return iconEl => {
		if (!initApp()) return;
		const root = Viewer.getRoot();
		const path = iconEl.getAttribute('data-path').replace(/\\/g, '/');
		const progress = root.querySelector('#progress');
		const box = root.querySelector('#preview .box');
		const preview = root.querySelector('#preview');

		progress.style.display = 'block';
		element.style.display = 'block';

		box.style.top = `${(window.innerHeight - 480) * 0.5}px`;
		preview.style.display = 'block';
		progress.style.display = 'none';
		document.body.style.overflow = 'hidden';

		preview.addEventListener(
			'click',
			() => {
				preview.style.display = 'none';
				document.body.style.overflow = 'auto';
				element.style.display = 'none';

				App._APP.postMessage({ type: 'stop' }, location.origin);
				window.removeEventListener('message', onMessage, false);
			},
			{ once: true }
		);

		window.addEventListener('message', onMessage, false);

		if (!ready) {
			box.appendChild(element);

			App.start();
			App.onReady = () => {
				App._APP.frameElement.style.border = '1px solid grey';
				App._APP.frameElement.style.backgroundColor = 'black';

				threadRedirect('MAP_PROGRESS');
				threadRedirect('MAP_WORLD');
				threadRedirect('MAP_GROUND');
				threadRedirect('MAP_ALTITUDE');
				threadRedirect('MAP_MODELS');

				synchronise();
			};
			App.onload = () => {
				App._APP.postMessage({ type: 'load', data: path }, location.origin);
			};
		} else {
			App._APP.postMessage({ type: 'load', data: path }, location.origin);
		}
	};
})();

/**
 * User click on text, display it
 */
function onTextClick(iconEl) {
	const root = Viewer.getRoot();
	const path = iconEl.getAttribute('data-path');
	const progress = root.querySelector('#progress');
	const box = root.querySelector('#preview .box');
	const preview = root.querySelector('#preview');

	progress.style.display = 'block';

	Client.loadFile(path, text => {
		box.style.top = `${(window.innerHeight - 300) / 2}px`;
		const pre = document.createElement('pre');
		pre.textContent = text;
		pre.addEventListener('click', e => e.stopPropagation());
		Object.assign(pre.style, {
			background: 'white',
			width: '500px',
			display: 'inline-block',
			height: '300px',
			overflow: 'scroll',
			textAlign: 'left',
			padding: '10px'
		});
		box.appendChild(pre);

		progress.style.display = 'none';

		preview.style.display = 'block';
		preview.addEventListener(
			'click',
			() => {
				preview.style.display = 'none';
				box.querySelectorAll('pre').forEach(el => el.remove());
			},
			{ once: true }
		);
	});
}

/**
 * User click on a Granny model, render it using GrannyModelViewer
 */
const onGrannyClick = (() => {
	let ready = false;
	let App = null;
	const element = document.createElement('div');

	function initApp() {
		if (App) return true;
		if (typeof ROBrowser === 'undefined') return false;
		App = new ROBrowser({
			target: element,
			type: ROBrowser.TYPE.FRAME, // eslint-disable-line no-undef
			application: ROBrowser.APP.GRANNYMODELVIEWER, // eslint-disable-line no-undef
			development: Configs.get('development', false),
			api: true,
			width: 500,
			height: 400,
			version: Configs.get('version', '')
		});
		return true;
	}

	function onMessage(event) {
		if (typeof event.data !== 'object') {
			return;
		}

		switch (event.data.type) {
			case 'SYNC':
				ready = true;
				App.onload();
				break;

			case 'SET_HOST':
			case 'CLEAN_GRF':
				return;

			default:
				Thread.send(event.data.type, event.data.data, function () {
					App._APP.postMessage(
						{
							arguments: Array.prototype.slice.call(arguments, 0),
							uid: event.data.uid
						},
						location.origin
					);
				});
		}
	}

	function synchronise() {
		if (!ready) {
			App._APP.postMessage({ type: 'init' }, location.origin);
			setTimeout(synchronise, 100);
		}
	}

	return iconEl => {
		if (!initApp()) return;
		const root = Viewer.getRoot();
		const path = iconEl.getAttribute('data-path').replace(/\\/g, '/');
		const box = root.querySelector('#preview .box');
		const preview = root.querySelector('#preview');

		box.style.top = `${(window.innerHeight - 400) * 0.5}px`;
		element.style.display = 'block';

		preview.style.display = 'block';

		preview.addEventListener(
			'click',
			() => {
				preview.style.display = 'none';
				element.style.display = 'none';
				App._APP.postMessage({ type: 'stop' }, location.origin);
				window.removeEventListener('message', onMessage, false);
			},
			{ once: true }
		);

		window.addEventListener('message', onMessage, false);

		if (!ready) {
			box.appendChild(element);

			App.start();
			App.onReady = () => {
				App._APP.frameElement.style.border = '1px solid grey';
				App._APP.frameElement.style.backgroundColor = '#45484d';
				synchronise();
			};
			App.onload = () => {
				App._APP.postMessage({ type: 'load', data: path }, location.origin);
			};
		} else {
			App._APP.postMessage({ type: 'load', data: path }, location.origin);
		}
	};
})();

/**
 * Stored component and return it
 */
export default Viewer;
