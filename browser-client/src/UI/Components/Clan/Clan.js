/**
 * UI/Components/Clan/Clan.js
 *
 * Character Clan
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author @vthibault, @Javierlog08, @scriptord3, @AoShinHo
 */
import KEYS from 'Controls/KeyEventHandler.js';
import Session from 'Engine/SessionStorage.js';
import Preferences from 'Core/Preferences.js';
import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import 'UI/Elements/Elements.js';
import GUIComponent from 'UI/GUIComponent.js';
import UIManager from 'UI/UIManager.js';
import htmlText from './Clan.html?raw';
import cssText from './Clan.css?raw';

const Clan = new GUIComponent('Clan', cssText);
/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get('Clan', { x: 150, y: 150 }, 1.0);

// render() just returns the static HTML — no template literals needed
Clan.render = () => htmlText;

Clan.init = function init() {
	this.draggable('.titlebar');
	const root = this.getRoot();
	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
		closeBtn.addEventListener('click', () => Clan.toggle());
	}
	this.ui.hide();
};

Clan.onAppend = function onAppend() {
	this._host.style.left = `${_preferences.x}px`;
	this._host.style.top = `${_preferences.y}px`;
};

Clan.onRemove = function onRemove() {
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.save();
};

Clan.onShortCut = function onShortCut(key) {
	if (key.cmd === 'TOGGLE') this.toggle();
};

Clan.onKeyDown = function onKeyDown(event) {
	if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && this.ui.is(':visible')) {
		this.toggle();
	}
};

Clan.toggle = function toggle() {
	if (!Session.hasClan) return;
	if (this.ui.is(':visible')) {
		this.hide();
	} else {
		this.show();
	}
};

Clan.show = function show() {
	this.focus();
	if (this.ui.is(':visible')) return;
	this.ui.show();
};

Clan.hide = function hide() {
	this.ui.hide();
};

Clan.setData = function setData(clan) {
	const root = this.getRoot();
	if (!root) return;
	const info = root.querySelector('.content.info');
	if (!info) return;
	info.querySelector('.name .value').textContent = clan.name;
	info.querySelector('.level .value').textContent = clan.level;
	info.querySelector('.master .value').textContent = clan.master;
	info.querySelector('.members .online').textContent = clan.membersOnline;
	info.querySelector('.members .maxMember').textContent = clan.membersTotal;
	const territory = (clan.territory || '').replace('.gat', '');
	info.querySelector('.territory .value').textContent = territory.charAt(0).toUpperCase() + territory.slice(1);
};

Clan.setMembersCount = function setMembersCount(members) {
	const root = this.getRoot();
	if (!root) return;
	const info = root.querySelector('.content.info');
	if (!info) return;
	info.querySelector('.members .online').textContent = members.membersOnline;
	info.querySelector('.members .maxMember').textContent = members.membersTotal;
};

Clan.setIllust = function setIllust(id) {
	const root = this.getRoot();
	if (!root) return;
	Client.loadFile(`${DB.INTERFACE_PATH}clan_system/clan_illust${id.toString().padStart(2, '0')}.bmp`, data => {
		const el = root.querySelector('.clan_illust');
		if (el) el.style.backgroundImage = 'url(' + data + ')';
	});
};

Clan.setEmblem = function setEmblem(id) {
	const root = this.getRoot();
	if (!root) return;
	Client.loadFile(`${DB.INTERFACE_PATH}clan_system/clan_emblem${id.toString().padStart(2, '0')}.bmp`, data => {
		const el = root.querySelector('.emblem_container');
		if (el) el.style.backgroundImage = 'url(' + data + ')';
	});
};

Clan.setRelations = function setRelations(type, clans) {
	const root = this.getRoot();
	if (!root) return;
	const list = root.querySelector(`.${type === 0 ? 'ally' : 'hostile'}_list`);
	if (!list) return;
	list.innerHTML = '';
	for (let i = 0; i < clans.length; i++) {
		this.addRelation(type, clans[i]);
	}
};

Clan.addRelation = function addRelation(type, clan) {
	const root = this.getRoot();
	if (!root) return;
	const list = root.querySelector(`.${type === 0 ? 'ally' : 'hostile'}_list`);
	if (!list) return;
	const div = document.createElement('div');
	div.dataset.clanId = clan;
	div.textContent = clan;
	list.appendChild(div);
};

Clan.leave = function leave() {
	Session.hasClan = false;
	this.ui.hide();
};

Clan.mouseMode = GUIComponent.MouseMode.STOP;

export default UIManager.addComponent(Clan);
