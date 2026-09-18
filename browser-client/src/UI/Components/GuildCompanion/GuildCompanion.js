import Renderer from 'Renderer/Renderer.js';
import Session from 'Engine/SessionStorage.js';
import DB from 'DB/DBManager.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import KEYS from 'Controls/KeyEventHandler.js';
import 'UI/Elements/Elements.js';
import htmlText from './GuildCompanion.html?raw';
import cssText from './GuildCompanion.css?raw';

const GuildCompanion = new GUIComponent('GuildCompanion', cssText);
GuildCompanion.render = () => htmlText;
let _mode = 'create';
GuildCompanion.onRequestCreateGuild = function onRequestCreateGuild() {};
GuildCompanion.onRequestBreakGuild = function onRequestBreakGuild() {};

GuildCompanion.init = function init() {
	const root = this._shadow;
	this.draggable(root.querySelector('.companion .titlebar'));
	const nameWin = root.querySelector('.win.namebox');
	const input = root.querySelector('.guildname');

	const closeAll = () => {
		GuildCompanion.remove();
	};

	root.querySelector('.btn_create').addEventListener('click', () => {
		nameWin.classList.add('visible');
		input.value = '';
		input.focus();
	});

	root.querySelector('.btn_close').addEventListener('click', closeAll);
	root.querySelector('.btn_x').addEventListener('click', closeAll);
	root.querySelector('.btn_x2').addEventListener('click', closeAll);

	const submit = () => {
		const name = input.value.trim();

		if (!name.length) {
			input.focus();
			return;
		}

		if (_mode === 'disband') {
			if (Session.guildName && name !== Session.guildName) {
				UIManager.showMessageBox(DB.getMessage(401, 'You have failed to disband the guild.'), 'ok', () => {
					input.value = '';
					input.focus();
				});
				return;
			}
			GuildCompanion.onRequestBreakGuild(name);
			return;
		}

		GuildCompanion.onRequestCreateGuild(name);
		closeAll();
	};

	const cancel = () => {
		if (_mode === 'disband') {
			closeAll();
			return;
		}

		nameWin.classList.remove('visible');
	};

	root.querySelector('.btn_ok').addEventListener('click', submit);
	root.querySelector('.btn_cancel').addEventListener('click', cancel);

	input.addEventListener('keydown', event => {
		if (event.which === KEYS.ENTER) {
			event.stopImmediatePropagation();
			submit();
		} else if (event.which === KEYS.ESCAPE) {
			event.stopImmediatePropagation();
			cancel();
		}
	});
};

function open(mode) {
	_mode = mode;

	if (!GuildCompanion.__active) {
		GuildCompanion.append();
	}

	const root = GuildCompanion._shadow;
	const companion = root.querySelector('.win.companion');
	const nameWin = root.querySelector('.win.namebox');
	const input = root.querySelector('.guildname');

	if (mode === 'disband') {
		companion.classList.add('hidden');
		nameWin.classList.add('visible');
		root.querySelector('.name_title').textContent = 'Disband the Guild';
		root.querySelector('.name_label').textContent = 'Enter Guild Name';
		input.value = '';
		input.focus();
	} else {
		companion.classList.remove('hidden');
		nameWin.classList.remove('visible');
		root.querySelector('.name_title').textContent = 'Create Guild';
		root.querySelector('.name_label').textContent = 'Guild Name';
		input.value = '';
	}

	center();
}

function center() {
	const host = GuildCompanion._host;

	if (!host) {
		return;
	}

	const rect = host.getBoundingClientRect();
	const w = Renderer.width || window.innerWidth;
	const h = Renderer.height || window.innerHeight;

	host.style.left = `${Math.max(0, Math.round((w - rect.width) / 2))}px`;
	host.style.top = `${Math.max(0, Math.round((h - rect.height) / 2))}px`;
}

GuildCompanion.openCreate = function openCreate() {
	open('create');
};

GuildCompanion.openDisband = function openDisband() {
	open('disband');
};

GuildCompanion.closeDisband = function closeDisband() {
	if (_mode === 'disband' && GuildCompanion.__active) {
		GuildCompanion.remove();
	}
};

GuildCompanion.toggleCreate = function toggleCreate() {
	if (GuildCompanion.__active) {
		GuildCompanion.remove();
		return;
	}

	GuildCompanion.openCreate();
};

GuildCompanion.mouseMode = GUIComponent.MouseMode.STOP;
GuildCompanion.needFocus = true;

export default UIManager.addComponent(GuildCompanion);
