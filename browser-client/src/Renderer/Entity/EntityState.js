/**
 * Renderer/EntityState.js
 *
 * Entity status state & visual state manager — handles body/health/effect states,
 * status effects (poison, freeze, stone, curse, silence, etc.), mounts, and emotion emoticons.
 *
 * @author Vincent Thibault, Antares
 */

/**
 * EntityState mixin
 * Adds status state setters and getters (bodyState, healthState, effectState, emotion) to Entity.
 *
 * @mixin EntityState
 * @property {number} bodyState Body status state ID
 * @property {number} healthState Health status state ID
 * @property {number} effectState Effect state bitmask
 * @property {function(number): void} setEmotion Play emotion icon above entity
 */

import Sound from 'Audio/SoundManager.js';
import StatusConst from 'DB/Status/StatusState.js';
import MountTable from 'DB/Jobs/MountTable.js';
import AllMountTable from 'DB/Jobs/AllMountTable.js';
import Session from 'Engine/SessionStorage.js';
import Emotions from 'DB/Emotions.js';

/**
 * Load dependencies
 */
/**
 * Calculate new color
 */
function recalculateBlendingColor() {
	this.effectColor[0] =
		this._bodyStateColor[0] *
		this._healthStateColor[0] *
		this._effectStateColor[0] *
		this._virtueColor[0] *
		this._flashColor[0];
	this.effectColor[1] =
		this._bodyStateColor[1] *
		this._healthStateColor[1] *
		this._effectStateColor[1] *
		this._virtueColor[1] *
		this._flashColor[1];
	this.effectColor[2] =
		this._bodyStateColor[2] *
		this._healthStateColor[2] *
		this._effectStateColor[2] *
		this._virtueColor[2] *
		this._flashColor[2];
	this.effectColor[3] =
		this._bodyStateColor[3] *
		this._healthStateColor[3] *
		this._effectStateColor[3] *
		this._virtueColor[3] *
		this._flashColor[3];
}

const _stateToVirtue = {};
_stateToVirtue[StatusConst.Status.TWOHANDQUICKEN] = StatusConst.OPT3.QUICKEN;
_stateToVirtue[StatusConst.Status.SPEARQUICKEN] = StatusConst.OPT3.QUICKEN;
_stateToVirtue[StatusConst.Status.LKCONCENTRATION] = StatusConst.OPT3.QUICKEN;
_stateToVirtue[StatusConst.Status.ONEHANDQUICKEN] = StatusConst.OPT3.QUICKEN;
_stateToVirtue[StatusConst.Status.EXPLOSIONSPIRITS] = StatusConst.OPT3.EXPLOSIONSPIRITS;
_stateToVirtue[StatusConst.Status.STEELBODY] = StatusConst.OPT3.STEELBODY;
_stateToVirtue[StatusConst.Status.AURABLADE] = StatusConst.OPT3.AURABLADE;
_stateToVirtue[StatusConst.Status.BLADESTOP] = StatusConst.OPT3.BLADESTOP;
_stateToVirtue[StatusConst.Status.OVERTHRUST] = StatusConst.OPT3.OVERTHRUST;
_stateToVirtue[StatusConst.Status.OVERTHRUSTMAX] = StatusConst.OPT3.OVERTHRUST;
_stateToVirtue[StatusConst.Status.SWOO] = StatusConst.OPT3.OVERTHRUST;
_stateToVirtue[StatusConst.Status.ENERGYCOAT] = StatusConst.OPT3.ENERGYCOAT;
//_stateToVirtue[StatusConst.Status.SKE] = StatusConst.OPT3.ENERGYCOAT; // TODO: Don't use until it's fully implemented, it breaks night mode and looks ugly
_stateToVirtue[StatusConst.Status.BERSERK] = StatusConst.OPT3.BERSERK;
_stateToVirtue[StatusConst.Status.MARIONETTE] = StatusConst.OPT3.MARIONETTE;
_stateToVirtue[StatusConst.Status.MARIONETTE_MASTER] = StatusConst.OPT3.MARIONETTE;
_stateToVirtue[StatusConst.Status.ASSUMPTIO] = StatusConst.OPT3.ASSUMPTIO;
_stateToVirtue[StatusConst.Status.ASSUMPTIO2] = StatusConst.OPT3.ASSUMPTIO;
_stateToVirtue[StatusConst.Status.SG_WARM] = StatusConst.OPT3.WARM;
_stateToVirtue[StatusConst.Status.SG_SUN_WARM] = StatusConst.OPT3.WARM;
_stateToVirtue[StatusConst.Status.SG_MOON_WARM] = StatusConst.OPT3.WARM;
_stateToVirtue[StatusConst.Status.SG_STAR_WARM] = StatusConst.OPT3.WARM;
_stateToVirtue[StatusConst.Status.KAITE] = StatusConst.OPT3.KAITE;
_stateToVirtue[StatusConst.Status.NJ_BUNSINJYUTSU] = StatusConst.OPT3.BUNSIN;
_stateToVirtue[StatusConst.Status.SOULLINK] = StatusConst.OPT3.SOULLINK;
_stateToVirtue[StatusConst.Status.PROPERTYUNDEAD] = StatusConst.OPT3.UNDEAD;
_stateToVirtue[StatusConst.Status.DA_CONTRACT] = StatusConst.OPT3.CONTRACT;
_stateToVirtue[StatusConst.Status.HALLUCINATIONWALK] = StatusConst.OPT3.HALLUCINATIONWALK;

