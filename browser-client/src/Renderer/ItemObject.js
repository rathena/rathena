/**
 * Renderer/ItemObject.js
 *
 * Manage Items in ground
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import EntityManager from './EntityManager.js';
import EffectManager from './EffectManager.js';
import Entity from './Entity/Entity.js';
import Altitude from 'Renderer/Map/Altitude.js';

/**
 * @class ItemObject
 * @description Manages the rendering of items on the ground.
 */
class ItemObject {
	/**
	 * Find an Entity and return its index
	 *
	 * @param {number} gid
	 * @param {number} itemid
	 * @param {boolean} identify
	 * @param {number} count
	 * @param {number} x
	 * @param {number} y
	 * @param {number} z
	 * @param {number} dropeffectmode
	 * @param {boolean} showdropeffect
	 */
	static add(gid, itemid, identify, count, x, y, z, dropeffectmode, showdropeffect) {
		const it = DB.getItemInfo(itemid);
		const path = DB.getItemPath(itemid, identify);
		const entity = new Entity();
		const name = identify ? it.identifiedDisplayName : it.unidentifiedDisplayName;
		/*var dropEffectPostition = [x, y, z];*/ // UNUSED
		entity.GID = gid;
		entity.objecttype = Entity.TYPE_ITEM;
		entity.position[0] = x;
		entity.position[1] = y;
		entity.position[2] = z;

		entity.display.load = entity.display.TYPE.COMPLETE;
		entity.display.name = DB.getMessage(183).replace('%s', name).replace('%d', count);
		entity.display.update(entity.display.STYLE.ITEM);

		entity.files.body.spr = path + '.spr';
		entity.files.body.act = path + '.act';

		entity.files.shadow.size = 0.25;

		if (showdropeffect) {
			entity.dropEffect.load(EffectManager, dropeffectmode);
		}

		// Item falling
		entity.animations.add(function (tick) {
			const level = Altitude.getCellHeight(entity.position[0], entity.position[1]);
			entity.position[2] = Math.max(level, z - tick / 40);
			return entity.position[2] === level;
		});

		EntityManager.add(entity);
	}

	/**
	 * Remove an object from ground
	 *
	 * @param {number} gid
	 */
	static remove(gid) {
		const entity = EntityManager.get(gid);
		if (entity) {
			entity.dropEffect.remove(EffectManager);
		}

		EntityManager.remove(gid);
	}
}
/**
 * Export
 */
export default ItemObject;
