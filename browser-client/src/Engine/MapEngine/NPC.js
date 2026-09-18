/**
 * Engine/MapEngine/NPC.js
 *
 * Manage Entity based on received packets from server
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import DB from 'DB/DBManager.js';
import Sound from 'Audio/SoundManager.js';
import BGM from 'Audio/BGM.js';
import Client from 'Core/Client.js';
import Session from 'Engine/SessionStorage.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import Renderer from 'Renderer/Renderer.js';
import NpcBox from 'UI/Components/NpcBox/NpcBox.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import NpcMenu from 'UI/Components/NpcMenu/NpcMenu.js';
import WinPopup from 'UI/Components/WinPopup/WinPopup.js';
import MiniMap from 'UI/Components/MiniMap/MiniMap.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import GUIComponent from 'UI/GUIComponent.js';

/**
 * NPC write a message
 *
 * @param {object} pkt - PACKET.ZC.SAY_DIALOG
 */
function onMessage(pkt) {
	NpcBox.append();
	NpcBox.setText(pkt.msg, pkt.NAID);
}

/**
 * Display a "next" button
 *
 * @param {object} pkt - PACKET_ZC_WAIT_DIALOG
 */
function onNextAppear(pkt) {
	NpcBox.append();
	NpcBox.addNext(pkt.NAID);
}

/**
 * Display a "close button"
 *
 * @param {object} pkt - PACKET.ZC.CLOSE_DIALOG
 */
function onCloseAppear(pkt) {
	// Should not happened, but sometimes dev write a "close" in script instead of
	// a "end" when there is no message.
	if (NpcBox.ui && NpcBox.ui.is(':visible')) {
		NpcBox.addClose(pkt.NAID);
	}
}

/**
 * Server want to close a npc
 *
 * @param {object} pkt - PACKET.ZC.CLOSE_SCRIPT
 */
function onCloseScript(pkt) {
	if (NpcBox.ownerID === pkt.NAID) {
		NpcBox.remove();
		NpcMenu.remove();

		const cutin = document.getElementById('cutin');
		if (cutin && cutin.parentNode) {
			document.body.removeChild(cutin);
		}
	}
}

/**
 * Server sent request about Dynamic NPC Create Result
 *
 * @param {object} pkt - PACKET.ZC.DYNAMICNPC_CREATE_RESULT
 */
