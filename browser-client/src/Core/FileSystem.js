/**
 * Core/FileSystem.js
 *
 * File System
 * Manage the client files (saving it)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * @param {Array} FileList
 */
let _files = [];

/**
 * @param {number} client total size (in octets)
 */
let _clientSize = 0;

/**
 * @param {number} progress octects when uploading
 */
let _streamOffset = 0;

/**
 * @param {Object} Events list
 */
const _events = {};

/**
 * @param {FileSyStem} sync
 */
let _fs_sync;

/**
 * @param {FileSyStem} async (used for streaming)
 */
let _fs;

/**
 * @param {boolean} is API available ? (do not need to check for the async)
 */
const _available = !!(self.requestFileSystemSync || self.webkitRequestFileSystemSync);

/**
 * @param {boolean} save data to file system ?
 */
let _save = false;

/**
 * Normalize file path
 *
 * @param {array} FileList
 * @returns {array} normalized filelist
 */
function normalizeFilesPath(files) {
	let i, count;
	const list = new Array(files.length);
	const backslash = /\\\\/g;

	for (i = 0, count = files.length; i < count; ++i) {
		list[i] = files[i].file;
		list[i]._path = files[i].path.replace(backslash, '/');
	}

	return list;
}

/**
 * Error Handler give a human error
 */
function errorHandler(e) {
	let msg = '';
	const FileError = {
		QUOTA_EXCEEDED_ERR: 22,
		NOT_FOUND_ERR: 1,
		SECURITY_ERR: 2,
		INVALID_MODIFICATION_ERR: 9,
		INVALID_STATE_ERR: 7
	};
	switch (e.code) {
		case FileError.QUOTA_EXCEEDED_ERR:
			msg = 'QUOTA_EXCEEDED_ERR';
			break;
		case FileError.NOT_FOUND_ERR:
			msg = 'NOT_FOUND_ERR';
			break;
		case FileError.SECURITY_ERR:
			msg = 'SECURITY_ERR';
			break;
		case FileError.INVALID_MODIFICATION_ERR:
			msg = 'INVALID_MODIFICATION_ERR';
			break;
		case FileError.INVALID_STATE_ERR:
			msg = 'INVALID_STATE_ERR';
			break;
		default:
			msg = 'Unknown Error';
			break;
	}

	trigger('onerror', msg);
}

/**
 * Calculate FullClient total size
 * @returns {integer}
 */
function calculateClientSize() {
	let i, count;

	_clientSize = 0;

	for (i = 0, count = _files.length; i < count; ++i) {
		_clientSize += _files[i].size || 0;
	}
}

/**
 * Start to upload files to FileSystem (async !)
 *
 * @param {number} index
 */
function processUpload(index) {
	const file = _files[index];

	// Finished.
	if (index >= _files.length) {
		let i, count;

		// Move all files from the directory to root.
		const tmpDir = _fs_sync.root.getDirectory('/__tmp_upload/', {});
		const dirReader = tmpDir.createReader();
		const entries = dirReader.readEntries();

		for (i = 0, count = entries.length; i < count; ++i) {
			entries[i].moveTo(_fs_sync.root, entries[i].name);
		}

		tmpDir.removeRecursively();
		_files.length = 0;

		trigger('onuploaded');
		return;
	}

	if (file.name[0] === '.') {
		_files.splice(index, 1);
		processUpload(index);
		return;
	}

	_fs.root.getFile(
		'/__tmp_upload/' + file._path,
		{ create: true },
		function (fileEntry) {
			fileEntry.createWriter(function (writer) {
				writer.onerror = errorHandler;
				writer.onwriteend = function () {
					_streamOffset += file.size;
					processUpload(index + 1);
				};

				let last_tick = Date.now();
				writer.onprogress = function (evt) {
					// Do not spam the main thread
					const now = Date.now();
					if (last_tick + 100 > now) {
						return;
					}

					last_tick = now;
					trigger('onprogress', {
						filename: file.name,
						filePath: file._path,
						file: {
							total: evt.total,
							loaded: evt.loaded,
							perc: ((evt.loaded / evt.total) * 100).toFixed(2)
						},
						total: {
							total: _clientSize,
							loaded: _streamOffset + evt.loaded,
							perc: (((_streamOffset + evt.loaded) / _clientSize) * 100).toFixed(2)
						}
					});
				};

				writer.write(file);
			});
		},
		errorHandler
	);
}

/**
 * Build directory hierarchy
 */
function buildHierarchy() {
	const cache = {};
	let i = 0,
		count = _files.length;
	let path;
	const filename = /\/?[^/]+$/;

	// Extract directory from each file path
	for (; i < count; ++i) {
		path = _files[i]._path.split('/').slice(0, -1).join('/');
		while (!(path in cache) && path.length) {
			cache[path] = true;
			path = path.replace(filename, '');
		}
	}

	// Extract keys and build directories
	const keys = Object.keys(cache);
	keys.sort();

	// Directory where to upload data
	_fs_sync.root.getDirectory('/__tmp_upload/', { create: true });

	for (i = 0, count = keys.length; i < count; ++i) {
		_fs_sync.root.getDirectory('/__tmp_upload/' + keys[i], { create: true });
	}
}

/**
 * Trigger an event
 *
 * @param {string} eventname
 * @param {mixed...}
 */
