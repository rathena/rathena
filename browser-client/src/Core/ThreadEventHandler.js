/**
 * Core/ThreadEventHandler.js
 *
 * Handler data received from Main Thread and process.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import FileManager from 'Core/FileManager.js';
import FileSystem from 'Core/FileSystem.js';
import MapLoader from 'Loaders/MapLoader.js';

/**
 *	Send an Error to main thread
 *
 * @param {string} error
 */
function sendError() {
	postMessage({ type: 'THREAD_ERROR', data: Array.prototype.slice.call(arguments, 0) });
}

/**
 *	Send a message log to main thread
 *
 * @param {string} error
 */
function sendLog() {
	postMessage({ type: 'THREAD_LOG', data: Array.prototype.slice.call(arguments, 0) });
}

/**
 * Receiving data, process action
 *
 * @param {object} event - EventHandler
 */
onmessage = function receive(event) {
	const msg = event.data;

	switch (msg.type) {
		// Modify client host
		case 'SET_HOST':
			if (msg.data.substr(-1) !== '/') {
				msg.data += '/';
			}

			FileManager.remoteClient = msg.data;
			break;

		// Save full client and use it
		case 'CLIENT_INIT':
			FileSystem.bind('onprogress', function (progress) {
				postMessage({ type: 'CLIENT_SAVE_PROGRESS', data: progress });
			});

			// full client saved !
			FileSystem.bind('onuploaded', function () {
				postMessage({ type: 'CLIENT_SAVE_COMPLETE' });
			});

			FileManager.onGameFileLoaded = function (filename) {
				sendLog('Success to load GRF file "' + filename + '"');
			};

			FileManager.onGameFileError = function (filename, error) {
				sendError('Error loading GRF file "' + filename + '" : ' + error);
			};

			// Start loading GRFs files
			FileSystem.bind('onready', function () {
				FileManager.clean();
				FileManager.init(msg.data.grfList);

				postMessage({
					uid: msg.uid,
					arguments: [FileManager.gameFiles.length, null, msg.data]
				});
			});

			// Saving full client
			FileSystem.init(msg.data.files, msg.data.save, msg.data.quota);
			break;

		// Files alias
		case 'CLIENT_FILES_ALIAS':
			FileManager.filesAlias = msg.data;
			break;

		// Get a file from client/grf
		case 'GET_FILE':
			FileManager.get(msg.data.filename, function (result, error) {
				if (error) {
					sendError('[Thread] ' + error + ' (' + msg.data.filename + ')');
				}

				if (msg.uid) {
					postMessage({
						uid: msg.uid,
						arguments: [result, error, msg.data]
					});
				}
			});
			break;

		// Get and load a file from client/grf
		case 'LOAD_FILE':
			FileManager.load(
				msg.data.filename,
				function (result, error) {
					if (error) {
						sendError('[Thread] ' + error + ' (' + msg.data.filename + ')');
					}

					if (msg.uid) {
						postMessage({
							uid: msg.uid,
							arguments: [result, error, msg.data]
						});
					}
				},
				msg.data.args
			);
			break;

		// Search a file in Client
		case 'SEARCH_FILE':
			if (msg.uid) {
				postMessage({
					uid: msg.uid,
					arguments: [FileManager.search(msg.data), null, msg.data]
				});
			}
			break;

		// Start loading a map
		case 'LOAD_MAP': {
			const map = new MapLoader();

			map.onprogress = function (progress) {
				postMessage({ type: 'MAP_PROGRESS', data: progress });
			};

			map.onload = function (success, error) {
				if (msg.uid) {
					postMessage({
						uid: msg.uid,
						arguments: [success, error, msg.data]
					});
				}
			};

			map.ondata = function (type, data) {
				postMessage({ type: type, data: data });
			};

			map.load(msg.data);
			break;
		}
	}
};

/**
 * Once the thread is ready
 */
postMessage({ type: 'THREAD_READY' });
