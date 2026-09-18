/**
 * Renderer/EntityAction.js
 *
 * Manage entity action (animation state, action mapping, setAction).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * @typedef {Object} EntityAnimationState
 * @property {number} tick Animation start tick
 * @property {number} frame Current frame index
 * @property {boolean} repeat Loop animation
 * @property {boolean} play Play animation flag
 * @property {boolean} next Next frame ready flag
 * @property {number} delay Animation frame delay
 * @property {boolean|Object} save Saved pending action option
 */

/**
 * EntityAction mixin
 * Adds animation action state, action constants, and setAction method to Entity.
 *
 * @mixin EntityAction
 * @property {Action} ACTION Action mapping constants (IDLE, WALK, ATTACK, etc.)
 * @property {EntityAnimationState} animation Current animation playback state
 * @property {function(Object): void} setAction Set entity animation action
 */

import DB from 'DB/DBManager.js';

/**
 * Action frames
 */
function Action() {
	this.IDLE = 0;
	this.ATTACK = -2;

	this.WALK = -1;
	this.SIT = -1;
	this.PICKUP = -1;
	this.READYFIGHT = -1;
	this.FREEZE = -1;
	this.HURT = -1;
	this.DIE = -1;
	this.FREEZE2 = -1;
	this.ATTACK1 = -1;
	this.ATTACK2 = -1;
	this.ATTACK3 = -1;
	this.SKILL = -1;
	this.ACTION = -1;

	this.SPECIAL = -1;
	this.PERF1 = -1;
	this.PERF2 = -1;
	this.PERF3 = -1;
}

/**
 * Animation object
 */
function Animation() {
	this.tick = 0;
	this.frame = 0;
	this.repeat = true;
	this.play = true;
	this.next = false;
	this.delay = 0;
	this.save = false;
}

/**
 * Modify action, reinitialize animation
 *
 * @param {object} option
 */
function setAction(option) {
	const anim = this.animation;

	if (option.delay) {
		const targetDelay = option.delay < 1000000000 ? Date.now() + option.delay : option.delay;
		if (targetDelay > Date.now()) {
			anim.delay = targetDelay;
			option.delay = 0;
			anim.save = option;
			anim.next = false;
			return;
		}
		option.delay = 0;
	}
	// Know attack frame based on weapon type
	if (option.action === this.ACTION.ATTACK) {
		if (this.objecttype === this.constructor.TYPE_PC) {
			const attack = DB.getWeaponAction(this.weapon, this._job, this._sex);
			option.action = [this.ACTION.ATTACK1, this.ACTION.ATTACK2, this.ACTION.ATTACK3][attack];

			if (!option.length) {
				const slice = DB.getAttackSlice(this._job, this.weapon);
				if (slice) {
					if (typeof option.frame === 'undefined' || option.frame === 0) {
						option.frame = slice.frame;
					}
					option.length = slice.length;
				}
			}
		}

		// No action loaded yet
		if (option.action === -2) {
			option.action = this.ACTION.ATTACK1;
		}
	}

	// FIX: Detect the walk animation change and reset pathfinding route
	const wasWalking = this.action === this.ACTION.WALK;
	const newAction = option.action === -1 || typeof option.action === 'undefined' ? this.ACTION.IDLE : option.action;
	const willWalk = newAction === this.ACTION.WALK;

	if (
		wasWalking &&
		!willWalk &&
		!this.isFastMoving &&
		this.walk &&
		this.walk.total > 0 &&
		this.objecttype !== this.constructor.TYPE_FALCON &&
		this.objecttype !== this.constructor.TYPE_WUG
	) {
		this.resetRoute();
	}

	this.action = newAction;
	anim.tick = Date.now() + 0;
	anim.delay = 0;
	anim.frame = option.frame || 0;
	anim.speed = option.speed || false;
	anim.length = option.length || false;
	anim.repeat = option.repeat || false;
	anim.play = typeof option.play !== 'undefined' ? option.play : true;
	anim.next = option.next || false;
	anim.save = false;

	// Reset sounds
	this.sound.free();
}

/**
 * Initialize Entity action
 */
