/**
 * Loaders/Ground.js
 *
 * Loaders for Gravity .gnd file (Ground)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import BinaryReader from 'Utils/BinaryReader.js';
import glMatrix from 'Utils/gl-matrix.js';

/**
 * Import
 */
const { vec3, vec4 } = glMatrix;

/**
 * Ground class loader
 *
 * @param {ArrayBuffer} data - optional
 */
class GND {
	constructor(data) {
		if (data) {
			this.load(data);
		}
	}

	/**
	 * Load file
	 *
	 * @param {ArrayBuffer} data
	 */
	load(data) {
		this.fp = new BinaryReader(data);
		const header = this.fp.readBinaryString(4);

		if (header !== 'GRGN') {
			throw new Error(`GND::load() - Invalid header "${header}"`);
		}

		this.version = this.fp.readUByte() + this.fp.readUByte() / 10;
		this.width = this.fp.readULong();
		this.height = this.fp.readULong();
		this.zoom = this.fp.readFloat();

		this.parseTextures();
		this.parseLightmaps();

		this.tiles = this.parseTiles();
		this.surfaces = this.parseSurfaces();

		// Read water-related parameters
		if (this.version >= 1.8) {
			this.water = {
				level: this.fp.readFloat() / 5,
				type: this.fp.readLong(),
				waveHeight: this.fp.readFloat() / 5,
				waveSpeed: this.fp.readFloat(),
				wavePitch: this.fp.readFloat(),
				animSpeed: this.fp.readLong(),
				splitWidth: this.fp.readLong(),
				splitHeight: this.fp.readLong()
			};
			if (this.version >= 1.9) {
				this.water.Zones = [];
				const count = this.water.splitWidth * this.water.splitHeight;

				for (let i = 0; i < count; i++) {
					const waterSub = {
						level: this.fp.readFloat(),
						type: this.fp.readLong(),
						waveHeight: this.fp.readFloat(),
						waveSpeed: this.fp.readFloat(),
						wavePitch: this.fp.readFloat(),
						animSpeed: this.fp.readLong()
					};
					this.water.Zones.push(waterSub);
				}
			}
		}
	}

	/**
	 * Loading textures
	 */
	parseTextures() {
		let pos;
		let texture;

		const count = this.fp.readULong();
		const length = this.fp.readULong();
		const indexes = new Array(count);
		const textures = [];

		for (let i = 0, j = 0; i < count; ++i) {
			texture = this.fp.readBinaryString(length);
			pos = textures.indexOf(texture);

			if (pos === -1) {
				textures[j] = texture;
				pos = j;
				j++;
			}

			indexes[i] = pos;
		}

		this.textures = textures;
		this.textureIndexes = indexes;
	}

	/**
	 * Parse Lightmap
	 *
	 * @return lightmap[][]
	 */
	parseLightmaps() {
		// Load info
		const fp = this.fp;
		const count = fp.readLong();
		const per_cell_x = fp.readLong();
		const per_cell_y = fp.readLong();
		const size_cell = fp.readLong();
		const per_cell = per_cell_x * per_cell_y * size_cell;

		this.lightmap = {
			per_cell,
			count,
			data: new Uint8Array(fp.buffer, fp.offset, count * per_cell * 4)
		};

		fp.seek(count * per_cell * 4, SEEK_CUR);
	}

