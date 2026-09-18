/**
 * UI/Components/PartyFriends/PartyHelper/PartyHelper.js
 *
 * Helper for the PartyFriends window child (to manage party creation, invitation and updating settings).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import KEYS from 'Controls/KeyEventHandler.js';
import PACKETVER from 'Network/PacketVerManager.js';
import htmlText from './PartyHelper.html?raw';
import cssText from './PartyHelper.css?raw';
import WhisperBox from 'UI/Components/WhisperBox/WhisperBox.js';

/**
 * Create Component
 */
const PartyHelper = new GUIComponent('PartyHelper', cssText);

/**
 * Window type constants
 */
PartyHelper.Type = {
	CREATE: 0,
	SETUP: 1,
	INVITE: 2,
	FRIEND_SETUP: 3
};

/**
 * Current window type
 */
let _type = PartyHelper.Type.CREATE;

/**
 * Helper: query inside shadow root
 */
function _root() {
	return PartyHelper._shadow || PartyHelper._host;
}

/**
 * Render HTML
 */
PartyHelper.render = () => htmlText;

/**
 * Has input fields — protect keyboard events
 */
PartyHelper.captureKeyEvents = true;

/**
 * Initialize component event listeners
 */
PartyHelper.init = function init() {
	const root = _root();

	// Stop propagation to prevent drag/drop conflicts
	const baseBtn = root.querySelector('.base');
	if (baseBtn) {
		baseBtn.addEventListener('mousedown', e => {
			e.stopImmediatePropagation();
			e.preventDefault();
		});
	}

	// Close window handlers
	const closeBtn = root.querySelector('.close');
	if (closeBtn) {
		closeBtn.addEventListener('click', () => {
			PartyHelper.remove();
		});
	}

	const cancelBtn = root.querySelector('.cancel');
	if (cancelBtn) {
		cancelBtn.addEventListener('click', () => {
			PartyHelper.remove();
		});
	}

	// Validation handler
	const okBtn = root.querySelector('.ok');
	if (okBtn) {
		okBtn.addEventListener('click', () => {
			onValidate();
		});
	}

	// Enter key validation on name input
	const nameInput = root.querySelector('.name');
	if (nameInput) {
		nameInput.addEventListener('keydown', event => {
			if (event.which === 13 || event.key === 'Enter') {
				onValidate();
				event.preventDefault();
				event.stopImmediatePropagation();
			}
		});
	}

	// Setting-row toggle handler (WhisperBox preferences)
	root.querySelectorAll('.setting-row').forEach(row => {
		row.addEventListener('mousedown', event => {
			const on = row.querySelector('.on');
			const off = row.querySelector('.off');

			if (!on || !off) {
				return;
			}

			// Toggle visual state
			on.classList.remove('on');
			on.classList.add('off');
			off.classList.remove('off');
			off.classList.add('on');

			// Save preferences immediately
			const prefs = WhisperBox.preferences;
			const rootEl = _root();

			const strangerOn = rootEl.querySelector('.open1to1Stranger .on');
			const friendOn = rootEl.querySelector('.open1to1Friend .on');
			const alarmOn = rootEl.querySelector('.alarm1to1 .on');

			prefs.open1to1Stranger = strangerOn ? parseInt(strangerOn.dataset.value, 10) === 1 : prefs.open1to1Stranger;
			prefs.open1to1Friend = friendOn ? parseInt(friendOn.dataset.value, 10) === 1 : prefs.open1to1Friend;
			prefs.alarm1to1 = alarmOn ? parseInt(alarmOn.dataset.value, 10) === 1 : prefs.alarm1to1;
			prefs.save();

			event.stopImmediatePropagation();
			event.preventDefault();
		});
	});

	// Radio button toggle logic (non-setting-row)
	root.addEventListener('mousedown', event => {
		const off = event.target.closest('.off');
		if (!off) return;

		// Setting-row buttons are managed by their own handler
		if (off.parentNode.classList.contains('setting-row')) return;

		// Check if content is disabled
		const contentEl = root.querySelector('.content');
		if (contentEl && contentEl.classList.contains('disabled')) return;

		const on = off.parentNode.querySelector('.on');
		if (!on) return;

		on.className = 'off';
		off.className = 'on';

		const tmp = on.style.backgroundImage;
		on.style.backgroundImage = off.style.backgroundImage;
		off.style.backgroundImage = tmp;
	});

	// Block interaction on 'on' buttons (non-setting-row)
	root.addEventListener(
		'mousedown',
		event => {
			const on = event.target.closest('.on');
			if (!on) return;
			if (on.parentNode.classList.contains('setting-row')) return;

			event.stopImmediatePropagation();
			event.preventDefault();
		},
		true
	);

	this.draggable('.titlebar');
};

