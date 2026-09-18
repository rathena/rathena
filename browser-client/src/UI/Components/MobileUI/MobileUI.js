/**
 * UI/Components/MobileUI/MobileUI.js
 *
 * Mobile/Touchscreen assist UI
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
+ */

import Context from 'Core/Context.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Preferences from 'Core/Preferences.js';
import Session from 'Engine/SessionStorage.js';
import Renderer from 'Renderer/Renderer.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import EntityManager from 'Renderer/EntityManager.js';
import Network from 'Network/NetworkManager.js';
import PathFinding from 'Utils/PathFinding.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Events from 'Core/Events.js';
import htmlText from './MobileUI.html?raw';
import cssText from './MobileUI.css?raw';
import glMatrix from 'Vendors/gl-matrix.js';
import Camera from 'Renderer/Camera.js';
import BattleMode from 'Controls/BattleMode.js';

const vec2 = glMatrix.vec2;
const mat2 = glMatrix.mat2;

// Object to initialize
const direction = vec2.create();
const rotate = mat2.create();

//Memory
const targetPos = [0, 0];

let movementTimer = null; // Timer for continuous joystick movement

/**
 * Create Component
 */
const MobileUI = new GUIComponent('MobileUI', cssText);

MobileUI.render = () => htmlText;

/**
 * @var {Preferences} window preferences
 */
const _preferences = Preferences.get(
	'MobileUI',
	{
		x: 0,
		y: 0,
		zIndex: 1000,
		width: window.innerWidth,
		height: window.innerHeight,
		show: false
	},
	1.0
);

let showButtons = false;
let autoTargetTimer;
const C_AUTOTARGET_DELAY = 500;
const C_TOUCH_CLICK_GUARD = 750;

let centerX, centerY;
let maxDistance = 0;
let normalizedX = 0;
let normalizedY = 0;

// Joystick element references (captured in setupJoystick)
let _joystickBase = null;
let _joystickThumb = null;

/**
 * Helper to bind click+touchstart on an element
 */
function bindButton(root, selector, handler) {
	const el = root.querySelector(selector);
	if (el) {
		let touchHandled = false;
		let releaseTimer = null;

		const clearGuard = () => {
			if (releaseTimer !== null) {
				clearTimeout(releaseTimer);
				releaseTimer = null;
			}
		};

		const releaseGuard = () => {
			clearGuard();
			releaseTimer = setTimeout(() => {
				releaseTimer = null;
				touchHandled = false;
			}, C_TOUCH_CLICK_GUARD);
		};

		el.addEventListener('click', event => {
			if (touchHandled) {
				touchHandled = false;
				clearGuard();
				event.preventDefault();
				event.stopImmediatePropagation();
				return;
			}
			handler(event);
		});
		el.addEventListener('touchstart', event => {
			touchHandled = true;
			clearGuard();
			handler(event);
		});
		el.addEventListener('touchend', releaseGuard);
		el.addEventListener('touchcancel', releaseGuard);
	}
}

/**
 * Initialize UI
 */