function toggleOpt3(state, enabled) {
	if (state === 0) {
		return;
	}
	const value = _stateToVirtue[state];
	if (value === undefined) {
		console.error('toggleState: unknown state', state);
		return;
	}
	if (enabled) {
		this.virtue = this.virtue | value;
	} else {
		this.virtue = this.virtue & ~value;
	}
}

function getOpt3(state) {
	if (state === 0) {
		return false;
	}
	const value = _stateToVirtue[state];
	if (value === undefined) {
		console.error('toggleState: unknown state', state);
		return false;
	}
	return (this.virtue & value) !== 0;
}
function updateVirtue(value) {
	// Reset value
	this._virtueColor[0] = 1.0;
	this._virtueColor[1] = 1.0;
	this._virtueColor[2] = 1.0;
	this._virtueColor[3] = 1.0;

	if (value & StatusConst.OPT3.QUICKEN) {
		this._virtueColor[2] = 0.0;
	}

	if (value & StatusConst.OPT3.EXPLOSIONSPIRITS) {
		this._virtueColor[0] = 1.0;
		this._virtueColor[1] = 0.75;
		this._virtueColor[2] = 0.75;
	}

	if (value & StatusConst.OPT3.BLADESTOP) {
		this._virtueColor[0] = 0.25;
		this._virtueColor[1] = 0.25;
		this._virtueColor[2] = 0.25;
	}

	if (value & StatusConst.OPT3.ENERGYCOAT || value & StatusConst.OPT3.BUNSIN) {
		this._virtueColor[0] = 0.5;
		this._virtueColor[1] = 0.5;
		this._virtueColor[2] = 0.85;
	}

	if (value & StatusConst.OPT3.OVERTHRUST) {
		this._virtueColor[1] = 0.75;
		this._virtueColor[2] = 0.75;
	}

	if (value & StatusConst.OPT3.WARM) {
		this._virtueColor[1] = 0.4;
		this._virtueColor[2] = 0.4;
	}

	if (value & StatusConst.OPT3.SOULLINK) {
		this._virtueColor[0] = 0.35;
		this._virtueColor[1] = 0.35;
		this._virtueColor[2] = 0.9;
		this._virtueColor[3] = 0.9;
	}

	if (value & StatusConst.OPT3.UNDEAD) {
		this._virtueColor[0] = 0.7;
		this._virtueColor[2] = 0.65;
	}

	if (value & StatusConst.OPT3.MARIONETTE) {
		this._virtueColor[0] = 1.0;
		this._virtueColor[1] = 0.34;
		this._virtueColor[2] = 0.71;
		this._virtueColor[3] = 0.5;
	}

	if (value & StatusConst.OPT3.BERSERK) {
		this._virtueColor[0] = 1.0;
		this._virtueColor[1] = 0.4;
		this._virtueColor[2] = 0.4;
		this._virtueColor[3] = 1.0;
	}

	recalculateBlendingColor.call(this);
	this._virtue = value;
}

