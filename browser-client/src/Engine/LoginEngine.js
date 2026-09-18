/**
 * Engine/LoginEngine.js
 *
 * Login Engine
 * Manage login server, connexion
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

// Load dependencies
import TextEncoding from 'Utils/CodepageManager.js';
import DB from 'DB/DBManager.js';
import BGM from 'Audio/BGM.js';
import Sound from 'Audio/SoundManager.js';
import Configs from 'Core/Configs.js';
import Thread from 'Core/Thread.js';
import Session from 'Engine/SessionStorage.js';
import CharEngine from 'Engine/CharEngine.js';
import Network from 'Network/NetworkManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import PluginManager from 'Plugins/PluginManager.js';
import Renderer from 'Renderer/Renderer.js';
import UIManager from 'UI/UIManager.js';
import WinList from 'UI/Components/WinList/WinList.js';
import WinPopup from 'UI/Components/WinPopup/WinPopup.js';
import Queue from 'Utils/Queue.js';
import Background from 'UI/Background.js';
import MD5 from 'Vendors/spark-md5.min.js';
import Rijndael from 'Utils/Rijndael.js';

// Version Dependent UIs
import WinLogin from 'UI/Components/WinLogin/WinLogin.js';

/**
 * Creating WinLoading
 */
const WinLoading = WinPopup.clone('WinLoading');
WinLoading.init = function () {
	Object.assign(this._host.style, {
		top: (Renderer.height - 120) / 1.5 + 'px',
		left: (Renderer.width - 280) / 2.0 + 'px'
	});
	this._shadow.querySelector('.text').textContent = DB.getMessage(121);
};
UIManager.addComponent(WinLoading);

/**
 * @var {object} server object stored in clientinfo.xml
 */
let _server = null;

/**
 * @var {array} char-servers list
 */
let _charServers = [];

/**
 * @var {string} Stored username to send as ping
 */
let _loginID = '';