	/**
	 * Parse Tiles
	 *
	 * @return Tiles[]
	 */
	parseTiles() {
		const fp = this.fp;

		const count = fp.readULong();
		const tiles = new Array(count);

		// Texture atlas stuff
		const ATLAS_COLS = Math.round(Math.sqrt(this.textures.length));
		const ATLAS_ROWS = Math.ceil(Math.sqrt(this.textures.length));
		const ATLAS_WIDTH = Math.pow(2, Math.ceil(Math.log(ATLAS_COLS * 258) / Math.log(2)));
		const ATLAS_HEIGHT = Math.pow(2, Math.ceil(Math.log(ATLAS_ROWS * 258) / Math.log(2)));
		const ATLAS_FACTOR_U = (ATLAS_COLS * 258) / ATLAS_WIDTH;
		const ATLAS_FACTOR_V = (ATLAS_ROWS * 258) / ATLAS_HEIGHT;
		const ATLAS_PX_U = 1 / 258;
		const ATLAS_PX_V = 1 / 258;

		function ATLAS_GENERATE(tile) {
			const u = tile.texture % ATLAS_COLS;
			const v = Math.floor(tile.texture / ATLAS_COLS);
			tile.u1 = ((u + tile.u1 * (1 - ATLAS_PX_U * 2) + ATLAS_PX_U) * ATLAS_FACTOR_U) / ATLAS_COLS;
			tile.u2 = ((u + tile.u2 * (1 - ATLAS_PX_U * 2) + ATLAS_PX_U) * ATLAS_FACTOR_U) / ATLAS_COLS;
			tile.u3 = ((u + tile.u3 * (1 - ATLAS_PX_U * 2) + ATLAS_PX_U) * ATLAS_FACTOR_U) / ATLAS_COLS;
			tile.u4 = ((u + tile.u4 * (1 - ATLAS_PX_U * 2) + ATLAS_PX_U) * ATLAS_FACTOR_U) / ATLAS_COLS;
			tile.v1 = ((v + tile.v1 * (1 - ATLAS_PX_V * 2) + ATLAS_PX_V) * ATLAS_FACTOR_V) / ATLAS_ROWS;
			tile.v2 = ((v + tile.v2 * (1 - ATLAS_PX_V * 2) + ATLAS_PX_V) * ATLAS_FACTOR_V) / ATLAS_ROWS;
			tile.v3 = ((v + tile.v3 * (1 - ATLAS_PX_V * 2) + ATLAS_PX_V) * ATLAS_FACTOR_V) / ATLAS_ROWS;
			tile.v4 = ((v + tile.v4 * (1 - ATLAS_PX_V * 2) + ATLAS_PX_V) * ATLAS_FACTOR_V) / ATLAS_ROWS;
		}

		// Read Tiles
		for (let i = 0; i < count; ++i) {
			tiles[i] = {
				u1: fp.readFloat(),
				u2: fp.readFloat(),
				u3: fp.readFloat(),
				u4: fp.readFloat(),
				v1: fp.readFloat(),
				v2: fp.readFloat(),
				v3: fp.readFloat(),
				v4: fp.readFloat(),
				texture: fp.readUShort(),
				light: fp.readUShort(),
				color: [fp.readUByte(), fp.readUByte(), fp.readUByte(), fp.readUByte()]
			};

			// Generate texture atlas only if having texture on the cell.
			tiles[i].texture = this.textureIndexes[tiles[i].texture];
			ATLAS_GENERATE(tiles[i]);
		}

		return tiles;
	}

	/**
	 * Parse GND surfaces
	 *
	 * @return Surfaces[]
	 */
	parseSurfaces() {
		const fp = this.fp;

		const count = this.width * this.height;
		const surfaces = new Array(count);

		for (let i = 0; i < count; ++i) {
			surfaces[i] = {
				height: [fp.readFloat() / 5, fp.readFloat() / 5, fp.readFloat() / 5, fp.readFloat() / 5],
				tile_up: fp.readLong(),
				tile_front: fp.readLong(),
				tile_right: fp.readLong()
			};
		}

		return surfaces;
	}

