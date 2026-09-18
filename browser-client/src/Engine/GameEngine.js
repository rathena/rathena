/**
 * Engine/GameEngine.js
 *
 * Game Engine
 * Global game Engine
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

// Load dependencies
import Queue from 'Utils/Queue.js';
import Sound from 'Audio/SoundManager.js';
import BGM from 'Audio/BGM.js';
import DB from 'DB/DBManager.js';
import Configs from 'Core/Configs.js';
import Client from 'Core/Client.js';
import Thread from 'Core/Thread.js';
import Context from 'Core/Context.js';
import LoginEngine from 'Engine/LoginEngine.js';
import Network from 'Network/NetworkManager.js';
import Renderer from 'Renderer/Renderer.js';
import MapRenderer from 'Renderer/MapRenderer.js';
import UIManager from 'UI/UIManager.js';
import Cursor from 'UI/CursorManager.js';
import Scrollbar from 'UI/Scrollbar.js';
import Background from 'UI/Background.js';
import Intro from 'UI/Components/Intro/Intro.js';
import WinList from 'UI/Components/WinList/WinList.js';
import ConsoleManager from 'Utils/ConsoleManager.js';
import TextEncoding from 'Utils/CodepageManager.js';
import { roInitSpinner } from 'App/PreLoader.js';

/**
 * @var {Array} Login server list
 */
let _servers = [];

/**
 * @var {server} the previously selected server (if set)
 */
let _previous_server = undefined;

/**
 * @var {boolean} is thread ready ? (fix)
 */
let _thread_ready = false;

/**
 * Load files.
 */
function loadFiles(callback) {
	const q = new Queue();

	// Start Intro, wait the user to add files
	q.add(() => {
		Client.onFilesLoaded = count => {
			if (!Configs.get('remoteClient') && !count && !window.electronAPI?.isElectron) {
				alert('No client to initialize roBrowser');
				Intro.remove();
				Intro.append();
				return;
			}
			q._next();
		};

		if (Configs.get('skipIntro')) {
			Client.init([]);
			return;
		}

		Intro.onFilesSubmit = Client.init.bind(Client);
		Intro.append();
	});

	// Loading Game file (txt, lua, lub)
	q.add(() => {
		// Remove preloader now that Background is taking over.
		// For skipIntro=false, Intro.onAppend already removed it (this is a safe no-op).
		roInitSpinner.remove();

		DB.onReady = () => {
			q._next();
		};
		DB.onProgress = (i, count) => {
			Background.setPercent(Math.floor((i / count) * 100));
		};
		UIManager.removeComponents();
		Background.init();
		Background.resize(Renderer.width, Renderer.height);
		Background.setImage('bgi_temp.bmp', () => {
			DB.init();
		});
	});

	q.add(() => {
		Thread.send('CLIENT_FILES_ALIAS', DB.mapalias);
		loadClientInfo(q.next);
	});

	// Initialize cursor
	q.add(() => {
		Scrollbar.init();
		Cursor.init(q.next);
	});

	// Run callback
	q.add(() => {
		callback();
	});

	// Execute
	q.run();
}

class GameEngine {
	/**
	 * Initialize Game
	 */
	static init() {
		// Enable/Disable console based on settings
		ConsoleManager.init();
		ConsoleManager.toggle();

		const q = new Queue();

		// Waiting for the Thread to be ready
		q.add(() => {
			if (!_thread_ready) {
				Thread.hook('THREAD_ERROR', onThreadError);
				Thread.hook('THREAD_LOG', onThreadLog);
				Thread.hook('THREAD_READY', () => {
					_thread_ready = true;
					q._next();
				});
				Thread.init();
			} else {
				q._next();
			}
		});

		// Initialize renderer
		q.add(() => {
			Renderer.init();
			q._next();
		});

		// Load everything.
		q.add(() => {
			// Load files and initialize Login
			loadFiles(GameEngine.reload);
		});

		Context.checkSupport();

		// Execute
		q.run();

		// NOTE: roInitSpinner.remove() was removed from here.
		// It is now called inside loadFiles() at the right async moment.
	}