class LoginEngine {
	/**
	 * Init Game
	 */
	static init(server) {
		const q = new Queue();
		const old_server = _server;

		Configs.setServer(server);
		UIManager.removeComponents();
		Session.LangType = 'langtype' in server ? parseInt(server.langtype, 10) : 1; // default to SERVICETYPE_AMERICA

		// Renewal switch
		Session.isRenewal = Configs.get('renewal', false);
		console.log('%c[LOGIN] Game Mode: ', 'color:#007000', Session.isRenewal ? 'RENEWAL' : 'PRE-RENEWAL');

		// Setup Default Charset based on LangType
		const charset = TextEncoding.detectEncodingByLangtype(Session.LangType, Configs.get('disableKorean'));

		console.log('%c[LOGIN] Language Type: ', 'color:#007000', Session.LangType);
		console.log('%c[LOGIN] Encoding: ', 'color:#007000', charset);

		_server = server;

		// Add support for "packetver" definition in Server listing
		const packetver = String(Configs.get('packetver'));
		const remoteClient = Configs.get('remoteClient');
		const autoLogin = Configs.get('autoLogin');
		const audioExt = Configs.get('BGMFileExtension');

		// Server packetver
		if (packetver) {
			if (packetver.match(/^\d+$/)) {
				PACKETVER.value = parseInt(packetver, 10);
			}
		}

		if (!PACKETVER.value) {
			UIManager.showErrorBox('Sorry, no PACKETVER configs found.');
			return;
		}

		// Add support for remote client in server definition
		if (remoteClient) {
			Thread.send('SET_HOST', remoteClient);

			// Check if the selected server changed.
			if (old_server != null && (old_server.address != _server.address || old_server.port != _server.port)) {
				// Re-Loading game data with server specific files (txt, lua, lub)
				q.add(() => {
					DB.onReady = () => {
						Background.setLoginBackground(); // remove loading
						q._next();
					};
					DB.onProgress = (i, count) => {
						Background.setPercent(Math.floor((i / count) * 100));
					};
					UIManager.removeComponents();
					Background.init();
					Background.resize(Renderer.width, Renderer.height);
					Background.setLoginBackground(() => {
						DB.isLoaded = false;
						DB.init();
					});
				});
			}
		}

		// Server audio configuration
		if (audioExt) {
			BGM.setAvailableExtensions(audioExt);
		}

		// GMs account list from server
		Session.AdminList = server.adminList || [];

		// Init per server plugins
		PluginManager.init();

		// Hooking win_login
		WinLogin.selectUIVersion();
		Background.setLoginBackground();

		WinLogin.getUI().onConnectionRequest = onConnectionRequest;
		WinLogin.getUI().onExitRequest = onExitRequest;

		// Handle unexpected disconnects during login phase
		Network.onDisconnect = () => {
			UIManager.showMessageBox(
				DB.getMessage(1),
				'ok',
				() => {
					UIManager.removeComponents();
					WinLogin.getUI().append();
				},
				true
			);
		};

		// Autologin features
		if (autoLogin instanceof Array && autoLogin[0] && autoLogin[1]) {
			onConnectionRequest.apply(null, autoLogin);
			Configs.set('autoLogin', null);
		} else {
			q.add(function () {
				WinLogin.getUI().append();
			});
		}

		// Hook packets
		if (PACKETVER.value < 20170315) {
			Network.hookPacket(PACKET.AC.ACCEPT_LOGIN, onConnectionAccepted);
		} else {
			Network.hookPacket(PACKET.AC.ACCEPT_LOGIN3, onConnectionAccepted);
		}

		// Refuse login packets
		Network.hookPacket(PACKET.AC.REFUSE_LOGIN, onConnectionRefused);
		Network.hookPacket(PACKET.AC.REFUSE_LOGIN_R2, onConnectionRefused);
		Network.hookPacket(PACKET.AC.REFUSE_LOGIN3, onConnectionRefused);
		Network.hookPacket(PACKET.AC.REFUSE_LOGIN_EX, onConnectionRefused);

		// international refuse packets
		Network.hookPacket(PACKET.AC.REFUSE_LOGIN_USA, onInternationalConnectionRefused);

		// Taren refuse packets
		Network.hookPacket(PACKET.AC.LOGIN_TAREN_REFUSE, onTarenConnectionRefused);
		Network.hookPacket(PACKET.AC.LOGIN_TAREN_REFUSE2, onTarenConnectionRefused2);

		// forced disconnect
		Network.hookPacket(PACKET.SC.NOTIFY_BAN, onServerClosed);

		// Execute
		q.run();
	}

	/**
	 * Reload WinLogin
	 */
	static reload() {
		UIManager.removeComponents();
		WinLogin.getUI().onConnectionRequest = onConnectionRequest;
		WinLogin.getUI().onExitRequest = onExitRequest;
		WinLogin.getUI().append();

		Network.close();
	}

	/**
	 * setLoadedServer()
	 *
	 * Called by GameEngine when it reloads files due to a service change
	 * so we don't wind up trying to load the db twice. (Or concurrently.)
	 */
	static setLoadedServer(server) {
		_server = server;
	}
}

/**
 * Trying to connect to Login server
 *
 * @param {string} username
 * @param {string} password
 */
