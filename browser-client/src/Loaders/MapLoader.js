/**
 * Loaders/MapLoader.js
 *
 * Loaders for Ragnarok Map
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import FileManager from 'Core/FileManager.js';

/**
 * Helper to load list
 *
 * @param {array} file list to load
 */
class Loader {
	constructor(list) {
		this.files = list;
		this.list = list.slice(0);
		this.offset = 0;
		this.count = list.length;
		this.out = new Array(this.count);
	}

	/**
	 * @var {number} static file count (avoid "too much recursion from firefox")
	 */
	static count = 0;

	/**
	 * @var {number} How many files do you want to load at the same time ?
	 * PHP servers: keep at 4-6 to avoid connection exhaustion.
	 * Node.js (RemoteClient-JS): supports 12+ with HTTP/2 multiplexing.
	 */
	parallelDownload = 6;

	/**
	 * Start to load the list
	 */
	start() {
		let i;
		this.offset = 0;

		// No files...
		if (!this.list.length) {
			this.onload(this.list, this.list);
			return;
		}

		for (i = 0; i < this.count && i < this.parallelDownload; ++i) {
			this._next();
		}
	}

	/**
	 * Next file to load
	 *
	 * @param {number} index in list
	 */
	_next() {
		// Possible problem with setTimeout
		if (!this.list.length) {
			return;
		}

		const filename = this.list.shift();
		FileManager.load(filename, data => {
			// Store the result
			this.out[this.files.indexOf(filename)] = data;
			this.offset++;

			// Start the progress
			if (this.onprogress && this.offset <= this.count) {
				this.onprogress(this.offset, this.count);
			}

			// Ended ?
			if (this.offset === this.count) {
				this.onload(this.out, this.files);
				return;
			}

			// Continue the queue
			if (this.list.length) {
				// To fix "too much recursion" on Firefox
				if (++Loader.count % 50 === 0) {
					setTimeout(() => this._next(), 4);
				} else {
					this._next();
				}
			}
		});
	}
}
/**
 * MapLoader constructor
 *
 * @param {string} mapname
 */
class MapLoader {
	constructor(mapname) {
		if (mapname) {
			this.load(mapname);
		}
	}

	/**
	 * Count files to load
	 * @var integer
	 */
	fileCount = 0;

	/**
	 * MapLoader Progress
	 * @var integer
	 */
	progress = 0;

	/**
	 * Offset in the progress
	 * @var integer
	 */
	offset = 0;

	/**
	 * MapLoader update progress
	 *
	 * @param {number} percent
	 */
	setProgress(percent) {
		const progress = Math.min(100, Math.floor(percent));

		if (progress !== this.progress) {
			if (this.onprogress) {
				this.onprogress(progress);
			}
			this.progress = progress;
		}
	}

	/**
	 * Load a map
	 *
	 * @param {string} mapname
	 */
	load(mapname) {
		// Initialize the loading
		this.setProgress(0);

		const loader = this;
		let world;

		//  Get file path (if it's a copy of a file)
		function getFilePath(path) {
			if (path in FileManager.filesAlias) {
				return FileManager.filesAlias[path];
			}

			return path;
		}

		// loading world
		function onWorldReady(resourceWorld) {
			if (!resourceWorld) {
				loader.onload(false, `Can't find file "${mapname}" ! `);
				return;
			}

			world = resourceWorld;
			loader.setProgress(1);

			// Load Altitude
			FileManager.load(`data\\${getFilePath(world.files.gat)}`, onAltitudeReady);
		}

		// Loading altitude
		function onAltitudeReady(altitude) {
			if (!altitude) {
				loader.onload(false, `Can't find file "${world.files.gat}" !`);
				return;
			}

			loader.setProgress(2);
			loader.ondata('MAP_ALTITUDE', altitude.compile());

			FileManager.load(`data\\${getFilePath(world.files.gnd)}`, onGroundReady);
		}

		// Load ground
		function onGroundReady(ground) {
			if (!ground) {
				loader.onload(false, `Can't find file "${world.files.gnd}" !`);
				return;
			}

			loader.setProgress(3);

			// Compiling ground
			if (ground && ground.version >= 1.8) {
				world.water = ground.water;
			}
			const compiledGround = ground.compile(world.water.level, world.water.waveHeight);

			// Just to approximate, guess we have 2 textures for each models
			// To get a more linear loading
			loader.fileCount = ground.textures.length + world.models.length * 3;

			// Add water to the list
			if (compiledGround.waterVertCount) {
				loader.fileCount += 32;
			}

			// Loading Gound and Water textures
			loader.loadGroundTextures(world, compiledGround, function onLoaded(waters, textures) {
				world.water.images = waters;
				compiledGround.textures = textures;

				loader.ondata('MAP_WORLD', world.compile());
				loader.ondata('MAP_GROUND', compiledGround);

				// Start loading models
				loader.loadModels(world.models, ground);
			});
		}

		// Start loading World Resource file
		FileManager.load(`data\\${getFilePath(mapname)}`, onWorldReady);
	}

