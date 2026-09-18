/**
 * @module Renderer/EffectManager
 *
 * Effects Manager
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import EffectDB from 'DB/Effects/EffectTable.js';
import SkillEffect from 'DB/Skills/SkillEffect.js';
import SkillUnit from 'DB/Skills/SkillUnit.js';
import SU from 'DB/Skills/SkillUnitConst.js';
import ItemEffect from 'DB/Items/ItemEffect.js';
import Commands from 'Controls/ProcessCommand.js';
import Events from 'Core/Events.js';
import Configs from 'Core/Configs.js';
import Cylinder from 'Renderer/Effects/Cylinder.js';
import StrEffect from 'Renderer/Effects/StrEffect.js';
import RsmEffect from 'Renderer/Effects/RsmEffect.js';
import TwoDEffect from 'Renderer/Effects/TwoDEffect.js';
import ThreeDEffect from 'Renderer/Effects/ThreeDEffect.js';
import Entity from 'Renderer/Entity/Entity.js';
import EntityManager from 'Renderer/EntityManager.js';
import Renderer from 'Renderer/Renderer.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Sound from 'Audio/SoundManager.js';
import Preferences from 'Preferences/Map.js';
import QuadHorn from 'Renderer/Effects/QuadHorn.js';
import WaterfallEffect from 'Renderer/Effects/WaterfallEffect.js';
import Session from 'Engine/SessionStorage.js';
import GraphicsSettings from 'Preferences/Graphics.js';

/**
 * @type {object} saved webgl context
 */
let _gl;

/**
 * @type {object} effect listing
 */
const _list = {};

/**
 * @type {number} used to differenciate constructors
 */
let _uniqueId = 1;

/**
 * Create a new EF_Init_Par object and attach existing call params
 */
function PrepareInit(callParams) {
	const Params = {
		effectId: -1,
		skillId: null,
		ownerAID: null,
		position: null,
		startTick: null,
		duration: null,
		persistent: false,
		repeatEnd: null,
		repeatDelay: 0,
		otherAID: null,
		otherPosition: null
	};
	Object.assign(Params, callParams);

	if (!Params.startTick) {
		Params.startTick = Renderer.tick;
	}

	Params.ownerEntity = EntityManager.get(Params.ownerAID);
	Params.otherEntity = EntityManager.get(Params.otherAID);

	return Params;
}

/**
 * Spam an effect to the scene
 *
 * @param {object} effect params
 */
function spamSTR(Params) {
	let filename;
	const texturePath = Params.effect.texturePath || '';

	// Get STR file
	if (Preferences.mineffect && Params.effect.min) {
		filename = Params.effect.min;
	} else {
		filename = Params.effect.file;
	}

	// Randomize STR file name
	if (Params.effect.rand) {
		filename = filename.replace(
			'%d',
			Math.round(Params.effect.rand[0] + (Params.effect.rand[1] - Params.effect.rand[0]) * Math.random())
		);
	}

	// Start effect
	EffectManager.add(
		new StrEffect(
			'data/texture/effect/' + filename + '.str',
			Params.Inst.position,
			Params.Inst.startTick,
			texturePath
		),
		Params
	);
}

/**
 * Spam an effect to the scene
 *
 * @param {object} effect prams
 */
function spamSprite(Params) {
	let entity = Params.Init.ownerEntity;
	let isNewEntity = false;

	if (!entity) {
		entity = new Entity();
		entity.GID = Params.Init.ownerAID;
		entity.position = Params.Inst.position;
		entity.objecttype = entity.constructor.TYPE_EFFECT;
		isNewEntity = true;
	} else if (!Params.effect.attachedEntity) {
		entity = new Entity();
		entity.GID = -1;
		entity.position = Params.Inst.position;
		entity.objecttype = entity.constructor.TYPE_EFFECT;
		isNewEntity = true;
	}

	// Sprite effect
	// Use Params.Inst.effectID as uid (for hat effects this is the hatEffectID)
	// Fall back to Params.effect.effectID for other cases.
	// Append duplicateID to make each duplicate instance independent.
	const baseUid = Params.Inst.effectID || Params.effect.effectID;
	const uid = Params.Inst.duplicateID > 0 ? `${baseUid}_${Params.Inst.duplicateID}` : baseUid;

	entity.attachments.add({
		uid: uid,
		file: Params.effect.file,
		head: !!Params.effect.head,
		direction: !!Params.effect.direction,
		repeat: Params.effect.repeat || Params.Inst.persistent,
		duration: Params.effect.duration,
		stackable: !!Params.effect.stackable,
		stopAtEnd: Params.effect.stopAtEnd,
		xOffset: Params.effect.xOffset,
		yOffset: Params.effect.yOffset,
		frame: Params.effect.frame,
		delay: Params.effect.delayFrame,
		renderBefore: !!Params.effect.renderBeforeEntities
	});

	if (isNewEntity) {
		EntityManager.add(entity);
	}
}