function onDynamicNPCCreateRequest(pkt) {
	switch (pkt.result) {
		case 0:
			break;
		case 1:
			ChatBox.addText('[Dynamic NPC] Unknown error', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 2:
			ChatBox.addText('[Dynamic NPC] Unknown NPC', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 3:
			ChatBox.addText('[Dynamic NPC] Duplicate NPC', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
		case 4:
			ChatBox.addText('[Dynamic NPC] Out of time', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
	}
}

/**
 * Display a menu
 *
 * @param {object} pkt - PACKET.ZC.MENU_LIST
 */
function onMenuAppear(pkt) {
	NpcMenu.append();
	NpcMenu.setMenu(pkt.msg, pkt.NAID);

	NpcMenu.onSelectMenu = function onSelectMenu(NAID, index) {
		NpcMenu.remove();

		// Remove npc box when pressed "cancel"
		if (index === 255) {
			NpcBox.remove();
		}

		const _pkt = new PACKET.CZ.CHOOSE_MENU();
		_pkt.NAID = NAID;
		_pkt.num = index;
		Network.sendPacket(_pkt);
	};
}

/**
 * Display input
 *
 * @param {object} pkt - PACKET.ZC.OPEN_EDITDLG or PACKET.ZC.OPEN_EDITDLGSTR
 */
function onInputAppear(pkt) {
	const type = pkt instanceof PACKET.ZC.OPEN_EDITDLGSTR ? 'text' : 'number';
	const id = pkt.NAID;
	InputBox.onSubmitRequest = function OnSubmitRequest(data) {
		InputBox.remove();
		let _pkt;

		switch (type) {
			case 'text':
				_pkt = new PACKET.CZ.INPUT_EDITDLGSTR();
				_pkt.msg = data;
				break;

			default:
			case 'number':
				_pkt = new PACKET.CZ.INPUT_EDITDLG();
				_pkt.value = data;
				break;
		}

		_pkt.NAID = id;
		Network.sendPacket(_pkt);
	};

	InputBox.append();
	InputBox.setType(type, true);
}

/**
 * On Shop selection (buy/sell)
 * @param {object} pkt - PACKET.ZC.SELECT_DEALTYPE
 */
function onDealSelection(pkt) {
	const WinDeal = WinPopup.clone('WinDeal');
	const NAID = pkt.NAID;

	WinDeal.init = function Init() {
		this.draggable();
		this._shadow.querySelector('.text').textContent = DB.getMessage(92);

		Object.assign(this._host.style, {
			top: Renderer.height / 1.5 + 'px',
			left: (Renderer.width - 280) / 2.0 + 'px',
			zIndex: '100'
		});

		const btns = this._shadow.querySelector('.btns');

		const createBtn = (name, onClick) => {
			const btn = document.createElement('button');
			btn.className = 'btn';
			btn.dataset.background = 'btn_' + name + '.bmp';
			btn.dataset.hover = 'btn_' + name + '_a.bmp';
			btn.dataset.down = 'btn_' + name + '_b.bmp';
			btn.addEventListener('click', onClick, { once: true });
			GUIComponent.processDataAttrs(btn);
			return btn;
		};

		btns.appendChild(
			createBtn('buy', function () {
				WinDeal.remove();
				const _pkt = new PACKET.CZ.ACK_SELECT_DEALTYPE();
				_pkt.type = 0;
				_pkt.NAID = NAID;
				Network.sendPacket(_pkt);
			})
		);

		btns.appendChild(
			createBtn('sell', function () {
				WinDeal.remove();
				const _pkt = new PACKET.CZ.ACK_SELECT_DEALTYPE();
				_pkt.type = 1;
				_pkt.NAID = NAID;
				Network.sendPacket(_pkt);
			})
		);

		btns.appendChild(
			createBtn('cancel', function () {
				WinDeal.remove();
			})
		);
	};

	WinDeal.append();
}

/**
 * Receive NPC image to display
 * @param {object} pkt - PACKET.ZC.SHOW_IMAGE
 */
function onCutin(pkt) {
	// Only one instance of cutin
	const cutin = document.getElementById('cutin');
	if (cutin) {
		document.body.removeChild(cutin);
	}

	// Sending empty string just hide the cutin
	if (!pkt.imageName.length) {
		return;
	}

	if (pkt.imageName.indexOf('.') === -1) {
		pkt.imageName += '.bmp';
	}

	Client.loadFile(DB.INTERFACE_PATH + 'illust/' + pkt.imageName, function (url) {
		const img = new Image();
		img.decoding = 'async';
		img.src = url;
		img.style.position = 'absolute';
		img.style.zIndex = 40;
		img.id = 'cutin';
		img.draggable = false;

		switch (pkt.type) {
			default:
				return;

			case 0:
				img.style.bottom = '0px';
				img.style.left = '0px';
				break;

			case 1:
				img.style.bottom = '0px';
				img.style.left = '50%';
				img.style.marginLeft = '-' + Math.floor(img.width / 2) + 'px';
				break;

			case 2:
				img.style.bottom = '0px';
				img.style.right = '0px';
				break;

			case 3:
			// TODO: extend cutin system
			// break;

			case 4:
				img.style.top = '50%';
				img.style.left = '50%';
				img.style.marginLeft = '-' + Math.floor(img.width / 2) + 'px';
				img.style.marginTop = '-' + Math.floor(img.height / 2) + 'px';
				break;
		}

		document.body.appendChild(img);
	});
}

/**
 * NPC put a mark on minimap
 * @param {object} pkt - PACKET.ZC.COMPASS
 */
function onMinimapMarker(pkt) {
	// TODO: do we need to use NPC ID ? (pkt.NAID)

	switch (pkt.type) {
		// Add a mark for 15 seconds
		case 0:
			MiniMap.getUI().addNpcMark(pkt.id, pkt.xPos, pkt.yPos, pkt.color, 15000);
			break;

		// Add a mark
		case 1:
			MiniMap.getUI().addNpcMark(pkt.id, pkt.xPos, pkt.yPos, pkt.color, Infinity);
			break;

		// Remove a mark
		case 2:
			MiniMap.getUI().removeNpcMark(pkt.id);
			break;
	}
}

/**
 * Receive progressbar
 *
 * @param {object} pkt - PACKET_ZC_PROGRESS
 */
function onProgressBar(pkt) {
	Session.Entity.cast.onComplete = function () {
		const _pkt = new PACKET.CZ.PROGRESS();
		Network.sendPacket(_pkt);
	};

	const rgb =
		'rgb(' +
		[(pkt.color & 0x00ff0000) >> 16, (pkt.color & 0x0000ff00) >> 8, pkt.color & 0x000000ff].join(',') +
		')';

	// Color added only if the progressbar isn't black
	Session.Entity.cast.set(pkt.time * 1000, pkt.color ? rgb : null);
}

/**
 * Stop Progressbar
 *
 * @param {object} pkt - PACKET.CZ.PROGRESS
 */
function onProgressBarStop() {
	Session.Entity.cast.clean();
}

/**
 * Received sound from NPC
 *
 * @param {object} pkt - PACKET.ZC.SOUND
 */
function onSound(pkt) {
	switch (pkt.act) {
		// Play once
		case 0:
		case 1:
			Sound.play(pkt.fileName);
			break;

		// From rathena, should stop a sound but doesn't seems to work in official client ?
		case 2:
			Sound.stop(pkt.fileName);
			break;
	}

	// TODO: 3D sound based on npc(pkt.NAID) position ?
	// what is pkt.term ?
}

/**
 * Received bgm from npc
 *
 * @param {object} pkt - PACKET.ZC.PLAY_NPC_BGM
 */
function onBGM(pkt) {
	if (!pkt.Bgm.match(/\.mp3$/i)) {
		pkt.Bgm += '.mp3';
	}

	BGM.play(pkt.Bgm);
}

/**
 * Next button pressed
 *
 * @param {number} NAID - npc id
 */
NpcBox.onNextPressed = function onNextPressed(NAID) {
	const pkt = new PACKET.CZ.REQ_NEXT_SCRIPT();
	pkt.NAID = NAID;
	Network.sendPacket(pkt);
};

/**
 * Close button pressed
 *
 * @param {number} NAID - npc id
 */
NpcBox.onClosePressed = function onClosePressed(NAID) {
	const pkt = new PACKET.CZ.CLOSE_DIALOG();
	pkt.NAID = NAID;
	Network.sendPacket(pkt);

	NpcBox.remove();
};

/**
 * Initialize
 */
export default function NPCEngine() {
	Network.hookPacket(PACKET.ZC.SAY_DIALOG, onMessage);
	Network.hookPacket(PACKET.ZC.WAIT_DIALOG, onNextAppear);
	Network.hookPacket(PACKET.ZC.CLOSE_DIALOG, onCloseAppear);
	Network.hookPacket(PACKET.ZC.OPEN_EDITDLG, onInputAppear);
	Network.hookPacket(PACKET.ZC.OPEN_EDITDLGSTR, onInputAppear);
	Network.hookPacket(PACKET.ZC.MENU_LIST, onMenuAppear);
	Network.hookPacket(PACKET.ZC.SELECT_DEALTYPE, onDealSelection);
	Network.hookPacket(PACKET.ZC.SHOW_IMAGE, onCutin);
	Network.hookPacket(PACKET.ZC.SHOW_IMAGE2, onCutin);
	Network.hookPacket(PACKET.ZC.COMPASS, onMinimapMarker);
	Network.hookPacket(PACKET.ZC.PROGRESS, onProgressBar);
	Network.hookPacket(PACKET.ZC.PROGRESS_CANCEL, onProgressBarStop);
	Network.hookPacket(PACKET.ZC.SOUND, onSound);
	Network.hookPacket(PACKET.ZC.PLAY_NPC_BGM, onBGM);
	Network.hookPacket(PACKET.ZC.CLOSE_SCRIPT, onCloseScript);
	Network.hookPacket(PACKET.ZC.DYNAMICNPC_CREATE_RESULT, onDynamicNPCCreateRequest);
}
