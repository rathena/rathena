/**
 * Engine/CharEngine.js
 *
 * Char Engine
 * Manage char server, connection, character selection / creation / deletion, etc.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import DB from 'DB/DBManager.js';
import Configs from 'Core/Configs.js';
import Events from 'Core/Events.js';
import Sound from 'Audio/SoundManager.js';
import BGM from 'Audio/BGM.js';
import Session from 'Engine/SessionStorage.js';
import MapEngine from 'Engine/MapEngine.js';
import Network from 'Network/NetworkManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import UIManager from 'UI/UIManager.js';
import Background from 'UI/Background.js';
import PincodeWindow from 'UI/Components/PincodeWindow/PincodeWindow.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import JoystickUI from 'UI/Components/JoystickUI/JoystickUI.js';
import CharSelect from 'UI/Components/CharSelect/CharSelect.js';
import CharCreate from 'UI/Components/CharCreate/CharCreate.js';
import Player from 'Renderer/Entity/Player.js';

// Load modules
// Version Dependent UIs
/**
 * @type {object} server data
 */
let _server = null;

/**
 * @type {number} where to create character ?
 */
let _creationSlot = 0;

/**
 * @type {number} times attempted to provide pin code.
 */
let _pincodeAttempts = 0;

/**
 * @type {boolean} are we currently attempting to authenticate a pin code reset?
 */
let _inAuthPincodeReset = false;

/**
 * @type {boolean} are we resetting a pincode?
 */
let _resettingPincode = false;

/**
 * @type {boolean} are we creating a pincode?
 */
let _creatingPincode = false;

class CharEngine {
	/*
	 * Connect to char server
	 */
	static init(server) {
		BGM.play('01.mp3');

		//Notify MapEngine if it needs UI update
		MapEngine.needsUIVerUpdate = _server !== server;

		// Storing variable
		_server = server;

		// Connect to char server
		const forceAddress = Configs.get('forceUseAddress');
		const server_info = Configs.getServer();
		const ip = forceAddress ? server_info.address : Network.utils.longToIP(server.ip);
		Network.connect(ip, server.port, success => {
			// Fail to connect...
			if (!success) {
				UIManager.showErrorBox(DB.getMessage(1));
				return;
			}

			// Success, try to connect
			const pkt = new PACKET.CH.ENTER();
			pkt.AID = Session.AID;
			pkt.AuthCode = Session.AuthCode;
			pkt.userLevel = Session.UserLevel;
			pkt.Sex = Session.Sex;
			pkt.clientType = Session.LangType;
			Network.sendPacket(pkt);

			// Server send back (new) AID
			Network.read(fp => {
				Session.AID = fp.readLong();
			});
		});

		//Select UI version
		CharSelect.selectUIVersion();
		CharCreate.selectUIVersion();

		// Hook packets
		Network.hookPacket(PACKET.HC.ACCEPT_ENTER_NEO_UNION, onConnectionAccepted);
		Network.hookPacket(PACKET.HC.REFUSE_ENTER, onConnectionRefused);
		Network.hookPacket(PACKET.HC.REFUSE_SELECTCHAR, onSelectionRefused);
		Network.hookPacket(PACKET.HC.ACCEPT_MAKECHAR_NEO_UNION, onCreationSuccess);
		Network.hookPacket(PACKET.HC.ACCEPT_MAKECHAR, onCreationSuccess);
		Network.hookPacket(PACKET.HC.REFUSE_MAKECHAR, onCreationFail);
		Network.hookPacket(PACKET.HC.ACCEPT_DELETECHAR, onDeleteAnswer);
		Network.hookPacket(PACKET.HC.DELETE_CHAR3, onDeleteAnswer);
		Network.hookPacket(PACKET.HC.REFUSE_DELETECHAR, onDeleteAnswer);
		Network.hookPacket(PACKET.HC.NOTIFY_ZONESVR, onReceiveMapInfo);
		Network.hookPacket(PACKET.HC.NOTIFY_ZONESVR2, onReceiveMapInfo);
		Network.hookPacket(PACKET.HC.ACCEPT_ENTER_NEO_UNION_HEADER, onConnectionAccepted);
		Network.hookPacket(PACKET.HC.ACCEPT_ENTER_NEO_UNION_LIST, onCharacterListChunk);
		Network.hookPacket(PACKET.HC.ACCEPT_ENTER_NEO_UNION_LIST2, onCharacterListChunk);
		Network.hookPacket(PACKET.HC.NOTIFY_ACCESSIBLE_MAPNAME, onMapUnavailable);
		Network.hookPacket(PACKET.HC.SECOND_PASSWD_LOGIN, onPincodeCheckSuccess);
		Network.hookPacket(PACKET.HC.DELETE_CHAR3_RESERVED, onRequestCharDel);
		Network.hookPacket(PACKET.HC.CHARLIST_NOTIFY, onCharListNotify);
		JoystickUI.onRestore();
	}

