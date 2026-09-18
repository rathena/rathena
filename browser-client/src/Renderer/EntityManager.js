/**
 * @module Renderer/EntityManager
 *
 * Manage Entity
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

// Load dependencies
import Session from 'Engine/SessionStorage.js';
import Entity from './Entity/Entity.js';
import SpriteRenderer from './SpriteRenderer.js';
import Mouse from 'Controls/MouseEventHandler.js';
import KEYS from 'Controls/KeyEventHandler.js';
import PathFinding from 'Utils/PathFinding.js';
import GraphicsSettings from 'Preferences/Graphics.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Water from 'Renderer/Map/Water.js';
import GR2ModelRenderer from 'Renderer/GR2/GR2ModelRenderer.js';
const _list = [];

/**
 * Release an entity's GR2 model instance (if any) at the true-removal sites, so a
 * removed mob leaves no ghost in the renderer's instance list. Idempotent.
 * @param {Entity} entity
 */
function releaseGr2(entity) {
	if (entity.gr2Model) {
		GR2ModelRenderer.detach(entity.gr2Model);
		entity.gr2Model = null;
	}
}

// O(1) GID lookup map
const _gidMap = new Map();

// Sort optimization flags
let _renderSortDirty = true;
let _renderFrameCounter = 0;
let _pickSortDirty = true;
let _pickList = [];
let _lastSupportPriority = false;

/**
 * Find an Entity and return it directly via Map lookup (O(1))
 *
 * @param {number} gid
 * @returns {object|null} Entity
 */
function getEntityByGID(gid) {
	if (gid < 0) {
		return null;
	}
	return _gidMap.get(gid) || null;
}

/**
 * Find an Entity and return its index
 *
 * @param {number} aid
 * @returns {number} position
 */
function getEntityIndexBy(getter, value) {
	if (value < 0) {
		return -1;
	}

	const count = _list.length;
	for (let i = 0; i < count; ++i) {
		if (getter(_list[i]) === value) {
			return i;
		}
	}

	return -1;
}

/**
 * Fetch all entities using a callback
 *
 * @param {function} callback
 */
function forEach(callback) {
	const count = _list.length;
	for (let i = 0; i < count; ++i) {
		if (callback(_list[i]) === false) {
			return;
		}
	}
}

/**
 * Find an Entity and return it
 *
 * @param {number} gid
 * @returns {object} Entity
 */
function getEntity(gid) {
	// Reason for this check:
	// - Most packets your received is for the main character, so
	//   this check speed up the process.
	// - When you load a map, the main character is not in the list yet
	//   so we skip a lot of vital informations
	if (Session.Entity && Session.Entity.GID === gid) {
		return Session.Entity;
	}

	return getEntityByGID(gid);
}

/**
 * Pending transformations that arrived before entity spawned
 * { GID: { monster_transform: value, active_monster_transform: value, job_transform: value } }
 */
const pendingTransformations = {};

/**
 * Helper to safely store a pending transformation before entity spawns
 *
 * @param {number} aid - Actor ID
 * @param {string} key - transformation property name
 * @param {*} value - transformation value (monster ID, JobId, or null to clear)
 */
function storePendingTransform(aid, key, value) {
	if (!pendingTransformations[aid]) {
		pendingTransformations[aid] = {};
	}
	pendingTransformations[aid][key] = value;
}

/**
 * Find an Entity via AID and return it
 * Note: Currently Character ID (CID) is stored as AID in Entity
 * Note2: Need to review all AID implementation in both packet and entity
 * @param {number} aid
 * @returns {object} Entity
 */
function getEntityByCID(aid) {
	if (Session.Entity && Session.Entity.AID === aid) {
		return Session.Entity;
	}

	const index = getEntityIndexBy(e => e.AID, aid);
	return index < 0 ? null : _list[index];
}

/**
 * Add or replace entity
 *
 * @param {object} entity
 * @return {object}
 */
function addEntity(entity) {
	const existing = getEntityByGID(entity.GID);
	if (!existing) {
		_list.push(entity);
		_gidMap.set(entity.GID, entity);
		_renderSortDirty = true;
		_pickSortDirty = true;
		return entity;
	} else {
		existing.set(entity);
		return existing;
	}
}

/**
 * Clean up entities from list
 */
function free() {
	_list.forEach(entity => {
		releaseGr2(entity);
		entity.clean();
	});

	_list.length = 0;
	_gidMap.clear();
	_pickList.length = 0;
	_renderSortDirty = true;
	_pickSortDirty = true;
}

