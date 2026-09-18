/**
 * Renderer/Entity.js
 *
 * Entity class
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Altitude from 'Renderer/Map/Altitude.js';
import Session from 'Engine/SessionStorage.js';
import Client from 'Core/Client.js';
import glMatrix from 'Utils/gl-matrix.js';

// Example mixins (these would be imported from other files)
import entityControl from 'Controls/EntityControl.js';
import entityAction from './EntityAction.js';
import entityCast from './EntityCast.js';
import entityLife from './EntityLife.js';
import entityDisplay from './EntityDisplay.js';
import entityDialog from './EntityDialog.js';
import entitySound from './EntitySound.js';
import entityView from './EntityView.js';
import entityWalk from './EntityWalk.js';
import entityRender from './EntityRender.js';
import entityRoom from './EntityRoom.js';
import entityState from './EntityState.js';
import entityAttachments from './EntityAttachments.js';
import entityAnimations from './EntityAnimations.js';
import entityAura from './EntityAura.js';
import entityDropEffect from './EntityDropEffect.js';
import entityEmblem from './EntityEmblem.js';
import EntityManager from 'Renderer/EntityManager.js';

const vec3 = glMatrix.vec3;
const mat4 = glMatrix.mat4;

/**
 * Base Entity class
 *
 * @class Entity
 *
 * Base identification & state properties:
 * @property {number} GID Global ID of the entity
 * @property {number} AID Account ID of the entity
 * @property {number} objecttype Type of entity (PC, MOB, NPC, ITEM, etc.)
 * @property {number} direction Current facing direction (0..7)
 * @property {number} headDir Facing direction of the head
 * @property {Float32Array} position [x, y, z] world coordinates
 * @property {Float32Array} matrix World matrix
 * @property {number} clevel Base level
 * @property {number} attack_range Attack range
 * @property {number} attack_speed Attack speed / delay (amotion)
 * @property {boolean} isAdmin GM / Admin flag
 * @property {boolean} hideShadow Whether to draw entity shadow
 * @property {number} remove_tick Time when entity starts vanishing
 * @property {number} remove_delay Delay in ms before entity removal
 *
 * Prototype view/state properties:
 * @property {number} _sex Entity gender
 * @property {number} _job Entity job / ID
 * @property {number} _head Entity head style
 * @property {number} _weapon Entity weapon visual
 * @property {number} _shield Entity shield visual
 * @property {number} _accessory Upper headgear visual
 * @property {number} _accessory2 Middle headgear visual
 * @property {number} _accessory3 Lower headgear visual
 * @property {number} robe Garment / robe visual
 * @property {number} _bodypalette Body palette index
 * @property {number} _headpalette Head palette index
 * @property {number} _bodyState Body status state
 * @property {number} _healthState Health status state
 * @property {number} _effectState Visual effect state flags
 *
 * Companion & Cart properties:
 * @property {boolean} hasCart Whether entity has a cart
 * @property {number} CartNum Cart style index
 * @property {Entity|null} falcon Associated Falcon entity
 * @property {Entity|null} wug Associated Warg entity
 *
 * Mixin Subsystems:
 * @property {Object} walk EntityWalk mixin controller
 * @property {Object} life EntityLife mixin controller (hp, sp, display)
 * @property {Object} display EntityDisplay mixin controller (name, title, emblem)
 * @property {Object} cast EntityCast mixin controller
 * @property {Object} dialog EntityDialog mixin controller
 * @property {Object} room EntityRoom mixin controller
 * @property {Object} emblem EntityEmblem mixin controller
 * @property {Object} attachments EntityAttachments controller
 * @property {Object} animations EntityAnimations controller
 * @property {Object} aura EntityAura controller
 * @property {Object} dropEffect EntityDropEffect controller
 */
class Entity {
	/**
	 * Constantes
	 */
	static TYPE_WUG = -7;
	static TYPE_FALCON = -6;
	static TYPE_EFFECT = -5;
	static TYPE_UNKNOWN = -4;
	static TYPE_UNIT = -3;
	static TYPE_TRAP = -2;
	static TYPE_WARP = -1;
	static TYPE_PC = 0;
	static TYPE_DISGUISED = 1;
	static TYPE_ITEM = 2;
	static TYPE_SKILL = 3;
	static TYPE_CHAT = 4;
	static TYPE_MOB = 5;
	static TYPE_NPC = 6;
	static TYPE_PET = 7;
	static TYPE_HOM = 8;
	static TYPE_MERC = 9;
	static TYPE_ELEM = 10;
	static TYPE_ITEM2 = 11; // need to check it was TYPE_ITEM before, but according to rAthena, HercWS and Openkore tha value of TYPE_ITEM is 2
	static TYPE_NPC2 = 12; // recognized as walkable npc
	static TYPE_NPC_ABR = 13; // recognized as mob
	static TYPE_NPC_BIONIC = 14; // recognized as mob