	/**
	 * Reload Char-Select
	 */
	static reload() {
		Network.close();
		Background.setLoginBackground(() => {
			UIManager.removeComponents();
			CharEngine.init(_server);
		});
	}
}
/**
 * Request to go back to Login Window
 */
function onExitRequest() {
	import('Engine/LoginEngine.js').then(m => m.default.reload());
}

/**
 * Server send character list chunk
 *
 * @param {object} pkt - PACKET.HC.ACCEPT_ENTER_NEO_UNION_LIST or PACKET.HC.ACCEPT_ENTER_NEO_UNION_LIST2
 */
function onCharacterListChunk(pkt) {
	const ChSel = CharSelect.getUI();
	if (!ChSel) return;
	pkt.charInfo.forEach(charInfo => {
		ChSel.addCharacter(charInfo);
	});
}

/**
 * Connection accepted from char-server
 * Displaying Character list
 *
 * @param {object} pkt - PACKET.HC.ACCEPT_ENTER_NEO_UNION
 */
function onConnectionAccepted(pkt) {
	pkt.sex = Session.Sex;

	// Start sending ping
	const ping = new PACKET.CZ.PING();
	ping.AID = Session.AID;
	Network.setPing(() => {
		Network.sendPacket(ping);
	});

	Session.Playing = false;
	Session.hasCart = false;

	// Reset the active player for the new char-select session
	Session.Entity = null;

	// Reset Announcement component
	const Announce = UIManager.getComponent('Announce');
	if (Announce) {
		Announce.remove();
	}

	// Reset MapName component
	const MapName = UIManager.getComponent('MapName');
	if (MapName) {
		MapName.remove();
		MapName.resetState();
	}

	UIManager.getComponent('WinLoading').remove();

	// Initialize window
	const ChSel = CharSelect.getUI();
	ChSel.onExitRequest = onExitRequest;
	ChSel.onConnectRequest = onConnectRequest;
	ChSel.onCreateRequest = onCreateRequest;
	ChSel.onDeleteRequest = onDeleteRequest;
	ChSel.onDeleteReqDelay = onDeleteReqDelay;
	ChSel.onCancelDeleteRequest = onCancelDeleteRequest;
	ChSel.append();
	ChSel.setInfo(pkt);

	/**
	 * In PACKETVERs < 20180124 that support pincode auth, we're supposed to
	 * show a button that will ask the server to perform it.
	 * In this case, the server will send the pincode request packet.
	 *
	 * Later PACKETVERs have the char server handle this on initial connection,
	 * so the button was removed, and we don't have to do anything here.
	 */
	//if (PACKETVER.value < 20180124 && PACKETVER.value >= 20110309) {
	//sendPincodeRequest(); // This causes duplicate packets, please fix

	/**
	 * TODO: rAthena says this button was removed with PACKETVER >= 20180124. See also: rathena/src/char/char.hpp
	 * Need to find out where this button is supposed to be and place it on the correct screen.
	 */
	//}
}

/**
 * Server don't want the user to select a character
 *
 * @param {object} pkt - PACKET.HC.REFUSE_SELECTCHAR
 */
