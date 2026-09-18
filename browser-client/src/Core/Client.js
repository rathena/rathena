/**
 * Core/Client.js
 *
 * Client Manager
 * Manage client files, load GRFs, DATA.INI, extract files from GRFs, ...
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

// Load dependencies
import Executable from 'Utils/Executable.js';
import Configs from './Configs.js';
import Thread from './Thread.js';
import Memory from './MemoryManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import Texture from 'Utils/Texture.js';
import WebGL from 'Utils/WebGL.js';
import GraphicsSettings from 'Preferences/Graphics.js';

class Client {
	/**
	 * Initialize Client
	 * Load interesting files (executable, data.ini, GRFs, ...)
	 *
	 * @param {Array} FileList to load
	 */
	static init(files) {
		let i, count;
		const packetver = Configs.get('packetver');
		const remoteClient = Configs.get('remoteClient');

		function OnDate(date) {
			// Avoid errors
			if (date > 20000000) {
				PACKETVER.value = date;
			}
		}

		// Find executable and set the packetver
		if (!packetver || String(packetver).match(/^executable$/i)) {
			for (i = 0, count = files.length; i < count; ++i) {
				if (Executable.isROExec(files[i])) {
					Executable.getDate(files[i], OnDate);
					break;
				}
			}
		} else if (typeof packetver === 'number') {
			PACKETVER.value = packetver;
		}

		// GRF Host config
		if (remoteClient) {
			Thread.send('SET_HOST', remoteClient);
		}

		// Save full client
		savingFiles(files);
	}

	/**
	 * Get a file from Game Data files
	 *
	 * @param {string} filename
	 * @param {function} onload
	 * @param {function} onerror
	 * @param {Array} args - optional
	 */
	static getFile(filename, onload, onerror, args) {
		if (!Memory.exist(filename)) {
			Thread.send('GET_FILE', { filename, args: args || null }, onFileGetted);
		}

		return Memory.get(filename, onload, onerror);
	}

	/**
	 * Get files from Game Data files
	 *
	 * @param {string[]} filenames
	 * @param {function} callback once loaded
	 */
	static getFiles(filenames, callback) {
		let index;
		const count = filenames.length;
		const out = new Array(count);

		function onload(data) {
			out[index++] = data;

			if (index === count) {
				if (callback) {
					callback.apply(null, out);
				}
				return;
			}

			Client.getFile(filenames[index], onload);
		}

		index = 0;

		Client.getFile(filenames[index], onload);
	}

	/**
	 * Get and load a file from Game Data files
	 *
	 * @param {string} filename
	 * @param {function} onload
	 * @param {function} onerror
	 * @param {Array} args - optional
	 */
	static loadFile(filename, onload, onerror, args = {}) {
		if (!Memory.exist(filename)) {
			Thread.send('LOAD_FILE', { filename, args: args || null }, onFileLoaded);
		}

		return Memory.get(filename, onload, onerror);
	}

	/**
	 * Get and load files from Game Data files
	 *
	 * @param {string[]} filenames
	 * @param {function} callback once loaded
	 */
	static loadFiles(filenames, callback) {
		let index;
		const count = filenames.length;
		const out = new Array(count);

		function onload(data) {
			out[index++] = data;

			if (index === count) {
				if (callback) {
					callback.apply(null, out);
				}
				return;
			}

			Client.loadFile(filenames[index], onload);
		}

		index = 0;

		Client.loadFile(filenames[index], onload);
	}

	/**
	 * Apply a regex on fileList to search a file
	 *
	 * @param regex
	 * @param callback
	 */
	static search(regex, callback) {
		Thread.send('SEARCH_FILE', regex, callback);
	}

	static onFilesLoaded = () => {};
}

/**
 * Saving fullclient files in filesystem, display a progressbar during the upload
 *
 * @param {Array} FileList
 */