/**
 * Change body effect
 * (Stone, sleep, freeze)
 *
 * @param {number} new value
 */
function updateBodyState(value) {
	if (value === this._bodyState) {
		return;
	}

	// Reset value
	this._bodyStateColor[0] = 1.0;
	this._bodyStateColor[1] = 1.0;
	this._bodyStateColor[2] = 1.0;
	this._bodyStateColor[3] = 1.0;

	// Remove previous effect
	switch (this._bodyState) {
		case StatusConst.BodyState.SLEEP:
			this.attachments.remove('status-sleep');
			break;

		case StatusConst.BodyState.FREEZE:
			Sound.playPosition('_frozen_explosion.wav', this.position);
			this.attachments.add({
				frame: 1,
				uid: 'status-freeze',
				file: '\xbe\xf3\xc0\xbd\xb6\xaf'
			});
			this.setAction({
				action: this.ACTION.READYFIGHT,
				frame: 0,
				repeat: true,
				play: true
			});
			break;

		case StatusConst.BodyState.STONE:
			Sound.playPosition('_stone_explosion.wav', this.position);
			this.animation.play = true;
			break;

		case StatusConst.BodyState.STUN:
			this.attachments.remove('status-stun');
			break;
	}

	// Add new effect
	switch (value) {
		case StatusConst.BodyState.STONE:
			this._bodyStateColor[0] = 0.1;
			this._bodyStateColor[1] = 0.1;
			this._bodyStateColor[2] = 0.1;
			this.animation.play = false;
			break;

		case StatusConst.BodyState.STONEWAIT:
			this._bodyStateColor[0] = 0.3;
			this._bodyStateColor[1] = 0.3;
			this._bodyStateColor[2] = 0.3;
			Sound.playPosition('_stonecurse.wav', this.position);
			break;

		case StatusConst.BodyState.SLEEP:
			this.attachments.add({
				repeat: true,
				frame: 0,
				uid: 'status-sleep',
				file: 'status-sleep',
				head: true
			});
			break;

		case StatusConst.BodyState.FREEZE:
			this._bodyStateColor[0] = 0.0;
			this._bodyStateColor[1] = 0.4;
			this._bodyStateColor[2] = 0.8;
			this.attachments.add({
				animationId: 0,
				frame: 0,
				uid: 'status-freeze',
				file: '\xbe\xf3\xc0\xbd\xb6\xaf'
			});
			this.setAction({
				action: this.ACTION.FREEZE2,
				frame: 0,
				repeat: false,
				play: false
			});
			break;

		case StatusConst.BodyState.STUN:
			Sound.playPosition('_stun.wav', this.position);
			this.attachments.add({
				repeat: true,
				frame: 0,
				uid: 'status-stun',
				file: 'status-stun',
				head: true
			});
			break;
	}

	this._bodyState = value;
	recalculateBlendingColor.call(this);
}

/**
 * Modify entity status (freeze, poison)
 *
 * @param {number} new value
 */