// TODO: Move these somewhere else, maybe a DB file
/** for the EffectManager.spamSkillZone */
const targetableUnits = [SU.UNT_ICEWALL, SU.UNT_REVERBERATION];

/** for the EffectManager.spamSkillZone */
const traps = [
	SU.UNT_TRAP,
	SU.UNT_BLASTMINE,
	SU.UNT_SKIDTRAP,
	SU.UNT_ANKLESNARE,
	SU.UNT_LANDMINE,
	SU.UNT_SHOCKWAVE,
	SU.UNT_SANDMAN,
	SU.UNT_FLASHER,
	SU.UNT_FREEZINGTRAP,
	SU.UNT_CLAYMORETRAP,
	SU.UNT_TALKIEBOX,
	SU.UNT_MAGENTATRAP,
	SU.UNT_COBALTTRAP,
	SU.UNT_MAIZETRAP,
	SU.UNT_VERDURETRAP,
	SU.UNT_FIRINGTRAP,
	SU.UNT_ICEBOUNDTRAP,
	SU.UNT_ELECTRICSHOCKE,
	SU.UNT_CLUSTERBOMB,
	SU.UNT_ICEMINE
];

/**
 * Repeat an existing effect if needed
 *
 * @param {object} effect
 */
function repeatEffect(effect) {
	const Params = effect._Params;
	let restartTick = false,
		RepeatParams,
		EF_Inst_Par;

	if (!Params || !Params.Inst) {
		return 0;
	}

	if ((Params.Inst.persistent || Params.Inst.repeatEnd) && !effect._AlreadyRepeated) {
		if (
			Params.Inst.duration &&
			Params.Inst.duration > 0 &&
			Renderer.tick > Params.Inst.endTick + Params.Inst.repeatDelay
		) {
			// Has predefined duration and time to repeat (negative delay)

			if (!Params.Inst.repeatEnd || Params.Inst.repeatEnd > Params.Inst.endTick + Params.Inst.repeatDelay) {
				// Repeat period not ended
				restartTick = Params.Inst.endTick + Params.Inst.repeatDelay; // Reference original timing to avoid timing going crazy
			}
		} else if (effect.needCleanUp) {
			// Finished rendering and need to set a repeat (0 or positive delay)

			if (!Params.Inst.repeatEnd || Params.Inst.repeatEnd > Renderer.tick + Params.Inst.repeatDelay) {
				// Repeat period not ended
				restartTick = Renderer.tick + Params.Inst.repeatDelay;
			}
		}

		if (restartTick) {
			// Re-spam effect if needed to repeat
			EF_Inst_Par = {
				effectID: Params.Inst.effectID,
				duplicateID: Params.Inst.duplicateID,
				startTick: restartTick,
				noDelay: true // Offsets and delays are no longer used
			};

			RepeatParams = {
				effect: Params.effect,
				Inst: EF_Inst_Par,
				Init: Params.Init
			};

			EffectManager.spamEffect(RepeatParams);
			effect._AlreadyRepeated = true;
			return 1;
		}
	}

	return 0;
}

function clean(name, AID, effectID) {
	const effectIdList = Array.isArray(effectID) ? effectID : [effectID];
	let i, count;

	const list = _list[name];
	count = list.length;

	for (i = 0; i < count; ++i) {
		if (
			(!AID || list[i]._Params.Init.ownerAID === AID) &&
			(!effectID || effectIdList.includes(list[i]._Params.Inst.effectID))
		) {
			if (list[i].free) {
				list[i].free(_gl);
			}
			list.splice(i, 1);
			i--;
			count--;
		}
	}

	if (!count) {
		//if (effect.free) {
		//	effect.free(_gl);
		//}
		delete _list[name];
	}
}