/**
 * Remove a GID from the lookup map without removing from render list.
 * Used when entity vanishes (OUTOFSIGHT) so the fade-out animation
 * continues but the GID slot is freed for re-use if entity re-appears.
 * @param {number} gid
 */
function removeGID(gid) {
	_gidMap.delete(gid);
}

/**
 * Remove an entity
 * @param {number} gid
 */
function removeEntity(gid) {
	const entity = _gidMap.get(gid);

	if (entity) {
		releaseGr2(entity);
		entity.clean();
		_gidMap.delete(gid);
		const index = _list.indexOf(entity);
		if (index > -1) {
			_list.splice(index, 1);
		}
		_renderSortDirty = true;
		_pickSortDirty = true;
	}
}

/**
 * @let {Entity} mouse over
 */
let _over = null;

/**
 * Return the entity the mouse is over
 */
function getOverEntity() {
	return _over;
}

/**
 * Set over entity
 */
let _saveShift = false;

function setOverEntity(target) {
	const current = _over;

	if (target === current && _saveShift === KEYS.SHIFT) {
		return;
	}

	_saveShift = KEYS.SHIFT;

	if (current) {
		current.onMouseOut();
	}

	if (target) {
		_over = target;
		target.onMouseOver();
	} else {
		_over = null;
	}
}

/**
 * @let {Entity} target
 */
let _focus = null;

/**
 * Return the entity selected by the user
 */
function getFocusEntity() {
	return _focus;
}

/**
 * Set over entity
 * @param {Entity} entity
 */
function setFocusEntity(entity) {
	_focus = entity;
}

/**
 * Sort entities by z-Index
 *
 * @param {Entity} a
 * @param {Entity} b
 */
function sort(a, b) {
	const aDepth = a.depth + (a.GID % 100) / 1000;
	const bDepth = b.depth + (b.GID % 100) / 1000;

	return bDepth - aDepth;
}

let _supportPriority = false;

/**
 * Set reverse priority for entity sorting (for supportive skills)
 * @param {boolean} v
 */
function setSupportPicking(v) {
	if (_supportPriority !== v) {
		_supportPriority = v;
		_pickSortDirty = true;
	}
}

/**
 * Sort entities by z-index and priorities
 *
 * @param {Entity} a
 * @param {Entity} b
 */
function sortByPriority(a, b) {
	let aDepth = a.depth + (!isNaN(a.GID) ? a.GID % 100 : 0) / 1000;
	let bDepth = b.depth + (!isNaN(b.GID) ? b.GID % 100 : 0) / 1000;

	if (_supportPriority) {
		aDepth -= Entity.PickingPriority.Support[a.objecttype] * 100;
		bDepth -= Entity.PickingPriority.Support[b.objecttype] * 100;
	} else {
		aDepth -= Entity.PickingPriority.Normal[a.objecttype] * 100;
		bDepth -= Entity.PickingPriority.Normal[b.objecttype] * 100;
	}

	return aDepth - bDepth;
}

/**
 * Player-relative view-area culling parameters (performance mode only)
 *
 * @returns {object|null} { x, y, viewAreaSq } or null when culling is off
 */
function getCulling() {
	if (!GraphicsSettings.performanceMode || !Session.Entity || !Session.Entity.position) {
		return null;
	}

	return {
		x: Session.Entity.position[0],
		y: Session.Entity.position[1],
		viewAreaSq: GraphicsSettings.viewArea * GraphicsSettings.viewArea
	};
}

/**
 * @param {object|null} culling from getCulling()
 * @param {Entity} entity
 * @returns {boolean} true when the entity is outside the view area
 */
function isCulled(culling, entity) {
	if (!culling) {
		return false;
	}

	const dx = entity.position[0] - culling.x;
	const dy = entity.position[1] - culling.y;
	return dx * dx + dy * dy > culling.viewAreaSq;
}

/**
 * Render all entities (picking or not)
 *
 * @param {object} gl webgl context
 * @param {mat4} modelView
 * @param {mat4} projection
 * @param {object} fog structure
 * @param {object} renderEffects effect entities? true/false
 *
 * Infos: RO Game doesn't seems to render ambiant and diffuse on Sprites
 */
