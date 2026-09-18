import { describe, it, expect } from 'vitest';
import { gr2ActionFor, frustumCullClip } from 'Renderer/GR2/gr2Pack.js';

// The mob ACTION enum (EntityAction.js): IDLE=0, WALK=1, ATTACK=2, HURT=3, DIE=4.
const MOB = { IDLE: 0, WALK: 1, ATTACK: 2, HURT: 3, DIE: 4 };
// An NPC enum lacks WALK/ATTACK/DIE keys -> those comparisons never match.
const NPC = { IDLE: 0 };

describe('gr2ActionFor (entity action -> GR2 action name)', () => {
	it('maps a mob action enum semantically', () => {
		expect(gr2ActionFor({ ACTION: MOB, action: MOB.DIE })).toBe('dead');
		expect(gr2ActionFor({ ACTION: MOB, action: MOB.HURT })).toBe('damage');
		expect(gr2ActionFor({ ACTION: MOB, action: MOB.ATTACK })).toBe('attack');
		expect(gr2ActionFor({ ACTION: MOB, action: MOB.WALK })).toBe('move');
		expect(gr2ActionFor({ ACTION: MOB, action: MOB.IDLE })).toBe('stand');
	});

	it('defaults to stand for an out-of-range / negative action', () => {
		expect(gr2ActionFor({ ACTION: MOB, action: -1 })).toBe('stand');
		expect(gr2ActionFor({ ACTION: MOB, action: 99 })).toBe('stand');
	});

	it('an NPC (no WALK/ATTACK/DIE keys) at state 0 stays stand', () => {
		expect(gr2ActionFor({ ACTION: NPC, action: 0 })).toBe('stand');
	});
});

describe('frustumCullClip', () => {
	it('keeps a point inside the NDC box', () => {
		// clip x=0.5, y=-0.3, w=1 -> |x/w|,|y/w| < 1
		expect(frustumCullClip(0.5, -0.3, 1, 0.2)).toBe(false);
	});

	it('culls a point behind the camera (w <= 0)', () => {
		expect(frustumCullClip(0, 0, 0, 0.2)).toBe(true);
		expect(frustumCullClip(0, 0, -2, 0.2)).toBe(true);
	});

	it('culls a point beyond the padded box on x or y', () => {
		// margin 0.2 -> bound = 1.2 * w. x = 1.5 * w is outside.
		expect(frustumCullClip(1.5, 0, 1, 0.2)).toBe(true);
		expect(frustumCullClip(0, -1.5, 1, 0.2)).toBe(true);
	});

	it('keeps a point exactly on the margin edge', () => {
		// x = 1.2 * w == bound -> not strictly greater, kept.
		expect(frustumCullClip(1.2, 0, 1, 0.2)).toBe(false);
	});
});