function updateHealthState(value) {
	if (value === this._healthState) {
		return;
	}

	this._healthStateColor[0] = 1.0;
	this._healthStateColor[1] = 1.0;
	this._healthStateColor[2] = 1.0;
	this._healthStateColor[3] = 1.0;

	// Curse
	if (value & StatusConst.HealthState.CURSE) {
		// Do not attach multiple times.
		if (!(this._healthState & StatusConst.HealthState.CURSE)) {
			Sound.playPosition('_curse.wav', this.position);
			this.attachments.add({
				repeat: true,
				uid: 'status-curse',
				file: 'status-curse',
				head: true,
				opacity: 0.5
			});
		}

		this._healthStateColor[0] *= 0.5;
		this._healthStateColor[1] *= 0.15;
		this._healthStateColor[2] *= 0.1;
	} else if (!(value & StatusConst.HealthState.CURSE)) {
		this.attachments.remove('status-curse');
	}

	// Poison
	if (value & StatusConst.HealthState.POISON) {
		if (!(this._healthState & StatusConst.HealthState.POISON)) {
			Sound.playPosition('_poison.wav', this.position);
		}
		this._healthStateColor[0] *= 0.9;
		this._healthStateColor[1] *= 0.4;
		this._healthStateColor[2] *= 0.8;
	}

	// Blind
	if (value & StatusConst.HealthState.BLIND) {
		if (!(this._healthState & StatusConst.HealthState.BLIND)) {
			Sound.playPosition('_blind.wav', this.position);
		}
	}

	// Silence
	if (value & StatusConst.HealthState.SILENCE) {
		if (!(this._healthState & StatusConst.HealthState.SILENCE)) {
			Sound.playPosition('_silence.wav', this.position);
			this.attachments.add({
				frame: Emotions.indexes[9],
				file: 'emotion',
				uid: 'status-silence',
				play: true,
				head: true,
				repeat: true,
				depth: 5.0
			});
		}
	} else if (!(value & StatusConst.HealthState.SILENCE)) {
		this.attachments.remove('status-silence');
	}

	this._healthState = value;
	recalculateBlendingColor.call(this);
}

/**
 * Update entity effect (invisible, ...)
 *
 * @param {number} new value
 */
function updateEffectState(value) {
	let costume = 0;

	if (this._allRidingState) {
		// Preserve riding constume
		costume = this.costume;
	}

	this._effectStateColor[0] = 1.0;
	this._effectStateColor[1] = 1.0;
	this._effectStateColor[2] = 1.0;
	this._effectStateColor[3] = 1.0;

	// ------------------------
	// Riding
	// ------------------------

	const RIDING =
		StatusConst.EffectState.RIDING |
		StatusConst.EffectState.DRAGON1 |
		StatusConst.EffectState.DRAGON2 |
		StatusConst.EffectState.DRAGON3 |
		StatusConst.EffectState.DRAGON4 |
		StatusConst.EffectState.DRAGON5 |
		StatusConst.EffectState.WUGRIDER |
		StatusConst.EffectState.MADOGEAR;

	if (value & RIDING) {
		if (this._job in MountTable) {
			costume = MountTable[this._job];
		}
	}

	// ------------------------
	// Costume
	// ------------------------

	// Wedding clones
	if (value & StatusConst.EffectState.WEDDING) {
		costume = 22;
	}

	// Xmas costume
	if (value & StatusConst.EffectState.XMAS) {
		costume = 26;
	}

	// Summer
	if (value & StatusConst.EffectState.SUMMER) {
		costume = 27;
	}

	// ------------------------
	// Effects
	// ------------------------

	// Never show option invisible
	if (value & StatusConst.EffectState.INVISIBLE) {
		this._effectStateColor[3] = 0.0;
	}

	// Cloack / Hide
	else if (
		value &
		(StatusConst.EffectState.HIDE | StatusConst.EffectState.CLOAK | StatusConst.EffectState.CHASEWALK)
	) {
		// Maya purple card
		if (Session.Entity?.intravision) {
			this._effectStateColor[0] = 0.0;
			this._effectStateColor[1] = 0.0;
			this._effectStateColor[2] = 0.0;
		} else {
			this._effectStateColor[3] = 0.0;
		}
	}

	// Camouflage / Stealth Field (receiver)
	else if (this.Camouflage || this.Stealthfield) {
		// Maya purple card
		if (Session.Entity?.intravision) {
			this._effectStateColor[0] = 0.0;
			this._effectStateColor[1] = 0.0;
			this._effectStateColor[2] = 0.0;
		} else {
			this._effectStateColor[3] = 0.1;
			Sound.play('effect/assasin_cloaking.wav');
		}
	}

	// Shadow form
	else if (this.Shadowform) {
		// Maya purple card
		if (Session.Entity?.intravision) {
			this._effectStateColor[0] = 0.0;
			this._effectStateColor[1] = 0.0;
			this._effectStateColor[2] = 0.0;
		} else {
			this._effectStateColor[0] = 0.2;
			this._effectStateColor[1] = 0.2;
			this._effectStateColor[2] = 0.2;
			this._effectStateColor[3] = 0.2;
			Sound.play('effect/assasin_cloaking.wav', this.position);
		}
	}

	// Orcish head
	if (value & StatusConst.EffectState.ORCISH) {
		this.isOrcish = true;
	} else {
		this.isOrcish = false;
	}
	// it triggers the setter defined via Object.defineProperty to re-evaluate the head sprite (e.g., when orcish state changes).
	// eslint-disable-next-line no-self-assign
	this.head = this.head;

	// ------------------------
	// Apply
	// ------------------------

	if (costume !== this.costume) {
		this.costume = costume;
		this.job = this._job;
	}

	this._effectState = value;
	recalculateBlendingColor.call(this);
}

