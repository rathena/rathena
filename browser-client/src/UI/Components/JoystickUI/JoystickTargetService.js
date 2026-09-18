/**
 * UI/Components/JoystickUI/JoystickTargetService.js
 *
 * Utility service for identifying and focusing entities (mobs or players)
 * based on proximity or health, optimized for controller-based targeting.
 *
 * @author AoShinHo
 */

import Session from 'Engine/SessionStorage.js';
import EntityManager from 'Renderer/EntityManager.js';
import ControlsSettings from 'Preferences/Controls.js';

function getEntityInContext() {
	let target = null;
	if (ControlsSettings.attackTargetMode === 1) {
		// Lowest HP first
		target = EntityManager.getLowestHpEntity(Session.Entity, Session.Entity.constructor.TYPE_MOB);
		if (!target) {
			target = EntityManager.getLowestHpEntity(Session.Entity, Session.Entity.constructor.TYPE_PC);
		}
	}
	if (!target) {
		target = EntityManager.getClosestEntity(Session.Entity, Session.Entity.constructor.TYPE_MOB);
	}
	if (!target) {
		target = EntityManager.getClosestEntity(Session.Entity, Session.Entity.constructor.TYPE_PC);
	}

	return target || Session.Entity;
}

function focusTarget(entity) {
	let focus = EntityManager.getFocusEntity();
	if (!focus || focus.action === focus.ACTION.DIE) {
		focus = EntityManager.getFocusEntity();
	}
	if (focus && entity.GID !== focus.GID) {
		focus.onFocusEnd();
		EntityManager.setFocusEntity(null);
		entity.onFocus();
		EntityManager.setFocusEntity(entity);
	} else if (!focus) {
		entity.onFocus();
		EntityManager.setFocusEntity(entity);
	}
}
export default {
	getEntity: getEntityInContext,
	focus: focusTarget
};
