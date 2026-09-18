/**
 * UI/Components/PartyFriends/PartyFriendsCommon.js
 *
 * Shared factory for the Party & Friends windows.
 *
 * Collapses the PartyFriends version siblings (V0, V1) into a single
 * createPartyFriends(config) factory. Each version file passes its own
 * htmlText/cssText plus capability flags describing its real differences:
 *   - V0 : classic party layout (simple name/map/HP node), jQuery-proxy
 *          visibility, ChatBox-based mail helper, no detachable member windows.
 *   - V1 : renewal party layout (job icons, level, crown, HP bar, status
 *          icons, member cap footer), host-style visibility, Rodex mail,
 *          detachable member mini-windows, tooltips and member sorting.
 *
 * The single `renewalParty` flag selects the V1 implementation; everything
 * else (friend list handling, window chrome, resize, tab switching, lock,
 * option/invite/create windows) is shared verbatim between both versions.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Camera from 'Renderer/Camera.js';
import MiniMap from 'UI/Components/MiniMap/MiniMap.js';
import Preferences from 'Core/Preferences.js';
import MonsterTable from 'DB/Monsters/MonsterTable.js';
import Client from 'Core/Client.js';
import Renderer from 'Renderer/Renderer.js';
import Session from 'Engine/SessionStorage.js';
import Mouse from 'Controls/MouseEventHandler.js';
import KEYS from 'Controls/KeyEventHandler.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PartyHelper from './PartyHelper/PartyHelper.js';
import PartyMemberExternal from './PartyMemberExternal/PartyMemberExternal.js';
import ContextMenu from 'UI/Components/ContextMenu/ContextMenu.js';
import Mail from 'UI/Components/Mail/Mail.js';
import Rodex from 'UI/Components/Rodex/Rodex.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import WhisperBox from 'UI/Components/WhisperBox/WhisperBox.js';
import SkillTargetSelection from 'UI/Components/SkillTargetSelection/SkillTargetSelection.js';

export function createPartyFriends(config) {
	const { name: componentName, htmlText, cssText, renewalParty = false } = config;

	/**
	 * Create Component
	 */
	const Component = new GUIComponent(componentName, cssText);

	/**
	 * @var {number} index of selection
	 */
	let _index = -1;

	/**
	 * @var {Array} friends list
	 */
	const _friends = [];

	/**
	 * @var {Array} party list
	 */
	const _party = [];

	/**
	 * @var {Object} party setup
	 */
	const _options = {
		exp_share: 0,
		item_share: 0,
		item_sharing_type: 0
	};

	/**
	 * Renewal-only detached member state (unused in classic layout).
	 */
	let _detachedMembers = {}; // Map of AID -> Component
	let _savedPositions = null; // Local cache for restored positions
	let _skipSaveOnRemove = false; // Set by clean() so onRemove skips saving

	/**
	 * @var {Preferences} structure
	 */
	const _prefsDefault = {
		x: 200,
		y: 200,
		width: 12,
		height: 6,
		show: false,
		friend: true,
		lock: false
	};
	if (renewalParty) {
		_prefsDefault.detachedMembers = {}; // Map of AID -> { x, y }
	}
	const _preferences = Preferences.get(componentName, _prefsDefault, 1.0);

	/**
	 * Helper: query inside shadow root
	 */
	function _root() {
		return Component.getRoot();
	}

	/**
	 * Helper: escape HTML
	 */
	function _escapeHTML(text) {
		const div = document.createElement('div');
		div.textContent = text;
		return div.innerHTML;
	}

	/**
	 * Visibility helpers — V0 drives the jQuery-compat proxy, V1 the host node.
	 */
	function _isVisible() {
		return renewalParty ? Component._host.style.display !== 'none' : Component.ui.is(':visible');
	}
	function _showWindow() {
		if (renewalParty) {
			Component._host.style.display = 'block';
		} else {
			Component.ui.show();
		}
	}
	function _toggleWindow() {
		if (renewalParty) {
			Component._host.style.display = _isVisible() ? 'none' : 'block';
		} else {
			Component.ui.toggle();
		}
	}

	/**
	 * Render HTML
	 */
	Component.render = () => htmlText;

	/**
	 * Initialize the component (event listener, etc.)
	 */
	Component.init = function init() {
		const root = _root();

		// Start loading the helper
		PartyHelper.prepare();

		// Avoid drag drop problems
		const baseBtn = root.querySelector('.base');
		if (baseBtn) {
			baseBtn.addEventListener('mousedown', event => {
				event.stopImmediatePropagation();
				event.preventDefault();
			});
		}

		// Bind buttons
		const closeBtn = root.querySelector('.close');
		if (closeBtn) {
			closeBtn.addEventListener('click', onClose);
		}

		root.querySelectorAll('.lock').forEach(el => {
			el.addEventListener('mousedown', onToggleLock);
		});

		root.querySelectorAll('.switchtab.off').forEach(el => {
			el.addEventListener('mousedown', onChangeTab);
		});

		const removeBtn = root.querySelector('.remove');
		if (removeBtn) removeBtn.addEventListener('mousedown', onRequestRemoveSelection);

		const pmBtn = root.querySelector('.privatemessage');
		if (pmBtn) pmBtn.addEventListener('mousedown', onRequestPrivateMessage);

		const leaveBtn = root.querySelector('.leave');
		if (leaveBtn) leaveBtn.addEventListener('mousedown', onRequestLeaveParty);

		const resizeBtn = root.querySelector('.resize');
		if (resizeBtn) resizeBtn.addEventListener('mousedown', onResize);

		const mailBtn = root.querySelector('.mail');
		if (mailBtn) mailBtn.addEventListener('mousedown', onOpenMailCreationWindow);

		const createBtn = root.querySelector('.party.create');
		if (createBtn) createBtn.addEventListener('mousedown', onOpenPartyCreationWindow);

		const addBtn = root.querySelector('.party.add');
		if (addBtn) addBtn.addEventListener('mousedown', onOpenPartyInviteWindow);

		const infoBtn = root.querySelector('.info');
		if (infoBtn) infoBtn.addEventListener('mousedown', onOpenPartyOptionWindow);

		if (renewalParty) {
			const sortBtn = root.querySelector('.party.sort');
			if (sortBtn) sortBtn.addEventListener('mousedown', sortDetachedMembers);
		}

		// Context menu and selection on nodes
		const content = root.querySelector('.content');
		if (content) {
			if (renewalParty) {
				content.addEventListener('contextmenu', event => {
					const node = event.target.closest('.node');
					if (node) {
						onRightClickInfoRenewal.call(node, event);
					}
				});

				content.addEventListener('mousedown', event => {
					const node = event.target.closest('.node');
					if (node) {
						onMemberMouseDown.call(node, event);
					}
				});

				content.addEventListener('mouseover', event => {
					const el = event.target.closest('[data-tooltip]');
					if (el) onTooltipShow.call(el, event);
				});

				content.addEventListener('mousemove', event => {
					const el = event.target.closest('[data-tooltip]');
					if (el) onTooltipMove.call(el, event);
				});

				content.addEventListener('mouseout', event => {
					const el = event.target.closest('[data-tooltip]');
					if (el) onTooltipHide.call(el, event);
				});
			} else {
				content.addEventListener('contextmenu', e => {
					const node = e.target.closest('.node');
					if (node) {
						e.preventDefault();
						e.stopImmediatePropagation();
						onRightClickInfoClassic();
					}
				});
				content.addEventListener('mousedown', e => {
					const node = e.target.closest('.node');
					if (node) {
						onSelectionChangeClassic(node);
					}
				});
			}
		}

		this.draggable('.titlebar');

		if (renewalParty) {
			// Save external window position when dragging ends
			PartyMemberExternal.onDragEnd = function (movedComponent) {
				const pos = {
					left: movedComponent._host.offsetLeft,
					top: movedComponent._host.offsetTop
				};
				const oldPos = movedComponent._lastPos;

				if (oldPos) {
					for (const aid in _detachedMembers) {
						const other = _detachedMembers[aid];
						if (other && other !== movedComponent && other._host) {
							const otherPos = {
								left: other._host.offsetLeft,
								top: other._host.offsetTop
							};
							if (Math.abs(otherPos.left - pos.left) < 5 && Math.abs(otherPos.top - pos.top) < 5) {
								other._host.style.left = `${oldPos.left}px`;
								other._host.style.top = `${oldPos.top}px`;
								Component.saveDetachedMembers();
								return;
							}
						}
					}
				}

				Component.saveDetachedMembers();
			};
		}
	};

	/**
	 * Once append to the DOM, start to position the UI
	 */
	Component.onAppend = function onAppend() {
		const root = _root();

		// Initialize the tab
		_preferences.friend = !_preferences.friend;
		onChangeTab();

		// Lock features
		const lockOn = root.querySelector('.lock.on');
		const lockOff = root.querySelector('.lock.off');

		if (_preferences.lock) {
			if (lockOn) lockOn.style.display = 'inline-block';
			if (lockOff) lockOff.style.display = 'none';
		} else {
			if (lockOn) lockOn.style.display = 'none';
			if (lockOff) lockOff.style.display = 'inline-block';
		}

		this.resize(_preferences.width, _preferences.height);

		this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - (this._host.offsetHeight || 0))}px`;
		this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - (this._host.offsetWidth || 0))}px`;

		if (renewalParty) {
			// Load footer background
			Client.loadFile(DB.INTERFACE_PATH + 'renewalparty/bg_partymember.bmp', function (url) {
				const countBox = root.querySelector('.count-box');
				if (countBox) countBox.style.backgroundImage = `url(${url})`;
			});
		}

		if (!_preferences.show) {
			this._host.style.display = 'none';
		}

		if (renewalParty) {
			// Restore detached members for same-map teleports
			for (let i = 0; i < _party.length; i++) {
				restoreDetachedMember(_party[i]);
			}
		}
	};

	/**
	 * Clean up UI
	 */
	Component.clean = function clean() {
		const root = _root();

		_party.length = 0;
		_friends.length = 0;

		if (renewalParty) {
			_skipSaveOnRemove = true;
			for (const aid in _detachedMembers) {
				if (_detachedMembers[aid]) {
					_detachedMembers[aid].remove();
				}
			}
			_detachedMembers = {};
			_savedPositions = null;
		}

		_options.exp_share = 0;
		_options.item_share = 0;
		_options.item_sharing_type = 0;

		const partyName = root.querySelector('.partyname');
		if (partyName) partyName.textContent = '';

		const friendCount = root.querySelector('.friendcount');
		if (friendCount) friendCount.textContent = '0';

		const partyContent = root.querySelector('.content .party');
		if (partyContent) partyContent.innerHTML = '';

		const friendContent = root.querySelector('.content .friend');
		if (friendContent) friendContent.innerHTML = '';

		if (renewalParty) {
			const innerCount = root.querySelector('.count-box .inner-count');
			if (innerCount) innerCount.textContent = '0/12';
		}

		// Reset buttons
		_preferences.friend = !_preferences.friend;
		onChangeTab();
	};

	/**
	 * Removing the UI from window, save preferences
	 */
	Component.onRemove = function onRemove() {
		_preferences.show = _isVisible();
		if (renewalParty) {
			_preferences.y = parseInt(this._host.style.top, 10) || 0;
			_preferences.x = parseInt(this._host.style.left, 10) || 0;
		} else {
			_preferences.y = parseInt(this._host.style.top, 10);
			_preferences.x = parseInt(this._host.style.left, 10);
		}
		_preferences.save();

		if (renewalParty) {
			if (!_skipSaveOnRemove && Object.keys(_detachedMembers).length > 0) {
				this.saveDetachedMembers();
			}
			_skipSaveOnRemove = false;

			_savedPositions = null;

			const tooltip = document.getElementById('ro-tooltip-party');
			if (tooltip) tooltip.style.display = 'none';
		}
	};

	/**
	 * Window Shortcuts
	 */
	Component.onShortCut = function onShortCut(key) {
		switch (key.cmd) {
			case 'FRIEND':
				if (_preferences.friend) {
					_toggleWindow();
				} else {
					_preferences.friend = false;
					onChangeTab();
					_showWindow();
				}

				if (_isVisible()) {
					this.focus();
				}
				break;

			case 'PARTY':
				if (!_preferences.friend) {
					_toggleWindow();
				} else {
					_preferences.friend = true;
					onChangeTab();
					_showWindow();
				}

				if (_isVisible()) {
					this.focus();
				}
				break;
		}
	};

	/**
	 * Show/Hide UI
	 */
	Component.toggle = function toggle() {
		_toggleWindow();
		PartyHelper.remove();
		if (_isVisible()) {
			this.focus();
		}
	};

	Component.onKeyDown = function onKeyDown(event) {
		if ((event.which === KEYS.ESCAPE || event.key === 'Escape') && _isVisible()) {
			this.toggle();
		}
	};

	/**
	 * Set friends to UI
	 *
	 * @param {Array} friends list
	 */
	Component.setFriends = function setFriends(friends) {
		const root = _root();
		const count = friends.length;
		const friendContainer = root.querySelector('.content .friend');

		_friends.length = friends.length;
		if (friendContainer) friendContainer.innerHTML = '';

		for (let i = 0; i < count; i++) {
			_friends[i] = friends[i];
			if (friendContainer) {
				friendContainer.insertAdjacentHTML(
					'beforeend',
					`<div class="node${friends[i].State === 0 ? ' online' : ''}">` +
						`<span class="name">${_escapeHTML(friends[i].Name)}</span>` +
						`</div>`
				);
			}
		}

		const friendCount = root.querySelector('.friendcount');
		if (friendCount) friendCount.textContent = String(count);
		_index = -1;
	};

	/**
	 * Update friend (online/offline) state
	 *
	 * @param {number} index
	 * @param {boolean} state
	 */
	Component.updateFriendState = function updateFriendState(index, state) {
		const root = _root();
		const nodes = root.querySelectorAll('.content .friend .node');
		const node = nodes[index];

		_friends[index].State = state;

		if (state) {
			if (node) node.style.backgroundImage = '';
			ChatBox.addText(
				DB.getMessage(1042).replace('%s', _friends[index].Name),
				ChatBox.TYPE.BLUE,
				ChatBox.FILTER.PUBLIC_LOG
			);
			return;
		}

		ChatBox.addText(
			DB.getMessage(1041).replace('%s', _friends[index].Name),
			ChatBox.TYPE.BLUE,
			ChatBox.FILTER.PUBLIC_LOG
		);
		Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/grp_online.bmp', function (url) {
			if (node) node.style.backgroundImage = `url(${url})`;
		});
	};

	/**
	 * Update/Add a friend to the list
	 *
	 * @param {number} index
	 * @param {object} friend data
	 */
	Component.updateFriend = function updateFriend(idx, friend) {
		const root = _root();

		// Add it
		if (!_friends[idx]) {
			_friends[idx] = {};

			const friendContainer = root.querySelector('.content .friend');
			if (friendContainer) {
				friendContainer.insertAdjacentHTML('beforeend', '<div class="node"><span class="name"></span></div>');
			}

			const friendCount = root.querySelector('.friendcount');
			if (friendCount) friendCount.textContent = String(_friends.length);
		}

		_friends[idx].Name = friend.Name;
		_friends[idx].GID = friend.GID;
		_friends[idx].AID = friend.AID;
		_friends[idx].State = friend.State || 0;

		const nodes = root.querySelectorAll('.content .friend .node');
		const node = nodes[idx];
		if (node) {
			const nameEl = node.querySelector('.name');
			if (nameEl) nameEl.textContent = friend.Name;
		}

		Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/grp_online.bmp', function (url) {
			if (node) node.style.backgroundImage = `url(${url})`;
		});
	};

	/**
	 * Remove friend from list
	 *
	 * @param {number} index
	 */
	Component.removeFriend = function removeFriend(index) {
		const root = _root();
		_friends.splice(index, 1);

		const nodes = root.querySelectorAll('.content .friend .node');
		if (nodes[index]) nodes[index].remove();

		const friendCount = root.querySelector('.friendcount');
		if (friendCount) friendCount.textContent = String(_friends.length);

		if (_index === index) {
			_index = -1;
		}
	};

	// ─── Party list (layout-specific) ──────────────────────

	if (renewalParty) {
		/**
		 * Refresh the party list rendering
		 */
		Component.refreshPartyList = function refreshPartyList() {
			const root = _root();
			const partyContent = root.querySelector('.content .party');
			if (partyContent) partyContent.innerHTML = '';

			for (let i = 0; i < _party.length; i++) {
				this.renderPartyMember(_party[i]);
			}

			const innerCount = root.querySelector('.count-box .inner-count');
			if (innerCount) innerCount.textContent = `${_party.length}/12`;
		};

		/**
		 * Add members to party
		 */
		Component.setParty = function setParty(name, members) {
			const root = _root();

			const partyName = root.querySelector('.partyname');
			if (partyName) partyName.textContent = `(${name})`;
			Session.isPartyLeader = false;

			const createBtn = root.querySelector('.party.create');
			if (createBtn) createBtn.style.display = 'none';

			const leaveBtn = root.querySelector('.party.leave');
			if (leaveBtn) leaveBtn.style.display = '';

			const sortBtn = root.querySelector('.party.sort');
			if (sortBtn) sortBtn.style.display = '';

			const count = members.length;
			const newAIDs = {};

			for (let i = 0; i < count; i++) {
				const member = members[i];
				newAIDs[member.AID] = true;
				Component.addPartyMember(member);
			}

			// Remove members who are no longer in the party
			if (_party.length > 0 && count > 0) {
				for (let i = 0; i < _party.length; i++) {
					if (!newAIDs[_party[i].AID]) {
						const removed = _party.splice(i, 1)[0];
						const nodeEl = root.querySelector(`.content .party .node[data-aid="${removed.AID}"]`);
						if (nodeEl) nodeEl.remove();
						i--;
					}
				}
			}

			const innerCount = root.querySelector('.count-box .inner-count');
			if (innerCount) innerCount.textContent = `${_party.length}/12`;

			// Garbage Collection: Remove windows for members who left the party
			let removedCount = 0;
			for (const aid in _detachedMembers) {
				let foundInPacket = false;
				for (let i = 0; i < count; i++) {
					if (members[i].AID == aid || String(members[i].AID) === aid) {
						foundInPacket = true;
						break;
					}
				}
				if (!foundInPacket) {
					if (_detachedMembers[aid]) {
						console.log('[PartyFriendsV1] GC removing orphaned window for AID:', aid);
						_detachedMembers[aid].remove();
						removedCount++;
					}
				}
			}
			if (removedCount > 0) {
				console.log(`[PartyFriendsV1] Cleaned up ${removedCount} orphaned detached windows`);
			}
		};

		/**
		 * Add a new party member to the list
		 */
		Component.addPartyMember = function addPartyMember(player) {
			const root = _root();
			const role = player.role || 0;
			const count = _party.length;
			let i;

			// Check if we are the leader
			if (player.AID === Session.AID) {
				Session.isPartyLeader = role === 0;
				const addBtn = root.querySelector('.party.add');
				if (addBtn) {
					addBtn.style.display = Session.isPartyLeader ? '' : 'none';
				}
			}

			// Search for duplicates entries
			for (i = 0; i < count; ++i) {
				if (_party[i].AID === player.AID || _party[i].characterName === player.characterName) {
					break;
				}
			}

			let hasChanged = false;

			if (i < count) {
				const old = _party[i];

				if (
					old.baseLevel !== (player.baseLevel !== undefined ? player.baseLevel : old.baseLevel) ||
					old.class_ !== (player.class_ !== undefined ? player.class_ : old.class_) ||
					old.mapName !== (player.mapName !== undefined ? player.mapName : old.mapName) ||
					old.role !== player.role ||
					old.state !== (player.state !== undefined ? player.state : old.state)
				) {
					hasChanged = true;
				}

				Object.assign(_party[i], player);
				player = _party[i];
			} else {
				player = Object.assign({}, player);
				_party.push(player);
				hasChanged = true;
			}

			if (!hasChanged) {
				return;
			}

			const innerCount = root.querySelector('.count-box .inner-count');
			if (innerCount) innerCount.textContent = `${_party.length}/12`;

			// Handle detached state
			if (_detachedMembers[player.AID]) {
				_detachedMembers[player.AID].setMember(player.AID, player);
			} else {
				restoreDetachedMember(player);
			}

			Component.renderPartyMember(player);
		};

		/**
		 * Render a party member into the UI
		 */
		Component.renderPartyMember = function renderPartyMember(player) {
			const root = _root();
			const role = player.role || player.Role || 0;
			const job = player.class_ || player.job || player.Job || 0;
			const level = player.baseLevel || player.level || player.Level || 0;
			const isDead = !!player.isDead;
			const isOnline = player.state === 0;
			const isDetached = !!_detachedMembers[player.AID];

			const jobName = MonsterTable[job] || 'Unknown';
			const mapDisplay = DB.getMapName(player.mapName);

			const color = MiniMap && MiniMap.getMemberColor ? MiniMap.getMemberColor(player.AID) : 'white';
			player.color = color;

			const nameTooltip = `${player.characterName} (${mapDisplay})`;

			const hasLife = !!(player.life && player.life.display);
			const barVisibility = hasLife ? 'visible' : 'hidden';

			const memberColor = isOnline ? player.color || '#333' : '#848ca5';

			const html =
				`<div class="node${role === 0 ? ' leader' : ''}${isOnline ? ' online' : ''}${isDetached ? ' detached' : ''}"` +
				` style="background-color: ${isDetached ? '#deefff' : 'white'};"` +
				` data-aid="${player.AID}"` +
				` data-tooltip="${isOnline ? _escapeHTML(nameTooltip) : ''}"` +
				`>` +
				`<div class="job-icon-container" data-tooltip="${_escapeHTML(jobName)}">` +
				`<div class="job-icon"></div>` +
				`<div class="crown"></div>` +
				`</div>` +
				`<div class="info-container">` +
				`<div class="row1">` +
				`<span class="level">Lv. ${level}</span>` +
				`<span class="name" style="color: ${memberColor};">${_escapeHTML(player.characterName)}</span>` +
				`<span class="map" style="color: ${memberColor};">(${_escapeHTML(mapDisplay)})</span>` +
				`</div>` +
				`<div class="row2">` +
				`<div class="hp-bar-container" style="visibility:${barVisibility}">` +
				`<canvas class="life" width="75" height="5"></canvas>` +
				`</div>` +
				`<span class="hp" style="visibility:${barVisibility}"></span>` +
				`<div class="status-icon"></div>` +
				`</div>` +
				`</div>` +
				`</div>`;

			let node = root.querySelector(`.content .party .node[data-aid="${player.AID}"]`);
			if (node) {
				node.outerHTML = html;
				node = root.querySelector(`.content .party .node[data-aid="${player.AID}"]`);
			} else {
				const partyContent = root.querySelector('.content .party');
				if (partyContent) {
					partyContent.insertAdjacentHTML('beforeend', html);
					node = root.querySelector(`.content .party .node[data-aid="${player.AID}"]`);
				}
			}

			if (!node) return;

			// Load Job Icon
			Client.loadFile(
				`${DB.INTERFACE_PATH}renewalparty/icon_jobs_${job}${isDead ? '_die' : ''}.bmp`,
				function (url) {
					const jobIcon = node.querySelector('.job-icon');
					if (jobIcon) jobIcon.style.backgroundImage = `url(${url})`;
				}
			);

			// Load Crown
			if (role === 0) {
				Client.loadFile(DB.INTERFACE_PATH + 'renewalparty/ico_partycrown.bmp', function (url) {
					const crown = node.querySelector('.crown');
					if (crown) crown.style.backgroundImage = `url(${url})`;
				});
			}

			// Load Status Icon
			const statusFile = `icon_party_${player.AID == Session.AID ? 'me.bmp' : isOnline ? 'on.bmp' : 'off.bmp'}`;
			Client.loadFile(DB.INTERFACE_PATH + 'renewalparty/' + statusFile, function (url) {
				const statusIcon = node.querySelector('.status-icon');
				if (statusIcon) statusIcon.style.backgroundImage = `url(${url})`;
			});

			// Draw HP bar
			updateCanvasLife(node, hasLife ? player.life.hp : undefined, hasLife ? player.life.hp_max : undefined);
		};

		/**
		 * Remove a character from list
		 */
		Component.removePartyMember = function removePartyMember(AID, characterName) {
			const root = _root();

			if (AID === Session.AID) {
				for (const aid in _detachedMembers) {
					if (_detachedMembers[aid]) {
						_detachedMembers[aid].onRemove = function () {
							delete UIManager.components[this.name];
						};
						_detachedMembers[aid].remove();
					}
				}
				_detachedMembers = {};

				_party.length = 0;

				const partyContent = root.querySelector('.content .party');
				if (partyContent) partyContent.innerHTML = '';

				const partyNameEl = root.querySelector('.partyname');
				if (partyNameEl) partyNameEl.textContent = '';

				const createBtn = root.querySelector('.party.create');
				if (createBtn) createBtn.style.display = '';

				const leaveBtn = root.querySelector('.party.leave');
				if (leaveBtn) leaveBtn.style.display = 'none';

				const addBtn = root.querySelector('.party.add');
				if (addBtn) addBtn.style.display = 'none';

				ChatBox.addText(DB.getMessage(84), ChatBox.TYPE.BLUE, ChatBox.FILTER.PARTY_SETUP);
				return;
			}

			const count = _party.length;

			for (let i = 0; i < count; ++i) {
				if (_party[i].AID === AID && _party[i].characterName === characterName) {
					_party.splice(i, 1);
					const nodes = root.querySelectorAll('.content .party .node');
					if (nodes[i]) nodes[i].remove();

					const innerCount = root.querySelector('.count-box .inner-count');
					if (innerCount) innerCount.textContent = `${_party.length}/12`;

					if (_detachedMembers[AID]) {
						_detachedMembers[AID].remove();
					}

					break;
				}
			}
		};

		/**
		 * Update player dead/alive state
		 */
		Component.updateMemberDead = function updateMemberDead(AID, isDead) {
			const count = _party.length;
			let player = null;

			for (let i = 0; i < count; ++i) {
				if (_party[i].AID === AID) {
					_party[i].isDead = isDead;
					player = _party[i];
					this.renderPartyMember(player);
					break;
				}
			}

			if (player && _detachedMembers[AID]) {
				_detachedMembers[AID].setMember(AID, player);
			}
		};

		/**
		 * Update player life in interface
		 */
		Component.updateMemberLife = function updateMemberLife(AID, canvas, hp, maxhp) {
			const root = _root();
			const count = _party.length;

			for (let i = 0; i < count; ++i) {
				if (_party[i].AID === AID) {
					if (!_party[i].life) {
						_party[i].life = {};
					}
					_party[i].life.hp = hp;
					_party[i].life.hp_max = maxhp;
					_party[i].life.display = true;

					if (hp === 0 && !_party[i].isDead) {
						_party[i].isDead = true;
						this.renderPartyMember(_party[i]);
					} else if (hp > 0 && _party[i].isDead) {
						_party[i].isDead = false;
						this.renderPartyMember(_party[i]);
					}
					break;
				}
			}

			if (_detachedMembers[AID]) {
				_detachedMembers[AID].updateMemberLife(hp, maxhp);
			}

			const node = root.querySelector(`.content .party .node[data-aid="${AID}"]`);
			if (node) {
				updateCanvasLife(node, hp, maxhp);
			}
		};

		/**
		 * Check if UI is locked
		 */
		Component.isLocked = function isLocked() {
			return !!_preferences.lock;
		};

		/**
		 * Save the current positions of all detached member windows to character-specific localStorage.
		 */
		Component.saveDetachedMembers = function () {
			if (!Session.Entity?.display?.name) {
				return;
			}

			const key = `PartyFriends_${Session.Entity.display.name}_Detached`;
			const saved = {};
			let count = 0;

			for (const aid in _detachedMembers) {
				const ext = _detachedMembers[aid];
				if (ext && ext._host) {
					saved[aid] = { x: ext._host.offsetLeft, y: ext._host.offsetTop };
					count++;
				}
			}

			localStorage.setItem(key, JSON.stringify(saved));
			if (count > 0) {
				console.log(`[PartyFriendsV1] Saved ${count} detached windows for ${Session.Entity.display.name}`);
			}
		};
	} else {
		/**
		 * Add members to party
		 *
		 * @param {string} party name
		 * @param {Array} member list
		 */
		Component.setParty = function setParty(name, members) {
			const root = _root();

			const partyName = root.querySelector('.partyname');
			if (partyName) partyName.textContent = `(${name})`;

			const partyContent = root.querySelector('.content .party');
			if (partyContent) partyContent.innerHTML = '';

			Session.isPartyLeader = false;

			const createBtn = root.querySelector('.party.create');
			const leaveBtn = root.querySelector('.party.leave');
			if (createBtn) createBtn.style.display = 'none';
			if (leaveBtn) leaveBtn.style.display = 'inline-block';

			const count = members.length;

			_party.length = 0;
			for (let i = 0; i < count; i++) {
				Component.addPartyMember(members[i]);
			}

			_index = -1;
		};

		/**
		 * Add a new party member to the list
		 *
		 * @param {object} player information
		 */
		Component.addPartyMember = function addPartyMember(player) {
			const role = player.role || player.Role || 0;
			const count = _party.length;
			const root = _root();
			let node;

			// Check if we are the leader
			if (player.AID === Session.AID) {
				Session.isPartyLeader = role === 0;
				const addBtn = root.querySelector('.party.add');
				if (addBtn) {
					addBtn.style.display = Session.isPartyLeader ? 'inline-block' : 'none';
				}
			}

			// Search for duplicates entries
			const nodes = root.querySelectorAll('.content .party .node');
			for (let i = 0; i < count; ++i) {
				if (_party[i].AID === player.AID && _party[i].characterName === player.characterName) {
					node = nodes[i];
					break;
				}
			}

			// Update
			if (node) {
				node.classList.remove('leader', 'online');

				if (role === 0) {
					node.classList.add('leader');
				}
				if (player.state === 0) {
					node.classList.add('online');
				}

				node.style.backgroundImage = '';
				const nameEl = node.querySelector('.name');
				if (nameEl) nameEl.textContent = player.characterName;
				const mapEl = node.querySelector('.map');
				if (mapEl) mapEl.textContent = `(${DB.getMapName(player.mapName)})`;
			}

			// Create
			else {
				player = Object.assign({}, player);

				_party.push(player);
				const partyContent = root.querySelector('.content .party');
				if (partyContent) {
					const div = document.createElement('div');
					div.className = `node${role === 0 ? ' leader' : ''}${player.state === 0 ? ' online' : ''}`;
					div.innerHTML =
						`<span class="name">${_escapeHTML(player.characterName)}</span>` +
						`<span class="map">(${_escapeHTML(DB.getMapName(player.mapName))})</span>` +
						'<canvas class="life" width="60" height="5"></canvas> <span class="hp"></span>';
					partyContent.appendChild(div);
					node = div;
				}
			}

			if (!node) return;

			const hpEl = node.querySelector('.hp');
			if (hpEl) hpEl.textContent = '';

			const canvas = node.querySelector('canvas');
			if (canvas) {
				const ctx = canvas.getContext('2d');
				ctx.clearRect(0, 0, ctx.canvas.width, ctx.canvas.height);

				// Update life
				if (player.life && player.life.display) {
					ctx.drawImage(player.life.canvas, 0, 0, 60, 5, 0, 0, 60, 5);
					if (hpEl) hpEl.textContent = `${player.life.hp}/${player.life.hp_max}`;
				}
			}

			// Add texture
			const texture =
				role === 0 && player.state === 0 ? 'grp_leader.bmp' : player.state === 0 ? 'grp_online.bmp' : '';
			if (texture) {
				Client.loadFile(DB.INTERFACE_PATH + 'basic_interface/' + texture, function (url) {
					if (node) node.style.backgroundImage = `url(${url})`;
				});
			}
		};

		/**
		 * Remove a character from list
		 *
		 * @param {number} account id
		 * @param {string} character name
		 */
		Component.removePartyMember = function removePartyMember(AID, characterName) {
			const root = _root();

			if (AID === Session.AID) {
				_party.length = 0;

				const partyContent = root.querySelector('.content .party');
				if (partyContent) partyContent.innerHTML = '';

				const partyName = root.querySelector('.partyname');
				if (partyName) partyName.textContent = '';

				const createBtn = root.querySelector('.party.create');
				if (createBtn) createBtn.style.display = 'inline-block';

				const leaveBtn = root.querySelector('.party.leave');
				const addBtn = root.querySelector('.party.add');
				if (leaveBtn) leaveBtn.style.display = 'none';
				if (addBtn) addBtn.style.display = 'none';

				ChatBox.addText(DB.getMessage(84), ChatBox.TYPE.BLUE, ChatBox.FILTER.PARTY_SETUP);
				return;
			}

			const count = _party.length;
			const nodes = root.querySelectorAll('.content .party .node');

			for (let i = 0; i < count; ++i) {
				if (_party[i].AID === AID && _party[i].characterName === characterName) {
					_party.splice(i, 1);
					if (nodes[i]) nodes[i].remove();
					break;
				}
			}
		};

		/**
		 * Update player life in interface
		 *
		 * @param {number} account id
		 * @param {canvas} canvas life element
		 * @param {number} hp
		 * @param {number} maxhp
		 */
		Component.updateMemberLife = function updateMemberLife(AID, canvas, hp, maxhp) {
			const count = _party.length;
			const root = _root();
			const nodes = root.querySelectorAll('.content .party .node');

			for (let i = 0; i < count; ++i) {
				if (_party[i].AID === AID && _party[i].state === 0) {
					const node = nodes[i];
					if (!node) break;
					const cvs = node.querySelector('canvas');
					if (cvs) {
						const ctx = cvs.getContext('2d');
						ctx.drawImage(canvas, 0, 0, 60, 5, 0, 0, 60, 5);
					}
					const hpEl = node.querySelector('.hp');
					if (hpEl) hpEl.textContent = `${hp}/${maxhp}`;
					break;
				}
			}
		};
	}

	/**
	 * Extend inventory window size
	 *
	 * @param {number} width
	 * @param {number} height
	 */
	Component.resize = function resize(width, height) {
		const root = _root();
		width = Math.min(Math.max(width, 12), 13);
		height = Math.min(Math.max(height, 6), 12);

		_preferences.width = width;
		_preferences.height = height;
		_preferences.save();

		const content = root.querySelector('.content');
		if (content) {
			content.style.width = `${width * 20}px`;
			content.style.height = `${height * 20}px`;
		}
	};

	/**
	 * Update party options
	 *
	 * @param {boolean} exp share option
	 * @param {boolean} item share option
	 * @param {boolean} item sharing option
	 */
	Component.setOptions = function setOptions(exp_share, item_share, item_sharing_type) {
		if (exp_share !== undefined) {
			_options.exp_share = exp_share;
		}
		if (item_share !== undefined) {
			_options.item_share = item_share;
		}
		if (item_sharing_type !== undefined) {
			_options.item_sharing_type = item_sharing_type;
		}
	};

	/**
	 * Check if character is a group member
	 *
	 * @param {string} character name
	 */
	Component.isGroupMember = function isGroupMember(characterName) {
		const count = _party.length;
		for (let i = 0; i < count; ++i) {
			if (_party[i].characterName === characterName) {
				return true;
			}
		}

		return false;
	};

	// ─── Renewal-only detached member helpers ──────────────

	/**
	 * Logic when clicking a member node to handle possible drag out
	 */
	function onMemberMouseDown(event) {
		// Only left click blocks event propagation to game world
		if (event.which === 1) {
			event.stopPropagation();
		} else {
			return;
		}

		const node = this;
		const AID = parseInt(node.dataset.aid, 10);

		event.preventDefault();

		// Selection change immediately on mousedown
		onSelectionChangeRenewal.call(node, event);

		const player = _party.find(member => member.AID == AID);

		if (!player) {
			return;
		}

		// Handle Skill Targeting
		if (SkillTargetSelection && SkillTargetSelection.__active && !_preferences.friend) {
			if (player.state === 0) {
				SkillTargetSelection.intersectEntityId(player.AID);
			}
			SkillTargetSelection.remove();
			event.preventDefault();
			event.stopImmediatePropagation();
			return;
		}

		// Block ONLY dragging if already detached
		if (_detachedMembers[AID]) {
			return;
		}

		// Offline members are not draggable
		if (player.state !== 0) {
			return;
		}

		// Block dragging out when locked
		if (_preferences.lock) {
			return;
		}

		const startX = event.pageX;
		const startY = event.pageY;
		let isDragging = false;
		const threshold = 5;
		let ghostWrapper = null;
		let ghostInner = null;

		const nodeRect = node.getBoundingClientRect();

		function onMouseMove(moveEvent) {
			if (!isDragging) {
				if (Math.abs(moveEvent.pageX - startX) > threshold || Math.abs(moveEvent.pageY - startY) > threshold) {
					isDragging = true;

					if (ContextMenu) {
						ContextMenu.remove();
					}

					// Create screen-wide drag shield
					ghostWrapper = document.createElement('div');
					ghostWrapper.className = 'drag-shield';
					ghostWrapper.style.cssText =
						'position:fixed; top:0; left:0; right:0; bottom:0; z-index:10000; background:transparent; cursor:grabbing;';

					ghostInner = document.createElement('div');
					ghostInner.id = 'PartyFriends';
					ghostInner.className = 'drag-ghost-wrapper';
					ghostInner.style.cssText =
						'position:absolute; pointer-events:none; opacity:0.6; background:none !important; border:none !important; height:auto !important; width:auto !important;';

					const ghostContent = document.createElement('div');
					ghostContent.className = 'content';
					ghostContent.style.cssText =
						'background:none !important; height:auto !important; width:auto !important;';

					const ghostParty = document.createElement('div');
					ghostParty.className = 'party info-v2';

					const clonedNode = node.cloneNode(true);
					ghostParty.appendChild(clonedNode);
					ghostContent.appendChild(ghostParty);
					ghostInner.appendChild(ghostContent);
					ghostWrapper.appendChild(ghostInner);
					document.body.appendChild(ghostWrapper);

					// Canvas content isn't cloned, redraw if possible
					const srcCanvas = node.querySelector('canvas');
					const dstCanvas = clonedNode.querySelector('canvas');
					if (srcCanvas && dstCanvas) {
						dstCanvas.width = srcCanvas.width;
						dstCanvas.height = srcCanvas.height;
						dstCanvas.getContext('2d').drawImage(srcCanvas, 0, 0);
					}

					ghostInner.style.width = `${node.offsetWidth}px`;
					ghostInner.style.height = `${node.offsetHeight}px`;
				}
			}

			if (isDragging && ghostInner) {
				ghostInner.style.left = `${moveEvent.clientX - (startX - nodeRect.left)}px`;
				ghostInner.style.top = `${moveEvent.clientY - (startY - nodeRect.top)}px`;
				moveEvent.preventDefault();
			}
		}

		function onMouseUp(upEvent) {
			window.removeEventListener('mousemove', onMouseMove);
			window.removeEventListener('mouseup', onMouseUp);

			if (ghostWrapper) {
				ghostWrapper.remove();
				ghostWrapper = null;
				ghostInner = null;
			}

			if (isDragging) {
				upEvent.stopImmediatePropagation();

				const hostEl = Component._host;
				const rect = hostEl.getBoundingClientRect();
				const x = upEvent.pageX;
				const y = upEvent.pageY;

				// Check if dropped outside (with a small 10px buffer)
				if (
					x < rect.left - 10 ||
					x > rect.left + rect.width + 10 ||
					y < rect.top - 10 ||
					y > rect.top + rect.height + 10
				) {
					detachMember(AID, player, x, y);
					Component.saveDetachedMembers();
				}
			}
		}

		window.addEventListener('mousemove', onMouseMove);
		window.addEventListener('mouseup', onMouseUp);
	}

	/**
	 * Find the next available slot for a detached member window.
	 */
	function getNextAvailableSlot() {
		const startX = 0;
		const startY = 100;
		const stepY = 38 + 10;
		let currentY = startY;

		let found = true;
		while (found) {
			found = false;
			for (const aid in _detachedMembers) {
				const ext = _detachedMembers[aid];
				if (ext && ext._host) {
					const posLeft = ext._host.offsetLeft;
					const posTop = ext._host.offsetTop;
					if (Math.abs(posLeft - startX) < 5 && Math.abs(posTop - currentY) < 5) {
						currentY += stepY;
						found = true;
						break;
					}
				}
			}
		}

		return { x: startX, y: currentY };
	}

	/**
	 * Position all detached members into a neat vertical stack
	 */
	function sortDetachedMembers() {
		const startX = 0;
		const startY = 100;
		const stepY = 38 + 10;
		let currentY = startY;

		for (let i = 0, count = _party.length; i < count; i++) {
			const player = _party[i];
			const external = _detachedMembers[player.AID];

			if (external && external._host) {
				external._host.style.left = `${startX}px`;
				external._host.style.top = `${currentY}px`;
				currentY += stepY;
			}
		}

		Component.saveDetachedMembers();
	}

	/**
	 * Create an external mini-window for a member
	 */
	function detachMember(AID, player, x, y, restorePos) {
		if (_detachedMembers[AID]) {
			return;
		}

		const external = PartyMemberExternal.clone(`PartyMemberExternal_${AID}`, true);
		external.name = `PartyMemberExternal_${AID}`;
		UIManager.addComponent(external);

		// Cleanup external window and refresh party list
		external.onRemove = function () {
			delete _detachedMembers[AID];
			delete UIManager.components[external.name];
			const tooltip = document.getElementById('ro-tooltip-party');
			if (tooltip) tooltip.style.display = 'none';
			Component.refreshPartyList();

			if (external._closedByUser) {
				Component.saveDetachedMembers();
			}
		};

		external.setMember(AID, player);
		external.append();

		let initX = x - 75;
		let initY = y - 19;

		if (restorePos) {
			initX = restorePos.x;
			initY = restorePos.y;
		} else {
			const slot = getNextAvailableSlot();
			initX = slot.x;
			initY = slot.y;
		}

		external._host.style.left = `${initX}px`;
		external._host.style.top = `${initY}px`;

		_detachedMembers[AID] = external;
		Component.refreshPartyList();
	}

	/**
	 * Helper to restore a detached member from saved layout.
	 */
	function restoreDetachedMember(player) {
		if (_detachedMembers[player.AID]) {
			return;
		}

		if (!Session.Entity?.display?.name) {
			return;
		}

		if (_savedPositions === null) {
			const key = `PartyFriends_${Session.Entity.display.name}_Detached`;
			const savedStr = localStorage.getItem(key);
			_savedPositions = {};
			try {
				if (savedStr) {
					_savedPositions = JSON.parse(savedStr);
				}
			} catch (e) {
				console.error('[PartyFriendsV1] Failed to parse saved detached members:', e);
			}
		}

		const entry = _savedPositions[player.AID] || _savedPositions[String(player.AID)];
		if (entry) {
			detachMember(player.AID, player, null, null, entry);
		}
	}

	/**
	 * Helper to update the HP bar on a canvas (renewal layout)
	 * @param {Element} node
	 * @param {number} hp
	 * @param {number} maxhp
	 */
	function updateCanvasLife(node, hp, maxhp) {
		const hasLife = hp !== undefined && maxhp !== undefined && maxhp > 0;
		const lifeRatio = hasLife ? hp / maxhp : 0;
		const barVisibility = hasLife ? 'visible' : 'hidden';

		const hpBarContainer = node.querySelector('.hp-bar-container');
		if (hpBarContainer) hpBarContainer.style.visibility = barVisibility;

		const hpEl = node.querySelector('.hp');
		if (hpEl) hpEl.style.visibility = barVisibility;

		if (hasLife) {
			const canvas = node.querySelector('canvas');
			if (canvas) {
				const ctx = canvas.getContext('2d');
				const width = Math.floor(lifeRatio * 75);
				canvas.width = 75;
				canvas.height = 5;
				ctx.clearRect(0, 0, 75, 5);
				ctx.fillStyle = lifeRatio <= 0.25 ? '#ef1010' : '#32cd32';
				ctx.fillRect(0, 0, width, 5);
			}
			if (hpEl) hpEl.textContent = `${hp}/${maxhp}`;
		} else {
			if (hpEl) hpEl.textContent = '';
		}
	}

	/**
	 * Custom RO-style tooltips (global, outside shadow DOM)
	 */
	function onTooltipShow(event) {
		if (event.__tooltipHandled) {
			return;
		}
		event.__tooltipHandled = true;

		const text = this.getAttribute('data-tooltip');
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
		if (event.__tooltipMoved) {
			return;
		}
		event.__tooltipMoved = true;

		const tooltip = document.getElementById('ro-tooltip-party');
		if (tooltip && tooltip.style.display === 'block') {
			tooltip.style.top = `${event.clientY + 15}px`;
			tooltip.style.left = `${event.clientX + 10}px`;
		}
	}

	function onTooltipHide(event) {
		if (event.__tooltipHidden) {
			return;
		}
		event.__tooltipHidden = true;

		const tooltip = document.getElementById('ro-tooltip-party');
		if (tooltip) tooltip.style.display = 'none';
	}

	// ─── Shared interaction handlers ───────────────────────

	/**
	 * Resizing UI
	 */
	function onResize() {
		const host = Component._host;
		const top = host.offsetTop;
		const left = host.offsetLeft;
		let lastWidth = 0;
		let lastHeight = 0;

		function resizing() {
			const extraX = -20;
			const extraY = 25 + 21;

			let w = Math.floor((Mouse.screen.x - left - extraX) / 20);
			let h = Math.floor((Mouse.screen.y - top - extraY) / 20);

			// Maximum and minimum window size
			w = Math.min(Math.max(w, 12), 13);
			h = Math.min(Math.max(h, 6), 12);

			if (w === lastWidth && h === lastHeight) {
				return;
			}

			Component.resize(w, h);
			lastWidth = w;
			lastHeight = h;
		}

		// Start resizing
		const _Interval = setInterval(resizing, 30);

		// Stop resizing on left click
		const onMouseUp = event => {
			if (event.which === 1) {
				clearInterval(_Interval);
				window.removeEventListener('mouseup', onMouseUp);
			}
		};
		window.addEventListener('mouseup', onMouseUp);
	}

	/**
	 * Close the window
	 */
	function onClose() {
		Component._host.style.display = 'none';
		PartyHelper.remove();
	}

	/**
	 * Enable or disable the lock features
	 */
	function onToggleLock() {
		const root = _root();
		_preferences.lock = !_preferences.lock;
		_preferences.save();

		const lockOn = root.querySelector('.lock.on');
		const lockOff = root.querySelector('.lock.off');

		if (_preferences.lock) {
			if (lockOn) lockOn.style.display = 'inline-block';
			if (lockOff) lockOff.style.display = 'none';
		} else {
			if (lockOn) lockOn.style.display = 'none';
			if (lockOff) lockOff.style.display = 'inline-block';
		}
	}

	/**
	 * Move to the other tab (Friend -> Party or Party -> Friend)
	 */
	function onChangeTab() {
		const root = _root();

		_preferences.friend = !_preferences.friend;
		_preferences.save();

		// Show/hide elements based on tab
		const friendEls = root.querySelectorAll('.friend');
		const partyEls = root.querySelectorAll('.party');

		if (_preferences.friend) {
			friendEls.forEach(el => {
				el.style.display = '';
			});
			partyEls.forEach(el => {
				el.style.display = 'none';
			});
		} else {
			friendEls.forEach(el => {
				el.style.display = 'none';
			});
			partyEls.forEach(el => {
				el.style.display = '';
			});

			if (Session.hasParty) {
				const createBtn = root.querySelector('.party.create');
				if (createBtn) createBtn.style.display = 'none';

				if (renewalParty) {
					const sortBtn = root.querySelector('.party.sort');
					if (sortBtn) sortBtn.style.display = '';
				}

				if (!Session.isPartyLeader) {
					const addBtn = root.querySelector('.party.add');
					if (addBtn) addBtn.style.display = 'none';
				}
			} else {
				const addBtn = root.querySelector('.party.add');
				const leaveBtn = root.querySelector('.party.leave');
				if (addBtn) addBtn.style.display = 'none';
				if (leaveBtn) leaveBtn.style.display = 'none';

				if (renewalParty) {
					const sortBtn = root.querySelector('.party.sort');
					if (sortBtn) sortBtn.style.display = 'none';
				}
			}
		}

		root.querySelectorAll('.node').forEach(n => n.classList.remove('selection'));
		_index = -1;
	}

	/**
	 * Ask confirmation to remove a character from the list
	 */
	function onRequestRemoveSelection() {
		if (
			_index < 0 ||
			_preferences.lock ||
			(_preferences.friend && !_friends[_index]) ||
			(!_preferences.friend && !_party[_index])
		) {
			return;
		}

		const text = _preferences.friend ? DB.getMessage(356) : DB.getMessage(363);

		// Are you sure that you want to delete/expel ?
		UIManager.showPromptBox(text, 'ok', 'cancel', () => {
			if (_preferences.friend) {
				Component.onRemoveFriend(_index);
			} else {
				Component.onExpelMember(_party[_index].AID, _party[_index].characterName);
			}
		});
	}

	/**
	 * Add nick name to chatbox
	 * Or open a new conversation window (todo)
	 */
	function onRequestPrivateMessage() {
		if (_index < 0 || _preferences.lock) {
			return;
		}

		const name = _preferences.friend ? _friends[_index].Name : _party[_index].characterName;

		if (renewalParty) {
			WhisperBox.show(name);
			return;
		}

		if (PACKETVER.value >= 20090617) {
			WhisperBox.show(name);
			return;
		}

		const chatRoot = ChatBox._shadow || ChatBox._host;
		if (chatRoot) {
			const usernameInput = chatRoot.querySelector('.username');
			if (usernameInput) usernameInput.value = name;
			const messageInput = chatRoot.querySelector('.message');
			if (messageInput) messageInput.focus();
		}
	}

	/**
	 * Add nick name to chatbox while clicking on player character sprite
	 */
	Component.onOpenChat1to1 = function onOpenChat1to1(name) {
		if (renewalParty) {
			WhisperBox.show(name);
			return;
		}

		if (PACKETVER.value >= 20090617) {
			WhisperBox.show(name);
			return;
		}

		const chatRoot = ChatBox._shadow || ChatBox._host;
		if (chatRoot) {
			const usernameInput = chatRoot.querySelector('.username');
			if (usernameInput) usernameInput.value = name;
			const messageInput = chatRoot.querySelector('.message');
			if (messageInput) messageInput.focus();
		}
	};

	/**
	 * Right click on a character (classic layout)
	 */
	function onRightClickInfoClassic() {
		if (_preferences.lock) {
			return;
		}

		ContextMenu.remove();
		ContextMenu.append();

		if (_preferences.friend) {
			ContextMenu.addElement(DB.getMessage(360), onRequestPrivateMessage);

			if (_friends[_index].GID !== Session.GID) {
				ContextMenu.addElement(DB.getMessage(351), onRequestRemoveSelection);
			}
		} else {
			ContextMenu.addElement(DB.getMessage(129), onRequestInformation);

			if (_party[_index].GID === Session.GID) {
				ContextMenu.addElement(DB.getMessage(2055), onRequestLeaveParty);
			} else {
				ContextMenu.addElement(DB.getMessage(360), onRequestPrivateMessage);

				if (Session.isPartyLeader) {
					ContextMenu.addElement(DB.getMessage(97), onRequestRemoveSelection);
					ContextMenu.addElement(DB.getMessage(1532), onRequestPartyDelegation);
				}
			}
		}
	}

	/**
	 * Request player information
	 */
	function onRequestInformation() {
		if (_preferences.lock) {
			return;
		}

		UIManager.showMessageBox(DB.getMessage(191), 'ok');
	}

	/**
	 * Right click on a character (renewal layout)
	 */
	function onRightClickInfoRenewal(event) {
		if (event) {
			event.stopImmediatePropagation();
			event.preventDefault();
		}

		if (Camera && Camera.rotate) {
			Camera.rotate(false);
		}

		if (_preferences.lock) {
			return;
		}

		onSelectionChangeRenewal.call(this, event);

		if (_index < 0) {
			return;
		}

		ContextMenu.remove();
		ContextMenu.append();

		if (_preferences.friend) {
			const friend = _friends[_index];
			if (!friend) {
				return;
			}

			ContextMenu.addElement(DB.getMessage(360), onRequestPrivateMessage);
			if (friend.GID !== Session.GID) {
				ContextMenu.addElement(DB.getMessage(351), onRequestRemoveSelection);
			}
		} else {
			const player = _party[_index];
			if (!player) {
				return;
			}

			const isMe = player.AID === Session.AID;
			const isLeader = Session.isPartyLeader;
			const isOnline = player.state === 0;

			ContextMenu.addElement(DB.getMessage(98), onOpenMailCreationWindow);

			if (isMe) {
				ContextMenu.addElement(DB.getMessage(2055), onRequestLeaveParty);
			} else {
				ContextMenu.addElement(DB.getMessage(360), onRequestPrivateMessage);

				if (isLeader) {
					ContextMenu.addElement(DB.getMessage(97), onRequestRemoveSelection);
					ContextMenu.addElement(DB.getMessage(1532), onRequestPartyDelegation);
				}
			}

			if (isOnline) {
				ContextMenu.addElement(DB.getMessage(3101), () => {
					detachMember(player.AID, player, Mouse.screen.x, Mouse.screen.y);
					Component.saveDetachedMembers();
				});
				ContextMenu.addElement(DB.getMessage(3102), () => {
					if (_detachedMembers[player.AID]) {
						_detachedMembers[player.AID].remove();
						Component.saveDetachedMembers();
					}
				});
			}
		}
	}

	/**
	 * Request to leave a party
	 */
	function onRequestLeaveParty() {
		if (_preferences.lock) {
			return;
		}

		UIManager.showPromptBox(DB.getMessage(357), 'ok', 'cancel', () => {
			Component.onRequestLeave();
		});
	}

	/**
	 * Request to change party leader
	 */
	function onRequestPartyDelegation() {
		if (_preferences.lock) {
			return;
		}

		UIManager.showPromptBox(DB.getMessage(1532), 'ok', 'cancel', () => {
			Component.onRequestChangeLeader(_party[_index].AID);
		});
	}

	/**
	 * Change selection (click on a friend/party) — classic layout
	 */
	function onSelectionChangeClassic(nodeEl) {
		const root = _root();
		root.querySelectorAll('.content .name').forEach(el => el.classList.remove('selection'));

		const nameEl = nodeEl.querySelector('.name');
		if (nameEl) nameEl.classList.add('selection');

		const parent = nodeEl.parentNode;
		const siblings = parent.querySelectorAll('.node');
		_index = Array.prototype.indexOf.call(siblings, nodeEl);

		if (SkillTargetSelection.intersectEntityId) {
			const entityId = _preferences.friend ? _friends[_index].AID : _party[_index].AID;
			SkillTargetSelection.intersectEntityId(entityId);
		}
	}

	/**
	 * Change selection (click on a friend/party) — renewal layout
	 */
	function onSelectionChangeRenewal() {
		const root = _root();
		root.querySelectorAll('.node').forEach(el => el.classList.remove('selection'));

		const node = this;
		node.classList.add('selection');

		const AID = parseInt(node.dataset.aid, 10);
		_index = -1;

		if (_preferences.friend) {
			const nodes = Array.from(node.parentNode.querySelectorAll('.node'));
			_index = nodes.indexOf(node);
		} else {
			const player = _party.find(member => member.AID == AID);
			_index = _party.indexOf(player);
		}
	}

	/**
	 * Functions to be hooked
	 */
	Component.onRequestPartyCreation = function onRequestPartyCreation() {};
	Component.onRequestAddingMember = function onRequestAddingMember() {};
	Component.onRequestLeave = function onRequestLeave() {};
	Component.onRequestChangeLeader = function onRequestChangeLeader() {};
	Component.onExpelMember = function onExpelMember() {};
	Component.onRemoveFriend = function onRemoveFriend() {};
	Component.onRequestSettingUpdate = function onRequestSettingUpdate() {};

	/**
	 * Open the party info window
	 */
	function onOpenPartyOptionWindow() {
		if (renewalParty) {
			if (_preferences.friend) {
				if (PartyHelper.__active && PartyHelper.getType() === PartyHelper.Type.FRIEND_SETUP) {
					PartyHelper.remove();
					return;
				}

				const whisperPrefs = WhisperBox.preferences;

				PartyHelper.append();
				PartyHelper.setType(PartyHelper.Type.FRIEND_SETUP);
				PartyHelper.setFriendOptions(whisperPrefs);
				return;
			}

			if (PartyHelper.__active && PartyHelper.getType() === PartyHelper.Type.SETUP) {
				PartyHelper.remove();
				return;
			}

			PartyHelper.append();
			PartyHelper.setType(PartyHelper.Type.SETUP);
			PartyHelper.setOptions(_options, Session.isPartyLeader);
			return;
		}

		if (_preferences.lock) {
			return;
		}

		const type =
			_preferences.friend && PACKETVER.value >= 20090617 ? PartyHelper.Type.FRIEND_SETUP : PartyHelper.Type.SETUP;
		if (PartyHelper.__active && PartyHelper.getType() === type) {
			PartyHelper.remove();
			return;
		}
		PartyHelper.append();
		if (type === PartyHelper.Type.FRIEND_SETUP) {
			const whisperPrefs = WhisperBox.preferences;
			PartyHelper.setType(PartyHelper.Type.FRIEND_SETUP);
			PartyHelper.setFriendOptions(whisperPrefs);
		} else {
			PartyHelper.setType(PartyHelper.Type.SETUP);
			PartyHelper.setOptions(_options, Session.isPartyLeader);
		}
	}

	/**
	 * Open the window to invite people
	 */
	function onOpenPartyInviteWindow() {
		if (!renewalParty && _preferences.lock) {
			return;
		}
		if (PartyHelper.__active && PartyHelper.getType() === PartyHelper.Type.INVITE) {
			PartyHelper.remove();
			return;
		}

		PartyHelper.append();
		PartyHelper.setType(PartyHelper.Type.INVITE);
	}

	/**
	 * Open the window to create a party
	 */
	function onOpenPartyCreationWindow() {
		if (!renewalParty && _preferences.lock) {
			return;
		}
		if (PartyHelper.__active && PartyHelper.getType() === PartyHelper.Type.CREATE) {
			PartyHelper.remove();
			return;
		}

		PartyHelper.append();
		PartyHelper.setType(PartyHelper.Type.CREATE);
	}

	/**
	 * Open the window to manage mails
	 */
	function onOpenMailCreationWindow() {
		if (_preferences.lock) {
			return;
		}

		let recipient = '';
		if (_preferences.friend && _friends[_index]) {
			recipient = _friends[_index].Name;
		} else if (!_preferences.friend && _party[_index]) {
			recipient = _party[_index].characterName;
		}

		if (renewalParty) {
			if (recipient) {
				Rodex.requestOpenWriteRodex(recipient);
			}
		} else {
			if (recipient) {
				Mail.replyNewMailFriends(recipient);
			} else {
				Mail.append();
			}
		}
	}

	Component.mouseMode = GUIComponent.MouseMode.STOP;

	/**
	 * Storing Requirement
	 */
	return UIManager.addComponent(Component);
}
