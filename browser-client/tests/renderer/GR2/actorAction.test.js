import { describe, it, expect } from 'vitest';
import { ACTION, createActor, setAction, actorPose } from 'Renderer/GR2/actorAction.js';

// A model with all five action banks present (indices 0..4 by ACTION order).
function fullModel() {
	return {
		parsed: {
			animations: [
				{ duration: 2 }, // 0 stand
				{ duration: 1.5 }, // 1 move
				{ duration: 0.5 }, // 2 attack
				{ duration: 3 }, // 3 dead
				{ duration: 1 } // 4 damage
			]
		}
	};
}
// A model with only the standby bank (attack/move/... banks absent -> bind pose).
function banklessModel() {
	return { parsed: { animations: [{ duration: 2 }] } };
}

describe('actorAction state machine', () => {
	it('starts in standby and loops the standby clip by its own duration', () => {
		const actor = createActor();
		expect(actor.name).toBe('stand');
		// standbyIdx 0, duration 2 s: elapsed 3 s -> t = 3 % 2 = 1.
		const pose = actorPose(actor, fullModel(), 3000, 0, null);
		expect(pose.animIndex).toBe(0);
		expect(pose.t).toBeCloseTo(1, 5);
	});

	it('standbyIdx < 0 is the NPC bind pose', () => {
		const pose = actorPose(createActor(), fullModel(), 1000, -1, null);
		expect(pose).toEqual({ animIndex: -1, t: 0 });
	});

	it('setAction hard-cuts: resets the clock and clears the loop latch', () => {
		const actor = createActor();
		actor.loop = true;
		setAction(actor, 'attack', 500);
		expect(actor.name).toBe('attack');
		expect(actor.startT).toBe(500);
		expect(actor.loop).toBe(false);
	});

	it('attack plays clip-relative, then auto-reverts to standby on clip end', () => {
		const actor = createActor();
		setAction(actor, 'attack', 0);
		// Mid-clip (attack dur 0.5 s): elapsed 0.3 s.
		let pose = actorPose(actor, fullModel(), 300, 0, null);
		expect(pose.animIndex).toBe(ACTION.attack.idx);
		expect(pose.t).toBeCloseTo(0.3, 5);
		// Past clip end -> reverts to standby (which restarts at 0 relative to now).
		pose = actorPose(actor, fullModel(), 600, 0, null);
		expect(actor.name).toBe('stand');
		expect(pose.animIndex).toBe(0);
		expect(pose.t).toBeCloseTo(0, 5);
	});

	it('dead holds its last frame (min(elapsed, dur))', () => {
		const actor = createActor();
		setAction(actor, 'dead', 0);
		const pose = actorPose(actor, fullModel(), 10000, 0, null); // dead dur 3 s
		expect(actor.name).toBe('dead');
		expect(pose.animIndex).toBe(ACTION.dead.idx);
		expect(pose.t).toBeCloseTo(3, 5);
	});

	it('move loops when latched and never auto-reverts', () => {
		const actor = createActor();
		setAction(actor, 'move', 0);
		actor.loop = true;
		const pose = actorPose(actor, fullModel(), 2000, 0, null); // move dur 1.5 s -> 2 % 1.5 = 0.5
		expect(actor.name).toBe('move');
		expect(pose.animIndex).toBe(ACTION.move.idx);
		expect(pose.t).toBeCloseTo(0.5, 5);
	});

	it('damage is one-shot: reverts to standby after its duration', () => {
		const actor = createActor();
		setAction(actor, 'damage', 0);
		const pose = actorPose(actor, fullModel(), 1200, 0, null); // damage dur 1 s
		expect(actor.name).toBe('stand');
		expect(pose.animIndex).toBe(0);
	});

	it('a missing bank yields the bind pose (attack held; move hard-cuts to standby)', () => {
		const attacker = createActor();
		setAction(attacker, 'attack', 0);
		expect(actorPose(attacker, banklessModel(), 100, 0, null)).toEqual({ animIndex: -1, t: 0 });
		expect(attacker.name).toBe('attack'); // held, no revert

		const mover = createActor();
		setAction(mover, 'move', 0);
		expect(actorPose(mover, banklessModel(), 100, 0, null)).toEqual({ animIndex: -1, t: 0 });
		expect(mover.name).toBe('stand'); // hard-cut to standby
	});
});