	/**
	 * Create a large image_data array with all lightmaps
	 *
	 * @return {Uint8Array} pixels
	 */
	createLightmapImage() {
		let _x, _y, idx;

		const lightmap = this.lightmap;
		const count = lightmap.count;
		const data = lightmap.data;
		const per_cell = lightmap.per_cell;

		const width = Math.round(Math.sqrt(count));
		const height = Math.ceil(Math.sqrt(count));
		const _width = Math.pow(2, Math.ceil(Math.log(width * 8) / Math.log(2)));
		const _height = Math.pow(2, Math.ceil(Math.log(height * 8) / Math.log(2)));

		const out = new Uint8Array(_width * _height * 4);

		for (let i = 0; i < count; ++i) {
			const pos = i * 4 * per_cell;
			const x = (i % width) * 8;
			const y = ((i / width) | 0) * 8;

			for (_x = 0; _x < 8; ++_x) {
				for (_y = 0; _y < 8; ++_y) {
					idx = (x + _x + (y + _y) * _width) * 4;
					out[idx + 0] = data[pos + per_cell + (_x + _y * 8) * 3 + 0];
					out[idx + 1] = data[pos + per_cell + (_x + _y * 8) * 3 + 1];
					out[idx + 2] = data[pos + per_cell + (_x + _y * 8) * 3 + 2];
					out[idx + 3] = data[pos + (_x + _y * 8)];
				}
			}
		}

		return out;
	}

	/**
	 * Create a large image_data array with all tiles color
	 *
	 * @return {Uint8Array} pixels
	 */
	createTilesColorImage() {
		const width = this.width;
		const height = this.height;
		const surfaces = this.surfaces;
		const tiles = this.tiles;
		const data = new Uint8Array(width * height * 4);

		for (let y = 0; y < height; ++y) {
			for (let x = 0; x < width; ++x) {
				const cell = surfaces[x + y * width];

				// Check tile up
				if (cell.tile_up > -1) {
					// Colors are stored in BGRA format, convert to RGBA for WebGL
					const [b, g, r, a] = tiles[cell.tile_up].color;
					data.set([r, g, b, a], (x + y * width) * 4);
				}
			}
		}

		return data;
	}

	/**
	 * Create ShadowMap data (only used to render shadow on Entities)
	 */
	createShadowmapData() {
		let i, j, index;

		const width = this.width;
		const height = this.height;
		const out = new Uint8Array(width * 8 * (height * 8));
		const data = this.lightmap.data;
		const per_cell = this.lightmap.per_cell;
		const tiles = this.tiles;
		const surfaces = this.surfaces;

		for (let y = 0; y < height; ++y) {
			for (let x = 0; x < width; ++x) {
				const cell = surfaces[x + y * width];

				if (cell.tile_up > -1) {
					index = tiles[cell.tile_up].light * 4 * per_cell;

					for (i = 0; i < 8; ++i) {
						for (j = 0; j < 8; ++j) {
							out[x * 8 + i + (y * 8 + j) * (width * 8)] = data[index + i + j * 8];
						}
					}
				}

				// If no ground, shadow should be 1.0
				else {
					for (i = 0; i < 8; ++i) {
						for (j = 0; j < 8; ++j) {
							out[x * 8 + i + (y * 8 + j) * (width * 8)] = 255;
						}
					}
				}
			}
		}

		return out;
	}

