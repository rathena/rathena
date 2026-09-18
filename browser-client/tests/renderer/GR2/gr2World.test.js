import { describe, it, expect } from 'vitest';
import { buildWorld, worldCore, computeGroundOffset, RB_UNIT_SCALE, RB_PLACEMENT_OFS } from 'Renderer/GR2/gr2World.js';

// Row-major flat mat4 (v.M): 3x3 linear at [0,1,2 / 4,5,6 / 8,9,10], translation at [12,13,14].
const rowMag = (m, r) => Math.hypot(m[r * 4], m[r * 4 + 1], m[r * 4 + 2]);
function det3(m) {
	const a = m[0],
		b = m[1],
		c = m[2],
		d = m[4],
		e = m[5],
		f = m[6],
		g = m[8],
		h = m[9],
		i = m[10];
	return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

describe('gr2World.buildWorld — /5 referential (build-7 reconciliation)', () => {
	// DIR 90 exercises a real rotation; POS 0 isolates the straddle.
	const rb = buildWorld(90, [0, 0, 0]);

	it('scales the geometry 3x3 rows to 0.2 (the /5 master scale)', () => {
		expect(RB_UNIT_SCALE).toBe(0.2);
		for (let r = 0; r < 3; r++) {
			expect(Math.abs(rowMag(rb, r) - 0.2)).toBeLessThan(1e-6);
		}
	});

	it('is reflection-free (det(3x3) > 0 — no in-model flipX; the camera carries RH->LH)', () => {
		expect(det3(rb)).toBeGreaterThan(0);
	});

	it('offsets to the cell centre (+0.5 on X,Z) with position fed unchanged', () => {
		// +0.5 = the cell centre (roBrowser's cell-middle convention: SpriteRenderer.vs / Altitude
		// / overlay all add +0.5 from the raw corner). Co-locates the GR2 with the 2D sprite/shadow.
		expect(RB_PLACEMENT_OFS).toBe(0.5);
		expect(Math.abs(rb[12] - 0.5)).toBeLessThan(1e-6);
		expect(Math.abs(rb[13] - 0)).toBeLessThan(1e-6);
		expect(Math.abs(rb[14] - 0.5)).toBeLessThan(1e-6);

		// Position is left in the caller referential (only the geometry scales) — a mob at
		// cell (10, 20) translates to the cell centre (10.5, 0, 20.5), NOT (2.5, ...).
		const placed = buildWorld(90, [10, 0, 20]);
		expect(Math.abs(placed[12] - 10.5)).toBeLessThan(1e-6);
		expect(Math.abs(placed[14] - 20.5)).toBeLessThan(1e-6);
	});

	it('applies ipRow innermost (identity ipRow is a no-op)', () => {
		const withIdentityIp = buildWorld(90, [0, 0, 0], [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]);
		for (let k = 0; k < 16; k++) {
			expect(Math.abs(withIdentityIp[k] - rb[k])).toBeLessThan(1e-6);
		}
	});

	it('worldCore leaves position unscaled (scale hits geometry, not translation)', () => {
		const core = worldCore(90, [4, 0, 8]);
		expect(Math.abs(core[12] - (4 + RB_PLACEMENT_OFS))).toBeLessThan(1e-6);
		expect(Math.abs(core[14] - (8 + RB_PLACEMENT_OFS))).toBeLessThan(1e-6);
	});
});

describe('gr2World.computeGroundOffset — grounds in the model IP frame', () => {
	// One vertex at model (0, 0, 10). Grounding = -(max world-Y over vertices).
	const mesh = { bind: new Float32Array([0, 0, 10]), vcount: 1 };
	// 90deg rotation about model X (row-vector), like the treasurebox InitialPlacement.
	const ipRotX = [1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1];

	it('null ipRow equals an explicit identity ipRow (backward compatible)', () => {
		const identity = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];
		const off0 = computeGroundOffset([mesh], null);
		const off1 = computeGroundOffset([mesh], identity);
		expect(Math.abs(off0 - off1)).toBeLessThan(1e-6);
	});

	it('a non-identity ipRow changes the grounding (IP is threaded, not ignored)', () => {
		const offIdentity = computeGroundOffset([mesh], null);
		const offRotated = computeGroundOffset([mesh], ipRotX);
		// The IP reorients the mesh before worldCore, so the world-Y extreme differs -> different
		// heightOffset. Equality here would mean the IP was dropped (the pre-fix bug).
		expect(Math.abs(offIdentity - offRotated)).toBeGreaterThan(1e-3);
	});
});
