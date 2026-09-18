import { beforeEach, describe, expect, it, vi } from 'vitest';

const mocks = vi.hoisted(() => ({
	heights: new Map(),
	altitude: {
		getCellHeight: vi.fn((x, y) => mocks.heights.get(`${x},${y}`) ?? 0)
	},
	webgl: {
		createShaderProgram: vi.fn(() => ({})),
		texture: vi.fn()
	}
}));

vi.mock('Utils/WebGL.js', () => ({ default: mocks.webgl }));
vi.mock('Renderer/SpriteRenderer.js', () => ({ default: {} }));
vi.mock('Renderer/Map/Altitude.js', () => ({ default: mocks.altitude }));
vi.mock('./Water.vs?raw', () => ({ default: '' }));
vi.mock('./Water.fs?raw', () => ({ default: '' }));

import Water from 'Renderer/Map/Water.js';

const gl = {
	ARRAY_BUFFER: 0,
	STATIC_DRAW: 0,
	createBuffer: vi.fn(() => ({})),
	bindBuffer: vi.fn(),
	bufferData: vi.fn(),
	deleteBuffer: vi.fn(),
	deleteProgram: vi.fn(),
	deleteTexture: vi.fn()
};

function waterData(overrides = {}) {
	return {
		vertCount: 6,
		waveHeight: 1,
		waveSpeed: 2,
		level: 10,
		animSpeed: 3,
		wavePitch: 50,
		type: 0,
		mesh: new Float32Array(0),
		images: [],
		...overrides
	};
}

// Altitude.getCellHeight() is up-positive (negated gat height): deeper ground is more negative,
// so a cell is submerged when its height is below -(level - waveHeight).
function setCellHeight(x, y, height) {
	mocks.heights.set(`${x},${y}`, height);
}

describe('Water.isSubmerged', () => {
	beforeEach(() => {
		mocks.heights.clear();
		Water.free(gl);
	});

	it('is false everywhere on a dry map', () => {
		Water.init(gl, waterData({ vertCount: 0 }));
		setCellHeight(1, 1, 100);

		expect(Water.hasWater()).toBe(false);
		expect(Water.isSubmerged(1, 1)).toBe(false);
	});

	it('uses the wave crest (level - waveHeight) as threshold', () => {
		Water.init(gl, waterData({ level: 10, waveHeight: 1 }));

		setCellHeight(0, 0, -9); // exactly at the crest
		setCellHeight(1, 0, -8.9); // just above the crest
		setCellHeight(2, 0, -9.1); // just below the crest

		expect(Water.hasWater()).toBe(true);
		expect(Water.isSubmerged(0, 0)).toBe(false);
		expect(Water.isSubmerged(1, 0)).toBe(false);
		expect(Water.isSubmerged(2, 0)).toBe(true);
	});

	it('follows the ground on a slope running into the water', () => {
		Water.init(gl, waterData({ level: 1, waveHeight: 0.5 })); // crest at height -0.5

		for (let x = 0; x < 6; ++x) {
			setCellHeight(x, 0, 2 - x); // 2 (dry shore) .. -3 (deep)
		}

		expect([0, 1, 2, 3, 4, 5].map(x => Water.isSubmerged(x, 0))).toEqual([
			false,
			false,
			false,
			true,
			true,
			true
		]);
	});

	it('resets when leaving a wet map for a dry one', () => {
		Water.init(gl, waterData({ level: 0, waveHeight: 0 }));
		setCellHeight(0, 0, -5);
		expect(Water.isSubmerged(0, 0)).toBe(true);

		Water.free(gl);
		expect(Water.hasWater()).toBe(false);
		expect(Water.isSubmerged(0, 0)).toBe(false);

		Water.init(gl, waterData({ vertCount: 0, level: -100 }));
		expect(Water.isSubmerged(0, 0)).toBe(false);
	});
});