MobileUI.init = function init() {
	const root = MobileUI.getRoot();

	bindButton(root, '#toggleUIButton', e => {
		toggleButtons();
		stopPropagation(e);
	});
	bindButton(root, '#fullscreenButton', e => {
		toggleFullScreen();
		stopPropagation(e);
	});

	// F-key buttons
	const fKeyMap = [
		['#f1Button', 112],
		['#f2Button', 113],
		['#f3Button', 114],
		['#f4Button', 115],
		['#f5Button', 116],
		['#f6Button', 117],
		['#f7Button', 118],
		['#f8Button', 119],
		['#f9Button', 120]
	];

	// Number key buttons
	const nKeyMap = [
		['#n1Button', 49],
		['#n2Button', 50],
		['#n3Button', 51],
		['#n4Button', 52],
		['#n5Button', 53],
		['#n6Button', 54],
		['#n7Button', 55],
		['#n8Button', 56],
		['#n9Button', 57]
	];

	// Letter key buttons
	const letterKeyMap = [
		['#qButton', 81],
		['#wButton', 87],
		['#eButton', 69],
		['#rButton', 82],
		['#tButton', 84],
		['#yButton', 89],
		['#uButton', 85],
		['#iButton', 73],
		['#oButton', 79],
		['#aButton', 65],
		['#sButton', 83],
		['#dButton', 68],
		['#fButton', 70],
		['#gButton', 71],
		['#hButton', 72],
		['#jButton', 74],
		['#kButton', 75],
		['#lButton', 76]
	];

	[...fKeyMap, ...nKeyMap, ...letterKeyMap].forEach(([selector, keyCode]) => {
		bindButton(root, selector, e => {
			skillKeyPress(keyCode);
			stopPropagation(e);
		});
	});

	bindButton(root, '#f10Button', e => {
		logKeyPress(121);
		stopPropagation(e);
	});
	bindButton(root, '#f12Button', e => {
		logKeyPress(123);
		stopPropagation(e);
	});
	bindButton(root, '#insButton', e => {
		logKeyPress(45);
		stopPropagation(e);
	});

	bindButton(root, '#toggleStatusButton', e => {
		toggleStatus();
		stopPropagation(e);
	});
	bindButton(root, '#toggleTargetingButton', e => {
		toggleTouchTargeting();
		stopPropagation(e);
	});
	bindButton(root, '#toggleAutoFollowButton', e => {
		toggleAutoFollow();
		stopPropagation(e);
	});
	bindButton(root, '#toggleAutoTargetButton', e => {
		toggleAutoTargeting();
		stopPropagation(e);
	});

	bindButton(root, '#attackButton', e => {
		attackTargeted();
		stopPropagation(e);
	});

	bindButton(root, '#pickupButton', e => {
		pickUpItem();
		stopPropagation(e);
	});

	bindButton(root, '#switchshorcutButton', e => {
		switchSkillButtons();
		stopPropagation(e);
	});

	// Press effect for .buttons and .FButton
	root.querySelectorAll('.buttons').forEach(btn => {
		btn.addEventListener('mousedown', e => e.target.classList.add('pressed'));
		btn.addEventListener('touchstart', e => e.target.classList.add('pressed'));
		btn.addEventListener('mouseup', e => e.target.classList.remove('pressed'));
		btn.addEventListener('touchend', e => e.target.classList.remove('pressed'));
	});

	root.querySelectorAll('.FButton').forEach(btn => {
		btn.addEventListener('mousedown', e => e.target.classList.add('pressed'));
		btn.addEventListener('touchstart', e => e.target.classList.add('pressed'));
		btn.addEventListener('mouseup', e => e.target.classList.remove('pressed'));
		btn.addEventListener('touchend', e => e.target.classList.remove('pressed'));
	});

	// Initialize the joystick - MicromeX
	setupJoystick();
	// Initialize the NPC Talk Button - MicromeX
	setupTalkToNpcButton();
};

/**
 * Logs the key press to the console and performs the key press action.
 * @param {number} keyCode - The key code of the pressed key.
 */
function logKeyPress(keyCode) {
	keyPress(keyCode);
}

/**
 * Skill bar button: run the hotkey directly, the number/letter rows are only
 * hotkeys for the keyboard while Battle Mode is on.
 *
 * @param {number} keyCode
 */
function skillKeyPress(keyCode) {
	if (!BattleMode.process(keyCode)) {
		keyPress(keyCode);
	}
}

/**
 * Toggles full screen display
 */
function toggleFullScreen() {
	if (!Context.isFullScreen()) {
		Context.requestFullScreen();
	} else {
		Context.cancelFullScreen();
	}
}

/**
 * Emulates a keypress event
 *
 * @param {number} keyId
 */