function render(gl, modelView, projection, fog, renderEffects) {
	let i, count;
	const tick = Date.now();

	// Stop rendering if no units to render (should never happened...)
	if (!_list.length) {
		return;
	}

	// Sort only when dirty or every 3 frames to stay responsive to depth changes
	_renderFrameCounter++;
	if (_renderSortDirty || (GraphicsSettings.performanceMode ? _renderFrameCounter >= 6 : _renderFrameCounter >= 3)) {
		_list.sort(sort);
		_renderSortDirty = false;
		_renderFrameCounter = 0;
		_pickSortDirty = true;
	}

	// Use program
	SpriteRenderer.bind3DContext(gl, modelView, projection, fog);

	const culling = getCulling();

	// Rendering
	for (i = 0, count = _list.length; i < count; ++i) {
		if (
			(_list[i].objecttype != _list[i].constructor.TYPE_EFFECT && !renderEffects) ||
			(_list[i].objecttype == _list[i].constructor.TYPE_EFFECT && renderEffects)
		) {
			// Remove from list
			if (_list[i].remove_tick && _list[i].remove_tick + _list[i].remove_delay < tick) {
				// Remove focus
				const entityFocus = getFocusEntity();
				if (entityFocus && entityFocus.GID === _list[i].GID) {
					entityFocus.onFocusEnd();
					setFocusEntity(null);
				}

				_gidMap.delete(_list[i].GID);
				releaseGr2(_list[i]);
				_list[i].clean();
				_list.splice(i, 1);
				i--;
				count--;
				_pickSortDirty = true;
				continue;
			}
			if (isCulled(culling, _list[i])) {
				continue;
			}
			_list[i].render(modelView, projection);
		}
	}

	// Clean program
	SpriteRenderer.unbind(gl);
}

/**
 * Depth-only pass for entities standing in water, run after all entities
 * are drawn and right before the water so it can hide their submerged part
 * without occluding other sprites.
 *
 * @param {object} gl context
 * @param {mat4} modelView
 * @param {mat4} projection
 * @param {object} fog
 */
function renderWaterDepth(gl, modelView, projection, fog) {
	if (!_list.length || !Water.hasWater()) {
		return;
	}

	const culling = getCulling();

	SpriteRenderer.bind3DContext(gl, modelView, projection, fog);
	gl.colorMask(false, false, false, false);

	for (let i = 0, count = _list.length; i < count; ++i) {
		if (!isCulled(culling, _list[i])) {
			_list[i].renderWaterDepth();
		}
	}

	gl.colorMask(true, true, true, true);
	SpriteRenderer.unbind(gl);
}

/**
 * Intersect Entities
 */
function intersect() {
	let i, count;
	let entity;

	// Stop rendering if no units to render (should never happened...)
	if (!_list.length) {
		return;
	}

	// Only re-sort pick list when entities or priority changed
	if (_pickSortDirty || _lastSupportPriority !== _supportPriority) {
		_pickList = _list.slice();
		_pickList.sort(sortByPriority);
		_pickSortDirty = false;
		_lastSupportPriority = _supportPriority;
	}

	const x = Mouse.screen.x;
	const y = Mouse.screen.y;

	// Culling for picking
	const doCulling = GraphicsSettings.performanceMode;
	let playerX, playerY, viewAreaSq;
	if (doCulling && Session.Entity && Session.Entity.position) {
		playerX = Session.Entity.position[0];
		playerY = Session.Entity.position[1];
		viewAreaSq = GraphicsSettings.viewArea * GraphicsSettings.viewArea;
	}

	for (i = 0, count = _pickList.length; i < count; ++i) {
		entity = _pickList[i];

		// Culling for picking
		if (doCulling) {
			const dx = entity.position[0] - playerX;
			const dy = entity.position[1] - playerY;
			if (dx * dx + dy * dy > viewAreaSq) {
				continue;
			}
		}

		// No picking on dead entites
		if ((entity.action !== entity.ACTION.DIE || entity.objecttype === Entity.TYPE_PC) && entity.remove_tick === 0) {
			if (
				x > entity.boundingRect.x1 &&
				x < entity.boundingRect.x2 &&
				y > entity.boundingRect.y1 &&
				y < entity.boundingRect.y2
			) {
				return entity;
			}
		}
	}

	return null;
}

/**
 * Returns the closest entity to the source entity
 *
 * @param {entity} source entity
 * @param {type} entity type to look for
 */