/**
 * Key handler — block shortcuts when input is focused
 */
PartyHelper.onKeyDown = function onKeyDown(event) {
	const shadow = this._shadow || this._host;
	const focused = shadow ? shadow.activeElement : null;

	if (focused && focused.tagName && focused.tagName.match(/input|select|textarea/i)) {
		if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
			PartyHelper.remove();
			event.stopImmediatePropagation();
			return false;
		}
		if (event.which === KEYS.ENTER || event.key === 'Enter') {
			// Let the nameInput keydown handler deal with Enter
			return true;
		}
		event.stopImmediatePropagation();
		return true;
	}

	if (event.which === KEYS.ESCAPE || event.key === 'Escape') {
		PartyHelper.remove();
		event.stopImmediatePropagation();
		return false;
	}

	return true;
};

/**
 * Position UI relative to PartyFriends window
 */
PartyHelper.onAppend = function onAppend() {
	const base = UIManager.getComponent('PartyFriends');
	const root = _root();

	const partyContent = root.querySelector('.party-content');
	const friendContent = root.querySelector('.friend-content');
	if (partyContent) partyContent.style.display = 'none';
	if (friendContent) friendContent.style.display = 'none';

	const nameInput = root.querySelector('.name');
	if (nameInput) nameInput.value = '';

	if (base && base._host) {
		this._host.style.top = base._host.style.top;
		this._host.style.left = `${parseInt(base._host.style.left, 10) + (base._host.offsetWidth || 0) + 10}px`;
	}

	window.addEventListener('keydown', this._onKeyDown, true);
};

/**
 * Cleanup on window removal
 */
PartyHelper.onRemove = function onRemove() {
	const root = _root();

	const partyContent = root.querySelector('.party-content');
	const friendContent = root.querySelector('.friend-content');
	if (partyContent) partyContent.style.display = 'none';
	if (friendContent) friendContent.style.display = 'none';

	const nameInput = root.querySelector('.name');
	if (nameInput) nameInput.value = '';

	window.removeEventListener('keydown', this._onKeyDown, true);
};

/**
 * Set window display mode
 * @param {number} type
 */
PartyHelper.setType = function setType(type) {
	const root = _root();

	const contentEls = root.querySelectorAll('.content');
	contentEls.forEach(el => el.classList.remove('disabled'));

	const footer = root.querySelector('.footer');
	if (footer) footer.style.display = 'block';

	// Restrict Friend Setup by packet version
	if (type === PartyHelper.Type.FRIEND_SETUP && PACKETVER.value < 20090617) {
		type = PartyHelper.Type.SETUP;
	}

	const innerRoot = root.querySelector('#PartyHelper');
	const friendContent = root.querySelector('.friend-content');
	const partyContent = root.querySelector('.party-content');
	const footerBtns = root.querySelectorAll('.footer .btn');

	// Helper to show/hide elements by selector
	const show = sel =>
		root.querySelectorAll(sel).forEach(el => {
			el.style.display = '';
		});
	const hide = sel =>
		root.querySelectorAll(sel).forEach(el => {
			el.style.display = 'none';
		});

	switch (type) {
		case PartyHelper.Type.CREATE:
			if (innerRoot) innerRoot.style.width = '130px';
			if (friendContent) friendContent.style.display = 'none';
			if (partyContent) partyContent.style.display = 'block';
			hide('.setup');
			hide('.invite');
			show('.create');
			hide('.titlebar .setup');
			hide('.titlebar .invite');
			hide('.titlebar .friend-setup');
			show('.titlebar .create');
			if (footer) footer.style.height = '27px';
			footerBtns.forEach(el => {
				el.style.display = '';
			});
			break;

		case PartyHelper.Type.INVITE:
			if (innerRoot) innerRoot.style.width = '130px';
			if (friendContent) friendContent.style.display = 'none';
			if (partyContent) partyContent.style.display = 'block';
			hide('.setup');
			hide('.create');
			show('.invite');
			hide('.titlebar .setup');
			hide('.titlebar .create');
			hide('.titlebar .friend-setup');
			show('.titlebar .invite');
			if (footer) footer.style.height = '27px';
			footerBtns.forEach(el => {
				el.style.display = '';
			});
			break;

		case PartyHelper.Type.SETUP:
			if (innerRoot) innerRoot.style.width = '130px';
			if (friendContent) friendContent.style.display = 'none';
			if (partyContent) partyContent.style.display = 'block';
			hide('.create');
			hide('.invite');
			show('.setup');
			hide('.titlebar .create');
			hide('.titlebar .invite');
			hide('.titlebar .friend-setup');
			show('.titlebar .setup');
			if (footer) footer.style.height = '27px';
			footerBtns.forEach(el => {
				el.style.display = '';
			});
			break;

		case PartyHelper.Type.FRIEND_SETUP:
			if (innerRoot) innerRoot.style.width = '230px';
			if (partyContent) partyContent.style.display = 'none';
			if (friendContent) friendContent.style.display = 'block';
			hide('.titlebar .create');
			hide('.titlebar .invite');
			hide('.titlebar .setup');
			show('.titlebar .friend-setup');
			if (footer) footer.style.height = '20px';
			footerBtns.forEach(el => {
				el.style.display = 'none';
			});
			break;
	}

	_type = type;
};