function onConnectionRequest(username, password) {
	// Play "¹öÆ°¼Ò¸®.wav" (possible problem with charset)
	Sound.play('\xB9\xF6\xC6\xB0\xBC\xD2\xB8\xAE.wav');

	// Add the loading screen
	// Store the ID to use for the ping
	WinLogin.getUI().remove();
	WinLoading.append();
	_loginID = username;

	// Try to connect
	Network.connect(_server.address, _server.port, success => {
		// Fail to connect...
		if (!success) {
			UIManager.showMessageBox(
				DB.getMessage(1),
				'ok',
				() => {
					UIManager.removeComponents();
					WinLogin.getUI().append();
				},
				true
			);
			return;
		}

		let pkt;
		let hash = false;

		// Get client hash
		if (Configs.get('calculateHash') && !Configs.get('development')) {
			// Calucalte hash from files (slower, more "secure")
			const files = Configs.get('hashFiles');
			let fileStatus = 0;
			const fileContents = [];

			for (let i = 0; i < files.length; i++) {
				const jsonFile = new XMLHttpRequest();
				jsonFile.open('GET', files[i], true);
				jsonFile.send();

				jsonFile.onreadystatechange = function () {
					if (jsonFile.readyState == 4 && jsonFile.status == 200) {
						fileContents[i] = jsonFile.responseText;
						fileStatus++;
					}

					if (fileStatus == files.length) {
						const contentString = fileContents.join('\r\n'); // Join strings with carrige return & newline
						hash = MD5.hash(contentString); // Just hash the whole array
						sendLogin();
					}
				};
			}
		} else {
			// Just use the predefined value (faster, less "secure")
			hash = Configs.get('clientHash');
			sendLogin();
		}

		function sendLogin() {
			if (hash) {
				// Convert hexadecimal hash to binary
				if (/^[a-f0-9]+$/i.test(hash)) {
					let str = '';
					let i;
					const count = hash.length;

					for (i = 0; i < count; i += 2) {
						str += String.fromCharCode(parseInt(hash.substr(i, 2), 16));
					}

					hash = str;
				}

				pkt = new PACKET.CA.EXE_HASHCHECK();
				pkt.HashValue = hash;
				Network.sendPacket(pkt);
			}

			if (Configs.get('loginMode') == 'han') {
				// password is encrypted with rijndael
				const paddedPassword = new Uint8Array(24);
				for (let i = 0; i < password.length; i++) {
					paddedPassword[i] = password.charCodeAt(i);
				}
				const encryptedPassword = Rijndael.encrypt(
					paddedPassword,
					Configs.get('rijndaelKey'),
					Configs.get('rijndaelChain'),
					24,
					'ecb'
				);

				if (!encryptedPassword) {
					UIManager.showErrorBox('Rijndael encryption failed. Check rijndaelKey and rijndaelChain config.');
					return;
				}

				pkt = new PACKET.CA.LOGIN_HAN();
				pkt.ID = username;
				pkt.Passwd = String.fromCharCode(...encryptedPassword);
				pkt.Version = parseInt(_server.version, 10);
				pkt.clienttype = parseInt(_server.langtype, 10);
				pkt.m_szIP = '192.168.0.1'; // dummy
				pkt.m_szMacAddr = '00:1A:2B:3C:4D:5E'; // dummy
				pkt.isHanGameUser = 0;
				Network.sendPacket(pkt);
			} else {
				// Try to connect
				pkt = new PACKET.CA.LOGIN();
				pkt.ID = username;
				pkt.Passwd = password;
				pkt.Version = parseInt(_server.version, 10);
				pkt.clienttype = parseInt(_server.langtype, 10);
				Network.sendPacket(pkt);
			}
		}
	});
}

/**
 * Go back to intro window
 */
function onExitRequest() {
	import('Engine/GameEngine.js').then(GameEngine => {
		GameEngine.default.reload();
	});
}

/**
 * User selected a char-server
 *
 * @param {number} index in server list
 */
function onCharServerSelected(index) {
	// Play "¹öÆ°¼Ò¸®.wav" (encode to avoid problem with charset)
	Sound.play('\xB9\xF6\xC6\xB0\xBC\xD2\xB8\xAE.wav');

	WinList.remove();
	WinLoading.append();

	Session.ServerName = _charServers[index].name; // Save server name
	Network.onDisconnect = null; // Let CharEngine handle its own disconnects
	CharEngine.init(_charServers[index]);
}

/**
 * Accepted connection from char-server
 *
 * @param {object} pkt - PACKET.AC.ACCEPT_LOGIN
 */