	/**
	 * Vanish Type
	 */
	static VT = {
		OUTOFSIGHT: 0,
		DEAD: 1,
		EXIT: 2,
		TELEPORT: 3,
		TRICKDEAD: 4
	};

	static PickingPriority = {
		Normal: {
			[Entity.TYPE_MOB]: 3,
			[Entity.TYPE_NPC_BIONIC]: 3,
			[Entity.TYPE_NPC_ABR]: 3,
			[Entity.TYPE_ITEM]: 2,
			[Entity.TYPE_NPC]: 1,
			[Entity.TYPE_NPC2]: 1,
			[Entity.TYPE_UNKNOWN]: 0,
			[Entity.TYPE_WARP]: 0,
			[Entity.TYPE_PC]: 0,
			[Entity.TYPE_DISGUISED]: 0,
			[Entity.TYPE_PET]: 0,
			[Entity.TYPE_HOM]: 0,
			[Entity.TYPE_MERC]: 0,
			[Entity.TYPE_ELEM]: 0,
			[Entity.TYPE_UNIT]: 0,
			[Entity.TYPE_TRAP]: 0,
			[Entity.TYPE_EFFECT]: -1,
			[Entity.TYPE_FALCON]: -1,
			[Entity.TYPE_WUG]: -1
		},
		Support: {
			[Entity.TYPE_PC]: 3,
			[Entity.TYPE_DISGUISED]: 3,
			[Entity.TYPE_HOM]: 3,
			[Entity.TYPE_MERC]: 3,
			[Entity.TYPE_ELEM]: 3,
			[Entity.TYPE_MOB]: 2,
			[Entity.TYPE_NPC_ABR]: 2,
			[Entity.TYPE_NPC_BIONIC]: 2,
			[Entity.TYPE_PET]: 1,
			[Entity.TYPE_ITEM]: 0,
			[Entity.TYPE_NPC]: 0,
			[Entity.TYPE_NPC2]: 0,
			[Entity.TYPE_UNKNOWN]: 0,
			[Entity.TYPE_WARP]: 0,
			[Entity.TYPE_UNIT]: 0,
			[Entity.TYPE_TRAP]: 0,
			[Entity.TYPE_EFFECT]: -1,
			[Entity.TYPE_FALCON]: -1,
			[Entity.TYPE_WUG]: -1
		}
	};

	constructor(data) {
		// Initialize properties
		this.boundingRect = { x1: 0, y1: 0, x2: 0, y2: 0 };
		this.matrix = mat4.create();
		this.position = vec3.create();

		// GR2 3D body: resolved model path (EntityView) + the renderer instance
		// handle (EntityRender attaches, EntityManager detaches on removal). Set
		// here, before the mixins below run, so every mixin method sees them defined.
		this.gr2 = null;
		this.gr2Model = null;

		// Apply mixins (adds methods from each module)
		entityControl.call(this);
		entityAction.call(this);
		entityCast.call(this);
		entityLife.call(this);
		entityDisplay.call(this);
		entityDialog.call(this);
		entitySound.call(this);
		entityView.call(this);
		entityWalk.call(this);
		entityRender.call(this);
		entityRoom.call(this);
		entityState.call(this);
		entityAttachments.call(this);
		entityAnimations.call(this);
		entityAura.call(this);
		entityDropEffect.call(this);
		entityEmblem.call(this);

		// Bind data
		if (data) {
			this.clean();
			// Bind data properties to the entity
			this.set(data);
		}
	}

