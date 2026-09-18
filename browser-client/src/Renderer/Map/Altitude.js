/**
 * Renderer/Map/Altitude.js
 *
 * Rendering altitude (used for color picking)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Vendors/gl-matrix.js';
import PathFinding from 'Utils/PathFinding.js';
import Mouse from 'Controls/MouseEventHandler.js';
import VerticalFlip from 'Renderer/Effects/Shaders/VerticalFlip.js';

/**
 * @var {array} Cells where stored altitude
 */
let _cells = null;

/**
 * @var {array} Altitudes cells type
 */
let _types = null;

const { mat4, vec3, vec4 } = glMatrix;
const _from = vec3.create();
const _to = vec4.create();
const _unit = vec3.create();
const _matrix = mat4.create();
const buffer1x1 = new Float32Array(1 * 1 * 30);
const buffer5x5 = new Float32Array(5 * 5 * 30);
const buffer7x7 = new Float32Array(7 * 7 * 30);
const buffer13x13 = new Float32Array(13 * 13 * 30);
const tmp = new Float32Array(5);

/**
 * Altitude Namespace
 */
class Altitude {
	/**
	 * Initialize Altitude mesh
	 *
	 * @param {object} gl context
	 * @param {object} data Altitude { mesh, vertCount, cells, width, height, colors }
	 */
	static init(data) {
		// Extract 'type' from cells
		let i;
		const count = data.cells.length / 5;
		const types = new Uint8Array(count);

		for (i = 0; i < count; ++i) {
			types[i] = data.cells[i * 5 + 4];
		}

		// Save information
		_cells = data.cells;
		_types = types;
		Altitude.width = data.width;
		Altitude.height = data.height;

		// Initialize PathFinding
		PathFinding.setGat({
			width: Altitude.width,
			height: Altitude.height,
			cells: types,
			types: Altitude.TYPE
		});
	}

	/**
	 * Get back cell data
	 *
	 * @param {number} x
	 * @param {number} y
	 * @return {Array} cell
	 */
	static getCell(x, y) {
		const cx = Math.floor(x);
		const cy = Math.floor(y);

		// Outside the map the index either runs past the array (undefined, which
		// becomes NaN once written into a Float32Array vertex buffer) or, for a
		// negative x with a positive y, wraps into a valid but wrong cell. Both
		// produce garbage geometry. See intersect() for how we get here.
		if (!_cells || cx < 0 || cy < 0 || cx >= Altitude.width || cy >= Altitude.height) {
			tmp[0] = tmp[1] = tmp[2] = tmp[3] = tmp[4] = 0.0;
			return tmp;
		}

		const index = (cx + cy * Altitude.width) * 5;

		tmp[0] = _cells[index + 0];
		tmp[1] = _cells[index + 1];
		tmp[2] = _cells[index + 2];
		tmp[3] = _cells[index + 3];
		tmp[4] = _cells[index + 4];

		return tmp;
	}

	/**
	 * Return cell type
	 *
	 * @param {number} x
	 * @param {number} y
	 * @return {number} cell type
	 */
	static getCellType(x, y) {
		if (!_types || x < 0 || y < 0 || x >= Altitude.width || y >= Altitude.height) {
			return Altitude.TYPE.NONE;
		}

		return _types[x + y * Altitude.width];
	}

	/**
	 * Return cell height
	 *
	 * @param {number} x
	 * @param {number} y
	 * @return {number} height
	 */
	static getCellHeight(x, y) {
		// Map not loaded yet ?
		if (!_cells) {
			return 0.0;
		}

		// Should be at the middle of the cell
		x += 0.5;
		y += 0.5;

		const cx = Math.floor(x);
		const cy = Math.floor(y);

		// Unbounded, this read NaN or a wrapped cell outside the map
		if (cx < 0 || cy < 0 || cx >= Altitude.width || cy >= Altitude.height) {
			return 0.0;
		}

		const index = (cx + cy * Altitude.width) * 5;

		x %= 1.0;
		y %= 1.0;

		const x1 = _cells[index + 0] + (_cells[index + 1] - _cells[index + 0]) * x;
		const x2 = _cells[index + 2] + (_cells[index + 3] - _cells[index + 2]) * x;

		return -(x1 + (x2 - x1) * y);
	}