function cleanRepeat(name, AID, effectID) {
	const effectIdList = Array.isArray(effectID) ? effectID : [effectID];
	const list = _list[name];

	list.forEach(item => {
		if ((!AID || item._Params.Init.ownerAID === AID) && (!effectID || effectIdList.includes(item.effectID))) {
			if (item._Params.Inst.persistent) {
				item._Params.Inst.persistent = false;
			}

			if (item._Params.Inst.repeatEnd) {
				item._Params.Inst.repeatEnd = false;
			}
		}
	});
}

/**
 * @type {object} Effects namespace
 */
class EffectManager {
	/**
	 * Initialize effects manager
	 */
	static init(gl) {
		_gl = gl;

		if (Configs.get('development')) {
			Commands.add(
				'd_effectmanager',
				'Print EffectManager list to console.',
				function () {
					EffectManager.debug();
				},
				['d_em'],
				true
			);
		} else {
			if (Commands.isEnabled('d_effectmanager')) {
				Commands.remove('d_effectmanager');
			}
		}
	}

	/**
	 * Add effect to the render list
	 *
	 * @param {function} effect renderer function
	 * @param {object} effect parameter object
	 */
	static add(effect, Params) {
		const name = effect.constructor.name || effect.constructor._uid || (effect.constructor._uid = _uniqueId++);

		if (!(name in _list)) {
			_list[name] = [];

			if (effect.constructor.init) {
				effect.constructor.needInit = true;
			}

			// Keep constructor renderBeforeEntities as-is; do not override to false here,
			// so external configuration (e.g., aura ordering) is respected.
		}

		if (effect.init) {
			effect.needInit = true;
		}

		effect._Params = Params;

		// Per-instance ordering: prefer explicit flag, else constructor default
		if (effect._Params && effect._Params.Inst && typeof effect._Params.Inst.renderBeforeEntities !== 'undefined') {
			effect.renderBeforeEntities = !!effect._Params.Inst.renderBeforeEntities;
		} else {
			effect.renderBeforeEntities = !!effect.constructor.renderBeforeEntities;
		}

		// Aura-specific: force renderBeforeEntities for known aura effect IDs to keep them under sprites.
		if (effect._Params && effect._Params.Inst && effect._Params.Inst.effectID) {
			const auraEffectIds = [200, 201, 202, 362, 397, 398, 881];
			if (auraEffectIds.indexOf(effect._Params.Inst.effectID) !== -1) {
				effect.renderBeforeEntities = true;
			}
		}

		_list[name].push(effect);
	}

	/**
	 * Destroy all effects
	 */
	static free(gl) {
		const keys = Object.keys(_list);

		keys.forEach(key => {
			const list = _list[key];
			const constructor = list[0].constructor;

			list.forEach(item => {
				if (item.free) {
					item.free(gl);
				}
			});

			if (constructor.free) {
				constructor.free(gl);
			}

			delete _list[key];
		});
	}

