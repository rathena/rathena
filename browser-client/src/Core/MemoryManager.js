/**
 * Core/MemoryManager.js
 *
 * Memory Manager
 *
 * Set up a cache context to avoid re-loading/parsing files each time, files are removed automatically if not used
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import MemoryItem from 'Core/MemoryItem.js';

/**
 * List of files in memory
 * @var List MemoryItem
 */
const _memory = {};

/**
 * Remove files from memory if not used until a period of time.
 * PHP servers: 30s is safe to avoid high memory usage on shared hosting.
 * Node.js (RemoteClient-JS): 120-300s recommended, heap is usually abundant.
 * @var {number}
 */
const _rememberTime = 30 * 1000; // 30s

/**
 * @var {number} last time we clean up variables
 */
let _lastCheckTick = 0;

/**
 * @var {number} perform the clean up every 10 secs
 */
const _cleanUpInterval = 10 * 1000;

/**
 * Async cleanup state tracking.
 * These variables are used to split memory cleanup into small chunks
 * to avoid blocking the main thread during large clean operations.
 */
let _cleaningInProgress = false; // Prevents multiple clean cycles running at the same time
let _cleanIndex = 0; // Tracks the current cleanup position
let _filesToClean = []; // List of memory entries scheduled for removal
class MemoryManager {
	/**
	 * Get back data from memory
	 *
	 * @param {string} filename
	 * @param {function} onload - optional
	 * @param {function} onerror - optional
	 * @return mixed data
	 */
	static get = (filename, onload, onerror) => {
		// Not in memory yet, create slot
		if (!_memory[filename]) {
			_memory[filename] = new MemoryItem();
		}

		const item = _memory[filename];

		if (onload) {
			item.addEventListener('load', onload);
		}

		if (onerror) {
			item.addEventListener('error', onerror);
		}

		return item.data;
	};

	/**
	 * Check if the entry exists
	 *
	 * @param {string} filename
	 * @return boolean isInMemory
	 */
	static exist = filename => {
		return !!_memory[filename];
	};

	/**
	 * Stored data in memory
	 *
	 * @param {string} filename
	 * @param {string|object} data
	 * @param {string} error - optional
	 */
	static set = (filename, data, error) => {
		// Not in memory yet, create slot
		if (!_memory[filename]) {
			_memory[filename] = new MemoryItem();
		}

		if (error || !data) {
			_memory[filename].onerror(error);
		} else {
			_memory[filename].onload(data);
		}
	};

	/**
	 * Clean up not used data from memory
	 *
	 * @param {object} gl - WebGL Context
	 * @param {number} now - game tick
	 */
	static clean = (gl, now) => {
		// Skip cleanup if interval has not elapsed or if an async cleanup is already running
		if (_lastCheckTick + _cleanUpInterval > now || _cleaningInProgress) {
			return;
		}

		const files = [];
		_filesToClean = []; // Reset pending cleanup list

		const keys = Object.keys(_memory);
		const count = keys.length;
		const tick = now - _rememberTime;

		for (let i = 0; i < count; ++i) {
			const item = _memory[keys[i]];
			if (item.complete && item.lastTimeUsed < tick) {
				_filesToClean.push(keys[i]); // Collect unused memory entries instead of removing them immediately
			}
		}
		// If nothing needs to be cleaned, just update the last check timestamp
		if (_filesToClean.length === 0) {
			_lastCheckTick = now;
			return;
		}

		// Mark cleanup as running to avoid re-entry
		_cleaningInProgress = true;
		_cleanIndex = 0;

		// Perform cleanup incrementally during idle time to reduce frame drops
		requestIdleCallback(function cleanChunk(deadline) {
			let processed = 0;
			// Limit the number of removals per idle callback
			const maxProcess = Math.min(5, _filesToClean.length - _cleanIndex);

			while (_cleanIndex < _filesToClean.length && processed < maxProcess && deadline.timeRemaining() > 0) {
				MemoryManager.remove(gl, _filesToClean[_cleanIndex]);
				files.push(_filesToClean[_cleanIndex]);
				_cleanIndex++;
				processed++;
			}

			if (_cleanIndex < _filesToClean.length) {
				// Continue cleanup in the next idle period
				requestIdleCallback(cleanChunk);
			} else {
				// Cleanup finished
				_cleaningInProgress = false;
				_lastCheckTick = now;
				_filesToClean = [];

				if (files.length) {
					console.log(
						'%c[MemoryManager] - Removed ' + files.length + ' unused elements from memory.',
						'color:#d35111',
						{ files }
					);
				}
			}
		});
	};

	/**
	 * Force immediate cleanup of memory entries matching an optional regex.
	 * Useful after bulk loading (e.g., DB.lazyInit) to free parsed file data.
	 *
	 * @param {object} gl - WebGL Context (can be null)
	 * @param {RegExp} [regex] - Optional pattern to match filenames. If omitted, all complete items are removed.
	 */
	static forceClean = async (gl, regex) => {
		const keys = Object.keys(_memory);
		const removed = [];

		keys.forEach(key => {
			const item = _memory[key];
			if (item.complete && (!regex || key.match(regex))) {
				MemoryManager.remove(gl, key);
				removed.push(key);
			}
		});

		if (removed.length) {
			console.log('%c[MemoryManager] - Removed ' + removed.length + ' elements from memory.', 'color:#d35111', {
				files: removed
			});
		}
	};

	/**
	 * Remove Item from memory
	 *
	 * @param {object} gl - WebGL Context
	 * @param {string} filename
	 */
	static remove = (gl, filename) => {
		// Not found or filename is undefined?
		if (!filename || !_memory[filename]) {
			return;
		}

		const file = MemoryManager.get(filename);
		let ext = '';
		let i, count;

		const matches = filename.match(/\.[^.]+$/);

		if (matches) {
			ext = matches.toString().toLowerCase();
		}

		// Free file
		if (file) {
			switch (ext) {
				// Delete GPU textures from sprites
				case '.spr':
					if (file.frames) {
						for (i = 0, count = file.frames.length; i < count; ++i) {
							if (file.frames[i].texture && gl != null && gl.isTexture(file.frames[i].texture)) {
								gl.deleteTexture(file.frames[i].texture);
							}
						}
					}
					if (file.texture && gl != null && gl.isTexture(file.texture)) {
						gl.deleteTexture(file.texture);
					}
					break;

				// Delete palette
				case '.pal':
					if (file.texture && gl != null && gl.isTexture(file.texture)) {
						gl.deleteTexture(file.texture);
					}
					break;

				// Delete GPU textures from STR effects
				case '.str':
					if (file.layers) {
						for (i = 0, count = file.layers.length; i < count; ++i) {
							if (file.layers[i].materials) {
								for (let j = 0, matCount = file.layers[i].materials.length; j < matCount; ++j) {
									if (
										file.layers[i].materials[j] &&
										gl != null &&
										gl.isTexture(file.layers[i].materials[j])
									) {
										gl.deleteTexture(file.layers[i].materials[j]);
									}
								}
							}
						}
					}
					break;

				// If file is a blob, remove it (wav, mp3, lua, lub, txt, ...)
				default:
					if (file.match && file.match(/^blob:/)) {
						URL.revokeObjectURL(file);
					}
					break;
			}
		}

		// Delete from memory
		delete _memory[filename];
	};

	/**
	 * Search files in memory based on a regex
	 *
	 * @param regex
	 * @return string[] filename
	 */
	static search = regex => {
		return Object.keys(_memory).filter(k => k.match(regex));
	};
}
/**
 * Export methods
 */
export default MemoryManager;