	static setCellType(x, y, type) {
		_types[x + y * Altitude.width] = TYPE_TABLE[type];
		PathFinding.updateGat(x, y, TYPE_TABLE[type]);
	}

	/**
	 * Intersect cell
	 *
	 * @param {mat4} modelView matrix
	 * @param {mat4} projection matrix
	 * @param {vec2} output vector
	 * @return {bool} success
	 */
	static intersect(modelView, projection, out) {
		let i;
		const count = Altitude.MAX_INTERSECT_COUNT;

		// Extract camera position
		mat4.invert(_matrix, modelView);
		_from[0] = _matrix[12];
		_from[1] = _matrix[13];
		_from[2] = _matrix[14];

		// set two vectors with opposing z values
		_to[0] = (Mouse.screen.x / Mouse.screen.width) * 2 - 1;
		_to[1] = -(Mouse.screen.y / Mouse.screen.height) * 2 + 1;

		// Check if the Vertical Flip (Illusion effect) is active
		// If true, invert the Y coordinate relative to the renderer height
		if (VerticalFlip.isActive()) {
			_to[1] = -_to[1];
		}

		_to[2] = 1.0;
		_to[3] = 1.0;

		// Unproject
		mat4.multiply(_matrix, projection, modelView);
		mat4.invert(_matrix, _matrix);
		vec4.transformMat4(_to, _to, _matrix);

		_to[0] /= _to[3];
		_to[1] /= _to[3];
		_to[2] /= _to[3];

		// Extract direction
		vec3.sub(_unit, _to, _from);
		vec3.normalize(_unit, _unit);

		// Search
		for (i = 0; i < count; ++i) {
			_from[0] += _unit[0];
			_from[1] += _unit[1];
			_from[2] += _unit[2];

			// The ray leaves the map on any map whose ground does not fill the
			// view - airplane_01, the Valkyrie realm, anything floating in sky.
			// Without this the march kept sampling cells outside the map and
			// could report a "hit" there, which put a bogus cell into
			// Mouse.world (walk packets read it) and drew the grid selector on
			// garbage geometry.
			if (_from[0] < 0 || _from[2] < 0 || _from[0] >= Altitude.width || _from[2] >= Altitude.height) {
				continue;
			}

			if (Math.abs(Altitude.getCellHeight(_from[0], _from[2]) + _from[1]) < 0.5) {
				out[0] = _from[0];
				out[1] = _from[2];
				return true;
			}
		}

		return false;
	}