	/**
	 * Initialized Entity data
	 */
	set(unit) {
		let i;

		// Erase previous data
		this.direction = 4;
		this.setAction({
			action: this.ACTION.IDLE,
			frame: 0,
			play: true,
			repeat: true
		});

		// Load shadow
		Client.loadFile(this.files.shadow.spr, null, null, { to_rgba: true });
		Client.loadFile(this.files.shadow.act);

		this.isAdmin = Session.AdminList.indexOf(unit.GID) > -1;
		this.sex = unit.hasOwnProperty('sex') ? unit.sex : this._sex;

		// Apply any pending transformations that arrived before entity spawned
		// We must do this BEFORE setting the job, so UpdateBody knows if it's a transform.
		const pendingTrans = EntityManager.pendingTransformations;
		if (unit.GID && pendingTrans && unit.GID in pendingTrans) {
			const pending = pendingTrans[unit.GID];
			if (pending.monster_transform !== undefined) {
				this._monster_transform = pending.monster_transform;
			}
			if (pending.active_monster_transform !== undefined) {
				this._active_monster_transform = pending.active_monster_transform;
			}
			if (pending.job_transform !== undefined) {
				this._job_transform = pending.job_transform;
			}
			delete pendingTrans[unit.GID];
		}

		// Don't override job if transformation is active
		if (unit.hasOwnProperty('job')) {
			if (this._active_monster_transform || this._monster_transform || this._job_transform) {
				// Transformation active - store base job but don't trigger UpdateBody yet
				this._job = unit.job;
				// Update effective job manually and force a redraw
				this.job = this._effectiveJob;
			} else {
				// No transformation - apply job normally (triggers UpdateBody)
				this.job = unit.job;
			}
		} else {
			this.job = this._job;
		}
		this.clothes = 0;
		const keys = Object.keys(unit);
		const count = keys.length;

		for (i = 0; i < count; ++i) {
			switch (keys[i]) {
				// Server send warp as npc,
				// roBrowser has a special type for warp.
				case 'objecttype':
					this.objecttype = unit.job === 45 ? Entity.TYPE_WARP : unit.objecttype;
					entityAction.call(this);
					break;

				// Already set
				case 'sex':
				case 'job':
					break;

				// Not used ?
				case 'xSize':
				case 'ySize':
					break;

				case 'PosDir':
					this.direction = [4, 3, 2, 1, 0, 7, 6, 5][(unit.PosDir[2] + 8) % 8];
					this.position[0] = unit.PosDir[0];
					this.position[1] = unit.PosDir[1];
					this.position[2] =
						Altitude.getCellHeight(unit.PosDir[0], unit.PosDir[1]) +
						(this.objecttype === Entity.TYPE_FALCON ? 5 : 0);
					if (isNaN(this.position[2])) {
						// this can happens when map provided by client is different from server
						this.position[2] = 0;
					}
					break;

				case 'state': {
					const actions = [this.ACTION.IDLE, this.ACTION.DIE, this.ACTION.SIT];
					this.setAction({
						action: actions[unit.state],
						frame: 0,
						play: true,
						repeat: unit.state !== 1
					});
					break;
				}
				case 'action':
					this.setAction({
						action: unit.action,
						play: true,
						frame: 0,
						repeat: unit.action !== 1
					});
					break;

				case 'actStartTime':
					this.animation.tick = unit.actStartTime;
					break;

				case 'speed':
					this.walk.speed = unit.speed;
					break;

				case 'moveStartTime':
					// Keep for walkTo() latency compensation; avoid overriding its tick.
					if (this.walk) {
						this.walk.serverStartTime = unit.moveStartTime;
					}
					break;

				case 'name':
					this.display.name = unit.name;
					if (this.display.name.length == 0) {
						this.display.load = this.display.TYPE.NONE;
					}
					this.display.update(
						this.objecttype === Entity.TYPE_MOB
							? this.display.STYLE.MOB
							: this.objecttype === Entity.TYPE_NPC_ABR
								? this.display.STYLE.MOB
								: this.objecttype === Entity.TYPE_NPC_BIONIC
									? this.display.STYLE.MOB
									: this.objecttype === Entity.TYPE_DISGUISED
										? this.display.STYLE.MOB
										: this.objecttype === Entity.TYPE_NPC
											? this.display.STYLE.NPC
											: this.objecttype === Entity.TYPE_NPC2
												? this.display.STYLE.NPC
												: this.display.STYLE.DEFAULT
					);
					break;

				case 'MoveData': {
					const curX = this.position[0];
					const curY = this.position[1];
					const isUninitialized = !curX && !curY;
					const isTooFar = Math.hypot(curX - unit.MoveData[0], curY - unit.MoveData[1]) > 16;
					if (isUninitialized || isTooFar) {
						this.position[0] = unit.MoveData[0];
						this.position[1] = unit.MoveData[1];
						this.position[2] = Altitude.getCellHeight(unit.MoveData[0], unit.MoveData[1]);
					}
					this.walkTo(
						unit.MoveData[0],
						unit.MoveData[1],
						unit.MoveData[2],
						unit.MoveData[3],
						undefined,
						unit.moveStartTime,
						unit.moveServerEndTime || unit.moveEndTime
					);
					break;
				}

				case 'accessory':
					this.accessory = unit.accessory;
					break;

				case 'accessory2':
					this.accessory2 = unit.accessory2;
					break;

				case 'accessory3':
					this.accessory3 = unit.accessory3;
					break;

				case 'Robe':
					this.robe = unit.Robe;
					break;

				case 'hideShadow':
					this.hideShadow = unit.hideShadow;
					break;

				case 'level':
					this.clevel = unit.level;
					break;

				default:
					if (Entity.prototype.hasOwnProperty(keys[i]) || Entity.prototype.hasOwnProperty(`_${keys[i]}`)) {
						this[keys[i]] = unit[keys[i]];
					}
					break;
			}
		}

		// Rendering life
		if (this.life.hp > -1 && this.life.hp_max > -1) {
			this.life.update();
			this.life.display = true;
		}
	}