	/**
	 * Renderering all effects
	 *
	 * @param {object} webgl context
	 * @param {mat4} modelView matrix
	 * @param {mat4} projection matrix
	 * @param {mat3} normal matrix
	 * @param {object} fog structure
	 * @param {object} light structure
	 * @param {number} game tick
	 * @param {boolean} render before entities ?
	 */
	static render(gl, modelView, projection, fog, tick, renderBeforeEntities) {
		const keys = Object.keys(_list);
		const count = keys.length;
		let i, j, size, list, constructor;

		// Calculate culling bounds
		// Simple distance check from player
		let center = [0, 0, 0];
		if (Session.Entity && Session.Entity.position) {
			center = Session.Entity.position;
		}
		const area_size = GraphicsSettings.performanceMode ? GraphicsSettings.viewArea : 20;
		const cullDistanceSq = area_size * area_size;

		for (i = 0; i < count; ++i) {
			list = _list[keys[i]];

			if (!list.length) {
				delete _list[keys[i]];
				continue;
			}

			constructor = list[0].constructor;

			// Initialize constructor if needed
			if (!constructor.ready && constructor.needInit) {
				constructor.init(gl);
				constructor.needInit = false;
			}

			if (constructor.ready) {
				constructor.beforeRender(gl, modelView, projection, fog, tick, null);

				for (j = 0, size = list.length; j < size; ++j) {
					// Filter per-instance ordering against the pass flag
					if (!!list[j].renderBeforeEntities !== renderBeforeEntities) {
						continue;
					}

					const effect = list[j];
					const pos = effect._Params && effect._Params.Inst ? effect._Params.Inst.position : null;

					// Culling: If effect has position, check distance
					// Some effects might track an entity, updating their position is usually done in render()
					// so we might need to run a lightweight update if we cull rendering.
					// However, most RO effects are static or simple.
					// Effects with a position can be distance-culled. Effects without one
					// (e.g. standalone viewer effects) have no cullable position and must
					// always render.
					let culled = false;
					if (pos) {
						const distSq =
							(pos[0] - center[0]) * (pos[0] - center[0]) + (pos[1] - center[1]) * (pos[1] - center[1]);
						if (distSq > cullDistanceSq) {
							// Cull this effect (don't render), but we must check lifecycle
							// Check if effect is finished

							// Simulate tick for removal
							let shouldRemove = false;
							if (
								effect._Params.Inst.duration > 0 &&
								effect._Params.Inst.endTick > 0 &&
								tick > effect._Params.Inst.endTick
							) {
								shouldRemove = true;
							}

							if (shouldRemove) {
								effect.needCleanUp = true;
								culled = true;
							} else {
								// Check for repeat logic even if culled
								size += repeatEffect(effect);
								continue; // Skip rendering
							}
						}
					}

					if (!culled) {
						if (!effect.ready && effect.needInit) {
							effect.init(gl);
							effect.needInit = false;
						}

						if (effect.ready) {
							effect.render(gl, tick);
						}
					}

					// Try repeating the effect.
					size += repeatEffect(effect);

					if (effect.needCleanUp) {
						if (effect.free) {
							effect.free(gl);
						}
						list.splice(j, 1);
						j--;
						size--;
					}
				}

				constructor.afterRender(gl);

				if (size === 0) {
					if (constructor.free) {
						constructor.free(gl);
					}
					delete _list[keys[i]];
				}
			}
		}
	}

	/**
	 * Spam an effect to the scene
	 *
	 * @param {object} Effect initial parameters {
	 *     @param effectId {number} effect id
	 *     @param ownerAID {number} owner actor id
	 *     @param position {Array} position
	 *     @param startTick {number} tick
	 *     @param persistent {boolean} is persistent?
	 *     @param repeatEnd {repeatEnd} for how long to repeat
	 *     @param otherAID {number} target/source (other) actor id
	 *     @param otherPosition {Array} target/source (other) position
	 * }
	 */
	static spam(EF_Init_Par) {
		// Empty call
		if (!EF_Init_Par) {
			return;
		}

		// No effect mode (/effect)
		if (!Preferences.effect) {
			return;
		}

		// Prepare params
		EF_Init_Par = PrepareInit(EF_Init_Par);

		// Not found
		if (!(EF_Init_Par.effectId in EffectDB)) {
			return;
		}

		const effects = EffectDB[EF_Init_Par.effectId];
		let EF_Inst_Par;
		let Params;
		let i, j, count;

		for (i = 0, count = effects.length; i < count; ++i) {
			if (effects[i].duplicate == -1) {
				effects[i].duplicate = 999;
			} else {
				effects[i].duplicate = effects[i].duplicate ? Math.min(effects[i].duplicate, 999) : 1;
			}

			effects[i].timeBetweenDupli = !isNaN(effects[i].timeBetweenDupli) ? effects[i].timeBetweenDupli : 200;

			for (j = 0; j < effects[i].duplicate; ++j) {
				EF_Inst_Par = {
					effectID: EF_Init_Par.effectId,
					duplicateID: j,
					startTick: EF_Init_Par.startTick + effects[i].timeBetweenDupli * j
				};

				Params = {
					effect: effects[i],
					Inst: EF_Inst_Par,
					Init: EF_Init_Par
				};

				EffectManager.spamEffect(Params);
			}
		}
	}

