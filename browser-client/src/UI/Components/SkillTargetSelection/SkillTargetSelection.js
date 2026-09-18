/**
 * UI/Components/SkillTargetSelection/SkillTargetSelection.js
 *
 * Target selection screen when using skill
 *
 * This file is part of ROBrowser (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import SkillInfo from 'DB/Skills/SkillInfo.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Mouse from 'Controls/MouseEventHandler.js';
import Renderer from 'Renderer/Renderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import EntityManager from 'Renderer/EntityManager.js';
import Session from 'Engine/SessionStorage.js';
import Controls from 'Preferences/Controls.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Cursor from 'UI/CursorManager.js';
import PartyFriends from 'UI/Components/PartyFriends/PartyFriends.js';
import htmlText from './SkillTargetSelection.html?raw';
import cssText from './SkillTargetSelection.css?raw';

/**
 * Create component
 */
const SkillTargetSelection = new GUIComponent('SkillTargetSelection', cssText);

SkillTargetSelection.render = () => htmlText;

/**
 * Mouse can cross this UI
 */
SkillTargetSelection.mouseMode = GUIComponent.MouseMode.CROSS;

/**
 * Do not focus this UI
 */
SkillTargetSelection.needFocus = false;

/**
 * Handle ESCAPE before other handlers
 */
SkillTargetSelection.captureKeyEvents = true;

/**
 * @var {constant}
 */
SkillTargetSelection.TYPE = {
	ENEMY: 1,
	PLACE: 2,
	SELF: 4,
	FRIEND: 16,
	TRAP: 32,
	TARGET: 1 | 2 | 16 | 32,
	PET: 64,
	HOMUN: 128
};

/**
 * @var {number} target type (see constants)
 */
let _flag = 0;

/**
 * @var {Skill} skill structure
 */
let _skill;

/**
 * @var {CanvasElement} container for skill name
 */
let _skillName;

/**
 * @var {CanvasElement} container for description
 */
let _description;

/**
 * @var {CanvasElement} container for skill level
 */
let _skillLevel;

/**
 * @var {object} last position used to render the skill level indicator
 */
const _skillLevelPosition = { x: NaN, y: NaN };

/**
 * Initialize component
 */
SkillTargetSelection.init = function init() {
	const root = this.getRoot();

	_skillName = root.querySelector('.skill-name');
	_description = root.querySelector('.skill-description');
	_skillLevel = root.querySelector('.skill-level');

	_skillName.style.display = 'none';
	_description.style.display = 'none';
	_skillLevel.style.display = 'none';

	renderText(DB.getMessage(234), _description);
};

/**
 * Move the skill level indicator next to the pointer.
 * Uses Mouse.screen so it also follows the finger on touch devices.
 */
function updateSkillLevelPosition() {
	if (_skillLevelPosition.x === Mouse.screen.x && _skillLevelPosition.y === Mouse.screen.y) {
		return;
	}

	_skillLevelPosition.x = Mouse.screen.x;
	_skillLevelPosition.y = Mouse.screen.y;

	_skillLevel.style.left = `${Mouse.screen.x + 20}px`;
	_skillLevel.style.top = `${Mouse.screen.y - 18}px`;
}

/**
 * Append to body
 */
SkillTargetSelection.onAppend = function onAppend() {
	_skillName.style.display = 'block';
	_description.style.display = 'block';
	_skillLevel.style.display = 'block';

	updateSkillLevelPosition();

	Renderer.stop(updateSkillLevelPosition);
	Renderer.render(updateSkillLevelPosition);
};

/**
 * Possible to exit using ESCAPE
 */
SkillTargetSelection.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		this.remove();
		event.stopImmediatePropagation();
		return false;
	}

	return true;
};

/**
 * Remove from body
 */