function keyPress(k) {
	const roWindow = window;
	roWindow.document.getElementsByTagName('body')[0].focus();
	roWindow.dispatchEvent(
		new KeyboardEvent('keydown', {
			keyCode: k,
			which: k
		})
	);
}

/**
 * Toggles MobileUI button bars visibility (and thus buttons)
 */
function toggleButtons() {
	const root = MobileUI.getRoot();

	if (showButtons) {
		// Hide all bars and buttons
		[
			'#topBar',
			'#leftBar',
			'#rightBar',
			'#joystickContainer',
			'#buttonContainer',
			'#attackButton',
			'#pickupButton',
			'#talktonpcButton',
			'#switchshorcutButton'
		].forEach(sel => {
			const el = root.querySelector(sel);
			if (el) {
				el.classList.add('disabled');
			}
		});

		// Hide F-key buttons
		for (let i = 1; i <= 9; i++) {
			const fBtn = root.querySelector(`#f${i}Button`);
			if (fBtn) {
				fBtn.classList.add('disabled');
			}
		}

		// Hide number, letter buttons
		['n', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'].forEach(key => {
			const btn =
				root.querySelector(`#${key}Button`) || root.querySelector(`#${key}${key === 'n' ? '' : 'B'}utton`);
			if (btn) {
				btn.classList.add('disabled');
			}
		});

		// Hide number buttons specifically
		for (let i = 1; i <= 9; i++) {
			const nBtn = root.querySelector(`#n${i}Button`);
			if (nBtn) {
				nBtn.classList.add('disabled');
			}
		}

		// Hide letter buttons
		['q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l'].forEach(key => {
			const btn = root.querySelector(`#${key}Button`);
			if (btn) {
				btn.classList.add('disabled');
			}
		});

		if (Session.TouchTargeting) {
			toggleTouchTargeting();
		}

		showButtons = false;
	} else {
		[
			'#topBar',
			'#leftBar',
			'#rightBar',
			'#joystickContainer',
			'#buttonContainer',
			'#attackButton',
			'#pickupButton',
			'#talktonpcButton',
			'#switchshorcutButton'
		].forEach(sel => {
			const el = root.querySelector(sel);
			if (el) {
				el.classList.remove('disabled');
			}
		});

		for (let i = 1; i <= 9; i++) {
			const fBtn = root.querySelector(`#f${i}Button`);
			if (fBtn) {
				fBtn.classList.remove('disabled');
			}
		}

		showButtons = true;
	}
}

/**
 * Toggles switch skill
 */
function switchSkillButtons() {
	const root = MobileUI.getRoot();

	const skillSets = [
		[
			'#f1Button',
			'#f2Button',
			'#f3Button',
			'#f4Button',
			'#f5Button',
			'#f6Button',
			'#f7Button',
			'#f8Button',
			'#f9Button'
		],
		[
			'#n1Button',
			'#n2Button',
			'#n3Button',
			'#n4Button',
			'#n5Button',
			'#n6Button',
			'#n7Button',
			'#n8Button',
			'#n9Button'
		],
		['#qButton', '#wButton', '#eButton', '#rButton', '#tButton', '#yButton', '#uButton', '#iButton', '#oButton'],
		['#aButton', '#sButton', '#dButton', '#fButton', '#gButton', '#hButton', '#jButton', '#kButton', '#lButton']
	];

	const currentSetIndex = switchSkillButtons.currentSetIndex || 0;
	const nextSetIndex = (currentSetIndex + 1) % skillSets.length;

	// Hide all skill sets
	skillSets.flat().forEach(selector => {
		const el = root.querySelector(selector);
		if (el) {
			el.classList.add('disabled');
		}
	});

	// Show only the next set
	skillSets[nextSetIndex].forEach(selector => {
		const el = root.querySelector(selector);
		if (el) {
			el.classList.remove('disabled');
		}
	});

	switchSkillButtons.currentSetIndex = nextSetIndex;
}

/**
 * Toggles status view
 */