	/**
	 * Spam an effect
	 *
	 * @param {object} effect params
	 */
	static spamEffect(Params) {
		let filename;

		Params.Inst.position = Params.Init.position;
		Params.Inst.otherPosition = Params.Init.otherPosition;

		// Propagate per-instance render ordering from effect definition
		if (typeof Params.effect.renderBeforeEntities !== 'undefined') {
			Params.Inst.renderBeforeEntities = !!Params.effect.renderBeforeEntities;
		}

		if (!Params.Inst.position) {
			if (!Params.Init.ownerEntity) {
				return;
			}
			Params.Inst.position = Params.Init.ownerEntity.position;
		}

		if (!Params.Inst.otherPosition) {
			if (Params.Init.otherEntity) {
				Params.Inst.otherPosition = Params.Init.otherEntity.position;
			} else {
				Params.Inst.otherPosition = [
					Params.Inst.position[0] - 5,
					Params.Inst.position[1] + 5,
					Params.Inst.position[2]
				];
			}
		}

		// Copy instead of get reference
		Params.Inst.position = Params.effect.attachedEntity
			? Params.Inst.position
			: [Params.Inst.position[0], Params.Inst.position[1], Params.Inst.position[2]];

		// Repeat
		Params.Inst.persistent = Params.Init.persistent || false;

		if (typeof Params.effect.repeat !== 'undefined' && Params.effect.repeat !== null) {
			Params.Inst.persistent = Params.effect.repeat; // Effect conf overrides. We can selecively enable/disable repeat on parts using this.
		}

		Params.Inst.repeatEnd = Params.Init.repeatEnd ? Params.Init.repeatEnd : Params.effect.repeatEnd || 0; // Main has priority
		Params.Inst.repeatDelay = Params.effect.repeatDelay ? Params.effect.repeatDelay : Params.Init.repeatDelay; // Instance has priority

		// Play sound
		if (Params.effect.wav) {
			filename = Params.effect.wav;

			if (Params.effect.rand) {
				filename = filename.replace(
					'%d',
					Math.round(Params.effect.rand[0] + (Params.effect.rand[1] - Params.effect.rand[0]) * Math.random())
				);
			}

			Events.setTimeout(
				function () {
					//calculate the sound volume from distance
					Sound.playPosition(filename + '.wav', Params.Inst.position);
				},
				Params.Inst.startTick + (!isNaN(Params.effect.delayWav) ? Params.effect.delayWav : 0) - Renderer.tick
			);
		}

		Params.Inst.direction =
			Params.effect.attachedEntity && Params.Init.ownerEntity ? Params.Init.ownerEntity.direction : 0;

		//Set delays
		Params.Inst.duration = !isNaN(Params.effect.duration) ? Params.effect.duration : Params.Init.duration;

		Params.Inst.delayOffsetDelta = !isNaN(Params.effect.delayOffsetDelta)
			? Params.effect.delayOffsetDelta * Params.Inst.duplicateID
			: 0;
		Params.Inst.delayLateDelta = !isNaN(Params.effect.delayLateDelta)
			? Params.effect.delayLateDelta * Params.Inst.duplicateID
			: 0;

		Params.Inst.delayOffset = !isNaN(Params.effect.delayOffset)
			? Params.effect.delayOffset + Params.Inst.delayOffsetDelta
			: 0;
		Params.Inst.delayLate = !isNaN(Params.effect.delayLate)
			? Params.effect.delayLate + Params.Inst.delayLateDelta
			: 0;

		//Start and End
		Params.Inst.startTick =
			Params.Inst.startTick +
			(Params.Inst.noDelay ? Params.Inst.delayOffset + Params.Inst.delayLate : Params.effect.delayStart || 0);
		Params.Inst.endTick =
			Params.Inst.duration > 0
				? Params.Inst.startTick + (Params.Inst.noDelay ? Params.Inst.delayOffset : 0) + Params.Inst.duration
				: -1;

		switch (Params.effect.type) {
			case 'SPR':
				spamSprite(Params);
				break;

			case 'STR':
				spamSTR(Params);
				break;

			case 'CYLINDER':
				EffectManager.add(new Cylinder(Params.effect, Params.Inst, Params.Init), Params);
				break;

			case '2D':
				EffectManager.add(new TwoDEffect(Params.effect, Params.Inst, Params.Init), Params);
				break;

			case '3D':
				EffectManager.add(new ThreeDEffect(Params.effect, Params.Inst, Params.Init), Params);
				break;

			case 'RSM':
			case 'RSM2':
				EffectManager.add(new RsmEffect(Params), Params);
				break;

			case 'WATERFALL':
				EffectManager.add(new WaterfallEffect(Params.effect, Params.Inst, Params.Init), Params);
				break;

			case 'QuadHorn':
				EffectManager.add(new QuadHorn(Params.effect, Params.Inst, Params.Init), Params);
				break;

			case 'FUNC':
				if (Params.effect.func) {
					if (Params.effect.attachedEntity) {
						if (Params.Init.ownerEntity) {
							Params.effect.func.call(this, Params);
						}
					} else {
						Params.effect.func.call(this, Params);
					}
				}
				break;
		}
	}

