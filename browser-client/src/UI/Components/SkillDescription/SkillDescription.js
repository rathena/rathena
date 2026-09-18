/**
 * UI/Components/SkillDescription/SkillDescription.js
 *
 * Skill Information
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Renderer from 'Renderer/Renderer.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Mouse from 'Controls/MouseEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './SkillDescription.html?raw';
import cssText from './SkillDescription.css?raw';

/**
 * Whitelist of allowed HTML tags in skill descriptions
 */
const _allowedTags = new Set(['font', 'i', 'b']);

/**
 * Sanitize and format RO text with ^rrggbb color codes, ^nItemID^NNN
 * item name substitution, and newline conversion.
 *
 * @param {string} value - raw skill description text
 * @returns {string} safe HTML string
 */
function _formatROText(value) {
	const tmp = document.createElement('div');
	tmp.innerHTML = String(value);

	tmp.querySelectorAll('*').forEach(el => {
		if (!_allowedTags.has(el.tagName.toLowerCase())) {
			el.replaceWith(...el.childNodes);
		}
	});

	let txt = tmp.innerHTML;

	let result;
	const colorReg = /\^([a-fA-F0-9]{6})/;
	while ((result = colorReg.exec(txt))) {
		txt = txt.replace(result[0], `<span style="color:#${result[1]}">`) + '</span>';
	}

	const itemReg = /\^nItemID\^(\d+)/g;
	while ((result = itemReg.exec(txt))) {
		txt = txt.replace(result[0], DB.getItemInfo(result[1]).identifiedDisplayName);
	}

	txt = txt.replace(/\n/g, '<br/>');

	return txt;
}

/**
 * Create Component
 */
const SkillDescription = new GUIComponent('SkillDescription', cssText);

SkillDescription.render = () => htmlText;

/**
 * SkillDescription unique id
 */
SkillDescription.uid = -1;

/**
 * Possible to exit using ESCAPE
 */
SkillDescription.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this._host.style.display !== 'none') {
		this.remove();
	}
};

/**
 * Once removed
 */
SkillDescription.onRemove = function onRemove() {
	this.uid = -1;
};

/**
 * Initialize UI
 */
SkillDescription.init = function init() {
	const root = this.getRoot();

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		closeBtn.addEventListener('click', () => SkillDescription.remove());
	}

	this.draggable();
};

/**
 * Add content to the box
 *
 * @param {number} skill id
 */
SkillDescription.setSkill = function setSkill(id) {
	this.uid = id;

	const root = this.getRoot();
	const content = root.querySelector('.content');
	if (content) {
		content.innerHTML = _formatROText(DB.getSkillDescription(id));
	}

	const hostWidth = this._host.getBoundingClientRect().width;
	const hostHeight = this._host.getBoundingClientRect().height;

	this._host.style.top = `${Math.min(Mouse.screen.y + 10, Renderer.height - hostHeight)}px`;
	this._host.style.left = `${Math.min(Mouse.screen.x + 10, Renderer.width - hostWidth)}px`;
};

/**
 * Create component and export it
 */
export default UIManager.addComponent(SkillDescription);