/**
 * Update party settings UI
 * @param {object} options
 * @param {boolean} editable
 */
PartyHelper.setOptions = function setOptions(options, editable) {
	const root = _root();

	function swap(off) {
		const on = off.parentNode.querySelector('.on');
		const tmp = on.style.backgroundImage;

		on.className = 'off';
		off.className = 'on';

		on.style.backgroundImage = off.style.backgroundImage;
		off.style.backgroundImage = tmp;
	}

	const list = ['exp_share', 'item_share', 'item_sharing_type'];
	const count = list.length;

	for (let i = 0; i < count; ++i) {
		if (options[list[i]] === undefined) {
			continue;
		}

		const container = root.querySelector(`.${list[i]}`);
		if (!container) continue;
		const element = container.querySelector('.off');
		if (element && options[list[i]] == element.dataset.value) {
			swap(element);
		}
	}

	const contentEls = root.querySelectorAll('.content');
	if (!editable) {
		contentEls.forEach(el => el.classList.add('disabled'));
	} else {
		contentEls.forEach(el => el.classList.remove('disabled'));
	}
};

/**
 * Update friend/whisper settings UI
 * @param {object} options
 */
PartyHelper.setFriendOptions = function setFriendOptions(options) {
	const root = _root();

	function swap(off) {
		const on = off.parentNode.querySelector('.on');
		on.className = 'off';
		off.className = 'on';
	}

	const list = ['open1to1Stranger', 'open1to1Friend', 'alarm1to1'];
	const count = list.length;

	for (let i = 0; i < count; ++i) {
		const value = options[list[i]] === true || options[list[i]] == 1;
		const row = root.querySelector(`.${list[i]}`);
		if (!row) continue;
		const on = row.querySelector('.on');
		const off = row.querySelector('.off');

		if (on && off && value !== (on.dataset.value == 1)) {
			swap(off);
		}
	}
};

/**
 * Get current window type
 * @returns {number}
 */
PartyHelper.getType = function getType() {
	return _type;
};

/**
 * Validate and process form data
 */
function onValidate() {
	const root = _root();
	const PartyFriends = UIManager.getComponent('PartyFriends');

	switch (_type) {
		case PartyHelper.Type.CREATE: {
			const nameInput = root.querySelector('.content .name');
			const name = nameInput ? nameInput.value : '';
			if (name.length) {
				const itemShareOn = root.querySelector('.item_share .on');
				const itemSharingOn = root.querySelector('.item_sharing_type .on');
				PartyFriends.onRequestPartyCreation(
					name,
					itemShareOn ? parseInt(itemShareOn.dataset.value, 10) : 0,
					itemSharingOn ? parseInt(itemSharingOn.dataset.value, 10) : 0
				);
				PartyHelper.remove();
			}
			break;
		}

		case PartyHelper.Type.INVITE: {
			const nameInput = root.querySelector('.content .name');
			const name = nameInput ? nameInput.value : '';
			if (name.length) {
				PartyFriends.onRequestAddingMember(0, name);
			}
			break;
		}

		case PartyHelper.Type.SETUP: {
			const expShareOn = root.querySelector('.exp_share .on');
			const itemShareOn = root.querySelector('.item_share .on');
			const itemSharingOn = root.querySelector('.item_sharing_type .on');
			PartyFriends.onRequestSettingUpdate(
				expShareOn ? parseInt(expShareOn.dataset.value, 10) : 0,
				itemShareOn ? parseInt(itemShareOn.dataset.value, 10) : 0,
				itemSharingOn ? parseInt(itemSharingOn.dataset.value, 10) : 0
			);
			PartyHelper.remove();
			break;
		}
	}
}

/**
 * Hooks
 */
PartyHelper.onCreate = function onCreate() {};
PartyHelper.onInvite = function onInvite() {};
PartyHelper.onSetupUpdate = function onSetUpUpdate() {};

PartyHelper.mouseMode = GUIComponent.MouseMode.STOP;

/**
 * Export component
 */
export default UIManager.addComponent(PartyHelper);