function toggleStatus() {
	// StatusIcons is a separate component outside this shadow DOM
	const statusIcons = document.querySelector('#StatusIcons');
	if (statusIcons) {
		statusIcons.style.display = statusIcons.style.display === 'none' ? '' : 'none';
	}
}

/**
 * Toggles touch targeting
 */
function toggleTouchTargeting() {
	const root = MobileUI.getRoot();

	if (Session.TouchTargeting) {
		root.querySelector('#toggleTargetingButton').classList.remove('active');
		root.querySelector('#toggleAutoFollowButton').classList.add('disabled');
		root.querySelector('#toggleAutoTargetButton').classList.add('disabled');

		if (Session.AutoTargeting) {
			toggleAutoTargeting();
		}

		Session.TouchTargeting = false;
	} else {
		root.querySelector('#toggleTargetingButton').classList.add('active');
		root.querySelector('#toggleAutoFollowButton').classList.remove('disabled');
		root.querySelector('#toggleAutoTargetButton').classList.remove('disabled');

		Session.TouchTargeting = true;
	}
}

/**
 * Toggles automatic targeting
 */
function toggleAutoTargeting() {
	const root = MobileUI.getRoot();

	if (Session.AutoTargeting) {
		root.querySelector('#toggleAutoTargetButton').classList.remove('active');
		Session.AutoTargeting = false;
	} else {
		root.querySelector('#toggleAutoTargetButton').classList.add('active');
		Session.AutoTargeting = true;
		autoTarget();
	}
}

/**
 * Toggles auto follow
 */
function toggleAutoFollow() {
	const root = MobileUI.getRoot();

	if (Session.autoFollow) {
		root.querySelector('#toggleAutoFollowButton').classList.remove('active');
		Session.autoFollow = false;
	} else {
		const entityFocus = EntityManager.getFocusEntity();
		if (entityFocus) {
			root.querySelector('#toggleAutoFollowButton').classList.add('active');
			Session.autoFollow = true;
			Session.autoFollowTarget = entityFocus;
			onAutoFollow();
		}
	}
}

/**
 * Attacks a targeted enemy (if present)
 */
function attackTargeted() {
	const main = Session.Entity;
	let pkt;

	let entityFocus = EntityManager.getFocusEntity();

	if (!entityFocus || entityFocus.action === entityFocus.ACTION.DIE) {
		autoTarget();
		entityFocus = EntityManager.getFocusEntity();
	}

	if (entityFocus) {
		const out = [];
		const count = PathFinding.search(
			main.position[0] | 0,
			main.position[1] | 0,
			entityFocus.position[0] | 0,
			entityFocus.position[1] | 0,
			main.attack_range + 1,
			out
		);

		if (!count) {
			return true;
		}

		if (PACKETVER.value >= 20180307) {
			pkt = new PACKET.CZ.REQUEST_ACT2();
		} else {
			pkt = new PACKET.CZ.REQUEST_ACT();
		}
		pkt.action = 7;
		pkt.targetGID = entityFocus.GID;

		if (count < 2) {
			Network.sendPacket(pkt);
			return true;
		}

		Session.moveAction = pkt;

		if (PACKETVER.value >= 20180307) {
			pkt = new PACKET.CZ.REQUEST_MOVE2();
		} else {
			pkt = new PACKET.CZ.REQUEST_MOVE();
		}
		pkt.dest[0] = out[(count - 1) * 2 + 0];
		pkt.dest[1] = out[(count - 1) * 2 + 1];
		Network.sendPacket(pkt);
	}
}

/**
 * Automatically targeting the closest enemy
 */