	/**
	 * Remove Entity's attached GUI
	 */
	clean() {
		// Remove UI elements
		this.life.clean();
		this.emblem.clean();
		this.display.clean();
		this.dialog.clean();
		this.cast.clean();
		this.room.clean();
		this.attachments.remove('lockon');
		this.animations.free();
		this.aura.free();
		this.dropEffect.free();

		// Remove
		this.remove_tick = 0;
		this.remove_delay = 0;

		this.falcon = null;
		this.wug = null;
		// Avoid conflict if entity re-appears. Official sets it to -1
		this.GID = -1;
	}

	/**
	 * Remove Entity from screen (die, spam, etc.)
	 *
	 * @param {number} type
	 * @return {boolean} removed immediatly ?
	 */
	remove(type) {
		switch (type) {
			case Entity.VT.OUTOFSIGHT:
				this.clean();
				this.remove_tick = +Date.now();
				this.remove_delay = 1000;
				break;

			case Entity.VT.DEAD: {
				const is_pc = this.objecttype === Entity.TYPE_PC;
				const is_falcon = this.objecttype === Entity.TYPE_FALCON;
				const is_wug = this.objecttype === Entity.TYPE_WUG;
				if (!is_falcon) {
					this.setAction({
						action: this.ACTION.DIE,
						repeat: is_pc,
						play: true,
						frame: 0,
						next: false
					});

					if (!is_pc && !is_wug) {
						this.clean();
						this.remove_tick = +Date.now();
						this.remove_delay = 5000;
					}
				}
				break;
			}
			// Effects are added in onEntityVanish
			//case Entity.VT.EXIT: break;
			//case Entity.VT.TELEPORT: break;
			//case Entity.VT.TRICKDEAD: break;
			default: // No other way ?
				this.clean();
				this.remove_tick = Date.now();
				this.remove_delay = 0;
				break;
		}
	}

	/**
	 * Look at a cell
	 *
	 * @param {number} to_x
	 * @param {number} to_y
	 */
	lookTo(to_x, to_y) {
		const x = Math.round(to_x - this.position[0]);
		const y = Math.round(to_y - this.position[1]);
		let dir;

		if (x >= 1) {
			dir = y >= 1 ? 5 : y === 0 ? 6 : 7;
		}
		if (x === 0) {
			dir = y >= 1 ? 4 : 0;
		}
		if (x <= -1) {
			dir = y >= 1 ? 3 : y === 0 ? 2 : 1;
		}

		const prevDirection = this.direction;
		if (prevDirection === dir) {
			// turn head straight
			this.headDir = 0;
		} else {
			switch (((prevDirection - dir + 8) % 8) - 4) {
				// turn head left
				case -3:
					if (this.headDir === 2) {
						this.direction = dir;
						this.headDir = 0;
						break;
					}
				case -2:
				case -1:
					this.direction = (dir + 9) % 8;
					this.headDir = 2;
					break;

				// turn head right
				case 3:
					if (this.headDir === 1) {
						this.direction = dir;
						this.headDir = 0;
						break;
					}
				case 2:
				case 1:
					this.direction = (dir + 7) % 8;
					this.headDir = 1;
					break;

				case 0:
					switch (this.headDir) {
						case 2:
							this.direction = (dir + 9) % 8;
							break;
						case 1:
							this.direction = (dir + 7) % 8;
							break;

						default:
							this.direction = dir;
							this.headDir = 0;
					}
					break;

				// turn
				default:
					this.direction = dir;
			}
		}
	}

