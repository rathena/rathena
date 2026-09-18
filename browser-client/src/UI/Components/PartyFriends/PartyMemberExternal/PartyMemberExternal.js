/**
 * UI/Components/PartyFriends/PartyMemberExternal/PartyMemberExternal.js
 *
 * Floating mini-window for a single party member.
 *
 */

import Client from 'Core/Client.js';
import DB from 'DB/DBManager.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import htmlText from './PartyMemberExternal.html?raw';
import cssText from './PartyMemberExternal.css?raw';
import ContextMenu from 'UI/Components/ContextMenu/ContextMenu.js';
import SkillTargetSelection from 'UI/Components/SkillTargetSelection/SkillTargetSelection.js';
import PartyFriendsV1 from 'UI/Components/PartyFriends/PartyFriendsV1/PartyFriendsV1.js';
import Camera from 'Renderer/Camera.js';
import Rodex from 'UI/Components/Rodex/Rodex.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Session from 'Engine/SessionStorage.js';
import WhisperBox from 'UI/Components/WhisperBox/WhisperBox.js';
import PartyFriends from 'UI/Components/PartyFriends/PartyFriends.js';

/**
 * Create Component
 */
const PartyMemberExternal = new GUIComponent('PartyMemberExternal', cssText);

/**
 * Helper: query inside shadow root
 */
function _root(comp) {
	return comp._shadow || comp._host;
}

/**
 * Render HTML
 */
PartyMemberExternal.render = () => htmlText;

/**
 * Initialize the component
 */
PartyMemberExternal.init = function init() {
	const self = this;
	const root = _root(this);

	root.addEventListener('mousedown', event => {
		self._lastPos = {
			top: self._host.offsetTop,
			left: self._host.offsetLeft
		};
		event.preventDefault();
		event.stopPropagation();

		if (ContextMenu) {
			ContextMenu.remove();
		}

		if (SkillTargetSelection && SkillTargetSelection.__active) {
			if (event.which === 1 && self._player && self._player.state === 0) {
				SkillTargetSelection.intersectEntityId(self._aid);
			}
			SkillTargetSelection.remove();
			event.stopImmediatePropagation();
			return;
		}

		// Block dragging when UI is locked
		if (PartyFriendsV1 && PartyFriendsV1.isLocked && PartyFriendsV1.isLocked()) {
			event.stopImmediatePropagation();
		}
	});

	// Right-click context menu
	root.addEventListener('contextmenu', event => {
		event.preventDefault();
		event.stopImmediatePropagation();

		// Prevent camera rotation from sticking
		if (Camera && Camera.rotate) {
			Camera.rotate(false);
		}

		// Block context menu when UI is locked
		if (PartyFriendsV1 && PartyFriendsV1.isLocked && PartyFriendsV1.isLocked()) {
			return;
		}

		if (!ContextMenu) {
			return;
		}

		ContextMenu.remove();
		ContextMenu.append();

		// Send Mail
		ContextMenu.addElement(DB.getMessage(98), () => {
			if (!self._player) {
				return;
			}
			if (Rodex) {
				Rodex.requestOpenWriteRodex(self._player.characterName);
			}
		});

		const isMe = self._player && Session && self._player.AID === Session.AID;

		if (isMe) {
			// Self: Leave party
			ContextMenu.addElement(DB.getMessage(2055), () => {
				UIManager.showPromptBox(DB.getMessage(357), 'ok', 'cancel', () => {
					if (PartyFriends && PartyFriends.onRequestLeave) {
						PartyFriends.onRequestLeave();
					} else {
						const ui = PartyFriends ? PartyFriends.getUI() : null;
						if (ui && ui.onRequestLeave) {
							ui.onRequestLeave();
						}
					}
				});
			});
		} else {
			// Others: 1:1 Chat
			ContextMenu.addElement(DB.getMessage(360), () => {
				if (!self._player) {
					return;
				}
				if (WhisperBox) {
					WhisperBox.show(self._player.characterName);
				} else if (ChatBox) {
					const chatRoot = ChatBox._shadow || ChatBox._host;
					if (chatRoot) {
						const usernameInput = chatRoot.querySelector('.username');
						if (usernameInput) usernameInput.value = self._player.characterName;
						const messageInput = chatRoot.querySelector('.message');
						if (messageInput) messageInput.focus();
					}
				}
			});
		}

		// Remove small party window
		ContextMenu.addElement('Remove small party window', () => {
			self._closedByUser = true;
			self.remove();
		});
	});

	this.gridSnap = {
		width: 147 + 25, // container 147px + 25px gap
		height: 38 + 10, // container 38px + 10px gap
		padX: 0, // margin off edge of the screen
		padY: 4
	};
	this.snapDuration = 80;
	this.draggable();

	this.onDragEnd = function () {
		if (PartyMemberExternal.onDragEnd) {
			PartyMemberExternal.onDragEnd(this);
		}
	};

	// Tooltip events
	const hostEl = this._host;
	hostEl.addEventListener('mouseenter', onTooltipShow);
	hostEl.addEventListener('mousemove', onTooltipMove);
	hostEl.addEventListener('mouseleave', onTooltipHide);
};

/**
 * Once append to the DOM, start to position the UI
 */
PartyMemberExternal.onAppend = function onAppend() {
	if (this._player) {
		this.update(this._player);
	}
};

/**
 * Set the member and update the UI
 * @param {number} AID
 * @param {object} player
 */
PartyMemberExternal.setMember = function setMember(AID, player) {
	this._aid = AID;
	this._player = player;
	if (this.__active) {
		this.update(player);
	}
};

/**
 * Update UI with player data
 * @param {object} player
 */