function onSelectionRefused(pkt) {
	let msg_id;

	switch (pkt.ErrorCode) {
		case 0:
			msg_id = 1350;
			break;
		case 1:
			msg_id = 1351;
			break;
		case 2:
			msg_id = 1352;
			break;
		case 3:
			msg_id = 1353;
			break;
		case 4:
			msg_id = 1354;
			break;
		case 5:
			msg_id = 1364;
			break;
		default:
			msg_id = 2;
			break;
	}

	UIManager.showErrorBox(DB.getMessage(msg_id));
}

/**
 * Server don't want the user to connect
 *
 * @param {object} pkt - PACKET.HC.REFUSE_ENTER
 */
function onConnectionRefused(pkt) {
	let msg_id;

	switch (pkt.ErrorCode) {
		case 1:
			msg_id = 208;
			break;
		case 111:
			msg_id = 1865;
			break;
		case 118:
			msg_id = 3818;
			break;
		case 119:
			msg_id = 2929;
			break;
		default:
			msg_id = 2;
			break;
	}

	UIManager.showErrorBox(DB.getMessage(msg_id));
}

/**
 * No map server available
 *
 * @param {object} pkt - PACKET.HC.NOTIFY_ACCESSIBLE_MAPNAME
 */
function onMapUnavailable(pkt) {
	// no map server avaiable
	UIManager.showMessageBox(
		DB.getMessage(1811),
		'ok',
		() => {
			UIManager.getComponent('WinLoading').remove();
			CharSelect.getUI().append();
		},
		true
	);
}

/**
 * Char Delete Request Result
 */
function onRequestCharDel(pkt) {
	if (!pkt) {
		return;
	}

	// Just pass the packet info
	CharSelect.getUI().reqdeleteAnswer(pkt);
}

/**
 * Char Delete Request Cancel
 */
function onCancelDeleteRequest(charID) {
	if (charID === 0) {
		return;
	}

	const pkt = new PACKET.CH.DELETE_CHAR3_CANCEL();
	pkt.GID = charID;
	Network.sendPacket(pkt);
}

/**
 * User want to delete a character with delay
 *
 * @param {number} charID - Character ID
 */
function onDeleteReqDelay(charID) {
	if (!charID) {
		return;
	}

	const pkt = new PACKET.CH.DELETE_CHAR3_RESERVED();
	pkt.GID = charID;
	Network.sendPacket(pkt);
}

/**
 * User want to delete a character
 *
 * @param {number} charID - Character ID
 */