	/**
	 * Loading Ground and Water textures
	 *
	 * @param {object} world resource file
	 * @param {object} compiledGround
	 * @param {function} callback
	 */
	loadGroundTextures(world, ground, callback) {
		let i, count;
		const textures = [];

		// Get water textures
		if (ground.waterVertCount) {
			const path = `data\\texture\\\xbf\xf6\xc5\xcd/water${world.water.type}`;
			for (i = 0; i < 32; ++i) {
				textures.push(`${path}${i < 10 ? '0' + i : i}.jpg`);
			}
		}

		// Load ground textures
		for (i = 0, count = ground.textures.length; i < count; ++i) {
			textures.push(`data\\texture\\${ground.textures[i]}`);
		}

		// Start loading
		const loader = new Loader(textures);

		// On progress
		loader.onprogress = () => {
			this.setProgress(3 + (97 / this.fileCount) * ++this.offset);
		};

		// Once load
		loader.onload = _textures => {
			callback(_textures.splice(0, ground.waterVertCount ? 32 : 0), _textures);
		};

		// Start the queue
		loader.start();
	}

	/**
	 * Loading World Models
	 *
	 * @param {Array} model list
	 * @param {Ground}
	 * @returns {object} compiled mesh
	 */
	loadModels(models, ground) {
		let i, count;
		const files = [];

		// Get a list of files to load
		for (i = 0, count = models.length; i < count; ++i) {
			models[i].filename = `data\\model\\${models[i].filename}`;

			if (files.indexOf(models[i].filename) < 0) {
				files.push(models[i].filename);
			}
		}

		const loader = new Loader(files);

		// Update the progressbar
		loader.onprogress = () => {
			this.setProgress(3 + (97 / this.fileCount) * ++this.offset);
		};

		// Start creating instances
		loader.onload = (objects, filenames) => {
			let pos;

			for (i = 0, count = models.length; i < count; ++i) {
				pos = filenames.indexOf(models[i].filename);

				// Duplicate from a model removed from list
				if (pos === -1) {
					continue;
				}

				// Because of a problem the model isn't load, remove it from the list
				if (!objects[pos]) {
					objects.splice(pos, 1);
					filenames.splice(pos, 1);
					continue;
				}

				objects[pos].filename = filenames[pos];
				objects[pos].createInstance(models[i], ground.width, ground.height);
			}

			this.compileModels(objects);
		};

		// Start loading models
		loader.start();
	}

	/**
	 * Extract model meshes
	 *
	 * @param {Array} objects list
	 */
	compileModels(objects) {
		let i, j, count, size, bufferSize;
		let object, nodes, meshes;
		let index;
		const progress = this.progress;
		const models = [];
		const animatedModels = [];

		bufferSize = 0;

		for (i = 0, count = objects.length; i < count; ++i) {
			// Check if this model has animation - skip static compilation
			if (objects[i].hasAnimation && objects[i].hasAnimation()) {
				animatedModels.push(objects[i]);
				this.setProgress(progress + (((100 - progress) / count) * (i + 1)) / 2);
				continue; // Don't include in static mesh, will be rendered by AnimatedModels
			}

			object = objects[i].compile();
			nodes = object.meshes;

			for (j = 0, size = nodes.length; j < size; ++j) {
				meshes = nodes[j];

				for (index in meshes) {
					models.push({
						texture: `data\\texture\\${object.textures[index]}`,
						alpha: objects[i].alpha,
						mesh: meshes[index]
					});

					bufferSize += meshes[index].length;
				}
			}

			this.setProgress(progress + (((100 - progress) / count) * (i + 1)) / 2);
		}

		// Store animated models for later
		this._animatedModels = animatedModels;

		// Merge mesh
		this.mergeMeshes(models, bufferSize);
	}