PartyMemberExternal.update = function update(player) {
	const root = _root(this);
	if (!root) return;

	const level = player.baseLevel || player.level || player.Level || 0;
	const jobID = player.class_ || player.job || player.Job || 0;
	const state = player.state || 0;
	const role = player.role || 0;

	const isOnline = state === 0;
	const isLeader = role === 0;

	const nameEl = root.querySelector('.name');
	if (nameEl) nameEl.textContent = player.characterName;

	const levelEl = root.querySelector('.level');
	if (levelEl) levelEl.textContent = `Lv. ${level}`;

	// Draw HP if available
	updateCanvasLife(
		root,
		player.life && player.life.display ? player.life.hp : undefined,
		player.life && player.life.display ? player.life.hp_max : undefined
	);

	// Job Icon
	const jobIcon = root.querySelector('.job-icon');
	const isDead = !!player.isDead;
	const asset = isDead ? `icon_jobs_${jobID}_die.bmp` : `icon_jobs_${jobID}.bmp`;

	Client.loadFile(DB.INTERFACE_PATH + 'renewalparty/' + asset, function (url) {
		if (jobIcon) jobIcon.style.backgroundImage = `url(${url})`;
	});

	// Crown
	const crownEl = root.querySelector('.crown');
	if (crownEl) {
		crownEl.style.display = isLeader ? 'block' : 'none';
	}
	if (isLeader) {
		Client.loadFile(DB.INTERFACE_PATH + 'renewalparty/ico_partycrown.bmp', function (url) {
			if (crownEl) crownEl.style.backgroundImage = `url(${url})`;
		});
	}

	// Tooltip and UI Text
	const mapDisplay = DB.getMapName(player.mapName || '');
	const tooltipText = `Lv.${level} ${player.characterName}(${mapDisplay})`;
	this._host.dataset.tooltip = tooltipText;

	const memberColor = isOnline ? 'white' : '#adadad';
	if (nameEl) {
		nameEl.textContent = player.characterName;
		nameEl.style.color = memberColor;
	}

	const mapEl = root.querySelector('.map');
	if (mapEl) {
		mapEl.textContent = `(${mapDisplay})`;
		mapEl.style.color = memberColor;
	}

	if (levelEl) {
		levelEl.textContent = `Lv. ${level}`;
		levelEl.style.color = memberColor;
	}

	const innerRoot = root.querySelector('#PartyMemberExternal');
	if (innerRoot) {
		if (isOnline) {
			innerRoot.classList.add('online');
		} else {
			innerRoot.classList.remove('online');
		}
	}
};

/**
 * Custom RO-style tooltips (global, outside shadow DOM)
 */
function onTooltipShow(event) {
	const text = event.currentTarget.dataset.tooltip;
	if (text) {
		let tooltip = document.getElementById('ro-tooltip-party');
		if (!tooltip) {
			tooltip = document.createElement('div');
			tooltip.id = 'ro-tooltip-party';
			tooltip.className = 'ro-tooltip';
			tooltip.style.cssText =
				'display:none;position:fixed;background-color:rgba(0,0,0,0.8);text-shadow:1px 1px black;color:white;padding:2px 6px;white-space:nowrap;z-index:20000;border-radius:2px;pointer-events:none;line-height:1.2;font-size:11px;';
			document.body.appendChild(tooltip);
		}
		tooltip.textContent = text;
		tooltip.style.display = 'block';
	}
}

function onTooltipMove(event) {
	const tooltip = document.getElementById('ro-tooltip-party');
	if (tooltip && tooltip.style.display === 'block') {
		tooltip.style.top = `${event.clientY + 15}px`;
		tooltip.style.left = `${event.clientX + 10}px`;
	}
}

function onTooltipHide() {
	const tooltip = document.getElementById('ro-tooltip-party');
	if (tooltip) {
		tooltip.style.display = 'none';
	}
}

/**
 * Update HP bar
 * @param {number} hp
 * @param {number} maxhp
 */
PartyMemberExternal.updateMemberLife = function updateMemberLife(hp, maxhp) {
	const root = _root(this);
	if (root) updateCanvasLife(root, hp, maxhp);
};

/**
 * Helper to update the HP bar on a canvas
 * @param {Element} root
 * @param {number} hp
 * @param {number} maxhp
 */
function updateCanvasLife(root, hp, maxhp) {
	const hasLife = hp !== undefined && maxhp !== undefined && maxhp > 0;
	const lifeRatio = hasLife ? hp / maxhp : 0;

	const hpBarContainer = root.querySelector('.hp-bar-container');
	if (hpBarContainer) hpBarContainer.style.visibility = 'visible';

	if (hasLife) {
		const canvas = root.querySelector('canvas');
		if (canvas) {
			const ctx = canvas.getContext('2d');
			const width = Math.floor(lifeRatio * 75);
			canvas.width = 75;
			canvas.height = 5;
			ctx.clearRect(0, 0, 75, 5);
			ctx.fillStyle = lifeRatio <= 0.25 ? '#ef1010' : '#32cd32';
			ctx.fillRect(0, 0, width, 5);
		}
		const hpText = root.querySelector('.hp-text');
		if (hpText) hpText.textContent = `${hp}/${maxhp}`;
	} else {
		const canvas = root.querySelector('canvas');
		if (canvas) {
			const ctx = canvas.getContext('2d');
			ctx.clearRect(0, 0, 75, 5);
		}
		const hpText = root.querySelector('.hp-text');
		if (hpText) hpText.textContent = '';
	}
}

/**
 * Exports
 */
export default UIManager.addComponent(PartyMemberExternal);