	/**
	 * Spam effect on ground
	 *
	 * @param {number} unit id
	 * @param {number} position x
	 * @param {number} position y
	 * @param {number} skill unique id
	 */
	static spamSkillZone(unit_id, xPos, yPos, uid, creatorUid) {
		// No effect mode (/effect)
		if (!Preferences.effect) {
			return;
		}

		if (!(unit_id in SkillUnit)) {
			return;
		}

		const effectId = SkillUnit[unit_id];

		if (!(effectId in EffectDB)) {
			return;
		}

		// Remove old version if present (effect & entity)
		EffectManager.remove(null, uid);

		// New Entity
		const entity = new Entity();
		entity.GID = uid;
		entity.position = [xPos, yPos, Altitude.getCellHeight(xPos, yPos)];
		entity.hideShadow = true;
		entity.objecttype = traps.includes(unit_id)
			? entity.constructor.TYPE_TRAP
			: targetableUnits.includes(unit_id)
				? entity.constructor.TYPE_UNIT
				: entity.constructor.TYPE_EFFECT;
		entity.creatorGID = creatorUid;

		EntityManager.add(entity);

		// Effect
		const EF_Init_Par = {
			effectId: effectId,
			ownerAID: uid,
			position: [xPos, yPos, Altitude.getCellHeight(xPos, yPos)],
			startTick: Renderer.tick,
			persistent: true,
			duration: -1, // Infinite by default but the effect param can have a duration that overrides this
			otherAID: creatorUid
		};

		EffectManager.spam(EF_Init_Par);
	}

	/**
	 * Spam a skill on a target
	 *
	 * @param {number} skill id
	 * @param {number} target aid
	 * @param {Array} position
	 * @param {number} tick
	 */
	static spamSkill(skillId, destAID, position, tick, srcAID) {
		let effects, EF_Init_Par;
		if (!(skillId in SkillEffect)) {
			return;
		}

		if (SkillEffect[skillId].effectId) {
			if (typeof SkillEffect[skillId].effectId === 'function') effects = SkillEffect[skillId].effectId(srcAID);
			else effects = SkillEffect[skillId].effectId;
			effects = Array.isArray(effects) ? effects : [effects];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: destAID,
					position: position,
					startTick: tick,
					otherAID: srcAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}

		if (SkillEffect[skillId].effectIdOnCaster && srcAID) {
			effects = Array.isArray(SkillEffect[skillId].effectIdOnCaster)
				? SkillEffect[skillId].effectIdOnCaster
				: [SkillEffect[skillId].effectIdOnCaster];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: srcAID,
					position: position,
					startTick: tick,
					otherAID: destAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}
	}

	/**
	 * Spam skill effect on success
	 *
	 * @param {number} skill id
	 * @param {number} target aid
	 * @param {number} tick
	 */
	static spamSkillSuccess(skillId, destAID, tick, srcAID) {
		let effects, EF_Init_Par;
		if (!(skillId in SkillEffect)) {
			return;
		}

		if (SkillEffect[skillId].successEffectId) {
			effects = Array.isArray(SkillEffect[skillId].successEffectId)
				? SkillEffect[skillId].successEffectId
				: [SkillEffect[skillId].successEffectId];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: destAID,
					startTick: tick,
					otherAID: srcAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}

		if (SkillEffect[skillId].successEffectIdOnCaster) {
			effects = Array.isArray(SkillEffect[skillId].successEffectIdOnCaster)
				? SkillEffect[skillId].successEffectIdOnCaster
				: [SkillEffect[skillId].successEffectIdOnCaster];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: srcAID,
					startTick: tick,
					otherAID: destAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}
	}