	/**
	 * Smooth ground normals
	 *
	 * @return normals[]
	 */
	getSmoothNormal() {
		let x, y, n;
		const surfaces = this.surfaces;
		const width = this.width;
		const height = this.height;
		const a = vec3.create();
		const b = vec3.create();
		const c = vec3.create();
		const d = vec3.create();

		const count = width * height;
		const tmp = new Array(count);
		const normals = new Array(count);
		const empty_vec = vec3.create();
		let cell;

		// Calculate normal for each cells
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				tmp[x + y * width] = vec3.create();
				cell = surfaces[x + y * width];

				// Tile Up
				if (cell.tile_up > -1) {
					a[0] = (x + 0) * 2;
					a[1] = cell.height[0];
					a[2] = (y + 0) * 2;
					b[0] = (x + 1) * 2;
					b[1] = cell.height[1];
					b[2] = (y + 0) * 2;
					c[0] = (x + 1) * 2;
					c[1] = cell.height[3];
					c[2] = (y + 1) * 2;
					d[0] = (x + 0) * 2;
					d[1] = cell.height[2];
					d[2] = (y + 1) * 2;

					vec4.calcNormal(a, b, c, d, tmp[x + y * width]);
				}
			}
		}

		// Smooth normals
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				n = normals[x + y * width] = [
					[0, 0, 0],
					[0, 0, 0],
					[0, 0, 0],
					[0, 0, 0]
				];

				// Up Left
				vec3.add(n[0], n[0], tmp[x + 0 + (y + 0) * width]);
				vec3.add(n[0], n[0], tmp[x - 1 + (y + 0) * width] || empty_vec);
				vec3.add(n[0], n[0], tmp[x - 1 + (y - 1) * width] || empty_vec);
				vec3.add(n[0], n[0], tmp[x + 0 + (y - 1) * width] || empty_vec);
				vec3.normalize(n[0], n[0]);

				// Up Right
				vec3.add(n[1], n[1], tmp[x + 0 + (y + 0) * width]);
				vec3.add(n[1], n[1], tmp[x + 1 + (y + 0) * width] || empty_vec);
				vec3.add(n[1], n[1], tmp[x + 1 + (y - 1) * width] || empty_vec);
				vec3.add(n[1], n[1], tmp[x + 0 + (y - 1) * width] || empty_vec);
				vec3.normalize(n[1], n[1]);

				// Bottom Right
				vec3.add(n[2], n[2], tmp[x + 0 + (y + 0) * width]);
				vec3.add(n[2], n[2], tmp[x + 1 + (y + 0) * width] || empty_vec);
				vec3.add(n[2], n[2], tmp[x + 1 + (y + 1) * width] || empty_vec);
				vec3.add(n[2], n[2], tmp[x + 0 + (y + 1) * width] || empty_vec);
				vec3.normalize(n[2], n[2]);

				// Bottom Left
				vec3.add(n[3], n[3], tmp[x + 0 + (y + 0) * width]);
				vec3.add(n[3], n[3], tmp[x - 1 + (y + 0) * width] || empty_vec);
				vec3.add(n[3], n[3], tmp[x - 1 + (y + 1) * width] || empty_vec);
				vec3.add(n[3], n[3], tmp[x + 0 + (y + 1) * width] || empty_vec);
				vec3.normalize(n[3], n[3]);
			}
		}

		return normals;
	}

	/**
	 * Compile GND file
	 *
	 * @param {number} WATER_LEVEL
	 * @param {number} WATER_HEIGHT
	 * @return object export
	 */
	compile(WATER_LEVEL, WATER_HEIGHT) {
		// Shortcut access
		const width = this.width,
			height = this.height;
		const surfaces = this.surfaces,
			tiles = this.tiles;

		// Normals
		const normals = this.getSmoothNormal();

		// Pre-defined vars
		let tile, cell_a, cell_b, n, h_a, h_b;
		let x, y;

		// Water
		const water = [],
			mesh = [];

		// Lightmap Stuff
		const l = {};
		const lightmap = this.createLightmapImage();
		const l_count_w = Math.round(Math.sqrt(this.lightmap.count));
		const l_count_h = Math.ceil(Math.sqrt(this.lightmap.count));
		const l_width = Math.pow(2, Math.ceil(Math.log(l_count_w * 8) / Math.log(2)));
		const l_height = Math.pow(2, Math.ceil(Math.log(l_count_h * 8) / Math.log(2)));
		function lightmap_atlas(i) {
			l.u1 = (((i % l_count_w) + 0.125) / l_count_w) * ((l_count_w * 8) / l_width);
			l.u2 = (((i % l_count_w) + 0.875) / l_count_w) * ((l_count_w * 8) / l_width);
			l.v1 = ((((i / l_count_w) | 0) + 0.125) / l_count_h) * ((l_count_h * 8) / l_height);
			l.v2 = ((((i / l_count_w) | 0) + 0.875) / l_count_h) * ((l_count_h * 8) / l_height);
		}

		// Compiling mesh
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				cell_a = surfaces[x + y * width];
				h_a = cell_a.height;

				// Check tile up
				if (cell_a.tile_up > -1) {
					tile = tiles[cell_a.tile_up];

					// Check if has texture
					n = normals[x + y * width];
					lightmap_atlas(tile.light);

					mesh.push(
						//      vec3 pos           |        vec3 normals          |     vec2 texcoords     |  vec2 lightcoord  |      vec2 tileCoords
						(x + 0) * 2,
						h_a[0],
						(y + 0) * 2,
						n[0][0],
						n[0][1],
						n[0][2],
						tile.u1,
						tile.v1,
						l.u1,
						l.v1,
						(x + 0.5) / width,
						(y + 0.5) / height,
						(x + 1) * 2,
						h_a[1],
						(y + 0) * 2,
						n[1][0],
						n[1][1],
						n[1][2],
						tile.u2,
						tile.v2,
						l.u2,
						l.v1,
						(x + 1.5) / width,
						(y + 0.5) / height,
						(x + 1) * 2,
						h_a[3],
						(y + 1) * 2,
						n[2][0],
						n[2][1],
						n[2][2],
						tile.u4,
						tile.v4,
						l.u2,
						l.v2,
						(x + 1.5) / width,
						(y + 1.5) / height,
						(x + 1) * 2,
						h_a[3],
						(y + 1) * 2,
						n[2][0],
						n[2][1],
						n[2][2],
						tile.u4,
						tile.v4,
						l.u2,
						l.v2,
						(x + 1.5) / width,
						(y + 1.5) / height,
						(x + 0) * 2,
						h_a[2],
						(y + 1) * 2,
						n[3][0],
						n[3][1],
						n[3][2],
						tile.u3,
						tile.v3,
						l.u1,
						l.v2,
						(x + 0.5) / width,
						(y + 1.5) / height,
						(x + 0) * 2,
						h_a[0],
						(y + 0) * 2,
						n[0][0],
						n[0][1],
						n[0][2],
						tile.u1,
						tile.v1,
						l.u1,
						l.v1,
						(x + 0.5) / width,
						(y + 0.5) / height
					);

					// Add water only if it's upper than the ground.
					if (
						h_a[0] > WATER_LEVEL - WATER_HEIGHT ||
						h_a[1] > WATER_LEVEL - WATER_HEIGHT ||
						h_a[2] > WATER_LEVEL - WATER_HEIGHT ||
						h_a[3] > WATER_LEVEL - WATER_HEIGHT
					) {
						water.push(
							//        vec3 pos            |            vec2 texcoords
							(x + 0) * 2,
							WATER_LEVEL,
							(y + 0) * 2,
							((x + 0) % 5) / 5,
							((y + 0) % 5) / 5,
							(x + 1) * 2,
							WATER_LEVEL,
							(y + 0) * 2,
							((x + 1) % 5) / 5 || 1,
							((y + 0) % 5) / 5,
							(x + 1) * 2,
							WATER_LEVEL,
							(y + 1) * 2,
							((x + 1) % 5) / 5 || 1,
							((y + 1) % 5) / 5 || 1,
							(x + 1) * 2,
							WATER_LEVEL,
							(y + 1) * 2,
							((x + 1) % 5) / 5 || 1,
							((y + 1) % 5) / 5 || 1,
							(x + 0) * 2,
							WATER_LEVEL,
							(y + 1) * 2,
							((x + 0) % 5) / 5,
							((y + 1) % 5) / 5 || 1,
							(x + 0) * 2,
							WATER_LEVEL,
							(y + 0) * 2,
							((x + 0) % 5) / 5,
							((y + 0) % 5) / 5
						);
					}
				}

				// Check tile front
				if (cell_a.tile_front > -1 && y + 1 < height) {
					tile = tiles[cell_a.tile_front];

					cell_b = surfaces[x + (y + 1) * width];
					h_b = cell_b.height;
					lightmap_atlas(tile.light);

					mesh.push(
						//      vec3 pos           |  vec3 normals     |    vec2 texcoords      |   vec2 lightcoord  |   vec2 tileCoords
						(x + 0) * 2,
						h_b[0],
						(y + 1) * 2,
						0.0,
						0.0,
						1.0,
						tile.u3,
						tile.v3,
						l.u1,
						l.v2,
						0,
						0,
						(x + 1) * 2,
						h_a[3],
						(y + 1) * 2,
						0.0,
						0.0,
						1.0,
						tile.u2,
						tile.v2,
						l.u2,
						l.v1,
						0,
						0,
						(x + 1) * 2,
						h_b[1],
						(y + 1) * 2,
						0.0,
						0.0,
						1.0,
						tile.u4,
						tile.v4,
						l.u2,
						l.v2,
						0,
						0,
						(x + 0) * 2,
						h_b[0],
						(y + 1) * 2,
						0.0,
						0.0,
						1.0,
						tile.u3,
						tile.v3,
						l.u1,
						l.v2,
						0,
						0,
						(x + 1) * 2,
						h_a[3],
						(y + 1) * 2,
						0.0,
						0.0,
						1.0,
						tile.u2,
						tile.v2,
						l.u2,
						l.v1,
						0,
						0,
						(x + 0) * 2,
						h_a[2],
						(y + 1) * 2,
						0.0,
						0.0,
						1.0,
						tile.u1,
						tile.v1,
						l.u1,
						l.v1,
						0,
						0
					);
				}

				// Check tile right
				if (cell_a.tile_right > -1 && x + 1 < width) {
					tile = tiles[cell_a.tile_right];

					cell_b = surfaces[x + 1 + y * width];
					h_b = cell_b.height;
					lightmap_atlas(tile.light);

					mesh.push(
						//      vec3 pos           |  vec3 normals    |    vec2 texcoords      |   vec2 lightcoord   |    vec2 tileCoords
						(x + 1) * 2,
						h_a[1],
						(y + 0) * 2,
						1.0,
						0.0,
						0.0,
						tile.u2,
						tile.v2,
						l.u2,
						l.v1,
						0,
						0,
						(x + 1) * 2,
						h_a[3],
						(y + 1) * 2,
						1.0,
						0.0,
						0.0,
						tile.u1,
						tile.v1,
						l.u1,
						l.v1,
						0,
						0,
						(x + 1) * 2,
						h_b[0],
						(y + 0) * 2,
						1.0,
						0.0,
						0.0,
						tile.u4,
						tile.v4,
						l.u2,
						l.v2,
						0,
						0,
						(x + 1) * 2,
						h_b[0],
						(y + 0) * 2,
						1.0,
						0.0,
						0.0,
						tile.u4,
						tile.v4,
						l.u2,
						l.v2,
						0,
						0,
						(x + 1) * 2,
						h_b[2],
						(y + 1) * 2,
						1.0,
						0.0,
						0.0,
						tile.u3,
						tile.v3,
						l.u1,
						l.v2,
						0,
						0,
						(x + 1) * 2,
						h_a[3],
						(y + 1) * 2,
						1.0,
						0.0,
						0.0,
						tile.u1,
						tile.v1,
						l.u1,
						l.v1,
						0,
						0
					);
				}
			}
		}

		// Return mesh informations
		return {
			width: this.width,
			height: this.height,
			textures: this.textures,

			lightmap: lightmap,
			lightmapSize: this.lightmap.count,
			tileColor: this.createTilesColorImage(),
			shadowMap: this.createShadowmapData(),

			mesh: new Float32Array(mesh),
			meshVertCount: mesh.length / 12,

			waterMesh: new Float32Array(water),
			waterVertCount: water.length / 5
		};
	}
}
/**
 * Export
 */
export default GND;