SkillTargetSelection.onRemove = function onRemove() {
	Renderer.stop(updateSkillLevelPosition);

	Cursor.blockMagnetism = false;
	Cursor.freeze = false;
	Cursor.setType(Cursor.ACTION.DEFAULT);

	EntityManager.setSupportPicking(false);

	_skillName.style.display = 'none';
	_description.style.display = 'none';
	_skillLevel.style.display = 'none';

	Mouse.state = Mouse.MOUSE_STATE.NORMAL;
};

/**
 * Set informations for the target
 *
 * @param {object} skill
 * @param {number} skill type
 * @param {string} description name (optional)
 */
SkillTargetSelection.set = function set(skill, target, description) {
	_flag = target;
	_skill = skill;

	if (!_flag) {
		return;
	}

	if (Session.TouchTargeting && !(_flag & SkillTargetSelection.TYPE.PLACE)) {
		const entityFocus = EntityManager.getFocusEntity();
		if (entityFocus) {
			SkillTargetSelection.onUseSkillToId(
				_skill.SKID,
				_skill.useLevel ? _skill.useLevel : _skill.level,
				entityFocus.GID
			);
			SkillTargetSelection.remove();
			return;
		}
	}

	if (_flag & SkillTargetSelection.TYPE.PLACE) {
		Cursor.blockMagnetism = true;
	}

	Mouse.state = Mouse.MOUSE_STATE.USESKILL;

	EntityManager.setSupportPicking((_flag & SkillTargetSelection.TYPE.FRIEND) > 0);

	const sk = SkillInfo[skill.SKID];
	renderText(description || sk.SkillName, _skillName);
	renderLevel(_skill.useLevel ? _skill.useLevel : _skill.level, _skillLevel);

	Cursor.setType(Cursor.ACTION.TARGET);
	Cursor.freeze = true;
};

SkillTargetSelection.setSkillLevelDelta = function setSkillLevelDelta(delta) {
	const sk = SkillInfo[_skill.SKID];
	if (!sk.bSeperateLv) {
		return;
	}
	if (!_skill.useLevel) {
		_skill.useLevel = _skill.level;
	}
	_skill.useLevel += delta;
	if (_skill.useLevel < 1) {
		_skill.useLevel = 1;
	}
	if (_skill.useLevel > _skill.level) {
		_skill.useLevel = _skill.level;
	}

	renderLevel(_skill.useLevel, _skillLevel);
};

/**
 * Render text into the canvas
 *
 * @param {string} text to render
 * @param {CanvasElement} canvas node
 */
function renderText(text, canvas) {
	const fontSize = 12;
	const ctx = canvas.getContext('2d');

	ctx.font = `${fontSize}px Arial`;
	canvas.width = ctx.measureText(text).width + 7 * 2;
	canvas.height = 23;

	ctx.font = `${fontSize}px Arial`;
	ctx.clearRect(0, 0, canvas.width, canvas.height);
	ctx.fillStyle = 'rgba(0,0,0,0.5)';
	ctx.fillRect(0, 0, canvas.width, canvas.height);

	ctx.fillStyle = 'black';
	ctx.fillText(text, 8, 17);

	ctx.fillStyle = '#00FF00';
	ctx.fillText(text, 7, 16);

	canvas.style.left = `${(Renderer.width - canvas.width) >> 1}px`;
}

/**
 * Render level text into the canvas
 *
 * @param {string} text to render
 * @param {CanvasElement} canvas node
 */
function renderLevel(text, canvas) {
	const fontSize = 24;
	const ctx = canvas.getContext('2d');

	canvas.width = 35;
	canvas.height = 35;

	ctx.font = `${fontSize}px Arial`;
	ctx.strokeStyle = '#333333';
	ctx.lineWidth = 3;
	ctx.strokeText(text, 0, 30);
	ctx.fillStyle = 'white';
	ctx.fillText(text, 0, 30);
}

/**
 * Intersect entity when clicking
 *
 * @param {MouseEvent} [event] undefined on touch devices
 * @return {boolean} true when the click still has to be processed by the map controls
 */