function autoTarget() {
	const Player = Session.Entity;

	const entityFocus = EntityManager.getFocusEntity();

	const closestEntity = EntityManager.getClosestEntity(Player, Session.Entity.constructor.TYPE_MOB);

	if (closestEntity) {
		if (entityFocus && closestEntity.GID !== entityFocus.GID) {
			entityFocus.onFocusEnd();
			EntityManager.setFocusEntity(null);

			closestEntity.onFocus();
			EntityManager.setFocusEntity(closestEntity);
		} else if (!entityFocus) {
			closestEntity.onFocus();
			EntityManager.setFocusEntity(closestEntity);
		}
	}

	if (Session.AutoTargeting && Session.Playing) {
		startAutoTarget();
	}
}

/**
 * Starting automatic targeting cycle
 */
function startAutoTarget() {
	autoTargetTimer = window.setTimeout(autoTarget, C_AUTOTARGET_DELAY);
}

/**
 * Stopping automatic targeting cycle
 * Currently unused, preserved for future development
 */
function _stopAutoTarget() {
	window.clearTimeout(autoTargetTimer);
}

/**
 * Stop event propagation
 */
function stopPropagation(event) {
	if (event && typeof event.preventDefault === 'function') {
		event.preventDefault();
	}
	event.stopImmediatePropagation();
	return false;
}

/**
 * Auto follow logic
 */
function onAutoFollow() {
	const root = MobileUI.getRoot();

	if (Session.autoFollow) {
		const player = Session.Entity;
		const target = Session.autoFollowTarget;

		const dx = Math.abs(player.position[0] - target.position[0]);
		const dy = Math.abs(player.position[1] - target.position[1]);

		if (dx > 1 || dy > 1) {
			const dest = [0, 0];

			if (checkFreeCell(Math.round(target.position[0]), Math.round(target.position[1]), 1, dest)) {
				let pkt;
				if (PACKETVER.value >= 20180307) {
					pkt = new PACKET.CZ.REQUEST_MOVE2();
				} else {
					pkt = new PACKET.CZ.REQUEST_MOVE();
				}
				pkt.dest = dest;
				Network.sendPacket(pkt);
			}
		}

		Events.setTimeout(onAutoFollow, 500);
	} else {
		root.querySelector('#toggleAutoFollowButton').classList.remove('active');
	}
}

/**
 * Picks up the nearest item - MicromeX
 */
function pickUpItem() {
	const player = Session.Entity;

	if (!player) {
		return;
	}

	const closestItem = EntityManager.getClosestEntity(player, Session.Entity.constructor.TYPE_ITEM);

	if (!closestItem) {
		return;
	}

	let dx = Math.abs(player.position[0] - closestItem.position[0]);
	let dy = Math.abs(player.position[1] - closestItem.position[1]);
	if (dx < 0) {
		dx = -dx;
	}
	if (dy < 0) {
		dy = -dy;
	}

	if ((dx < dy ? dy : dx) > 2) {
		const dest = [0, 0];

		if (checkFreeCell(Math.round(closestItem.position[0]), Math.round(closestItem.position[1]), 1, dest)) {
			let pkt;
			if (PACKETVER.value >= 20180307) {
				pkt = new PACKET.CZ.REQUEST_MOVE2();
			} else {
				pkt = new PACKET.CZ.REQUEST_MOVE();
			}
			pkt.dest = dest;
			Network.sendPacket(pkt);
		}
	}

	let pickUpPacket;

	if (PACKETVER.value >= 20180307) {
		pickUpPacket = new PACKET.CZ.ITEM_PICKUP2();
	} else {
		pickUpPacket = new PACKET.CZ.ITEM_PICKUP();
	}

	pickUpPacket.ITAID = closestItem.GID;

	Network.sendPacket(pickUpPacket);
}

/**
 * Joystick handling for both mouse and touch input - MicromeX
 */
function setupJoystick() {
	const root = MobileUI.getRoot();
	_joystickBase = root.querySelector('#joystickBase');
	_joystickThumb = root.querySelector('#joystickThumb');

	maxDistance = _joystickBase.offsetWidth / 2;

	_joystickThumb.addEventListener('mousedown', startDrag);
	_joystickThumb.addEventListener('touchstart', startDrag);
}

