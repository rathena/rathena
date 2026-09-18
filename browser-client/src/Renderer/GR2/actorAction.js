/**
 * Renderer/GR2/actorAction.js
 *
 * The C3dGrannyGameActor ACTION state machine — SetAction (fcn.00b2c180) + the
 * per-frame driver (fcn.00b2a1b0 -> fcn.00b2a0a0 / fcn.00689a10), transcribed 1:1.
 * Full byte-cite: re-ro-clients-asm/ragexe-mars26/pipelines/gr2-mob-action-mechanic-mars26.md.
 *
 * THE MODEL CLOCK IS ACTION-RELATIVE. SetAction is a hard cut: it resets the model
 * clock to 0, so EVERY action — including the return to standby — restarts its clip at
 * t=0. A missing animation bank yields the BIND pose (animIndex -1), not a held frame.
 *
 * Pure (no WebGL, no wasm) so the headless unit tests drive the transitions directly.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

/**
 * actionType -> anim index (fcn.00b2c180.c:21-53) + name (fcn.00977090).
 */
export const ACTION = {
	stand: { type: 0x00, idx: 0, name: 'ST_STAND' },
	move: { type: 0x08, idx: 1, name: 'ST_MOVE' },
	attack: { type: 0x50, idx: 2, name: 'ST_ATTACK' },
	dead: { type: 0x40, idx: 3, name: 'ST_DEAD' },
	damage: { type: 0x30, idx: 4, name: 'ST_DAMAGE' }
};

/**
 * createActor() -> a fresh actor in the standby state.
 */
export function createActor() {
	return { name: 'stand', startT: 0, loop: false };
}

/**
 * setAction(actor, name, tNow) — the hard cut: reset the action clock (fcn.00b2c180.c:63
 * +0x514=0) and clear the loop latch. A caller that wants the clip to loop (a mob re-driven
 * each movement tick) re-arms actor.loop after.
 */
export function setAction(actor, name, tNow) {
	if (!ACTION[name]) {
		return;
	}
	actor.name = name;
	actor.startT = tNow;
	actor.loop = false;
}

/**
 * actorPose(actor, model, tNow, standbyIdx, frozenT) -> { animIndex, t }, evaluated EVERY
 * frame (standby included) so the clip clock is action-relative. standbyIdx = the model's
 * standby clip (registry animIndex; -1 = the NPC bind pose). frozenT (seconds) freezes the
 * standby scrub for manual A/B; pass null to run. Mutates actor on revert.
 */
export function actorPose(actor, model, tNow, standbyIdx, frozenT) {
	const anims = (model.parsed && model.parsed.animations) || [];
	const elapsed = (tNow - actor.startT) / 1000;

	if (actor.name === 'stand') {
		if (standbyIdx < 0) {
			return { animIndex: -1, t: 0 };
		}
		const standbyDur = anims[standbyIdx] ? anims[standbyIdx].duration || 1 : 1;
		return { animIndex: standbyIdx, t: (frozenT != null ? frozenT : elapsed) % standbyDur };
	}

	const a = ACTION[actor.name];
	const present = anims[a.idx] != null;

	if (actor.name === 'attack') {
		// The ONLY action with a per-frame auto-revert (fcn.00b2a1b0.c:53, gated on +0x52c==2).
		if (!present) {
			return { animIndex: -1, t: 0 };
		}
		const dur = anims[a.idx].duration || 1;
		if (elapsed >= dur) {
			setAction(actor, 'stand', tNow);
			return actorPose(actor, model, tNow, standbyIdx, frozenT);
		}
		return { animIndex: a.idx, t: elapsed };
	}

	if (actor.name === 'dead') {
		// Holds its last frame; the unit is removed by the 0x80 CLR_DEAD vanish, not returned.
		if (!present) {
			return { animIndex: -1, t: 0 };
		}
		const dur = anims[a.idx].duration || 1;
		return { animIndex: a.idx, t: Math.min(elapsed, dur) };
	}

	// damage / move: no per-frame auto-revert (the +0x9d flag is AI-can-act-again, not clip
	// visibility). A missing bank hard-cuts to standby.
	if (!present) {
		setAction(actor, 'stand', tNow);
		return { animIndex: -1, t: 0 };
	}
	const durDM = anims[a.idx].duration || 1;
	// actor.loop = a continuously-driven MOVE (SetAction(MOVE) re-issued each movement tick):
	// scrub modulo duration and never auto-revert — the caller ends it with setAction('stand').
	if (actor.loop) {
		return { animIndex: a.idx, t: elapsed % durDM };
	}
	if (elapsed >= durDM) {
		setAction(actor, 'stand', tNow);
		return actorPose(actor, model, tNow, standbyIdx, frozenT);
	}
	return { animIndex: a.idx, t: elapsed };
}