function onDeleteRequest(charID) {
	let _ui_box;
	let _inputValue;
	let _time_end;
	let _render = false;
	let _canvas, _ctx, _width, _height;
	let _TimeOut;

	// Delete the character
	function deleteCharacter() {
		if (PACKETVER.value > 20100803) {
			const pkt = new PACKET.CH.DELETE_CHAR3();
			pkt.GID = charID;
			pkt.Birth = _inputValue.substring(2); // Server only needs the 6 digits
			Network.sendPacket(pkt);
		} else {
			const pkt = new PACKET.CH.DELETE_CHAR();
			pkt.GID = charID;
			pkt.key = _inputValue;
			Network.sendPacket(pkt);
		}
	}

	// Cancel the prompt
	function onCancel() {
		InputBox.remove();
		_ui_box.remove();
		_overlay.remove();
		Events.clearTimeout(_TimeOut);
		onDeleteAnswer({ ErrorCode: -2 });
	}

	// Ask the mail/birthdate
	function onOk() {
		InputBox.append();
		if (PACKETVER.value >= 20100803) {
			InputBox.setType('birthdate', true);
		} else {
			InputBox.setType('mail', true);
		}
		InputBox.onSubmitRequest = onSubmit;
		_ui_box._host.style.zIndex = '50'; // ui same zIndex bg
		_overlay.style.zIndex = '51'; // overlay same zIndex input
		_ui_box.append(); // don't remove message box
	}

	// Display prompt message
	_ui_box = UIManager.showPromptBox(DB.getMessage(19), 'ok', 'cancel', onOk, onCancel);
	const _overlay = document.createElement('div');
	_overlay.className = 'win_popup_overlay';
	document.body.appendChild(_overlay);

	// Submit the mail/birthdate
	function onSubmit(input) {
		_inputValue = input;
		InputBox.remove();
		_ui_box.remove();

		if (PACKETVER.value < 20180124) {
			// Not sure which date should we not use this loading delete anymore
			// Stop rendering...
			_ui_box = UIManager.showMessageBox(DB.getMessage(296).replace('%d', 10), 'cancel', () => {
				_render = false;
				onCancel();
			});

			// Build canvas
			_canvas = document.createElement('canvas');
			_ctx = _canvas.getContext('2d');
			_width = _canvas.width = 240;
			_height = _canvas.height = 15;
			_canvas.style.marginTop = '10px';
			_canvas.style.marginLeft = '20px';
			_ui_box._shadow.querySelector('.container').appendChild(_canvas);

			// Parameter
			_time_end = Date.now() + 10000;
			_render = true;

			// Start the timing
			render();
		} else {
			// No waiting time
			_ui_box.remove();
			_overlay.remove();
			deleteCharacter();
			return;
		}
	}

	// Rendering
	function render() {
		// Calculate percent
		const time_left = _time_end - Date.now();
		const percent = Math.round(100 - time_left / 100);

		// Delete character
		if (percent >= 100) {
			_ui_box.remove();
			_overlay.remove();
			deleteCharacter();
			return;
		}

		// Update text
		_ui_box._shadow.querySelector('.text').textContent = DB.getMessage(296).replace(
			'%d',
			Math.round(10 - percent / 10)
		);

		// Update progressbar
		_ctx.clearRect(0, 0, _width, _height);
		_ctx.fillStyle = 'rgb(0,255,255)';
		_ctx.fillRect(0, 0, _width, _height);
		_ctx.fillStyle = 'rgb(140,140,140)';
		_ctx.fillRect(1, 1, _width - 2, _height - 2);
		_ctx.fillStyle = 'rgb(66,99,165)';
		_ctx.fillRect(2, 2, Math.round((percent * (_width - 4)) / 100), _height - 4);
		_ctx.fillStyle = 'rgb(255,255,0)';
		_ctx.fillText(percent + '%', (_width - _ctx.measureText(percent + '%').width) * 0.5, 12);

		_TimeOut = Events.setTimeout(render, 30);
	}
}

/*
 * Answer from server to delete character
 *
 * @param {object} PACKET.HC.REFUSE_DELETECHAR or PACKET.HC.ACCEPT_DELETECHAR
 * @param {object} PACKET.HC.DELETE_CHAR3 <GID> <Result>
 */
function onDeleteAnswer(pkt) {
	let result;
	if (PACKETVER.value <= 20100803) {
		// Email deletion result
		result = typeof pkt.ErrorCode === 'undefined' ? -1 : pkt.ErrorCode;
	} else {
		// Birthday deletion result
		result = typeof pkt.Result === 'undefined' ? -1 : pkt.Result;
	}
	CharSelect.getUI().deleteAnswer(result);
}

/**
 * Asking from CharSelect to create a character, moving to CharCreate window
 *
 * @param {number} index - slot where to create character
 */
function onCreateRequest(index) {
	const ChSel = CharSelect.getUI();
	const ChCre = CharCreate.getUI();
	_creationSlot = index;
	ChSel.remove();
	ChCre.setAccountSex(Session.Sex);
	ChCre.onCharCreationRequest = onCharCreationRequest;
	ChCre.onExitRequest = function () {
		ChCre.remove();
		ChSel.append();
	};
	ChCre.append();
}

/**
 * User want to create a character, send data to server
 *
 * @param {string} name
 * @param {number} Str - strength stat
 * @param {number} Agi - agility stat
 * @param {number} Vit - vitality stat
 * @param {number} Int - intelligence stat
 * @param {number} Dex - dexterity stat
 * @param {number} Luk - luck stat
 * @param {number} hair - hair style
 * @param {number} color - hair color
 * @param {number} job - job
 * @param {number} sex - sex
 */