function trigger(eventname) {
	if (_events[eventname]) {
		_events[eventname].apply(null, Array.prototype.slice.call(arguments, 1));
	}
}
class FileSystem {
	/**
	 * Bind an event
	 *
	 * @param {string} eventname
	 * @param {function} callback
	 */
	static bind(eventname, callback) {
		_events[eventname] = callback;
	}

	/**
	 * Get a file in FileSystem (sync)
	 *
	 * @param {string} filename
	 * @returns {File}
	 */
	static getFileSync(filename) {
		filename = filename.replace(/\\/g, '/');

		if (!_available || _files.length) {
			let i;
			const count = _files.length;

			for (i = 0; i < count; ++i) {
				// Not case sensitive...
				if (_files[i]._path.toLowerCase() === filename.toLowerCase()) {
					return _files[i];
				}
			}

			return null;
		}

		let fileEntry;

		try {
			fileEntry = _fs_sync.root.getFile(filename, { create: false });
		} catch (_e) {
			// not found
			return null;
		}

		if (fileEntry.isFile) {
			return fileEntry.file();
		}

		return null;
	}

	/**
	 * Get a file in FileSystem (async)
	 *
	 * @param {string} filename
	 * @param {function} once loaded
	 * @param {function} callback if not found
	 */
	static getFile(filename, onload, onerror) {
		filename = filename.replace(/\\/g, '/');

		if (!_available || _files.length) {
			let i;
			const count = _files.length;

			for (i = 0; i < count; ++i) {
				// Not case sensitive...
				if (_files[i]._path.toLowerCase() === filename.toLowerCase()) {
					onload(_files[i]);
					return;
				}
			}

			onerror();
			return;
		}

		_fs.root.getFile(
			filename,
			{ create: false },
			fileEntry => {
				if (fileEntry.isFile) {
					fileEntry.file(onload);
				} else {
					onerror();
				}
			},
			onerror
		);
	}

	/**
	 * Save the content of a files in file system
	 * (used to save the remote client)
	 *
	 * @param {string} filePath
	 * @param {ArrayBuffer} buffer
	 */
	static saveFile(filePath, buffer) {
		if (!_save || !_available) {
			return;
		}

		const filename = filePath.replace(/\\/g, '/');
		const directories = filename.split('/').slice(0, -1);
		let path = '';

		// Create hierarchy
		while (directories.length) {
			path += directories.shift() + '/';
			_fs_sync.root.getDirectory(path, { create: true });
		}

		const fileEntry = _fs_sync.root.getFile(filename, { create: true });
		const writer = fileEntry.createWriter();

		writer.write(new Blob([buffer]));
	}

	/**
	 * Search a file from FileSystem using a regex
	 *
	 * @param {RegExp|string} to match the filename
	 */
	static search(regex) {
		let i, count;
		const list = [];

		if (!(regex instanceof RegExp)) {
			regex = new RegExp('^' + regex.replace(/([.*+?^=!:${}()|[\]/\\])/g, '\\$1') + '$', 'i');
		}

		if (!_available || _files.length) {
			for (i = 0, count = _files.length; i < count; ++i) {
				if (_files[i].name.match(regex)) {
					list.push(_files[i]);
				}
			}

			return list;
		}

		const entries = _fs_sync.root.createReader().readEntries();

		for (i = 0, count = entries.length; i < count; ++i) {
			if (entries[i].isFile && entries[i].name.match(regex)) {
				list.push(entries[i].file());
			}
		}

		return list;
	}

	/**
	 * Remove all files from FileSystem
	 */
	static cleanup() {
		let i, count;
		const dirReader = _fs_sync.root.createReader();
		const entries = dirReader.readEntries();
		let retryCount = 0;
		const maxRetries = 3;

		function removeWithRetry(entry, callback) {
			try {
				if (entry.isDirectory) {
					entry.removeRecursively(callback, callback);
				} else {
					entry.remove(callback, callback);
				}
			} catch (e) {
				if (retryCount < maxRetries && e.name === 'InvalidModificationError') {
					retryCount++;
					setTimeout(() => {
						removeWithRetry(entry, callback);
					}, 100);
				} else {
					callback(e);
				}
			}
		}

		for (i = 0, count = entries.length; i < count; ++i) {
			removeWithRetry(entries[i], error => {
				if (error) {
					console.warn('Failed to remove entry:', error);
				}
			});
		}
	}

	/**
	 * Initialize FileSystem API
	 *
	 * @param {Array} FileList
	 * @param {boolean} save files
	 * @param {Object} quota information
	 */
	static init(files, save, quota) {
		_files = normalizeFilesPath(files);

		if (!_available) {
			trigger('onready');
			return;
		}

		calculateClientSize();

		const requestFileSystemSync = self.requestFileSystemSync || self.webkitRequestFileSystemSync;
		const requestFileSystem = self.requestFileSystem || self.webkitRequestFileSystem;

		const size = _clientSize || quota.used || quota.remaining;

		requestFileSystem(
			self.TEMPORARY,
			size,
			fs => {
				_fs = fs;
				_fs_sync = requestFileSystemSync(self.TEMPORARY, size);

				if (save && _files.length) {
					FileSystem.cleanup();
					buildHierarchy();
					processUpload(0);
				}

				_save = save;
				trigger('onready');
			},
			errorHandler
		);
	}
}

/**
 * Public methods
 */
export default FileSystem;
