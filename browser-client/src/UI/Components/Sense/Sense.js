/**
 * UI/Components/Sense/Sense.js
 *
 * Sense window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 *
 */

import DB from 'DB/DBManager.js';
import Renderer from 'Renderer/Renderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import htmlText from './Sense.html?raw';
import cssText from './Sense.css?raw';

/**
 * Create component
 */
const Sense = new GUIComponent('Sense', cssText);

Sense.render = () => htmlText;

let Elements = [];
let Sizes = [];
let Races = [];

/**
 * @var {object} model
 */
const _model = {
	entity: new Entity(),
	ctx: null,
	render: false,
	tick: 0
};

/**
 * Initialize popup
 */
Sense.init = function init() {
	this._host.style.top = `${(Renderer.height - 120) / 1.5 - 120}px`;
	this._host.style.left = `${(Renderer.width - 280) / 2.0}px`;
	this._host.style.zIndex = '100';

	const root = this.getRoot();

	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => {
			e.stopImmediatePropagation();
		});
		closeBtn.addEventListener('click', () => Sense.remove());
	}

	this.draggable('.header');

	const canvas = root.querySelector('#canvas_model');
	_model.ctx = canvas.getContext('2d');

	Elements = [
		DB.getMessage(414), // 0 = Neutral
		DB.getMessage(415), // 1 = Water
		DB.getMessage(416), // 2 = Earth
		DB.getMessage(417), // 3 = Fire
		DB.getMessage(418), // 4 = Wind
		DB.getMessage(419), // 5 = Poison
		DB.getMessage(420), // 6 = Holy
		DB.getMessage(421), // 7 = Shadow
		DB.getMessage(422), // 8 = Ghost
		DB.getMessage(423) // 9 = Undead
	];

	Sizes = [
		DB.getMessage(443), // 0 = Small
		DB.getMessage(444), // 1 = Medium
		DB.getMessage(445) // 2 = Large
	];

	Races = [
		DB.getMessage(2285), // 0 = Formless
		DB.getMessage(2276), // 1 = Undead
		DB.getMessage(2277), // 2 = Brute
		DB.getMessage(2278), // 3 = Plant
		DB.getMessage(2279), // 4 = Insect
		DB.getMessage(2280), // 5 = Fish
		DB.getMessage(2281), // 6 = Demon
		DB.getMessage(2282), // 7 = Demi-human
		DB.getMessage(2283), // 8 = Angel
		DB.getMessage(2284) // 9 = Dragon
	];
};

/**
 * Set element class based on property value
 */
function _setElementClass(el, value, label) {
	el.className = '';
	if (value < 100) {
		el.className = 'element_bad';
	} else if (value > 100) {
		el.className = 'element_good';
	}
	el.textContent = `${label}: ${value}`;
}

/**
 * Set stats
 *
 * @param {object} pkt
 */
Sense.setWindow = function setWindow(pkt) {
	const root = this.getRoot();

	// TITLE
	root.querySelector('.header .title').textContent = DB.getMessage(406);

	// SPRITE
	_model.entity.set({
		job: pkt.job,
		action: 0,
		direction: 0
	});
	_model.render = true;

	// STATS
	root.querySelector('#label_name').textContent = DB.getMessage(407);

	const valueName = root.querySelector('#value_name');
	valueName.innerHTML = `<a href="https://ratemyserver.net/mob_db.php?small=1&mob_id=${pkt.job}" target="_blank">${DB.getMonsterName(pkt.job)} </a>`;

	root.querySelector('#label_size').textContent = DB.getMessage(410);
	root.querySelector('#value_size').textContent = Sizes[pkt.size];

	root.querySelector('#label_level').textContent = DB.getMessage(408);
	root.querySelector('#value_level').textContent = pkt.level;

	root.querySelector('#label_type').textContent = DB.getMessage(411);
	root.querySelector('#value_type').textContent = Races[pkt.raceType];

	root.querySelector('#label_hp').textContent = DB.getMessage(409);
	root.querySelector('#value_hp').textContent = pkt.hp;

	root.querySelector('#label_mdef').textContent = DB.getMessage(412);
	root.querySelector('#value_mdef').textContent = pkt.mdefPower;

	root.querySelector('#label_def').textContent = DB.getMessage(270);
	root.querySelector('#value_def').textContent = pkt.def;

	root.querySelector('#label_attr').textContent = DB.getMessage(413);
	root.querySelector('#value_attr').textContent = Elements[pkt.property];

	// PROPS
	_setElementClass(root.querySelector('#element_water'), pkt.propertyTable.water, DB.getMessage(415));
	_setElementClass(root.querySelector('#element_wind'), pkt.propertyTable.wind, DB.getMessage(418));
	_setElementClass(root.querySelector('#element_shadow'), pkt.propertyTable.dark, DB.getMessage(421));
	_setElementClass(root.querySelector('#element_earth'), pkt.propertyTable.earth, DB.getMessage(416));
	_setElementClass(root.querySelector('#element_poison'), pkt.propertyTable.poison, DB.getMessage(419));
	_setElementClass(root.querySelector('#element_ghost'), pkt.propertyTable.mental, DB.getMessage(422));
	_setElementClass(root.querySelector('#element_fire'), pkt.propertyTable.fire, DB.getMessage(417));
	_setElementClass(root.querySelector('#element_holy'), pkt.propertyTable.saint, DB.getMessage(420));
	_setElementClass(root.querySelector('#element_undead'), pkt.propertyTable.undead, DB.getMessage(423));

	this._host.style.display = '';

	Renderer.render(render);
};

/**
 * Remove component from HTML
 * Stop rendering
 */
Sense.onRemove = function onRemove() {
	Renderer.stop(render);
};

/**
 * Rendering the Character
 */
function render() {
	SpriteRenderer.bind2DContext(_model.ctx, Math.floor(_model.ctx.canvas.width / 2), _model.ctx.canvas.height);
	_model.ctx.clearRect(0, 0, _model.ctx.canvas.width, _model.ctx.canvas.height);
	_model.entity.renderEntity();
}

/**
 * Create component based on view file and export it
 */
export default UIManager.addComponent(Sense);