function onCharCreationRequest(name, Str, Agi, Vit, Int, Dex, Luk, hair, color, job, sex) {
	let pkt;

	// Old Packet required stats
	if (PACKETVER.value < 20120307) {
		pkt = new PACKET.CH.MAKE_CHAR();
		pkt.Str = Str;
		pkt.Agi = Agi;
		pkt.Vit = Vit;
		pkt.Int = Int;
		pkt.Dex = Dex;
		pkt.Luk = Luk;
	} else if (PACKETVER.value >= 20120307 && PACKETVER.value < 20151001) {
		pkt = new PACKET.CH.MAKE_CHAR2();
	} else {
		pkt = new PACKET.CH.MAKE_CHAR3();
	}

	pkt.name = name;
	pkt.head = hair;
	pkt.headPal = color;
	pkt.CharNum = _creationSlot;
	pkt.Job = job;
	pkt.Sex = sex;
	Network.sendPacket(pkt);
}

/**
 * Success to create a character
 *
 * @param {object} pkt - PACKET.HC.ACCEPT_MAKECHAR_NEO_UNION
 * @param {object} pkt - PACKET.HC.ACCEPT_MAKECHAR
 */
function onCreationSuccess(pkt) {
	CharCreate.getUI().remove();
	const ChSel = CharSelect.getUI();
	ChSel.addCharacter(pkt.charinfo);
	ChSel.append();
}

/**
 * Fail to create a character
 *
 * @param {object} pkt - PACKET.HC.REFUSE_MAKECHAR
 */
function onCreationFail(pkt) {
	let msg_id;

	switch (pkt.ErrorCode) {
		case 0x00:
			msg_id = 10;
			break; // 'Charname already exists'
		case 0x01:
			msg_id = 298;
			break; // 'You are underaged'
		case 0x02:
			msg_id = 1272;
			break; // 'Symbols in Character Names are forbidden'
		case 0x03:
			msg_id = 1355;
			break; // 'You are not elegible to open the Character Slot.'
		default:
		case 0xff:
			msg_id = 11;
			break; // 'Char creation denied'
	}

	UIManager.showMessageBox(DB.getMessage(msg_id), 'ok');
}

/*function sendPincodeRequest() {
		let pkt;

		pkt = new PACKET.CH.PINCODE_REQUEST();
		pkt.AID = Session.AID;

		Network.sendPacket(pkt);
	}*/ // UNUSED

function onPincodeCheckRequest(pincode) {
	const pkt = new PACKET.CH.PINCODE_CHECK();
	pkt.AID = Session.AID;
	pkt.PINCODE = pincode;

	Network.sendPacket(pkt);
}

function onPincodeCreate(pincode, bad) {
	_creatingPincode = true;

	const pkt = new PACKET.CH.PINCODE_FIRST_PIN();
	pkt.AID = Session.AID;
	pkt.PINCODE = pincode;

	Network.sendPacket(pkt);
}

function onPincodeReset(oldpin, newpin) {
	_inAuthPincodeReset = false;
	_resettingPincode = true;

	const pkt = new PACKET.CH.PINCODE_CHANGE();
	pkt.AID = Session.AID;
	pkt.OLD_PINCODE = oldpin;
	pkt.NEW_PINCODE = newpin;

	Network.sendPacket(pkt);
}

/*function onAuthPincodeReset(oldpin, newpin) {
		_inAuthPincodeReset = true;
		onPincodeCheckRequest(oldpin);
	}*/ // UNUSED

function onUserPincodeResetReq() {
	_pincodeAttempts = 0;
	PincodeWindow.onPincodeReset = onPincodeReset;
}

