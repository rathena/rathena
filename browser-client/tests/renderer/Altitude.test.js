import { beforeEach, describe, expect, it, vi } from 'vitest';

vi.mock('Utils/PathFinding.js', () => ({ default: { setGat: vi.fn(), updateGat: vi.fn() } }));
vi.mock('Controls/MouseEventHandler.js', () => ({ default: { screen: { x: 0, y: 0, width: 800, height: 600 } } }));
vi.mock('Renderer/Effects/Shaders/VerticalFlip.js', () => ({ default: { isActive: () => false } }));

const { default: Altitude } = await import('Renderer/Map/Altitude.js');

const WIDTH = 4;
const HEIGHT = 4;

/**
 * Build a synthetic altitude grid.
 *
 * Cells are 5 floats: four corner heights then the type. Loaders/Altitude.js
 * stores an already-mapped TYPE bitmask in that last slot, not a raw GAT code,
 * so seed it the same way. Cell n gets height -(n + 1), which makes every cell
 * distinguishable and lets a wrapped read be identified by the value it returns.
 *
 * @param {number} width
 * @param {number} height
 * @return {Float32Array} cells
 */
function buildCells(width, height) {
	const cells = new Float32Array(width * height * 5);
	const walkable = Altitude.TYPE.WALKABLE | Altitude.TYPE.SNIPABLE;

	for (let i = 0; i < width * height; ++i) {
		cells[i * 5 + 0] = cells[i * 5 + 1] = cells[i * 5 + 2] = cells[i * 5 + 3] = -(i + 1);
		cells[i * 5 + 4] = walkable;
	}

	return cells;
}

describe('Renderer/Map/Altitude bounds handling', () => {
	beforeEach(() => {
		Altitude.init({ cells: buildCells(WIDTH, HEIGHT), width: WIDTH, height: HEIGHT });
	});

	describe('in-bounds lookups are unaffected', () => {
		it('returns interpolated cell heights', () => {
			expect(Altitude.getCellHeight(0, 0)).toBe(1);
			expect(Altitude.getCellHeight(3, 3)).toBe(16);
		});

		it('reports walkable cells as walkable', () => {
			expect(Altitude.getCellType(0, 0) & Altitude.TYPE.WALKABLE).not.toBe(0);
		});

		it('returns the raw corner heights', () => {
			expect(Altitude.getCell(2, 1)[0]).toBe(-7);
		});

		it('builds a plane from the real cell heights', () => {
			const plane = Altitude.generatePlane(2, 2, 1);

			expect(plane[1]).toBe(-11);
			expect(plane[6]).toBe(-11);
		});
	});

	describe('out-of-bounds lookups', () => {
		// A negative x with a positive y lands on a valid index belonging to a
		// different cell: (-1, 2) computes (-1 + 2 * 4) * 5 = 35, which is cell
		// (3, 1). Unguarded, that reports a plausible height for a position that
		// is not on the map at all.
		it('does not wrap a negative x into a neighbouring row', () => {
			expect(Altitude.getCellHeight(-1, 2)).toBe(0);
			expect(Altitude.getCellType(-1, 2) & Altitude.TYPE.WALKABLE).toBe(0);
		});

		it.each([
			[99, 0],
			[0, 99],
			[-5, -5],
			[WIDTH, 0],
			[0, HEIGHT]
		])('returns a height of 0 rather than NaN at (%i, %i)', (x, y) => {
			const height = Altitude.getCellHeight(x, y);

			expect(Number.isNaN(height)).toBe(false);
			expect(height).toBe(0);
		});

		it('does not report an off-map cell as walkable', () => {
			expect(Altitude.getCellType(99, 99) & Altitude.TYPE.WALKABLE).toBe(0);
		});

		it('returns a zeroed cell instead of undefined values', () => {
			expect(Array.from(Altitude.getCell(-3, 99))).toEqual([0, 0, 0, 0, 0]);
		});
	});

	describe('generatePlane near the map edge', () => {
		// Reads past the array yield undefined, which becomes NaN once written
		// into the Float32Array vertex buffer, and a NaN vertex rasterises as
		// undefined geometry.
		it.each([
			['a corner', 0, 0, 5],
			['past the far edge', 3, 3, 7]
		])('emits no NaN vertices at %s', (_label, x, y, size) => {
			const plane = Altitude.generatePlane(x, y, size);

			expect(plane).not.toBeNull();
			expect(Array.from(plane).some(Number.isNaN)).toBe(false);
		});
	});
});