	/**
	 * Reload the game
	 */
	static reload() {
		BGM.setAvailableExtensions(Configs.get('BGMFileExtension', ['mp3']));
		BGM.play('01.mp3');

		UIManager.removeComponents();
		Network.close();
		// Setup background
		Background.init();
		Background.resize(Renderer.width, Renderer.height);
		Background.setImage('bgi_temp.bmp', () => {
			onReload();
		});
		// Hooking WinList
		WinList.onIndexSelected = onLoginServerSelected;
		WinList.onExitRequest = onExit;
	}
}
function onReload() {
	// Display server list
	const list = new Array(_servers.length);
	let i;
	const count = list.length;

	// WTF no servers ?
	if (count === 0) {
		UIManager.showMessageBox('Sorry, no server found.', 'ok', GameEngine.init);
	}

	// Just 1 server, skip the WinList
	else if (count === 1 && Configs.get('skipServerList')) {
		LoginEngine.init(_servers[0]);
	} else {
		for (i = 0; i < count; ++i) {
			list[i] = _servers[i].display;
		}

		WinList.append();
		WinList.setList(list);
	}
	Renderer.stop();
	MapRenderer.free();
	BGM.play('01.mp3');
}

function onReadyLoginServer(index) {
	// Set the previous server.
	_previous_server = _servers[index];

	WinList.remove();
	LoginEngine.init(_servers[index]);
}

/**
 * Once a server is selected
 *
 * @param {number} index in server list
 */
function onLoginServerSelected(index) {
	// Play "¹öÆ°¼Ò¸®.wav" (possible problem with charset)
	Sound.play('\xB9\xF6\xC6\xB0\xBC\xD2\xB8\xAE.wav');

	// Check if the selected server is different than the previous one.
	if (
		_previous_server !== undefined &&
		(_previous_server.address != _servers[index].address || _previous_server.port != _servers[index].port)
	) {
		UIManager.removeComponents();
		Network.close();
		// Need to reload the files.
		loadFiles(() => {
			LoginEngine.setLoadedServer(_servers[index]);
			onReadyLoginServer(index);
		});
	} else {
		onReadyLoginServer(index);
	}
}

/**
 * Ask to exit window
 */
function onExit() {
	Sound.stop();
	Renderer.stop();
	UIManager.removeComponents();
	GameEngine.reload();
}

/**
 * Loading clientinfo file
 *
 * @param {function} callback
 */
function loadClientInfo(callback) {
	const servers = Configs.get('servers', 'data/clientinfo.xml');

	if (servers instanceof Array) {
		_servers = servers;
		callback();
		return;
	}

	_servers.length = 0;
	Client.loadFile(
		servers,
		xml => {
			// $.parseXML() don't parse buggy xml (and a lot of clientinfo.xml are not properly write)...
			if (xml instanceof Uint8Array) {
				xml = TextEncoding.decode(xml);
			}
			xml = xml.replace(/^.*<\?xml/, '<?xml');
			const parser = new DOMParser();
			const doc = parser.parseFromString(xml, 'application/xml');

			const _getChildText = (parent, tag) => {
				const el = parent.querySelector(tag);
				return el ? el.textContent : '';
			};

			const connections = doc.querySelectorAll('clientinfo connection');

			if (!connections.length) {
				callback();
				return;
			}

			connections.forEach((element, index) => {
				_servers.push({
					display: _getChildText(element, 'display'),
					desc: _getChildText(element, 'desc'),
					address: _getChildText(element, 'address'),
					port: _getChildText(element, 'port'),
					version: _getChildText(element, 'version'),
					langtype: _getChildText(element, 'langtype'),
					packetver: _getChildText(element, 'packetver'),
					registrationweb: _getChildText(element, 'registrationweb'),
					renewal: ['true', '1', 1, true].includes(_getChildText(element, 'renewal').toLowerCase()),
					adminList: (() => {
						const _list = [];
						element.querySelectorAll('yellow admin, aid admin').forEach(adminEl => {
							_list.push(parseInt(adminEl.textContent, 10));
						});
						return _list;
					})()
				});

				if (index === connections.length - 1) {
					callback();
				}
			});
		},
		callback
	);
}

/**
 * When getting an error from Thread
 *
 * @param {Array} data
 */
function onThreadError(data) {
	console.warn.apply(console, data);
}

/**
 * Received log from Thread
 *
 * @param {Array} data
 */
function onThreadLog(data) {
	console.log.apply(console, data);
}

/**
 * Export
 */
export default GameEngine;