	/**
	 * Set guild emblem and refresh entity
	 * @param {Image} image
	 */
	setEntityGuildEmblem(image, animatedCanvas) {
		// Siege emblem (standalone) — always static image
		this.emblem.emblem = image;
		this.emblem.update();

		// Nameplate — EntityDisplay stores animated canvas internally
		this.display.setEmblem(image, animatedCanvas);
		this.display.refresh(this);
	}
}
/**
 * Entity data fields — declared on the prototype intentionally.
 *
 * These MUST stay on Entity.prototype (not as ES2022 class fields) because
 * the set() method uses Entity.prototype.hasOwnProperty(key) as an
 * auto-synchronizing whitelist to decide which server-packet keys are safe
 * to assign.
 *
 * Class instance fields (ES2022) live on `this`, not on the prototype,
 * so hasOwnProperty on the prototype would always return false for them.
 *
 * Mixin-managed properties (walk, life, display, files, room, cast, etc.)
 * are added directly to each instance by the mixin Init functions via
 * .call(this) in the constructor, so they are NOT on the prototype and
 * are correctly rejected by the hasOwnProperty gate.
 *
 * If you add a new data field, add it here so set() can accept it from
 * server packets automatically.
 */
Entity.prototype.objecttype = Entity.TYPE_UNKNOWN;
Entity.prototype.GID = 0;
Entity.prototype.AID = 0;
Entity.prototype._bodyState = 0;
Entity.prototype._healthState = 0;
Entity.prototype._effectState = 0;
Entity.prototype._sex = -1;
Entity.prototype._job = -1;
Entity.prototype._bodypalette = 0;
Entity.prototype._head = -1;
Entity.prototype._headpalette = 0;
Entity.prototype._weapon = -1;
Entity.prototype._shield = -1;
Entity.prototype._accessory = -1;
Entity.prototype._accessory2 = -1;
Entity.prototype._accessory3 = -1;
Entity.prototype.robe = -1;
Entity.prototype.GUID = 0;
Entity.prototype.GEmblemVer = 0;
Entity.prototype.honor = 0;
Entity.prototype.virtue = 0;
Entity.prototype.isPKModeON = 0;
Entity.prototype.xSize = 5;
Entity.prototype.ySize = 5;
Entity.prototype.state = 0;
Entity.prototype.clevel = 0;
Entity.prototype.action = 0;
Entity.prototype.costume = 0;
Entity.prototype.clanId = 0;
Entity.prototype.matrix = null;
Entity.prototype.depth = 0;
Entity.prototype.headDir = 0;
Entity.prototype.direction = 0;
Entity.prototype.position = null;
Entity.prototype.attack_range = 0;
Entity.prototype.attack_speed = 300;
Entity.prototype.effectColor = null;
Entity.prototype.isAdmin = false;
Entity.prototype.hasCart = false;
Entity.prototype.CartNum = 0;
Entity.prototype.lastSKID = 0;
Entity.prototype.lastSkLvl = 0;
Entity.prototype.amotionTick = 0;
Entity.prototype.targetGID = 0;
Entity.prototype.isOverWeight = false;
Entity.prototype.falcon = null;
Entity.prototype.wug = null;
Entity.prototype.hideShadow = false;
Entity.prototype.call_flag = 0;
Object.defineProperty(Entity.prototype, 'level', {
	get() {
		return this.clevel;
	},
	set(val) {
		this.clevel = val;
	},
	enumerable: true,
	configurable: true
});

/**
 * Player-specific fields — declared on Entity.prototype so Entity.set() can
 * auto-map them from server packets (charselect, status updates, etc.).
 *
 * The whitelist gate in set() uses Entity.prototype.hasOwnProperty() which does
 * NOT walk the prototype chain, so any field only on Player.prototype is
 * invisible to it. Entity.prototype already hosts player-specific state
 * (clevel, hasCart, falcon, call_flag, etc.), so this follows the established
 * pattern.
 */

// ── Job level ────────────────────────────────────────────────────────────────
Entity.prototype.joblevel = 0; // StatusProperty.JOBLEVEL / charselect 'joblevel'

// ── Economy ───────────────────────────────────────────────────────────────────
Entity.prototype.money = 0; // StatusProperty.MONEY (zeny) / charselect 'money'
Entity.prototype.weight = 0; // StatusProperty.WEIGHT
Entity.prototype.max_weight = 0; // StatusProperty.MAXWEIGHT

// ── Player-only flags ─────────────────────────────────────────────────────────
Entity.prototype.intravision = false; // PACKET.ZC.STATE_CHANGE3 (GM invisibility)

/**
 * @var {integer} tick to remove
 */
Entity.prototype.remove_tick = 0;
/**
 * @var {integer} time to wait to disapear
 */
Entity.prototype.remove_delay = 0;

/**
 * Export
 */
export default Entity;
