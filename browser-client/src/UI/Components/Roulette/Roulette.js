/**
 * UI/Components/Roulette/Roulette.js
 *
 * Roulette window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author [Your Name]
 */

import DB from 'DB/DBManager.js';
import Client from 'Core/Client.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import htmlText from './Roulette.html?raw';
import cssText from './Roulette.css?raw';

/**
 * Create Component
 */
const Roulette = new GUIComponent('Roulette', cssText);

/**
 * Render HTML
 */
Roulette.render = () => htmlText;

/**
 * @var {Preferences} structure
 */
const _preferences = Preferences.get(
	'Roulette',
	{
		x: 200,
		y: 200,
		show: false
	},
	1.0
);

/**
 * @var {object} Roulette data
 */
const _rouletteInfo = {
	step: 0,
	idx: 0,
	goldPoint: 0,
	silverPoint: 0,
	bronzePoint: 0,
	additionItemID: 0,
	items: []
};

/**
 * @var {boolean} Is spinning
 */
let _isSpinning = false;

/**
 * Initialize UI
 */
Roulette.init = function init() {
	const root = this.getRoot();

	root.querySelector('.close').addEventListener('click', () => {
		Roulette.onClose();
	});

	root.querySelector('.btn-close').addEventListener('click', () => {
		Roulette.onClose();
	});

	root.querySelector('.btn-spin').addEventListener('click', () => {
		if (!_isSpinning) {
			Roulette.onSpin();
		}
	});

	root.querySelector('.btn-info').addEventListener('click', () => {
		Roulette.onInfo();
	});

	this.draggable('.titlebar');
};

/**
 * @var {number} Timeout ID for server response
 */
let _responseTimeout = null;

/**
 * Click on roulette icon
 */
function onClickIcon() {
	if (Roulette._host.style.display !== 'none') {
		Roulette.onClose();
		return;
	}

	const pkt = new PACKET.CZ.REQ_OPEN_ROULETTE();
	Network.sendPacket(pkt);

	if (_responseTimeout) {
		clearTimeout(_responseTimeout);
	}

	_responseTimeout = setTimeout(() => {
		_responseTimeout = null;
	}, 2000);
}

/**
 * Once append to the DOM
 */
Roulette.onAppend = function onAppend() {
	this._host.style.top = Math.min(Math.max(0, _preferences.y), Renderer.height - this._host.offsetHeight) + 'px';
	this._host.style.left = Math.min(Math.max(0, _preferences.x), Renderer.width - this._host.offsetWidth) + 'px';

	if (!_preferences.show) {
		this._host.style.display = 'none';
	}

	if (ROConfig.enableRoulette === false) {
		return;
	}

	if (PACKETVER.value < 20141008) {
		return;
	}

	addRouletteIcon();
};

/**
 * @var {HTMLButtonElement|null} Standalone roulette icon (light DOM)
 */
let _iconBtn = null;

/**
 * Create the roulette icon as a standalone light-DOM button.
 */
function addRouletteIcon() {
	if (_iconBtn) {
		return;
	}
	_iconBtn = document.createElement('button');
	_iconBtn.className = 'rouletteIcon';
	Object.assign(_iconBtn.style, {
		position: 'absolute',
		top: '74px',
		right: '145px',
		width: '43px',
		height: '43px',
		border: 'none',
		backgroundColor: 'transparent',
		backgroundRepeat: 'no-repeat',
		backgroundSize: 'contain',
		backgroundPosition: 'center',
		zIndex: '50',
		cursor: 'pointer'
	});
	_iconBtn.addEventListener('mousedown', e => e.stopImmediatePropagation());
	_iconBtn.addEventListener('click', onClickIcon);
	document.body.appendChild(_iconBtn);
	const iconPath = 'basic_interface/roullette/RoulletteIcon.bmp';
	Client.loadFile(DB.INTERFACE_PATH + iconPath, function (data) {
		if (_iconBtn) {
			_iconBtn.style.backgroundImage = `url(${data})`;
		}
	});
}

/**
 * Remove from DOM
 */
Roulette.onRemove = function onRemove() {
	if (_iconBtn) {
		_iconBtn.remove();
		_iconBtn = null;
	}
	_preferences.show = this._host.style.display !== 'none';
	_preferences.x = parseInt(this._host.style.left, 10);
	_preferences.y = parseInt(this._host.style.top, 10);
	_preferences.save();
};

/**
 * Open Roulette Window
 */
Roulette.onOpen = function onOpen(pkt) {
	if (pkt) {
		_rouletteInfo.step = pkt.step || 0;
		_rouletteInfo.idx = pkt.idx || 0;
		_rouletteInfo.goldPoint = pkt.goldPoint || 0;
		_rouletteInfo.silverPoint = pkt.silverPoint || 0;
		_rouletteInfo.bronzePoint = pkt.bronzePoint || 0;
		_rouletteInfo.additionItemID = pkt.additionItemID || 0;
	}

	this._host.style.display = '';
	this.updateUI();
	this.focus();
};

/**
 * Close Roulette Window
 */
Roulette.onClose = function onClose() {
	Roulette._host.style.display = 'none';

	const pkt = new PACKET.CZ.REQ_CLOSE_ROULETTE();
	Network.sendPacket(pkt);
};

/**
 * Request to spin the roulette
 */