function onPincodeCheckSuccess(pkt) {
	if (!PincodeWindow.__active && pkt.State == 0) {
		console.log('Pincode is disabled.');
		return;
	}

	PincodeWindow.remove();

	if (PACKETVER.value < 20110309) {
		console.log('Pincode packet sent from server, but PACKETVER is too old. Ignoring.');
		return;
	}
	PincodeWindow.onPincodeCheckRequest = onPincodeCheckRequest;
	PincodeWindow.onUserPincodeResetReq = onUserPincodeResetReq;
	PincodeWindow.onExitRequest = () => {
		_pincodeAttempts = 0;
		_inAuthPincodeReset = false;
		_resettingPincode = false;
		_creatingPincode = false;
		PincodeWindow.resetUI();
		PincodeWindow.remove();
		onExitRequest();
	};

	/*
	 * Pincode
	 *
	 * S 08b8 <AID>.L <data>.4B - check PIN
	 * S 08c5 <AID>.L <data>.4B - request for PIN button ?
	 * S 08be <AID>.L <old>.4B <new>.4B - change PIN
	 * S 08ba <AID>.L <new>.4B - set PIN
	 * R 08b9 <seed>.L <AID>.L <state>.W
	 *	State:
	 *	0 = pin is correct OR pincode feature is disabled
	 *	1 = ask for pin - client sends 0x8b8
	 *	2 = create new pin - client sends 0x8ba
	 *	3 = pin must be changed - client 0x8be
	 *	4 = create new pin ?? - client sends 0x8ba
	 *	5 = client shows msgstr(1896) Unable to use restricted number in 2nd password
	 *	6 = client shows msgstr(1897) Unable to use your KSSN number
	 *	7 = char select window shows a button - client sends 0x8c5
	 *	8 = pincode was incorrect
	 */
	switch (pkt.State) {
		case 7: // Pin is correct on PACKETVERs < 20180124.
			if (PACKETVER.value >= 20180124) {
				console.log(
					'PINCODE: Received invalid state from server for configured PACKETVER: ' +
						pkt.State +
						'. Aborting, please fix your PACKETVER in the config.'
				);
				PincodeWindow.onExitRequest();
			}
		case 0: // pin is correct
			_pincodeAttempts = 0;
			if (_inAuthPincodeReset === true) {
				PincodeWindow.onPincodeReset = onPincodeReset;
				PincodeWindow.onOldPincodeCheckResult(true);
			} else {
				if (_creatingPincode === true) {
					_creatingPincode = false;
					UIManager.showMessageBox(DB.getMessage(1889), 'ok');
				}
				if (_resettingPincode === true) {
					_resettingPincode = false;
					UIManager.showMessageBox(DB.getMessage(1891), 'ok');
				}
				PincodeWindow.resetUI();
				CharSelect.getUI().setUIEnabled(true);
			}
			break;
		case 1: // ask for pin
			CharSelect.getUI().setUIEnabled(false);
			PincodeWindow.selectInput(0);
			if (_pincodeAttempts < 3) {
				PincodeWindow.clearPin();
				PincodeWindow.setUserSeed(pkt.Seed);
				PincodeWindow.append();
			} else {
				PincodeWindow.onExitRequest(); // Failed authentication.
			}
			break;
		case 2: // create new pin
		case 4: // create new pin ??
			CharSelect.getUI().setUIEnabled(false);
			UIManager.showMessageBox(DB.getMessage(1900), 'ok');
			PincodeWindow.selectInput(0);
			PincodeWindow.setUserSeed(pkt.Seed);
			PincodeWindow.onPincodeCheckRequest = onPincodeCreate;
			PincodeWindow.append();
			break;
		case 3: // pin must be changed
			CharSelect.getUI().setUIEnabled(false);
			if (_pincodeAttempts < 3) {
				UIManager.showMessageBox(DB.getMessage(2345), 'ok');
				PincodeWindow.setUserSeed(pkt.Seed);
				PincodeWindow.onPincodeReset = onPincodeReset;
				PincodeWindow.onParentPincodeResetReq();
				PincodeWindow.append();
			} else {
				PincodeWindow.onExitRequest(); // Failed authentication.
			}
			break;
		case 5: // client shows msgstr(1896) Unable to use restricted number in 2nd password
		case 6: // client shows msgstr(1897) Unable to use your KSSN number
		case 8: // pincode was incorrect
			if (_creatingPincode === true) {
				UIManager.showMessageBox(DB.getMessage(1893), 'ok');
			} else {
				UIManager.showMessageBox(DB.getMessage(pkt.State == 5 ? 1896 : pkt.State == 6 ? 1897 : 1892), 'ok');
			}
			_pincodeAttempts++;
			if (_pincodeAttempts < 3) {
				PincodeWindow.resetPins();
				PincodeWindow.setUserSeed(pkt.Seed);
				if (_inAuthPincodeReset === true) {
					_inAuthPincodeReset = false;
					PincodeWindow.onOldPincodeCheckResult(false);
				}
				PincodeWindow.append();
			} else {
				PincodeWindow.onExitRequest(); // Failed authentication.
			}
			break;
		default:
			console.log('PINCODE: Received unknown state from server: ' + pkt.State);
			PincodeWindow.append();
			break;
	}
}