function onConnectionAccepted(pkt) {
	UIManager.removeComponents();

	Session.AuthCode = pkt.AuthCode;
	Session.AID = pkt.AID;
	Session.UserLevel = pkt.userLevel;
	Session.Sex = pkt.Sex;

	if (PACKETVER.value >= 20170315) {
		Session.WebToken = pkt.webAuthToken;
	}

	_charServers = pkt.ServerList;

	// Build list of servers
	let i;
	const count = _charServers.length;
	const list = new Array(count);
	for (i = 0; i < count; ++i) {
		list[i] = _charServers[i].property ? DB.getMessage(482) + ' ' : '';
		list[i] += _charServers[i].name;
		list[i] += _charServers[i].state
			? DB.getMessage(484)
			: ' ' + DB.getMessage(483).replace('%d', _charServers[i].usercount);
	}

	// No choice, connect directly to the server
	if (count === 1 && Configs.get('skipServerList')) {
		WinLoading.append();
		Session.ServerName = _charServers[0].name; // Save server name
		Network.onDisconnect = null; // Let CharEngine handle its own disconnects
		CharEngine.init(_charServers[0]);
	}

	// Have to select server in the list
	else {
		// Show window
		WinList.onIndexSelected = onCharServerSelected;
		WinList.onExitRequest = () => {
			Network.close();
			WinList.remove();
			WinLogin.getUI().append();
		};
		WinList.append();
		WinList.setList(list);
	}

	// Set ping
	const ping = new PACKET.CA.CONNECT_INFO_CHANGED();
	ping.ID = _loginID;
	Network.setPing(() => {
		Network.sendPacket(ping);
	});

	if (PACKETVER.value >= 20170315 && Session.WebToken) {
		import('UI/Components/ShortCut/ShortCut.js').then(ShortCut => {
			ShortCut.default.loadFromServer();
		});
	}
}

/**
 * Received data from server, connection refused (Taren)
 *
 * @param {object} pkt - PACKET.AC.LOGIN_TAREN_REFUSE
 */
function onTarenConnectionRefused(pkt) {
	let msg_id;

	switch (pkt.ErrorCode) {
		case 1:
			msg_id = 3196;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_ACCOUNT_NOTEXIST = This account does not exist
		case 2:
			msg_id = 3197;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_PASSWORD = Passwords does not match
		case 3:
			msg_id = 3198;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_IP = Failed to pass IP authentication
		case 4:
			msg_id = 3199;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_PERSONALNUMBER = No identification number, supplement your registration information
		case 5:
			msg_id = 3200;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_ACCOUNT_BLOCK = Account block
		case 6:
			msg_id = 3201;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_SYSTEM = System error
		case 7:
			msg_id = 3214;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_NOTACTIVATED = Account is not active
		case 9:
			msg_id = 3911;
			break; // MSI_BAN_TEENAGER = Youth access is restricted
		default:
			msg_id = 3202;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_UNKNOWN = unknown error found
	}

	UIManager.showMessageBox(
		DB.getMessage(msg_id),
		'ok',
		() => {
			UIManager.removeComponents();
			WinLogin.getUI().append();
		},
		true
	);

	Network.close();
}

/**
 * Received data from server, connection refused (Taren)
 *
 * @param {object} pkt - PACKET.AC.LOGIN_TAREN_REFUSE2
 */
function onTarenConnectionRefused2(pkt) {
	let msg_id;

	switch (pkt.ErrorCode) {
		case 1:
			msg_id = 3196;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_ACCOUNT_NOTEXIST = This account does not exist.
		case 2:
			msg_id = 3197;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_PASSWORD = Passwords does not match.
		case 3:
			msg_id = 3198;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_IP = Failed to pass IP authentication
		case 4:
			msg_id = 3199;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_PERSONALNUMBER = No identification number, supplement your registration information
		case 5:
			msg_id = 3200;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_ACCOUNT_BLOCK = Account block
		case 6:
			msg_id = 3201;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_SYSTEM = System error
		case 7:
			msg_id = 3214;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_NOTACTIVATED = Account is not active
		case 8:
		case 100:
		case 102:
			msg_id = 3202;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_UNKNOWN = unknown error found.
		case 9:
			msg_id = 3911;
			break; // MSI_BAN_TEENAGER = Youth access is restricted.
		case 101:
			msg_id = 449;
			break; // MSI_LOGIN_REFUSE_BLOCKED_UNTIL = You are prohibited to log in until %s.
		default:
			msg_id = 3202;
			break; // MSI_TAREN_LOGINREFUSE_FAIL_UNKNOWN = unknown error found.
	}

	UIManager.showMessageBox(
		DB.getMessage(msg_id).replace('%s', pkt.blockDate),
		'ok',
		() => {
			UIManager.removeComponents();
			WinLogin.getUI().append();
		},
		true
	);
}

