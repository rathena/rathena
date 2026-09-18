/**
 * UI/Components/CharSelect/CharSelectCommon.js
 *
 * Shared factory for the Character Selection windows.
 *
 * Collapses the CharSelect version siblings (V1, V2, V3, V4) into a single
 * createCharSelect(config) factory. Each version file passes its own
 * htmlText/cssText plus capability flags describing its real differences:
 *   - V1 : paginated (3-slot) layout, immediate delete only, no countdown.
 *   - V2 : paginated layout, delete-reservation flow gated by PACKETVER.
 *   - V3 : paginated layout with page balls + per-slot make buttons.
 *   - V4 : grid layout (`gridLayout`) with background-image slots and
 *          duration-based countdown intervals.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import MonsterTable from 'DB/Monsters/MonsterTable.js';
import Preferences from 'Core/Preferences.js';
import Client from 'Core/Client.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Renderer from 'Renderer/Renderer.js';
import Entity from 'Renderer/Entity/Entity.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import StatusConst from 'DB/Status/StatusState.js';
import Camera from 'Renderer/Camera.js';
import UIManager from 'UI/UIManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import 'UI/Elements/Elements.js';
import PACKETVER from 'Network/PacketVerManager.js';

export function createCharSelect(config) {
	const {
		name,
		htmlText,
		cssText,
		gridLayout = false,
		hostHeight = 342,
		defaultMaxSlots = 3 * 9,
		deleteReservation = false,
		packetverGatedDelete = false,
		pageBalls = false
	} = config;

	const Component = new GUIComponent(name, cssText);

	Component.render = () => htmlText;

	/**
	 * @var {Preferences} save where the cursor position is
	 */
	const _preferences = Preferences.get(
		name,
		{
			index: 0
		},
		1.0
	);

	/**
	 * @var {number} max slots
	 */
	let _maxSlots = defaultMaxSlots;

	/**
	 * var {Array} list of characters
	 */
	const _list = [];

	/**
	 * @var {Array} list of characters (index by slot)
	 */
	const _slots = [];

	/**
	 * @var {Array} list of entities (index by slot)
	 */
	const _entitySlots = [];

	/**
	 * @var {number} selector index
	 */
	let _index = 0;

	/**
	 * @var {Array} canvas context
	 */
	const _ctx = [];

	/**
	 * var {number} sex
	 */
	let _sex = 0;

	/**
	 * var {boolean} disable input
	 */
	let _disable_UI = false;

	/**
	 * Grid (V4) background/countdown state
	 */
	let img = 0;
	let _curindex = 0;
	let shouldRunBackgroundChange = false;
	let countdownInterval;
	let _bgInterval = null;

	const render = gridLayout ? renderGrid : renderPaginated;
	const moveCursorTo = gridLayout ? moveCursorToGrid : moveCursorToPaginated;

	/**
	 * Initialize UI
	 */
	Component.init = function init() {
		const root = this.getRoot();

		if (gridLayout) {
			// Bind buttons
			root.querySelector('.ok').addEventListener('click', connect);
			root.querySelector('.cancel').addEventListener('click', cancel);
			root.querySelector('.delete').addEventListener('click', reserve);
			root.querySelector('.canceldelete').addEventListener('click', removedelete);
			root.querySelector('.finaldelete').addEventListener('click', suppress);

			// Bind canvas
			for (let i = 0; i < 15; i++) {
				const slot = root.querySelector(`#slot${i}`);
				if (slot) {
					slot.addEventListener('mousedown', genericCanvasDown(i));
				}
			}

			root.querySelectorAll('canvas').forEach(canvas => {
				canvas.addEventListener('dblclick', () => {
					if (_slots[_index]) {
						connect();
					} else {
						create();
					}
				});
				_ctx.push(canvas.getContext('2d'));
			});

			// Load charinfo panel background
			Client.loadFile(`${DB.INTERFACE_PATH}select_character_ver3/img_info.bmp`, dataURI => {
				root.querySelector('.charinfo').style.backgroundImage = `url(${dataURI})`;
			});

			// Load default slot backgrounds
			for (let i = 0; i < 15; i++) {
				const slotCanvas = root.querySelector(`#slot${i}`);
				if (slotCanvas) {
					Client.loadFile(`${DB.INTERFACE_PATH}select_character_ver3/img_slot2_normal.bmp`, dataURI => {
						slotCanvas.style.backgroundImage = `url(${dataURI})`;
					});
				}
			}

			return;
		}

		this.draggable();

		// Bind buttons
		root.querySelector('.ok').addEventListener('click', connect);
		root.querySelector('.cancel').addEventListener('click', cancel);
		root.querySelector('.make').addEventListener('click', create);
		root.querySelector('.delete').addEventListener('click', deleteReservation ? reserve : suppress);

		if (deleteReservation) {
			root.querySelector('.canceldelete').addEventListener('click', removedelete);
			root.querySelector('.finaldelete').addEventListener('click', suppress);
		}

		root.querySelector('.arrow.left').addEventListener('mousedown', genericArrowDown(-1));
		root.querySelector('.arrow.right').addEventListener('mousedown', genericArrowDown(+1));

		// Bind canvas
		root.querySelector('.slot1').addEventListener('mousedown', genericCanvasDown(0));
		root.querySelector('.slot2').addEventListener('mousedown', genericCanvasDown(1));
		root.querySelector('.slot3').addEventListener('mousedown', genericCanvasDown(2));

		if (pageBalls) {
			root.querySelector('.make1').addEventListener('mousedown', e => {
				genericCanvasDown(0)(e);
				create();
			});
			root.querySelector('.make2').addEventListener('mousedown', e => {
				genericCanvasDown(1)(e);
				create();
			});
			root.querySelector('.make3').addEventListener('mousedown', e => {
				genericCanvasDown(2)(e);
				create();
			});
		}

		root.querySelectorAll('canvas').forEach(canvas => {
			canvas.addEventListener('dblclick', () => {
				if (_slots[_index]) {
					connect();
				} else {
					create();
				}
			});
			_ctx.push(canvas.getContext('2d'));
		});
	};

	/**
	 * Once append to body
	 */
	Component.onAppend = function onAppend() {
		if (gridLayout) {
			Component.updateCharSlot();

			startCountdownInterval();

			// Update values
			moveCursorTo(_index);
			_bgInterval = setInterval(changeBackgroundEverySecond, 250);
			// Start rendering
			Renderer.render(render);
			return;
		}

		const root = this.getRoot();

		this._host.style.top = `${(Renderer.height - hostHeight) / 2}px`;
		this._host.style.left = `${(Renderer.width - 576) / 2}px`;

		_index = _preferences.index;

		root.querySelector('.slotinfo .number').textContent = `${_list.length} / ${_maxSlots}`;
		if (!pageBalls) {
			root.querySelector('.pageinfo .count').textContent = _maxSlots / 3;
		}

		// Update values
		moveCursorTo(_index);

		// Start rendering
		Renderer.render(render);
	};

	/**
	 * Stop rendering
	 */
	Component.onRemove = function onRemove() {
		if (gridLayout) {
			if (_bgInterval) {
				clearInterval(_bgInterval);
				_bgInterval = null;
			}
			stopCountdownInterval();
			_preferences.index = _index;
			_preferences.save();
			Renderer.stop();
			return;
		}

		const root = this.getRoot();

		_preferences.index = _index;
		_preferences.save();
		if (deleteReservation) {
			root.querySelectorAll('.timedelete').forEach(el => {
				el.style.display = 'none';
				el.textContent = '';
			});
		}
		Renderer.stop();
	};

	/**
	 * Bind Key events
	 *
	 * @param {object} event
	 */
	Component.onKeyDown = function onKeyDown(event) {
		if (this._host.style.display === 'none') {
			return true;
		}
		switch (event.which) {
			case KEYS.ESCAPE:
				cancel();
				break;

			case KEYS.LEFT:
				if (gridLayout) {
					moveCursorTo(_index - 1 > _list.length - 1 ? _list.length - 1 : _index - 1 < 0 ? 0 : _index - 1);
				} else {
					moveCursorTo(_index - 1);
				}
				break;

			case KEYS.RIGHT:
				if (gridLayout) {
					moveCursorTo(_index + 1 > _list.length - 1 ? _list.length - 1 : _index + 1 < 0 ? 0 : _index + 1);
				} else {
					moveCursorTo(_index + 1);
				}
				break;

			case KEYS.SUPR:
				if (_slots[_index]) {
					suppress();
				}
				break;

			case KEYS.ENTER:
				if (_slots[_index]) {
					connect();
				} else {
					create();
				}
				break;

			default:
				return true;
		}

		event.stopImmediatePropagation();
		return false;
	};

	/**
	 * Add players to window
	 *
	 * @param {object} pkt - packet structure
	 */
	Component.setInfo = function setInfo(pkt) {
		if (gridLayout) {
			Component.clearAllSlots();

			_maxSlots = Math.floor(pkt.TotalSlotNum + pkt.PremiumStartSlot || 15); // default 15 ?
			_sex = pkt.sex;

			if (pkt.charInfo) {
				for (let i = 0, count = pkt.charInfo.length; i < count; ++i) {
					Component.addCharacter(pkt.charInfo[i]);
				}
			}

			moveCursorTo(_index);
			return;
		}

		const root = this.getRoot();

		_maxSlots = Math.floor(pkt.TotalSlotNum + pkt.PremiumStartSlot || 9); // default 9 ?
		_sex = pkt.sex;
		_slots.length = 0;
		_entitySlots.length = 0;
		_list.length = 0;

		if (pkt.charInfo) {
			for (let i = 0, count = pkt.charInfo.length; i < count; ++i) {
				Component.addCharacter(pkt.charInfo[i]);

				// Guess the max slot
				// required if the client is < 20100413 and have more than 9 slots
				_maxSlots = Math.max(_maxSlots, Math.floor(pkt.charInfo[i].CharNum / 3 + 1) * 3);
			}
		}

		root.querySelector('.slotinfo .number').textContent = `${_list.length} / ${_maxSlots}`;
		if (!pageBalls) {
			root.querySelector('.pageinfo .count').textContent = _maxSlots / 3;
		}

		moveCursorTo(_index);
	};

	/**
	 * Answer from server to delete a character
	 *
	 * @param {number} error id
	 */
	Component.deleteAnswer = function deleteAnswer(error) {
		this.on('keydown');

		if (gridLayout) {
			switch (error) {
				// Do nothing, just re-set the keydown
				case -1:
				case -2:
					return;

				// Success (clean up character)
				case 1: {
					delete _slots[_index];
					delete _entitySlots[_index];

					if (_preferences.index === _index) {
						_preferences.index = 0;
					}

					let i = 0;
					let count = _list.length;

					while (i < count) {
						if (_list[i].CharNum === _index) {
							_list.splice(i, 1);
							--count;
						} else {
							i++;
						}
					}

					Component.updateCharSlot(_index);

					// Refresh UI
					moveCursorTo(_index);
					return;
				}

				default: // Others error ?
				case 0:
				case 2: // 2: Due to system settings can not be deleted.
				case 6: // 6: Name does not match.
					UIManager.showMessageBox(DB.getMessage(1821), 'ok');
					return;
				case 3: // 3: A database error occurred.
					UIManager.showMessageBox(DB.getMessage(1817), 'ok');
					return;
				case 4: // 4: Deleting not yet possible time.
					UIManager.showMessageBox(DB.getMessage(1820), 'ok');
					return;
				case 5: // 5: Date of birth do not match.
					UIManager.showMessageBox(DB.getMessage(1822), 'ok');
					return;
				case 7: // 7: Character Deletion has failed because you have entered an incorrect e-mail address.
					UIManager.showMessageBox(DB.getMessage(301), 'ok');
					return;
			}
		}

		const useNew = deleteReservation && (!packetverGatedDelete || PACKETVER.value >= 20100803);

		if (useNew) {
			const root = this.getRoot();

			switch (error) {
				// Do nothing, just re-set the keydown
				case -1:
				case -2:
					return;

				// Success (clean up character)
				case 1: {
					delete _slots[_index];
					delete _entitySlots[_index];

					let i = 0;
					let count = _list.length;

					while (i < count) {
						if (_list[i].CharNum === _index) {
							_list.splice(i, 1);
							--count;
						} else {
							i++;
						}
					}

					// Refresh UI
					moveCursorTo(_index);
					root.querySelector('.slotinfo .number').textContent = `${_list.length} / ${_maxSlots}`;
					return;
				}

				default: // Others error ?
				case 0:
				case 2: // 2: Due to system settings can not be deleted.
				case 6: // 6: Name does not match.
					UIManager.showMessageBox(DB.getMessage(1821), 'ok');
					return;
				case 3: // 3: A database error occurred.
					UIManager.showMessageBox(DB.getMessage(1817), 'ok');
					return;
				case 4: // 4: Deleting not yet possible time.
					UIManager.showMessageBox(DB.getMessage(1820), 'ok');
					return;
				case 5: // 5: Date of birth do not match.
					UIManager.showMessageBox(DB.getMessage(1822), 'ok');
					return;
				case 7: // 7: Character Deletion has failed because you have entered an incorrect e-mail address.
					UIManager.showMessageBox(DB.getMessage(301), 'ok');
					return;
			}
		} else {
			switch (error) {
				// Do nothing, just re-set the keydown
				case -2:
					return;

				// Success (clean up character)
				case -1: {
					const root = this.getRoot();

					delete _slots[_index];
					delete _entitySlots[_index];

					let i = 0;
					let count = _list.length;

					while (i < count) {
						if (_list[i].CharNum === _index) {
							_list.splice(i, 1);
							--count;
						} else {
							i++;
						}
					}

					// Refresh UI
					moveCursorTo(_index);
					root.querySelector('.slotinfo .number').textContent = `${_list.length} / ${_maxSlots}`;
					return;
				}

				default: // Others error ?
				case 0: // Incorrect adress email
					UIManager.showMessageBox(DB.getMessage(301), 'ok');
					break;
			}
		}
	};

	/**
	 * Adding a Character to the list
	 *
	 * @param {object} character data
	 */
	Component.addCharacter = function addCharacter(character) {
		if (!('sex' in character) || character.sex === 99) {
			character.sex = _sex;
		}

		if (gridLayout) {
			_list.push(character);
			_slots[character.CharNum] = character;

			_entitySlots[character.CharNum] = new Entity();
			_entitySlots[character.CharNum].set(character);
			_entitySlots[character.CharNum].effectState =
				_entitySlots[character.CharNum]._effectState & ~StatusConst.EffectState.INVISIBLE;
			_entitySlots[character.CharNum].hideShadow = true;

			Component.updateCharSlot(character.CharNum);
			return;
		}

		//Adjust from remaining time to fixed datetime
		if (deleteReservation && character.DeleteDate) {
			const now = Math.floor(Date.now() / 1000);
			const timer =
				(PACKETVER.value > 20130000 && PACKETVER.value <= 20141022) || PACKETVER.value >= 20150513
					? character.DeleteDate + now
					: character.DeleteDate;
			character.DeleteDate = timer;
		}

		_list.push(character);
		_slots[character.CharNum] = character;

		_entitySlots[character.CharNum] = new Entity();
		_entitySlots[character.CharNum].set(character);
		_entitySlots[character.CharNum].effectState =
			_entitySlots[character.CharNum]._effectState & ~StatusConst.EffectState.INVISIBLE;

		if (deleteReservation && (!packetverGatedDelete || PACKETVER.value >= 20100803)) {
			if (_slots[character.CharNum].DeleteDate && Math.floor(_index / 3) === Math.floor(character.CharNum / 3)) {
				const root = Component.getRoot();
				const slotNum = ((character.CharNum + _maxSlots) % _maxSlots) + 1;
				const countdown = root.querySelector(`.timedelete.slot${slotNum}`);
				const entity = _entitySlots[character.CharNum];
				if (countdown) {
					countdown.dataset.datetime = _slots[character.CharNum].DeleteDate;
					countdown.textContent = formatDatetime(_slots[character.CharNum].DeleteDate);
					countdown.style.display = 'block';
				}
				entity.setAction({
					action: entity.ACTION.SIT,
					frame: 0,
					play: true,
					repeat: true
				});
			}
		}
	};

	/**
	 * Disable or Enable the UI.
	 *
	 * @param {boolean}
	 */
	Component.setUIEnabled = function setUIEnabled(value) {
		_disable_UI = !value;
	};

	/**
	 * Callback to use
	 */
	Component.onExitRequest = function onExitRequest() {};
	Component.onDeleteRequest = function onDeleteRequest() {};
	Component.onCreateRequest = function onCreateRequest() {};
	Component.onConnectRequest = function onConnectRequest() {};
	if (deleteReservation) {
		Component.onDeleteReqDelay = function onDeleteReqDelay() {};
		Component.onCancelDeleteRequest = function onCancelDeleteRequest() {};
	}

	/**
	 * Generic method to handle mousedown on arrow
	 *
	 * @param {number} value to move
	 */
	function genericArrowDown(value) {
		return event => {
			moveCursorTo((_index + _maxSlots + value) % _maxSlots);
			event.stopImmediatePropagation();
			return false;
		};
	}

	/**
	 * Generic method to handle mousedown on canvas
	 *
	 * @param {number} value to move
	 */
	function genericCanvasDown(value) {
		return event => {
			if (gridLayout) {
				moveCursorTo(value);
			} else {
				moveCursorTo(Math.floor(_index / 3) * 3 + value);
			}
			event.stopImmediatePropagation();
			return false;
		};
	}

	/**
	 * Press "cancel" or ESCAPE key
	 */
	function cancel() {
		if (_disable_UI === false) {
			if (gridLayout) {
				UIManager.showPromptBox(
					DB.getMessage(17),
					'ok',
					'cancel',
					() => {
						Component.onExitRequest();
						Component.clearAllSlots();
					},
					null
				);
				stopCountdownInterval();
			} else {
				UIManager.showPromptBox(
					DB.getMessage(17),
					'ok',
					'cancel',
					() => {
						Component.onExitRequest();
					},
					null
				);
			}
		}
	}

	/**
	 * Jumping to Character creation window
	 */
	function create() {
		if (_disable_UI === false) {
			Component.onCreateRequest(_index);
		}
	}

	/**
	 * Select Player, connect
	 */
	function connect() {
		if (_disable_UI === false) {
			if (gridLayout) {
				if (_slots[_index] && !_slots[_index].DeleteDate) {
					_preferences.index = _index;
					_preferences.save();
					Component.onConnectRequest(_slots[_index]);
					stopCountdownInterval();
				}
			} else if (_slots[_index]) {
				_preferences.index = _index;
				_preferences.save();
				Component.onConnectRequest(_slots[_index]);
			}
		}
	}

	/**
	 * Request to delete a character
	 */
	function reserve() {
		if (_disable_UI === false) {
			if (_slots[_index]) {
				Component.off('keydown');
				Component.onDeleteReqDelay(_slots[_index].GID);
			}
		}
	}

	/**
	 * Delete a character
	 */
	function suppress() {
		if (_disable_UI === false) {
			if (_slots[_index]) {
				Component.off('keydown');
				Component.onDeleteRequest(_slots[_index].GID);
			}
		}
	}

	/* ------------------------------------------------------------------ *
	 * Delete-reservation flow (V2 / V3 / V4)
	 * ------------------------------------------------------------------ */

	if (deleteReservation) {
		/**
		 * Result of Request in Deleting the Character
		 *
		 * @param {object} pkt - packet structure
		 */
		Component.reqdeleteAnswer = gridLayout
			? function reqdeleteAnswer(pkt) {
					this.on('keydown');
					const deleteReservedDate = pkt.DeleteReservedDate;
					const result = typeof pkt.Result === 'undefined' ? -1 : pkt.Result;
					const info = _slots[_index];

					switch (result) {
						case 0: // 0: An unknown error has occurred.
							return;

						case 1: {
							// 1: none/success
							const now = Math.floor(Date.now() / 1000);
							info.DeleteDate = deleteReservedDate + now;
							requestdelete(_index, deleteReservedDate);
							break;
						}

						case 3: // 3: A database error occurred.
							return;

						case 4: // 4: To delete a character you must withdraw from the guild.
							UIManager.showMessageBox(DB.getMessage(1818), 'ok');
							break;
						case 5: // 5: To delete a character you must withdraw from the party.
							UIManager.showMessageBox(DB.getMessage(1819), 'ok');
							break;

						default:
							return;
					}
				}
			: function reqdeleteAnswer(pkt) {
					this.on('keydown');
					const now = Math.floor(Date.now() / 1000);
					const result = typeof pkt.Result === 'undefined' ? -1 : pkt.Result;
					const info = _slots[_index];

					switch (result) {
						case 0: // 0: An unknown error has occurred.
							return;

						case 1: {
							// 1: none/success
							//Adjust from remaining time to fixed datetime
							const timer =
								(PACKETVER.value > 20130000 && PACKETVER.value <= 20141022) ||
								PACKETVER.value >= 20150513
									? pkt.DeleteReservedDate + now
									: pkt.DeleteReservedDate;
							info.DeleteDate = timer;
							requestdelete(_index, timer);
							break;
						}

						case 3: // 3: A database error occurred.
							return;

						case 4: // 4: To delete a character you must withdraw from the guild.
							UIManager.showMessageBox(DB.getMessage(1818), 'ok');
							break;
						case 5: // 5: To delete a character you must withdraw from the party.
							UIManager.showMessageBox(DB.getMessage(1819), 'ok');
							break;

						default:
							return;
					}
				};
	}

	/**
	 * When successfully requested for character deletion
	 * Update UI and add timer
	 */
	function requestdelete(index, timer) {
		const root = Component.getRoot();

		if (gridLayout) {
			// Make it sit
			_entitySlots[index].action = 2;

			// Add the timer
			const countdown = root.querySelector(`.timedelete.slot${index}`);
			if (countdown) {
				countdown.dataset.duration = timer;
				countdown.classList.remove('hidden');
				countdown.style.display = 'block';
			}

			// Adjust the buttons
			root.querySelector('.delete').style.display = 'none';
			root.querySelector('.canceldelete').style.display = 'block';
			root.querySelector('.finaldelete').style.display = 'block';
			return;
		}

		const entity = _entitySlots[index];
		let action;

		const countdown = root.querySelector(`.timedelete.slot${(index % 3) + 1}`);
		if (countdown) {
			countdown.dataset.datetime = timer;
			countdown.textContent = formatDatetime(timer);
			countdown.style.display = 'block';
			if (Math.floor(Date.now() / 1000) > timer) {
				countdown.classList.remove('waitdelete');
				countdown.classList.add('candelete');
				action = entity.ACTION.DIE;
			} else {
				countdown.classList.remove('candelete');
				countdown.classList.add('waitdelete');
				action = entity.ACTION.SIT;
			}
		}

		// Set action
		entity.action = action;

		// Adjust the buttons
		root.querySelector('.delete').style.display = 'none';
		root.querySelector('.canceldelete').style.display = 'block';
		if (Math.floor(Date.now() / 1000) > timer) {
			root.querySelector('.finaldelete').style.display = 'block';
		} else {
			root.querySelector('.finaldelete').style.display = 'none';
		}
	}

	/**
	 * Cancel reservation of character for deletion
	 * Update UI and remove timer
	 */
	function removedelete() {
		if (_slots[_index]) {
			const root = Component.getRoot();

			// Delete here as well? Though server should tell us this
			_slots[_index].DeleteDate = 0;

			if (gridLayout) {
				// Make it stand
				_entitySlots[_index].action = 0;
				render();

				// Remove the timer
				const countdown = root.querySelector(`.timedelete.slot${_index}`);
				if (countdown) {
					countdown.dataset.duration = 0;
					countdown.classList.add('hidden');
					countdown.style.display = 'none';
				}

				// Adjust the buttons
				root.querySelector('.canceldelete').style.display = 'none';
				root.querySelector('.finaldelete').style.display = 'none';
				root.querySelector('.delete').style.display = 'block';

				// Send request to the server
				Component.onCancelDeleteRequest(_slots[_index].GID);
				return;
			}

			// Make it stand
			_entitySlots[_index].action = _entitySlots[_index].ACTION.READYFIGHT;
			render();

			// Remove the timer
			const countdown = root.querySelector(`.timedelete.slot${(_index % 3) + 1}`);
			countdown.dataset.datetime = 0;
			countdown.textContent = formatDatetime('');
			countdown.style.display = 'none';

			// Adjust the buttons
			root.querySelector('.canceldelete').style.display = 'none';
			root.querySelector('.finaldelete').style.display = 'none';
			root.querySelector('.delete').style.display = 'block';

			// Send request to the server
			Component.onCancelDeleteRequest(_slots[_index].GID);
		}
	}

	/* ------------------------------------------------------------------ *
	 * Paginated variant (V1 / V2 / V3)
	 * ------------------------------------------------------------------ */

	/**
	 * Draw a page selection ball (V3)
	 */
	function drawBall(btnContainer, index, sel) {
		const btn = document.createElement('button');
		btn.className = `btn_pageinfo btn_pageinfo${index}`;
		btn.style.border = '0';
		btn.style.width = '8px';
		btn.style.height = '8px';
		btn.style.backgroundColor = 'transparent';
		btn.style.backgroundRepeat = 'no-repeat';
		btn.style.cursor = 'pointer';

		const imgEl = document.createElement('img');
		imgEl.width = 8;

		const imagePath = sel ? 'select_character/page_ball_fill.bmp' : 'select_character/page_ball_empty.bmp';
		Client.loadFile(DB.INTERFACE_PATH + imagePath, data => {
			btn.style.backgroundImage = `url("${data}")`;
		});

		btn.addEventListener('click', () => {
			moveCursorTo((index - 1) * 3);
		});

		btnContainer.appendChild(btn);
		btnContainer.appendChild(imgEl);
	}

	/**
	 * Format delay date time (V2 / V3)
	 */
	function formatDatetime(epoch) {
		const datetime = new Date(0);
		datetime.setSeconds(epoch);

		const month = datetime.getMonth() + 1;
		const day = datetime.getDate();
		const hours = datetime.getHours();
		const minutes = datetime.getMinutes();
		const seconds = datetime.getSeconds();

		const formattedDatetime = DB.getMessage(2097)
			.replace('%d', `${month}`)
			.replace('%d', `${day}`)
			.replace('%d', `${hours}`)
			.replace('%d', `${minutes}`)
			.replace('%d', `${seconds}`);

		return formattedDatetime;
	}

	/**
	 * Move cursor, update window value (V1 / V2 / V3)
	 *
	 * @param {number} index
	 */
	function moveCursorToPaginated(index) {
		const root = Component.getRoot();
		const charinfo = root.querySelector('.charinfo');

		// Set the last entity to idle
		let entity = _entitySlots[_index];
		let info = _slots[_index];
		let action;
		if (entity) {
			if (deleteReservation && info.DeleteDate && (!packetverGatedDelete || PACKETVER.value >= 20100803)) {
				action = entity.ACTION.SIT;
			} else {
				action = entity.ACTION.IDLE;
			}

			entity.setAction({
				action: action,
				frame: 0,
				play: true,
				repeat: true
			});
		}

		// Move
		_index = (index + _maxSlots) % _maxSlots;
		const boxSelect = root.querySelector('.box_select');
		boxSelect.classList.remove('slot1', 'slot2', 'slot3');
		boxSelect.classList.add(`slot${(_index % 3) + 1}`);

		// Set page
		if (pageBalls) {
			const pagebtn = root.querySelector('.pageinfo .pagebtn');
			pagebtn.textContent = '';

			for (let i = 1; i <= _maxSlots / 3; i++) {
				drawBall(pagebtn, i, Math.floor(_index / 3) + 1 === i);
			}
			// pageinfo position center
			root.querySelector('.pageinfo').style.left = `${576 / 2 - (_maxSlots / 3) * 8}px`;

			// show make add button
			let mix = (index + 1) % 3 === 0 ? index + 1 - 3 : index + 1 - ((index + 1) % 3);
			mix = mix >= _maxSlots ? 0 : mix;
			for (let i = 1; i <= 3; i++) {
				root.querySelector(`.make${i}`).style.display = 'none';
				if (!_entitySlots[mix + (i - 1)]) {
					root.querySelector(`.make${i}`).style.display = 'block';
				}
			}
		} else {
			root.querySelector('.pageinfo .current').textContent = Math.floor(_index / 3) + 1;
		}

		// Update page deltimes
		if (deleteReservation) {
			for (let i = 0; i < 3; i++) {
				const tmpIndex = _index - (_index % 3) + i;
				info = _slots[tmpIndex];
				entity = _entitySlots[tmpIndex];
				const countdown = root.querySelector(`.timedelete.slot${(tmpIndex % 3) + 1}`);

				if (info && entity) {
					if (info.DeleteDate && (!packetverGatedDelete || PACKETVER.value >= 20100803)) {
						countdown.dataset.datetime = info.DeleteDate;
						countdown.textContent = formatDatetime(info.DeleteDate);
						countdown.style.display = 'block';
						if (Math.floor(Date.now() / 1000) > info.DeleteDate) {
							countdown.classList.remove('waitdelete');
							countdown.classList.add('candelete');
						} else {
							countdown.classList.remove('candelete');
							countdown.classList.add('waitdelete');
						}
						entity.action = entity.ACTION.SIT;
					} else {
						countdown.dataset.datetime = 0;
						countdown.textContent = formatDatetime('');
						countdown.style.display = 'none';
						entity.action = entity.ACTION.IDLE;
					}
				} else {
					countdown.dataset.datetime = 0;
					countdown.textContent = formatDatetime('');
					countdown.style.display = 'none';
				}
			}
		}

		// Not found, just clean up.
		entity = _entitySlots[_index];
		if (!entity) {
			charinfo.querySelectorAll('div').forEach(div => {
				div.textContent = '';
			});
			root.querySelector('.make').style.display = 'block';
			root.querySelector('.delete').style.display = 'none';
			if (deleteReservation) {
				root.querySelector('.canceldelete').style.display = 'none';
				root.querySelector('.finaldelete').style.display = 'none';
			}
			root.querySelector('.ok').style.display = 'none';
			return;
		}

		info = _slots[_index];
		// Bind new value
		if (deleteReservation && info.DeleteDate && (!packetverGatedDelete || PACKETVER.value >= 20100803)) {
			root.querySelector('.delete').style.display = 'none';
			root.querySelector('.canceldelete').style.display = 'block';
			if (Math.floor(Date.now() / 1000) > info.DeleteDate) {
				root.querySelector('.finaldelete').style.display = 'block';
				action = entity.ACTION.DIE;
			} else {
				root.querySelector('.finaldelete').style.display = 'none';
				action = entity.ACTION.SIT;
			}
			root.querySelector('.make').style.display = 'none';
			root.querySelector('.ok').style.display = 'block';
		} else {
			if (deleteReservation) {
				if (!packetverGatedDelete || PACKETVER.value >= 20100803) {
					root.querySelector('.delete').style.display = 'block';
					root.querySelector('.canceldelete').style.display = 'none';
					root.querySelector('.finaldelete').style.display = 'none';
				} else {
					root.querySelector('.delete').style.display = 'none';
					root.querySelector('.canceldelete').style.display = 'none';
					root.querySelector('.finaldelete').style.display = 'block';
				}
				root.querySelector('.make').style.display = 'none';
			} else {
				root.querySelector('.make').style.display = 'none';
				root.querySelector('.delete').style.display = 'block';
			}
			root.querySelector('.ok').style.display = 'block';
			action = entity.ACTION.READYFIGHT;
		}

		// Animate the character
		entity.setAction({
			action: action,
			frame: 0,
			play: true,
			repeat: true
		});

		charinfo.querySelector('.name').textContent = info.name;
		charinfo.querySelector('.job').textContent = MonsterTable[info.job] || '';
		charinfo.querySelector('.lvl').textContent = info.level;
		charinfo.querySelector('.exp').textContent = info.exp;
		charinfo.querySelector('.hp').textContent = info.hp;
		charinfo.querySelector('.sp').textContent = info.sp;
		charinfo.querySelector('.map').textContent = DB.getMapName(info.lastMap, '') || '';
		charinfo.querySelector('.str').textContent = info.Str;
		charinfo.querySelector('.agi').textContent = info.Agi;
		charinfo.querySelector('.vit').textContent = info.Vit;
		charinfo.querySelector('.int').textContent = info.Int;
		charinfo.querySelector('.dex').textContent = info.Dex;
		charinfo.querySelector('.luk').textContent = info.Luk;
	}

	/**
	 * Render sprites to canvas (V1 / V2 / V3)
	 */
	function renderPaginated() {
		Camera.direction = 4;
		const idx = Math.floor(_index / 3) * 3;
		const count = _ctx.length;

		for (let i = 0; i < count; ++i) {
			_ctx[i].clearRect(0, 0, _ctx[i].canvas.width, _ctx[i].canvas.height);

			if (_entitySlots[idx + i]) {
				SpriteRenderer.bind2DContext(_ctx[i], 63, 130);
				_entitySlots[idx + i].renderEntity();
			}
		}
	}

	/* ------------------------------------------------------------------ *
	 * Grid variant (V4)
	 * ------------------------------------------------------------------ */

	/**
	 * Format delay duration (V4)
	 */
	function formatDuration(seconds) {
		const hours = Math.floor(seconds / 3600);
		const minutes = Math.floor((seconds % 3600) / 60);
		const remainingSeconds = seconds % 60;

		const replacer = DB.getMessage(3349).includes('%d') ? '%d' : '%02d';

		return DB.getMessage(3349)
			.replace(replacer, hours.toString().padStart(2, '0'))
			.replace(replacer, minutes.toString().padStart(2, '0'))
			.replace(replacer, remainingSeconds.toString().padStart(2, '0'));
	}

	/**
	 * Countdown for delay in deletion (V4)
	 */
	function updateAllVisibleCountdowns() {
		const root = Component.getRoot();
		const visibleCountdowns = root.querySelectorAll('.timedelete:not(.hidden)');

		visibleCountdowns.forEach(countdownDiv => {
			const deleteReservedDuration = parseInt(countdownDiv.dataset.duration, 10);
			const updatedDuration = Math.max(0, deleteReservedDuration - 1);

			countdownDiv.textContent = formatDuration(updatedDuration);
			if (updatedDuration > 0) {
				countdownDiv.style.color = 'red';
			} else {
				countdownDiv.style.color = 'blue';
			}

			countdownDiv.dataset.duration = updatedDuration.toString();
		});
	}

	/**
	 * Start the countdown update interval only when in grid UI
	 */
	function startCountdownInterval() {
		if (!countdownInterval) {
			countdownInterval = setInterval(updateAllVisibleCountdowns, 1000);
		}
	}

	/**
	 * Stop the countdown update interval
	 */
	function stopCountdownInterval() {
		if (countdownInterval) {
			clearInterval(countdownInterval);
			countdownInterval = null;
		}
	}

	/**
	 * Move cursor, update window value (V4)
	 *
	 * @param {number} index
	 */
	function moveCursorToGrid(index) {
		const root = Component.getRoot();
		const charinfo = root.querySelector('.charinfo');

		const prevIndex = _index;
		let entity = _slots[_index];
		shouldRunBackgroundChange = false;

		if (entity) {
			Client.loadFile(`${DB.INTERFACE_PATH}select_character_ver3/img_slot_normal.bmp`, dataURI => {
				const prevSlot = root.querySelector(`#slot${prevIndex}`);
				if (prevSlot) {
					prevSlot.style.backgroundImage = `url(${dataURI})`;
				}
			});
		}

		const slotIndex = (_index = index > _maxSlots ? _maxSlots : index < 0 ? 0 : index);

		// Not found, just clean up.
		entity = _slots[_index];
		if (!entity) {
			charinfo.querySelectorAll('div').forEach(div => {
				div.textContent = '';
			});
			root.querySelector('.delete').style.display = 'none';
			root.querySelector('.canceldelete').style.display = 'none';
			root.querySelector('.finaldelete').style.display = 'none';
			root.querySelector('.ok').style.display = 'none';
			const countdown = root.querySelector(`.timedelete.slot${_index}`);
			if (countdown) {
				countdown.dataset.duration = 0;
				countdown.classList.add('hidden');
				countdown.style.display = 'none';
			}
			return;
		} else {
			_curindex = slotIndex;
			shouldRunBackgroundChange = true;
		}

		if (shouldRunBackgroundChange === true) {
			changeBackgroundEverySecond();
		}

		const info = _slots[_index];
		// Bind new value
		if (info.DeleteDate) {
			root.querySelector('.delete').style.display = 'none';
			root.querySelector('.canceldelete').style.display = 'block';
			root.querySelector('.finaldelete').style.display = 'block';
		} else {
			root.querySelector('.canceldelete').style.display = 'none';
			root.querySelector('.finaldelete').style.display = 'none';
			root.querySelector('.delete').style.display = 'block';
		}

		root.querySelector('.ok').style.display = 'block';

		charinfo.querySelector('.map').textContent = DB.getMapName(info.lastMap, '') || '';
		charinfo.querySelector('.job').textContent = MonsterTable[info.job] || '';
		charinfo.querySelector('.lvl').textContent = info.level;
		charinfo.querySelector('.exp').textContent = info.exp;
		charinfo.querySelector('.hp').textContent = info.hp;
		charinfo.querySelector('.sp').textContent = info.sp;
		charinfo.querySelector('.str').textContent = info.Str;
		charinfo.querySelector('.agi').textContent = info.Agi;
		charinfo.querySelector('.vit').textContent = info.Vit;
		charinfo.querySelector('.int').textContent = info.Int;
		charinfo.querySelector('.dex').textContent = info.Dex;
		charinfo.querySelector('.luk').textContent = info.Luk;
	}

	function changeBackgroundEverySecond() {
		const root = Component.getRoot();
		const backgroundchange = root.querySelector(`#slot${_curindex}`);
		if (backgroundchange && shouldRunBackgroundChange === true) {
			Client.loadFile(`${DB.INTERFACE_PATH}select_character_ver3/img_slot_select${img}.bmp`, dataURI => {
				backgroundchange.style.backgroundImage = `url(${dataURI})`;
				backgroundchange.style.width = '157px';
				backgroundchange.style.height = '197px';
				backgroundchange.style.backgroundSize = 'contain';
				backgroundchange.style.backgroundRepeat = 'no-repeat';
			});

			// Increment the slot index and wrap around if needed
			img = (img + 1) % 8;
		}
	}

	Component.updateCharSlot = function updateCharSlot(slotId) {
		const root = this.getRoot();
		let start = 0;
		let loopMax = Math.max(_maxSlots, _slots.length);

		if (typeof slotId !== 'undefined') {
			start = slotId;
			loopMax = slotId + 1;
		}

		const charCanvases = root.querySelectorAll('.char_canvas');
		const jobIcons = root.querySelectorAll('.job_icon');

		for (let i = start; i < loopMax; ++i) {
			if (charCanvases[i]) {
				charCanvases[i].querySelector('.name').innerHTML = _slots[i] ? _slots[i].name : '';
			}
			if (!_slots[i]) {
				if (jobIcons[i]) {
					jobIcons[i].style.backgroundImage = '';
				}
				const slotCanvas = root.querySelector(`#slot${i}`);
				if (slotCanvas) {
					Client.loadFile(`${DB.INTERFACE_PATH}select_character_ver3/img_slot2_normal.bmp`, dataURI => {
						slotCanvas.style.backgroundImage = `url(${dataURI})`;
					});
				}
				const countdown = root.querySelector(`.timedelete.slot${i}`);
				if (countdown) {
					countdown.dataset.duration = 0;
					countdown.classList.add('hidden');
					countdown.style.display = 'none';
				}
			} else {
				const slotCanvas = root.querySelector(`#slot${i}`);
				if (slotCanvas) {
					Client.loadFile(`${DB.INTERFACE_PATH}select_character_ver3/img_slot_normal.bmp`, dataURI => {
						slotCanvas.style.backgroundImage = `url(${dataURI})`;
					});
				}

				if (jobIcons[i]) {
					const slotJobIcon = jobIcons[i];
					Client.loadFile(`${DB.INTERFACE_PATH}renewalparty/icon_jobs_${_slots[i].job}.bmp`, dataURI => {
						slotJobIcon.style.backgroundImage = `url(${dataURI})`;
					});
				}

				if (_slots[i].DeleteDate) {
					const countdown = root.querySelector(`.timedelete.slot${i}`);
					if (countdown) {
						countdown.dataset.duration = _slots[i].DeleteDate;
						countdown.classList.remove('hidden');
						countdown.style.display = 'block';
					}
				}
			}
		}
	};

	Component.clearAllSlots = function clearAllSlots() {
		_slots.length = 0;
		_entitySlots.length = 0;
		_list.length = 0;
		Component.updateCharSlot();
	};

	/**
	 * Render sprites to canvas (V4)
	 */
	function renderGrid() {
		Camera.direction = 4;
		const idx = Math.floor(_index / _maxSlots) * _maxSlots;
		const count = _ctx.length;

		for (let i = 0; i < count; ++i) {
			_ctx[i].clearRect(0, 0, _ctx[i].canvas.width, _ctx[i].canvas.height);

			if (_entitySlots[idx + i]) {
				SpriteRenderer.bind2DContext(_ctx[i], 78, 157);
				if (_slots[idx + i] && _slots[idx + i].DeleteDate) {
					_entitySlots[idx + i].action = 2;
				}
				_entitySlots[idx + i].renderEntity();
			}
		}
	}

	/**
	 * Create component and export it
	 */
	return UIManager.addComponent(Component);
}