/**
 * User ask to connect with its player
 *
 * @param {object} entity to connect with
 */
function onConnectRequest(entity) {
	// Play sound
	Sound.play('\xB9\xF6\xC6\xB0\xBC\xD2\xB8\xAE.wav');

	CharSelect.getUI().remove();
	UIManager.getComponent('WinLoading').append();
	// The char-select list holds plain packet structures, build the player from it.
	// Done here (instead of on char-list reception) so that characters delivered by
	// any char-list packet, and freshly created ones, all end up with a Player.
	Session.Entity = new Player(entity);
	const pkt = new PACKET.CH.SELECT_CHAR();
	pkt.CharNum = entity.CharNum;
	Network.sendPacket(pkt);
}

/**
 * Server send char list info, ask for char list
 *
 * @param {object} charListInfo - PACKET.HC.CHARLIST_NOTIFY
 */
function onCharListNotify(charListInfo) {
	const total = Math.max(charListInfo.TotalCnt, 1);
	for (let i = 0; i < total; i++) {
		const pkt = new PACKET.CH.CHARLIST_REQ();
		Network.sendPacket(pkt);
	}
}

/**
 * Player selection successful, get mapserver information to connect
 *
 * @param {object} pkt - PACKET.HC.NOTIFY_ZONESVR
 */
let retryCount = 0;
function onReceiveMapInfo(pkt) {
	if (!DB.isLoaded) {
		if (!DB.startedLazyInit) {
			DB.lazyInit();
			DB.startedLazyInit = true;
		}
		retryCount++;
		if (retryCount > 600) {
			UIManager.showMessageBox('Failed loading databases, please restart the game', 'ok', () => {
				CharEngine.reload();
			});
			retryCount = 0;
			DB.startedLazyInit = false;
			return;
		}
		setTimeout(() => onReceiveMapInfo(pkt), 100);
		return;
	}
	DB.startedLazyInit = false;
	retryCount = 0;
	Session.GID = pkt.GID;
	MapEngine.init(pkt.addr.ip, pkt.addr.port, pkt.mapName);
}

// TODO: Add support for captcha, rename, changeslot and pincode.
/*
 * Captcha
 *
 * S 07e5 PACKET.CH.ENTER_CHECKBOT - Request for the captcha ?
 * S 07e7 PACKET.CH.CHECKBOT - Send code
 * R 07e8 PACKET.HC.CHECKBOT - image url ?
 * R 07e9 PACKET.HC.CHECKBOT_RESULT - Result for captcha
 */

/*
 * Rename (http://ragnarok.levelupgames.ph/main/new-loki-server-merge-faq/)
 *
 * S 08fc <char ID>.l <new name>.24B (new one) - Ask if valid
 * S 028d PACKET.CH.REQ_IS_VALID_CHARNAME - Ask if valid
 * R 028e PACKET.HC.ACK_IS_VALID_CHARNAME - Result
 * S 028f PACKET.CH.REQ_CHANGE_CHARNAME (confirm)
 */

/*
 * Change slot (http://ragnarok.levelupgames.ph/main/new-loki-server-merge-faq/)
 *
 * S 08d4 <from>.W <to>.W <unk>.W
 * R 08d5 <len>.W <success>.W <unk>.W
 */

/**
 * Export
 */
export default CharEngine;