/**
 * Received data from server, connection refused (International)
 *
 * @param {object} pkt - PACKET.AC.REFUSE_LOGIN_USA
 */
function onInternationalConnectionRefused(pkt) {
	let msg_id;
	switch (pkt.ErrorCode) {
		default:
		case 5200:
			msg_id = 3314;
			break; // MSI_USA_LOGINERRORMSG_UNKNOWN = UNKNOWN ERROR:%d
		case 5201:
			msg_id = 3315;
			break; // MSI_USA_LOGINERRORMSG_NOT_USER = NOT USER
		case 5202:
			msg_id = 3316;
			break; // MSI_USA_LOGINERRORMSG_ID_BLOCK = THIS ACCOUNT ID IS BLOCKED
		case 5203:
			msg_id = 3317;
			break; // MSI_USA_LOGINERRORMSG_ID_INACTIVAT = COUNTRY REJECT (OR NOT AVALIABLE USER )
		case 5204:
			msg_id = 3318;
			break; // MSI_USA_LOGINERRORMSG_PASSWORD = NOT MATCH PASSWORD
		case 5205:
			msg_id = 3319;
			break; // MSI_USA_LOGINERRORMSG_EMAIL_CERT = NOT EMAIL CERT
		case 5206:
			msg_id = 3320;
			break; // MSI_USA_LOGINERRORMSG_PAYPAL_BLOCK1 = PAYPAL BLOCK
		case 5207:
			msg_id = 3321;
			break; // MSI_USA_LOGINERRORMSG_COUNTRY_REJECT = COUNTRY REJECT
		case 5208:
			msg_id = 3322;
			break; // MSI_USA_LOGINERRORMSG_PAYPAL_BLOCK2 = PAYPAL BLOCK
		case 5209:
			msg_id = 3323;
			break; // MSI_USA_LOGINERRORMSG_WEB_BLOCK = WEB BLOCK
		case 5210:
			msg_id = 3324;
			break; // MSI_USA_LOGINERRORMSG_AGE = AGE LIMIT USER
		case 5211:
			msg_id = 3325;
			break; // MSI_USA_LOGINERRORMSG_PASSWORD_CHANGE = PASSWORD HAS NOT BEEN CHANGED FOR MORE THAN 90DAYS
		case 5212:
			msg_id = 3326;
			break; // MSI_USA_LOGINERRORMSG_INPUTDATA = INPUT DATA ERROR
		case 5213:
			msg_id = 3327;
			break; // MSI_USA_LOGINERRORMSG_DATABASE = ERROR DATABASE
		case 5214:
			msg_id = 3328;
			break; // MSI_USA_LOGINERRORMSG_SYSTEM = ERROR SYSTEM
	}

	UIManager.showMessageBox(
		DB.getMessage(msg_id).replace('%d', pkt.blockDate),
		'ok',
		() => {
			UIManager.removeComponents();
			WinLogin.getUI().append();
		},
		true
	);

	Network.close();
}

/**
 * Received data from server, connection refused
 *
 * @param {object} pkt - PACKET.AC.REFUSE_LOGIN
 */