	/**
	 * Spam skill effect on hit with damage
	 *
	 * @param {number} skill id
	 * @param {number} target aid
	 * @param {number} tick
	 */
	static spamSkillHit(skillId, destAID, tick, srcAID) {
		let effects, EF_Init_Par;
		if (!(skillId in SkillEffect)) {
			return;
		}

		if (SkillEffect[skillId].hitEffectId) {
			effects = Array.isArray(SkillEffect[skillId].hitEffectId)
				? SkillEffect[skillId].hitEffectId
				: [SkillEffect[skillId].hitEffectId];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: destAID,
					startTick: tick,
					otherAID: srcAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}

		if (SkillEffect[skillId].hitEffectIdOnCaster) {
			effects = Array.isArray(SkillEffect[skillId].hitEffectIdOnCaster)
				? SkillEffect[skillId].hitEffectIdOnCaster
				: [SkillEffect[skillId].hitEffectIdOnCaster];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: srcAID,
					startTick: tick,
					otherAID: destAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}
	}

	/**
	 * Spam skill before the hit lands (regardless of damage)
	 *
	 * @param {number} skill id
	 * @param {number} target aid
	 * @param {number} tick
	 */
	static spamSkillBeforeHit(skillId, destAID, tick, srcAID) {
		let effects, EF_Init_Par;
		if (!(skillId in SkillEffect)) {
			return;
		}

		if (SkillEffect[skillId].beforeHitEffectId) {
			effects = Array.isArray(SkillEffect[skillId].beforeHitEffectId)
				? SkillEffect[skillId].beforeHitEffectId
				: [SkillEffect[skillId].beforeHitEffectId];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: destAID,
					startTick: tick,
					otherAID: srcAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}

		if (SkillEffect[skillId].beforeHitEffectIdOnCaster) {
			effects = Array.isArray(SkillEffect[skillId].beforeHitEffectIdOnCaster)
				? SkillEffect[skillId].beforeHitEffectIdOnCaster
				: [SkillEffect[skillId].beforeHitEffectIdOnCaster];

			// var EF_Init_Par = { // Unused
			// 	effectId: effectId,
			// 	ownerAID: srcAID,
			// 	startTick: tick,
			// 	otherAID: destAID
			// };

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: srcAID,
					startTick: tick,
					otherAID: destAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}
	}

	/**
	 * Spam skill cast effect
	 *
	 * @param {number} skill id
	 * @param {number} target aid
	 * @param {number} tick
	 */
	static spamSkillCast(skillId, destAID, tick, srcAID) {
		let effects, EF_Init_Par;
		if (!(skillId in SkillEffect)) {
			return;
		}

		if (SkillEffect[skillId].beginCastEffectId) {
			effects = Array.isArray(SkillEffect[skillId].beginCastEffectId)
				? SkillEffect[skillId].beginCastEffectId
				: [SkillEffect[skillId].beginCastEffectId];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: destAID,
					startTick: tick,
					otherAID: srcAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}
	}

	/**
	 * Spam a item on a target
	 *
	 * @param {number} item id
	 * @param {number} target aid
	 * @param {Array} position
	 * @param {number} tick
	 */
	static spamItem(itemId, destAID, position, tick, srcAID) {
		let effects, EF_Init_Par;
		if (!(itemId in ItemEffect)) {
			return;
		}

		if (ItemEffect[itemId].effectId) {
			effects = Array.isArray(ItemEffect[itemId].effectId)
				? ItemEffect[itemId].effectId
				: [ItemEffect[itemId].effectId];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: destAID,
					position: position,
					startTick: tick,
					otherAID: srcAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}

		if (ItemEffect[itemId].effectIdOnCaster && srcAID) {
			effects = Array.isArray(ItemEffect[itemId].effectIdOnCaster)
				? ItemEffect[itemId].effectIdOnCaster
				: [ItemEffect[itemId].effectIdOnCaster];

			effects.forEach(effectId => {
				EF_Init_Par = {
					effectId: effectId,
					ownerAID: srcAID,
					position: position,
					startTick: tick,
					otherAID: destAID
				};

				EffectManager.spam(EF_Init_Par);
			});
		}
	}