function getClosestEntity(sourceEntity, type) {
	let closestEntity = false;
	let distance = Infinity;

	const srcX = sourceEntity.position[0];
	const srcY = sourceEntity.position[1];
	const view_range = GraphicsSettings.performanceMode ? GraphicsSettings.viewArea : 20;
	const viewRangeSq = view_range * view_range;

	_list.forEach(entity => {
		if (
			entity.GID !== sourceEntity.GID &&
			entity.objecttype === type &&
			entity.action !== entity.ACTION.DIE &&
			entity.remove_tick === 0
		) {
			const dx = entity.position[0] - srcX;
			const dy = entity.position[1] - srcY;
			const distSq = dx * dx + dy * dy;
			if (distSq > viewRangeSq) {
				return;
			}

			let dst = Infinity;
			if (closestEntity) {
				// Only pathfind if potentially closer than current best
				if (distSq < distance * distance) {
					dst = getPathDistance(sourceEntity, entity);
					if (dst && dst < distance) {
						closestEntity = entity;
						distance = dst;
					}
				}
			} else {
				dst = getPathDistance(sourceEntity, entity);
				if (dst) {
					closestEntity = entity;
					distance = dst;
				}
			}
		}
	});

	return closestEntity;
}

/**
 * Returns the lowest HP entity to the source entity
 *
 * @param {entity} source entity
 * @param {type} entity type to look for
 */
function getLowestHpEntity(sourceEntity, type) {
	let lowestHpEntity = null;
	let lowestHp = Infinity;

	const srcX = sourceEntity.position[0];
	const srcY = sourceEntity.position[1];
	const view_range = GraphicsSettings.performanceMode ? GraphicsSettings.viewArea : 20;
	const viewRangeSq = view_range * view_range;

	_list.forEach(entity => {
		if (
			entity.GID !== sourceEntity.GID &&
			entity.objecttype === type &&
			entity.life &&
			entity.life.hp > 0 &&
			entity.action !== entity.ACTION.DIE &&
			entity.remove_tick === 0
		) {
			const dx = entity.position[0] - srcX;
			const dy = entity.position[1] - srcY;
			const distSq = dx * dx + dy * dy;
			if (distSq > viewRangeSq) {
				return;
			}

			if (entity.life.hp < lowestHp) {
				lowestHp = entity.life.hp;
				lowestHpEntity = entity;
			}
		}
	});

	return lowestHpEntity;
}

/**
 * Returns the distance between two entities based on direct walkpath
 *
 * @param {entity} from entity
 * @param {entity} to entity
 */
function getPathDistance(fromEntity, toEntity) {
	const out = [];
	const count = PathFinding.search(
		fromEntity.position[0] | 0,
		fromEntity.position[1] | 0,
		toEntity.position[0] | 0,
		toEntity.position[1] | 0,
		1,
		out,
		Altitude.TYPE.WALKABLE
	);
	return count;
}

const _lifeCache = new Map();

function storeLife(gid, data) {
	const existing = _lifeCache.get(gid) || {};
	// Merge parcial: só atualiza campos que foram passados (não undefined)
	if (data.hp !== undefined) existing.hp = data.hp;
	if (data.hp_max !== undefined) existing.hp_max = data.hp_max;
	if (data.sp !== undefined) existing.sp = data.sp;
	if (data.sp_max !== undefined) existing.sp_max = data.sp_max;
	if (data.hunger !== undefined) existing.hunger = data.hunger;
	if (data.hunger_max !== undefined) existing.hunger_max = data.hunger_max;
	_lifeCache.set(gid, existing);
}

function getLife(gid) {
	return _lifeCache.get(gid) || null;
}

function removeLife(gid) {
	_lifeCache.delete(gid);
}

function clearLifeCache() {
	_lifeCache.clear();
}

const EntityManager = {
	free: free,
	add: addEntity,
	remove: removeEntity,
	removeGID: removeGID,
	get: getEntity,
	getByCID: getEntityByCID,
	forEach: forEach,

	getOverEntity: getOverEntity,
	setOverEntity: setOverEntity,
	getFocusEntity: getFocusEntity,
	setFocusEntity: setFocusEntity,

	getClosestEntity: getClosestEntity,
	getLowestHpEntity: getLowestHpEntity,

	storeLife: storeLife,
	getLife: getLife,
	removeLife: removeLife,
	clearLifeCache: clearLifeCache,

	render: render,
	renderWaterDepth: renderWaterDepth,
	intersect: intersect,
	setSupportPicking: setSupportPicking,

	pendingTransformations: pendingTransformations,
	storePendingTransform: storePendingTransform
};

/**
 * Get access to manager from Entity object
 */
Entity.Manager = EntityManager;

/**
 * Export
 */
export default EntityManager;