Roulette.onSpin = function onSpin() {
	if (_isSpinning) {
		return;
	}

	const pkt = new PACKET.CZ.REQ_GENERATE_ROULETTE();
	Network.sendPacket(pkt);
};

/**
 * Handle roulette item list response
 */
Roulette.onRouletteInfo = function onRouletteInfo(pkt) {
	if (pkt && pkt.items) {
		_rouletteInfo.items = pkt.items;
		_rouletteInfo.serial = pkt.serial;
	}

	Roulette.generateWheelSlots();
};

/**
 * Show info window
 */
Roulette.onInfo = function onInfo() {
	// TODO: Show information about roulette
};

/**
 * Update UI with current roulette data
 */
Roulette.updateUI = function updateUI() {
	const root = this.getRoot();

	this.generateWheelSlots();

	const btnSpin = root.querySelector('.btn-spin');
	if (btnSpin) {
		btnSpin.disabled = _isSpinning;
	}
};

/**
 * Generate wheel slots from items
 */
Roulette.generateWheelSlots = function generateWheelSlots() {
	const root = this.getRoot();
	const wheelSlots = root.querySelector('.wheel-slots');
	if (!wheelSlots) {
		return;
	}
	wheelSlots.innerHTML = '';

	const items = _rouletteInfo.items || [];
	const numSlots = items.length || 10;

	for (let i = 0; i < numSlots; i++) {
		const angle = (360 / numSlots) * i;
		const distance = 120;
		const radian = (angle - 90) * (Math.PI / 180);

		const x = distance * Math.cos(radian);
		const y = distance * Math.sin(radian);

		const slot = document.createElement('div');
		slot.className = 'wheel-slot';
		slot.setAttribute('data-index', i);
		Object.assign(slot.style, {
			transform: `translate(${x}px, ${y}px) rotate(${angle}deg)`
		});

		if (items[i]) {
			const itemId = items[i].itemId || items[i].item_id || items[i].ItemID;
			if (itemId) {
				Client.loadFile(DB.INTERFACE_PATH + `item/${itemId}.bmp`, function (data) {
					const icon = document.createElement('div');
					icon.className = 'item-icon';
					icon.style.backgroundImage = `url(${data})`;
					slot.appendChild(icon);
				});
			}
		}

		wheelSlots.appendChild(slot);
	}
};

/**
 * Start spinning animation
 */
Roulette.startSpin = function startSpin(resultIndex) {
	if (_isSpinning) {
		return;
	}

	_isSpinning = true;
	const root = this.getRoot();
	const btnSpin = root.querySelector('.btn-spin');
	if (btnSpin) {
		btnSpin.disabled = true;
	}

	const wheelSlots = root.querySelector('.wheel-slots');
	if (!wheelSlots) {
		_isSpinning = false;
		if (btnSpin) {
			btnSpin.disabled = false;
		}
		return;
	}
	const numSlots = _rouletteInfo.items.length || 10;
	const degreesPerSlot = 360 / numSlots;

	const targetRotation = 360 * 5 + resultIndex * degreesPerSlot;

	wheelSlots.style.transform = `translate(-50%, -50%) rotate(${targetRotation}deg)`;

	setTimeout(() => {
		_isSpinning = false;
		const r = Roulette.getRoot();
		const btn = r.querySelector('.btn-spin');
		if (btn) {
			btn.disabled = false;
		}
		Roulette.showResult(resultIndex);
	}, 3000);
};

/**
 * Show result after spin
 */
Roulette.showResult = function showResult(resultIndex) {
	const root = this.getRoot();
	const resultDisplay = root.querySelector('.result-item');
	if (!resultDisplay) {
		return;
	}
	resultDisplay.innerHTML = '';

	if (_rouletteInfo.items[resultIndex]) {
		const item = _rouletteInfo.items[resultIndex];
		const itemId = item.itemId || item.item_id || item.ItemID;

		if (itemId) {
			Client.loadFile(DB.INTERFACE_PATH + `item/${itemId}.bmp`, function (data) {
				resultDisplay.style.backgroundImage = `url(${data})`;
			});
		}
	}
};

/**
 * Handle roulette result from server
 */
Roulette.onResult = function onResult(pkt) {
	if (pkt.step !== undefined) {
		_rouletteInfo.step = pkt.step;
	}

	if (pkt.idx !== undefined) {
		_rouletteInfo.idx = pkt.idx;
	}

	if (pkt.remainGold !== undefined) {
		_rouletteInfo.goldPoint = pkt.remainGold;
	}
	if (pkt.remainSilver !== undefined) {
		_rouletteInfo.silverPoint = pkt.remainSilver;
	}
	if (pkt.remainBronze !== undefined) {
		_rouletteInfo.bronzePoint = pkt.remainBronze;
	}

	if (pkt.idx !== undefined) {
		Roulette.startSpin(pkt.idx);
	}
};

/**
 * Handle item received from server
 */
Roulette.onItemReceived = function onItemReceived(_pkt) {
	Roulette.updateUI();
};

/**
 * Request to receive the roulette item
 */
Roulette.requestReceiveItem = function requestReceiveItem(condition) {
	const pkt = new PACKET.CZ.RECV_ROULETTE_ITEM();
	pkt.condition = condition || 0;
	Network.sendPacket(pkt);
};

/**
 * Export
 */
export default UIManager.addComponent(Roulette);