	/**
	 * Spawn a hat effect attached to an entity
	 */
	static spamHatEffect(Params) {
		if (!Params || !Preferences.effect) {
			return;
		}

		const init = Params.Init || {};
		const ownerAID = init.ownerAID;
		const ownerEntity = init.ownerEntity || EntityManager.get(ownerAID);
		if (!ownerEntity) {
			return;
		}

		const eff = Params.effect || {};
		if (!ownerEntity._hatEffects) {
			ownerEntity._hatEffects = {};
		}

		const resource = (eff.file || eff.resourceFileName || '').toString();

		// EffectTable fallback
		if (!resource && eff.hatEffectID != null) {
			const startTick = init.startTick || Renderer.tick;
			const hatEffectID = eff.hatEffectID;

			if (hatEffectID in EffectDB) {
				const effectEntries = EffectDB[hatEffectID];
				effectEntries.forEach((effectEntry, i) => {
					const EF_Inst_Par = {
						effectID: hatEffectID,
						duplicateID: i,
						startTick: startTick + (effectEntry.timeBetweenDupli || 200) * i
					};

					EffectManager.spamEffect({
						effect: effectEntry,
						Inst: EF_Inst_Par,
						Init: {
							effectId: hatEffectID,
							position: ownerEntity.position,
							ownerEntity: ownerEntity,
							startTick: startTick,
							duration: effectEntry.duration || 0,
							persistent: true,
							repeatEnd: 0,
							repeatDelay: effectEntry.repeatDelay || 0,
							renderBeforeEntities: true
						}
					});
				});
			}

			if (eff.effectID != null) {
				ownerEntity._hatEffects[eff.effectID] = { type: 'effect', effectTableId: eff.hatEffectID };
			}
			return;
		}

		// STR attachment
		if (/\.str$/i.test(resource)) {
			const uid = 'hat-str-' + ownerAID + '-' + (eff.effectID || Date.now());
			const strPath = 'data/texture/effect/' + resource.replace(/\\/g, '/');

			ownerEntity.attachments.add({
				uid: uid,
				strFile: strPath,
				head: eff.isAttachedHead || eff.head || false,
				repeat: true,
				xOffset: eff.xOffset || eff.hatEffectPosX || 0,
				yOffset: eff.yOffset || eff.hatEffectPos || 0,
				renderBefore: !!eff.isRenderBeforeCharacter
			});

			if (eff.effectID != null) {
				ownerEntity._hatEffects[eff.effectID] = { type: 'str', uid: uid };
			}
		}
	}

	/**
	 * Remove a hat effect from an entity
	 */
	static removeHatEffect(ownerAID, effectID) {
		if (ownerAID == null || effectID == null) {
			return;
		}
		const entity = EntityManager.get(ownerAID);
		if (!entity) {
			return;
		}

		const attachments = entity.attachments;
		const hatEffects = entity._hatEffects;
		const info = hatEffects && hatEffects[effectID] ? hatEffects[effectID] : null;

		if (attachments && typeof attachments.remove === 'function') {
			if (info && info.uid) {
				attachments.remove(info.uid);
			} else {
				attachments.remove('hat-str-' + ownerAID + '-' + effectID);
			}

			if (info && info.type === 'effect' && info.effectTableId) {
				attachments.remove(info.effectTableId);
			}
		}

		if (hatEffects) {
			delete hatEffects[effectID];
		}
	}

	static debug() {
		console.log('%c[DEBUG] EffectManager _list: ', 'color:#F5B342', _list);
	}

	/**
	 * Remove an effect
	 *
	 * @param {effect}
	 * @param {mixed} effect owner ID
	 */
	static remove(effect, AID, effectID) {
		if (!effect || !(effect.name in _list)) {
			Object.keys(_list).forEach(key => clean(key, AID, effectID));
		} else {
			clean(effect.name, AID, effectID);
		}

		// Remove entity effects
		if (!(AID == null)) {
			const entity = EntityManager.get(AID);
			if (entity) {
				if (entity.objecttype === entity.constructor.TYPE_EFFECT) {
					EntityManager.remove(AID); // Whole entity is an effect, just remove it
				} else if (!(effectID == null)) {
					entity.attachments.remove(effectID); // Only remove attached effect

					// Cleanup hat effects tracking if present
					if (entity._hatEffects && entity._hatEffects[effectID]) {
						delete entity._hatEffects[effectID];
					}
				}
			}
		}
	}
	/**
	 * Stops an effect's repeat
	 *
	 * @param {effect}
	 * @param {mixed} effect owner ID
	 * @param {mixed} effect ID
	 */
	static endRepeat(effect, AID, effectID) {
		if (!effect || !(effect.name in _list)) {
			Object.keys(_list).forEach(key => cleanRepeat(key, AID, effectID));
			return;
		}

		cleanRepeat(effect.name, AID, effectID);
	}
}

/**
 * Export
 */
export default EffectManager;