	/**
	 * Merge objects using the same texture to avoid drawcall
	 *
	 * @param {Array} objects list
	 * @param {number} BufferSize
	 */
	mergeMeshes(objects, bufferSize) {
		let i, j, count, size, offset;
		let object, texture;
		const textures = [],
			infos = [];

		const fcount = 1 / 9;
		const progress = this.progress;

		// Create buffer where to concat meshes
		const buffer = new Float32Array(bufferSize);
		offset = 0;

		// Sort objects by textures type
		objects.sort(SortMeshByTextures);

		// Merge meshes
		for (i = 0, j = 0, count = objects.length; i < count; ++i) {
			object = objects[i];
			size = object.mesh.length;

			// Same texture, just change vertCount to save drawcall
			// and avoid loading multiple time the same texture.
			if (texture === object.texture) {
				infos[j - 1].vertCount += size * fcount;
			}

			// Load the texture
			else {
				texture = object.texture;
				textures.push(texture);

				infos[j++] = {
					filename: texture,
					vertOffset: offset * fcount,
					vertCount: size * fcount
				};
			}

			// Add to buffer
			buffer.set(object.mesh, offset);
			offset += size;
		}

		// Load texture
		const loader = new Loader(textures);

		// On Progress
		loader.onprogress = (index, _count) => {
			this.setProgress(progress + ((100 - progress) / _count) * (index + 1));
		};

		// Once texture loaded, push the textures
		// in the resulted mesh, and send it back
		loader.onload = (_textures, filenames) => {
			let pos;

			for (i = 0, count = infos.length; i < count; ++i) {
				pos = filenames.indexOf(infos[i].filename);
				infos[i].texture = _textures[pos];
			}

			this.ondata('MAP_MODELS', {
				buffer: buffer,
				infos: infos
			});

			// Send animated models data
			if (this._animatedModels && this._animatedModels.length > 0) {
				this.sendAnimatedModels(this._animatedModels);
			}

			this.onload(true);
		};

		loader.start();
	}

	/**
	 * Send animated model data to main thread
	 *
	 * @param {Array} animatedModels - RSM objects with animation
	 */
	sendAnimatedModels(animatedModels) {
		for (let i = 0; i < animatedModels.length; i++) {
			const model = animatedModels[i];

			// Serialize model data for transfer
			const modelData = {
				filename: model.filename,
				animLen: model.animLen,
				frameRatePerSecond: model.frameRatePerSecond || 30,
				shadeType: model.shadeType,
				alpha: model.alpha,
				textures: model.textures.slice(),
				instances: [],
				nodes: [],
				box: {
					center: Array.from(model.box.center),
					max: Array.from(model.box.max),
					min: Array.from(model.box.min),
					offset: Array.from(model.box.offset),
					range: Array.from(model.box.range)
				}
			};

			// Serialize instances
			for (let j = 0; j < model.instances.length; j++) {
				modelData.instances.push(Array.from(model.instances[j]));
			}

			// Serialize nodes
			for (let k = 0; k < model.nodes.length; k++) {
				const node = model.nodes[k];

				modelData.nodes.push({
					name: node.name,
					parentname: node.parentname,
					is_only: node.is_only,
					textures: node.textures.slice(),
					vertices: node.vertices,
					tvertices: Array.from(node.tvertices),
					faces: node.faces,
					pos: Array.from(node.pos),
					rotangle: node.rotangle,
					rotaxis: Array.from(node.rotaxis),
					scale: Array.from(node.scale),
					offset: Array.from(node.offset),
					mat3: Array.from(node.mat3),
					rotKeyframes: node.rotKeyframes || [],
					posKeyframes: node.posKeyframes || [],
					scaleKeyFrames: node.scaleKeyFrames || []
				});
			}

			this.ondata('MAP_ANIMATED_MODEL', modelData);
		}
	}
}

/**
 * Sort the Object by their textures
 * To avoid some problem in the render, the textures with
 * alpha opacity should be rendered first !
 *
 * @param {Object} a
 * @param {Object} b
 * @return {number}
 */
function SortMeshByTextures(a, b) {
	const reg_tga = /\.tga$/i;

	if (a.texture.match(reg_tga)) {
		return 1;
	}

	if (b.texture.match(reg_tga)) {
		return -1;
	}

	if (a.alpha !== b.alpha) {
		return a.alpha < b.alpha ? 1 : -1;
	}

	if (a.texture < b.texture) {
		return -1;
	}

	if (a.texture > b.texture) {
		return 1;
	}

	return 0;
}

/**
 * Export
 */
export default MapLoader;