function updateAllRidingState(value) {
	let costume = 0;
	// ------------------------
	// Riding
	// ------------------------
	const EFFECT =
		StatusConst.EffectState.RIDING |
		StatusConst.EffectState.DRAGON1 |
		StatusConst.EffectState.DRAGON2 |
		StatusConst.EffectState.DRAGON3 |
		StatusConst.EffectState.DRAGON4 |
		StatusConst.EffectState.DRAGON5 |
		StatusConst.EffectState.WUGRIDER |
		StatusConst.EffectState.MADOGEAR |
		StatusConst.EffectState.WEDDING |
		StatusConst.EffectState.XMAS |
		StatusConst.EffectState.SUMMER;

	if (value && !(this._effectState & EFFECT)) {
		if (this._job in AllMountTable) {
			costume = AllMountTable[this._job];
		}
	}

	// ------------------------
	// Apply
	// ------------------------
	this._allRidingState = value;
	if (costume !== this.costume) {
		this.costume = costume;
		this.job = this._job;
	}

	recalculateBlendingColor.call(this);
}

function isVisible() {
	return !(
		this._effectState &
			(StatusConst.EffectState.INVISIBLE |
				StatusConst.EffectState.HIDE |
				StatusConst.EffectState.CLOAK |
				StatusConst.EffectState.CHASEWALK) ||
		!!this.Shadowform ||
		!!this.Camouflage ||
		!!this.Stealthfield
	);
}

function isDead() {
	return this.action === this.ACTION.DIE;
}

/**
 * Hooking, export
 */
export default function Init() {
	this._bodyStateColor = new Float32Array([1, 1, 1, 1]);
	this._healthStateColor = new Float32Array([1, 1, 1, 1]);
	this._effectStateColor = new Float32Array([1, 1, 1, 1]);
	this._virtueColor = new Float32Array([1, 1, 1, 1]);
	this._flashColor = new Float32Array([1, 1, 1, 1]);
	this.effectColor = new Float32Array([1, 1, 1, 1]);
	this.isVisible = isVisible.bind(this);
	this.isDead = isDead.bind(this);

	Object.defineProperty(this, 'bodyState', {
		get: function () {
			return this._bodyState;
		},
		set: updateBodyState
	});

	Object.defineProperty(this, 'healthState', {
		get: function () {
			return this._healthState;
		},
		set: updateHealthState
	});

	Object.defineProperty(this, 'effectState', {
		get: function () {
			return this._effectState;
		},
		set: updateEffectState
	});

	Object.defineProperty(this, 'allRidingState', {
		get: function () {
			return this._allRidingState;
		},
		set: updateAllRidingState
	});

	Object.defineProperty(this, 'virtue', {
		get: function () {
			return this._virtue;
		},
		set: updateVirtue
	});

	this.toggleOpt3 = toggleOpt3;
	this.getOpt3 = getOpt3;
	this.recalculateBlendingColor = recalculateBlendingColor;
}