	/**
	 * Generate a plane stick to the ground
	 * Used for effects
	 *
	 * @param {number} position x
	 * @param {number} position y
	 * @param {number} plane size
	 */
	static generatePlane(pos_x, pos_y, size) {
		if (!_cells) {
			return null;
		}

		let x, y, index, i;
		let buffer;
		const middle = Math.floor(size / 2);

		pos_x = Math.floor(pos_x);
		pos_y = Math.floor(pos_y);
		i = 0;

		// Avoid memory allocation
		switch (size) {
			case 1:
				buffer = buffer1x1;
				break;
			case 5:
				buffer = buffer5x5;
				break;
			case 7:
				buffer = buffer7x7;
				break;
			case 13:
				buffer = buffer13x13;
				break;
			default:
				buffer = new Float32Array(size * size * 30);
				break;
		}

		for (x = -middle; x <= middle; ++x) {
			for (y = -middle; y <= middle; ++y, i += 30) {
				const gx = pos_x + x;
				const gy = pos_y + y;
				const oob = gx < 0 || gy < 0 || gx >= Altitude.width || gy >= Altitude.height;

				// Effects near a map edge overlap cells that do not exist; an
				// unguarded read puts undefined -> NaN into the vertex buffer.
				index = oob ? -1 : (gx + gy * Altitude.width) * 5;

				const h0 = oob ? 0.0 : _cells[index + 0];
				const h1 = oob ? 0.0 : _cells[index + 1];
				const h2 = oob ? 0.0 : _cells[index + 2];
				const h3 = oob ? 0.0 : _cells[index + 3];

				// Triangle 1
				buffer[i + 0] = pos_x + x + 0;
				buffer[i + 1] = h0;
				buffer[i + 2] = pos_y + y + 0;
				buffer[i + 3] = (x + 0 + middle) / size;
				buffer[i + 4] = (y + 0 + middle) / size;

				buffer[i + 5] = pos_x + x + 1;
				buffer[i + 6] = h1;
				buffer[i + 7] = pos_y + y + 0;
				buffer[i + 8] = (x + 1 + middle) / size;
				buffer[i + 9] = (y + 0 + middle) / size;

				buffer[i + 10] = pos_x + x + 1;
				buffer[i + 11] = h3;
				buffer[i + 12] = pos_y + y + 1;
				buffer[i + 13] = (x + 1 + middle) / size;
				buffer[i + 14] = (y + 1 + middle) / size;

				// Triangle 2
				buffer[i + 15] = pos_x + x + 1;
				buffer[i + 16] = h3;
				buffer[i + 17] = pos_y + y + 1;
				buffer[i + 18] = (x + 1 + middle) / size;
				buffer[i + 19] = (y + 1 + middle) / size;

				buffer[i + 20] = pos_x + x + 0;
				buffer[i + 21] = h2;
				buffer[i + 22] = pos_y + y + 1;
				buffer[i + 23] = (x + 0 + middle) / size;
				buffer[i + 24] = (y + 1 + middle) / size;

				buffer[i + 25] = pos_x + x + 0;
				buffer[i + 26] = h0;
				buffer[i + 27] = pos_y + y + 0;
				buffer[i + 28] = (x + 0 + middle) / size;
				buffer[i + 29] = (y + 0 + middle) / size;
			}
		}

		return buffer;
	}

	static getCellsInSquareRange(x, y, range) {
		const result = [];

		// get all cells in square range and return x, y if is in  and walkable
		for (let i = x - range; i <= x + range; ++i) {
			for (let j = y - range; j <= y + range; ++j) {
				if (Altitude.getCellType(i, j) & Altitude.TYPE.WALKABLE) {
					result.push({ x: i, y: j });
				}
			}
		}

		return result;
	}
}

/**
 * @var {number} map width
 */
Altitude.width = 0;

/**
 * @var {number} map height
 */
Altitude.height = 0;

/**
 * @var {object} enum cell type
 * (Copy from Loaders/Altitude.js)
 */
Altitude.TYPE = {
	NONE: 1 << 0,
	WALKABLE: 1 << 1,
	WATER: 1 << 2,
	SNIPABLE: 1 << 3
};

/**
 * @var {number} ray intersection count
 */
Altitude.MAX_INTERSECT_COUNT = 200;

/**
 * Taken from *athena at src/map/map.c
 * I don't know if it's a good source but it's a good idea to match this references for now
 */
const TYPE_TABLE = {
	0: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE, // walkable ground
	1: Altitude.TYPE.NONE, // non-walkable ground
	2: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE, // ???
	3: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE | Altitude.TYPE.WATER, // walkable water
	4: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE, // ???
	5: Altitude.TYPE.SNIPABLE, // gat (snipable)
	6: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE, // ???

	/* Taken from Grf Editor */
	[-1]: Altitude.TYPE.NONE, // NoGat (-1)
	[0x80000000]: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE, // Weird0 (Int32.MinValue)
	[0x80000001]: Altitude.TYPE.NONE, // Weird1
	[0x80000002]: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE, // Weird2
	[0x80000003]: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE | Altitude.TYPE.WATER, // Weird3
	[0x80000004]: Altitude.TYPE.NONE, // Weird4
	[0x80000005]: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE, // Weird5
	[0x80000006]: Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE, // Weird6
	[0x80000007]: Altitude.TYPE.NONE, // Weird7
	[0x80000008]: Altitude.TYPE.NONE, // Weird8
	[0x80000009]: Altitude.TYPE.NONE // Weird9
};

/**
 * Export
 */
export default Altitude;