export default function Init() {
	this.ACTION = new Action();
	this.animation = new Animation();
	this.setAction = setAction;
	const Entity = this.constructor;

	switch (this.objecttype) {
		// Define action, base on type
		case Entity.TYPE_PC:
		case Entity.TYPE_DISGUISED:
			this.ACTION.IDLE = 0;
			this.ACTION.WALK = 1;
			this.ACTION.SIT = 2;
			this.ACTION.PICKUP = 3;
			this.ACTION.READYFIGHT = 4;
			this.ACTION.ATTACK1 = 5;
			this.ACTION.HURT = 6;
			this.ACTION.FREEZE = 7;
			this.ACTION.DIE = 8;
			this.ACTION.FREEZE2 = 9;
			this.ACTION.ATTACK2 = 10;
			this.ACTION.ATTACK3 = 11;
			this.ACTION.SKILL = 12;
			break;

		case Entity.TYPE_MERC:
			if (this._job == 6017) {
				// Bowman
				this.ACTION.IDLE = 0;
				this.ACTION.WALK = 1;
				this.ACTION.SIT = 2;
				this.ACTION.PICKUP = 3;
				this.ACTION.DIE = 4;
				this.ACTION.ATTACK1 = 5;
				this.ACTION.HURT = 6;
				this.ACTION.FREEZE = 7;
				this.ACTION.FREEZE2 = 8;
				// this.ACTION.READYFIGHT = 9; // Gets stuck
				this.ACTION.ATTACK2 = 10;
				this.ACTION.ATTACK3 = 11;
				this.ACTION.SKILL = 12;
			} else if (this._job == 6027 || this._job == 6037) {
				// Spearman/Fencer
				this.ACTION.IDLE = 0;
				this.ACTION.WALK = 1;
				this.ACTION.SIT = 2;
				this.ACTION.PICKUP = 3;
				// this.ACTION.READYFIGHT = 4; // Gets stuck
				this.ACTION.ATTACK1 = 5;
				this.ACTION.HURT = 6;
				this.ACTION.FREEZE = 7;
				this.ACTION.DIE = 8;
				this.ACTION.FREEZE2 = 9;
				this.ACTION.ATTACK2 = 10;
				this.ACTION.ATTACK3 = 11;
				this.ACTION.SKILL = 12;
			} else {
				// Monsters
				this.ACTION.IDLE = 0;
				this.ACTION.WALK = 1;
				this.ACTION.ATTACK = 2;
				this.ACTION.HURT = 3;
				this.ACTION.DIE = 4;
				this.ACTION.ATTACK2 = 5;
				this.ACTION.ATTACK3 = 6;
				this.ACTION.ACTION = 7;
			}
			break;

		// Mob action
		case Entity.TYPE_MOB:
		case Entity.TYPE_NPC_ABR:
		case Entity.TYPE_NPC_BIONIC:
		case Entity.TYPE_WUG:
			this.ACTION.IDLE = 0;
			this.ACTION.WALK = 1;
			this.ACTION.ATTACK = 2;
			this.ACTION.HURT = 3;
			this.ACTION.DIE = 4;
			break;

		case Entity.TYPE_PET:
			this.ACTION.IDLE = 0;
			this.ACTION.WALK = 1;
			this.ACTION.ATTACK = 2;
			this.ACTION.HURT = 3;
			this.ACTION.DIE = 4;
			this.ACTION.SPECIAL = 5;
			this.ACTION.PERF1 = 6;
			this.ACTION.PERF2 = 7;
			this.ACTION.PERF3 = 8;
			break;

		// NPC action
		case Entity.TYPE_NPC:
		case Entity.TYPE_NPC2:
			this.ACTION.IDLE = 0;
			// For those NPC that move with unitwalk scriptcommand
			this.ACTION.WALK = 1;
			break;

		// When you see a warp with /effect, it's 3 times bigger.
		// TODO: put it somewhere else
		case Entity.TYPE_WARP:
			this.xSize = 20;
			this.ySize = 20;
			break;

		// Homunculus
		case Entity.TYPE_HOM:
		case Entity.TYPE_ELEM:
			this.ACTION.IDLE = 0;
			this.ACTION.WALK = 1;
			this.ACTION.ATTACK = 2;
			this.ACTION.HURT = 3;
			this.ACTION.DIE = 4;
			this.ACTION.ATTACK2 = 5;
			this.ACTION.ATTACK3 = 6;
			this.ACTION.ACTION = 7;
			break;

		case Entity.TYPE_FALCON:
			this.ACTION.IDLE = 0;
			this.ACTION.WALK = 1;
			break;
	}
}