function onConnectionRefused(pkt) {
	let error = 9;
	switch (pkt.ErrorCode) {
		case 0:
			error = 6;
			break; // Unregistered ID
		case 1:
			error = 267;
			break; // Incorrect Password
		case 2:
			error = 8;
			break; // This ID is expired
		case 3:
			error = 3;
			break; // Rejected from Server
		case 4:
			error = 266;
			break; // Checked: 'Login is currently unavailable. Please try again shortly.'- 2br
		case 5:
			error = 310;
			break; // Your Game's EXE file is not the latest version
		case 6:
			error = 449;
			break; // Your are Prohibited to log in until %s
		case 7:
			error = 439;
			break; // Server is jammed due to over populated
		case 8:
			error = 681;
			break; // Checked: 'This account can't connect the Sakray server.'
		case 9:
			error = 703;
			break; // 9 = MSI_REFUSE_BAN_BY_DBA
		case 10:
			error = 704;
			break; // 10 = MSI_REFUSE_EMAIL_NOT_CONFIRMED
		case 11:
			error = 705;
			break; // 11 = MSI_REFUSE_BAN_BY_GM
		case 12:
			error = 706;
			break; // 12 = MSI_REFUSE_TEMP_BAN_FOR_DBWORK
		case 13:
			error = 707;
			break; // 13 = MSI_REFUSE_SELF_LOCK
		case 14:
			error = 708;
			break; // 14 = MSI_REFUSE_NOT_PERMITTED_GROUP
		case 15:
			error = 709;
			break; // 15 = MSI_REFUSE_NOT_PERMITTED_GROUP
		case 16:
			error = 9;
			break;
		case 17:
			error = 1380;
			break;
		case 18:
			error = 1381;
			break;
		case 19:
			error = 1751;
			break;
		case 20:
			error = 1752;
			break;
		case 21:
			error = 1786;
			break;
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 30:
			error = 3440;
			break;
		case 28:
			error = 2731;
			break;
		case 29:
			error = 2732;
			break;
		case 33:
			error = 2682;
			break;
		case 36:
			error = 3453;
			break;
		case 99:
			error = 368;
			break; // 99 = This ID has been totally erased
		case 100:
			error = 809;
			break; // 100 = Login information remains at %s
		case 101:
			error = 810;
			break; // 101 = Account has been locked for a hacking investigation. Please contact the GM Team for more information
		case 102:
			error = 811;
			break; // 102 = This account has been temporarily prohibited from login due to a bug-related investigation
		case 103:
			error = 859;
			break; // 103 = This character is being deleted. Login is temporarily unavailable for the time being
		case 104:
			error = 860;
			break; // 104 = This character is being deleted. Login is temporarily unavailable for the time being
		case 105:
			error = 1372;
			break;
		case 106:
			error = 1293;
			break;
		case 108:
			error = 1393;
			break;
		case 109:
			error = 1394;
			break;
		case 110:
			error = 2345;
			break;
		case 112:
			error = 2410;
			break;
		case 242:
			error = 3439;
			break;
		case 243:
			error = 3687;
			break;
		case 244:
			error = 4190;
			break;
		case 5011:
			error = 1839;
			break;
		case 5012:
			error = 823;
			break;
		case 5013:
		case 5054:
			error = 1789;
			break;
		case 5014:
		case 5016:
		case 5019:
		case 5021:
		case 5052:
		case 5053:
		case 5055:
		case 5056:
		case 5057:
		case 5058:
		case 5059:
		case 5060:
		case 5061:
			error = 1838;
			break;
		case 5015:
			error = 1840;
			break;
		case 5017:
			error = 1841;
			break;
		case 5018:
			error = 1786;
			break;
		case 5050:
			error = 1830;
			break;
		case 5051:
			error = 5;
			break;
		case 5062:
			error = 266;
			break;
		case 5063:
			error = 1793;
			break;
		case 5064:
			error = 1794;
			break;
		case 5300:
			error = 3534;
			break;
		case 5301:
			error = 3539;
			break;
	}

	UIManager.showMessageBox(
		DB.getMessage(error).replace('%s', pkt.blockDate),
		'ok',
		() => {
			UIManager.removeComponents();
			WinLogin.getUI().append();
		},
		true
	);

	Network.close();
}

/**
 * Received closed connection from server
 *
 * @param {object} pkt - PACKET.SC.NOTIFY_BAN
 */