function startDrag(event) {
	event.preventDefault();

	const touch = event.touches ? event.touches[0] : event;

	const rect = _joystickBase.getBoundingClientRect();
	centerX = rect.left + rect.width / 2;
	centerY = rect.top + rect.height / 2;

	document.addEventListener('mousemove', moveJoystick);
	document.addEventListener('mouseup', stopDrag);
	document.addEventListener('touchmove', moveJoystick);
	document.addEventListener('touchend', stopDrag);

	moveJoystick(touch);
	startMovement();
}

function moveJoystick(event) {
	const deadZone = 15;

	const touch = event.touches ? event.touches[0] : event;

	const deltaX = touch.clientX - centerX;
	const deltaY = touch.clientY - centerY;

	const distance = Math.min(Math.sqrt(deltaX ** 2 + deltaY ** 2), maxDistance);
	const angle = Math.atan2(deltaY, deltaX);

	const offsetX = Math.cos(angle) * distance;
	const offsetY = Math.sin(angle) * distance;

	_joystickThumb.style.transform = `translate(${offsetX}px, ${offsetY}px)`;

	if (distance < deadZone) {
		normalizedX = 0;
		normalizedY = 0;
		return;
	}

	normalizedX = offsetX / maxDistance;
	normalizedY = -offsetY / maxDistance;
}

function stopDrag() {
	_joystickThumb.style.transform = 'translate(0, 0)';
	normalizedX = 0;
	normalizedY = 0;

	stopMovement();

	document.removeEventListener('mousemove', moveJoystick);
	document.removeEventListener('mouseup', stopDrag);
	document.removeEventListener('touchmove', moveJoystick);
	document.removeEventListener('touchend', stopDrag);
}

function startMovement() {
	const tileSize = 3;

	if (movementTimer) {
		clearInterval(movementTimer);
	}

	const executeMove = () => {
		if (normalizedX !== 0 || normalizedY !== 0) {
			moveCharacter(normalizedX, normalizedY, tileSize);
		}
	};

	executeMove();

	movementTimer = setInterval(executeMove, 100);
}

function stopMovement() {
	if (movementTimer) {
		clearInterval(movementTimer);
		movementTimer = null;
	}
}

/**
 * Moves the character to a new tile and waits for the movement to complete.
 * @param {number} x - Normalized x-axis input (-1 to 1)
 * @param {number} y - Normalized y-axis input (-1 to 1)
 * @param {number} tileSize - The size of each tile in the game world
 */
function moveCharacter(x, y, tileSize) {
	const player = Session.Entity;

	if (!player) {
		return;
	}

	direction[0] = x;
	direction[1] = y;

	mat2.identity(rotate);
	mat2.rotate(rotate, rotate, ((-Camera.direction * 45) / 180) * Math.PI);

	vec2.transformMat2(direction, direction, rotate);

	const newPos = [
		Math.round(player.position[0] + direction[0] * tileSize),
		Math.round(player.position[1] + direction[1] * tileSize)
	];

	const dest = [0, 0];

	if (checkFreeCell(newPos[0], newPos[1], 5, dest)) {
		if (targetPos[0] !== dest[0] || targetPos[1] !== dest[1]) {
			targetPos[0] = dest[0];
			targetPos[1] = dest[1];

			let movePacket;
			if (PACKETVER.value >= 20180307) {
				movePacket = new PACKET.CZ.REQUEST_MOVE2();
			} else {
				movePacket = new PACKET.CZ.REQUEST_MOVE();
			}

			movePacket.dest[0] = dest[0];
			movePacket.dest[1] = dest[1];

			Network.sendPacket(movePacket);
		}
	}
}

/**
 * Talk to NPC Button Function - MicromeX
 */