function intersectEntities(event) {
	SkillTargetSelection.remove();

	if (!Mouse.intersect) {
		return false;
	}

	if (event && event.which !== 1) {
		return true;
	}

	if (event) {
		event.stopImmediatePropagation();
		event.preventDefault();
	}

	if (_flag & SkillTargetSelection.TYPE.PLACE) {
		SkillTargetSelection.onUseSkillToPos(
			_skill.SKID,
			_skill.useLevel ? _skill.useLevel : _skill.level,
			Mouse.world.x,
			Mouse.world.y
		);
		return false;
	}

	const entity = EntityManager.getOverEntity();

	if (!entity) {
		return false;
	}

	if (entity.objecttype === Entity.TYPE_TRAP && !(_flag & SkillTargetSelection.TYPE.TRAP)) {
		return false;
	}

	intersectEntity(entity);
	return false;
}

/**
 * Handle a click/tap on the map while selecting a target.
 * Called from Controls/MapControl.js, the single entrypoint for mouse and touch input.
 *
 * @param {MouseEvent} [event] undefined on touch devices
 * @return {boolean} true when the click was consumed by the target selection
 */
SkillTargetSelection.onMapMouseDown = function onMapMouseDown(event) {
	return !intersectEntities(event);
};

/**
 * Intersect with an entity
 *
 * @param {object} entity
 */
function intersectEntity(entity) {
	let target = 0;

	switch (entity.objecttype) {
		case Entity.TYPE_MOB:
		case Entity.TYPE_UNIT:
			target = SkillTargetSelection.TYPE.ENEMY | SkillTargetSelection.TYPE.PET;
			break;

		case Entity.TYPE_TRAP:
			target = SkillTargetSelection.TYPE.TRAP;
			break;

		case Entity.TYPE_HOM:
		case Entity.TYPE_MERC:
			target = SkillTargetSelection.TYPE.HOMUN | SkillTargetSelection.TYPE.FRIEND;
			break;

		case Entity.TYPE_PC:
		case Entity.TYPE_ELEM:
			target = SkillTargetSelection.TYPE.FRIEND;
			break;

		default:
			return;
	}

	if (!(target & _flag) && !KEYS.SHIFT && !Controls.noshift && !SkillTargetSelection.checkMapState(entity)) {
		return;
	}

	if (_flag === SkillTargetSelection.TYPE.PET) {
		SkillTargetSelection.onPetSelected(entity.GID);
		return;
	}

	if (_flag & SkillTargetSelection.TYPE.ENEMY && entity === Session.Entity) {
		return;
	}

	SkillTargetSelection.onUseSkillToId(_skill.SKID, _skill.useLevel ? _skill.useLevel : _skill.level, entity.GID);
}

/**
 * Intersect with an entity ID
 * (used in party UI)
 */
SkillTargetSelection.intersectEntityId = function intersectEntityId(id) {
	const entity = EntityManager.get(id);
	if (entity) {
		intersectEntity(entity);
	}
};

/**
 * Check if can use Skill on target based on MapState
 */
SkillTargetSelection.checkMapState = function checkMapState(entity) {
	if (Session.mapState.isPVP) {
		if (Session.hasParty && PartyFriends.isGroupMember(entity.display.name)) {
			return false;
		}
		return true;
	} else if (Session.mapState.isGVG) {
		if (
			(Session.Entity.GUID > 0 && entity.GUID !== Session.Entity.GUID) ||
			(entity.GUID == 0 && entity !== Session.Entity)
		) {
			return true;
		}
	}
	return false;
};

/**
 * Functions to define
 */
SkillTargetSelection.onUseSkillToId = function onUseSkillToId(/*id, level, GID*/) {};
SkillTargetSelection.onUseSkillToPos = function onUseSkillToPos(/*id, level, x, y*/) {};

/**
 * Create component and return it
 */
export default UIManager.addComponent(SkillTargetSelection);