function onServerClosed(pkt) {
	let msg_id;

	switch (pkt.ErrorCode) {
		default:
		case 0:
			msg_id = 3;
			break; // MSI_BANNED = Disconnected from Server!
		case 1:
			msg_id = 4;
			break; // MSI_SERVER_OFF = Server closed.
		case 2:
			msg_id = 5;
			break; // MSI_DOUBLE_LOGIN_PROHIBITED = Someone has Logged in with this ID.
		case 3:
			msg_id = 241;
			break; // MSI_SPEEDHACK = You've been disconnected due to a time gap between you and the server
		case 4:
			msg_id = 264;
			break; // MSI_PC_OVERFLOW = Server is jammed due to over population. Please try again shortly.
		case 5:
			msg_id = 305;
			break; // MSI_UNDER_AGE = You are underaged and cannot join this server.
		case 6:
			msg_id = 764;
			break; // MSI_CANT_CONNECT_TO_PAY_SERVER = Trial players can't connect Pay to Play Server. (761)
		case 8:
			msg_id = 440;
			break; // MSI_INFORMATION_REMAINED = The game server still recognizes your last log-in. Please try again after about 30 seconds.
		case 9:
			msg_id = 529;
			break; // MSI_BAN_IP_OVERFLOW = IP capacity of this Internet Cafe is full. Would you like to pay the personal base?
		case 10:
			msg_id = 530;
			break; // MSI_BAN_PAY_OUT = You are out of available paid playing time. Game will be shut down automatically. (528)
		case 11:
			msg_id = 575;
			break; // MSI_BAN_PAY_SUSPEND = Your account is suspended.
		case 12:
			msg_id = 576;
			break; // MSI_BAN_PAY_CHANGE = Your connection is terminated due to change in the billing policy. Please connect again.
		case 13:
			msg_id = 577;
			break; // MSI_BAN_PAY_WRONGIP = Your connection is terminated because your IP doesn't match the authorized IP from the account server.
		case 14:
			msg_id = 578;
			break; // MSI_BAN_PAY_PNGAMEROOM = Your connection is terminated to prevent charging from your account's play time.
		case 15:
			msg_id = 579;
			break; // MSI_BAN_OP_FORCE = You have been forced to disconnect by the Game Master Team.
		case 16:
			msg_id = 606;
			break; // MSI_BAN_JAPAN_REFUSE1 = N/A?
		case 17:
			msg_id = 607;
			break; // MSI_BAN_JAPAN_REFUSE2 = N/A?
		case 18:
			msg_id = 678;
			break; // MSI_BAN_INFORMATION_REMAINED_ANOTHER_ACCOUNT = Your account is already connected to account server.
		case 100:
			msg_id = 1123;
			break; // MSI_BAN_PC_IP_UNFAIR = Please check the connection, more than 2 accounts are connected with Internet Cafe Time Plan.
		case 101:
			msg_id = 1178;
			break; // MSI_BAN_PC_IP_COUNT_ALL = More than 30 players sharing the same IP have logged into the game for an hour. Please check this matter.
		case 102:
			msg_id = 1179;
			break; // MSI_BAN_PC_IP_COUNT = More than 10 connections sharing the same IP have logged into the game for an hour. Please check this matter.
		case 103:
			msg_id = 1309;
			break; // MSI_BAN_GRAVITY_MEM_AGREE = You need to accept the Privacy Policy from Gravity in order to use the service.
		case 104:
			msg_id = 1310;
			break; // MSI_BAN_GAME_MEM_AGREE = You need to accept the User Agreement in order to use the service.
		case 105:
			msg_id = 1311;
			break; // MSI_BAN_HAN_VALID = Incorrect or nonexistent ID.
		case 106:
			msg_id = 1373;
			break; // MSI_BAN_PC_IP_LIMIT_ACCESS = The number of accounts connected to this IP has exceeded the limit.
		case 107:
			msg_id = 1429;
			break; // MSI_BAN_OVER_CHARACTER_LIST = You have %d characters in your account. Server does not allow you to make more.
		case 108:
			msg_id = 1582;
			break; // MSI_BAN_IP_BLOCK = It is impossible to connect using this IP in Ragnarok Online. Please contact the customer support center or home.
		case 109:
			msg_id = 1583;
			break; // MSI_BAN_INVALID_PWD_CNT = You have entered a wrong password for more than six times, please check your personal information again.
		case 110:
			msg_id = 1589;
			break; // MSI_BAN_NOT_ALLOWED_JOBCLASS = Sorry the character you are trying to use is banned for testing connection.
	}

	UIManager.showMessageBox(
		DB.getMessage(msg_id),
		'ok',
		() => {
			UIManager.removeComponents();
			WinLogin.getUI().append();
		},
		true
	);
	Network.close();
}

/**
 * Export
 */
export default LoginEngine;