function setupTalkToNpcButton() {
	const root = MobileUI.getRoot();
	const talkButton = root.querySelector('#talktonpcButton');

	function findNearestNpc() {
		const player = Session.Entity;

		if (!player) {
			return null;
		}

		let nearestNpc = null;
		let minDistance = 3;

		EntityManager.forEach(entity => {
			if (entity.objecttype === entity.constructor.TYPE_NPC) {
				const dx = entity.position[0] - player.position[0];
				const dy = entity.position[1] - player.position[1];
				const distance = Math.sqrt(dx ** 2 + dy ** 2);

				if (distance <= minDistance) {
					minDistance = distance;
					nearestNpc = entity;
				}
			}
		});

		return nearestNpc;
	}

	function talkToNearestNpc() {
		const nearestNpc = findNearestNpc();

		if (!nearestNpc) {
			return;
		}

		const talkPacket = new PACKET.CZ.CONTACTNPC();
		talkPacket.NAID = nearestNpc.GID;

		Network.sendPacket(talkPacket);
	}

	talkButton.addEventListener('click', talkToNearestNpc);
}

/**
 * Search free cells around a position
 *
 * @param {number} x
 * @param {number} y
 * @param {number} range
 * @param {array} out
 */
function checkFreeCell(x, y, range, out) {
	let _x, _y, r;
	const d_x = Session.Entity.position[0] < x ? -1 : 1;
	const d_y = Session.Entity.position[1] < y ? -1 : 1;

	for (r = 0; r <= range; ++r) {
		for (_x = -r; _x <= r; ++_x) {
			for (_y = -r; _y <= r; ++_y) {
				if (isFreeCell(x + _x * d_x, y + _y * d_y)) {
					out[0] = x + _x * d_x;
					out[1] = y + _y * d_y;
					return true;
				}
			}
		}
	}

	return false;
}

/**
 * Does a cell is free (walkable, and no entity on)
 *
 * @param {number} x
 * @param {number} y
 * @param {returns} is free
 */
function isFreeCell(x, y) {
	if (!(Altitude.getCellType(x, y) & Altitude.TYPE.WALKABLE)) {
		return false;
	}

	let free = true;

	EntityManager.forEach(entity => {
		if (
			entity.objecttype !== entity.constructor.TYPE_EFFECT &&
			entity.objecttype !== entity.constructor.TYPE_UNIT &&
			entity.objecttype !== entity.constructor.TYPE_TRAP &&
			Math.round(entity.position[0]) === x &&
			Math.round(entity.position[1]) === y
		) {
			free = false;
			return false;
		}

		return true;
	});

	return free;
}

/**
 * Apply preferences once append to body
 */
MobileUI.onAppend = function onAppend() {
	if (Session.isTouchDevice) {
		this._host.style.display = 'block';
	} else {
		this._host.style.display = 'none';
	}

	this._host.style.top = '0px';
	this._host.style.left = '0px';
	this._host.style.zIndex = '1000';
	// Width/height handled by CSS :host { width:100%; height:100% }
	// so the overlay always matches the viewport on resize/rotate.
};

/**
 * Process shortcut
 *
 * @param {object} key
 */
MobileUI.onShortCut = function onShortCut(key) {
	switch (key.cmd) {
		case 'SHOW':
			Session.isTouchDevice = true;
			this.show();
			break;
		case 'TOGGLE':
			toggleButtons();
			break;
		case 'TG':
			toggleTouchTargeting();
			break;
		case 'AT':
			toggleAutoTargeting();
			break;
		case 'ATK':
			attackTargeted();
			break;
	}
};

/**
 * Removes MobileUI
 */
MobileUI.onRemove = function onRemove() {
	_preferences.y = 0;
	_preferences.x = 0;
	_preferences.zIndex = 1000;
	_preferences.width = Renderer.width;
	_preferences.height = Renderer.height;
	_preferences.save();

	if (Session.AutoTargeting) {
		toggleAutoTargeting();
	}
};

/**
 * Shows MobileUI
 */
MobileUI.show = function show() {
	this._host.style.display = 'block';
};

/**
 * Create component and export it
 */
export default UIManager.addComponent(MobileUI);