function savingFiles(files) {
	const progressbar = document.createElement('div');
	const info = document.createElement('div');
	let last_tick = Date.now();
	const list = [];
	let i, count;

	if (files.length) {
		// Progressbar
		progressbar.style.position = 'fixed';
		progressbar.style.zIndex = '2147483647';
		progressbar.style.top = '0px';
		progressbar.style.left = '0px';
		progressbar.style.backgroundColor = 'rgb(180,0,0)';
		progressbar.style.transition = 'width 500ms linear';
		progressbar.style.width = '0px';
		progressbar.style.height = '3px';
		progressbar.onmouseover = () => {
			info.style.display = 'block';
		};
		progressbar.onmouseout = () => {
			info.style.display = 'none';
		};

		// Progress text on hover 'Saving fullclient... (x%)'
		info.textContent = 'Saving fullclient... (0.00 %)';
		info.style.position = 'absolute';
		info.style.left = '20px';
		info.style.top = '0px';
		info.style.whiteSpace = 'nowrap';
		info.style.zIndex = '2147483646';
		info.style.height = '12px';
		info.style.padding = '5px';
		info.style.background = 'linear-gradient( rgb(180,0,0), rgb(136,0,0) 30%)';
		info.style.color = 'white';
		info.style.textShadow = '1px 1px black';
		info.style.borderBottomLeftRadius = '5px';
		info.style.borderBottomRightRadius = '5px';
		info.style.textAlign = 'center';
		info.style.width = '160px';
		info.style.display = 'none';

		document.body.appendChild(progressbar);
		document.body.appendChild(info);

		// Get progress on saving the client
		Thread.hook('CLIENT_SAVE_PROGRESS', data => {
			const now = Date.now();
			if (last_tick + 400 < now) {
				progressbar.style.width = data.total.perc + '%';
				info.textContent = 'Saving fullclient... (' + data.total.perc + ' %)';
				last_tick = now;
			}
		});

		Thread.hook('CLIENT_SAVE_COMPLETE', () => {
			if (progressbar.parentNode) {
				document.body.removeChild(progressbar);
			}
			if (info.parentNode) {
				document.body.removeChild(info);
			}
		});

		// Seems like files property are reset when sent to another thread
		for (i = 0, count = files.length; i < count; ++i) {
			list.push({
				file: files[i],
				path: files[i].fullPath || files[i].relativePath || files[i].webkitRelativePath || files[i].name
			});
		}
	}

	// Get temporary storage info at main thread, the worker can't access it.
	// https://github.com/vthibault/roBrowser/issues/110
	const temporaryStorage = navigator.temporaryStorage ||
		navigator.webkitTemporaryStorage || {
			queryUsageAndQuota: function (callback) {
				callback(0, 0);
			}
		};

	temporaryStorage.queryUsageAndQuota((used, remaining) => {
		const quota = {
			used: used,
			remaining: remaining
		};

		// Initialize client files (load GRF, etc).
		Thread.send(
			'CLIENT_INIT',
			{
				files: list,
				grfList: Configs.get('grfList') || 'DATA.INI',
				save: !!Configs.get('saveFiles'),
				quota: quota
			},
			(...args) => Client.onFilesLoaded(...args)
		);
	});
}

function onFileGetted(data, error, input) {
	Memory.set(input.filename, data, error);
}

async function onFileLoaded(data, error, input) {
	let i, count, j, size;
	let gl, frames, texture, layers, palette;
	let precision;

	if (data && !error) {
		switch (input.filename.substr(-3)) {
			// Remove magenta on textures
			case 'bmp':
				Texture.load(data, function () {
					Memory.set(input.filename, this.toDataURL(), error);
				});
				return;

			// Load str textures
			case 'str':
				gl = (await import('Renderer/Renderer.js')).default.getContext();
				layers = data.layers;

				for (i = 0; i < data.layernum; ++i) {
					layers[i].materials = new Array(layers[i].texcnt);

					for (j = 0; j < layers[i].texcnt; ++j) {
						(function (url, materials, textureId) {
							Client.loadFile(url, function (loadedUrl) {
								WebGL.texture(gl, loadedUrl, function (loadedTexture) {
									materials[textureId] = loadedTexture;
								});
							});
						})(layers[i].texname[j], layers[i].materials, j);
					}
				}

				Memory.set(input.filename, data, error);
				return;

			case 'spr':
				gl = (await import('Renderer/Renderer.js')).default.getContext();
				frames = data.frames;
				count = frames.length;

				// Send sprites to GPU
				for (i = 0; i < count; i++) {
					frames[i].texture = gl.createTexture();
					precision = GraphicsSettings.pixelPerfectSprites
						? gl.NEAREST
						: frames[i].type
							? gl.LINEAR
							: gl.NEAREST;
					size = frames[i].type ? gl.RGBA : gl.LUMINANCE;
					gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
					gl.bindTexture(gl.TEXTURE_2D, frames[i].texture);
					gl.texImage2D(
						gl.TEXTURE_2D,
						0,
						size,
						frames[i].width,
						frames[i].height,
						0,
						size,
						gl.UNSIGNED_BYTE,
						frames[i].data
					);
					gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, precision);
					gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, precision);
					gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
					gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
				}

				// Send palette to GPU
				if (data.rgba_index !== 0) {
					data.texture = gl.createTexture();
					gl.bindTexture(gl.TEXTURE_2D, data.texture);
					gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 256, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, data.palette);
					gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
					gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
				}

				Memory.set(input.filename, data, error);
				return;

			// Build palette
			case 'pal': {
				const enableMipmap = Configs.get('enableMipmap');
				gl = (await import('Renderer/Renderer.js')).default.getContext();
				texture = gl.createTexture();
				palette = new Uint8Array(data);

				gl.bindTexture(gl.TEXTURE_2D, texture);
				gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 256, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, palette);
				gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
				gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
				if (enableMipmap) {
					gl.generateMipmap(gl.TEXTURE_2D);
				}

				Memory.set(input.filename, { palette: palette, texture: texture }, error);
				return;
			}
		}
	}

	Memory.set(input.filename, data, error);
}

/**
 * Export
 */
export default Client;
