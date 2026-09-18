/**
 * Network/PacketStructures.js
 *
 * List of the structure of all known packets.
 * Based on : http://rathena.org/wiki/index.php?title=Packets&action=edit
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import BinaryWriter from 'Utils/BinaryWriter.js';
import PACKETVER from './PacketVerManager.js';
import Struct from 'Utils/Struct.js';
import Configs from 'Core/Configs.js';

const NAME_LENGTH = 24; // Must be equal to same name var in mmo.h, -1 reads the rest of the packet
const MAP_NAME_LENGTH = 11 + 1;
const MAP_NAME_LENGTH_EXT = MAP_NAME_LENGTH + 4;
const PACKET = {};
const RENEWAL = Configs.get('renewal') || false;
const CLASSIC = !RENEWAL; // For ease of reading checks
const UNUSED_PACKET = PACKET;

/**
 * Reads the trailing name field of a variable length packet, honoring NAME_LENGTH.
 *
 * @param {object} fp - BinaryReader
 * @param {number} end - packet end offset
 * @returns {string}
 */
function readTrailingName(fp, end) {
	return fp.readString(NAME_LENGTH === -1 ? end - fp.tell() : NAME_LENGTH);
}

PACKET.CA = {};
PACKET.AC = {}; // Login
PACKET.CH = {};
PACKET.HC = {}; // Char
PACKET.CZ = {};
PACKET.ZC = {}; // Map
PACKET.CS = {};
PACKET.SC = {}; // All servers
PACKET.ZH = {}; // ??? typo error ?
PACKET.AHC = {};
PACKET.CAH = {}; // Security

// 0x186 PACKET_COLLECTORDEAD ??

// * auto-generated *

// 0x64
PACKET.CA.LOGIN = function PACKET_CA_LOGIN() {
	this.Version = 0;
	this.ID = '';
	this.Passwd = '';
	this.clienttype = 0;
};
PACKET.CA.LOGIN.prototype.build = function () {
	const pkt_len = 2 + 4 + 24 + 24 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x64);
	pkt_buf.writeULong(this.Version);
	pkt_buf.writeString(this.ID, 24);
	pkt_buf.writeString(this.Passwd, 24);
	pkt_buf.writeUChar(this.clienttype);
	return pkt_buf;
};

// 0x65
PACKET.CH.ENTER = function PACKET_CH_ENTER() {
	this.AID = 0;
	this.AuthCode = 0;
	this.userLevel = 0;
	this.clientType = 0;
	this.Sex = 0;
};
PACKET.CH.ENTER.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4 + 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x65);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.AuthCode);
	pkt_buf.writeULong(this.userLevel);
	pkt_buf.writeUShort(this.clientType);
	pkt_buf.writeUChar(this.Sex);
	return pkt_buf;
};

// 0x66
PACKET.CH.SELECT_CHAR = function PACKET_CH_SELECT_CHAR() {
	this.CharNum = 0;
};
PACKET.CH.SELECT_CHAR.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x66);
	pkt_buf.writeUChar(this.CharNum);
	return pkt_buf;
};

// 0x67
PACKET.CH.MAKE_CHAR = function PACKET_CH_MAKE_CHAR() {
	this.name = '';
	this.Str = 0;
	this.Agi = 0;
	this.Vit = 0;
	this.Int = 0;
	this.Dex = 0;
	this.Luk = 0;
	this.CharNum = 0;
	this.headPal = 0;
	this.head = 0;
};
PACKET.CH.MAKE_CHAR.prototype.build = function () {
	const pkt_len = 2 + 24 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x67);
	pkt_buf.writeString(this.name, 24);
	pkt_buf.writeUChar(this.Str);
	pkt_buf.writeUChar(this.Agi);
	pkt_buf.writeUChar(this.Vit);
	pkt_buf.writeUChar(this.Int);
	pkt_buf.writeUChar(this.Dex);
	pkt_buf.writeUChar(this.Luk);
	pkt_buf.writeUChar(this.CharNum);
	pkt_buf.writeShort(this.headPal);
	pkt_buf.writeShort(this.head);
	return pkt_buf;
};

// 0x68
PACKET.CH.DELETE_CHAR = function PACKET_CH_DELETE_CHAR() {
	this.GID = 0;
	this.key = '';
};
PACKET.CH.DELETE_CHAR.prototype.build = function () {
	const pkt_len = 2 + 4 + 40;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x68);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeString(this.key, 40);
	return pkt_buf;
};

// 0x72
PACKET.CZ.ENTER = function PACKET_CZ_ENTER() {
	this.AID = 0;
	this.GID = 0;
	this.AuthCode = 0;
	this.clientTime = 0;
	this.Sex = 0;
};
PACKET.CZ.ENTER.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.AID, true);
	pkt.view.setUint32(ver[4], this.GID, true);
	pkt.view.setUint32(ver[5], this.AuthCode, true);
	pkt.view.setUint32(ver[6], this.ClientTime, true);
	pkt.view.setUint8(ver[7], this.Sex, true);

	return pkt;
};

// 0x7d
PACKET.CZ.NOTIFY_ACTORINIT = function PACKET_CZ_NOTIFY_ACTORINIT() {};
PACKET.CZ.NOTIFY_ACTORINIT.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7d);
	return pkt_buf;
};

// 0x7e
PACKET.CZ.REQUEST_TIME = function PACKET_CZ_REQUEST_TIME() {
	this.clientTime = 0;
};
PACKET.CZ.REQUEST_TIME.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.clientTime, true);
	return pkt;
};

// 0x844
PACKET.CZ.HBT = function PACKET_CZ_HBT() {};
PACKET.CZ.HBT.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0844);
	return pkt_buf;
};

// 0x82
PACKET.CZ.REQUEST_QUIT = function PACKET_CZ_REQUEST_QUIT() {};
PACKET.CZ.REQUEST_QUIT.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x82);
	return pkt_buf;
};

// 0x85
PACKET.CZ.REQUEST_MOVE = function PACKET_CZ_REQUEST_MOVE() {
	this.dest = [0, 0];
};
PACKET.CZ.REQUEST_MOVE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setPos(ver[3], this.dest, true);
	return pkt;
};

// 0x89
PACKET.CZ.REQUEST_ACT = function PACKET_CZ_REQUEST_ACT() {
	this.targetGID = 0;
	this.action = 0;
};
PACKET.CZ.REQUEST_ACT.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.targetGID, true);
	pkt.view.setUint8(ver[4], this.action, true);
	return pkt;
};

// 0x8c
PACKET.CZ.REQUEST_CHAT = function PACKET_CZ_REQUEST_CHAT() {
	this.msg = '';
};
PACKET.CZ.REQUEST_CHAT.prototype.build = function () {
	const version = this.getPacketVersion();
	const pkt_len = 2 + 2 + this.msg.length + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(version[1]);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.msg);

	return pkt_buf;
};

// 0x90
PACKET.CZ.CONTACTNPC = function PACKET_CZ_CONTACTNPC() {
	this.NAID = 0;
	this.type = 0;
};
PACKET.CZ.CONTACTNPC.prototype.build = function () {
	const pkt_len = 2 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x90);
	pkt_buf.writeULong(this.NAID);
	pkt_buf.writeUChar(this.type);
	return pkt_buf;
};

// 0x94
PACKET.CZ.REQNAME = function PACKET_CZ_REQNAME() {
	this.AID = 0;
};
PACKET.CZ.REQNAME.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.AID, true);
	return pkt;
};

// 0x96
PACKET.CZ.WHISPER = function PACKET_CZ_WHISPER() {
	this.receiver = '';
	this.msg = '';
};
PACKET.CZ.WHISPER.prototype.build = function () {
	const pkt_len = 2 + 2 + 24 + this.msg.length + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x96);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.receiver, 24);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x9b
PACKET.CZ.CHANGE_DIRECTION = function PACKET_CZ_CHANGE_DIRECTION() {
	this.headDir = 0;
	this.dir = 0;
};
PACKET.CZ.CHANGE_DIRECTION.prototype.build = function () {
	const servDirection = [4, 3, 2, 1, 0, 7, 6, 5];

	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.headDir, true);
	pkt.view.setUint8(ver[4], servDirection[(this.dir + 8) % 8], true);
	return pkt;
};

// 0x9f
PACKET.CZ.ITEM_PICKUP = function PACKET_CZ_ITEM_PICKUP() {
	this.ITAID = 0;
};
PACKET.CZ.ITEM_PICKUP.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.ITAID, true);
	return pkt;
};

// 0xa2
PACKET.CZ.ITEM_THROW = function PACKET_CZ_ITEM_THROW() {
	this.Index = 0;
	this.count = 0;
};
PACKET.CZ.ITEM_THROW.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint16(ver[3], this.Index, true);
	pkt.view.setInt16(ver[4], this.count, true);
	return pkt;
};

// 0xa7
PACKET.CZ.USE_ITEM = function PACKET_CZ_USE_ITEM() {
	this.index = 0;
	this.AID = 0;
};
PACKET.CZ.USE_ITEM.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint16(ver[3], this.index, true);
	pkt.view.setUint32(ver[4], this.AID, true);
	return pkt;
};

// 0xa9
PACKET.CZ.REQ_WEAR_EQUIP = function PACKET_CZ_REQ_WEAR_EQUIP() {
	this.index = 0;
	this.wearLocation = 0;
};
PACKET.CZ.REQ_WEAR_EQUIP.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint16(ver[3], this.index, true);
	if (PACKETVER.value >= 20120925) {
		pkt.view.setUint32(ver[4], this.wearLocation, true);
	} else {
		pkt.view.setUint16(ver[4], this.wearLocation, true);
	}
	return pkt;
};

// 0xab
PACKET.CZ.REQ_TAKEOFF_EQUIP = function PACKET_CZ_REQ_TAKEOFF_EQUIP() {
	this.index = 0;
};
PACKET.CZ.REQ_TAKEOFF_EQUIP.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xab);
	pkt_buf.writeUShort(this.index);
	return pkt_buf;
};

// 0xb2
PACKET.CZ.RESTART = function PACKET_CZ_RESTART() {
	this.type = 0;
};
PACKET.CZ.RESTART.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb2);
	pkt_buf.writeUChar(this.type);
	return pkt_buf;
};

// 0xb8
PACKET.CZ.CHOOSE_MENU = function PACKET_CZ_CHOOSE_MENU() {
	this.NAID = 0;
	this.num = 0;
};
PACKET.CZ.CHOOSE_MENU.prototype.build = function () {
	const pkt_len = 2 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb8);
	pkt_buf.writeULong(this.NAID);
	pkt_buf.writeUChar(this.num);
	return pkt_buf;
};

// 0xb9
PACKET.CZ.REQ_NEXT_SCRIPT = function PACKET_CZ_REQ_NEXT_SCRIPT() {
	this.NAID = 0;
};
PACKET.CZ.REQ_NEXT_SCRIPT.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb9);
	pkt_buf.writeULong(this.NAID);
	return pkt_buf;
};

// 0xba
PACKET.CZ.REQ_STATUS = function PACKET_CZ_REQ_STATUS() {};
PACKET.CZ.REQ_STATUS.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xba);
	return pkt_buf;
};

// 0xbb
PACKET.CZ.STATUS_CHANGE = function PACKET_CZ_STATUS_CHANGE() {
	this.statusID = 0;
	this.changeAmount = 0;
};
PACKET.CZ.STATUS_CHANGE.prototype.build = function () {
	let pkt_len;
	if (this.statusID >= 219 && this.statusID <= 224) {
		pkt_len = 2 + 3 + 1;
	} else {
		pkt_len = 2 + 2 + 1;
	}
	const pkt_buf = new BinaryWriter(pkt_len);

	if (this.statusID >= 219 && this.statusID <= 224) {
		pkt_buf.writeShort(0xb24);
	} else {
		pkt_buf.writeShort(0xbb);
	}
	pkt_buf.writeUShort(this.statusID);
	pkt_buf.writeUChar(this.changeAmount);
	return pkt_buf;
};

// 0xbf
PACKET.CZ.REQ_EMOTION = function PACKET_CZ_REQ_EMOTION() {
	this.type = 0;
};
PACKET.CZ.REQ_EMOTION.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xbf);
	pkt_buf.writeUChar(this.type);
	return pkt_buf;
};

// 0xc1
PACKET.CZ.REQ_USER_COUNT = function PACKET_CZ_REQ_USER_COUNT() {};
PACKET.CZ.REQ_USER_COUNT.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xc1);
	return pkt_buf;
};

// 0xc5
PACKET.CZ.ACK_SELECT_DEALTYPE = function PACKET_CZ_ACK_SELECT_DEALTYPE() {
	this.NAID = 0;
	this.type = 0;
};
PACKET.CZ.ACK_SELECT_DEALTYPE.prototype.build = function () {
	const pkt_len = 2 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xc5);
	pkt_buf.writeULong(this.NAID);
	pkt_buf.writeUChar(this.type);
	return pkt_buf;
};

// 0xc8
PACKET.CZ.PC_PURCHASE_ITEMLIST = function PACKET_CZ_PC_PURCHASE_ITEMLIST() {
	this.itemList = [];
};
PACKET.CZ.PC_PURCHASE_ITEMLIST.prototype.build = function () {
	let pkt_len;
	if (PACKETVER.value >= 20181121) {
		pkt_len = 2 + 2 + this.itemList.length * 6;
	} else {
		pkt_len = 2 + 2 + this.itemList.length * 4;
	}
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xc8);
	pkt_buf.writeShort(pkt_len);

	const count = this.itemList.length;
	for (let i = 0; i < count; ++i) {
		pkt_buf.writeShort(this.itemList[i].count);
		if (PACKETVER.value >= 20181121) {
			pkt_buf.writeULong(this.itemList[i].ITID);
		} else {
			pkt_buf.writeUShort(this.itemList[i].ITID);
		}
	}

	return pkt_buf;
};

// 0xc9
PACKET.CZ.PC_SELL_ITEMLIST = function PACKET_CZ_PC_SELL_ITEMLIST() {
	this.itemList = [];
};
PACKET.CZ.PC_SELL_ITEMLIST.prototype.build = function () {
	const pkt_len = 2 + 2 + this.itemList.length * 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xc9);
	pkt_buf.writeShort(pkt_len);

	const count = this.itemList.length;
	for (let i = 0; i < count; ++i) {
		pkt_buf.writeShort(this.itemList[i].index);
		pkt_buf.writeShort(this.itemList[i].count);
	}

	return pkt_buf;
};

// 0xcf
PACKET.CZ.SETTING_WHISPER_PC = function PACKET_CZ_SETTING_WHISPER_PC() {
	this.name = '';
	this.type = 0;
};
PACKET.CZ.SETTING_WHISPER_PC.prototype.build = function () {
	const pkt_len = 2 + 24 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xcf);
	pkt_buf.writeString(this.name, 24);
	pkt_buf.writeUChar(this.type);
	return pkt_buf;
};

// 0xd0
PACKET.CZ.SETTING_WHISPER_STATE = function PACKET_CZ_SETTING_WHISPER_STATE() {
	this.type = 0;
};
PACKET.CZ.SETTING_WHISPER_STATE.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xd0);
	pkt_buf.writeUChar(this.type);
	return pkt_buf;
};

// 0xd3
PACKET.CZ.REQ_WHISPER_LIST = function PACKET_CZ_REQ_WHISPER_LIST() {};
PACKET.CZ.REQ_WHISPER_LIST.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xd3);
	return pkt_buf;
};

// 0xd5
PACKET.CZ.CREATE_CHATROOM = function PACKET_CZ_CREATE_CHATROOM() {
	this.size = 0;
	this.type = 0;
	this.passwd = '';
	this.title = '';
};
PACKET.CZ.CREATE_CHATROOM.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 1 + 8 + this.title.length;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xd5);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeShort(this.size);
	pkt_buf.writeUChar(this.type);
	pkt_buf.writeString(this.passwd, 8);
	pkt_buf.writeString(this.title);
	return pkt_buf;
};

// 0xd9
PACKET.CZ.REQ_ENTER_ROOM = function PACKET_CZ_REQ_ENTER_ROOM() {
	this.roomID = 0;
	this.passwd = '';
};
PACKET.CZ.REQ_ENTER_ROOM.prototype.build = function () {
	const pkt_len = 2 + 4 + 8;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xd9);
	pkt_buf.writeULong(this.roomID);
	pkt_buf.writeString(this.passwd, 8);
	return pkt_buf;
};

// 0xde
PACKET.CZ.CHANGE_CHATROOM = function PACKET_CZ_CHANGE_CHATROOM() {
	this.size = 0;
	this.type = 0;
	this.passwd = '';
	this.title = '';
};
PACKET.CZ.CHANGE_CHATROOM.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 1 + 8 + this.title.length;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xde);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeShort(this.size);
	pkt_buf.writeUChar(this.type);
	pkt_buf.writeString(this.passwd, 8);
	pkt_buf.writeString(this.title);
	return pkt_buf;
};

// 0xe0
PACKET.CZ.REQ_ROLE_CHANGE = function PACKET_CZ_REQ_ROLE_CHANGE() {
	this.role = 0;
	this.name = '';
};
PACKET.CZ.REQ_ROLE_CHANGE.prototype.build = function () {
	const pkt_len = 2 + 4 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xe0);
	pkt_buf.writeULong(this.role);
	pkt_buf.writeString(this.name, 24);
	return pkt_buf;
};

// 0xe2
PACKET.CZ.REQ_EXPEL_MEMBER = function PACKET_CZ_REQ_EXPEL_MEMBER() {
	this.name = '';
};
PACKET.CZ.REQ_EXPEL_MEMBER.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xe2);
	pkt_buf.writeString(this.name, 24);
	return pkt_buf;
};

// 0xe3
PACKET.CZ.EXIT_ROOM = function PACKET_CZ_EXIT_ROOM() {};
PACKET.CZ.EXIT_ROOM.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xe3);
	return pkt_buf;
};

// 0xe4
PACKET.CZ.REQ_EXCHANGE_ITEM = function PACKET_CZ_REQ_EXCHANGE_ITEM() {
	this.AID = 0;
};
PACKET.CZ.REQ_EXCHANGE_ITEM.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xe4);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0xe6
PACKET.CZ.ACK_EXCHANGE_ITEM = function PACKET_CZ_ACK_EXCHANGE_ITEM() {
	this.result = 0;
};
PACKET.CZ.ACK_EXCHANGE_ITEM.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xe6);
	pkt_buf.writeUChar(this.result);
	return pkt_buf;
};

// 0xe8
PACKET.CZ.ADD_EXCHANGE_ITEM = function PACKET_CZ_ADD_EXCHANGE_ITEM() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.ADD_EXCHANGE_ITEM.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xe8);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeLong(this.count);
	return pkt_buf;
};

// 0xeb
PACKET.CZ.CONCLUDE_EXCHANGE_ITEM = function PACKET_CZ_CONCLUDE_EXCHANGE_ITEM() {};
PACKET.CZ.CONCLUDE_EXCHANGE_ITEM.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xeb);
	return pkt_buf;
};

// 0xed
PACKET.CZ.CANCEL_EXCHANGE_ITEM = function PACKET_CZ_CANCEL_EXCHANGE_ITEM() {};
PACKET.CZ.CANCEL_EXCHANGE_ITEM.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xed);
	return pkt_buf;
};

// 0xef
PACKET.CZ.EXEC_EXCHANGE_ITEM = function PACKET_CZ_EXEC_EXCHANGE_ITEM() {};
PACKET.CZ.EXEC_EXCHANGE_ITEM.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xef);
	return pkt_buf;
};

// 0xf3
PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_STORE = function PACKET_CZ_MOVE_ITEM_FROM_BODY_TO_STORE() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_STORE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.index, true);
	pkt.view.setInt32(ver[4], this.count, true);
	return pkt;
};

// 0xf5
PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_BODY = function PACKET_CZ_MOVE_ITEM_FROM_STORE_TO_BODY() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_BODY.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.index, true);
	pkt.view.setInt32(ver[4], this.count, true);
	return pkt;
};

// 0xf7
PACKET.CZ.CLOSE_STORE = function PACKET_CZ_CLOSE_STORE() {};
PACKET.CZ.CLOSE_STORE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);

	return pkt;
};

// 0xf9
PACKET.CZ.MAKE_GROUP = function PACKET_CZ_MAKE_GROUP() {
	this.groupName = '';
};
PACKET.CZ.MAKE_GROUP.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xf9);
	pkt_buf.writeString(this.groupName, 24);
	return pkt_buf;
};

// 0xfc
PACKET.CZ.REQ_JOIN_GROUP = function PACKET_CZ_REQ_JOIN_GROUP() {
	this.AID = 0;
	this.CharName = '';
};
PACKET.CZ.REQ_JOIN_GROUP.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	if (ver[2] === 26) {
		pkt.view.setString(ver[3], this.CharName, 24);
	} else {
		pkt.view.setInt32(ver[3], this.AID, true);
	}
	return pkt;
};

// 0xff
PACKET.CZ.JOIN_GROUP = function PACKET_CZ_JOIN_GROUP() {
	this.GRID = 0;
	this.answer = 0;
};
PACKET.CZ.JOIN_GROUP.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xff);
	pkt_buf.writeULong(this.GRID);
	pkt_buf.writeLong(this.answer);
	return pkt_buf;
};

// 0x100
PACKET.CZ.REQ_LEAVE_GROUP = function PACKET_CZ_REQ_LEAVE_GROUP() {};
PACKET.CZ.REQ_LEAVE_GROUP.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x100);
	return pkt_buf;
};

// 0x102
PACKET.CZ.CHANGE_GROUPEXPOPTION = function PACKET_CZ_CHANGE_GROUPEXPOPTION() {
	this.expOption = 0;
};
PACKET.CZ.CHANGE_GROUPEXPOPTION.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x102);
	pkt_buf.writeULong(this.expOption);
	return pkt_buf;
};

// 0x103
PACKET.CZ.REQ_EXPEL_GROUP_MEMBER = function PACKET_CZ_REQ_EXPEL_GROUP_MEMBER() {
	this.AID = 0;
	this.characterName = '';
};
PACKET.CZ.REQ_EXPEL_GROUP_MEMBER.prototype.build = function () {
	const pkt_len = 2 + 4 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x103);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeString(this.characterName, 24);
	return pkt_buf;
};

// 0x108
PACKET.CZ.REQUEST_CHAT_PARTY = function PACKET_CZ_REQUEST_CHAT_PARTY() {
	this.msg = '';
};
PACKET.CZ.REQUEST_CHAT_PARTY.prototype.build = function () {
	const pkt_len = 2 + 2 + this.msg.length + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x108);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x112
PACKET.CZ.UPGRADE_SKILLLEVEL = function PACKET_CZ_UPGRADE_SKILLLEVEL() {
	this.SKID = 0;
};
PACKET.CZ.UPGRADE_SKILLLEVEL.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x112);
	pkt_buf.writeUShort(this.SKID);
	return pkt_buf;
};

// 0x113
PACKET.CZ.USE_SKILL = function PACKET_CZ_USE_SKILL() {
	this.selectedLevel = 0;
	this.SKID = 0;
	this.targetID = 0;
};
PACKET.CZ.USE_SKILL.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.selectedLevel, true);
	pkt.view.setUint16(ver[4], this.SKID, true);
	pkt.view.setUint32(ver[5], this.targetID, true);
	return pkt;
};

// 0x116
PACKET.CZ.USE_SKILL_TOGROUND = function PACKET_CZ_USE_SKILL_TOGROUND() {
	this.selectedLevel = 0;
	this.SKID = 0;
	this.xPos = 0;
	this.yPos = 0;
};
PACKET.CZ.USE_SKILL_TOGROUND.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.selectedLevel, true);
	pkt.view.setInt16(ver[4], this.SKID, true);
	pkt.view.setInt16(ver[5], this.xPos, true);
	pkt.view.setInt16(ver[6], this.yPos, true);
	return pkt;
};

// 0x118
PACKET.CZ.CANCEL_LOCKON = function PACKET_CZ_CANCEL_LOCKON() {};
PACKET.CZ.CANCEL_LOCKON.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x118);
	return pkt_buf;
};

// 0x11b
PACKET.CZ.SELECT_WARPPOINT = function PACKET_CZ_SELECT_WARPPOINT() {
	this.SKID = 0;
	this.mapName = '';
};
PACKET.CZ.SELECT_WARPPOINT.prototype.build = function () {
	const pkt_len = 2 + 2 + 16;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x11b);
	pkt_buf.writeUShort(this.SKID);
	pkt_buf.writeString(this.mapName, 16);
	return pkt_buf;
};

// 0x11d
PACKET.CZ.REMEMBER_WARPPOINT = function PACKET_CZ_REMEMBER_WARPPOINT() {};
PACKET.CZ.REMEMBER_WARPPOINT.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x11d);
	return pkt_buf;
};

// 0x126
PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_CART = function PACKET_CZ_MOVE_ITEM_FROM_BODY_TO_CART() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_CART.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x126);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeLong(this.count);
	return pkt_buf;
};

// 0x127
PACKET.CZ.MOVE_ITEM_FROM_CART_TO_BODY = function PACKET_CZ_MOVE_ITEM_FROM_CART_TO_BODY() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MOVE_ITEM_FROM_CART_TO_BODY.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x127);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeLong(this.count);
	return pkt_buf;
};

// 0x128
PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_CART = function PACKET_CZ_MOVE_ITEM_FROM_STORE_TO_CART() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_CART.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x128);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeLong(this.count);
	return pkt_buf;
};

// 0x129
PACKET.CZ.MOVE_ITEM_FROM_CART_TO_STORE = function PACKET_CZ_MOVE_ITEM_FROM_CART_TO_STORE() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MOVE_ITEM_FROM_CART_TO_STORE.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x129);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeLong(this.count);
	return pkt_buf;
};

// 0x12a
PACKET.CZ.REQ_CARTOFF = function PACKET_CZ_REQ_CARTOFF() {};
PACKET.CZ.REQ_CARTOFF.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x12a);
	return pkt_buf;
};

// 0x12e
PACKET.CZ.REQ_CLOSESTORE = function PACKET_CZ_REQ_CLOSESTORE() {};
PACKET.CZ.REQ_CLOSESTORE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x12e);
	return pkt_buf;
};

// 0x12f
PACKET.CZ.REQ_OPENSTORE = function PACKET_CZ_REQ_OPENSTORE() {
	this.storeName = '';
	this.storeList = [];
};
PACKET.CZ.REQ_OPENSTORE.prototype.build = function () {
	const pkt_len = 2 + 2 + 80 + this.storeList.length * 8;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x12f);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.storeName, 80);

	const count = this.storeList.length;
	for (let i = 0; i < count; ++i) {
		pkt_buf.writeShort(this.storeList[i].index);
		pkt_buf.writeShort(this.storeList[i].count);
		pkt_buf.writeLong(this.storeList[i].Price);
	}

	return pkt_buf;
};

// 0x130
PACKET.CZ.REQ_BUY_FROMMC = function PACKET_CZ_REQ_BUY_FROMMC() {
	this.AID = 0;
};
PACKET.CZ.REQ_BUY_FROMMC.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x130);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x134
PACKET.CZ.PC_PURCHASE_ITEMLIST_FROMMC = function PACKET_CZ_PC_PURCHASE_ITEMLIST_FROMMC() {
	this.AID = 0;
	this.itemList = [];
};
PACKET.CZ.PC_PURCHASE_ITEMLIST_FROMMC.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + this.itemList.length * 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x134);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeULong(this.AID);

	const count = this.itemList.length;
	for (let i = 0; i < count; ++i) {
		pkt_buf.writeShort(this.itemList[i].count);
		pkt_buf.writeShort(this.itemList[i].index);
	}

	return pkt_buf;
};

// 0x138
PACKET.CZ.PKMODE_CHANGE = function PACKET_CZ_PKMODE_CHANGE() {
	this.isTurnOn = 0;
};
PACKET.CZ.PKMODE_CHANGE.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x138);
	pkt_buf.writeUChar(this.isTurnOn);
	return pkt_buf;
};

// 0x143
PACKET.CZ.INPUT_EDITDLG = function PACKET_CZ_INPUT_EDITDLG() {
	this.NAID = 0;
	this.value = 0;
};
PACKET.CZ.INPUT_EDITDLG.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x143);
	pkt_buf.writeULong(this.NAID);
	pkt_buf.writeLong(this.value);
	return pkt_buf;
};

// 0x146
PACKET.CZ.CLOSE_DIALOG = function PACKET_CZ_CLOSE_DIALOG() {
	this.NAID = 0;
};
PACKET.CZ.CLOSE_DIALOG.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x146);
	pkt_buf.writeULong(this.NAID);
	return pkt_buf;
};

// 0x14d
PACKET.CZ.REQ_GUILD_MENUINTERFACE = function PACKET_CZ_REQ_GUILD_MENUINTERFACE() {};
PACKET.CZ.REQ_GUILD_MENUINTERFACE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x14d);
	return pkt_buf;
};

// 0x14f
PACKET.CZ.REQ_GUILD_MENU = function PACKET_CZ_REQ_GUILD_MENU() {
	this.Type = 0;
};
PACKET.CZ.REQ_GUILD_MENU.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x14f);
	pkt_buf.writeLong(this.Type);
	return pkt_buf;
};

// 0x151
PACKET.CZ.REQ_GUILD_EMBLEM_IMG = function PACKET_CZ_REQ_GUILD_EMBLEM_IMG() {
	this.GDID = 0;
};
PACKET.CZ.REQ_GUILD_EMBLEM_IMG.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x151);
	pkt_buf.writeLong(this.GDID);
	return pkt_buf;
};

// 0x153
PACKET.CZ.REGISTER_GUILD_EMBLEM_IMG = function PACKET_CZ_REGISTER_GUILD_EMBLEM_IMG() {
	this.img = null;
};
PACKET.CZ.REGISTER_GUILD_EMBLEM_IMG.prototype.build = function () {
	const pkt_len = 2 + 2 + this.img.byteLength;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x153);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeBuffer(this.img);

	return pkt_buf;
};

// 0x155
PACKET.CZ.REQ_CHANGE_MEMBERPOS = function PACKET_CZ_REQ_CHANGE_MEMBERPOS() {
	this.memberInfo = [];
};
PACKET.CZ.REQ_CHANGE_MEMBERPOS.prototype.build = function () {
	const pkt_len = 2 + 2 + this.memberInfo.length * 12;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x155);
	pkt_buf.writeShort(pkt_len);

	const count = this.memberInfo.length;
	for (let i = 0; i < count; ++i) {
		pkt_buf.writeLong(this.memberInfo[i].AID);
		pkt_buf.writeLong(this.memberInfo[i].GID);
		pkt_buf.writeLong(this.memberInfo[i].positionID);
	}

	return pkt_buf;
};

// 0x157
PACKET.CZ.REQ_OPEN_MEMBER_INFO = function PACKET_CZ_REQ_OPEN_MEMBER_INFO() {
	this.AID = 0;
};
PACKET.CZ.REQ_OPEN_MEMBER_INFO.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x157);
	pkt_buf.writeLong(this.AID);
	return pkt_buf;
};

// 0x159
PACKET.CZ.REQ_LEAVE_GUILD = function PACKET_CZ_REQ_LEAVE_GUILD() {
	this.GDID = 0;
	this.AID = 0;
	this.GID = 0;
	this.reasonDesc = '';
};
PACKET.CZ.REQ_LEAVE_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4 + 40;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x159);
	pkt_buf.writeULong(this.GDID);
	pkt_buf.writeLong(this.AID);
	pkt_buf.writeLong(this.GID);
	pkt_buf.writeString(this.reasonDesc, 40);
	return pkt_buf;
};

// 0x15b
PACKET.CZ.REQ_BAN_GUILD = function PACKET_CZ_REQ_BAN_GUILD() {
	this.GDID = 0;
	this.AID = 0;
	this.GID = 0;
	this.reasonDesc = '';
};
PACKET.CZ.REQ_BAN_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4 + 40;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x15b);
	pkt_buf.writeULong(this.GDID);
	pkt_buf.writeLong(this.AID);
	pkt_buf.writeLong(this.GID);
	pkt_buf.writeString(this.reasonDesc, 40);
	return pkt_buf;
};

// 0x15d
PACKET.CZ.REQ_DISORGANIZE_GUILD = function PACKET_CZ_REQ_DISORGANIZE_GUILD() {
	this.key = '';
};
PACKET.CZ.REQ_DISORGANIZE_GUILD.prototype.build = function () {
	const pkt_len = 2 + 40;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x15d);
	pkt_buf.writeString(this.key, 40);
	return pkt_buf;
};

// 0x161
PACKET.CZ.REG_CHANGE_GUILD_POSITIONINFO = function PACKET_CZ_REG_CHANGE_GUILD_POSITIONINFO() {
	this.memberList = [];
};
PACKET.CZ.REG_CHANGE_GUILD_POSITIONINFO.prototype.build = function () {
	const pkt_len = 2 + 2 + this.memberList.length * 40;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x161);
	pkt_buf.writeShort(pkt_len);

	const count = this.memberList.length;
	for (let i = 0; i < count; ++i) {
		pkt_buf.writeLong(this.memberList[i].positionID);
		pkt_buf.writeLong(this.memberList[i].right);
		pkt_buf.writeLong(this.memberList[i].ranking);
		pkt_buf.writeLong(this.memberList[i].payRate);
		pkt_buf.writeString(this.memberList[i].posName, 24);
	}

	return pkt_buf;
};

// 0x165
PACKET.CZ.REQ_MAKE_GUILD = function PACKET_CZ_REQ_MAKE_GUILD() {
	this.GID = 0;
	this.GName = '';
};
PACKET.CZ.REQ_MAKE_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x165);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeString(this.GName, 24);
	return pkt_buf;
};

// 0x168
PACKET.CZ.REQ_JOIN_GUILD = function PACKET_CZ_REQ_JOIN_GUILD() {
	this.AID = 0;
	this.MyAID = 0;
	this.MyGID = 0;
};
PACKET.CZ.REQ_JOIN_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x168);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.MyAID);
	pkt_buf.writeULong(this.MyGID);
	return pkt_buf;
};

// 0x916
// Requires PACKETVER >= 20120131 (length table defines 0x916 only from that
// date). Sent by /guildinvite via Guild.requestPlayerInvitationByName, which
// guards the version and reports an error on older clients.
PACKET.CZ.REQ_JOIN_GUILD2 = function PACKET_CZ_REQ_JOIN_GUILD2() {
	this.name = '';
};
PACKET.CZ.REQ_JOIN_GUILD2.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x916);
	pkt_buf.writeString(this.name, 24);
	return pkt_buf;
};

// 0x16b
PACKET.CZ.JOIN_GUILD = function PACKET_CZ_JOIN_GUILD() {
	this.GDID = 0;
	this.answer = 0;
};
PACKET.CZ.JOIN_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x16b);
	pkt_buf.writeULong(this.GDID);
	pkt_buf.writeLong(this.answer);
	return pkt_buf;
};

// 0x16e
PACKET.CZ.GUILD_NOTICE = function PACKET_CZ_GUILD_NOTICE() {
	this.GDID = 0;
	this.subject = '';
	this.notice = '';
};
PACKET.CZ.GUILD_NOTICE.prototype.build = function () {
	const pkt_len = 2 + 4 + 60 + 120;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x16e);
	pkt_buf.writeULong(this.GDID);
	pkt_buf.writeString(this.subject, 60);
	pkt_buf.writeString(this.notice, 120);
	return pkt_buf;
};

// 0x170
PACKET.CZ.REQ_ALLY_GUILD = function PACKET_CZ_REQ_ALLY_GUILD() {
	this.AID = 0;
	this.MyAID = 0;
	this.MyGID = 0;
};
PACKET.CZ.REQ_ALLY_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x170);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.MyAID);
	pkt_buf.writeULong(this.MyGID);
	return pkt_buf;
};

// 0x172
PACKET.CZ.ALLY_GUILD = function PACKET_CZ_ALLY_GUILD() {
	this.otherAID = 0;
	this.answer = 0;
};
PACKET.CZ.ALLY_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x172);
	pkt_buf.writeULong(this.otherAID);
	pkt_buf.writeLong(this.answer);
	return pkt_buf;
};

// 0x175
PACKET.CZ.REQ_GUILD_MEMBER_INFO = function PACKET_CZ_REQ_GUILD_MEMBER_INFO() {
	this.GID = 0;
};
PACKET.CZ.REQ_GUILD_MEMBER_INFO.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x175);
	pkt_buf.writeLong(this.GID);
	return pkt_buf;
};

// 0x178
PACKET.CZ.REQ_ITEMIDENTIFY = function PACKET_CZ_REQ_ITEMIDENTIFY() {
	this.index = 0;
};
PACKET.CZ.REQ_ITEMIDENTIFY.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x178);
	pkt_buf.writeShort(this.index);
	return pkt_buf;
};

// 0x17a
PACKET.CZ.REQ_ITEMCOMPOSITION_LIST = function PACKET_CZ_REQ_ITEMCOMPOSITION_LIST() {
	this.cardIndex = 0;
};
PACKET.CZ.REQ_ITEMCOMPOSITION_LIST.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x17a);
	pkt_buf.writeShort(this.cardIndex);
	return pkt_buf;
};

// 0x17c
PACKET.CZ.REQ_ITEMCOMPOSITION = function PACKET_CZ_REQ_ITEMCOMPOSITION() {
	this.cardIndex = 0;
	this.equipIndex = 0;
};
PACKET.CZ.REQ_ITEMCOMPOSITION.prototype.build = function () {
	const pkt_len = 2 + 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x17c);
	pkt_buf.writeShort(this.cardIndex);
	pkt_buf.writeShort(this.equipIndex);
	return pkt_buf;
};

// 0x17e
PACKET.CZ.GUILD_CHAT = function PACKET_CZ_GUILD_CHAT() {
	this.msg = '';
};
PACKET.CZ.GUILD_CHAT.prototype.build = function () {
	const pkt_len = 2 + 2 + this.msg.length + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x17e);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x180
PACKET.CZ.REQ_HOSTILE_GUILD = function PACKET_CZ_REQ_HOSTILE_GUILD() {
	this.AID = 0;
};
PACKET.CZ.REQ_HOSTILE_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x180);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x183
PACKET.CZ.REQ_DELETE_RELATED_GUILD = function PACKET_CZ_REQ_DELETE_RELATED_GUILD() {
	this.OpponentGDID = 0;
	this.Relation = 0;
};
PACKET.CZ.REQ_DELETE_RELATED_GUILD.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x183);
	pkt_buf.writeULong(this.OpponentGDID);
	pkt_buf.writeLong(this.Relation);
	return pkt_buf;
};

// 0x187
PACKET.CZ.PING = function PACKET_CZ_PING() {
	this.AID = 0;
};
PACKET.CZ.PING.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x187);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x18a
PACKET.CZ.REQ_DISCONNECT = function PACKET_CZ_REQ_DISCONNECT() {
	this.type = 0;
};
PACKET.CZ.REQ_DISCONNECT.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x18a);
	pkt_buf.writeShort(this.type);
	return pkt_buf;
};

// 0x18e
PACKET.CZ.REQMAKINGITEM = function PACKET_CZ_REQMAKINGITEM() {
	this.itemList = {};
};
PACKET.CZ.REQMAKINGITEM.prototype.build = function () {
	const pkt_len = PACKETVER.value >= 20181121 ? 18 : 10;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x18e);
	if (PACKETVER.value >= 20181121) {
		pkt_buf.writeULong(this.itemList.ITID);
		pkt_buf.writeULong(this.itemList.material_ID[0]);
		pkt_buf.writeULong(this.itemList.material_ID[1]);
		pkt_buf.writeULong(this.itemList.material_ID[2]);
	} else {
		pkt_buf.writeUShort(this.itemList.ITID);
		pkt_buf.writeUShort(this.itemList.material_ID[0]);
		pkt_buf.writeUShort(this.itemList.material_ID[1]);
		pkt_buf.writeUShort(this.itemList.material_ID[2]);
	}
	return pkt_buf;
};

// 0x190
PACKET.CZ.USE_SKILL_TOGROUNDMoreInfo = function PACKET_CZ_USE_SKILL_TOGROUND_MoreInfo() {
	this.selectedLevel = 0;
	this.SKID = 0;
	this.xPos = 0;
	this.yPos = 0;
	this.contents = '';
};
PACKET.CZ.USE_SKILL_TOGROUNDMoreInfo.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.selectedLevel, true);
	pkt.view.setUint16(ver[4], this.SKID, true);
	pkt.view.setInt16(ver[5], this.xPos, true);
	pkt.view.setInt16(ver[6], this.yPos, true);
	pkt.view.setString(ver[7], this.contents, 80);
	return pkt;
};

// 0x193
PACKET.CZ.REQNAME_BYGID = function PACKET_CZ_REQNAME_BYGID() {
	this.GID = 0;
};
PACKET.CZ.REQNAME_BYGID.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.GID, true);
	return pkt;
};

// 0x9d4
PACKET.CZ.NPC_TRADE_QUIT = function PACKET_CZ_NPC_TRADE_QUIT() {};
PACKET.CZ.NPC_TRADE_QUIT.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9d4);
	return pkt_buf;
};

// 0x9d5
PACKET.ZC.NPC_MARKET_OPEN = function PACKET_ZC_NPC_MARKET_OPEN(fp, end) {
	this.itemList = (function () {
		// Determine item size based on PACKETVER
		const item_size = PACKETVER.value >= 20181121 ? 15 : 13; // Adjust sizes based on nameid (4 or 2 bytes)
		const count = ((end - fp.tell()) / item_size) | 0; // Calculate item count
		const out = new Array(count);

		for (let i = 0; i < count; ++i) {
			out[i] = {};
			// Parse fields with conditional handling for nameid
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort(); // uint32 or uint16
			out[i].type = fp.readUChar();
			out[i].price = fp.readULong();
			out[i].qty = fp.readULong();
			out[i].weight = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.NPC_MARKET_OPEN.size = -1;

// 0x9d6
PACKET.CZ.NPC_MARKET_PURCHASE = function PACKET_CZ_NPC_MARKET_PURCHASE() {
	this.itemList = [];
};
PACKET.CZ.NPC_MARKET_PURCHASE.prototype.build = function () {
	// Determine the item size based on PACKETVER
	const item_size = PACKETVER.value >= 20181121 ? 8 : 6; // 8 bytes (4 + 4) or 6 bytes (2 + 4)
	const pkt_len = 4 + this.itemList.length * item_size; // Total packet length
	const pkt_buf = new BinaryWriter(pkt_len); // Create a buffer with the required size

	pkt_buf.writeShort(0x9d6); // Packet type
	pkt_buf.writeShort(pkt_len); // Packet length

	for (let i = 0; i < this.itemList.length; ++i) {
		// Write ITID based on PACKETVER
		if (PACKETVER.value >= 20181121) {
			pkt_buf.writeULong(this.itemList[i].itemId); // uint32
		} else {
			pkt_buf.writeUShort(this.itemList[i].itemId); // uint16
		}

		// Write quantity (int32)
		pkt_buf.writeLong(this.itemList[i].amount); // int32
	}

	return pkt_buf;
};

// 0x9d7
PACKET.ZC.NPC_MARKET_PURCHASE_RESULT = function PACKET_ZC_NPC_MARKET_PURCHASE_RESULT(fp, end) {
	this.result = fp.readUChar();
	this.itemList = (function () {
		// Determine item size based on PACKETVER
		const item_size = PACKETVER.value >= 20181121 ? 10 : 8; // Adjust sizes based on nameid (4 or 2 bytes)
		const count = ((end - fp.tell()) / item_size) | 0; // Calculate item count
		const out = new Array(count);

		for (let i = 0; i < count; ++i) {
			out[i] = {};
			// Parse fields with conditional handling for nameid
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort(); // uint32 or uint16
			out[i].qty = fp.readUShort();
			out[i].price = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.NPC_MARKET_PURCHASE_RESULT.size = -1;

// 0x9d8
PACKET.CZ.NPC_MARKET_CLOSE = function PACKET_CZ_NPC_MARKET_CLOSE() {};
PACKET.CZ.NPC_MARKET_CLOSE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9d8);
	return pkt_buf;
};

// 0x19f
PACKET.CZ.TRYCAPTURE_MONSTER = function PACKET_CZ_TRYCAPTURE_MONSTER() {
	this.targetAID = 0;
};
PACKET.CZ.TRYCAPTURE_MONSTER.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x19f);
	pkt_buf.writeULong(this.targetAID);
	return pkt_buf;
};

// 0x1a1
PACKET.CZ.COMMAND_PET = function PACKET_CZ_COMMAND_PET() {
	this.cSub = 0;
};
PACKET.CZ.COMMAND_PET.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1a1);
	pkt_buf.writeChar(this.cSub);
	return pkt_buf;
};

// 0x1a5
PACKET.CZ.RENAME_PET = function PACKET_CZ_RENAME_PET() {
	this.szName = '';
};
PACKET.CZ.RENAME_PET.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1a5);
	pkt_buf.writeString(this.szName, 24);
	return pkt_buf;
};

// 0x1a7
PACKET.CZ.SELECT_PETEGG = function PACKET_CZ_SELECT_PETEGG() {
	this.index = 0;
};
PACKET.CZ.SELECT_PETEGG.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1a7);
	pkt_buf.writeShort(this.index);
	return pkt_buf;
};

// 0x1a8
PACKET.CZ.PETEGG_INFO = function PACKET_CZ_PETEGG_INFO() {
	this.index = 0;
};
PACKET.CZ.PETEGG_INFO.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1a8);
	pkt_buf.writeShort(this.index);
	return pkt_buf;
};

// 0x1a9
PACKET.CZ.PET_ACT = function PACKET_CZ_PET_ACT() {
	this.data = 0;
};
PACKET.CZ.PET_ACT.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1a9);
	pkt_buf.writeLong(this.data);
	return pkt_buf;
};

// 0x1ae
PACKET.CZ.REQ_MAKINGARROW = function PACKET_CZ_REQ_MAKINGARROW() {
	this.id = 0;
};
PACKET.CZ.REQ_MAKINGARROW.prototype.build = function () {
	let pkt_len = 2 + 2;
	if (PACKETVER.value >= 20181121) {
		pkt_len += 2;
	}
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1ae);
	if (PACKETVER.value >= 20181121) {
		pkt_buf.writeULong(this.id);
	} else {
		pkt_buf.writeUShort(this.id);
	}
	return pkt_buf;
};

// 0x1af
PACKET.CZ.REQ_CHANGECART = function PACKET_CZ_REQ_CHANGECART() {
	this.num = 0;
};
PACKET.CZ.REQ_CHANGECART.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1af);
	pkt_buf.writeShort(this.num);
	return pkt_buf;
};

// 0x1b2
PACKET.CZ.REQ_OPENSTORE2 = function PACKET_CZ_REQ_OPENSTORE2() {
	this.storeName = '';
	this.result = 0;
	this.storeList = [];
};
PACKET.CZ.REQ_OPENSTORE2.prototype.build = function () {
	const pkt_len = 2 + 2 + 80 + 1 + this.storeList.length * 8;
	const pkt_buf = new BinaryWriter(pkt_len);
	let i, count;

	pkt_buf.writeShort(0x1b2);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.storeName, 80);
	pkt_buf.writeUChar(this.result);

	for (i = 0, count = this.storeList.length; i < count; ++i) {
		pkt_buf.writeShort(this.storeList[i].index);
		pkt_buf.writeShort(this.storeList[i].count);
		pkt_buf.writeLong(this.storeList[i].price);
	}

	return pkt_buf;
};

// 0x1b7
PACKET.CZ.GUILD_ZENY = function PACKET_CZ_GUILD_ZENY() {
	this.zeny = 0;
};
PACKET.CZ.GUILD_ZENY.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1b7);
	pkt_buf.writeLong(this.zeny);
	return pkt_buf;
};

// 0x1bb
PACKET.CZ.SHIFT = function PACKET_CZ_SHIFT() {
	this.CharacterName = '';
};
PACKET.CZ.SHIFT.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1bb);
	pkt_buf.writeString(this.CharacterName, 24);
	return pkt_buf;
};

// 0x1bf
PACKET.CA.REPLY_PNGAMEROOM = function PACKET_CA_REPLY_PNGAMEROOM() {
	this.Permission = 0;
};
PACKET.CA.REPLY_PNGAMEROOM.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1bf);
	pkt_buf.writeUChar(this.Permission);
	return pkt_buf;
};

// 0x1c0
PACKET.CZ.REQ_REMAINTIME = function PACKET_CZ_REQ_REMAINTIME() {};
PACKET.CZ.REQ_REMAINTIME.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1c0);
	return pkt_buf;
};

// 0x1c6
PACKET.CS.REQ_ENCRYPTION = function PACKET_CS_REQ_ENCRYPTION() {
	this.encCount = 0;
	this.decCount = 0;
};
PACKET.CS.REQ_ENCRYPTION.prototype.build = function () {
	const pkt_len = 2 + 1 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1c6);
	pkt_buf.writeChar(this.encCount);
	pkt_buf.writeChar(this.decCount);
	return pkt_buf;
};

// 0x1ca
PACKET.CZ.REQMAKINGHOMUN = function PACKET_CZ_REQMAKINGHOMUN() {
	this.result = 0;
};
PACKET.CZ.REQMAKINGHOMUN.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1ca);
	pkt_buf.writeUChar(this.result);
	return pkt_buf;
};

// 0x1cb
PACKET.CZ.MONSTER_TALK = function PACKET_CZ_MONSTER_TALK() {
	this.GID = 0;
	this.stateId = 0;
	this.skillId = 0;
	this.arg1 = 0;
};
PACKET.CZ.MONSTER_TALK.prototype.build = function () {
	const pkt_len = 2 + 4 + 1 + 1 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1cb);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeUChar(this.stateId);
	pkt_buf.writeUChar(this.skillId);
	pkt_buf.writeUChar(this.arg1);
	return pkt_buf;
};

// 0x1ce
PACKET.CZ.SELECTAUTOSPELL = function PACKET_CZ_SELECTAUTOSPELL() {
	this.SKID = 0;
};
PACKET.CZ.SELECTAUTOSPELL.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1ce);
	pkt_buf.writeLong(this.SKID);
	return pkt_buf;
};

// 0x1d5
PACKET.CZ.INPUT_EDITDLGSTR = function PACKET_CZ_INPUT_EDITDLGSTR() {
	this.NAID = 0;
	this.msg = '';
};
PACKET.CZ.INPUT_EDITDLGSTR.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + this.msg.length + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1d5);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeULong(this.NAID);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x1db
PACKET.CA.REQ_HASH = function PACKET_CA_REQ_HASH() {};
PACKET.CA.REQ_HASH.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1db);
	return pkt_buf;
};

// 0x1dd
PACKET.CA.LOGIN2 = function PACKET_CA_LOGIN2() {
	this.Version = 0;
	this.ID = '';
	this.PasswdMD5 = '';
	this.clienttype = 0;
};
PACKET.CA.LOGIN2.prototype.build = function () {
	const pkt_len = 2 + 4 + 24 + 16 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1dd);
	pkt_buf.writeULong(this.Version);
	pkt_buf.writeString(this.ID, 24);
	pkt_buf.writeString(this.PasswdMD5, 16);
	pkt_buf.writeUChar(this.clienttype);
	return pkt_buf;
};

/*// 0x01b2
	PACKET.CZ.REQ_OPEN_VENDING = function PACKET_CZ_REQ_REQ_OPEN_VENDING() {
		this.shop_name = '';
		this.result = 0;
		this.itemlist = [];
	};
	PACKET.CZ.REQ_OPEN_VENDING.prototype.build = function() {
		let i = 0;
		let ver = this.getPacketVersion();
		let pkt_len = 2+2+80+1+itemlist.length*8;
		let pkt_buf = new BinaryWriter(len);
		pkt_buf.writeShort(ver[1]);
		pkt_buf.writeShort(pkt_len);
		pkt_buf.writeString(this.shop_name,80);
		pkt_buf.writeChar(this.result);
		for(i=0; i < this.itemlist.length; i += 1)
		{
			pkt_buf.writeShort(this.itemlist[i].index);
			pkt_buf.writeShort(this.itemlist[i].amount);
			pkt_buf.writeLong(this.itemlist[i].price);
		}
		return pkt_buf;
	};*/

// 0x1df
PACKET.CZ.REQ_ACCOUNTNAME = function PACKET_CZ_REQ_ACCOUNTNAME() {
	this.AID = 0;
};
PACKET.CZ.REQ_ACCOUNTNAME.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.AID, true);
	return pkt;
};

// 0x1e3
PACKET.CZ.JOIN_COUPLE = function PACKET_CZ_JOIN_COUPLE() {
	this.AID = 0;
	this.GID = 0;
	this.answer = 0;
};
PACKET.CZ.JOIN_COUPLE.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1e3);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeLong(this.answer);
	return pkt_buf;
};

// 0x1e5
PACKET.CZ.REQ_JOIN_COUPLE = function PACKET_CZ_REQ_JOIN_COUPLE() {
	this.AID = 0;
};
PACKET.CZ.REQ_JOIN_COUPLE.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1e5);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x1e7
PACKET.CZ.DORIDORI = function PACKET_CZ_DORIDORI() {};
PACKET.CZ.DORIDORI.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1e7);
	return pkt_buf;
};

// 0x1e8
PACKET.CZ.MAKE_GROUP2 = function PACKET_CZ_MAKE_GROUP2() {
	this.groupName = '';
	this.ItemPickupRule = 0;
	this.ItemDivisionRule = 0;
};
PACKET.CZ.MAKE_GROUP2.prototype.build = function () {
	const pkt_len = 2 + 24 + 1 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1e8);
	pkt_buf.writeString(this.groupName, 24);
	pkt_buf.writeUChar(this.ItemPickupRule);
	pkt_buf.writeUChar(this.ItemDivisionRule);
	return pkt_buf;
};

// 0x1ed
PACKET.CZ.CHOPOKGI = function PACKET_CZ_CHOPOKGI() {};
PACKET.CZ.CHOPOKGI.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1ed);
	return pkt_buf;
};

// 0x1f7
PACKET.CZ.JOIN_BABY = function PACKET_CZ_JOIN_BABY() {
	this.AID = 0;
	this.GID = 0;
	this.answer = 0;
};
PACKET.CZ.JOIN_BABY.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1f7);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeLong(this.answer);
	return pkt_buf;
};

// 0x1f9
PACKET.CZ.REQ_JOIN_BABY = function PACKET_CZ_REQ_JOIN_BABY() {
	this.AID = 0;
};
PACKET.CZ.REQ_JOIN_BABY.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1f9);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x1fa
PACKET.CA.LOGIN3 = function PACKET_CA_LOGIN3() {
	this.Version = 0;
	this.ID = '';
	this.PasswdMD5 = '';
	this.clienttype = 0;
	this.ClientInfo = 0;
};
PACKET.CA.LOGIN3.prototype.build = function () {
	const pkt_len = 2 + 4 + 24 + 16 + 1 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1fa);
	pkt_buf.writeULong(this.Version);
	pkt_buf.writeString(this.ID, 24);
	pkt_buf.writeString(this.PasswdMD5, 16);
	pkt_buf.writeUChar(this.clienttype);
	pkt_buf.writeUChar(this.ClientInfo);
	return pkt_buf;
};

// 0x1fb
PACKET.CH.DELETE_CHAR2 = function PACKET_CH_DELETE_CHAR2() {
	this.GID = 0;
	this.key = '';
};
PACKET.CH.DELETE_CHAR2.prototype.build = function () {
	const pkt_len = 2 + 4 + 50;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1fb);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeString(this.key, 50);
	return pkt_buf;
};

// 0x1fd
PACKET.CZ.REQ_ITEMREPAIR = function PACKET_CZ_REQ_ITEMREPAIR() {
	this.index = 0;
	this.itemId = 0;
	this.RefiningLevel = 0;
	this.slots = {
		card1: 0,
		card2: 0,
		card3: 0,
		card4: 0
	};
};
PACKET.CZ.REQ_ITEMREPAIR.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.writeShort(this.index);

	if (ver[2] === 15) {
		if (PACKETVER.value >= 20181121) {
			pkt.writeULong(this.itemId);
		} else {
			pkt.writeUShort(this.itemId);
		}

		pkt.writeUByte(this.RefiningLevel);

		if (PACKETVER.value >= 20181121) {
			pkt.writeLong(this.slots.card1);
			pkt.writeLong(this.slots.card2);
			pkt.writeLong(this.slots.card3);
			pkt.writeLong(this.slots.card4);
		} else {
			pkt.writeShort(this.slots.card1);
			pkt.writeShort(this.slots.card2);
			pkt.writeShort(this.slots.card3);
			pkt.writeShort(this.slots.card4);
		}
	}

	return pkt;
};

// 0x200
PACKET.CA.CONNECT_INFO_CHANGED = function PACKET_CA_CONNECT_INFO_CHANGED() {
	this.ID = '';
};
PACKET.CA.CONNECT_INFO_CHANGED.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x200);
	pkt_buf.writeString(this.ID, 24);
	return pkt_buf;
};

// 0x202
PACKET.CZ.ADD_FRIENDS = function PACKET_CZ_ADD_FRIENDS() {
	this.name = '';
};
PACKET.CZ.ADD_FRIENDS.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setString(ver[3], this.name, 24);
	return pkt;
};

// 0x203
PACKET.CZ.DELETE_FRIENDS = function PACKET_CZ_DELETE_FRIENDS() {
	this.AID = 0;
	this.GID = 0;
};
PACKET.CZ.DELETE_FRIENDS.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x203);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.GID);
	return pkt_buf;
};

// 0x204
PACKET.CA.EXE_HASHCHECK = function PACKET_CA_EXE_HASHCHECK() {
	this.HashValue = '';
};
PACKET.CA.EXE_HASHCHECK.prototype.build = function () {
	const pkt_len = 2 + 16;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x204);
	pkt_buf.writeBinaryString(this.HashValue, 16);
	return pkt_buf;
};

// 0x208
PACKET.CZ.ACK_REQ_ADD_FRIENDS = function PACKET_CZ_ACK_REQ_ADD_FRIENDS() {
	this.ReqAID = 0;
	this.ReqGID = 0;
	this.Result = 0;
};
PACKET.CZ.ACK_REQ_ADD_FRIENDS.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.ReqAID, true);
	pkt.view.setUint32(ver[4], this.ReqGID, true);

	if (ver[2] === 11) {
		pkt.view.setInt8(ver[5], this.Result, true);
	} else {
		pkt.view.setInt32(ver[5], this.Result, true);
	}

	return pkt;
};

// 0x20b
PACKET.CH.EXE_HASHCHECK = function PACKET_CH_EXE_HASHCHECK() {
	this.ClientType = 0;
	this.HashValue = '';
};
PACKET.CH.EXE_HASHCHECK.prototype.build = function () {
	const pkt_len = 2 + 1 + 16;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x20b);
	pkt_buf.writeUChar(this.ClientType);
	pkt_buf.writeBinaryString(this.HashValue, 16);
	return pkt_buf;
};

// 0x20c
PACKET.CZ.EXE_HASHCHECK = function PACKET_CZ_EXE_HASHCHECK() {
	this.ClientType = 0;
	this.HashValue = '';
};
PACKET.CZ.EXE_HASHCHECK.prototype.build = function () {
	const pkt_len = 2 + 1 + 16;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x20c);
	pkt_buf.writeUChar(this.ClientType);
	pkt_buf.writeBinaryString(this.HashValue, 16);
	return pkt_buf;
};

// 0x20f
PACKET.CZ.REQ_PVPPOINT = function PACKET_CZ_REQ_PVPPOINT() {
	this.AID = 0;
	this.GID = 0;
};
PACKET.CZ.REQ_PVPPOINT.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x20f);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.GID);
	return pkt_buf;
};

// 0x212
PACKET.CZ.REQ_GIVE_MANNER_BYNAME = function PACKET_CZ_REQ_GIVE_MANNER_BYNAME() {
	this.CharName = '';
};
PACKET.CZ.REQ_GIVE_MANNER_BYNAME.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x212);
	pkt_buf.writeString(this.CharName, 24);
	return pkt_buf;
};

// 0x213
PACKET.CZ.REQ_STATUS_GM = function PACKET_CZ_REQ_STATUS_GM() {
	this.CharName = '';
};
PACKET.CZ.REQ_STATUS_GM.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x213);
	pkt_buf.writeString(this.CharName, 24);
	return pkt_buf;
};

// 0x217
PACKET.CZ.BLACKSMITH_RANK = function PACKET_CZ_BLACKSMITH_RANK() {};
PACKET.CZ.BLACKSMITH_RANK.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x217);
	return pkt_buf;
};

// 0x218
PACKET.CZ.ALCHEMIST_RANK = function PACKET_CZ_ALCHEMIST_RANK() {};
PACKET.CZ.ALCHEMIST_RANK.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x218);
	return pkt_buf;
};

// 0x21d
PACKET.CZ.LESSEFFECT = function PACKET_CZ_LESSEFFECT() {
	this.isLess = 0;
};
PACKET.CZ.LESSEFFECT.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x21d);
	pkt_buf.writeLong(this.isLess);
	return pkt_buf;
};

// 0x222
PACKET.CZ.REQ_WEAPONREFINE = function PACKET_CZ_REQ_WEAPONREFINE() {
	this.Index = 0;
};
PACKET.CZ.REQ_WEAPONREFINE.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x222);
	pkt_buf.writeLong(this.Index);
	return pkt_buf;
};

// 0x225
PACKET.CZ.TAEKWON_RANK = function PACKET_CZ_TAEKWON_RANK() {};
PACKET.CZ.TAEKWON_RANK.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x225);
	return pkt_buf;
};

// 0x228
PACKET.CZ.ACK_GAME_GUARD = function PACKET_CZ_ACK_GAME_GUARD() {
	this.AuthData = 0;
};
PACKET.CZ.ACK_GAME_GUARD.prototype.build = function () {
	const pkt_len = 2 + 16;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x228);
	pkt_buf.writeULong(this.AuthData[0]);
	pkt_buf.writeULong(this.AuthData[1]);
	pkt_buf.writeULong(this.AuthData[2]);
	pkt_buf.writeULong(this.AuthData[3]);
	return pkt_buf;
};

// 0x22d
PACKET.CZ.COMMAND_MER = function PACKET_CZ_COMMAND_MER() {
	this.type = 0;
	this.command = 0;
};
PACKET.CZ.COMMAND_MER.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.type, true);
	pkt.view.setInt8(ver[4], this.command, true);
	return pkt;
};

// 0x231
PACKET.CZ.RENAME_MER = function PACKET_CZ_RENAME_MER() {
	this.name = '';
};
PACKET.CZ.RENAME_MER.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x231);
	pkt_buf.writeString(this.name, 24);
	return pkt_buf;
};

// 0x232
PACKET.CZ.REQUEST_MOVENPC = function PACKET_CZ_REQUEST_MOVENPC() {
	this.GID = 0;
	this.dest = [0, 0];
};
PACKET.CZ.REQUEST_MOVENPC.prototype.build = function () {
	const pkt_len = 2 + 4 + 3;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x232);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writePos(this.dest);
	return pkt_buf;
};

// 0x233
PACKET.CZ.REQUEST_ACTNPC = function PACKET_CZ_REQUEST_ACTNPC() {
	this.GID = 0;
	this.targetGID = 0;
	this.action = 0;
};
PACKET.CZ.REQUEST_ACTNPC.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x233);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeULong(this.targetGID);
	pkt_buf.writeUChar(this.action);
	return pkt_buf;
};

// 0x234
PACKET.CZ.REQUEST_MOVETOOWNER = function PACKET_CZ_REQUEST_MOVETOOWNER() {
	this.GID = 0;
};
PACKET.CZ.REQUEST_MOVETOOWNER.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x234);
	pkt_buf.writeULong(this.GID);
	return pkt_buf;
};

// 0x235
PACKET.ZC.HOSKILLINFO_LIST = function PACKET_ZC_HOSKILLINFO_LIST(fp, end) {
	this.skillList = (function () {
		const count = ((end - fp.tell()) / 37) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].SKID = fp.readShort();
			out[i].type = fp.readLong();
			out[i].level = fp.readShort();
			out[i].spcost = fp.readShort();
			out[i].attackRange = fp.readShort();
			out[i].skillName = fp.readBinaryString(NAME_LENGTH);
			out[i].upgradable = fp.readChar();
		}
		return out;
	})();
};
PACKET.ZC.HOSKILLINFO_LIST.size = -1;

// 0x237
PACKET.CZ.RANKING_PK = function PACKET_CZ_RANKING_PK() {};
PACKET.CZ.RANKING_PK.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x237);
	return pkt_buf;
};

// 0x239
PACKET.ZC.HOSKILLINFO_UPDATE = function PACKET_ZC_HOSKILLINFO_UPDATE(fp, end) {
	this.SKID = fp.readUShort();
	this.level = fp.readShort();
	this.spcost = fp.readShort();
	this.attackRange = fp.readShort();
	this.upgradable = fp.readUChar();
};
PACKET.ZC.HOSKILLINFO_UPDATE.size = 11;

// 0x23b
PACKET.CZ.ACK_STORE_PASSWORD = function PACKET_CZ_ACK_STORE_PASSWORD() {
	this.Type = 0;
	this.Password = '';
	this.NewPassword = '';
};
PACKET.CZ.ACK_STORE_PASSWORD.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.Type, true);
	pkt.view.setString(ver[4], this.Password, 16);
	pkt.view.setString(ver[5], this.NewPassword, 16);
	return pkt;
};

// 0x23f
PACKET.CZ.MAIL_GET_LIST = function PACKET_CZ_MAIL_GET_LIST() {};
PACKET.CZ.MAIL_GET_LIST.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x23f);
	return pkt_buf;
};

// 0x241
PACKET.CZ.MAIL_OPEN = function PACKET_CZ_MAIL_OPEN() {
	this.MailID = 0;
};
PACKET.CZ.MAIL_OPEN.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x241);
	pkt_buf.writeLong(this.MailID);
	return pkt_buf;
};

// 0x243
PACKET.CZ.MAIL_DELETE = function PACKET_CZ_MAIL_DELETE() {
	this.MailID = 0;
};
PACKET.CZ.MAIL_DELETE.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x243);
	pkt_buf.writeLong(this.MailID);
	return pkt_buf;
};

// 0x244
PACKET.CZ.MAIL_GET_ITEM = function PACKET_CZ_MAIL_GET_ITEM() {
	this.MailID = 0;
};
PACKET.CZ.MAIL_GET_ITEM.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x244);
	pkt_buf.writeLong(this.MailID);
	return pkt_buf;
};

// 0x246
PACKET.CZ.MAIL_RESET_ITEM = function PACKET_CZ_MAIL_RESET_ITEM() {
	this.Type = 0;
};
PACKET.CZ.MAIL_RESET_ITEM.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x246);
	pkt_buf.writeShort(this.Type);
	return pkt_buf;
};

// 0x247
PACKET.CZ.MAIL_ADD_ITEM = function PACKET_CZ_MAIL_ADD_ITEM() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MAIL_ADD_ITEM.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.index, true);
	pkt.view.setInt32(ver[4], this.count, true);
	return pkt;
};

// 0x248
PACKET.CZ.MAIL_SEND = function PACKET_CZ_MAIL_SEND() {
	this.ReceiveName = '';
	this.Header = '';
	this.msg_len = 0;
	this.msg = '';
};
PACKET.CZ.MAIL_SEND.prototype.build = function () {
	const pkt_len = 2 + 2 + 24 + 40 + 1 + this.msg.length;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x248);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.ReceiveName, 24);
	pkt_buf.writeString(this.Header, 40);
	pkt_buf.writeUChar(this.msg_len);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x24b
PACKET.CZ.AUCTION_CREATE = function PACKET_CZ_AUCTION_CREATE() {
	this.Type = 0;
};
PACKET.CZ.AUCTION_CREATE.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x24b);
	pkt_buf.writeShort(this.Type);
	return pkt_buf;
};

// 0x24c
PACKET.CZ.AUCTION_ADD_ITEM = function PACKET_CZ_AUCTION_ADD_ITEM() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.AUCTION_ADD_ITEM.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x24c);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeLong(this.count);
	return pkt_buf;
};

// 0x24d
PACKET.CZ.AUCTION_ADD = function PACKET_CZ_AUCTION_ADD() {
	this.NowMoney = 0;
	this.MaxMoney = 0;
	this.DeleteHour = 0;
};
PACKET.CZ.AUCTION_ADD.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x24d);
	pkt_buf.writeULong(this.NowMoney);
	pkt_buf.writeULong(this.MaxMoney);
	pkt_buf.writeShort(this.DeleteHour);
	return pkt_buf;
};

// 0x24e
PACKET.CZ.AUCTION_ADD_CANCEL = function PACKET_CZ_AUCTION_ADD_CANCEL() {
	this.AuctionID = 0;
};
PACKET.CZ.AUCTION_ADD_CANCEL.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x24e);
	pkt_buf.writeULong(this.AuctionID);
	return pkt_buf;
};

// 0x24f
PACKET.CZ.AUCTION_BUY = function PACKET_CZ_AUCTION_BUY() {
	this.AuctionID = 0;
	this.Money = 0;
};
PACKET.CZ.AUCTION_BUY.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x24f);
	pkt_buf.writeULong(this.AuctionID);
	pkt_buf.writeULong(this.Money);
	return pkt_buf;
};

// 0x251
PACKET.CZ.AUCTION_ITEM_SEARCH = function PACKET_CZ_AUCTION_ITEM_SEARCH() {
	this.Type = 0;
	this.AuctionID = 0;
	this.Name = '';
	this.Page = 0;
};
PACKET.CZ.AUCTION_ITEM_SEARCH.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + 24 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x251);
	pkt_buf.writeShort(this.Type);
	pkt_buf.writeULong(this.AuctionID);
	pkt_buf.writeString(this.Name, 24);
	pkt_buf.writeUShort(this.Page);
	return pkt_buf;
};

// 0x254
PACKET.CZ.AGREE_STARPLACE = function PACKET_CZ_AGREE_STARPLACE() {
	this.which = 0;
};
PACKET.CZ.AGREE_STARPLACE.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x254);
	pkt_buf.writeChar(this.which);
	return pkt_buf;
};

// 0x258
PACKET.CA.REQ_GAME_GUARD_CHECK = function PACKET_CA_REQ_GAME_GUARD_CHECK() {};
PACKET.CA.REQ_GAME_GUARD_CHECK.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x258);
	return pkt_buf;
};

// 0x25b
PACKET.CZ.REQ_MAKINGITEM = function PACKET_CZ_REQ_MAKINGITEM() {
	this.mkType = 0;
	this.id = 0;
};
PACKET.CZ.REQ_MAKINGITEM.prototype.build = function () {
	const pkt_len = PACKETVER.value >= 20181121 ? 8 : 6;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x25b);
	pkt_buf.writeShort(this.mkType);
	if (PACKETVER.value >= 20181121) {
		pkt_buf.writeUShort(this.id);
	} else {
		pkt_buf.writeULong(this.id);
	}
	return pkt_buf;
};

// 0x25c
PACKET.CZ.AUCTION_REQ_MY_INFO = function PACKET_CZ_AUCTION_REQ_MY_INFO() {
	this.Type = 0;
};
PACKET.CZ.AUCTION_REQ_MY_INFO.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x25c);
	pkt_buf.writeShort(this.Type);
	return pkt_buf;
};

// 0x25d
PACKET.CZ.AUCTION_REQ_MY_SELL_STOP = function PACKET_CZ_AUCTION_REQ_MY_SELL_STOP() {
	this.AuctionID = 0;
};
PACKET.CZ.AUCTION_REQ_MY_SELL_STOP.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x25d);
	pkt_buf.writeULong(this.AuctionID);
	return pkt_buf;
};

// 0x264
PACKET.CA.ACK_LOGIN_OLDEKEY = function PACKET_CA_ACK_LOGIN_OLDEKEY() {
	this.m_SeedValue = '';
	this.m_EKey = '';
};
PACKET.CA.ACK_LOGIN_OLDEKEY.prototype.build = function () {
	const pkt_len = 2 + 9 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x264);
	pkt_buf.writeBinaryString(this.m_SeedValue, 9);
	pkt_buf.writeBinaryString(this.m_EKey, 9);
	return pkt_buf;
};

// 0x265
PACKET.CA.ACK_LOGIN_NEWEKEY = function PACKET_CA_ACK_LOGIN_NEWEKEY() {
	this.m_SeedValue = '';
	this.m_EKey = '';
};
PACKET.CA.ACK_LOGIN_NEWEKEY.prototype.build = function () {
	const pkt_len = 2 + 9 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x265);
	pkt_buf.writeBinaryString(this.m_SeedValue, 9);
	pkt_buf.writeBinaryString(this.m_EKey, 9);
	return pkt_buf;
};

// 0x266
PACKET.CA.ACK_LOGIN_CARDPASS = function PACKET_CA_ACK_LOGIN_CARDPASS() {
	this.m_cardPass = '';
};
PACKET.CA.ACK_LOGIN_CARDPASS.prototype.build = function () {
	const pkt_len = 2 + 28;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x266);
	pkt_buf.writeString(this.m_cardPass, 28);
	return pkt_buf;
};

// 0x271
PACKET.CA.ACK_LOGIN_ACCOUNT_INFO = function PACKET_CA_ACK_LOGIN_ACCOUNT_INFO() {
	this.sex = 0;
	this.bPoint = 0;
	this.E_mail = '';
};
PACKET.CA.ACK_LOGIN_ACCOUNT_INFO.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 34;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x271);
	pkt_buf.writeShort(this.sex);
	pkt_buf.writeShort(this.bPoint);
	pkt_buf.writeString(this.E_mail, 34);
	return pkt_buf;
};

// 0x273
PACKET.CZ.REQ_MAIL_RETURN = function PACKET_CZ_REQ_MAIL_RETURN() {
	this.MailID = 0;
	this.ReceiveName = '';
};
PACKET.CZ.REQ_MAIL_RETURN.prototype.build = function () {
	const pkt_len = 2 + 4 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x273);
	pkt_buf.writeLong(this.MailID);
	pkt_buf.writeString(this.ReceiveName, 24);
	return pkt_buf;
};

// 0x275
PACKET.CH.ENTER2 = function PACKET_CH_ENTER2() {
	this.AID = 0;
	this.AuthCode = 0;
	this.userLevel = 0;
	this.clientType = 0;
	this.Sex = 0;
	this.macData = '';
	this.iAccountSID = 0;
};
PACKET.CH.ENTER2.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4 + 2 + 1 + 16 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x275);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.AuthCode);
	pkt_buf.writeULong(this.userLevel);
	pkt_buf.writeUShort(this.clientType);
	pkt_buf.writeUChar(this.Sex);
	pkt_buf.writeBinaryString(this.macData, 16);
	pkt_buf.writeLong(this.iAccountSID);
	return pkt_buf;
};

// 0x277
PACKET.CA.LOGIN_PCBANG = function PACKET_CA_LOGIN_PCBANG() {
	this.Version = 0;
	this.ID = '';
	this.Passwd = '';
	this.clienttype = 0;
	this.IP = '';
	this.MacAdress = '';
};
PACKET.CA.LOGIN_PCBANG.prototype.build = function () {
	const pkt_len = 2 + 4 + 24 + 24 + 1 + 16 + 13;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x277);
	pkt_buf.writeULong(this.Version);
	pkt_buf.writeString(this.ID, 24);
	pkt_buf.writeString(this.Passwd, 24);
	pkt_buf.writeUChar(this.clienttype);
	pkt_buf.writeBinaryString(this.IP, 16);
	pkt_buf.writeBinaryString(this.MacAdress, 13);
	return pkt_buf;
};

// 0x279
PACKET.CZ.HUNTINGLIST = function PACKET_CZ_HUNTINGLIST() {};
PACKET.CZ.HUNTINGLIST.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x279);
	return pkt_buf;
};

// 0x27c
PACKET.CA.LOGIN4 = function PACKET_CA_LOGIN4() {
	this.Version = 0;
	this.ID = '';
	this.PasswdMD5 = '';
	this.clienttype = 0;
	this.macData = '';
};
PACKET.CA.LOGIN4.prototype.build = function () {
	const pkt_len = 2 + 4 + 24 + 16 + 1 + 13;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x27c);
	pkt_buf.writeULong(this.Version);
	pkt_buf.writeString(this.ID, 24);
	pkt_buf.writeString(this.PasswdMD5, 16);
	pkt_buf.writeUChar(this.clienttype);
	pkt_buf.writeBinaryString(this.macData, 13);
	return pkt_buf;
};

// 0x27f
PACKET.CA.CLIENT_TYPE = function PACKET_CA_CLIENT_TYPE() {
	this.ClientType = 0;
	this.nVer = 0;
};
PACKET.CA.CLIENT_TYPE.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x27f);
	pkt_buf.writeShort(this.ClientType);
	pkt_buf.writeLong(this.nVer);
	return pkt_buf;
};

// 0x281
PACKET.CZ.GANGSI_RANK = function PACKET_CZ_GANGSI_RANK() {
	this.PacketSwitch = 0;
};
PACKET.CZ.GANGSI_RANK.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x281);
	pkt_buf.writeShort(this.PacketSwitch);
	return pkt_buf;
};

// 0x286
PACKET.CZ.DEATH_QUESTION = function PACKET_CZ_DEATH_QUESTION() {
	this.Qanswer = 0;
};
PACKET.CZ.DEATH_QUESTION.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x286);
	pkt_buf.writeShort(this.Qanswer);
	return pkt_buf;
};

// 0x288
PACKET.CZ.PC_BUY_CASH_POINT_ITEM = function PACKET_CZ_PC_BUY_CASH_POINT_ITEM() {
	this.list = [];
	this.kafrapts = 0;
};
PACKET.CZ.PC_BUY_CASH_POINT_ITEM.prototype.build = function () {
	const ver = this.getPacketVersion();
	const itemSize = PACKETVER.value >= 20181121 ? 6 : 4;
	const len = 10 + this.list.length * itemSize;
	const pkt = new BinaryWriter(len);
	pkt.writeShort(ver[1]); // cmd
	if (PACKETVER.value < 20100803) {
		// can only buy 1 item per packet
		pkt.writeShort(this.list[0].ITID); // nameID
		pkt.writeShort(this.list[0].count); // amount
		if (PACKETVER.value >= 20070711) {
			pkt.writeShort(this.list[0].price);
		} // amount
		return pkt;
	}
	pkt.writeShort(len);
	pkt.writeULong(this.kafrapts, true);
	pkt.writeShort(this.list.length, true);
	const count = this.list.length;

	for (let i = 0; i < count; ++i) {
		pkt.writeShort(this.list[i].count);
		if (PACKETVER.value >= 20181121) {
			pkt.writeULong(this.list[i].ITID);
		} else {
			pkt.writeShort(this.list[i].ITID);
		}
	}
	return pkt;
};

// 0x28c
PACKET.CH.SELECT_CHAR_GOINGTOBEUSED = function PACKET_CH_SELECT_CHAR_GOINGTOBEUSED() {
	this.dwAID = 0;
	this.nCountSelectedChar = 0;
	this.ardwSelectedGID = 0;
};
PACKET.CH.SELECT_CHAR_GOINGTOBEUSED.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 36;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x28c);
	pkt_buf.writeULong(this.dwAID);
	pkt_buf.writeLong(this.nCountSelectedChar);
	pkt_buf.writeULong(this.ardwSelectedGID[0]);
	pkt_buf.writeULong(this.ardwSelectedGID[1]);
	pkt_buf.writeULong(this.ardwSelectedGID[2]);
	pkt_buf.writeULong(this.ardwSelectedGID[3]);
	return pkt_buf;
};

// 0x28d
PACKET.CH.REQ_IS_VALID_CHARNAME = function PACKET_CH_REQ_IS_VALID_CHARNAME() {
	this.dwAID = 0;
	this.dwGID = 0;
	this.szCharName = '';
};
PACKET.CH.REQ_IS_VALID_CHARNAME.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x28d);
	pkt_buf.writeULong(this.dwAID);
	pkt_buf.writeULong(this.dwGID);
	pkt_buf.writeString(this.szCharName, 24);
	return pkt_buf;
};

// 0x28f
PACKET.CH.REQ_CHANGE_CHARNAME = function PACKET_CH_REQ_CHANGE_CHARNAME() {
	this.dwGID = 0;
};
PACKET.CH.REQ_CHANGE_CHARNAME.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x28f);
	pkt_buf.writeULong(this.dwGID);
	return pkt_buf;
};

// 0x292
PACKET.CZ.STANDING_RESURRECTION = function PACKET_CZ_STANDING_RESURRECTION() {};
PACKET.CZ.STANDING_RESURRECTION.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x292);
	return pkt_buf;
};

// 0x29f
PACKET.CZ.MER_COMMAND = function PACKET_CZ_MER_COMMAND() {
	this.command = 0;
};
PACKET.CZ.MER_COMMAND.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x29f);
	pkt_buf.writeChar(this.command);
	return pkt_buf;
};

// 0x2a0
UNUSED_PACKET.CZ.MER_USE_SKILL = function UNUSED_PACKET_CZ_MER_USE_SKILL() {
	this.selectedLevel = 0;
	this.SKID = 0;
	this.targetID = 0;
};
UNUSED_PACKET.CZ.MER_USE_SKILL.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2a0);
	pkt_buf.writeShort(this.selectedLevel);
	pkt_buf.writeUShort(this.SKID);
	pkt_buf.writeULong(this.targetID);
	return pkt_buf;
};

// 0x2a1
UNUSED_PACKET.CZ.MER_UPGRADE_SKILLLEVEL = function UNUSED_PACKET_CZ_MER_UPGRADE_SKILLLEVEL() {
	this.SKID = 0;
};
UNUSED_PACKET.CZ.MER_UPGRADE_SKILLLEVEL.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2a1);
	pkt_buf.writeUShort(this.SKID);
	return pkt_buf;
};

// 0x2a5
PACKET.CZ.KSY_EVENT = function PACKET_CZ_KSY_EVENT() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.KSY_EVENT.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2a5);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeLong(this.count);
	return pkt_buf;
};

// 0x2ab
PACKET.CZ.ACK_CASH_PASSWORD = function PACKET_CZ_ACK_CASH_PASSWORD() {
	this.Type = 0;
	this.Password = '';
	this.NewPassword = '';
};
PACKET.CZ.ACK_CASH_PASSWORD.prototype.build = function () {
	const pkt_len = 2 + 2 + 16 + 16;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2ab);
	pkt_buf.writeShort(this.Type);
	pkt_buf.writeString(this.Password, 16);
	pkt_buf.writeString(this.NewPassword, 16);
	return pkt_buf;
};

// 0x2b0
PACKET.CA.LOGIN_HAN = function PACKET_CA_LOGIN_HAN() {
	this.Version = 0;
	this.ID = '';
	this.Passwd = '';
	this.clienttype = 0;
	this.m_szIP = '';
	this.m_szMacAddr = '';
	this.isHanGameUser = 0;
};
PACKET.CA.LOGIN_HAN.prototype.build = function () {
	const pkt_len = 2 + 4 + 24 + 24 + 1 + 16 + 13 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2b0);
	pkt_buf.writeULong(this.Version);
	pkt_buf.writeString(this.ID, 24);
	pkt_buf.writeBinaryString(this.Passwd, 24);
	pkt_buf.writeUChar(this.clienttype);
	pkt_buf.writeBinaryString(this.m_szIP, 16);
	pkt_buf.writeBinaryString(this.m_szMacAddr, 13);
	pkt_buf.writeUChar(this.isHanGameUser);
	return pkt_buf;
};

// 0x2b6
PACKET.CZ.ACTIVE_QUEST = function PACKET_CZ_ACTIVE_QUEST() {
	this.questID = 0;
	this.active = 0;
};
PACKET.CZ.ACTIVE_QUEST.prototype.build = function () {
	const pkt_len = 2 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2b6);
	pkt_buf.writeULong(this.questID);
	pkt_buf.writeUChar(this.active);
	return pkt_buf;
};

// 0x2ba
PACKET.CZ.SHORTCUT_KEY_CHANGE1 = function PACKET_CZ_SHORTCUT_KEY_CHANGE1() {
	this.Index = 0;
	this.ShortCutKey = {};
};
PACKET.CZ.SHORTCUT_KEY_CHANGE1.prototype.build = function () {
	const pkt_len = 2 + 2 + 1 + 4 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2ba);
	pkt_buf.writeUShort(this.Index);
	pkt_buf.writeChar(this.ShortCutKey.isSkill);
	pkt_buf.writeULong(this.ShortCutKey.ID);
	pkt_buf.writeShort(this.ShortCutKey.count);
	return pkt_buf;
};

// 0x2c0
PACKET.CZ.SRPACKETR2_START = function PACKET_CZ_SRPACKETR2_START() {
	this.ProtectFactor = 0;
};
PACKET.CZ.SRPACKETR2_START.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2c0);
	pkt_buf.writeUShort(this.ProtectFactor);
	return pkt_buf;
};

// 0x2c4
PACKET.CZ.PARTY_JOIN_REQ = function PACKET_CZ_PARTY_JOIN_REQ() {
	this.characterName = '';
};
PACKET.CZ.PARTY_JOIN_REQ.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.writeString(this.characterName, 24);
	return pkt;
};

// 0x2c7
PACKET.CZ.PARTY_JOIN_REQ_ACK = function PACKET_CZ_PARTY_JOIN_REQ_ACK() {
	this.GRID = 0;
	this.bAccept = 0;
};
PACKET.CZ.PARTY_JOIN_REQ_ACK.prototype.build = function () {
	const pkt_len = 2 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2c7);
	pkt_buf.writeULong(this.GRID);
	pkt_buf.writeUChar(this.bAccept);
	return pkt_buf;
};

// 0x2c8
PACKET.CZ.PARTY_CONFIG = function PACKET_CZ_PARTY_CONFIG() {
	this.bRefuseJoinMsg = 0;
};
PACKET.CZ.PARTY_CONFIG.prototype.build = function () {
	const pkt_len = 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2c8);
	pkt_buf.writeUChar(this.bRefuseJoinMsg);
	return pkt_buf;
};

// 0x2cf
PACKET.CZ.MEMORIALDUNGEON_COMMAND = function PACKET_CZ_MEMORIALDUNGEON_COMMAND() {
	this.Command = 0;
};
PACKET.CZ.MEMORIALDUNGEON_COMMAND.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2cf);
	pkt_buf.writeLong(this.Command);
	return pkt_buf;
};

// 0x2d6
PACKET.CZ.EQUIPWIN_MICROSCOPE = function PACKET_CZ_EQUIPWIN_MICROSCOPE() {
	this.AID = 0;
};
PACKET.CZ.EQUIPWIN_MICROSCOPE.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2d6);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x2d8
PACKET.CZ.CONFIG = function PACKET_CZ_CONFIG() {
	this.Config = 0;
	this.Value = 0;
};
PACKET.CZ.CONFIG.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2d8);
	pkt_buf.writeLong(this.Config);
	pkt_buf.writeLong(this.Value);
	return pkt_buf;
};

// 0x2db
PACKET.CZ.BATTLEFIELD_CHAT = function PACKET_CZ_BATTLEFIELD_CHAT() {
	this.msg = '';
};
PACKET.CZ.BATTLEFIELD_CHAT.prototype.build = function () {
	const pkt_len = 2 + 2 + this.msg.length;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2db);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x2e6
PACKET.CZ.BOT_CHECK = function PACKET_CZ_BOT_CHECK() {
	this.IsBot = 0;
};
PACKET.CZ.BOT_CHECK.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2e6);
	pkt_buf.writeLong(this.IsBot);
	return pkt_buf;
};

// 0x2f1
PACKET.CZ.PROGRESS = function PACKET_CZ_PROGRESS() {};
PACKET.CZ.PROGRESS.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x2f1);
	return pkt_buf;
};

// 0x35c
PACKET.CZ.OPEN_SIMPLE_CASHSHOP_ITEMLIST = function PACKET_CZ_OPEN_SIMPLE_CASHSHOP_ITEMLIST() {};
PACKET.CZ.OPEN_SIMPLE_CASHSHOP_ITEMLIST.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x35c);
	return pkt_buf;
};

// 0x35e
PACKET.CZ.CLOSE_WINDOW = function PACKET_CZ_CLOSE_WINDOW() {};
PACKET.CZ.CLOSE_WINDOW.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x35e);
	return pkt_buf;
};

// 0x35f
PACKET.CZ.REQUEST_MOVE2 = function PACKET_CZ_REQUEST_MOVE2() {
	this.dest = [0, 0];
};
PACKET.CZ.REQUEST_MOVE2.prototype.build = function () {
	const pkt_len = 2 + 3;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x35f);
	pkt_buf.writePos(this.dest);
	return pkt_buf;
};

// 0x360
PACKET.CZ.REQUEST_TIME2 = function PACKET_CZ_REQUEST_TIME2() {
	this.clientTime = 0;
};
PACKET.CZ.REQUEST_TIME2.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.clientTime, true);
	return pkt;
};

// 0x361
PACKET.CZ.CHANGE_DIRECTION2 = function PACKET_CZ_CHANGE_DIRECTION2() {
	this.headDir = 0;
	this.dir = 0;
};
PACKET.CZ.CHANGE_DIRECTION2.prototype.build = function () {
	const servDirection = [4, 3, 2, 1, 0, 7, 6, 5];

	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.headDir, true);
	pkt.view.setUint8(ver[4], servDirection[(this.dir + 8) % 8], true);
	return pkt;
};

// 0x362
PACKET.CZ.ITEM_PICKUP2 = function PACKET_CZ_ITEM_PICKUP2() {
	this.ITAID = 0;
};
PACKET.CZ.ITEM_PICKUP2.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.ITAID, true);
	return pkt;
};

// 0x363
PACKET.CZ.ITEM_THROW2 = function PACKET_CZ_ITEM_THROW2() {
	this.Index = 0;
	this.count = 0;
};
PACKET.CZ.ITEM_THROW2.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint16(ver[3], this.Index, true);
	pkt.view.setInt16(ver[4], this.count, true);
	return pkt;
};

// 0x364
PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_STORE2 = function PACKET_CZ_MOVE_ITEM_FROM_BODY_TO_STORE2() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MOVE_ITEM_FROM_BODY_TO_STORE2.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.index, true);
	pkt.view.setInt32(ver[4], this.count, true);
	return pkt;
};

// 0x365
PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_BODY2 = function PACKET_CZ_MOVE_ITEM_FROM_STORE_TO_BODY2() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.MOVE_ITEM_FROM_STORE_TO_BODY2.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(ver[3], this.index, true);
	pkt.view.setInt32(ver[4], this.count, true);
	return pkt;
};

// 0x366
PACKET.CZ.USE_SKILL_TOGROUND2 = function PACKET_CZ_USE_SKILL_TOGROUND2() {
	this.selectedLevel = 0;
	this.SKID = 0;
	this.xPos = 0;
	this.yPos = 0;
};
PACKET.CZ.USE_SKILL_TOGROUND2.prototype.build = function () {
	const pkt = new BinaryWriter(10);

	pkt.writeShort(0x0366);
	pkt.writeShort(this.selectedLevel, true);
	pkt.writeShort(this.SKID, true);
	pkt.writeShort(this.xPos, true);
	pkt.writeShort(this.yPos, true);
	return pkt;
};

// 0x368
PACKET.CZ.REQNAME2 = function PACKET_CZ_REQNAME2() {
	this.AID = 0;
};
PACKET.CZ.REQNAME2.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.AID, true);
	return pkt;
};

// 0x369
PACKET.CZ.REQNAME_BYGID2 = function PACKET_CZ_REQNAME_BYGID2() {
	this.GID = 0;
};
PACKET.CZ.REQNAME_BYGID2.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.GID, true);
	return pkt;
};

// 0x436
PACKET.CZ.ENTER2 = function PACKET_CZ_ENTER2() {
	this.AID = 0;
	this.GID = 0;
	this.AuthCode = 0;
	this.clientTime = 0;
	this.unknown = 0;
	this.Sex = 0;
};
PACKET.CZ.ENTER2.prototype.build = function () {
	let pkt_len = 2 + 4 + 4 + 4 + 4 + 1;
	if (PACKETVER.value >= 20210630) {
		pkt_len += 4;
	}
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x436);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeULong(this.AuthCode);
	pkt_buf.writeULong(this.clientTime);
	if (PACKETVER.value >= 20210630) {
		pkt_buf.writeULong(this.unknown);
	}
	pkt_buf.writeUChar(this.Sex);
	return pkt_buf;
};

// 0x437
PACKET.CZ.REQUEST_ACT2 = function PACKET_CZ_REQUEST_ACT2() {
	this.targetGID = 0;
	this.action = 0;
};
PACKET.CZ.REQUEST_ACT2.prototype.build = function () {
	const pkt_len = 2 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x437);
	pkt_buf.writeULong(this.targetGID);
	pkt_buf.writeUChar(this.action);
	return pkt_buf;
};

// 0x438
PACKET.CZ.USE_SKILL2 = function PACKET_CZ_USE_SKILL2() {
	this.selectedLevel = 0;
	this.SKID = 0;
	this.targetID = 0;
};
PACKET.CZ.USE_SKILL2.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x438);
	pkt_buf.writeShort(this.selectedLevel);
	pkt_buf.writeUShort(this.SKID);
	pkt_buf.writeULong(this.targetID);
	return pkt_buf;
};

// 0x439
PACKET.CZ.USE_ITEM2 = function PACKET_CZ_USE_ITEM2() {
	this.index = 0;
	this.AID = 0;
};
PACKET.CZ.USE_ITEM2.prototype.build = function () {
	const pkt_len = 2 + 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x439);
	pkt_buf.writeUShort(this.index);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x443
PACKET.CZ.SKILL_SELECT_RESPONSE = function PACKET_CZ_SKILL_SELECT_RESPONSE() {
	this.why = 0;
	this.SKID = 0;
};
PACKET.CZ.SKILL_SELECT_RESPONSE.prototype.build = function () {
	const pkt_len = 2 + 4 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x443);
	pkt_buf.writeLong(this.why);
	pkt_buf.writeUShort(this.SKID);
	return pkt_buf;
};

// 0x445
PACKET.CZ.SIMPLE_BUY_CASH_POINT_ITEM = function PACKET_CZ_SIMPLE_BUY_CASH_POINT_ITEM() {
	this.ITID = 0;
	this.count = 0;
};
PACKET.CZ.SIMPLE_BUY_CASH_POINT_ITEM.prototype.build = function () {
	const pkt_len = 2 + 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x445);
	pkt_buf.writeUShort(this.ITID);
	pkt_buf.writeShort(this.count);
	return pkt_buf;
};

// 0x447
PACKET.CZ.BLOCKING_PLAY_CANCEL = function PACKET_CZ_BLOCKING_PLAY_CANCEL() {};
PACKET.CZ.BLOCKING_PLAY_CANCEL.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x447);
	return pkt_buf;
};

// 0x44a
PACKET.CZ.CLIENT_VERSION = function PACKET_CZ_CLIENT_VERSION() {
	this.clientVer = 0;
};
PACKET.CZ.CLIENT_VERSION.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x44a);
	pkt_buf.writeLong(this.clientVer);
	return pkt_buf;
};

// 0x44b
PACKET.CZ.CLOSE_SIMPLECASH_SHOP = function PACKET_CZ_CLOSE_SIMPLECASH_SHOP() {};
PACKET.CZ.CLOSE_SIMPLECASH_SHOP.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x44b);
	return pkt_buf;
};

// 0x7d1
PACKET.CZ.ES_GET_LIST = function PACKET_CZ_ES_GET_LIST() {};
PACKET.CZ.ES_GET_LIST.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7d1);
	return pkt_buf;
};

// 0x7d3
PACKET.CZ.ES_CHOOSE = function PACKET_CZ_ES_CHOOSE() {
	this.esNo = 0;
};
PACKET.CZ.ES_CHOOSE.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7d3);
	pkt_buf.writeShort(this.esNo);
	return pkt_buf;
};

// 0x7d4
PACKET.CZ.ES_CANCEL = function PACKET_CZ_ES_CANCEL() {
	this.esNo = 0;
};
PACKET.CZ.ES_CANCEL.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7d4);
	pkt_buf.writeShort(this.esNo);
	return pkt_buf;
};

// 0x7d7
PACKET.CZ.GROUPINFO_CHANGE_V2 = function PACKET_CZ_GROUPINFO_CHANGE_V2() {
	this.expOption = 0;
	this.ItemPickupRule = 0;
	this.ItemDivisionRule = 0;
};
PACKET.CZ.GROUPINFO_CHANGE_V2.prototype.build = function () {
	const pkt_len = 2 + 4 + 1 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7d7);
	pkt_buf.writeULong(this.expOption);
	pkt_buf.writeUChar(this.ItemPickupRule);
	pkt_buf.writeUChar(this.ItemDivisionRule);
	return pkt_buf;
};

// 0x7da
PACKET.CZ.CHANGE_GROUP_MASTER = function PACKET_CZ_CHANGE_GROUP_MASTER() {
	this.AID = 0;
};
PACKET.CZ.CHANGE_GROUP_MASTER.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7da);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x7dc
PACKET.CZ.SEEK_PARTY = function PACKET_CZ_SEEK_PARTY() {
	this.Option = 0;
};
PACKET.CZ.SEEK_PARTY.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7dc);
	pkt_buf.writeULong(this.Option);
	return pkt_buf;
};

// 0x7de
PACKET.CZ.SEEK_PARTY_MEMBER = function PACKET_CZ_SEEK_PARTY_MEMBER() {
	this.Job = 0;
	this.Level = 0;
	this.mapName = '';
	this.Option = 0;
};
PACKET.CZ.SEEK_PARTY_MEMBER.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 16 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7de);
	pkt_buf.writeULong(this.Job);
	pkt_buf.writeULong(this.Level);
	pkt_buf.writeString(this.mapName, 16);
	pkt_buf.writeULong(this.Option);
	return pkt_buf;
};

// 0x7e4
PACKET.CZ.ITEMLISTWIN_RES = function PACKET_CZ_ITEMLISTWIN_RES() {
	this.Type = 0;
	this.Action = 0;
	this.MaterialList = [];
};
PACKET.CZ.ITEMLISTWIN_RES.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt_len = 2 + 2 + 4 + 4 + this.MaterialList.length * 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(ver[1]);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeULong(this.Type);
	pkt_buf.writeULong(this.Action);
	let i, count;

	for (i = 0, count = this.MaterialList.length; i < count; ++i) {
		pkt_buf.writeShort(this.MaterialList[i].index);
		pkt_buf.writeShort(this.MaterialList[i].count);
	}

	return pkt_buf;
};

// 0x7e5
PACKET.CH.ENTER_CHECKBOT = function PACKET_CH_ENTER_CHECKBOT() {
	this.dwAID = 0;
	this.szStringInfo = '';
};
PACKET.CH.ENTER_CHECKBOT.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + this.szStringInfo.length;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7e5);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeULong(this.dwAID);
	pkt_buf.writeString(this.szStringInfo);
	return pkt_buf;
};

// 0x7e7
PACKET.CH.CHECKBOT = function PACKET_CH_CHECKBOT() {
	this.dwAID = 0;
	this.szStringInfo = '';
};
PACKET.CH.CHECKBOT.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7e7);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeULong(this.dwAID);
	pkt_buf.writeString(this.szStringInfo, 24);
	return pkt_buf;
};

// 0x7ea
PACKET.CZ.BATTLE_FIELD_LIST = function PACKET_CZ_BATTLE_FIELD_LIST() {};
PACKET.CZ.BATTLE_FIELD_LIST.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7ea);
	return pkt_buf;
};

// 0x7ec
PACKET.CZ.JOIN_BATTLE_FIELD = function PACKET_CZ_JOIN_BATTLE_FIELD() {
	this.BFNO = 0;
	this.JoinTeam = 0;
};
PACKET.CZ.JOIN_BATTLE_FIELD.prototype.build = function () {
	const pkt_len = 2 + 4 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7ec);
	pkt_buf.writeULong(this.BFNO);
	pkt_buf.writeShort(this.JoinTeam);
	return pkt_buf;
};

// 0x7ee
PACKET.CZ.CANCEL_BATTLE_FIELD = function PACKET_CZ_CANCEL_BATTLE_FIELD() {
	this.BFNO = 0;
};
PACKET.CZ.CANCEL_BATTLE_FIELD.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7ee);
	pkt_buf.writeULong(this.BFNO);
	return pkt_buf;
};

// 0x7f0
PACKET.CZ.REQ_BATTLE_STATE_MONITOR = function PACKET_CZ_REQ_BATTLE_STATE_MONITOR() {
	this.BFNO = 0;
	this.PowerSwitch = 0;
};
PACKET.CZ.REQ_BATTLE_STATE_MONITOR.prototype.build = function () {
	const pkt_len = 2 + 4 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7f0);
	pkt_buf.writeULong(this.BFNO);
	pkt_buf.writeShort(this.PowerSwitch);
	return pkt_buf;
};

// 0x7f5
PACKET.CZ.GM_FULLSTRIP = function PACKET_CZ_GM_FULLSTRIP() {
	this.TargetAID = 0;
};
PACKET.CZ.GM_FULLSTRIP.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x7f5);
	pkt_buf.writeULong(this.TargetAID);
	return pkt_buf;
};

// 0x801
PACKET.CZ.PC_PURCHASE_ITEMLIST_FROMMC2 = function PACKET_CZ_PC_PURCHASE_ITEMLIST_FROMMC2() {
	this.AID = 0;
	this.UniqueID = 0;
	this.itemList = [];
};
PACKET.CZ.PC_PURCHASE_ITEMLIST_FROMMC2.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + 4 + this.itemList.length * 4;
	const pkt_buf = new BinaryWriter(pkt_len);
	let i, count;

	pkt_buf.writeShort(0x801);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeULong(this.UniqueID);

	for (i = 0, count = this.itemList.length; i < count; ++i) {
		pkt_buf.writeShort(this.itemList[i].count);
		pkt_buf.writeShort(this.itemList[i].index);
	}
	return pkt_buf;
};

// 0x802
PACKET.CZ.PARTY_BOOKING_REQ_REGISTER = function PACKET_CZ_PARTY_BOOKING_REQ_REGISTER() {
	this.RegisterInfo = {};
};
PACKET.CZ.PARTY_BOOKING_REQ_REGISTER.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);

	if (ver[2] === 18) {
		pkt.writeShort(this.RegisterInfo.Level);
		pkt.writeShort(this.RegisterInfo.MapID);

		for (let i = 0; i < 6; ++i) {
			pkt.writeShort(this.RegisterInfo.Job[i]);
		}
	} else {
		pkt.writeShort(this.RegisterInfo.Level);
		pkt.writeString(this.RegisterInfo.Notice, 37);
	}
	return pkt;
};

// 0x804
PACKET.CZ.PARTY_BOOKING_REQ_SEARCH = function PACKET_CZ_PARTY_BOOKING_REQ_SEARCH() {
	this.Level = 0;
	this.MapID = 0;
	this.Job = 0;
	this.LastIndex = 0;
	this.ResultCount = 0;
};
PACKET.CZ.PARTY_BOOKING_REQ_SEARCH.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.writeShort(this.Level);
	pkt.writeShort(this.MapID);

	if (ver[2] > 12) {
		pkt.writeShort(this.Job);
	}

	pkt.writeULong(this.LastIndex);
	pkt.writeShort(this.ResultCount);
	return pkt;
};

// 0x806
PACKET.CZ.PARTY_BOOKING_REQ_DELETE = function PACKET_CZ_PARTY_BOOKING_REQ_DELETE() {};
PACKET.CZ.PARTY_BOOKING_REQ_DELETE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	return pkt;
};

// 0x808
PACKET.CZ.PARTY_BOOKING_REQ_UPDATE = function PACKET_CZ_PARTY_BOOKING_REQ_UPDATE() {
	this.Job = 0;
};
PACKET.CZ.PARTY_BOOKING_REQ_UPDATE.prototype.build = function () {
	const pkt_len = 2 + 12;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x808);
	pkt_buf.writeShort(this.Job[0]);
	pkt_buf.writeShort(this.Job[1]);
	return pkt_buf;
};

// 0x80c
PACKET.CZ.SIMPLE_CASH_BTNSHOW = function PACKET_CZ_SIMPLE_CASH_BTNSHOW() {};
PACKET.CZ.SIMPLE_CASH_BTNSHOW.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x80c);
	return pkt_buf;
};

// 0x811
PACKET.CZ.REQ_OPEN_BUYING_STORE = function PACKET_CZ_REQ_OPEN_BUYING_STORE() {
	this.LimitZeny = 0;
	this.result = 0;
	this.storeName = '';
	this.storeList = [];
};
PACKET.CZ.REQ_OPEN_BUYING_STORE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const item_len = PACKETVER.value >= 20181121 ? 10 : 8;
	const pkt_len = 2 + 2 + 4 + 1 + 80 + this.storeList.length * item_len;
	const pkt = new BinaryWriter(pkt_len);
	let i, count;

	pkt.writeShort(ver[1]); // header
	pkt.writeShort(pkt_len); // len -1
	pkt.writeULong(this.LimitZeny);
	pkt.writeUChar(this.result);
	pkt.writeString(this.storeName, 80);

	for (i = 0, count = this.storeList.length; i < count; ++i) {
		if (PACKETVER.value >= 20181121) {
			pkt.writeULong(this.storeList[i].ITID);
		} else {
			pkt.writeUShort(this.storeList[i].ITID);
		}
		pkt.writeShort(this.storeList[i].count);
		pkt.writeLong(this.storeList[i].price);
	}

	return pkt;
};

// 0x815
PACKET.CZ.REQ_CLOSE_BUYING_STORE = function PACKET_CZ_REQ_CLOSE_BUYING_STORE() {};
PACKET.CZ.REQ_CLOSE_BUYING_STORE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	return pkt;
};

// 0x817
PACKET.CZ.REQ_CLICK_TO_BUYING_STORE = function PACKET_CZ_REQ_CLICK_TO_BUYING_STORE() {
	this.makerAID = 0;
};
PACKET.CZ.REQ_CLICK_TO_BUYING_STORE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.makerAID, true);
	return pkt;
};

// 0x819
PACKET.CZ.REQ_TRADE_BUYING_STORE = function PACKET_CZ_REQ_TRADE_BUYING_STORE() {
	this.AID = 0;
	this.UniqueID = 0;
	this.itemList = [];
};
PACKET.CZ.REQ_TRADE_BUYING_STORE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const len = 2 + 2 + 4 + 4 + this.itemList.length * 6; // ver[2] = -1
	const pkt = new BinaryWriter(len);
	let i, count;

	pkt.writeShort(ver[1]);
	pkt.writeShort(len);
	pkt.writeULong(this.AID);
	pkt.writeULong(this.UniqueID);

	for (i = 0, count = this.itemList.length; i < count; ++i) {
		pkt.writeUShort(this.itemList[i].index);
		pkt.writeUShort(this.itemList[i].ITID);
		pkt.writeShort(this.itemList[i].count);
	}

	return pkt;
};

//0xb6e
PACKET.ZC.SE_CASHSHOP_OPEN = function PACKET_ZC_SE_CASHSHOP_OPEN(fp, end) {
	this.cashPoints = fp.readULong();
	this.kafraPoints = fp.readULong();
	this.tab = fp.readULong();
};
PACKET.ZC.SE_CASHSHOP_OPEN.size = 14;

//0x0a2b
PACKET.ZC.SE_CASHSHOP_OPEN2 = function PACKET_ZC_SE_CASHSHOP_OPEN2(fp, end) {
	this.cashPoints = fp.readULong();
	this.kafraPoints = fp.readULong();
	this.tab = fp.readULong();
};
PACKET.ZC.SE_CASHSHOP_OPEN2.size = 14;

//0x0845
PACKET.ZC.SE_CASHSHOP_OPEN3 = function PACKET_ZC_SE_CASHSHOP_OPEN3(fp, end) {
	this.cashPoints = fp.readULong();
	this.kafraPoints = fp.readULong();
};
PACKET.ZC.SE_CASHSHOP_OPEN3.size = 10;

//0x08ca
PACKET.ZC.ACK_SCHEDULER_CASHITEM = function PACKET_ZC_ACK_SCHEDULER_CASHITEM(fp, end) {
	this.count = fp.readUShort();
	this.tabNum = fp.readUShort();
	this.items = (function () {
		const out = [];
		let divider = 6; // original
		if (PACKETVER.value >= 20181121 || PACKETVER.value >= 20180704 || PACKETVER.value >= 20181114) {
			divider = 8;
		}
		const cnt = (end - fp.tell()) / divider;
		for (let i = 0; i < cnt; ++i) {
			out[i] = {};
			if (PACKETVER.value >= 20181121 || PACKETVER.value >= 20180704 || PACKETVER.value >= 20181114) {
				out[i].itemId = fp.readULong();
			} else {
				out[i].itemId = fp.readUShort();
			}
			out[i].price = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.ACK_SCHEDULER_CASHITEM.size = -1;

// 0x8cb
// <packet len>.W <exp>.L <death>.L <drop>.L <DETAIL_EXP_INFO>13B
// (ZC_PERSONAL_INFORMATION) <InfoType>.B <Exp>.L <Death>.L <Drop>.L (DETAIL_EXP_INFO)
PACKET.ZC.PERSONAL_INFORMATION = function PACKET_ZC_PERSONAL_INFORMATION(fp, end) {
	this.total_exp = fp.readShort();
	this.total_death = fp.readShort();
	this.total_drop = fp.readShort();

	this.info = (function () {
		const count = ((end - fp.tell()) / 7) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].type = fp.readChar(); // ?
			out[i].exp = fp.readShort();
			out[i].death = fp.readShort();
			out[i].drop = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.PERSONAL_INFORMATION.size = -1;

//0x08c0
PACKET.ZC.ACK_SE_CASH_ITEM_LIST2 = function PACKET_ZC_ACK_SE_CASH_ITEM_LIST2(fp, end) {
	this.len = fp.readULong();
	this.openIdentity = fp.readULong();
	this.itemcount = fp.readUShort();
};
PACKET.ZC.ACK_SE_CASH_ITEM_LIST2.size = 8;

//0x846
PACKET.CZ.REQ_SE_CASH_TAB_CODE = function PACKET_CZ_REQ_SE_CASH_TAB_CODE() {
	this.tabid = 0;
};
PACKET.CZ.REQ_SE_CASH_TAB_CODE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(4);

	pkt_buf.writeShort(0x846);
	pkt_buf.setUint16(2, this.tabid, true);
	return pkt_buf;
};

//0x0844
PACKET.CZ.SE_CASHSHOP_OPEN1 = function PACKET_CZ_SE_CASHSHOP_OPEN1() {};
PACKET.CZ.SE_CASHSHOP_OPEN1.prototype.build = function () {
	// var ver = this.getPacketVersion();
	// var pkt_buf = new BinaryWriter(ver[2]);
	const pkt_buf = new BinaryWriter(2);

	pkt_buf.writeShort(0x0844);
	return pkt_buf;
};

//0x0b6d
PACKET.CZ.SE_CASHSHOP_OPEN2 = function PACKET_CZ_SE_CASHSHOP_OPEN2() {
	this.tab = 0;
};
PACKET.CZ.SE_CASHSHOP_OPEN2.prototype.build = function () {
	const pkt_buf = new BinaryWriter(6);

	pkt_buf.writeShort(0xb6d);
	pkt_buf.writeULong(this.tab);
	return pkt_buf;
};

//0x08c9
PACKET.CZ.PC_CASH_POINT_ITEMLIST = function PACKET_CZ_PC_CASH_POINT_ITEMLIST() {};
PACKET.CZ.PC_CASH_POINT_ITEMLIST.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);

	pkt_buf.writeShort(0x08c9);
	return pkt_buf;
};

//0x084a
PACKET.CZ.CASH_SHOP_CLOSE = function PACKET_CZ_CASH_SHOP_CLOSE() {};
PACKET.CZ.CASH_SHOP_CLOSE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);

	pkt_buf.writeShort(0x84a);
	return pkt_buf;
};

// 0x822
PACKET.CA.OTP_AUTH_REQ = function PACKET_CA_OTP_AUTH_REQ() {
	this.OTPCode = '';
};
PACKET.CA.OTP_AUTH_REQ.prototype.build = function () {
	const pkt_len = 2 + 7;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x822);
	pkt_buf.writeString(this.OTPCode, 7);
	return pkt_buf;
};

//0848 <packet len>.W <count>.W <packet len>.W <kafra points>.L <count>.W { <amount>.W <name id>.W <tab>.W }.6B*count
PACKET.CZ.SE_PC_BUY_CASHITEM_LIST = function PACKET_CZ_SE_PC_BUY_CASHITEM_LIST() {
	this.kafraPoints = 0;
	this.item_list = [];
};
PACKET.CZ.SE_PC_BUY_CASHITEM_LIST.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 4 + this.item_list.length * 10;
	const pkt = new BinaryWriter(pkt_len);
	let i, count;

	pkt.writeShort(0x848);
	pkt.writeShort(pkt_len);
	pkt.writeUShort(this.item_list.length);
	pkt.writeULong(this.kafraPoints);

	for (i = 0, count = this.item_list.length; i < count; ++i) {
		pkt.writeULong(this.item_list[i].itemId);
		pkt.writeULong(this.item_list[i].amount);
		pkt.writeShort(this.item_list[i].tab);
	}

	return pkt;
};

//0x0849
PACKET.ZC.SE_PC_BUY_CASHITEM_RESULT = function PACKET_ZC_SE_PC_BUY_CASHITEM_RESULT(fp, end) {
	this.kafraPoints = fp.readUShort();
	this.itemId = fp.readShort();
	this.result = fp.readShort();
	this.cashPoints = fp.readUShort();
};
PACKET.ZC.SE_PC_BUY_CASHITEM_RESULT.size = 16;

// 0x825a
PACKET.CA.SSO_LOGIN_REQa = function PACKET_CA_SSO_LOGIN_REQa() {
	this.Version = 0;
	this.clienttype = 0;
	this.ID = '';
	this.MacAddr = '';
	this.IpAddr = '';
	this.t1 = '';
};
PACKET.CA.SSO_LOGIN_REQa.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + 1 + 24 + 17 + 15 + this.t1.length;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x825a);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeULong(this.Version);
	pkt_buf.writeUChar(this.clienttype);
	pkt_buf.writeString(this.ID, 24);
	pkt_buf.writeBinaryString(this.MacAddr, 17);
	pkt_buf.writeBinaryString(this.IpAddr, 15);
	pkt_buf.writeBinaryString(this.t1);
	return pkt_buf;
};

// 0x825
PACKET.CA.SSO_LOGIN_REQ = function PACKET_CA_SSO_LOGIN_REQ() {
	this.Version = 0;
	this.clienttype = 0;
	this.ID = '';
	this.Passwd = '';
	this.MacAdress = '';
	this.IP = '';
	this.t1 = '';
};
PACKET.CA.SSO_LOGIN_REQ.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + 1 + 24 + 27 + 17 + 15 + this.t1.length;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x825);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeULong(this.Version);
	pkt_buf.writeUChar(this.clienttype);
	pkt_buf.writeString(this.ID, 24);
	pkt_buf.writeString(this.Passwd, 27);
	pkt_buf.writeBinaryString(this.MacAdress, 17);
	pkt_buf.writeBinaryString(this.IP, 15);
	pkt_buf.writeBinaryString(this.t1);
	return pkt_buf;
};

// 0x827
PACKET.CH.DELETE_CHAR3_RESERVED = function PACKET_CH_DELETE_CHAR3_RESERVED() {
	this.GID = 0;
};
PACKET.CH.DELETE_CHAR3_RESERVED.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x827);
	pkt_buf.writeULong(this.GID);
	return pkt_buf;
};

// 0x829
PACKET.CH.DELETE_CHAR3 = function PACKET_CH_DELETE_CHAR3() {
	this.GID = 0;
	this.Birth = '';
};
PACKET.CH.DELETE_CHAR3.prototype.build = function () {
	const pkt_len = 2 + 4 + 6;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x829);
	pkt_buf.writeULong(this.GID);
	pkt_buf.writeString(this.Birth, 6);
	return pkt_buf;
};

// 0x82b
PACKET.CH.DELETE_CHAR3_CANCEL = function PACKET_CH_DELETE_CHAR3_CANCEL() {
	this.GID = 0;
};
PACKET.CH.DELETE_CHAR3_CANCEL.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x82b);
	pkt_buf.writeULong(this.GID);
	return pkt_buf;
};

// 0x835
/// 0835 <packet len>.W <type>.B <max price>.L <min price>.L <name id count>.B <card count>.B { <name id>.W }* { <card>.W }*
PACKET.CZ.SEARCH_STORE_INFO = function PACKET_CZ_SEARCH_STORE_INFO() {
	this.StoreType = 0;
	this.maxPrice = 0;
	this.minPrice = 0;
	this.ItemIDList = 0;
	this.CardIDList = 0;
};
PACKET.CZ.SEARCH_STORE_INFO.prototype.build = function () {
	let i, count, offset;
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setInt16(
		ver[3],
		2 + 2 + 1 + 4 + 4 + 1 + 1 + this.ItemIDList.length * 2 + this.CardIDList.length * 2,
		true
	);
	pkt.view.setUint8(ver[4], this.StoreType, true);
	pkt.view.setUint32(ver[5], this.maxPrice, true);
	pkt.view.setUint32(ver[6], this.minPrice, true);
	pkt.view.setUint8(ver[7], this.ItemIDList.length, true);
	pkt.view.setUint8(ver[8], this.CardIDList.length, true);

	offset = ver[9];

	for (i = 0, count = this.ItemIDList.length; i < count; ++i) {
		pkt.view.setUint16(offset, this.ItemIDList[i], true);
		offset += 2;
	}

	for (i = 0, count = this.CardIDList.length; i < count; ++i) {
		pkt.view.setUint16(offset, this.CardIDList[i], true);
		offset += 2;
	}

	return pkt;
};

// 0x838
PACKET.CZ.SEARCH_STORE_INFO_NEXT_PAGE = function PACKET_CZ_SEARCH_STORE_INFO_NEXT_PAGE() {};
PACKET.CZ.SEARCH_STORE_INFO_NEXT_PAGE.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	return pkt;
};

// 0x83b
PACKET.CZ.CLOSE_SEARCH_STORE_INFO = function PACKET_CZ_CLOSE_SEARCH_STORE_INFO() {};
PACKET.CZ.CLOSE_SEARCH_STORE_INFO.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	return pkt;
};

// 0x83c
PACKET.CZ.SSILIST_ITEM_CLICK = function PACKET_CZ_SSILIST_ITEM_CLICK() {
	this.AID = 0;
	this.SSI_ID = 0;
	this.ITID = 0;
};
PACKET.CZ.SSILIST_ITEM_CLICK.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.AID, true);
	pkt.view.setULong(ver[4], this.SSI_ID, true);
	pkt.view.setUShort(ver[5], this.ITID, true);

	return pkt;
};

// 0x841
PACKET.CH.SELECT_ACCESSIBLE_MAPNAME = function PACKET_CH_SELECT_ACCESSIBLE_MAPNAME() {
	this.CharNum = 0;
	this.mapListNum = 0;
};
PACKET.CH.SELECT_ACCESSIBLE_MAPNAME.prototype.build = function () {
	const pkt_len = 2 + 1 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x841);
	pkt_buf.writeUChar(this.CharNum);
	pkt_buf.writeUChar(this.mapListNum);
	return pkt_buf;
};

// 0x843
PACKET.CZ.REMOVE_AID_SSO = function PACKET_CZ_REMOVE_AID_SSO() {
	this.AID = '';
};
PACKET.CZ.REMOVE_AID_SSO.prototype.build = function () {
	const ver = this.getPacketVersion();
	const pkt = new BinaryWriter(ver[2]);

	pkt.writeShort(ver[1]);
	pkt.view.setUint32(ver[3], this.AID, true);
};

// 0x8b8
PACKET.CH.PINCODE_CHECK = function PACKET_CH_PINCODE_CHECK() {
	this.AID = '';
	this.PINCODE = '';
};
PACKET.CH.PINCODE_CHECK.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x8b8);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeString(this.PINCODE, 4);
	return pkt_buf;
};

// 0x8be
PACKET.CH.PINCODE_CHANGE = function PACKET_CH_PINCODE_CHANGE() {
	this.AID = '';
	this.OLD_PINCODE = '';
	this.NEW_PINCODE = '';
};
PACKET.CH.PINCODE_CHANGE.prototype.build = function () {
	const pkt_len = 2 + 4 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x8be);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeString(this.OLD_PINCODE, 4);
	pkt_buf.writeString(this.NEW_PINCODE, 4);
	return pkt_buf;
};

// 0x8ba
PACKET.CH.PINCODE_FIRST_PIN = function PACKET_CH_PINCODE_FIRST_PIN() {
	this.AID = '';
	this.PINCODE = '';
};
PACKET.CH.PINCODE_FIRST_PIN.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x8ba);
	pkt_buf.writeULong(this.AID);
	pkt_buf.writeString(this.PINCODE, 4);
	return pkt_buf;
};

// 0x8c5
PACKET.CH.PINCODE_REQUEST = function PACKET_CH_PINCODE_REQUEST() {
	this.AID = '';
};
PACKET.CH.PINCODE_REQUEST.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x8c5);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0x970
PACKET.CH.MAKE_CHAR2 = function PACKET_CH_MAKE_CHAR2() {
	this.name = '';
	this.CharNum = 0;
	this.headPal = 0;
	this.head = 0;
};
PACKET.CH.MAKE_CHAR2.prototype.build = function () {
	const pkt_len = 2 + 24 + 1 + 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x970);
	pkt_buf.writeString(this.name, 24);
	pkt_buf.writeUChar(this.CharNum);
	pkt_buf.writeShort(this.headPal);
	pkt_buf.writeShort(this.head);
	return pkt_buf;
};

// 0x69
PACKET.AC.ACCEPT_LOGIN = function PACKET_AC_ACCEPT_LOGIN(fp, end) {
	this.AuthCode = fp.readLong();
	this.AID = fp.readULong();
	this.userLevel = fp.readULong();
	this.lastLoginIP = fp.readULong();
	this.lastLoginTime = fp.readBinaryString(26);
	this.Sex = fp.readUChar();
	this.ServerList = (function () {
		const count = ((end - fp.tell()) / 32) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].ip = fp.readULong();
			out[i].port = fp.readUShort();
			out[i].name = fp.readString(20);
			out[i].usercount = fp.readUShort();
			out[i].state = fp.readUShort();
			out[i].property = fp.readUShort();
		}
		return out;
	})();
};
PACKET.AC.ACCEPT_LOGIN.size = -1;

// 0x6a
PACKET.AC.REFUSE_LOGIN = function PACKET_AC_REFUSE_LOGIN(fp, end) {
	this.ErrorCode = fp.readUChar();
	this.blockDate = fp.readBinaryString(20);
};
PACKET.AC.REFUSE_LOGIN.size = 23;

// 0x6b
PACKET.HC.ACCEPT_ENTER_NEO_UNION = function PACKET_HC_ACCEPT_ENTER_NEO_UNION(fp, end) {
	if (PACKETVER.value >= 20100413) {
		this.TotalSlotNum = fp.readUChar();
		this.PremiumStartSlot = fp.readUChar();
		this.PremiumEndSlot = fp.readUChar();
	}
	this.dummy1_beginbilling = fp.readChar();
	this.code = fp.readULong();
	this.time1 = fp.readULong();
	this.time2 = fp.readULong();
	this.dummy2_endbilling = fp.readBinaryString(7);
	this.charInfo = PACKETVER.parseCharInfo(fp, end);
};
PACKET.HC.ACCEPT_ENTER_NEO_UNION.size = -1;

// 0x6c
PACKET.HC.REFUSE_ENTER = function PACKET_HC_REFUSE_ENTER(fp, end) {
	this.ErrorCode = fp.readUChar();
};
PACKET.HC.REFUSE_ENTER.size = 3;

// 0x6d
PACKET.HC.ACCEPT_MAKECHAR_NEO_UNION = function PACKET_HC_ACCEPT_MAKECHAR_NEO_UNION(fp, end) {
	this.charinfo = PACKETVER.parseCharInfo(fp, end)[0];
};
PACKET.HC.ACCEPT_MAKECHAR_NEO_UNION.size = 0;

// 0x6e
PACKET.HC.REFUSE_MAKECHAR = function PACKET_HC_REFUSE_MAKECHAR(fp, end) {
	this.ErrorCode = fp.readUChar();
};
PACKET.HC.REFUSE_MAKECHAR.size = 3;

// 0x6f
PACKET.HC.ACCEPT_DELETECHAR = function PACKET_HC_ACCEPT_DELETECHAR(fp, end) {};
PACKET.HC.ACCEPT_DELETECHAR.size = 2;

// 0x70
PACKET.HC.REFUSE_DELETECHAR = function PACKET_HC_REFUSE_DELETECHAR(fp, end) {
	this.ErrorCode = fp.readUChar();
};
PACKET.HC.REFUSE_DELETECHAR.size = 3;

// 0x71
PACKET.HC.NOTIFY_ZONESVR = function PACKET_HC_NOTIFY_ZONESVR(fp, end) {
	this.GID = fp.readULong();
	this.mapName = fp.readBinaryString(16);
	this.addr = {};
	this.addr.ip = fp.readULong();
	this.addr.port = fp.readUShort();
};
PACKET.HC.NOTIFY_ZONESVR.size = 28;

// 0x73
PACKET.ZC.ACCEPT_ENTER = function PACKET_ZC_ACCEPT_ENTER(fp, end) {
	this.startTime = fp.readULong();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
};
PACKET.ZC.ACCEPT_ENTER.size = 11;

// 0x74
PACKET.ZC.REFUSE_ENTER = function PACKET_ZC_REFUSE_ENTER(fp, end) {
	this.ErrorCode = fp.readUChar();
};
PACKET.ZC.REFUSE_ENTER.size = 3;

// 0x75
PACKET.ZC.NOTIFY_INITCHAR = function PACKET_ZC_NOTIFY_INITCHAR(fp, end) {
	this.GID = fp.readULong();
	this.Style = fp.readShort();
	this.Item = fp.readUChar();
};
PACKET.ZC.NOTIFY_INITCHAR.size = 11;

// 0x76
PACKET.ZC.NOTIFY_UPDATECHAR = function PACKET_ZC_NOTIFY_UPDATECHAR(fp, end) {
	this.GID = fp.readULong();
	this.Style = fp.readShort();
	this.Item = fp.readUChar();
};
PACKET.ZC.NOTIFY_UPDATECHAR.size = 9;

// 0x77
PACKET.ZC.NOTIFY_UPDATEPLAYER = function PACKET_ZC_NOTIFY_UPDATEPLAYER(fp, end) {
	this.Style = fp.readShort();
	this.Item = fp.readUChar();
};
PACKET.ZC.NOTIFY_UPDATEPLAYER.size = 5;

// 0x78
PACKET.ZC.NOTIFY_STANDENTRY = function PACKET_ZC_NOTIFY_STANDENTRY(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readShort();
	this.accessory = fp.readShort();
	this.shield = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readShort();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_STANDENTRY.size = 55;

// 0x79
PACKET.ZC.NOTIFY_NEWENTRY = function PACKET_ZC_NOTIFY_NEWENTRY(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readShort();
	this.accessory = fp.readShort();
	this.shield = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readShort();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_NEWENTRY.size = 53;

// 0x7a
PACKET.ZC.NOTIFY_ACTENTRY = function PACKET_ZC_NOTIFY_ACTENTRY(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readShort();
	this.accessory = fp.readShort();
	this.shield = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readShort();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.action = fp.readUChar();
	this.actStartTime = fp.readULong();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_ACTENTRY.size = 58;

// 0x7b
PACKET.ZC.NOTIFY_MOVEENTRY = function PACKET_ZC_NOTIFY_MOVEENTRY(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readShort();
	this.accessory = fp.readShort();
	this.moveStartTime = fp.readULong();
	this.shield = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readShort();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_MOVEENTRY.size = 60;

// 0x7c
PACKET.ZC.NOTIFY_STANDENTRY_NPC = function PACKET_ZC_NOTIFY_STANDENTRY_NPC(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readShort();
	this.accessory = fp.readShort();
	this.job = fp.readShort();
	this.shield = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
};
PACKET.ZC.NOTIFY_STANDENTRY_NPC.size = 42;

// 0x7f
PACKET.ZC.NOTIFY_TIME = function PACKET_ZC_NOTIFY_TIME(fp, end) {
	this.time = fp.readULong();
};
PACKET.ZC.NOTIFY_TIME.size = 6;

// 0x80
PACKET.ZC.NOTIFY_VANISH = function PACKET_ZC_NOTIFY_VANISH(fp, end) {
	this.GID = fp.readULong();
	this.type = fp.readUChar();
};
PACKET.ZC.NOTIFY_VANISH.size = 7;

// 0x81
PACKET.SC.NOTIFY_BAN = function PACKET_SC_NOTIFY_BAN(fp, end) {
	this.ErrorCode = fp.readUChar();
};
PACKET.SC.NOTIFY_BAN.size = 3;

// 0x83
PACKET.ZC.ACCEPT_QUIT = function PACKET_ZC_ACCEPT_QUIT(fp, end) {};
PACKET.ZC.ACCEPT_QUIT.size = 2;

// 0x84
PACKET.ZC.REFUSE_QUIT = function PACKET_ZC_REFUSE_QUIT(fp, end) {};
PACKET.ZC.REFUSE_QUIT.size = 2;

// 0x86
PACKET.ZC.NOTIFY_MOVE = function PACKET_ZC_NOTIFY_MOVE(fp, end) {
	this.GID = fp.readULong();
	this.MoveData = fp.readPos2();
	this.moveStartTime = fp.readULong();
};
PACKET.ZC.NOTIFY_MOVE.size = 16;

// 0x87
PACKET.ZC.NOTIFY_PLAYERMOVE = function PACKET_ZC_NOTIFY_PLAYERMOVE(fp, end) {
	this.moveStartTime = fp.readULong();
	this.MoveData = fp.readPos2();
};
PACKET.ZC.NOTIFY_PLAYERMOVE.size = 12;

// 0x88
PACKET.ZC.STOPMOVE = function PACKET_ZC_STOPMOVE(fp, end) {
	this.AID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
};
PACKET.ZC.STOPMOVE.size = 10;

// 0x8a
PACKET.ZC.NOTIFY_ACT = function PACKET_ZC_NOTIFY_ACT(fp, end) {
	this.GID = fp.readULong();
	this.targetGID = fp.readULong();
	this.startTime = fp.readULong();
	this.attackMT = fp.readLong();
	this.attackedMT = fp.readLong();
	this.damage = fp.readShort();
	this.count = fp.readShort();
	this.action = fp.readUChar();
	this.leftDamage = fp.readShort();
};
PACKET.ZC.NOTIFY_ACT.size = 29;

// 0x8b
PACKET.ZC.NOTIFY_ACT_POSITION = function PACKET_ZC_NOTIFY_ACT_POSITION(fp, end) {
	this.GID = fp.readULong();
	this.targetGID = fp.readULong();
	this.startTime = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.damage = fp.readShort();
	this.count = fp.readShort();
	this.action = fp.readUChar();
};
PACKET.ZC.NOTIFY_ACT_POSITION.size = 23;

// 0x8d
PACKET.ZC.NOTIFY_CHAT = function PACKET_ZC_NOTIFY_CHAT(fp, end) {
	this.GID = fp.readULong();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_CHAT.size = -1;

// 0x8e
PACKET.ZC.NOTIFY_PLAYERCHAT = function PACKET_ZC_NOTIFY_PLAYERCHAT(fp, end) {
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_PLAYERCHAT.size = -1;

// 0x8f
PACKET.ZC.ENTRY_ACK = function PACKET_ZC_ENTRY_ACK(fp, end) {
	this.Header = fp.readShort();
	this.AID = fp.readLong();
};
PACKET.ZC.ENTRY_ACK.size = 6;

// 0x91
PACKET.ZC.NPCACK_MAPMOVE = function PACKET_ZC_NPCACK_MAPMOVE(fp, end) {
	this.mapName = fp.readBinaryString(16);
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
};
PACKET.ZC.NPCACK_MAPMOVE.size = 22;

// 0x92
PACKET.ZC.NPCACK_SERVERMOVE = function PACKET_ZC_NPCACK_SERVERMOVE(fp, end) {
	this.mapName = fp.readBinaryString(16);
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.addr = {};
	this.addr.ip = fp.readULong();
	this.addr.port = fp.readUShort();
};
PACKET.ZC.NPCACK_SERVERMOVE.size = 28;

// 0x93
PACKET.ZC.NPCACK_ENABLE = function PACKET_ZC_NPCACK_ENABLE(fp, end) {};
PACKET.ZC.NPCACK_ENABLE.size = 2;

// 0x95
PACKET.ZC.ACK_REQNAME = function PACKET_ZC_ACK_REQNAME(fp, end) {
	this.AID = fp.readULong();
	this.CName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ACK_REQNAME.size = 30;

// 0x96
PACKET.ZC.UNK1 = function PACKET_ZC_UNK1(fp, end) {
	this.unk = fp.readUShort();
};
PACKET.ZC.UNK1.size = 4;

// 0x97
PACKET.ZC.WHISPER = function PACKET_ZC_WHISPER(fp, end) {
	this.sender = fp.readString(NAME_LENGTH);
	this.isAdmin = fp.readLong();
	if (this.isAdmin !== 0 && this.isAdmin !== 1) {
		fp.seek(-4, SEEK_CUR);
	}
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.WHISPER.size = -1;

// 0x9de
PACKET.ZC.WHISPER2 = function PACKET_ZC_WHISPER2(fp, end) {
	this.senderGID = fp.readULong();
	this.sender = fp.readString(NAME_LENGTH);
	this.isAdmin = fp.readUByte();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.WHISPER2.size = -1;

// 0x98
PACKET.ZC.ACK_WHISPER = function PACKET_ZC_ACK_WHISPER(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_WHISPER.size = 3;

// 0x9a
PACKET.ZC.BROADCAST = function PACKET_ZC_BROADCAST(fp, end) {
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.BROADCAST.size = -1;

// 0x9c
PACKET.ZC.CHANGE_DIRECTION = function PACKET_ZC_CHANGE_DIRECTION(fp, end) {
	this.AID = fp.readULong();
	this.headDir = fp.readShort();
	this.dir = fp.readUChar();
};
PACKET.ZC.CHANGE_DIRECTION.size = 9;

// 0x9d
PACKET.ZC.ITEM_ENTRY = function PACKET_ZC_ITEM_ENTRY(fp, end) {
	this.ITAID = fp.readULong();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.count = fp.readShort();
	this.subX = fp.readUChar();
	this.subY = fp.readUChar();
};
PACKET.ZC.ITEM_ENTRY.size = PACKETVER.value >= 20181121 ? 19 : 17;

// 0x9e
PACKET.ZC.ITEM_FALL_ENTRY = function PACKET_ZC_ITEM_FALL_ENTRY(fp, end) {
	this.ITAID = fp.readULong();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.subX = fp.readUChar();
	this.subY = fp.readUChar();
	this.count = fp.readShort();
};
PACKET.ZC.ITEM_FALL_ENTRY.size = PACKETVER.value >= 20181121 ? 19 : 17;

// 0xa0
PACKET.ZC.ITEM_PICKUP_ACK = function PACKET_ZC_ITEM_PICKUP_ACK(fp, end) {
	this.index = fp.readUShort();
	this.count = fp.readUShort();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
	this.location = fp.readUShort();
	this.type = fp.readUChar();
	this.result = fp.readUChar();
};
PACKET.ZC.ITEM_PICKUP_ACK.size = 23;

// 0xa1
PACKET.ZC.ITEM_DISAPPEAR = function PACKET_ZC_ITEM_DISAPPEAR(fp, end) {
	this.ITAID = fp.readULong();
};
PACKET.ZC.ITEM_DISAPPEAR.size = 6;

// 0xa3
PACKET.ZC.NORMAL_ITEMLIST = function PACKET_ZC_NORMAL_ITEMLIST(fp, end) {
	this.itemInfo = (function () {
		const count = ((end - fp.tell()) / 10) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.NORMAL_ITEMLIST.size = -1;

// 0xa4
PACKET.ZC.EQUIPMENT_ITEMLIST = function PACKET_ZC_EQUIPMENT_ITEMLIST(fp, end) {
	this.itemInfo = (function () {
		const count = ((end - fp.tell()) / 20) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.EQUIPMENT_ITEMLIST.size = -1;

// 0xa5
PACKET.ZC.STORE_NORMAL_ITEMLIST = function PACKET_ZC_STORE_NORMAL_ITEMLIST(fp, end) {
	this.itemInfo = (function () {
		const count = ((end - fp.tell()) / 10) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.STORE_NORMAL_ITEMLIST.size = -1;

// 0xa6
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST = function PACKET_ZC_STORE_EQUIPMENT_ITEMLIST(fp, end) {
	this.itemInfo = (function () {
		const count = ((end - fp.tell()) / 20) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST.size = -1;

// 0xa8
PACKET.ZC.USE_ITEM_ACK = function PACKET_ZC_USE_ITEM_ACK(fp, end) {
	this.index = fp.readUShort();
	this.count = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.USE_ITEM_ACK.size = 7;

// 0xaa
PACKET.ZC.REQ_WEAR_EQUIP_ACK = function PACKET_ZC_REQ_WEAR_EQUIP_ACK(fp, end) {
	this.index = fp.readUShort();
	this.wearLocation = fp.readUShort();
	if (PACKETVER.value >= 20100629) {
		this.viewid = fp.readUShort();
	}
	this.result = fp.readUChar();
};
PACKET.ZC.REQ_WEAR_EQUIP_ACK.size = 0;

// 0xac
PACKET.ZC.REQ_TAKEOFF_EQUIP_ACK = function PACKET_ZC_REQ_TAKEOFF_EQUIP_ACK(fp, end) {
	this.index = fp.readUShort();
	this.wearLocation = fp.readUShort();
	this.result = fp.readUChar();
};
PACKET.ZC.REQ_TAKEOFF_EQUIP_ACK.size = 7;

// 0xaf
PACKET.ZC.ITEM_THROW_ACK = function PACKET_ZC_ITEM_THROW_ACK(fp, end) {
	this.Index = fp.readUShort();
	this.count = fp.readShort();
};
PACKET.ZC.ITEM_THROW_ACK.size = 6;

// 0xb0
PACKET.ZC.PAR_CHANGE = function PACKET_ZC_PAR_CHANGE(fp, end) {
	this.varID = fp.readUShort();
	this.count = fp.readLong();
};
PACKET.ZC.PAR_CHANGE.size = 8;

// 0xb1
PACKET.ZC.LONGPAR_CHANGE = function PACKET_ZC_LONGPAR_CHANGE(fp, end) {
	this.varID = fp.readUShort();
	this.amount = fp.readLong();
};
PACKET.ZC.LONGPAR_CHANGE.size = 8;

// 0xb3
PACKET.ZC.RESTART_ACK = function PACKET_ZC_RESTART_ACK(fp, end) {
	this.type = fp.readUChar();
};
PACKET.ZC.RESTART_ACK.size = 3;

// 0xb4
PACKET.ZC.SAY_DIALOG = function PACKET_ZC_SAY_DIALOG(fp, end) {
	this.NAID = fp.readULong();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.SAY_DIALOG.size = -1;

// 0xb5
PACKET.ZC.WAIT_DIALOG = function PACKET_ZC_WAIT_DIALOG(fp, end) {
	this.NAID = fp.readULong();
};
PACKET.ZC.WAIT_DIALOG.size = 6;

// 0xb6
PACKET.ZC.CLOSE_DIALOG = function PACKET_ZC_CLOSE_DIALOG(fp, end) {
	this.NAID = fp.readULong();
};
PACKET.ZC.CLOSE_DIALOG.size = 6;

// 0xb7
PACKET.ZC.MENU_LIST = function PACKET_ZC_MENU_LIST(fp, end) {
	this.NAID = fp.readULong();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.MENU_LIST.size = -1;

// 0xbc
PACKET.ZC.STATUS_CHANGE_ACK = function PACKET_ZC_STATUS_CHANGE_ACK(fp, end) {
	this.statusID = fp.readUShort();
	this.result = fp.readUChar();
	this.value = fp.readUChar();
};
PACKET.ZC.STATUS_CHANGE_ACK.size = 6;

// 0xbd
PACKET.ZC.STATUS = function PACKET_ZC_STATUS(fp, end) {
	this.point = fp.readShort();
	this.str = fp.readUChar();
	this.standardStr = fp.readUChar();
	this.agi = fp.readUChar();
	this.standardAgi = fp.readUChar();
	this.vit = fp.readUChar();
	this.standardVit = fp.readUChar();
	this.Int = fp.readUChar();
	this.standardInt = fp.readUChar();
	this.dex = fp.readUChar();
	this.standardDex = fp.readUChar();
	this.luk = fp.readUChar();
	this.standardLuk = fp.readUChar();
	this.attPower = fp.readShort();
	this.refiningPower = fp.readShort();
	this.max_mattPower = fp.readShort();
	this.min_mattPower = fp.readShort();
	this.itemdefPower = fp.readShort();
	this.plusdefPower = fp.readShort();
	this.mdefPower = fp.readShort();
	this.plusmdefPower = fp.readShort();
	this.hitSuccessValue = fp.readShort();
	this.avoidSuccessValue = fp.readShort();
	this.plusAvoidSuccessValue = fp.readShort();
	this.criticalSuccessValue = fp.readShort();
	this.ASPD = fp.readShort();
	this.plusASPD = fp.readShort();
};
PACKET.ZC.STATUS.size = 44;

// 0xbe
PACKET.ZC.STATUS_CHANGE = function PACKET_ZC_STATUS_CHANGE(fp, end) {
	this.statusID = fp.readUShort();
	this.value = fp.readUChar();
};
PACKET.ZC.STATUS_CHANGE.size = 5;

// 0xc0
PACKET.ZC.EMOTION = function PACKET_ZC_EMOTION(fp, end) {
	this.GID = fp.readULong();
	this.type = fp.readUChar();
};
PACKET.ZC.EMOTION.size = 7;

// 0x0bea
PACKET.ZC.EMOTION_SUCCESS = function PACKET_ZC_EMOTION_SUCCESS(fp, end) {
	this.GID = fp.readULong();
	this.packID = fp.readUShort();
	this.emoteID = fp.readUShort();
};
PACKET.ZC.EMOTION_SUCCESS.size = 10;

// 0x0beb
PACKET.ZC.EMOTION_FAIL = function PACKET_ZC_EMOTION_FAIL(fp, end) {
	this.GID = fp.readULong();
	this.packID = fp.readUShort();
	this.emoteID = fp.readUShort();
};
PACKET.ZC.EMOTION_FAIL.size = 10;

// 0xc2
PACKET.ZC.USER_COUNT = function PACKET_ZC_USER_COUNT(fp, end) {
	this.count = fp.readLong();
};
PACKET.ZC.USER_COUNT.size = 6;

// 0xc3
PACKET.ZC.SPRITE_CHANGE = function PACKET_ZC_SPRITE_CHANGE(fp, end) {
	this.GID = fp.readULong();
	this.type = fp.readUChar();
	this.value = fp.readUChar();
};
PACKET.ZC.SPRITE_CHANGE.size = 8;

// 0xc4
PACKET.ZC.SELECT_DEALTYPE = function PACKET_ZC_SELECT_DEALTYPE(fp, end) {
	this.NAID = fp.readULong();
};
PACKET.ZC.SELECT_DEALTYPE.size = 6;

// 0xc6
PACKET.ZC.PC_PURCHASE_ITEMLIST = function PACKET_ZC_PC_PURCHASE_ITEMLIST(fp, end) {
	this.itemList = (function () {
		const item_size = PACKETVER.value >= 20181121 ? 13 : 11;
		const count = ((end - fp.tell()) / item_size) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].discountprice = fp.readLong();
			out[i].type = fp.readUChar();
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.PC_PURCHASE_ITEMLIST.size = -1;

// 0xc7
PACKET.ZC.PC_SELL_ITEMLIST = function PACKET_ZC_PC_SELL_ITEMLIST(fp, end) {
	this.itemList = (function () {
		const count = ((end - fp.tell()) / 10) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].price = fp.readLong();
			out[i].overchargeprice = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.PC_SELL_ITEMLIST.size = -1;

// 0xca
PACKET.ZC.PC_PURCHASE_RESULT = function PACKET_ZC_PC_PURCHASE_RESULT(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.PC_PURCHASE_RESULT.size = 3;

// 0xcb
PACKET.ZC.PC_SELL_RESULT = function PACKET_ZC_PC_SELL_RESULT(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.PC_SELL_RESULT.size = 3;

// 0xcd
PACKET.ZC.ACK_DISCONNECT_CHARACTER = function PACKET_ZC_ACK_DISCONNECT_CHARACTER(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_DISCONNECT_CHARACTER.size = 3;

// 0xd1
PACKET.ZC.SETTING_WHISPER_PC = function PACKET_ZC_SETTING_WHISPER_PC(fp, end) {
	this.type = fp.readUChar();
	this.result = fp.readUChar();
};
PACKET.ZC.SETTING_WHISPER_PC.size = 4;

// 0xd2
PACKET.ZC.SETTING_WHISPER_STATE = function PACKET_ZC_SETTING_WHISPER_STATE(fp, end) {
	this.type = fp.readUChar();
	this.result = fp.readUChar();
};
PACKET.ZC.SETTING_WHISPER_STATE.size = 4;

// 0xd4
PACKET.ZC.WHISPER_LIST = function PACKET_ZC_WHISPER_LIST(fp, end) {
	this.wisperList = (function () {
		const count = ((end - fp.tell()) / 24) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].name = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
};
PACKET.ZC.WHISPER_LIST.size = -1;

// 0xd6
PACKET.ZC.ACK_CREATE_CHATROOM = function PACKET_ZC_ACK_CREATE_CHATROOM(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_CREATE_CHATROOM.size = 3;

// 0xd7
PACKET.ZC.ROOM_NEWENTRY = function PACKET_ZC_ROOM_NEWENTRY(fp, end) {
	this.AID = fp.readULong();
	this.roomID = fp.readULong();
	this.maxcount = fp.readShort();
	this.curcount = fp.readShort();
	this.type = fp.readUChar();
	this.title = fp.readString(end - fp.tell());
};
PACKET.ZC.ROOM_NEWENTRY.size = -1;

// 0xd8
PACKET.ZC.DESTROY_ROOM = function PACKET_ZC_DESTROY_ROOM(fp, end) {
	this.roomID = fp.readULong();
};
PACKET.ZC.DESTROY_ROOM.size = 6;

// 0xda
PACKET.ZC.REFUSE_ENTER_ROOM = function PACKET_ZC_REFUSE_ENTER_ROOM(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.REFUSE_ENTER_ROOM.size = 3;

// 0xdb
PACKET.ZC.ENTER_ROOM = function PACKET_ZC_ENTER_ROOM(fp, end) {
	this.roomID = fp.readULong();
	this.memberList = (function () {
		const count = ((end - fp.tell()) / 28) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].role = fp.readULong();
			out[i].name = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
};
PACKET.ZC.ENTER_ROOM.size = -1;

// 0xdc
PACKET.ZC.MEMBER_NEWENTRY = function PACKET_ZC_MEMBER_NEWENTRY(fp, end) {
	this.curcount = fp.readShort();
	this.name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.MEMBER_NEWENTRY.size = 28;

// 0xdd
PACKET.ZC.MEMBER_EXIT = function PACKET_ZC_MEMBER_EXIT(fp, end) {
	this.curcount = fp.readShort();
	this.name = fp.readString(NAME_LENGTH);
	this.type = fp.readUChar();
};
PACKET.ZC.MEMBER_EXIT.size = 29;

// 0xdf
PACKET.ZC.CHANGE_CHATROOM = function PACKET_ZC_CHANGE_CHATROOM(fp, end) {
	this.AID = fp.readULong();
	this.roomID = fp.readULong();
	this.maxcount = fp.readShort();
	this.curcount = fp.readShort();
	this.type = fp.readUChar();
	this.title = fp.readString(end - fp.tell());
};
PACKET.ZC.CHANGE_CHATROOM.size = -1;

// 0xe1
PACKET.ZC.ROLE_CHANGE = function PACKET_ZC_ROLE_CHANGE(fp, end) {
	this.role = fp.readULong();
	this.name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ROLE_CHANGE.size = 30;

// 0xe5
PACKET.ZC.REQ_EXCHANGE_ITEM = function PACKET_ZC_REQ_EXCHANGE_ITEM(fp, end) {
	this.name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.REQ_EXCHANGE_ITEM.size = 26;

// 0xe7
PACKET.ZC.ACK_EXCHANGE_ITEM = function PACKET_ZC_ACK_EXCHANGE_ITEM(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_EXCHANGE_ITEM.size = 3;

// 0xe9
PACKET.ZC.ADD_EXCHANGE_ITEM = function PACKET_ZC_ADD_EXCHANGE_ITEM(fp, end) {
	this.count = fp.readLong();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
};
PACKET.ZC.ADD_EXCHANGE_ITEM.size = 19;

// 0xea
PACKET.ZC.ACK_ADD_EXCHANGE_ITEM = function PACKET_ZC_ACK_ADD_EXCHANGE_ITEM(fp, end) {
	this.Index = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_ADD_EXCHANGE_ITEM.size = 5;

// 0xec
PACKET.ZC.CONCLUDE_EXCHANGE_ITEM = function PACKET_ZC_CONCLUDE_EXCHANGE_ITEM(fp, end) {
	this.who = fp.readUChar();
};
PACKET.ZC.CONCLUDE_EXCHANGE_ITEM.size = 3;

// 0xee
PACKET.ZC.CANCEL_EXCHANGE_ITEM = function PACKET_ZC_CANCEL_EXCHANGE_ITEM(fp, end) {};
PACKET.ZC.CANCEL_EXCHANGE_ITEM.size = 2;

// 0xf0
PACKET.ZC.EXEC_EXCHANGE_ITEM = function PACKET_ZC_EXEC_EXCHANGE_ITEM(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.EXEC_EXCHANGE_ITEM.size = 3;

// 0xf1
PACKET.ZC.EXCHANGEITEM_UNDO = function PACKET_ZC_EXCHANGEITEM_UNDO(fp, end) {};
PACKET.ZC.EXCHANGEITEM_UNDO.size = 2;

// 0xf2
PACKET.ZC.NOTIFY_STOREITEM_COUNTINFO = function PACKET_ZC_NOTIFY_STOREITEM_COUNTINFO(fp, end) {
	this.curCount = fp.readShort();
	this.maxCount = fp.readShort();
};
PACKET.ZC.NOTIFY_STOREITEM_COUNTINFO.size = 6;

// 0xf4
PACKET.ZC.ADD_ITEM_TO_STORE = function PACKET_ZC_ADD_ITEM_TO_STORE(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readLong();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
};
PACKET.ZC.ADD_ITEM_TO_STORE.size = 21;

// 0xf6
PACKET.ZC.DELETE_ITEM_FROM_STORE = function PACKET_ZC_DELETE_ITEM_FROM_STORE(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readLong();
};
PACKET.ZC.DELETE_ITEM_FROM_STORE.size = 8;

// 0xf8
PACKET.ZC.CLOSE_STORE = function PACKET_ZC_CLOSE_STORE(fp, end) {};
PACKET.ZC.CLOSE_STORE.size = 2;

// 0xfa
PACKET.ZC.ACK_MAKE_GROUP = function PACKET_ZC_ACK_MAKE_GROUP(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_MAKE_GROUP.size = 3;

// 0xfb
PACKET.ZC.GROUP_LIST = function PACKET_ZC_GROUP_LIST(fp, end) {
	this.groupName = fp.readString(NAME_LENGTH);
	this.groupInfo = (function () {
		const count = ((end - fp.tell()) / 46) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AID = fp.readULong();
			out[i].characterName = fp.readString(NAME_LENGTH);
			out[i].mapName = fp.readBinaryString(16);
			out[i].role = fp.readUChar();
			out[i].state = fp.readUChar();
		}
		return out;
	})();
};
PACKET.ZC.GROUP_LIST.size = -1;

// 0xfd
PACKET.ZC.ACK_REQ_JOIN_GROUP = function PACKET_ZC_ACK_REQ_JOIN_GROUP(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.answer = fp.readUChar();
};
PACKET.ZC.ACK_REQ_JOIN_GROUP.size = 27;

// 0xfe
PACKET.ZC.REQ_JOIN_GROUP = function PACKET_ZC_REQ_JOIN_GROUP(fp, end) {
	this.GRID = fp.readULong();
	this.groupName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.REQ_JOIN_GROUP.size = 30;

// 0x101
PACKET.ZC.GROUPINFO_CHANGE = function PACKET_ZC_GROUPINFO_CHANGE(fp, end) {
	this.expOption = fp.readULong();
};
PACKET.ZC.GROUPINFO_CHANGE.size = 6;

// 0x104
PACKET.ZC.ADD_MEMBER_TO_GROUP = function PACKET_ZC_ADD_MEMBER_TO_GROUP(fp, end) {
	this.AID = fp.readULong();
	this.role = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.state = fp.readUChar();
	this.groupName = fp.readString(NAME_LENGTH);
	this.characterName = fp.readString(NAME_LENGTH);
	this.mapName = fp.readBinaryString(16);
};
PACKET.ZC.ADD_MEMBER_TO_GROUP.size = 79;

// 0x105
PACKET.ZC.DELETE_MEMBER_FROM_GROUP = function PACKET_ZC_DELETE_MEMBER_FROM_GROUP(fp, end) {
	this.AID = fp.readULong();
	this.characterName = fp.readString(NAME_LENGTH);
	this.result = fp.readUChar();
};
PACKET.ZC.DELETE_MEMBER_FROM_GROUP.size = 31;

// 0x106
PACKET.ZC.NOTIFY_HP_TO_GROUPM = function PACKET_ZC_NOTIFY_HP_TO_GROUPM(fp, end) {
	this.AID = fp.readULong();
	this.hp = fp.readShort();
	this.maxhp = fp.readShort();
};
PACKET.ZC.NOTIFY_HP_TO_GROUPM.size = 10;

// 0x107
PACKET.ZC.NOTIFY_POSITION_TO_GROUPM = function PACKET_ZC_NOTIFY_POSITION_TO_GROUPM(fp, end) {
	this.AID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
};
PACKET.ZC.NOTIFY_POSITION_TO_GROUPM.size = 10;

// 0x109
PACKET.ZC.NOTIFY_CHAT_PARTY = function PACKET_ZC_NOTIFY_CHAT_PARTY(fp, end) {
	this.AID = fp.readULong();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_CHAT_PARTY.size = -1;

// 0x10a
PACKET.ZC.MVP_GETTING_ITEM = function PACKET_ZC_MVP_GETTING_ITEM(fp, end) {
	this.ITID = fp.readUShort();
};
PACKET.ZC.MVP_GETTING_ITEM.size = 4;

// 0x10b
PACKET.ZC.MVP_GETTING_SPECIAL_EXP = function PACKET_ZC_MVP_GETTING_SPECIAL_EXP(fp, end) {
	this.exp = fp.readULong();
};
PACKET.ZC.MVP_GETTING_SPECIAL_EXP.size = 6;

// 0x10c
PACKET.ZC.MVP = function PACKET_ZC_MVP(fp, end) {
	this.AID = fp.readULong();
};
PACKET.ZC.MVP.size = 6;

// 0x10d
PACKET.ZC.THROW_MVPITEM = function PACKET_ZC_THROW_MVPITEM(fp, end) {};
PACKET.ZC.THROW_MVPITEM.size = 2;

// 0x10e
PACKET.ZC.SKILLINFO_UPDATE = function PACKET_ZC_SKILLINFO_UPDATE(fp, end) {
	this.SKID = fp.readUShort();
	this.level = fp.readShort();
	this.spcost = fp.readShort();
	this.attackRange = fp.readShort();
	this.upgradable = fp.readUChar();
};
PACKET.ZC.SKILLINFO_UPDATE.size = 11;

// 0x10f
PACKET.ZC.SKILLINFO_LIST = function PACKET_ZC_SKILLINFO_LIST(fp, end) {
	this.skillList = (function () {
		const count = ((end - fp.tell()) / 37) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].SKID = fp.readShort();
			out[i].type = fp.readLong();
			out[i].level = fp.readShort();
			out[i].spcost = fp.readShort();
			out[i].attackRange = fp.readShort();
			out[i].skillName = fp.readBinaryString(NAME_LENGTH);
			out[i].upgradable = fp.readChar();
		}
		return out;
	})();
};
PACKET.ZC.SKILLINFO_LIST.size = -1;

// 0x0110
PACKET.ZC.ACK_TOUSESKILL = function PACKET_ZC_ACK_TOUSESKILL(fp, end) {
	this.SKID = fp.readUShort();
	if ((CLASSIC && PACKETVER.value >= 20181121) || (RENEWAL && PACKETVER.value >= 20180704)) {
		this.NUM = fp.readLong();
		this.itemId = fp.readULong();
	} else {
		this.NUM = fp.readShort();
		this.itemId = fp.readUShort();
	}
	this.result = fp.readUChar();
	this.cause = fp.readUChar();
};
PACKET.ZC.ACK_TOUSESKILL.size =
	(CLASSIC && PACKETVER.value >= 20181121) || (RENEWAL && PACKETVER.value >= 20180704) ? 14 : 10;

// 0x111
PACKET.ZC.ADD_SKILL = function PACKET_ZC_ADD_SKILL(fp, end) {
	this.data = {};
	this.data.SKID = fp.readUShort();
	this.data.type = fp.readLong();
	this.data.level = fp.readShort();
	this.data.spcost = fp.readShort();
	this.data.attackRange = fp.readShort();
	this.data.skillName = fp.readBinaryString(NAME_LENGTH);
	this.data.upgradable = fp.readUChar();
};
PACKET.ZC.ADD_SKILL.size = 39;

// 0x114
PACKET.ZC.NOTIFY_SKILL = function PACKET_ZC_NOTIFY_SKILL(fp, end) {
	this.SKID = fp.readUShort();
	this.AID = fp.readULong();
	this.targetID = fp.readULong();
	this.startTime = fp.readULong();
	this.attackMT = fp.readLong();
	this.attackedMT = fp.readLong();
	this.damage = fp.readShort();
	this.level = fp.readShort();
	this.count = fp.readShort();
	this.action = fp.readUChar();
};
PACKET.ZC.NOTIFY_SKILL.size = 31;

// 0x115
PACKET.ZC.NOTIFY_SKILL_POSITION = function PACKET_ZC_NOTIFY_SKILL_POSITION(fp, end) {
	this.SKID = fp.readUShort();
	this.AID = fp.readULong();
	this.targetID = fp.readULong();
	this.startTime = fp.readULong();
	this.attackMT = fp.readLong();
	this.attackedMT = fp.readLong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.damage = fp.readShort();
	this.level = fp.readShort();
	this.count = fp.readShort();
	this.action = fp.readUChar();
};
PACKET.ZC.NOTIFY_SKILL_POSITION.size = 35;

// 0x117
PACKET.ZC.NOTIFY_GROUNDSKILL = function PACKET_ZC_NOTIFY_GROUNDSKILL(fp, end) {
	this.SKID = fp.readUShort();
	this.AID = fp.readULong();
	this.level = fp.readShort();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.startTime = fp.readULong();
};
PACKET.ZC.NOTIFY_GROUNDSKILL.size = 18;

// 0x119
PACKET.ZC.STATE_CHANGE = function PACKET_ZC_STATE_CHANGE(fp, end) {
	this.AID = fp.readULong();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.isPKModeON = fp.readUChar();
};
PACKET.ZC.STATE_CHANGE.size = 13;

// 0x11a
PACKET.ZC.USE_SKILL = function PACKET_ZC_USE_SKILL(fp, end) {
	this.SKID = fp.readUShort();
	this.level = fp.readShort();
	this.targetAID = fp.readULong();
	this.srcAID = fp.readULong();
	this.result = fp.readUChar();
};
PACKET.ZC.USE_SKILL.size = 15;

// 0x11c
PACKET.ZC.WARPLIST = function PACKET_ZC_WARPLIST(fp, end) {
	this.SKID = fp.readUShort();
	this.mapName = (function () {
		const count = 4;

		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readBinaryString(16);
		}
		return out;
	})();
};
PACKET.ZC.WARPLIST.size = 68;

// 0xabe
PACKET.ZC.WARPLIST2 = function PACKET_ZC_WARPLIST2(fp, end) {
	this.SKID = fp.readUShort();
	this.mapName = (function () {
		const count = ((end - fp.tell()) / 16) | 0;

		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readBinaryString(16);
		}
		return out;
	})();
};
PACKET.ZC.WARPLIST2.size = -1;

// 0x11e
PACKET.ZC.ACK_REMEMBER_WARPPOINT = function PACKET_ZC_ACK_REMEMBER_WARPPOINT(fp, end) {
	this.errorCode = fp.readUChar();
};
PACKET.ZC.ACK_REMEMBER_WARPPOINT.size = 3;

// 0x11f
PACKET.ZC.SKILL_ENTRY = function PACKET_ZC_SKILL_ENTRY(fp, end) {
	this.AID = fp.readULong();
	this.creatorAID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.job = fp.readUChar();
	this.isVisible = fp.readUChar();
};
PACKET.ZC.SKILL_ENTRY.size = 16;

// 0x120
PACKET.ZC.SKILL_DISAPPEAR = function PACKET_ZC_SKILL_DISAPPEAR(fp, end) {
	this.AID = fp.readULong();
};
PACKET.ZC.SKILL_DISAPPEAR.size = 6;

// 0x121
PACKET.ZC.NOTIFY_CARTITEM_COUNTINFO = function PACKET_ZC_NOTIFY_CARTITEM_COUNTINFO(fp, end) {
	this.curCount = fp.readShort();
	this.maxCount = fp.readShort();
	this.curWeight = fp.readLong();
	this.maxWeight = fp.readLong();
};
PACKET.ZC.NOTIFY_CARTITEM_COUNTINFO.size = 14;

// 0x122
PACKET.ZC.CART_EQUIPMENT_ITEMLIST = function PACKET_ZC_CART_EQUIPMENT_ITEMLIST(fp, end) {
	this.itemInfo = (function () {
		const count = ((end - fp.tell()) / 20) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.CART_EQUIPMENT_ITEMLIST.size = -1;

// 0x123
PACKET.ZC.CART_NORMAL_ITEMLIST = function PACKET_ZC_CART_NORMAL_ITEMLIST(fp, end) {
	this.itemInfo = (function () {
		const count = ((end - fp.tell()) / 10) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.CART_NORMAL_ITEMLIST.size = -1;

// 0x124
PACKET.ZC.ADD_ITEM_TO_CART = function PACKET_ZC_ADD_ITEM_TO_CART(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readLong();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
};
PACKET.ZC.ADD_ITEM_TO_CART.size = 21;

// 0x125
PACKET.ZC.DELETE_ITEM_FROM_CART = function PACKET_ZC_DELETE_ITEM_FROM_CART(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readLong();
};
PACKET.ZC.DELETE_ITEM_FROM_CART.size = 8;

// 0x12b
PACKET.ZC.CARTOFF = function PACKET_ZC_CARTOFF(fp, end) {};
PACKET.ZC.CARTOFF.size = 2;

// 0x12c
PACKET.ZC.ACK_ADDITEM_TO_CART = function PACKET_ZC_ACK_ADDITEM_TO_CART(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_ADDITEM_TO_CART.size = 3;

// 0x12d
PACKET.ZC.OPENSTORE = function PACKET_ZC_OPENSTORE(fp, end) {
	this.itemcount = fp.readShort();
};
PACKET.ZC.OPENSTORE.size = 4;

// 0x131
PACKET.ZC.STORE_ENTRY = function PACKET_ZC_STORE_ENTRY(fp, end) {
	this.makerAID = fp.readULong();
	this.storeName = fp.readString(80);
};
PACKET.ZC.STORE_ENTRY.size = 86;

// 0x132
PACKET.ZC.DISAPPEAR_ENTRY = function PACKET_ZC_DISAPPEAR_ENTRY(fp, end) {
	this.makerAID = fp.readULong();
};
PACKET.ZC.DISAPPEAR_ENTRY.size = 6;

// 0x133
PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC = function PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC(fp, end) {
	this.AID = fp.readULong();
	this.itemList = (function () {
		const count = ((end - fp.tell()) / 22) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].count = fp.readShort();
			out[i].index = fp.readShort();
			out[i].type = fp.readUChar();
			out[i].ITID = fp.readUShort();
			out[i].IsIdentified = fp.readUChar();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC.size = -1;

// 0x135
PACKET.ZC.PC_PURCHASE_RESULT_FROMMC = function PACKET_ZC_PC_PURCHASE_RESULT_FROMMC(fp, end) {
	this.index = fp.readShort();
	this.curcount = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.PC_PURCHASE_RESULT_FROMMC.size = 7;

// 0x136
PACKET.ZC.PC_PURCHASE_MYITEMLIST = function PACKET_ZC_PC_PURCHASE_MYITEMLIST(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.AID = fp.readULong();
	this.itemList = (function () {
		let len = 22;
		if (PACKETVER.value >= 20150226) {
			len = 47;
		}

		const count = ((end - fp.tell()) / len) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].index = fp.readShort();
			out[i].count = fp.readShort();
			out[i].type = fp.readUChar();
			out[i].ITID = fp.readUShort();
			out[i].IsIdentified = fp.readUChar();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();

			if (PACKETVER.value >= 20150226) {
				out[i].Options = [];
				out[i].Options[1] = fp.readStruct(option);
				out[i].Options[2] = fp.readStruct(option);
				out[i].Options[3] = fp.readStruct(option);
				out[i].Options[4] = fp.readStruct(option);
				out[i].Options[5] = fp.readStruct(option);
			}
		}
		return out;
	})();
};
PACKET.ZC.PC_PURCHASE_MYITEMLIST.size = -1;

// 0x137
PACKET.ZC.DELETEITEM_FROM_MCSTORE = function PACKET_ZC_DELETEITEM_FROM_MCSTORE(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readShort();
};
PACKET.ZC.DELETEITEM_FROM_MCSTORE.size = 6;

// 0x139
PACKET.ZC.ATTACK_FAILURE_FOR_DISTANCE = function PACKET_ZC_ATTACK_FAILURE_FOR_DISTANCE(fp, end) {
	this.targetAID = fp.readULong();
	this.targetXPos = fp.readShort();
	this.targetYPos = fp.readShort();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.currentAttRange = fp.readShort();
};
PACKET.ZC.ATTACK_FAILURE_FOR_DISTANCE.size = 16;

// 0x13a
PACKET.ZC.ATTACK_RANGE = function PACKET_ZC_ATTACK_RANGE(fp, end) {
	this.currentAttRange = fp.readShort();
};
PACKET.ZC.ATTACK_RANGE.size = 4;

// 0x13b
PACKET.ZC.ACTION_FAILURE = function PACKET_ZC_ACTION_FAILURE(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.ZC.ACTION_FAILURE.size = 4;

// 0x13c
PACKET.ZC.EQUIP_ARROW = function PACKET_ZC_EQUIP_ARROW(fp, end) {
	this.index = fp.readShort();
};
PACKET.ZC.EQUIP_ARROW.size = 4;

// 0x13d
PACKET.ZC.RECOVERY = function PACKET_ZC_RECOVERY(fp, end) {
	this.varID = fp.readShort();
	this.amount = fp.readShort();
};
PACKET.ZC.RECOVERY.size = 6;

// 0x13e
PACKET.ZC.USESKILL_ACK = function PACKET_ZC_USESKILL_ACK(fp, end) {
	this.AID = fp.readULong();
	this.targetID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.SKID = fp.readUShort();
	this.property = fp.readULong();
	this.delayTime = fp.readULong();
};
PACKET.ZC.USESKILL_ACK.size = 24;

// 0x141
PACKET.ZC.COUPLESTATUS = function PACKET_ZC_COUPLESTATUS(fp, end) {
	this.statusType = fp.readULong();
	this.defaultStatus = fp.readLong();
	this.plusStatus = fp.readLong();
};
PACKET.ZC.COUPLESTATUS.size = 14;

// 0x142
PACKET.ZC.OPEN_EDITDLG = function PACKET_ZC_OPEN_EDITDLG(fp, end) {
	this.NAID = fp.readULong();
};
PACKET.ZC.OPEN_EDITDLG.size = 6;

// 0x144
PACKET.ZC.COMPASS = function PACKET_ZC_COMPASS(fp, end) {
	this.NAID = fp.readULong();
	this.type = fp.readLong();
	this.xPos = fp.readLong();
	this.yPos = fp.readLong();
	this.id = fp.readUChar();
	this.color = fp.readULong();
};
PACKET.ZC.COMPASS.size = 23;

// 0x145
PACKET.ZC.SHOW_IMAGE = function PACKET_ZC_SHOW_IMAGE(fp, end) {
	this.imageName = fp.readBinaryString(16);
	this.type = fp.readUChar();
};
PACKET.ZC.SHOW_IMAGE.size = 19;

// 0x147
PACKET.ZC.AUTORUN_SKILL = function PACKET_ZC_AUTORUN_SKILL(fp, end) {
	this.data = {};
	this.data.SKID = fp.readUShort();
	this.data.type = fp.readLong();
	this.data.level = fp.readShort();
	this.data.spcost = fp.readShort();
	this.data.attackRange = fp.readShort();
	this.data.skillName = fp.readBinaryString(NAME_LENGTH);
	this.data.upgradable = fp.readUChar();
};
PACKET.ZC.AUTORUN_SKILL.size = 39;

// 0x148
PACKET.ZC.RESURRECTION = function PACKET_ZC_RESURRECTION(fp, end) {
	this.AID = fp.readULong();
	this.type = fp.readShort();
};
PACKET.ZC.RESURRECTION.size = 8;

// 0x14a
PACKET.ZC.ACK_GIVE_MANNER_POINT = function PACKET_ZC_ACK_GIVE_MANNER_POINT(fp, end) {
	this.result = fp.readULong();
};
PACKET.ZC.ACK_GIVE_MANNER_POINT.size = 6;

// 0x14b
PACKET.ZC.NOTIFY_MANNER_POINT_GIVEN = function PACKET_ZC_NOTIFY_MANNER_POINT_GIVEN(fp, end) {
	this.type = fp.readUChar();
	this.otherCharName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.NOTIFY_MANNER_POINT_GIVEN.size = 27;

// 0x14c
PACKET.ZC.MYGUILD_BASIC_INFO = function PACKET_ZC_MYGUILD_BASIC_INFO(fp, end) {
	this.relatedGuildList = (function () {
		const count = ((end - fp.tell()) / 32) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].relation = fp.readLong();
			out[i].GDID = fp.readLong();
			out[i].guildName = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
};
PACKET.ZC.MYGUILD_BASIC_INFO.size = -1;

// 0x14e
PACKET.ZC.ACK_GUILD_MENUINTERFACE = function PACKET_ZC_ACK_GUILD_MENUINTERFACE(fp, end) {
	this.guildMemuFlag = fp.readLong();
};
PACKET.ZC.ACK_GUILD_MENUINTERFACE.size = 6;

// 0x150
PACKET.ZC.GUILD_INFO = function PACKET_ZC_GUILD_INFO(fp, end) {
	this.GDID = fp.readLong();
	this.level = fp.readLong();
	this.userNum = fp.readLong();
	this.maxUserNum = fp.readLong();
	this.userAverageLevel = fp.readLong();
	this.exp = fp.readLong();
	this.maxExp = fp.readLong();
	this.point = fp.readLong();
	this.honor = fp.readLong();
	this.virtue = fp.readLong();
	this.emblemVersion = fp.readLong();
	this.guildname = fp.readString(NAME_LENGTH);
	this.masterName = fp.readString(NAME_LENGTH);
	this.manageLand = fp.readBinaryString(16);
};
PACKET.ZC.GUILD_INFO.size = 110;

// 0x152
PACKET.ZC.GUILD_EMBLEM_IMG = function PACKET_ZC_GUILD_EMBLEM_IMG(fp, end) {
	this.GDID = fp.readLong();
	this.emblemVersion = fp.readLong();
	this.img = new Uint8Array(fp.buffer, fp.offset, end - fp.offset);
};
PACKET.ZC.GUILD_EMBLEM_IMG.size = -1;

// 0x154
PACKET.ZC.MEMBERMGR_INFO = function PACKET_ZC_MEMBERMGR_INFO(fp, end) {
	this.memberInfo = (function () {
		const count = ((end - fp.tell()) / 104) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AID = fp.readULong();
			out[i].GID = fp.readULong();
			out[i].HeadType = fp.readShort();
			out[i].HeadPalette = fp.readShort();
			out[i].Sex = fp.readShort();
			out[i].Job = fp.readShort();
			out[i].Level = fp.readShort();
			out[i].MemberExp = fp.readLong();
			out[i].CurrentState = fp.readLong();
			out[i].GPositionID = fp.readLong();
			out[i].Memo = fp.readString(50);
			out[i].CharName = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
};
PACKET.ZC.MEMBERMGR_INFO.size = -1;

// 0x156
PACKET.ZC.ACK_REQ_CHANGE_MEMBERS = function PACKET_ZC_ACK_REQ_CHANGE_MEMBERS(fp, end) {
	this.memberInfo = (function () {
		const count = ((end - fp.tell()) / 12) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AID = fp.readLong();
			out[i].GID = fp.readLong();
			out[i].positionID = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.ACK_REQ_CHANGE_MEMBERS.size = -1;

// 0x158
PACKET.ZC.ACK_OPEN_MEMBER_INFO = function PACKET_ZC_ACK_OPEN_MEMBER_INFO(fp, end) {};
PACKET.ZC.ACK_OPEN_MEMBER_INFO.size = 2;

// 0x15a
PACKET.ZC.ACK_LEAVE_GUILD = function PACKET_ZC_ACK_LEAVE_GUILD(fp, end) {
	this.charName = fp.readString(NAME_LENGTH);
	this.reasonDesc = fp.readString(40);
};
PACKET.ZC.ACK_LEAVE_GUILD.size = 66;

// 0x15c
PACKET.ZC.ACK_BAN_GUILD = function PACKET_ZC_ACK_BAN_GUILD(fp, end) {
	this.charName = fp.readString(NAME_LENGTH);
	this.reasonDesc = fp.readString(40);
	this.account = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ACK_BAN_GUILD.size = 90;

// 0x15e
PACKET.ZC.ACK_DISORGANIZE_GUILD_RESULT = function PACKET_ZC_ACK_DISORGANIZE_GUILD_RESULT(fp, end) {
	this.reason = fp.readLong();
};
PACKET.ZC.ACK_DISORGANIZE_GUILD_RESULT.size = 6;

// 0x15f
PACKET.ZC.ACK_DISORGANIZE_GUILD = function PACKET_ZC_ACK_DISORGANIZE_GUILD(fp, end) {
	this.reasonDesc = fp.readString(40);
};
PACKET.ZC.ACK_DISORGANIZE_GUILD.size = 42;

// 0x160
PACKET.ZC.POSITION_INFO = function PACKET_ZC_POSITION_INFO(fp, end) {
	this.memberInfo = (function () {
		const count = ((end - fp.tell()) / 16) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].positionID = fp.readLong();
			out[i].right = fp.readLong();
			out[i].ranking = fp.readLong();
			out[i].payRate = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.POSITION_INFO.size = -1;

// 0x162
PACKET.ZC.GUILD_SKILLINFO = function PACKET_ZC_GUILD_SKILLINFO(fp, end) {
	this.skillPoint = fp.readShort();
	this.skillList = (function () {
		const count = ((end - fp.tell()) / 37) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].SKID = fp.readUShort();
			out[i].type = fp.readLong();
			out[i].level = fp.readShort();
			out[i].spcost = fp.readShort();
			out[i].attackRange = fp.readShort();
			out[i].skillName = fp.readBinaryString(NAME_LENGTH);
			out[i].upgradable = fp.readChar();
		}
		return out;
	})();
};
PACKET.ZC.GUILD_SKILLINFO.size = -1;

// 0x163
PACKET.ZC.BAN_LIST = function PACKET_ZC_BAN_LIST(fp, end) {
	this.banList = (function () {
		const size = PACKETVER.max < 20100803 ? 88 : 64;
		const count = ((end - fp.tell()) / size) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].charname = fp.readString(NAME_LENGTH);

			if (PACKETVER.max < 20100803) {
				out[i].account = fp.readString(NAME_LENGTH);
			}

			out[i].reason = fp.readString(40);
		}
		return out;
	})();
};
PACKET.ZC.BAN_LIST.size = -1;

// 0x164
PACKET.ZC.OTHER_GUILD_LIST = function PACKET_ZC_OTHER_GUILD_LIST(fp, end) {
	this.guildList = (function () {
		const count = ((end - fp.tell()) / 36) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].guildname = fp.readString(NAME_LENGTH);
			out[i].guildLevel = fp.readLong();
			out[i].guildMemberSize = fp.readLong();
			out[i].guildRanking = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.OTHER_GUILD_LIST.size = -1;

// 0x166
PACKET.ZC.POSITION_ID_NAME_INFO = function PACKET_ZC_POSITION_ID_NAME_INFO(fp, end) {
	this.memberList = (function () {
		const count = ((end - fp.tell()) / 28) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].positionID = fp.readLong();
			out[i].posName = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
};
PACKET.ZC.POSITION_ID_NAME_INFO.size = -1;

// 0x167
PACKET.ZC.RESULT_MAKE_GUILD = function PACKET_ZC_RESULT_MAKE_GUILD(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.RESULT_MAKE_GUILD.size = 3;

// 0x169
PACKET.ZC.ACK_REQ_JOIN_GUILD = function PACKET_ZC_ACK_REQ_JOIN_GUILD(fp, end) {
	this.answer = fp.readUChar();
};
PACKET.ZC.ACK_REQ_JOIN_GUILD.size = 3;

// 0x16a
PACKET.ZC.REQ_JOIN_GUILD = function PACKET_ZC_REQ_JOIN_GUILD(fp, end) {
	this.GDID = fp.readULong();
	this.guildName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.REQ_JOIN_GUILD.size = 30;

// 0x16c
PACKET.ZC.UPDATE_GDID = function PACKET_ZC_UPDATE_GDID(fp, end) {
	this.GDID = fp.readULong();
	this.emblemVersion = fp.readLong();
	this.right = fp.readLong();
	this.isMaster = fp.readUChar();
	this.InterSID = fp.readLong();
	this.GName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.UPDATE_GDID.size = 43;

// 0x2f7
PACKET.ZC.UPDATE_GDID2 = function PACKET_ZC_UPDATE_GDID2(fp, end) {
	if (end - fp.tell() < 45) {
		return;
	}

	this.GDID = fp.readULong();
	this.emblemVersion = fp.readLong();
	this.right = fp.readLong();
	this.isMaster = fp.readUChar();
	this.InterSID = fp.readLong();
	this.GName = fp.readString(NAME_LENGTH);
	this.masterGID = fp.readULong();
};
PACKET.ZC.UPDATE_GDID2.size = 47;

// 0x16d
PACKET.ZC.UPDATE_CHARSTAT = function PACKET_ZC_UPDATE_CHARSTAT(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.status = fp.readLong();
};
PACKET.ZC.UPDATE_CHARSTAT.size = 14;

// 0x16f
PACKET.ZC.GUILD_NOTICE = function PACKET_ZC_GUILD_NOTICE(fp, end) {
	this.subject = fp.readString(60);
	this.notice = fp.readString(120);
};
PACKET.ZC.GUILD_NOTICE.size = 182;

// 0x171
PACKET.ZC.REQ_ALLY_GUILD = function PACKET_ZC_REQ_ALLY_GUILD(fp, end) {
	this.otherAID = fp.readULong();
	this.guildName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.REQ_ALLY_GUILD.size = 30;

// 0x173
PACKET.ZC.ACK_REQ_ALLY_GUILD = function PACKET_ZC_ACK_REQ_ALLY_GUILD(fp, end) {
	this.answer = fp.readUChar();
};
PACKET.ZC.ACK_REQ_ALLY_GUILD.size = 3;

// 0x174
PACKET.ZC.ACK_CHANGE_GUILD_POSITIONINFO = function PACKET_ZC_ACK_CHANGE_GUILD_POSITIONINFO(fp, end) {
	this.memberList = (function () {
		const count = ((end - fp.tell()) / 40) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].positionID = fp.readLong();
			out[i].right = fp.readLong();
			out[i].ranking = fp.readLong();
			out[i].payRate = fp.readLong();
			out[i].posName = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
};
PACKET.ZC.ACK_CHANGE_GUILD_POSITIONINFO.size = -1;

// 0x176
PACKET.ZC.ACK_GUILD_MEMBER_INFO = function PACKET_ZC_ACK_GUILD_MEMBER_INFO(fp, end) {
	this.Info = {};
	this.Info.AID = fp.readLong();
	this.Info.GID = fp.readLong();
	this.Info.HeadType = fp.readShort();
	this.Info.HeadPalette = fp.readShort();
	this.Info.Sex = fp.readShort();
	this.Info.Job = fp.readShort();
	this.Info.Level = fp.readShort();
	this.Info.MemberExp = fp.readLong();
	this.Info.CurrentState = fp.readLong();
	this.Info.GPositionID = fp.readLong();
	this.Info.Memo = fp.readString(50);
	this.Info.CharName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ACK_GUILD_MEMBER_INFO.size = 106;

// 0x177
PACKET.ZC.ITEMIDENTIFY_LIST = function PACKET_ZC_ITEMIDENTIFY_LIST(fp, end) {
	this.ITIDList = (function () {
		const count = ((end - fp.tell()) / 2) | 0,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.ITEMIDENTIFY_LIST.size = -1;

// 0x179
PACKET.ZC.ACK_ITEMIDENTIFY = function PACKET_ZC_ACK_ITEMIDENTIFY(fp, end) {
	this.index = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_ITEMIDENTIFY.size = 5;

// 0x17b
PACKET.ZC.ITEMCOMPOSITION_LIST = function PACKET_ZC_ITEMCOMPOSITION_LIST(fp, end) {
	this.ITIDList = (function () {
		const count = ((end - fp.tell()) / 2) | 0,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.ITEMCOMPOSITION_LIST.size = -1;

// 0x17d
PACKET.ZC.ACK_ITEMCOMPOSITION = function PACKET_ZC_ACK_ITEMCOMPOSITION(fp, end) {
	this.equipIndex = fp.readShort();
	this.cardIndex = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_ITEMCOMPOSITION.size = 7;

// 0x17f
PACKET.ZC.GUILD_CHAT = function PACKET_ZC_GUILD_CHAT(fp, end) {
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.GUILD_CHAT.size = -1;

// 0x181
PACKET.ZC.ACK_REQ_HOSTILE_GUILD = function PACKET_ZC_ACK_REQ_HOSTILE_GUILD(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_REQ_HOSTILE_GUILD.size = 3;

// 0x182
PACKET.ZC.MEMBER_ADD = function PACKET_ZC_MEMBER_ADD(fp, end) {
	this.Info = {};
	this.Info.AID = fp.readLong();
	this.Info.GID = fp.readLong();
	this.Info.head = fp.readShort();
	this.Info.headPalette = fp.readShort();
	this.Info.sex = fp.readShort();
	this.Info.job = fp.readShort();
	this.Info.level = fp.readShort();
	this.Info.contributionExp = fp.readLong();
	this.Info.currentState = fp.readLong();
	this.Info.positionID = fp.readLong();
	this.Info.intro = fp.readString(50);
	this.Info.charname = fp.readString(NAME_LENGTH);
};
PACKET.ZC.MEMBER_ADD.size = 106;

// 0x184
PACKET.ZC.DELETE_RELATED_GUILD = function PACKET_ZC_DELETE_RELATED_GUILD(fp, end) {
	this.OpponentGDID = fp.readULong();
	this.Relation = fp.readLong();
};
PACKET.ZC.DELETE_RELATED_GUILD.size = 10;

// 0x185
PACKET.ZC.ADD_RELATED_GUILD = function PACKET_ZC_ADD_RELATED_GUILD(fp, end) {
	this.Info = {};
	this.Info.relation = fp.readLong();
	this.Info.GDID = fp.readLong();
	this.Info.guildname = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ADD_RELATED_GUILD.size = 34;

// 0x186
PACKET.ZC.COLLECTORDEAD = function PACKET_COLLECTORDEAD(fp, end) {
	this.ServerID = fp.readULong();
};
PACKET.ZC.COLLECTORDEAD.size = 6;

// 0x187
PACKET.HC.PING = function PACKET_HC_PING(fp, end) {
	this.AID = fp.readULong();
};
PACKET.HC.PING.size = 6;

// 0x188
PACKET.ZC.ACK_ITEMREFINING = function PACKET_ZC_ACK_ITEMREFINING(fp, end) {
	this.result = fp.readShort();
	this.itemIndex = fp.readShort();
	this.RefiningLevel = fp.readShort();
};
PACKET.ZC.ACK_ITEMREFINING.size = 8;

// 0x189
PACKET.ZC.NOTIFY_MAPINFO = function PACKET_ZC_NOTIFY_MAPINFO(fp, end) {
	this.type = fp.readShort();
};
PACKET.ZC.NOTIFY_MAPINFO.size = 4;

// 0x18b
PACKET.ZC.ACK_REQ_DISCONNECT = function PACKET_ZC_ACK_REQ_DISCONNECT(fp, end) {
	this.result = fp.readShort();
};
PACKET.ZC.ACK_REQ_DISCONNECT.size = 4;

// 0x18c
PACKET.ZC.MONSTER_INFO = function PACKET_ZC_MONSTER_INFO(fp, end) {
	this.job = fp.readShort();
	this.level = fp.readShort();
	this.size = fp.readShort();
	this.hp = fp.readLong();
	this.def = fp.readShort();
	this.raceType = fp.readShort();
	this.mdefPower = fp.readShort();
	this.property = fp.readShort();
	this.propertyTable = {};
	this.propertyTable.water = fp.readUChar();
	this.propertyTable.earth = fp.readUChar();
	this.propertyTable.fire = fp.readUChar();
	this.propertyTable.wind = fp.readUChar();
	this.propertyTable.poison = fp.readUChar();
	this.propertyTable.saint = fp.readUChar();
	this.propertyTable.dark = fp.readUChar();
	this.propertyTable.mental = fp.readUChar();
	this.propertyTable.undead = fp.readUChar();
};
PACKET.ZC.MONSTER_INFO.size = 29;

// 0x18d
PACKET.ZC.MAKABLEITEMLIST = function PACKET_ZC_MAKABLEITEMLIST(fp, end) {
	const size = PACKETVER.value >= 20181121 ? 16 : 8;
	this.itemList = (function (_size) {
		const count = ((end - fp.tell()) / _size) | 0;
		const out = new Array(count);

		for (let i = 0; i < count; ++i) {
			out[i] = {};
			if (_size == 16) {
				out[i].ITID = fp.readULong();
				out[i].material_ID = {};
				out[i].material_ID[0] = fp.readULong();
				out[i].material_ID[1] = fp.readULong();
				out[i].material_ID[2] = fp.readULong();
			} else {
				out[i].ITID = fp.readShort();
				out[i].material_ID = {};
				out[i].material_ID[0] = fp.readShort();
				out[i].material_ID[1] = fp.readShort();
				out[i].material_ID[2] = fp.readShort();
			}
		}
		return out;
	})(size);
};
PACKET.ZC.MAKABLEITEMLIST.size = -1;

// 0x18f
PACKET.ZC.ACK_REQMAKINGITEM = function PACKET_ZC_ACK_REQMAKINGITEM(fp, end) {
	this.result = fp.readShort();
	this.ITID = fp.readUShort();
};
PACKET.ZC.ACK_REQMAKINGITEM.size = 6;

// 0x191
PACKET.ZC.TALKBOX_CHATCONTENTS = function PACKET_ZC_TALKBOX_CHATCONTENTS(fp, end) {
	this.AID = fp.readULong();
	this.contents = fp.readBinaryString(80);
};
PACKET.ZC.TALKBOX_CHATCONTENTS.size = 86;

// 0x192
PACKET.ZC.UPDATE_MAPINFO = function PACKET_ZC_UPDATE_MAPINFO(fp, end) {
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.type = fp.readShort();
	this.mapName = fp.readBinaryString(16);
};
PACKET.ZC.UPDATE_MAPINFO.size = 24;

// 0x194
PACKET.ZC.ACK_REQNAME_BYGID = function PACKET_ZC_ACK_REQNAME_BYGID(fp, end) {
	this.GID = fp.readULong();
	this.CName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ACK_REQNAME_BYGID.size = 30;

// 0x195
PACKET.ZC.ACK_REQNAMEALL = function PACKET_ZC_ACK_REQNAMEALL(fp, end) {
	this.AID = fp.readULong();
	this.CName = fp.readString(NAME_LENGTH);
	this.PName = fp.readString(NAME_LENGTH);
	this.GName = fp.readString(NAME_LENGTH);
	this.RName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ACK_REQNAMEALL.size = 102;

// 0x196
PACKET.ZC.MSG_STATE_CHANGE = function PACKET_ZC_MSG_STATE_CHANGE(fp, end) {
	this.index = fp.readShort();
	this.AID = fp.readULong();
	this.state = fp.readUChar();
};
PACKET.ZC.MSG_STATE_CHANGE.size = 9;

// 0x199
PACKET.ZC.NOTIFY_MAPPROPERTY = function PACKET_ZC_NOTIFY_MAPPROPERTY(fp, end) {
	this.type = fp.readShort();
};
PACKET.ZC.NOTIFY_MAPPROPERTY.size = 4;

// 0x19a
PACKET.ZC.NOTIFY_RANKING = function PACKET_ZC_NOTIFY_RANKING(fp, end) {
	this.AID = fp.readULong();
	this.ranking = fp.readLong();
	this.total = fp.readLong();
};
PACKET.ZC.NOTIFY_RANKING.size = 14;

// 0x19b
PACKET.ZC.NOTIFY_EFFECT = function PACKET_ZC_NOTIFY_EFFECT(fp, end) {
	this.AID = fp.readULong();
	this.effectID = fp.readLong();
};
PACKET.ZC.NOTIFY_EFFECT.size = 10;

// 0x19e
PACKET.ZC.START_CAPTURE = function PACKET_ZC_START_CAPTURE(fp, end) {};
PACKET.ZC.START_CAPTURE.size = 2;

// 0x1a0
PACKET.ZC.TRYCAPTURE_MONSTER = function PACKET_ZC_TRYCAPTURE_MONSTER(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.TRYCAPTURE_MONSTER.size = 3;

// 0x1a2
PACKET.ZC.PROPERTY_PET = function PACKET_ZC_PROPERTY_PET(fp, end) {
	this.szName = fp.readString(NAME_LENGTH);
	this.bModified = fp.readUChar();
	this.nLevel = fp.readShort();
	this.nFullness = fp.readShort();
	this.nRelationship = fp.readShort();
	this.ITID = fp.readUShort();
	this.job = fp.readShort();
};
PACKET.ZC.PROPERTY_PET.size = 37;

// 0x1a3
PACKET.ZC.FEED_PET = function PACKET_ZC_FEED_PET(fp, end) {
	this.cRet = fp.readChar();
	this.ITID = fp.readUShort();
};
PACKET.ZC.FEED_PET.size = 5;

// 0x1a4
PACKET.ZC.CHANGESTATE_PET = function PACKET_ZC_CHANGESTATE_PET(fp, end) {
	this.type = fp.readChar();
	this.GID = fp.readLong();
	this.data = fp.readLong();
};
PACKET.ZC.CHANGESTATE_PET.size = 11;

// 0x1a6
PACKET.ZC.PETEGG_LIST = function PACKET_ZC_PETEGG_LIST(fp, end) {
	this.eggList = (function () {
		const count = ((end - fp.tell()) / 2) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.PETEGG_LIST.size = -1;

// 0x1aa
PACKET.ZC.PET_ACT = function PACKET_ZC_PET_ACT(fp, end) {
	this.GID = fp.readLong();
	this.data = fp.readLong();
};
PACKET.ZC.PET_ACT.size = 10;

// 0x1ab
PACKET.ZC.PAR_CHANGE_USER = function PACKET_ZC_PAR_CHANGE_USER(fp, end) {
	this.AID = fp.readULong();
	this.varID = fp.readUShort();
	this.count = fp.readLong();
};
PACKET.ZC.PAR_CHANGE_USER.size = 12;

// 0x1ac
PACKET.ZC.SKILL_UPDATE = function PACKET_ZC_SKILL_UPDATE(fp, end) {
	this.AID = fp.readULong();
};
PACKET.ZC.SKILL_UPDATE.size = 6;

// 0x1ad
PACKET.ZC.MAKINGARROW_LIST = function PACKET_ZC_MAKINGARROW_LIST(fp, end) {
	const size = PACKETVER.value >= 20181121 ? 4 : 2;
	this.arrowList = (function (_size) {
		const count = ((end - fp.tell()) / _size) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = _size == 2 ? fp.readShort() : fp.readLong();
		}
		return out;
	})(size);
};
PACKET.ZC.MAKINGARROW_LIST.size = -1;

// 0x1b0
PACKET.ZC.NPCSPRITE_CHANGE = function PACKET_ZC_NPCSPRITE_CHANGE(fp, end) {
	this.GID = fp.readULong();
	this.type = fp.readUChar();
	this.value = fp.readULong();
};
PACKET.ZC.NPCSPRITE_CHANGE.size = 11;

// 0x1b1
PACKET.ZC.SHOWDIGIT = function PACKET_ZC_SHOWDIGIT(fp, end) {
	this.type = fp.readUChar();
	this.value = fp.readLong();
};
PACKET.ZC.SHOWDIGIT.size = 7;

// 0x1b3
PACKET.ZC.SHOW_IMAGE2 = function PACKET_ZC_SHOW_IMAGE2(fp, end) {
	this.imageName = fp.readBinaryString(64);
	this.type = fp.readUChar();
};
PACKET.ZC.SHOW_IMAGE2.size = 67;

// 0x1b4
PACKET.ZC.CHANGE_GUILD = function PACKET_ZC_CHANGE_GUILD(fp, end) {
	this.AID = fp.readULong();
	this.GDID = fp.readULong();
	this.emblemVersion = fp.readShort();
};
PACKET.ZC.CHANGE_GUILD.size = 12;

// 0x1b5
PACKET.SC.BILLING_INFO = function PACKET_SC_BILLING_INFO(fp, end) {
	this.dwAmountRemain = fp.readULong();
	this.dwQuantityRemain = fp.readULong();
	this.dwReserved1 = fp.readULong();
	this.dwReserved2 = fp.readULong();
};
PACKET.SC.BILLING_INFO.size = 18;

// 0x1b6
PACKET.ZC.GUILD_INFO2 = function PACKET_ZC_GUILD_INFO2(fp, end) {
	this.GDID = fp.readLong();
	this.level = fp.readLong();
	this.userNum = fp.readLong();
	this.maxUserNum = fp.readLong();
	this.userAverageLevel = fp.readLong();
	this.exp = fp.readLong();
	this.maxExp = fp.readLong();
	this.point = fp.readLong();
	this.honor = fp.readLong();
	this.virtue = fp.readLong();
	this.emblemVersion = fp.readLong();
	this.guildname = fp.readString(NAME_LENGTH);
	this.masterName = fp.readString(NAME_LENGTH);
	this.manageLand = fp.readBinaryString(16);
	this.zeny = fp.readLong();
};
PACKET.ZC.GUILD_INFO2.size = 114;

// 0x1b8
PACKET.ZC.GUILD_ZENY_ACK = function PACKET_ZC_GUILD_ZENY_ACK(fp, end) {
	this.ret = fp.readUChar();
};
PACKET.ZC.GUILD_ZENY_ACK.size = 3;

// 0x1b9
PACKET.ZC.DISPEL = function PACKET_ZC_DISPEL(fp, end) {
	this.AID = fp.readULong();
};
PACKET.ZC.DISPEL.size = 6;

// 0x1be
PACKET.AC.ASK_PNGAMEROOM = function PACKET_AC_ASK_PNGAMEROOM(fp, end) {};
PACKET.AC.ASK_PNGAMEROOM.size = 2;

// 0x1c1
PACKET.ZC.REPLY_REMAINTIME = function PACKET_ZC_REPLY_REMAINTIME(fp, end) {
	this.Result = fp.readLong();
	this.ExpirationDate = fp.readLong();
	this.RemainTime = fp.readLong();
};
PACKET.ZC.REPLY_REMAINTIME.size = 14;

// 0x1c2
PACKET.ZC.INFO_REMAINTIME = function PACKET_ZC_INFO_REMAINTIME(fp, end) {
	this.Type = fp.readLong();
	this.RemainTime = fp.readLong();
};
PACKET.ZC.INFO_REMAINTIME.size = 10;

// 0x1c3
PACKET.ZC.BROADCAST2 = function PACKET_ZC_BROADCAST2(fp, end) {
	this.fontColor = fp.readULong();
	this.fontType = fp.readShort();
	this.fontSize = fp.readShort();
	this.fontAlign = fp.readShort();
	this.fontY = fp.readShort();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.BROADCAST2.size = -1;

// 0x1c4
PACKET.ZC.ADD_ITEM_TO_STORE2 = function PACKET_ZC_ADD_ITEM_TO_STORE2(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readLong();
	this.ITID = fp.readUShort();
	this.type = fp.readUChar();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
};
PACKET.ZC.ADD_ITEM_TO_STORE2.size = 22;

// 0x1c5
PACKET.ZC.ADD_ITEM_TO_CART2 = function PACKET_ZC_ADD_ITEM_TO_CART2(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readLong();
	this.ITID = fp.readUShort();
	this.type = fp.readUChar();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
};
PACKET.ZC.ADD_ITEM_TO_CART2.size = 22;

// 0x1c7
PACKET.SC.ACK_ENCRYPTION = function PACKET_SC_ACK_ENCRYPTION(fp, end) {};
PACKET.SC.ACK_ENCRYPTION.size = 2;

// 0x1c8
PACKET.ZC.USE_ITEM_ACK2 = function PACKET_ZC_USE_ITEM_ACK2(fp, end) {
	this.index = fp.readUShort();
	this.id = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.AID = fp.readULong();
	this.count = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.USE_ITEM_ACK2.size = PACKETVER.value >= 20181121 ? 15 : 13;

// 0x1c9
PACKET.ZC.SKILL_ENTRY2 = function PACKET_ZC_SKILL_ENTRY2(fp, end) {
	this.AID = fp.readULong();
	this.creatorAID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.job = fp.readUChar();
	this.isVisible = fp.readUChar();
	this.isContens = fp.readUChar();
	this.msg = fp.readString(80);
};
PACKET.ZC.SKILL_ENTRY2.size = 97;

// 0x1cc
PACKET.ZC.MONSTER_TALK = function PACKET_ZC_MONSTER_TALK(fp, end) {
	this.GID = fp.readULong();
	this.stateId = fp.readUChar();
	this.skillId = fp.readUChar();
	this.arg1 = fp.readUChar();
};
PACKET.ZC.MONSTER_TALK.size = 9;

// 0x1cd
PACKET.ZC.AUTOSPELLLIST = function PACKET_ZC_AUTOSPELLLIST(fp, end) {
	this.SKID = (function () {
		const count = 7,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.AUTOSPELLLIST.size = 30;

// 0x1cf
PACKET.ZC.DEVOTIONLIST = function PACKET_ZC_DEVOTIONLIST(fp, end) {
	this.myAID = fp.readULong();
	this.AID = (function () {
		const count = 5,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readULong();
		}
		return out;
	})();
	this.range = fp.readShort();
};
PACKET.ZC.DEVOTIONLIST.size = 28;

// 0x1d0
PACKET.ZC.SPIRITS = function PACKET_ZC_SPIRITS(fp, end) {
	this.AID = fp.readULong();
	this.num = fp.readShort();
};
PACKET.ZC.SPIRITS.size = 8;

// 0x1d1
PACKET.ZC.BLADESTOP = function PACKET_ZC_BLADESTOP(fp, end) {
	this.srcAID = fp.readULong();
	this.destAID = fp.readULong();
	this.flag = fp.readLong();
};
PACKET.ZC.BLADESTOP.size = 14;

// 0x1d2
PACKET.ZC.COMBODELAY = function PACKET_ZC_COMBODELAY(fp, end) {
	this.AID = fp.readULong();
	this.delayTime = fp.readULong();
};
PACKET.ZC.COMBODELAY.size = 10;

// 0x1d3
PACKET.ZC.SOUND = function PACKET_ZC_SOUND(fp, end) {
	this.fileName = fp.readBinaryString(NAME_LENGTH);
	this.act = fp.readUChar();
	this.term = fp.readULong();
	this.NAID = fp.readULong();
};
PACKET.ZC.SOUND.size = 35;

// 0x1d4
PACKET.ZC.OPEN_EDITDLGSTR = function PACKET_ZC_OPEN_EDITDLGSTR(fp, end) {
	this.NAID = fp.readULong();
};
PACKET.ZC.OPEN_EDITDLGSTR.size = 6;

// 0x1d6
PACKET.ZC.NOTIFY_MAPPROPERTY2 = function PACKET_ZC_NOTIFY_MAPPROPERTY2(fp, end) {
	this.type = fp.readShort();
};
PACKET.ZC.NOTIFY_MAPPROPERTY2.size = 4;

// 0x1d7
// value2 seems to be used only when LOOK_WEAPON as a Shield
PACKET.ZC.SPRITE_CHANGE2 = function PACKET_ZC_SPRITE_CHANGE2(fp, end) {
	this.GID = fp.readULong();
	this.type = fp.readUChar();
	if (PACKETVER.value >= 20180704) {
		this.value = fp.readULong();
		this.value2 = fp.readULong();
	} else {
		this.value = fp.readShort();
		this.value2 = fp.readShort();
	}
};
PACKET.ZC.SPRITE_CHANGE2.size = PACKETVER.value >= 20180704 ? 15 : 11;

// 0x1d8
PACKET.ZC.NOTIFY_STANDENTRY2 = function PACKET_ZC_NOTIFY_STANDENTRY2(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readShort();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_STANDENTRY2.size = 54;

// 0x1d9
PACKET.ZC.NOTIFY_NEWENTRY2 = function PACKET_ZC_NOTIFY_NEWENTRY2(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readShort();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_NEWENTRY2.size = 53;

// 0x1da
PACKET.ZC.NOTIFY_MOVEENTRY2 = function PACKET_ZC_NOTIFY_MOVEENTRY2(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readShort();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.moveStartTime = fp.readULong();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readShort();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_MOVEENTRY2.size = 60;

// 0x1dc
PACKET.AC.ACK_HASH = function PACKET_AC_ACK_HASH(fp, end) {
	this.secret = fp.readBinaryString(end - fp.tell());
};
PACKET.AC.ACK_HASH.size = -1;

// 0x1de
PACKET.ZC.NOTIFY_SKILL2 = function PACKET_ZC_NOTIFY_SKILL2(fp, end) {
	this.SKID = fp.readUShort();
	this.AID = fp.readULong();
	this.targetID = fp.readULong();
	this.startTime = fp.readULong();
	this.attackMT = fp.readLong();
	this.attackedMT = fp.readLong();
	this.damage = fp.readLong();
	this.level = fp.readShort();
	this.count = fp.readShort();
	this.action = fp.readUChar();
};
PACKET.ZC.NOTIFY_SKILL2.size = 33;

// 0x1e0
PACKET.ZC.ACK_ACCOUNTNAME = function PACKET_ZC_ACK_ACCOUNTNAME(fp, end) {
	this.AID = fp.readULong();
	this.name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ACK_ACCOUNTNAME.size = 30;

// 0x1e1
PACKET.ZC.SPIRITS2 = function PACKET_ZC_SPIRITS2(fp, end) {
	this.AID = fp.readULong();
	this.num = fp.readShort();
};
PACKET.ZC.SPIRITS2.size = 8;

// 0x1e2
PACKET.ZC.REQ_COUPLE = function PACKET_ZC_REQ_COUPLE(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.REQ_COUPLE.size = 34;

// 0x1e4
PACKET.ZC.START_COUPLE = function PACKET_ZC_START_COUPLE(fp, end) {};
PACKET.ZC.START_COUPLE.size = 2;

// 0x1e6
PACKET.ZC.COUPLENAME = function PACKET_ZC_COUPLENAME(fp, end) {
	this.CoupleName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.COUPLENAME.size = 26;

// 0x1e9
PACKET.ZC.ADD_MEMBER_TO_GROUP2 = function PACKET_ZC_ADD_MEMBER_TO_GROUP2(fp, end) {
	this.AID = fp.readULong();
	this.role = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.state = fp.readUChar();
	this.groupName = fp.readString(NAME_LENGTH);
	this.characterName = fp.readString(NAME_LENGTH);
	this.mapName = fp.readBinaryString(16);
	this.ItemPickupRule = fp.readUChar();
	this.ItemDivisionRule = fp.readUChar();
};
PACKET.ZC.ADD_MEMBER_TO_GROUP2.size = 81;

// 0x1ea
PACKET.ZC.CONGRATULATION = function PACKET_ZC_CONGRATULATION(fp, end) {
	this.AID = fp.readULong();
};
PACKET.ZC.CONGRATULATION.size = 6;

// 0x1eb
PACKET.ZC.NOTIFY_POSITION_TO_GUILDM = function PACKET_ZC_NOTIFY_POSITION_TO_GUILDM(fp, end) {
	this.AID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
};
PACKET.ZC.NOTIFY_POSITION_TO_GUILDM.size = 10;

// 0x1ec
PACKET.ZC.GUILD_MEMBER_MAP_CHANGE = function PACKET_ZC_GUILD_MEMBER_MAP_CHANGE(fp, end) {
	this.GDID = fp.readULong();
	this.AID = fp.readULong();
	this.mapName = fp.readBinaryString(16);
};
PACKET.ZC.GUILD_MEMBER_MAP_CHANGE.size = 26;

// 0x1ee
PACKET.ZC.NORMAL_ITEMLIST2 = function PACKET_ZC_NORMAL_ITEMLIST2(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 18) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.NORMAL_ITEMLIST2.size = -1;

// 0x1ef
PACKET.ZC.CART_NORMAL_ITEMLIST2 = function PACKET_ZC_CART_NORMAL_ITEMLIST2(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 18) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.CART_NORMAL_ITEMLIST2.size = -1;

// 0x1f0
PACKET.ZC.STORE_NORMAL_ITEMLIST2 = function PACKET_ZC_STORE_NORMAL_ITEMLIST2(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 18) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.STORE_NORMAL_ITEMLIST2.size = -1;

// 0x1f1
PACKET.AC.NOTIFY_ERROR = function PACKET_AC_NOTIFY_ERROR(fp, end) {
	this.msg = fp.readString(end - fp.tell());
};
PACKET.AC.NOTIFY_ERROR.size = -1;

// 0x1f2
PACKET.ZC.UPDATE_CHARSTAT2 = function PACKET_ZC_UPDATE_CHARSTAT2(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.status = fp.readLong();
	this.sex = fp.readShort();
	this.head = fp.readShort();
	this.headPalette = fp.readShort();
};
PACKET.ZC.UPDATE_CHARSTAT2.size = 20;

// 0x1f3
PACKET.ZC.NOTIFY_EFFECT2 = function PACKET_ZC_NOTIFY_EFFECT2(fp, end) {
	this.AID = fp.readULong();
	this.effectID = fp.readLong();
};
PACKET.ZC.NOTIFY_EFFECT2.size = 10;

// 0x1f4
PACKET.ZC.REQ_EXCHANGE_ITEM2 = function PACKET_ZC_REQ_EXCHANGE_ITEM2(fp, end) {
	this.name = fp.readString(NAME_LENGTH);
	this.GID = fp.readULong();
	this.level = fp.readShort();
};
PACKET.ZC.REQ_EXCHANGE_ITEM2.size = 32;

// 0x1f5
PACKET.ZC.ACK_EXCHANGE_ITEM2 = function PACKET_ZC_ACK_EXCHANGE_ITEM2(fp, end) {
	this.result = fp.readUChar();
	this.GID = fp.readULong();
	this.level = fp.readShort();
};
PACKET.ZC.ACK_EXCHANGE_ITEM2.size = 9;

// 0x1f6
PACKET.ZC.REQ_BABY = function PACKET_ZC_REQ_BABY(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.REQ_BABY.size = 34;

// 0x1f8
PACKET.ZC.START_BABY = function PACKET_ZC_START_BABY(fp, end) {};
PACKET.ZC.START_BABY.size = 2;

// 0x1fc
PACKET.ZC.REPAIRITEMLIST = function PACKET_ZC_REPAIRITEMLIST(fp, end) {
	this.itemList = (function () {
		const count = ((end - fp.tell()) / 13) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.REPAIRITEMLIST.size = -1;

// 0x1fe
PACKET.ZC.ACK_ITEMREPAIR = function PACKET_ZC_ACK_ITEMREPAIR(fp, end) {
	this.index = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_ITEMREPAIR.size = 5;

// 0x1ff
PACKET.ZC.HIGHJUMP = function PACKET_ZC_HIGHJUMP(fp, end) {
	this.AID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
};
PACKET.ZC.HIGHJUMP.size = 10;

// 0x201
PACKET.ZC.FRIENDS_LIST = function PACKET_ZC_FRIENDS_LIST(fp, end) {
	this.friendList = (function () {
		const count = ((end - fp.tell()) / 32) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AID = fp.readULong();
			out[i].GID = fp.readULong();
			out[i].Name = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
};
PACKET.ZC.FRIENDS_LIST.size = -1;

// 0x205
PACKET.ZC.DIVORCE = function PACKET_ZC_DIVORCE(fp, end) {
	this.name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.DIVORCE.size = 26;

// 0x206
PACKET.ZC.FRIENDS_STATE = function PACKET_ZC_FRIENDS_STATE(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.State = fp.readUChar();
};
PACKET.ZC.FRIENDS_STATE.size = 11;

// 0x207
PACKET.ZC.REQ_ADD_FRIENDS = function PACKET_ZC_REQ_ADD_FRIENDS(fp, end) {
	this.ReqAID = fp.readULong();
	this.ReqGID = fp.readULong();
	this.Name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.REQ_ADD_FRIENDS.size = 34;

// 0x209
PACKET.ZC.ADD_FRIENDS_LIST = function PACKET_ZC_ADD_FRIENDS_LIST(fp, end) {
	this.Result = fp.readShort();
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.Name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ADD_FRIENDS_LIST.size = 36;

// 0x20a
PACKET.ZC.DELETE_FRIENDS = function PACKET_ZC_DELETE_FRIENDS(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
};
PACKET.ZC.DELETE_FRIENDS.size = 10;

// 0x20d
PACKET.HC.BLOCK_CHARACTER = function PACKET_HC_BLOCK_CHARACTER(fp, end) {
	this.characterList = (function () {
		const count = ((end - fp.tell()) / 24) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].GID = fp.readULong();
			out[i].szExpireDate = fp.readBinaryString(20);
		}
		return out;
	})();
};
PACKET.HC.BLOCK_CHARACTER.size = -1;

// 0x20e
PACKET.ZC.STARSKILL = function PACKET_ZC_STARSKILL(fp, end) {
	this.monsterName = fp.readBinaryString(NAME_LENGTH);
	this.monsterID = fp.readLong();
	this.star = fp.readUChar();
	this.result = fp.readUChar();
};
PACKET.ZC.STARSKILL.size = 32;

// 0x210
PACKET.ZC.ACK_PVPPOINT = function PACKET_ZC_ACK_PVPPOINT(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.PVP = {};
	this.PVP.WinPoint = fp.readLong();
	this.PVP.LosePoint = fp.readLong();
	this.PVP.Point = fp.readLong();
};
PACKET.ZC.ACK_PVPPOINT.size = 22;

// 0x211
PACKET.ZH.MOVE_PVPWORLD = function PACKET_ZH_MOVE_PVPWORLD(fp, end) {
	this.GID = fp.readULong();
};
PACKET.ZH.MOVE_PVPWORLD.size = 6;

// 0x214
PACKET.ZC.ACK_STATUS_GM = function PACKET_ZC_ACK_STATUS_GM(fp, end) {
	this.str = fp.readUChar();
	this.standardStr = fp.readUChar();
	this.agi = fp.readUChar();
	this.standardAgi = fp.readUChar();
	this.vit = fp.readUChar();
	this.standardVit = fp.readUChar();
	this.Int = fp.readUChar();
	this.standardInt = fp.readUChar();
	this.dex = fp.readUChar();
	this.standardDex = fp.readUChar();
	this.luk = fp.readUChar();
	this.standardLuk = fp.readUChar();
	this.attPower = fp.readShort();
	this.refiningPower = fp.readShort();
	this.max_mattPower = fp.readShort();
	this.min_mattPower = fp.readShort();
	this.itemdefPower = fp.readShort();
	this.plusdefPower = fp.readShort();
	this.mdefPower = fp.readShort();
	this.plusmdefPower = fp.readShort();
	this.hitSuccessValue = fp.readShort();
	this.avoidSuccessValue = fp.readShort();
	this.plusAvoidSuccessValue = fp.readShort();
	this.criticalSuccessValue = fp.readShort();
	this.ASPD = fp.readShort();
	this.plusASPD = fp.readShort();
};
PACKET.ZC.ACK_STATUS_GM.size = 42;

// 0x215
PACKET.ZC.SKILLMSG = function PACKET_ZC_SKILLMSG(fp, end) {
	this.MsgNo = fp.readLong();
};
PACKET.ZC.SKILLMSG.size = 6;

// 0x216
PACKET.ZC.BABYMSG = function PACKET_ZC_BABYMSG(fp, end) {
	this.MsgNo = fp.readLong();
};
PACKET.ZC.BABYMSG.size = 6;

// 0x219
PACKET.ZC.BLACKSMITH_RANK = function PACKET_ZC_BLACKSMITH_RANK(fp, end) {
	this.Name = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
	this.Point = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.BLACKSMITH_RANK.size = 282;

// 0x21a
PACKET.ZC.ALCHEMIST_RANK = function PACKET_ZC_ALCHEMIST_RANK(fp, end) {
	this.Name = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
	this.Point = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.ALCHEMIST_RANK.size = 282;

// 0x21b
PACKET.ZC.BLACKSMITH_POINT = function PACKET_ZC_BLACKSMITH_POINT(fp, end) {
	this.Point = fp.readLong();
	this.TotalPoint = fp.readLong();
};
PACKET.ZC.BLACKSMITH_POINT.size = 10;

// 0x21c
PACKET.ZC.ALCHEMIST_POINT = function PACKET_ZC_ALCHEMIST_POINT(fp, end) {
	this.Point = fp.readLong();
	this.TotalPoint = fp.readLong();
};
PACKET.ZC.ALCHEMIST_POINT.size = 10;

// 0x21e
PACKET.ZC.LESSEFFECT = function PACKET_ZC_LESSEFFECT(fp, end) {
	this.isLess = fp.readLong();
};
PACKET.ZC.LESSEFFECT.size = 6;

// 0x21f
PACKET.ZC.NOTIFY_PKINFO = function PACKET_ZC_NOTIFY_PKINFO(fp, end) {
	this.winPoint = fp.readLong();
	this.losePoint = fp.readLong();
	this.killName = fp.readString(NAME_LENGTH);
	this.killedName = fp.readString(NAME_LENGTH);
	this.expireTime = {};
	this.expireTime.dwLowDateTime = fp.readULong();
	this.expireTime.dwHighDateTime = fp.readULong();
};
PACKET.ZC.NOTIFY_PKINFO.size = 66;

// 0x220
PACKET.ZC.NOTIFY_CRAZYKILLER = function PACKET_ZC_NOTIFY_CRAZYKILLER(fp, end) {
	this.AID = fp.readULong();
	this.isCrazyKiller = fp.readLong();
};
PACKET.ZC.NOTIFY_CRAZYKILLER.size = 10;

// 0x221
PACKET.ZC.NOTIFY_WEAPONITEMLIST = function PACKET_ZC_NOTIFY_WEAPONITEMLIST(fp, end) {
	this.itemList = (function () {
		const count = ((end - fp.tell()) / 13) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.NOTIFY_WEAPONITEMLIST.size = -1;

// 0x223
PACKET.ZC.ACK_WEAPONREFINE = function PACKET_ZC_ACK_WEAPONREFINE(fp, end) {
	this.msg = fp.readLong();
	this.ITID = fp.readUShort();
};
PACKET.ZC.ACK_WEAPONREFINE.size = 8;

// 0x224
PACKET.ZC.TAEKWON_POINT = function PACKET_ZC_TAEKWON_POINT(fp, end) {
	this.Point = fp.readLong();
	this.TotalPoint = fp.readLong();
};
PACKET.ZC.TAEKWON_POINT.size = 10;

// 0x226
PACKET.ZC.TAEKWON_RANK = function PACKET_ZC_TAEKWON_RANK(fp, end) {
	this.Name = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
	this.Point = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.TAEKWON_RANK.size = 282;

// 0x227
PACKET.ZC.GAME_GUARD = function PACKET_ZC_GAME_GUARD(fp, end) {
	this.AuthData = (function () {
		const count = 4,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.GAME_GUARD.size = 18;

// 0x229
PACKET.ZC.STATE_CHANGE3 = function PACKET_ZC_STATE_CHANGE3(fp, end) {
	this.AID = fp.readULong();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.isPKModeON = fp.readUChar();
};
PACKET.ZC.STATE_CHANGE3.size = 15;

// 0x22a
PACKET.ZC.NOTIFY_STANDENTRY3 = function PACKET_ZC_NOTIFY_STANDENTRY3(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_STANDENTRY3.size = 58;

// 0x22b
PACKET.ZC.NOTIFY_NEWENTRY3 = function PACKET_ZC_NOTIFY_NEWENTRY3(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_NEWENTRY3.size = 57;

// 0x22c
PACKET.ZC.NOTIFY_MOVEENTRY3 = function PACKET_ZC_NOTIFY_MOVEENTRY3(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.moveStartTime = fp.readULong();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
};
PACKET.ZC.NOTIFY_MOVEENTRY3.size = 64;

// 0x22e
PACKET.ZC.PROPERTY_HOMUN = function PACKET_ZC_PROPERTY_HOMUN(fp, end) {
	this.szName = fp.readString(NAME_LENGTH);
	this.bModified = fp.readUChar();
	this.nLevel = fp.readShort();
	this.nFullness = fp.readShort();
	this.nRelationship = fp.readShort();
	this.ITID = fp.readUShort();
	this.atk = fp.readShort();
	this.Matk = fp.readShort();
	this.hit = fp.readShort();
	this.critical = fp.readShort();
	this.def = fp.readShort();
	this.Mdef = fp.readShort();
	this.flee = fp.readShort();
	this.aspd = fp.readShort();
	this.hp = fp.readShort();
	this.maxHP = fp.readShort();
	this.sp = fp.readShort();
	this.maxSP = fp.readShort();
	this.exp = fp.readLong();
	this.maxEXP = fp.readLong();
	this.SKPoint = fp.readShort();
	this.ATKRange = fp.readShort();
};
PACKET.ZC.PROPERTY_HOMUN.size = 71;

//0x22f
PACKET.ZC.FEED_MER = function PACKET_ZC_FEED_MER(fp, end) {
	this.cRet = fp.readChar();
	this.ITID = fp.readUShort();
};
PACKET.ZC.FEED_MER.size = 5;

// 0x230
PACKET.ZC.CHANGESTATE_MER = function PACKET_ZC_CHANGESTATE_MER(fp, end) {
	this.type = fp.readChar();
	this.state = fp.readChar();
	this.GID = fp.readLong();
	this.data = fp.readLong();
};
PACKET.ZC.CHANGESTATE_MER.size = 12;

// 0x23a
PACKET.ZC.REQ_STORE_PASSWORD = function PACKET_ZC_REQ_STORE_PASSWORD(fp, end) {
	this.Info = fp.readShort();
};
PACKET.ZC.REQ_STORE_PASSWORD.size = 4;

// 0x23c
PACKET.ZC.RESULT_STORE_PASSWORD = function PACKET_ZC_RESULT_STORE_PASSWORD(fp, end) {
	this.Result = fp.readShort();
	this.ErrorCount = fp.readShort();
};
PACKET.ZC.RESULT_STORE_PASSWORD.size = 6;

// 0x23d
PACKET.AC.EVENT_RESULT = function PACKET_AC_EVENT_RESULT(fp, end) {
	this.EventItemCount = fp.readULong();
};
PACKET.AC.EVENT_RESULT.size = 6;

// 0x23e
PACKET.HC.REQUEST_CHARACTER_PASSWORD = function PACKET_HC_REQUEST_CHARACTER_PASSWORD(fp, end) {
	this.Result = fp.readShort();
	this.dummyValue = fp.readULong();
};
PACKET.HC.REQUEST_CHARACTER_PASSWORD.size = 8;

// 0x240
PACKET.ZC.MAIL_REQ_GET_LIST = function PACKET_ZC_MAIL_REQ_GET_LIST(fp, end) {
	this.MailNumber = fp.readLong();
	this.mailList = (function () {
		const count = ((end - fp.tell()) / 73) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].MailID = fp.readULong();
			out[i].HEADER = fp.readString(40);
			out[i].isOpen = fp.readChar();
			out[i].FromName = fp.readString(NAME_LENGTH);
			out[i].DeleteTime = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.MAIL_REQ_GET_LIST.size = -1;

// 0x242
PACKET.ZC.MAIL_REQ_OPEN = function PACKET_ZC_MAIL_REQ_OPEN(fp, end) {
	this.MailID = fp.readLong();
	this.Header = fp.readString(40);
	this.FromName = fp.readString(NAME_LENGTH);
	this.DeleteTime = fp.readLong();
	this.Money = fp.readULong();
	this.count = fp.readLong();
	this.ITID = fp.readUShort();
	this.Type = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
	this.msg_len = fp.readUChar();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.MAIL_REQ_OPEN.size = -1;

// 0x245
PACKET.ZC.MAIL_REQ_GET_ITEM = function PACKET_ZC_MAIL_REQ_GET_ITEM(fp, end) {
	this.Result = fp.readChar();
};
PACKET.ZC.MAIL_REQ_GET_ITEM.size = 3;

// 0x249
PACKET.ZC.MAIL_REQ_SEND = function PACKET_ZC_MAIL_REQ_SEND(fp, end) {
	this.Result = fp.readChar();
};
PACKET.ZC.MAIL_REQ_SEND.size = 3;

// 0x24a
PACKET.ZC.MAIL_RECEIVE = function PACKET_ZC_MAIL_RECEIVE(fp, end) {
	this.MailID = fp.readULong();
	this.Header = fp.readString(40);
	this.FromName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.MAIL_RECEIVE.size = 70;

// 0x250
PACKET.ZC.AUCTION_RESULT = function PACKET_ZC_AUCTION_RESULT(fp, end) {
	this.Result = fp.readChar();
};
PACKET.ZC.AUCTION_RESULT.size = 3;

// 0x252
PACKET.ZC.AUCTION_ITEM_REQ_SEARCH = function PACKET_ZC_AUCTION_ITEM_REQ_SEARCH(fp, end) {
	this.MaxPage = fp.readLong();
	this.Number = fp.readLong();
	this.auctionItemList = (function () {
		const count = ((end - fp.tell()) / 83) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AuctionID = fp.readULong();
			out[i].SellerName = fp.readString(NAME_LENGTH);
			out[i].ITID = fp.readUShort();
			out[i].Type = fp.readLong();
			out[i].count = fp.readShort();
			out[i].IsIdentified = fp.readUChar();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].NowPrice = fp.readLong();
			out[i].MaxPrice = fp.readLong();
			out[i].BuyerName = fp.readString(NAME_LENGTH);
			out[i].DeleteTime = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.AUCTION_ITEM_REQ_SEARCH.size = -1;

// 0x253
PACKET.ZC.STARPLACE = function PACKET_ZC_STARPLACE(fp, end) {
	this.which = fp.readChar();
};
PACKET.ZC.STARPLACE.size = 3;

// 0x255
PACKET.ZC.ACK_MAIL_ADD_ITEM = function PACKET_ZC_ACK_MAIL_ADD_ITEM(fp, end) {
	this.Index = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_MAIL_ADD_ITEM.size = 5;

// 0x256
PACKET.ZC.ACK_AUCTION_ADD_ITEM = function PACKET_ZC_ACK_AUCTION_ADD_ITEM(fp, end) {
	this.Index = fp.readShort();
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_AUCTION_ADD_ITEM.size = 5;

// 0x257
PACKET.ZC.ACK_MAIL_DELETE = function PACKET_ZC_ACK_MAIL_DELETE(fp, end) {
	this.MailID = fp.readLong();
	this.Result = fp.readUShort();
};
PACKET.ZC.ACK_MAIL_DELETE.size = 8;

// 0x259
PACKET.AC.ACK_GAME_GUARD = function PACKET_AC_ACK_GAME_GUARD(fp, end) {
	this.ucAnswer = fp.readUChar();
};
PACKET.AC.ACK_GAME_GUARD.size = 3;

// 0x25a
PACKET.ZC.MAKINGITEM_LIST = function PACKET_ZC_MAKINGITEM_LIST(fp, end) {
	if (PACKETVER.value >= 20211103) {
		this.makeItem = fp.readShort();
		const size = PACKETVER.value >= 20181121 ? 4 : 2;
		this.items = (function (_size) {
			const count = ((end - fp.tell()) / _size) | 0,
				out = new Array(count);
			for (let i = 0; i < count; ++i) {
				out[i] = {};
				out[i].itemId = _size == 4 ? fp.readULong() : fp.readUShort();
			}
			return out;
		})(size);
	} else {
		const size = PACKETVER.value >= 20181121 ? 4 : 2;
		this.idList = (function (_size) {
			const count = ((end - fp.tell()) / _size) | 0,
				out = new Array(count);
			for (let i = 0; i < count; ++i) {
				out[i] = _size == 4 ? fp.readULong() : fp.readUShort();
			}
			return out;
		})(size);
	}
};
PACKET.ZC.MAKINGITEM_LIST.size = -1;

// 0x25e
PACKET.ZC.AUCTION_ACK_MY_SELL_STOP = function PACKET_ZC_AUCTION_ACK_MY_SELL_STOP(fp, end) {
	this.Result = fp.readShort();
};
PACKET.ZC.AUCTION_ACK_MY_SELL_STOP.size = 4;

// 0x25f
PACKET.ZC.AUCTION_WINDOWS = function PACKET_ZC_AUCTION_WINDOWS(fp, end) {
	this.Type = fp.readLong();
};
PACKET.ZC.AUCTION_WINDOWS.size = 6;

// 0x260
PACKET.ZC.MAIL_WINDOWS = function PACKET_ZC_MAIL_WINDOWS(fp, end) {
	this.Type = fp.readLong();
};
PACKET.ZC.MAIL_WINDOWS.size = 6;

// 0x261
PACKET.AC.REQ_LOGIN_OLDEKEY = function PACKET_AC_REQ_LOGIN_OLDEKEY(fp, end) {
	this.m_SeedValue = fp.readBinaryString(9);
};
PACKET.AC.REQ_LOGIN_OLDEKEY.size = 11;

// 0x262
PACKET.AC.REQ_LOGIN_NEWEKEY = function PACKET_AC_REQ_LOGIN_NEWEKEY(fp, end) {
	this.m_SeedValue = fp.readBinaryString(9);
};
PACKET.AC.REQ_LOGIN_NEWEKEY.size = 11;

// 0x263
PACKET.AC.REQ_LOGIN_CARDPASS = function PACKET_AC_REQ_LOGIN_CARDPASS(fp, end) {
	this.m_SeedValue = fp.readBinaryString(9);
};
PACKET.AC.REQ_LOGIN_CARDPASS.size = 11;

// 0x267
PACKET.AC.ACK_EKEY_FAIL_NOTEXIST = function PACKET_AC_ACK_EKEY_FAIL_NOTEXIST(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.AC.ACK_EKEY_FAIL_NOTEXIST.size = 4;

// 0x268
PACKET.AC.ACK_EKEY_FAIL_NOTUSESEKEY = function PACKET_AC_ACK_EKEY_FAIL_NOTUSESEKEY(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.AC.ACK_EKEY_FAIL_NOTUSESEKEY.size = 4;

// 0x269
PACKET.AC.ACK_EKEY_FAIL_NOTUSEDEKEY = function PACKET_AC_ACK_EKEY_FAIL_NOTUSEDEKEY(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.AC.ACK_EKEY_FAIL_NOTUSEDEKEY.size = 4;

// 0x26a
PACKET.AC.ACK_EKEY_FAIL_AUTHREFUSE = function PACKET_AC_ACK_EKEY_FAIL_AUTHREFUSE(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.AC.ACK_EKEY_FAIL_AUTHREFUSE.size = 4;

// 0x26b
PACKET.AC.ACK_EKEY_FAIL_INPUTEKEY = function PACKET_AC_ACK_EKEY_FAIL_INPUTEKEY(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.AC.ACK_EKEY_FAIL_INPUTEKEY.size = 4;

// 0x26c
PACKET.AC.ACK_EKEY_FAIL_NOTICE = function PACKET_AC_ACK_EKEY_FAIL_NOTICE(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.AC.ACK_EKEY_FAIL_NOTICE.size = 4;

// 0x26d
PACKET.AC.ACK_EKEY_FAIL_NEEDCARDPASS = function PACKET_AC_ACK_EKEY_FAIL_NEEDCARDPASS(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.AC.ACK_EKEY_FAIL_NEEDCARDPASS.size = 4;

// 0x26e
PACKET.AC.ACK_AUTHEKEY_FAIL_NOTMATCHCARDPASS = function PACKET_AC_ACK_AUTHEKEY_FAIL_NOTMATCHCARDPASS(fp, end) {
	this.errorCode = fp.readShort();
};
PACKET.AC.ACK_AUTHEKEY_FAIL_NOTMATCHCARDPASS.size = 4;

// 0x26f
PACKET.AC.ACK_FIRST_LOGIN = function PACKET_AC_ACK_FIRST_LOGIN(fp, end) {};
PACKET.AC.ACK_FIRST_LOGIN.size = 2;

// 0x270
PACKET.AC.REQ_LOGIN_ACCOUNT_INFO = function PACKET_AC_REQ_LOGIN_ACCOUNT_INFO(fp, end) {};
PACKET.AC.REQ_LOGIN_ACCOUNT_INFO.size = 2;

// 0x272
PACKET.AC.ACK_PT_ID_INFO = function PACKET_AC_ACK_PT_ID_INFO(fp, end) {
	this.szPTID = fp.readBinaryString(21);
	this.szPTNumID = fp.readBinaryString(21);
};
PACKET.AC.ACK_PT_ID_INFO.size = 44;

// 0x274
PACKET.ZC.ACK_MAIL_RETURN = function PACKET_ZC_ACK_MAIL_RETURN(fp, end) {
	this.MailID = fp.readLong();
	this.Result = fp.readShort();
};
PACKET.ZC.ACK_MAIL_RETURN.size = 8;

// 0x276
PACKET.AC.ACCEPT_LOGIN2 = function PACKET_AC_ACCEPT_LOGIN2(fp, end) {
	this.AuthCode = fp.readLong();
	this.AID = fp.readULong();
	this.userLevel = fp.readULong();
	this.lastLoginIP = fp.readULong();
	this.lastLoginTime = fp.readBinaryString(26);
	this.Sex = fp.readUChar();
	this.iAccountSID = fp.readLong();
};
PACKET.AC.ACCEPT_LOGIN2.size = 51;

// 0x278
PACKET.ZC.NOTIFY_PCBANG = function PACKET_ZC_NOTIFY_PCBANG(fp, end) {};
PACKET.ZC.NOTIFY_PCBANG.size = 2;

// 0x27a
PACKET.ZC.HUNTINGLIST = function PACKET_ZC_HUNTINGLIST(fp, end) {
	this.HuntingList = (function () {
		const count = ((end - fp.tell()) / 12) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].mobGID = fp.readULong();
			out[i].maxCount = fp.readShort();
			out[i].count = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.HUNTINGLIST.size = -1;

// 0x27b
PACKET.ZC.PCBANG_EFFECT = function PACKET_ZC_PCBANG_EFFECT(fp, end) {
	this.ExpFactor = fp.readLong();
	this.ExpFactor2 = fp.readLong();
	this.DropFactor = fp.readLong();
};
PACKET.ZC.PCBANG_EFFECT.size = 14;

// 0x27d
PACKET.ZC.PROPERTY_MERCE = function PACKET_ZC_PROPERTY_MERCE(fp, end) {
	this.name = fp.readString(NAME_LENGTH);
	this.level = fp.readShort();
	this.faith = fp.readShort();
	this.summonCount = fp.readShort();
	this.atk = fp.readShort();
	this.Matk = fp.readShort();
	this.hit = fp.readShort();
	this.critical = fp.readShort();
	this.def = fp.readShort();
	this.Mdef = fp.readShort();
	this.flee = fp.readShort();
	this.aspd = fp.readShort();
	this.hp = fp.readShort();
	this.maxHP = fp.readShort();
	this.sp = fp.readShort();
	this.maxSP = fp.readShort();
	this.ATKRange = fp.readShort();
	this.exp = fp.readLong();
};
PACKET.ZC.PROPERTY_MERCE.size = 62;

// 0x27e
PACKET.ZC.SHANDA_PROTECT = function PACKET_ZC_SHANDA_PROTECT(fp, end) {
	this.CodeLen = fp.readShort();
	this.Code = fp.readBinaryString(end - fp.tell());
};
PACKET.ZC.SHANDA_PROTECT.size = -1;

// 0x280
PACKET.ZC.GANGSI_POINT = function PACKET_ZC_GANGSI_POINT(fp, end) {
	this.Point = fp.readLong();
	this.TotalPoint = fp.readLong();
	this.PacketSwitch = fp.readShort();
};
PACKET.ZC.GANGSI_POINT.size = 12;

// 0x282
PACKET.ZC.GANGSI_RANK = function PACKET_ZC_GANGSI_RANK(fp, end) {
	this.Name = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
	this.Point = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
	this.PacketSwitch = fp.readShort();
};
PACKET.ZC.GANGSI_RANK.size = 284;

// 0x283
PACKET.ZC.AID = function PACKET_ZC_AID(fp, end) {
	this.AID = fp.readULong();
};
PACKET.ZC.AID.size = 6;

// 0x284
PACKET.ZC.NOTIFY_EFFECT3 = function PACKET_ZC_NOTIFY_EFFECT3(fp, end) {
	this.AID = fp.readULong();
	this.effectID = fp.readLong();
	this.numdata = fp.readLong();
};
PACKET.ZC.NOTIFY_EFFECT3.size = 14;

// 0x285
PACKET.ZC.DEATH_QUESTION = function PACKET_ZC_DEATH_QUESTION(fp, end) {
	this.Qcategory = fp.readShort();
	this.Qnum = fp.readShort();
};
PACKET.ZC.DEATH_QUESTION.size = 6;

/// 0287
PACKET.ZC.PC_CASH_POINT_ITEMLIST = function PACKET_ZC_PC_CASH_POINT_ITEMLIST(fp, end) {
	this.KafraPoint = fp.readULong();
	this.CashPoint = fp.readULong();
	this.itemList = (function () {
		// base: price(4) + discountprice(4) + type(1) + ITID(2|4)
		// ext:  base + viewSprite(2) + location(4) + unused(1)
		const base = PACKETVER.value >= 20181121 ? 13 : 11;
		const ext = base + 7;
		const itemListLen = end - fp.tell();
		const itemLen = itemListLen % base !== 0 && itemListLen % ext === 0 ? ext : base;
		const out = [];
		while (fp.tell() + itemLen <= end) {
			const item = {};
			item.price = fp.readLong();
			item.discountprice = fp.readLong();
			item.type = fp.readUChar();
			item.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			if (itemLen === ext) {
				item.viewSprite = fp.readUShort();
				item.location = fp.readLong();
				item.unused = fp.readUChar();
			}
			out.push(item);
		}
		return out;
	})();
};
PACKET.ZC.PC_CASH_POINT_ITEMLIST.size = -1;

// 0x289
PACKET.ZC.PC_CASH_POINT_UPDATE = function PACKET_ZC_PC_CASH_POINT_UPDATE(fp, end) {
	this.KafraPoint = fp.readULong();
	this.CashPoint = fp.readULong();
	this.Error = fp.readShort();
};
PACKET.ZC.PC_CASH_POINT_UPDATE.size = 12;

// 0x28a
PACKET.ZC.NPC_SHOWEFST_UPDATE = function PACKET_ZC_NPC_SHOWEFST_UPDATE(fp, end) {
	this.AID = fp.readULong();
	this.effectState = fp.readLong();
	this.clevel = fp.readLong();
	this.showEFST = fp.readLong();
};
PACKET.ZC.NPC_SHOWEFST_UPDATE.size = 18;

// 0x28e
PACKET.HC.ACK_IS_VALID_CHARNAME = function PACKET_HC_ACK_IS_VALID_CHARNAME(fp, end) {
	this.sResult = fp.readShort();
};
PACKET.HC.ACK_IS_VALID_CHARNAME.size = 4;

// 0x290
PACKET.HC.ACK_CHANGE_CHARNAME = function PACKET_HC_ACK_CHANGE_CHARNAME(fp, end) {
	this.sResult = fp.readShort();
};
PACKET.HC.ACK_CHANGE_CHARNAME.size = 4;

// 0x291
PACKET.ZC.MSG = function PACKET_ZC_MSG(fp, end) {
	this.msg = fp.readUShort();
};
PACKET.ZC.MSG.size = 4;

// 0x9cd
PACKET.ZC.MSG_COLOR = function PACKET_ZC_MSG_COLOR(fp, end) {
	this.msg = fp.readUShort();
	this.color = fp.readUShort();
};
PACKET.ZC.MSG_COLOR.size = 8;

// 0x293
PACKET.ZC.BOSS_INFO = function PACKET_ZC_BOSS_INFO(fp, end) {
	this.infoType = fp.readUChar();
	this.xPos = fp.readLong();
	this.yPos = fp.readLong();
	this.minHour = fp.readUShort();
	this.minMinute = fp.readUShort();
	this.maxHour = fp.readUShort();
	this.maxMinute = fp.readUShort();
	this.name = fp.readString(51);
};
PACKET.ZC.BOSS_INFO.size = 70;

// 0x294
PACKET.ZC.READ_BOOK = function PACKET_ZC_READ_BOOK(fp, end) {
	this.bookID = fp.readULong();
	this.page = fp.readULong();
};
PACKET.ZC.READ_BOOK.size = 10;

// 0x295
PACKET.ZC.EQUIPMENT_ITEMLIST2 = function PACKET_ZC_EQUIPMENT_ITEMLIST2(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 24) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.EQUIPMENT_ITEMLIST2.size = -1;

// 0x296
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST2 = function PACKET_ZC_STORE_EQUIPMENT_ITEMLIST2(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 24) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST2.size = -1;

// 0x297
PACKET.ZC.CART_EQUIPMENT_ITEMLIST2 = function PACKET_ZC_CART_EQUIPMENT_ITEMLIST2(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 24) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.CART_EQUIPMENT_ITEMLIST2.size = -1;

// 0x298
PACKET.ZC.CASH_TIME_COUNTER = function PACKET_ZC_CASH_TIME_COUNTER(fp, end) {
	this.ITID = fp.readUShort();
	this.RemainSecond = fp.readULong();
};
PACKET.ZC.CASH_TIME_COUNTER.size = 8;

// 0x299
PACKET.ZC.CASH_ITEM_DELETE = function PACKET_ZC_CASH_ITEM_DELETE(fp, end) {
	this.index = fp.readShort();
	this.ITID = fp.readUShort();
};
PACKET.ZC.CASH_ITEM_DELETE.size = 6;

// 0x29a
PACKET.ZC.ITEM_PICKUP_ACK2 = function PACKET_ZC_ITEM_PICKUP_ACK2(fp, end) {
	this.index = fp.readUShort();
	this.count = fp.readUShort();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
	this.location = fp.readUShort();
	this.type = fp.readUChar();
	this.result = fp.readUChar();
	this.HireExpireDate = fp.readLong();
};
PACKET.ZC.ITEM_PICKUP_ACK2.size = 27;

// 0x29b
PACKET.ZC.MER_INIT = function PACKET_ZC_MER_INIT(fp, end) {
	this.AID = fp.readLong();
	this.atk = fp.readShort();
	this.Matk = fp.readShort();
	this.hit = fp.readShort();
	this.critical = fp.readShort();
	this.def = fp.readShort();
	this.Mdef = fp.readShort();
	this.flee = fp.readShort();
	this.aspd = fp.readShort();
	this.name = fp.readString(NAME_LENGTH);
	this.level = fp.readShort();
	this.hp = fp.readLong();
	this.maxHP = fp.readLong();
	this.sp = fp.readLong();
	this.maxSP = fp.readLong();
	this.ExpireDate = fp.readLong();
	this.faith = fp.readShort();
	this.toal_call_num = fp.readLong();
	this.approval_monster_kill_counter = fp.readLong();
	this.ATKRange = fp.readShort();
};
PACKET.ZC.MER_INIT.size = 80;

// 0x29c
PACKET.ZC.MER_PROPERTY = function PACKET_ZC_MER_PROPERTY(fp, end) {
	this.atk = fp.readShort();
	this.Matk = fp.readShort();
	this.hit = fp.readShort();
	this.critical = fp.readShort();
	this.def = fp.readShort();
	this.Mdef = fp.readShort();
	this.flee = fp.readShort();
	this.aspd = fp.readShort();
	this.name = fp.readString(NAME_LENGTH);
	this.level = fp.readShort();
	this.hp = fp.readShort();
	this.maxHP = fp.readShort();
	this.sp = fp.readShort();
	this.maxSP = fp.readShort();
	this.ExpireDate = fp.readLong();
	this.faith = fp.readShort();
	this.toal_call_num = fp.readLong();
	this.approval_monster_kill_counter = fp.readLong();
};
PACKET.ZC.MER_PROPERTY.size = 66;

// 0x29d
PACKET.ZC.MER_SKILLINFO_LIST = function PACKET_ZC_MER_SKILLINFO_LIST(fp, end) {
	this.skillList = (function () {
		const count = ((end - fp.tell()) / 37) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].SKID = fp.readShort();
			out[i].type = fp.readLong();
			out[i].level = fp.readShort();
			out[i].spcost = fp.readShort();
			out[i].attackRange = fp.readShort();
			out[i].skillName = fp.readBinaryString(NAME_LENGTH);
			out[i].upgradable = fp.readChar();
		}
		return out;
	})();
};
PACKET.ZC.MER_SKILLINFO_LIST.size = -1;

// 0x97b
// <packet len>.W <exp>.L <death>.L <drop>.L <DETAIL_EXP_INFO>13B
// (ZC_PERSONAL_INFORMATION2) <InfoType>.B <Exp>.L <Death>.L <Drop>.L (DETAIL_EXP_INFO)
PACKET.ZC.PERSONAL_INFORMATION2 = function PACKET_ZC_PERSONAL_INFORMATION2(fp, end) {
	this.total_exp = fp.readLong();
	this.total_death = fp.readLong();
	this.total_drop = fp.readLong();

	this.info = (function () {
		const count = ((end - fp.tell()) / 13) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].type = fp.readChar(); // ?
			out[i].exp = fp.readLong();
			out[i].death = fp.readLong();
			out[i].drop = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.PERSONAL_INFORMATION2.size = -1;

// 0x097C - CZ_REQ_RANKING (PACKETVER >= 20130605, unified ranking request)
// rankType: 0=Blacksmith, 1=Alchemist, 2=Taekwon, 3=PK
PACKET.CZ.RANKING = function PACKET_CZ_RANKING() {
	this.rankType = 0;
};
PACKET.CZ.RANKING.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x097c);
	pkt_buf.writeShort(this.rankType);
	return pkt_buf;
};

// 0x097d - ZC_ACK_RANKING (PACKETVER 20130605 to 20190730, unified ranking response with names)
// rankType: 0=Blacksmith, 1=Alchemist, 2=Taekwon, 3=PK
PACKET.ZC.ACK_RANKING = function PACKET_ZC_ACK_RANKING(fp, end) {
	this.rankType = fp.readShort();
	this.Name = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readString(NAME_LENGTH);
		}
		return out;
	})();
	this.Point = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
	this.myPoints = fp.readLong();
};
PACKET.ZC.ACK_RANKING.size = 288;

// 0x29e
PACKET.ZC.MER_SKILLINFO_UPDATE = function PACKET_ZC_MER_SKILLINFO_UPDATE(fp, end) {
	this.SKID = fp.readUShort();
	this.level = fp.readShort();
	this.spcost = fp.readShort();
	this.attackRange = fp.readShort();
	this.upgradable = fp.readUChar();
};
PACKET.ZC.MER_SKILLINFO_UPDATE.size = 11;

// 0x2a2
PACKET.ZC.MER_PAR_CHANGE = function PACKET_ZC_MER_PAR_CHANGE(fp, end) {
	this.param = fp.readUShort();
	this.value = fp.readLong();
};
PACKET.ZC.MER_PAR_CHANGE.size = 8;

// 0x2a3
PACKET.ZC.GAMEGUARD_LINGO_KEY = function PACKET_ZC_GAMEGUARD_LINGO_KEY(fp, end) {
	this.packetType = fp.readShort();
	this.lingoKey = {};
	this.lingoKey.dwAlgNum = fp.readULong();
	this.lingoKey.dwAlgKey1 = fp.readULong();
	this.lingoKey.dwAlgKey2 = fp.readULong();
	this.lingoKey.dwSeed = fp.readULong();
};
PACKET.ZC.GAMEGUARD_LINGO_KEY.size = 18;

// 0x2aa
PACKET.ZC.REQ_CASH_PASSWORD = function PACKET_ZC_REQ_CASH_PASSWORD(fp, end) {
	this.Info = fp.readShort();
};
PACKET.ZC.REQ_CASH_PASSWORD.size = 4;

// 0x2ac
PACKET.ZC.RESULT_CASH_PASSWORD = function PACKET_ZC_RESULT_CASH_PASSWORD(fp, end) {
	this.Result = fp.readShort();
	this.ErrorCount = fp.readShort();
};
PACKET.ZC.RESULT_CASH_PASSWORD.size = 6;

// 0x2ad
PACKET.AC.REQUEST_SECOND_PASSWORD = function PACKET_AC_REQUEST_SECOND_PASSWORD(fp, end) {
	this.Result = fp.readShort();
	this.dwSeed = fp.readULong();
};
PACKET.AC.REQUEST_SECOND_PASSWORD.size = 8;

// 0x2b1
PACKET.ZC.ALL_QUEST_LIST = function PACKET_ZC_ALL_QUEST_LIST(fp, end) {
	this.questCount = fp.readLong();
	this.QuestList = (function () {
		const count = ((end - fp.tell()) / 5) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].active = fp.readUChar();
			out[i].count = 0; // workaround
		}
		return out;
	})();
};
PACKET.ZC.ALL_QUEST_LIST.size = -1;

// 0x2b2
PACKET.ZC.ALL_QUEST_MISSION = function PACKET_ZC_ALL_QUEST_MISSION(fp, end) {
	this.questCount = fp.readLong();
	this.QuestList = (function () {
		const count = ((end - fp.tell()) / 104) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; i++) {
			fp.seek(end - (count - i) * 104, SEEK_SET); // Position to the 104 long chunk start
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].quest_svrTime = fp.readLong();
			out[i].quest_endTime = fp.readLong();
			out[i].count = fp.readShort();
			out[i].hunt = new Array(out[i].count);
			for (let j = 0; j < out[i].count; j++) {
				out[i].hunt[j] = {};
				out[i].hunt[j].mobGID = fp.readULong();
				out[i].hunt[j].huntCount = fp.readShort();
				out[i].hunt[j].mobName = fp.readString(NAME_LENGTH);
			}
		}
		return out;
	})();
};
PACKET.ZC.ALL_QUEST_MISSION.size = -1;

// 0x2b3
PACKET.ZC.ADD_QUEST = function PACKET_ZC_ADD_QUEST(fp, end) {
	this.questID = fp.readULong();
	this.active = fp.readUChar();
	this.quest_svrTime = fp.readLong();
	this.quest_endTime = fp.readLong();
	this.count = fp.readShort();
	this.hunt = (function (count) {
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].mobGID = fp.readULong();
			out[i].huntCount = fp.readShort();
			out[i].mobName = fp.readString(NAME_LENGTH);
		}
		return out;
	})(this.count);
};
PACKET.ZC.ADD_QUEST.size = 107;

// 0x2b4
PACKET.ZC.DEL_QUEST = function PACKET_ZC_DEL_QUEST(fp, end) {
	this.questID = fp.readULong();
};
PACKET.ZC.DEL_QUEST.size = 6;

// 0x2b5
PACKET.ZC.UPDATE_MISSION_HUNT = function PACKET_ZC_UPDATE_MISSION_HUNT(fp, end) {
	this.questCount = fp.readShort();
	this.hunt = (function () {
		const count = ((end - fp.tell()) / 12) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].mobGID = fp.readULong();
			out[i].maxCount = fp.readShort();
			out[i].huntCount = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.UPDATE_MISSION_HUNT.size = -1;

// 0x2b7
PACKET.ZC.ACTIVE_QUEST = function PACKET_ZC_ACTIVE_QUEST(fp, end) {
	this.questID = fp.readULong();
	this.active = fp.readUChar();
};
PACKET.ZC.ACTIVE_QUEST.size = 7;

// 0x2b8
PACKET.ZC.ITEM_PICKUP_PARTY = function PACKET_ZC_ITEM_PICKUP_PARTY(fp, end) {
	this.accountID = fp.readULong();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
	this.location = fp.readUShort();
	this.type = fp.readUChar();
};
PACKET.ZC.ITEM_PICKUP_PARTY.size = 22;

// 0x2b9
PACKET.ZC.SHORTCUT_KEY_LIST = function PACKET_ZC_SHORTCUT_KEY_LIST(fp, end) {
	this.ShortCutKey = (function () {
		const count = 27;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].isSkill = fp.readChar();
			out[i].ID = fp.readULong();
			out[i].count = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.SHORTCUT_KEY_LIST.size = 191;

// 0x2bb
PACKET.ZC.EQUIPITEM_DAMAGED = function PACKET_ZC_EQUIPITEM_DAMAGED(fp, end) {
	this.wearLocation = fp.readUShort();
	this.accountID = fp.readULong();
};
PACKET.ZC.EQUIPITEM_DAMAGED.size = 8;

// 0x2bc
PACKET.ZC.NOTIFY_PCBANG_PLAYING_TIME = function PACKET_ZC_NOTIFY_PCBANG_PLAYING_TIME(fp, end) {
	this.TimeMinute = fp.readLong();
};
PACKET.ZC.NOTIFY_PCBANG_PLAYING_TIME.size = 6;

// 0x2bf
PACKET.ZC.SRPACKETR2_INIT = function PACKET_ZC_SRPACKETR2_INIT(fp, end) {
	this.ProtectFactor = fp.readUShort();
	this.DeformSeedFactor = fp.readULong();
	this.DeformAddFactor = fp.readULong();
};
PACKET.ZC.SRPACKETR2_INIT.size = 12;

// 0x2c1
PACKET.ZC.NPC_CHAT = function PACKET_ZC_NPC_CHAT(fp, end) {
	this.accountID = fp.readULong();
	this.color = fp.readULong();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.NPC_CHAT.size = -1;

// 0x2c2
PACKET.ZC.FORMATSTRING_MSG = function PACKET_ZC_FORMATSTRING_MSG(fp, end) {
	this.msg = fp.readUShort();
	this.value = fp.readString(end - fp.tell());
};
PACKET.ZC.FORMATSTRING_MSG.size = -1;

// 0x2c5
PACKET.ZC.PARTY_JOIN_REQ_ACK = function PACKET_ZC_PARTY_JOIN_REQ_ACK(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.answer = fp.readLong();
};
PACKET.ZC.PARTY_JOIN_REQ_ACK.size = 30;

// 0x2c6
PACKET.ZC.PARTY_JOIN_REQ = function PACKET_ZC_PARTY_JOIN_REQ(fp, end) {
	this.GRID = fp.readULong();
	this.groupName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.PARTY_JOIN_REQ.size = 30;

// 0x2c9
PACKET.ZC.PARTY_CONFIG = function PACKET_ZC_PARTY_CONFIG(fp, end) {
	this.bRefuseJoinMsg = fp.readUChar();
};
PACKET.ZC.PARTY_CONFIG.size = 3;

// 0x2ca
PACKET.HC.REFUSE_SELECTCHAR = function PACKET_HC_REFUSE_SELECTCHAR(fp, end) {
	this.ErrorCode = fp.readUChar();
};
PACKET.HC.REFUSE_SELECTCHAR.size = 3;

// 0x2cb
PACKET.ZC.MEMORIALDUNGEON_SUBSCRIPTION_INFO = function PACKET_ZC_MEMORIALDUNGEON_SUBSCRIPTION_INFO(fp, end) {
	this.MemorialDungeonName = fp.readString(61);
	this.PriorityOrderNum = fp.readShort();
};
PACKET.ZC.MEMORIALDUNGEON_SUBSCRIPTION_INFO.size = 65;

// 0x2cc
PACKET.ZC.MEMORIALDUNGEON_SUBSCRIPTION_NOTIFY = function PACKET_ZC_MEMORIALDUNGEON_SUBSCRIPTION_NOTIFY(fp, end) {
	this.PriorityOrderNum = fp.readShort();
};
PACKET.ZC.MEMORIALDUNGEON_SUBSCRIPTION_NOTIFY.size = 4;

// 0x2cd
PACKET.ZC.MEMORIALDUNGEON_INFO = function PACKET_ZC_MEMORIALDUNGEON_INFO(fp, end) {
	this.MemorialDungeonName = fp.readString(61);
	this.DestroyDate = fp.readULong();
	this.EnterTimeOutDate = fp.readULong();
};
PACKET.ZC.MEMORIALDUNGEON_INFO.size = 71;

// 0x2ce
PACKET.ZC.MEMORIALDUNGEON_NOTIFY = function PACKET_ZC_MEMORIALDUNGEON_NOTIFY(fp, end) {
	this.Type = fp.readLong();
	this.EnterLimitDate = fp.readULong();
};
PACKET.ZC.MEMORIALDUNGEON_NOTIFY.size = 10;

// 0x2d0
PACKET.ZC.EQUIPMENT_ITEMLIST3 = function PACKET_ZC_EQUIPMENT_ITEMLIST3(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 28) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.EQUIPMENT_ITEMLIST3.size = -1;

// 0x2d1
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST3 = function PACKET_ZC_STORE_EQUIPMENT_ITEMLIST3(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 28) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST3.size = -1;

// 0x2d2
PACKET.ZC.CART_EQUIPMENT_ITEMLIST3 = function PACKET_ZC_CART_EQUIPMENT_ITEMLIST3(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 28) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.CART_EQUIPMENT_ITEMLIST3.size = -1;

// 0x2d3
PACKET.ZC.NOTIFY_BIND_ON_EQUIP = function PACKET_ZC_NOTIFY_BIND_ON_EQUIP(fp, end) {
	this.index = fp.readUShort();
};
PACKET.ZC.NOTIFY_BIND_ON_EQUIP.size = 4;

// 0x2d4
PACKET.ZC.ITEM_PICKUP_ACK3 = function PACKET_ZC_ITEM_PICKUP_ACK3(fp, end) {
	this.index = fp.readUShort();
	this.count = fp.readUShort();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
	this.location = fp.readUShort();
	this.type = fp.readUChar();
	this.result = fp.readUChar();
	this.HireExpireDate = fp.readLong();
	this.bindOnEquipType = fp.readUShort();
};
PACKET.ZC.ITEM_PICKUP_ACK3.size = 29;

// 0x2d5
PACKET.ZC.ISVR_DISCONNECT = function PACKET_ZC_ISVR_DISCONNECT(fp, end) {};
PACKET.ZC.ISVR_DISCONNECT.size = 2;

// 0x2d7
PACKET.ZC.EQUIPWIN_MICROSCOPE = function PACKET_ZC_EQUIPWIN_MICROSCOPE(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.sex = fp.readUChar();
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 28) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.EQUIPWIN_MICROSCOPE.size = -1;

// 0x2d9
PACKET.ZC.CONFIG = function PACKET_ZC_CONFIG(fp, end) {
	this.Config = fp.readLong();
	this.Value = fp.readLong();
};
PACKET.ZC.CONFIG.size = 10;

// 0x2da
PACKET.ZC.CONFIG_NOTIFY = function PACKET_ZC_CONFIG_NOTIFY(fp, end) {
	this.show_eq_flag = fp.readUChar();
};
PACKET.ZC.CONFIG_NOTIFY.size = 3;

// 0x2dc
PACKET.ZC.BATTLEFIELD_CHAT = function PACKET_ZC_BATTLEFIELD_CHAT(fp, end) {
	this.accountID = fp.readULong();
	this.name = fp.readString(NAME_LENGTH);
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.BATTLEFIELD_CHAT.size = -1;

// 0x2dd
PACKET.ZC.BATTLEFIELD_NOTIFY_CAMPINFO = function PACKET_ZC_BATTLEFIELD_NOTIFY_CAMPINFO(fp, end) {
	this.accountID = fp.readULong();
	this.name = fp.readString(NAME_LENGTH);
	this.camp = fp.readShort();
};
PACKET.ZC.BATTLEFIELD_NOTIFY_CAMPINFO.size = 32;

// 0x2de
PACKET.ZC.BATTLEFIELD_NOTIFY_POINT = function PACKET_ZC_BATTLEFIELD_NOTIFY_POINT(fp, end) {
	this.pointCampA = fp.readShort();
	this.pointCampB = fp.readShort();
};
PACKET.ZC.BATTLEFIELD_NOTIFY_POINT.size = 6;

// 0x2df
PACKET.ZC.BATTLEFIELD_NOTIFY_POSITION = function PACKET_ZC_BATTLEFIELD_NOTIFY_POSITION(fp, end) {
	this.accountID = fp.readULong();
	this.name = fp.readString(NAME_LENGTH);
	this.job = fp.readUShort();
	this.x = fp.readShort();
	this.y = fp.readShort();
};
PACKET.ZC.BATTLEFIELD_NOTIFY_POSITION.size = 36;

// 0x2e0
PACKET.ZC.BATTLEFIELD_NOTIFY_HP = function PACKET_ZC_BATTLEFIELD_NOTIFY_HP(fp, end) {
	this.accountID = fp.readULong();
	this.name = fp.readString(NAME_LENGTH);
	this.hp = fp.readShort();
	this.maxHp = fp.readShort();
};
PACKET.ZC.BATTLEFIELD_NOTIFY_HP.size = 34;

// 0x2e1
PACKET.ZC.NOTIFY_ACT2 = function PACKET_ZC_NOTIFY_ACT2(fp, end) {
	this.GID = fp.readULong();
	this.targetGID = fp.readULong();
	this.startTime = fp.readULong();
	this.attackMT = fp.readLong();
	this.attackedMT = fp.readLong();
	this.damage = fp.readLong();
	this.count = fp.readShort();
	this.action = fp.readUChar();
	this.leftDamage = fp.readLong();
};
PACKET.ZC.NOTIFY_ACT2.size = 33;

// 0x2e7
PACKET.ZC.MAPPROPERTY = function PACKET_ZC_MAPPROPERTY(fp, end) {
	this.type = fp.readShort();
	this.mapInfoTable = (function () {
		const count = ((end - fp.tell()) / 4) | 0,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.MAPPROPERTY.size = -1;

// 0x2e8
PACKET.ZC.NORMAL_ITEMLIST3 = function PACKET_ZC_NORMAL_ITEMLIST3(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 22) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.NORMAL_ITEMLIST3.size = -1;

// 0x2e9
PACKET.ZC.CART_NORMAL_ITEMLIST3 = function PACKET_ZC_CART_NORMAL_ITEMLIST3(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 22) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.CART_NORMAL_ITEMLIST3.size = -1;

// 0x2ea
PACKET.ZC.STORE_NORMAL_ITEMLIST3 = function PACKET_ZC_STORE_NORMAL_ITEMLIST3(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 22) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readUShort();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.STORE_NORMAL_ITEMLIST3.size = -1;

// 0x2eb
PACKET.ZC.ACCEPT_ENTER2 = function PACKET_ZC_ACCEPT_ENTER2(fp, end) {
	this.startTime = fp.readULong();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.font = fp.readShort();
};
PACKET.ZC.ACCEPT_ENTER2.size = 13;

// 0x2ec
PACKET.ZC.NOTIFY_MOVEENTRY4 = function PACKET_ZC_NOTIFY_MOVEENTRY4(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.moveStartTime = fp.readULong();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
};
PACKET.ZC.NOTIFY_MOVEENTRY4.size = 67;

// 0x2ed
PACKET.ZC.NOTIFY_NEWENTRY4 = function PACKET_ZC_NOTIFY_NEWENTRY4(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
};
PACKET.ZC.NOTIFY_NEWENTRY4.size = 59;

// 0x2ee
PACKET.ZC.NOTIFY_STANDENTRY4 = function PACKET_ZC_NOTIFY_STANDENTRY4(fp, end) {
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
};
PACKET.ZC.NOTIFY_STANDENTRY4.size = 60;

// 0x2ef
PACKET.ZC.NOTIFY_FONT = function PACKET_ZC_NOTIFY_FONT(fp, end) {
	this.AID = fp.readULong();
	this.font = fp.readShort();
};
PACKET.ZC.NOTIFY_FONT.size = 8;

// 0x2f0
PACKET.ZC.PROGRESS = function PACKET_ZC_PROGRESS(fp, end) {
	this.color = fp.readULong();
	this.time = fp.readULong();
};
PACKET.ZC.PROGRESS.size = 10;

// 0x2f2
PACKET.ZC.PROGRESS_CANCEL = function PACKET_ZC_PROGRESS_CANCEL(fp, end) {};
PACKET.ZC.PROGRESS_CANCEL.size = 2;

// 0x35d
PACKET.ZC.SIMPLE_CASHSHOP_POINT_ITEMLIST = function PACKET_ZC_SIMPLE_CASHSHOP_POINT_ITEMLIST(fp, end) {
	this.CashPoint = fp.readULong();
	this.md_itemcount = fp.readShort();
	this.md_itemSize = fp.readShort();
	this.best_itemcount = fp.readShort();
	this.best_itemsize = fp.readShort();
	this.ItemList = (function () {
		const count = ((end - fp.tell()) / 11) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].discountprice = fp.readLong();
			out[i].type = fp.readUChar();
			out[i].ITID = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.SIMPLE_CASHSHOP_POINT_ITEMLIST.size = -1;

// 0x3dd
PACKET.AHC.GAME_GUARD = function PACKET_AHC_GAME_GUARD(fp, end) {
	this.AuthData = (function () {
		const count = 4,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readULong();
		}
		return out;
	})();
};
PACKET.AHC.GAME_GUARD.size = 18;

// 0x3de
PACKET.CAH.ACK_GAME_GUARD = function PACKET_CAH_ACK_GAME_GUARD(fp, end) {
	this.AuthData = (function () {
		const count = 4,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readULong();
		}
		return out;
	})();
};
PACKET.CAH.ACK_GAME_GUARD.size = 18;

// 0x43d
PACKET.ZC.SKILL_POSTDELAY = function PACKET_ZC_SKILL_POSTDELAY(fp, end) {
	this.SKID = fp.readUShort();
	this.DelayTM = fp.readULong();
};
PACKET.ZC.SKILL_POSTDELAY.size = 8;

// 0x43e
PACKET.ZC.SKILL_POSTDELAY_LIST = function PACKET_ZC_SKILL_POSTDELAY_LIST(fp, end) {
	this.delayList = (function () {
		const count = ((end - fp.tell()) / 6) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].SKID = fp.readUShort();
			out[i].DelayTM = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.SKILL_POSTDELAY_LIST.size = -1;

// 0x43f
PACKET.ZC.MSG_STATE_CHANGE2 = function PACKET_ZC_MSG_STATE_CHANGE2(fp, end) {
	this.index = fp.readShort();
	this.AID = fp.readULong();
	this.state = fp.readUChar();
	this.RemainMS = fp.readULong();
	this.val = (function () {
		const count = 3,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.MSG_STATE_CHANGE2.size = 25;

// 0x440
PACKET.ZC.MILLENNIUMSHIELD = function PACKET_ZC_MILLENNIUMSHIELD(fp, end) {
	this.AID = fp.readULong();
	this.num = fp.readShort();
	this.state = fp.readShort();
};
PACKET.ZC.MILLENNIUMSHIELD.size = 10;

// 0x441
PACKET.ZC.SKILLINFO_DELETE = function PACKET_ZC_SKILLINFO_DELETE(fp, end) {
	this.SKID = fp.readUShort();
};
PACKET.ZC.SKILLINFO_DELETE.size = 4;

// 0x442
PACKET.ZC.SKILL_SELECT_REQUEST = function PACKET_ZC_SKILL_SELECT_REQUEST(fp, end) {
	this.why = fp.readLong();
	this.SKID = (function () {
		const count = ((end - fp.tell()) / 2) | 0,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.SKILL_SELECT_REQUEST.size = -1;

// 0x444
PACKET.ZC.SIMPLE_CASH_POINT_ITEMLIST = function PACKET_ZC_SIMPLE_CASH_POINT_ITEMLIST(fp, end) {
	this.CashPoint = fp.readULong();
	this.ItemList = (function () {
		const count = ((end - fp.tell()) / 11) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].discountprice = fp.readLong();
			out[i].type = fp.readUChar();
			out[i].ITID = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.SIMPLE_CASH_POINT_ITEMLIST.size = -1;

// 0x446
PACKET.ZC.QUEST_NOTIFY_EFFECT = function PACKET_ZC_QUEST_NOTIFY_EFFECT(fp, end) {
	this.npcID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.effect = fp.readShort();
	this.color = fp.readShort();
};
PACKET.ZC.QUEST_NOTIFY_EFFECT.size = 14;

// 0x448
PACKET.HC.CHARACTER_LIST = function PACKET_HC_CHARACTER_LIST(fp, end) {
	this.CharacterList = (function () {
		const count = ((end - fp.tell()) / 5) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].dwGID = fp.readULong();
			out[i].SlotIdx = fp.readUChar();
		}
		return out;
	})();
};
PACKET.HC.CHARACTER_LIST.size = -1;

// 0x449
PACKET.ZC.HACKSH_ERROR_MSG = function PACKET_ZC_HACKSH_ERROR_MSG(fp, end) {
	this.ErrorID = fp.readShort();
};
PACKET.ZC.HACKSH_ERROR_MSG.size = 4;

// 0x7d0
PACKET.ZC.ES_RESULT = function PACKET_ZC_ES_RESULT(fp, end) {
	this.esNo = fp.readShort();
	this.esMsg = fp.readShort();
};
PACKET.ZC.ES_RESULT.size = 6;

// 0x7d2
PACKET.ZC.ES_LIST = function PACKET_ZC_ES_LIST(fp, end) {
	this.Count = fp.readShort();
};
PACKET.ZC.ES_LIST.size = 6;

// 0x7d5
PACKET.ZC.ES_READY = function PACKET_ZC_ES_READY(fp, end) {
	this.esNo = fp.readShort();
};
PACKET.ZC.ES_READY.size = 4;

// 0x7d6
PACKET.ZC.ES_GOTO = function PACKET_ZC_ES_GOTO(fp, end) {
	this.esNo = fp.readShort();
};
PACKET.ZC.ES_GOTO.size = 4;

// 0x7d8
PACKET.ZC.REQ_GROUPINFO_CHANGE_V2 = function PACKET_ZC_REQ_GROUPINFO_CHANGE_V2(fp, end) {
	this.expOption = fp.readULong();
	this.ItemPickupRule = fp.readUChar();
	this.ItemDivisionRule = fp.readUChar();
};
PACKET.ZC.REQ_GROUPINFO_CHANGE_V2.size = 8;

// 0x7d9
PACKET.ZC.SHORTCUT_KEY_LIST_V2 = function PACKET_ZC_SHORTCUT_KEY_LIST_V2(fp, end) {
	this.ShortCutKey = (function () {
		const count = 38;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].isSkill = fp.readChar();
			out[i].ID = fp.readULong();
			out[i].count = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.SHORTCUT_KEY_LIST_V2.size = 268;

// 0x7db
PACKET.ZC.HO_PAR_CHANGE = function PACKET_ZC_HO_PAR_CHANGE(fp, end) {
	this.param = fp.readUShort();
	this.value = fp.readLong();
};
PACKET.ZC.HO_PAR_CHANGE.size = 8;

// 0x7dd
PACKET.ZC.SEEK_PARTY = function PACKET_ZC_SEEK_PARTY(fp, end) {
	this.Name = fp.readString(NAME_LENGTH);
	this.Job = fp.readULong();
	this.Level = fp.readULong();
	this.mapName = fp.readBinaryString(16);
	this.Option = fp.readULong();
};
PACKET.ZC.SEEK_PARTY.size = 54;

// 0x7df
PACKET.ZC.SEEK_PARTY_MEMBER = function PACKET_ZC_SEEK_PARTY_MEMBER(fp, end) {
	this.Name = fp.readString(NAME_LENGTH);
	this.Job = fp.readULong();
	this.Level = fp.readULong();
	this.mapName = fp.readBinaryString(16);
	this.Option = fp.readULong();
};
PACKET.ZC.SEEK_PARTY_MEMBER.size = 54;

// 0x7e0
PACKET.ZC.ES_NOTI_MYINFO = function PACKET_ZC_ES_NOTI_MYINFO(fp, end) {
	this.esNo = fp.readShort();
	this.esname = fp.readString(54);
};
PACKET.ZC.ES_NOTI_MYINFO.size = 58;

// 0x7e1
PACKET.ZC.SKILLINFO_UPDATE2 = function PACKET_ZC_SKILLINFO_UPDATE2(fp, end) {
	this.SKID = fp.readUShort();
	this.type = fp.readLong();
	this.level = fp.readShort();
	this.spcost = fp.readShort();
	this.attackRange = fp.readShort();
	this.upgradable = fp.readUChar();
};
PACKET.ZC.SKILLINFO_UPDATE2.size = 15;

// 0x7e2
PACKET.ZC.MSG_VALUE = function PACKET_ZC_MSG_VALUE(fp, end) {
	this.msg = fp.readUShort();
	this.value = fp.readLong();
};
PACKET.ZC.MSG_VALUE.size = 8;

// 0x7e3
PACKET.ZC.ITEMLISTWIN_OPEN = function PACKET_ZC_ITEMLISTWIN_OPEN(fp, end) {
	this.Type = fp.readLong();
};
PACKET.ZC.ITEMLISTWIN_OPEN.size = 6;

// 0x7e6
PACKET.ZC.MSG_SKILL = function PACKET_ZC_MSG_SKILL(fp, end) {
	this.SKID = fp.readUShort();
	this.MSGID = fp.readLong();
};
PACKET.ZC.MSG_SKILL.size = 8;

// 0x7e8
PACKET.HC.CHECKBOT = function PACKET_HC_CHECKBOT(fp, end) {
	this.img = fp.readBinaryString(end - fp.tell());
};
PACKET.HC.CHECKBOT.size = -1;

// 0x7e9
PACKET.HC.CHECKBOT_RESULT = function PACKET_HC_CHECKBOT_RESULT(fp, end) {
	this.Result = fp.readUChar();
};
PACKET.HC.CHECKBOT_RESULT.size = 5;

// 0x7eb
PACKET.ZC.BATTLE_FIELD_LIST = function PACKET_ZC_BATTLE_FIELD_LIST(fp, end) {
	this.Count = fp.readShort();
	this.ack_type = fp.readShort();
	this.InfoList = (function () {
		const count = ((end - fp.tell()) / 62) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].BFNO = fp.readULong();
			out[i].BattleFieldName = fp.readString(56);
			out[i].JoinTeam = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.BATTLE_FIELD_LIST.size = -1;

// 0x7ed
PACKET.ZC.JOIN_BATTLE_FIELD = function PACKET_ZC_JOIN_BATTLE_FIELD(fp, end) {
	this.BFNO = fp.readULong();
	this.JoinTeam = fp.readShort();
	this.Result = fp.readShort();
};
PACKET.ZC.JOIN_BATTLE_FIELD.size = 10;

// 0x7ef
PACKET.ZC.CANCEL_BATTLE_FIELD = function PACKET_ZC_CANCEL_BATTLE_FIELD(fp, end) {
	this.BFNO = fp.readULong();
	this.Result = fp.readShort();
};
PACKET.ZC.CANCEL_BATTLE_FIELD.size = 8;

// 0x7f1
PACKET.ZC.ACK_BATTLE_STATE_MONITOR = function PACKET_ZC_ACK_BATTLE_STATE_MONITOR(fp, end) {
	this.BFNO = fp.readULong();
	this.PlayCount = fp.readShort();
	this.BattleState = fp.readShort();
	this.TeamCount_A = fp.readShort();
	this.TeamCount_B = fp.readShort();
	this.MyCount = fp.readShort();
	this.JoinTeam = fp.readShort();
};
PACKET.ZC.ACK_BATTLE_STATE_MONITOR.size = 18;

// 0x7f2
PACKET.ZC.BATTLE_NOTI_START_STEP = function PACKET_ZC_BATTLE_NOTI_START_STEP(fp, end) {
	this.BFNO = fp.readULong();
	this.Result = fp.readShort();
};
PACKET.ZC.BATTLE_NOTI_START_STEP.size = 8;

// 0x7f3
PACKET.ZC.BATTLE_JOIN_NOTI_DEFER = function PACKET_ZC_BATTLE_JOIN_NOTI_DEFER(fp, end) {
	this.BFNO = fp.readULong();
};
PACKET.ZC.BATTLE_JOIN_NOTI_DEFER.size = 6;

// 0x7f4
PACKET.ZC.BATTLE_JOIN_DISABLE_STATE = function PACKET_ZC_BATTLE_JOIN_DISABLE_STATE(fp, end) {
	this.Enable = fp.readUChar();
};
PACKET.ZC.BATTLE_JOIN_DISABLE_STATE.size = 3;

// 0x7f6
PACKET.ZC.NOTIFY_EXP = function PACKET_ZC_NOTIFY_EXP(fp, end) {
	this.AID = fp.readULong();
	this.amount = fp.readLong();
	this.varID = fp.readUShort();
	this.expType = fp.readShort();
};
PACKET.ZC.NOTIFY_EXP.size = 14;

// 0x7f7
PACKET.ZC.NOTIFY_MOVEENTRY5 = function PACKET_ZC_NOTIFY_MOVEENTRY5(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.moveStartTime = fp.readULong();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_MOVEENTRY5.size = -1;

// 0x7f8
PACKET.ZC.NOTIFY_NEWENTRY5 = function PACKET_ZC_NOTIFY_NEWENTRY5(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_NEWENTRY5.size = -1;

// 0x7f9
PACKET.ZC.NOTIFY_STANDENTRY5 = function PACKET_ZC_NOTIFY_STANDENTRY5(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_STANDENTRY5.size = -1;

// 0x7fa
PACKET.ZC.DELETE_ITEM_FROM_BODY = function PACKET_ZC_DELETE_ITEM_FROM_BODY(fp, end) {
	this.DeleteType = fp.readShort();
	this.Index = fp.readUShort();
	this.Count = fp.readShort();
};
PACKET.ZC.DELETE_ITEM_FROM_BODY.size = 8;

// 0x7fb
PACKET.ZC.USESKILL_ACK2 = function PACKET_ZC_USESKILL_ACK2(fp, end) {
	this.AID = fp.readULong();
	this.targetID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.SKID = fp.readUShort();
	this.property = fp.readULong();
	this.delayTime = fp.readULong();
	this.isDisposable = fp.readUChar();
};
PACKET.ZC.USESKILL_ACK2.size = 25;

// 0x7fc
PACKET.ZC.CHANGE_GROUP_MASTER = function PACKET_ZC_CHANGE_GROUP_MASTER(fp, end) {
	this.OldMasterAID = fp.readULong();
	this.NewMasterAID = fp.readULong();
};
PACKET.ZC.CHANGE_GROUP_MASTER.size = 10;

// 0x7fe
PACKET.ZC.PLAY_NPC_BGM = function PACKET_ZC_PLAY_NPC_BGM(fp, end) {
	this.Bgm = fp.readBinaryString(NAME_LENGTH);
};
PACKET.ZC.PLAY_NPC_BGM.size = 26;

// 0x7ff
PACKET.ZC.DEFINE_CHECK = function PACKET_ZC_DEFINE_CHECK(fp, end) {
	this.Result = fp.readLong();
};
PACKET.ZC.DEFINE_CHECK.size = 8;

// 0x800
PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC2 = function PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC2(fp, end) {
	this.AID = fp.readULong();
	this.UniqueID = fp.readULong();
	this.itemList = (function () {
		let count = 0,
			i;
		const out = new Array(count);

		if (PACKETVER.value >= 20200723) {
			count = (end - fp.tell()) / (22 + 8 + 25 + 6 + 2 + 1); //Item options 25 bytes, (location viewSprite), itemId use Long now, grade
		} else if (PACKETVER.value >= 20181121) {
			count = (end - fp.tell()) / (22 + 8 + 25 + 6 + 2); //Item options 25 bytes, (location viewSprite), itemId use Long now
		} else if (PACKETVER.value >= 20160921) {
			count = (end - fp.tell()) / (22 + 25 + 6); //Item options 25 bytes, (location viewSprite)
		} else if (PACKETVER.value >= 20150226) {
			count = (end - fp.tell()) / (22 + 25); //Item options 25 bytes
		} else {
			count = (end - fp.tell()) / 22;
		}

		for (i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].count = fp.readShort();
			out[i].index = fp.readShort();
			out[i].type = fp.readUChar();
			if (PACKETVER.value >= 20181121) {
				out[i].ITID = fp.readULong();
			} else {
				out[i].ITID = fp.readUShort();
			}
			out[i].IsIdentified = fp.readUChar();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			if (PACKETVER.value >= 20181121) {
				out[i].slot.card1 = fp.readULong();
				out[i].slot.card2 = fp.readULong();
				out[i].slot.card3 = fp.readULong();
				out[i].slot.card4 = fp.readULong();
			} else {
				out[i].slot.card1 = fp.readUShort();
				out[i].slot.card2 = fp.readUShort();
				out[i].slot.card3 = fp.readUShort();
				out[i].slot.card4 = fp.readUShort();
			}
			if (PACKETVER.value >= 20150226) {
				const option = new Struct('short index', 'short value', 'char param');
				out[i].Options = [];
				out[i].Options[1] = fp.readStruct(option);
				out[i].Options[2] = fp.readStruct(option);
				out[i].Options[3] = fp.readStruct(option);
				out[i].Options[4] = fp.readStruct(option);
				out[i].Options[5] = fp.readStruct(option);
			}
			if (PACKETVER.value >= 20160921) {
				out[i].location = fp.readULong();
				out[i].viewSprite = fp.readUShort();
			}
			if (PACKETVER.value >= 20200723) {
				out[i].grade = fp.readUChar();
			}
		}
		return out;
	})();
};
PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC2.size = -1;

// 0x803
PACKET.ZC.PARTY_BOOKING_ACK_REGISTER = function PACKET_ZC_PARTY_BOOKING_ACK_REGISTER(fp, end) {
	this.Result = fp.readShort();
};
PACKET.ZC.PARTY_BOOKING_ACK_REGISTER.size = 4;

// 0x805
PACKET.ZC.PARTY_BOOKING_ACK_SEARCH = function PACKET_ZC_PARTY_BOOKING_ACK_SEARCH(fp, end) {
	this.IsExistMoreResult = fp.readUChar();
	this.Info = (function () {
		const count = ((end - fp.tell()) / 48) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].Index = fp.readULong();
			out[i].CharName = fp.readString(NAME_LENGTH);
			out[i].ExpireTime = fp.readLong();
			out[i].Detail = {};
			out[i].Detail.Level = fp.readShort();
			out[i].Detail.MapID = fp.readShort();
			out[i].Detail.Job = (function () {
				const _count = 6,
					_out = new Array(_count);
				for (let j = 0; j < _count; ++j) {
					_out[j] = fp.readShort();
				}
				return _out;
			})();
		}
		return out;
	})();
};
PACKET.ZC.PARTY_BOOKING_ACK_SEARCH.size = -1;

// 0x807
PACKET.ZC.PARTY_BOOKING_ACK_DELETE = function PACKET_ZC_PARTY_BOOKING_ACK_DELETE(fp, end) {
	this.Result = fp.readShort();
};
PACKET.ZC.PARTY_BOOKING_ACK_DELETE.size = 4;

// 0x809
PACKET.ZC.PARTY_BOOKING_NOTIFY_INSERT = function PACKET_ZC_PARTY_BOOKING_NOTIFY_INSERT(fp, end) {
	this.Info = {};
	this.Info.Index = fp.readULong();
	this.Info.CharName = fp.readString(NAME_LENGTH);
	this.Info.ExpireTime = fp.readLong();
	this.Info.Detail = {};
	this.Info.Detail.Level = fp.readShort();
	this.Info.Detail.MapID = fp.readShort();
	this.Info.Detail.Job1 = fp.readShort();
	this.Info.Detail.Job2 = fp.readShort();
	this.Info.Detail.Job3 = fp.readShort();
	this.Info.Detail.Job4 = fp.readShort();
	this.Info.Detail.Job5 = fp.readShort();
	this.Info.Detail.Job6 = fp.readShort();
};
PACKET.ZC.PARTY_BOOKING_NOTIFY_INSERT.size = 50;

// 0x80a
PACKET.ZC.PARTY_BOOKING_NOTIFY_UPDATE = function PACKET_ZC_PARTY_BOOKING_NOTIFY_UPDATE(fp, end) {
	this.Index = fp.readULong();
	this.Job1 = fp.readShort();
	this.Job2 = fp.readShort();
	this.Job3 = fp.readShort();
	this.Job4 = fp.readShort();
	this.Job5 = fp.readShort();
	this.Job6 = fp.readShort();
};
PACKET.ZC.PARTY_BOOKING_NOTIFY_UPDATE.size = 18;

// 0x80b
PACKET.ZC.PARTY_BOOKING_NOTIFY_DELETE = function PACKET_ZC_PARTY_BOOKING_NOTIFY_DELETE(fp, end) {
	this.Index = fp.readULong();
};
PACKET.ZC.PARTY_BOOKING_NOTIFY_DELETE.size = 6;

// 0x80d
PACKET.ZC.SIMPLE_CASH_BTNSHOW = function PACKET_ZC_SIMPLE_CASH_BTNSHOW(fp, end) {
	this.show = fp.readUChar();
};
PACKET.ZC.SIMPLE_CASH_BTNSHOW.size = 3;

// 0x80e
PACKET.ZC.NOTIFY_HP_TO_GROUPM_R2 = function PACKET_ZC_NOTIFY_HP_TO_GROUPM_R2(fp, end) {
	this.AID = fp.readULong();
	this.hp = fp.readLong();
	this.maxhp = fp.readLong();
};
PACKET.ZC.NOTIFY_HP_TO_GROUPM_R2.size = 14;

// 0x80f
PACKET.ZC.ADD_EXCHANGE_ITEM2 = function PACKET_ZC_ADD_EXCHANGE_ITEM2(fp, end) {
	this.ITID = fp.readUShort();
	this.type = fp.readUChar();
	this.count = fp.readLong();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
};
PACKET.ZC.ADD_EXCHANGE_ITEM2.size = 20;

// 0x810
PACKET.ZC.OPEN_BUYING_STORE = function PACKET_ZC_OPEN_BUYING_STORE(fp, end) {
	this.itemcount = fp.readUChar();
};
PACKET.ZC.OPEN_BUYING_STORE.size = 3;

// 0x812
PACKET.ZC.FAILED_OPEN_BUYING_STORE_TO_BUYER = function PACKET_ZC_FAILED_OPEN_BUYING_STORE_TO_BUYER(fp, end) {
	this.Result = fp.readShort();
	this.total_weight = fp.readLong();
};
PACKET.ZC.FAILED_OPEN_BUYING_STORE_TO_BUYER.size = 8;

// 0x813
PACKET.ZC.MYITEMLIST_BUYING_STORE = function PACKET_ZC_MYITEMLIST_BUYING_STORE(fp, end) {
	const size = PACKETVER.value >= 20181121 ? 11 : 9;
	this.AID = fp.readULong();
	this.limitZeny = fp.readLong();
	this.itemList = (function (_size) {
		const count = ((end - fp.tell()) / _size) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].count = fp.readShort();
			out[i].type = fp.readUChar();
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].index = i;
		}
		return out;
	})(size);
};
PACKET.ZC.MYITEMLIST_BUYING_STORE.size = -1;

// 0x814
PACKET.ZC.BUYING_STORE_ENTRY = function PACKET_ZC_BUYING_STORE_ENTRY(fp, end) {
	this.makerAID = fp.readULong();
	this.storeName = fp.readString(80);
};
PACKET.ZC.BUYING_STORE_ENTRY.size = 86;

// 0x816
PACKET.ZC.DISAPPEAR_BUYING_STORE_ENTRY = function PACKET_ZC_DISAPPEAR_BUYING_STORE_ENTRY(fp, end) {
	this.makerAID = fp.readULong();
};
PACKET.ZC.DISAPPEAR_BUYING_STORE_ENTRY.size = 6;

// 0x818
PACKET.ZC.ACK_ITEMLIST_BUYING_STORE = function PACKET_ZC_ACK_ITEMLIST_BUYING_STORE(fp, end) {
	this.AID = fp.readULong();
	this.UniqueID = fp.readULong();
	this.limitZeny = fp.readLong();

	const itemSize = PACKETVER.value >= 20181121 ? 11 : 9;
	this.itemList = (function () {
		const count = ((end - fp.tell()) / itemSize) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].count = fp.readShort();
			out[i].type = fp.readUChar();
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.ACK_ITEMLIST_BUYING_STORE.size = -1;

// 0x81a
PACKET.ZC.FAILED_TRADE_BUYING_STORE_TO_BUYER = function PACKET_ZC_FAILED_TRADE_BUYING_STORE_TO_BUYER(fp, end) {
	this.Result = fp.readShort();
};
PACKET.ZC.FAILED_TRADE_BUYING_STORE_TO_BUYER.size = 4;

// 0x81b
PACKET.ZC.UPDATE_ITEM_FROM_BUYING_STORE = function PACKET_ZC_UPDATE_ITEM_FROM_BUYING_STORE(fp, end) {
	this.ITID = fp.readUShort();
	this.count = fp.readShort();
	this.limitZeny = fp.readLong();
};
PACKET.ZC.UPDATE_ITEM_FROM_BUYING_STORE.size = 10;

// 0x81c
PACKET.ZC.ITEM_DELETE_BUYING_STORE = function PACKET_ZC_ITEM_DELETE_BUYING_STORE(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readShort();
	this.zeny = fp.readLong();
};
PACKET.ZC.ITEM_DELETE_BUYING_STORE.size = 10;

// 0x81d
PACKET.ZC.EL_INIT = function PACKET_ZC_EL_INIT(fp, end) {
	this.AID = fp.readLong();
	this.hp = fp.readLong();
	this.maxHP = fp.readLong();
	this.sp = fp.readLong();
	this.maxSP = fp.readLong();
};
PACKET.ZC.EL_INIT.size = 22;

// 0x81e
PACKET.ZC.EL_PAR_CHANGE = function PACKET_ZC_EL_PAR_CHANGE(fp, end) {
	this.param = fp.readUShort();
	this.value = fp.readLong();
};
PACKET.ZC.EL_PAR_CHANGE.size = 8;

// 0x81f
PACKET.ZC.BROADCAST4 = function PACKET_ZC_BROADCAST4(fp, end) {
	this.PakcetType = fp.readShort();
	this.Msgtype = fp.readUChar();
	this.ColorRGB = fp.readULong();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.BROADCAST4.size = -1;

// 0x820
PACKET.ZC.COSTUME_SPRITE_CHANGE = function PACKET_ZC_COSTUME_SPRITE_CHANGE(fp, end) {
	this.GID = fp.readULong();
	this.type = fp.readUChar();
	this.value = fp.readLong();
};
PACKET.ZC.COSTUME_SPRITE_CHANGE.size = 11;

// 0x821
PACKET.AC.OTP_USER = function PACKET_AC_OTP_USER(fp, end) {};
PACKET.AC.OTP_USER.size = 2;

// 0x823
PACKET.AC.OTP_AUTH_ACK = function PACKET_AC_OTP_AUTH_ACK(fp, end) {
	this.LoginResult = fp.readUShort();
};
PACKET.AC.OTP_AUTH_ACK.size = 6;

// 0x824
PACKET.ZC.FAILED_TRADE_BUYING_STORE_TO_SELLER = function PACKET_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER(fp, end) {
	this.Result = fp.readShort();
	this.ITID = fp.readUShort();
};
PACKET.ZC.FAILED_TRADE_BUYING_STORE_TO_SELLER.size = 6;

// 0x826
PACKET.AC.SSO_LOGIN_ACK = function PACKET_AC_SSO_LOGIN_ACK(fp, end) {
	this.Result = fp.readUShort();
};
PACKET.AC.SSO_LOGIN_ACK.size = 4;

// 0x828
PACKET.HC.DELETE_CHAR3_RESERVED = function PACKET_HC_DELETE_CHAR3_RESERVED(fp, end) {
	this.GID = fp.readULong();
	this.Result = fp.readLong();
	this.DeleteReservedDate = fp.readLong();
};
PACKET.HC.DELETE_CHAR3_RESERVED.size = 14;

// 0x82a
PACKET.HC.DELETE_CHAR3 = function PACKET_HC_DELETE_CHAR3(fp, end) {
	this.GID = fp.readULong();
	this.Result = fp.readLong();
};
PACKET.HC.DELETE_CHAR3.size = 10;

// 0x82d
PACKET.HC.ACCEPT_ENTER_NEO_UNION_HEADER = function PACKET_HC_ACCEPT_ENTER_NEO_UNION_HEADER(fp, end) {
	this.TotalSlotNum = fp.readUChar();
	this.PremiumStartSlot = fp.readUChar();
	this.PremiumEndSlot = fp.readUChar();
	this.dummy1_beginbilling = fp.readChar();
	this.code = fp.readChar();
	fp.seek(20, SEEK_CUR);
};
PACKET.HC.ACCEPT_ENTER_NEO_UNION_HEADER.size = -1;

// 0x82c
PACKET.HC.DELETE_CHAR3_CANCEL = function PACKET_HC_DELETE_CHAR3_CANCEL(fp, end) {
	this.GID = fp.readULong();
	this.Result = fp.readLong();
};
PACKET.HC.DELETE_CHAR3_CANCEL.size = 10;

// 0x836
PACKET.ZC.SEARCH_STORE_INFO_ACK = function PACKET_ZC_SEARCH_STORE_INFO_ACK(fp, end) {
	this.IsFirstPage = fp.readUChar();
	this.IsNexPage = fp.readUChar();
	this.RemainedSearchCnt = fp.readUChar();
	this.SSI_List = (function () {
		const count = ((end - fp.tell()) / 106) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].SSI_ID = fp.readULong();
			out[i].AID = fp.readULong();
			out[i].StoreName = fp.readString(80);
			out[i].ITID = fp.readUShort();
			out[i].ItemType = fp.readUChar();
			out[i].price = fp.readLong();
			out[i].count = fp.readUShort();
			out[i].RefiningLevel = fp.readUChar();
			out[i].card1 = fp.readUShort();
			out[i].card2 = fp.readUShort();
			out[i].card3 = fp.readUShort();
			out[i].card4 = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.SEARCH_STORE_INFO_ACK.size = -1;

// 0x837
PACKET.ZC.SEARCH_STORE_INFO_FAILED = function PACKET_ZC_SEARCH_STORE_INFO_FAILED(fp, end) {
	this.Reason = fp.readUChar();
};
PACKET.ZC.SEARCH_STORE_INFO_FAILED.size = 3;

// 0x839
PACKET.ZC.ACK_BAN_GUILD_SSO = function PACKET_ZC_ACK_BAN_GUILD_SSO(fp, end) {
	this.charName = fp.readString(NAME_LENGTH);
	this.reasonDesc = fp.readString(40);
};
PACKET.ZC.ACK_BAN_GUILD_SSO.size = 66;

// 0x83a
PACKET.ZC.OPEN_SEARCH_STORE_INFO = function PACKET_ZC_OPEN_SEARCH_STORE_INFO(fp, end) {
	this.openType = fp.readShort();
	this.SearchCntMax = fp.readUChar();
};
PACKET.ZC.OPEN_SEARCH_STORE_INFO.size = 5;

// 0x83d
PACKET.ZC.SSILIST_ITEM_CLICK_ACK = function PACKET_ZC_SSILIST_ITEM_CLICK_ACK(fp, end) {
	this.x = fp.readShort();
	this.y = fp.readShort();
};
PACKET.ZC.SSILIST_ITEM_CLICK_ACK.size = 6;

// 0x83e
PACKET.AC.REFUSE_LOGIN_R2 = function PACKET_AC_REFUSE_LOGIN_R2(fp, end) {
	this.ErrorCode = fp.readULong();
	this.blockDate = fp.readBinaryString(20);
};
PACKET.AC.REFUSE_LOGIN_R2.size = 26;

// 0x840
PACKET.HC.NOTIFY_ACCESSIBLE_MAPNAME = function PACKET_HC_NOTIFY_ACCESSIBLE_MAPNAME(fp, end) {
	// fp.readString(end-fp.tell());
};
PACKET.HC.NOTIFY_ACCESSIBLE_MAPNAME.size = -1;

// 0x84b
PACKET.ZC.ITEM_FALL_ENTRY2 = function PACKET_ZC_ITEM_FALL_ENTRY2(fp, end) {
	this.ITAID = fp.readULong();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.type = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.subX = fp.readUChar();
	this.subY = fp.readUChar();
	this.count = fp.readShort();
};
PACKET.ZC.ITEM_FALL_ENTRY2.size = PACKETVER.value >= 20181121 ? 21 : 19;

// 0x856
PACKET.ZC.NOTIFY_MOVEENTRY6 = function PACKET_ZC_NOTIFY_MOVEENTRY6(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	if (PACKETVER.value >= 20181121) {
		this.shield = fp.readLong();
	}
	this.accessory = fp.readShort();
	this.moveStartTime = fp.readULong();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.name = readTrailingName(fp, end);
};
PACKET.ZC.NOTIFY_MOVEENTRY6.size = -1;

// 0x857
PACKET.ZC.NOTIFY_STANDENTRY6 = function PACKET_ZC_NOTIFY_STANDENTRY6(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	if (PACKETVER.value >= 20181121) {
		this.shield = fp.readLong();
	}
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_STANDENTRY6.size = -1;

// 0x858
PACKET.ZC.NOTIFY_NEWENTRY6 = function PACKET_ZC_NOTIFY_NEWENTRY6(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	if (PACKETVER.value >= 20181121) {
		this.shield = fp.readLong();
	}
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_NEWENTRY6.size = -1;

// 0x859
PACKET.ZC.EQUIPWIN_MICROSCOPE_V2 = function PACKET_ZC_EQUIPWIN_MICROSCOPEV2(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.Robe = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.sex = fp.readUChar();
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 28) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.EQUIPWIN_MICROSCOPE_V2.size = -1;

// 0x85a
PACKET.ZC.REASSEMBLY_AUTH01 = function PACKET_ZC_REASSEMBLY_AUTH01(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH01.size = 2;

// 0x85b
PACKET.ZC.REASSEMBLY_AUTH02 = function PACKET_ZC_REASSEMBLY_AUTH02(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH02.size = 2;

// 0x85c
PACKET.ZC.REASSEMBLY_AUTH03 = function PACKET_ZC_REASSEMBLY_AUTH03(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH03.size = 2;

// 0x85d
PACKET.ZC.REASSEMBLY_AUTH04 = function PACKET_ZC_REASSEMBLY_AUTH04(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH04.size = 2;

// 0x85e
PACKET.ZC.REASSEMBLY_AUTH05 = function PACKET_ZC_REASSEMBLY_AUTH05(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH05.size = 2;

// 0x85f
PACKET.ZC.REASSEMBLY_AUTH06 = function PACKET_ZC_REASSEMBLY_AUTH06(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH06.size = 2;

// 0x860
PACKET.ZC.REASSEMBLY_AUTH07 = function PACKET_ZC_REASSEMBLY_AUTH07(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH07.size = 2;

// 0x861
PACKET.ZC.REASSEMBLY_AUTH08 = function PACKET_ZC_REASSEMBLY_AUTH08(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH08.size = 2;

// 0x862
PACKET.ZC.REASSEMBLY_AUTH09 = function PACKET_ZC_REASSEMBLY_AUTH09(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH09.size = 2;

// 0x863
PACKET.ZC.REASSEMBLY_AUTH10 = function PACKET_ZC_REASSEMBLY_AUTH10(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH10.size = 2;

// 0x864
PACKET.ZC.REASSEMBLY_AUTH11 = function PACKET_ZC_REASSEMBLY_AUTH11(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH11.size = 2;

// 0x865
PACKET.ZC.REASSEMBLY_AUTH12 = function PACKET_ZC_REASSEMBLY_AUTH12(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH12.size = 2;

// 0x866
PACKET.ZC.REASSEMBLY_AUTH13 = function PACKET_ZC_REASSEMBLY_AUTH13(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH13.size = 2;

// 0x867
PACKET.ZC.REASSEMBLY_AUTH14 = function PACKET_ZC_REASSEMBLY_AUTH14(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH14.size = 2;

// 0x868
PACKET.ZC.REASSEMBLY_AUTH15 = function PACKET_ZC_REASSEMBLY_AUTH15(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH15.size = 2;

// 0x869
PACKET.ZC.REASSEMBLY_AUTH16 = function PACKET_ZC_REASSEMBLY_AUTH16(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH16.size = 2;

// 0x86a
PACKET.ZC.REASSEMBLY_AUTH17 = function PACKET_ZC_REASSEMBLY_AUTH17(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH17.size = 2;

// 0x86b
PACKET.ZC.REASSEMBLY_AUTH18 = function PACKET_ZC_REASSEMBLY_AUTH18(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH18.size = 2;

// 0x86c
PACKET.ZC.REASSEMBLY_AUTH19 = function PACKET_ZC_REASSEMBLY_AUTH19(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH19.size = 2;

// 0x86d
PACKET.ZC.REASSEMBLY_AUTH20 = function PACKET_ZC_REASSEMBLY_AUTH20(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH20.size = 2;

// 0x86e
PACKET.ZC.REASSEMBLY_AUTH21 = function PACKET_ZC_REASSEMBLY_AUTH21(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH21.size = 2;

// 0x86f
PACKET.ZC.REASSEMBLY_AUTH22 = function PACKET_ZC_REASSEMBLY_AUTH22(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH22.size = 2;

// 0x870
PACKET.ZC.REASSEMBLY_AUTH23 = function PACKET_ZC_REASSEMBLY_AUTH23(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH23.size = 2;

// 0x871
PACKET.ZC.REASSEMBLY_AUTH24 = function PACKET_ZC_REASSEMBLY_AUTH24(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH24.size = 2;

// 0x872
PACKET.ZC.REASSEMBLY_AUTH25 = function PACKET_ZC_REASSEMBLY_AUTH25(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH25.size = 2;

// 0x873
PACKET.ZC.REASSEMBLY_AUTH26 = function PACKET_ZC_REASSEMBLY_AUTH26(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH26.size = 2;

// 0x874
PACKET.ZC.REASSEMBLY_AUTH27 = function PACKET_ZC_REASSEMBLY_AUTH27(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH27.size = 2;

// 0x875
PACKET.ZC.REASSEMBLY_AUTH28 = function PACKET_ZC_REASSEMBLY_AUTH28(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH28.size = 2;

// 0x876
PACKET.ZC.REASSEMBLY_AUTH29 = function PACKET_ZC_REASSEMBLY_AUTH29(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH29.size = 2;

// 0x877
PACKET.ZC.REASSEMBLY_AUTH30 = function PACKET_ZC_REASSEMBLY_AUTH30(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH30.size = 2;

// 0x878
PACKET.ZC.REASSEMBLY_AUTH31 = function PACKET_ZC_REASSEMBLY_AUTH31(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH31.size = 2;

// 0x879
PACKET.ZC.REASSEMBLY_AUTH32 = function PACKET_ZC_REASSEMBLY_AUTH32(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH32.size = 2;

// 0x87a
PACKET.ZC.REASSEMBLY_AUTH33 = function PACKET_ZC_REASSEMBLY_AUTH33(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH33.size = 2;

// 0x87b
PACKET.ZC.REASSEMBLY_AUTH34 = function PACKET_ZC_REASSEMBLY_AUTH34(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH34.size = 2;

// 0x87c
PACKET.ZC.REASSEMBLY_AUTH35 = function PACKET_ZC_REASSEMBLY_AUTH35(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH35.size = 2;

// 0x87d
PACKET.ZC.REASSEMBLY_AUTH36 = function PACKET_ZC_REASSEMBLY_AUTH36(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH36.size = 2;

// 0x87e
PACKET.ZC.REASSEMBLY_AUTH37 = function PACKET_ZC_REASSEMBLY_AUTH37(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH37.size = 2;

// 0x87f
PACKET.ZC.REASSEMBLY_AUTH38 = function PACKET_ZC_REASSEMBLY_AUTH38(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH38.size = 2;

// 0x880
PACKET.ZC.REASSEMBLY_AUTH39 = function PACKET_ZC_REASSEMBLY_AUTH39(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH39.size = 2;

// 0x881
PACKET.ZC.REASSEMBLY_AUTH40 = function PACKET_ZC_REASSEMBLY_AUTH40(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH40.size = 2;

// 0x882
PACKET.ZC.REASSEMBLY_AUTH41 = function PACKET_ZC_REASSEMBLY_AUTH41(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH41.size = 2;

// 0x883
PACKET.ZC.REASSEMBLY_AUTH42 = function PACKET_ZC_REASSEMBLY_AUTH42(fp, end) {};
PACKET.ZC.REASSEMBLY_AUTH42.size = 2;

// 0x0884
PACKET.CZ.REASSEMBLY_AUTH01 = function PACKET_CZ_REASSEMBLY_AUTH01() {};
PACKET.CZ.REASSEMBLY_AUTH01.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0885
PACKET.CZ.REASSEMBLY_AUTH02 = function PACKET_CZ_REASSEMBLY_AUTH02() {};
PACKET.CZ.REASSEMBLY_AUTH02.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0886
PACKET.CZ.REASSEMBLY_AUTH03 = function PACKET_CZ_REASSEMBLY_AUTH03() {};
PACKET.CZ.REASSEMBLY_AUTH03.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0887
PACKET.CZ.REASSEMBLY_AUTH04 = function PACKET_CZ_REASSEMBLY_AUTH04() {};
PACKET.CZ.REASSEMBLY_AUTH04.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0888
PACKET.CZ.REASSEMBLY_AUTH05 = function PACKET_CZ_REASSEMBLY_AUTH05() {};
PACKET.CZ.REASSEMBLY_AUTH05.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0889
PACKET.CZ.REASSEMBLY_AUTH06 = function PACKET_CZ_REASSEMBLY_AUTH06() {};
PACKET.CZ.REASSEMBLY_AUTH06.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x088a
PACKET.CZ.REASSEMBLY_AUTH07 = function PACKET_CZ_REASSEMBLY_AUTH07() {};
PACKET.CZ.REASSEMBLY_AUTH07.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x088b
PACKET.CZ.REASSEMBLY_AUTH08 = function PACKET_CZ_REASSEMBLY_AUTH08() {};
PACKET.CZ.REASSEMBLY_AUTH08.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x088c
PACKET.CZ.REASSEMBLY_AUTH09 = function PACKET_CZ_REASSEMBLY_AUTH09() {};
PACKET.CZ.REASSEMBLY_AUTH09.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x088d
PACKET.CZ.REASSEMBLY_AUTH10 = function PACKET_CZ_REASSEMBLY_AUTH10() {};
PACKET.CZ.REASSEMBLY_AUTH10.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x088e
PACKET.CZ.REASSEMBLY_AUTH11 = function PACKET_CZ_REASSEMBLY_AUTH11() {};
PACKET.CZ.REASSEMBLY_AUTH11.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x088f
PACKET.CZ.REASSEMBLY_AUTH12 = function PACKET_CZ_REASSEMBLY_AUTH12() {};
PACKET.CZ.REASSEMBLY_AUTH12.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0890
PACKET.CZ.REASSEMBLY_AUTH13 = function PACKET_CZ_REASSEMBLY_AUTH13() {};
PACKET.CZ.REASSEMBLY_AUTH13.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0891
PACKET.CZ.REASSEMBLY_AUTH14 = function PACKET_CZ_REASSEMBLY_AUTH14() {};
PACKET.CZ.REASSEMBLY_AUTH14.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0892
PACKET.CZ.REASSEMBLY_AUTH15 = function PACKET_CZ_REASSEMBLY_AUTH15() {};
PACKET.CZ.REASSEMBLY_AUTH15.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0893
PACKET.CZ.REASSEMBLY_AUTH16 = function PACKET_CZ_REASSEMBLY_AUTH16() {};
PACKET.CZ.REASSEMBLY_AUTH16.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0894
PACKET.CZ.REASSEMBLY_AUTH17 = function PACKET_CZ_REASSEMBLY_AUTH17() {};
PACKET.CZ.REASSEMBLY_AUTH17.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0895
PACKET.CZ.REASSEMBLY_AUTH18 = function PACKET_CZ_REASSEMBLY_AUTH18() {};
PACKET.CZ.REASSEMBLY_AUTH18.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0896
PACKET.CZ.REASSEMBLY_AUTH19 = function PACKET_CZ_REASSEMBLY_AUTH19() {};
PACKET.CZ.REASSEMBLY_AUTH19.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0897
PACKET.CZ.REASSEMBLY_AUTH20 = function PACKET_CZ_REASSEMBLY_AUTH20() {};
PACKET.CZ.REASSEMBLY_AUTH20.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0898
PACKET.CZ.REASSEMBLY_AUTH21 = function PACKET_CZ_REASSEMBLY_AUTH21() {};
PACKET.CZ.REASSEMBLY_AUTH21.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x0899
PACKET.CZ.REASSEMBLY_AUTH22 = function PACKET_CZ_REASSEMBLY_AUTH22() {};
PACKET.CZ.REASSEMBLY_AUTH22.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x089a
PACKET.CZ.REASSEMBLY_AUTH23 = function PACKET_CZ_REASSEMBLY_AUTH23() {};
PACKET.CZ.REASSEMBLY_AUTH23.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x089b
PACKET.CZ.REASSEMBLY_AUTH24 = function PACKET_CZ_REASSEMBLY_AUTH24() {};
PACKET.CZ.REASSEMBLY_AUTH24.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x089c
PACKET.CZ.REASSEMBLY_AUTH25 = function PACKET_CZ_REASSEMBLY_AUTH25() {};
PACKET.CZ.REASSEMBLY_AUTH25.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x089d
PACKET.CZ.REASSEMBLY_AUTH26 = function PACKET_CZ_REASSEMBLY_AUTH26() {};
PACKET.CZ.REASSEMBLY_AUTH26.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x089e
PACKET.CZ.REASSEMBLY_AUTH27 = function PACKET_CZ_REASSEMBLY_AUTH27() {};
PACKET.CZ.REASSEMBLY_AUTH27.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x089f
PACKET.CZ.REASSEMBLY_AUTH28 = function PACKET_CZ_REASSEMBLY_AUTH28() {};
PACKET.CZ.REASSEMBLY_AUTH28.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a0
PACKET.CZ.REASSEMBLY_AUTH29 = function PACKET_CZ_REASSEMBLY_AUTH29() {};
PACKET.CZ.REASSEMBLY_AUTH29.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a1
PACKET.CZ.REASSEMBLY_AUTH30 = function PACKET_CZ_REASSEMBLY_AUTH30() {};
PACKET.CZ.REASSEMBLY_AUTH30.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a2
PACKET.CZ.REASSEMBLY_AUTH31 = function PACKET_CZ_REASSEMBLY_AUTH31() {};
PACKET.CZ.REASSEMBLY_AUTH31.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a3
PACKET.CZ.REASSEMBLY_AUTH32 = function PACKET_CZ_REASSEMBLY_AUTH32() {};
PACKET.CZ.REASSEMBLY_AUTH32.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a4
PACKET.CZ.REASSEMBLY_AUTH33 = function PACKET_CZ_REASSEMBLY_AUTH33() {};
PACKET.CZ.REASSEMBLY_AUTH33.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a5
PACKET.CZ.REASSEMBLY_AUTH34 = function PACKET_CZ_REASSEMBLY_AUTH34() {};
PACKET.CZ.REASSEMBLY_AUTH34.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a6
PACKET.CZ.REASSEMBLY_AUTH35 = function PACKET_CZ_REASSEMBLY_AUTH35() {};
PACKET.CZ.REASSEMBLY_AUTH35.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a7
PACKET.CZ.REASSEMBLY_AUTH36 = function PACKET_CZ_REASSEMBLY_AUTH36() {};
PACKET.CZ.REASSEMBLY_AUTH36.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a8
PACKET.CZ.REASSEMBLY_AUTH37 = function PACKET_CZ_REASSEMBLY_AUTH37() {};
PACKET.CZ.REASSEMBLY_AUTH37.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08a9
PACKET.CZ.REASSEMBLY_AUTH38 = function PACKET_CZ_REASSEMBLY_AUTH38() {};
PACKET.CZ.REASSEMBLY_AUTH38.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08aa
PACKET.CZ.REASSEMBLY_AUTH39 = function PACKET_CZ_REASSEMBLY_AUTH39() {};
PACKET.CZ.REASSEMBLY_AUTH39.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08ab
PACKET.CZ.REASSEMBLY_AUTH40 = function PACKET_CZ_REASSEMBLY_AUTH40() {};
PACKET.CZ.REASSEMBLY_AUTH40.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08ac
PACKET.CZ.REASSEMBLY_AUTH41 = function PACKET_CZ_REASSEMBLY_AUTH41() {};
PACKET.CZ.REASSEMBLY_AUTH41.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x08ad
PACKET.CZ.REASSEMBLY_AUTH42 = function PACKET_CZ_REASSEMBLY_AUTH42() {};
PACKET.CZ.REASSEMBLY_AUTH42.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(this.constructor.id);
	return pkt_buf;
};

// 0x8b3
PACKET.ZC.SHOWSCRIPT = function PACKET_ZC_SHOWSCRIPT(fp, end) {
	this.GID = fp.readULong();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.SHOWSCRIPT.size = -1;

// 0x8b9
PACKET.HC.SECOND_PASSWD_LOGIN = function PACKET_HC_SECOND_PASSWD_LOGIN(fp, end) {
	this.Seed = fp.readLong();
	this.Aid = fp.readLong();
	this.State = fp.readShort();
};
PACKET.HC.SECOND_PASSWD_LOGIN.size = 12;

// 0x8c7
PACKET.ZC.SKILL_ENTRY3 = function PACKET_ZC_SKILL_ENTRY3(fp, end) {
	this.AID = fp.readULong();
	this.creatorAID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.job = fp.readUChar();
	this.range = fp.readShort(); // TODO: check if it's not a char
	this.isVisible = fp.readUChar();
	this.isContens = fp.readUChar();
	this.msg = fp.readString(80);
};
PACKET.ZC.SKILL_ENTRY3.size = -1;

// 0x8c8
PACKET.ZC.NOTIFY_ACT3 = function PACKET_ZC_NOTIFY_ACT3(fp, end) {
	this.GID = fp.readULong();
	this.targetGID = fp.readULong();
	this.startTime = fp.readULong();
	this.attackMT = fp.readLong();
	this.attackedMT = fp.readLong();
	this.damage = fp.readLong();
	fp.seek(1, SEEK_CUR);
	this.count = fp.readShort();
	this.action = fp.readUChar();
	this.leftDamage = fp.readLong();
};
PACKET.ZC.NOTIFY_ACT3.size = 34;

// 0x8d0
PACKET.ZC.REQ_WEAR_EQUIP_ACK2 = function PACKET_ZC_REQ_WEAR_EQUIP_ACK2(fp, end) {
	this.index = fp.readUShort();
	this.wearLocation = fp.readUShort();
	this.viewid = fp.readUShort();
	this.result = !fp.readUChar();
};
PACKET.ZC.REQ_WEAR_EQUIP_ACK2.size = 9;

// 0x8d1
PACKET.ZC.REQ_TAKEOFF_EQUIP_ACK2 = function PACKET_ZC_REQ_TAKEOFF_EQUIP_ACK2(fp, end) {
	this.index = fp.readUShort();
	this.wearLocation = fp.readUShort();
	this.result = !fp.readUChar();
};
PACKET.ZC.REQ_TAKEOFF_EQUIP_ACK2.size = 7;

// 0xb6
PACKET.ZC.CLOSE_SCRIPT = function PACKET_ZC_CLOSE_SCRIPT(fp, end) {
	this.NAID = fp.readULong();
};
PACKET.ZC.CLOSE_SCRIPT.size = 6;

// 0x8d2
PACKET.ZC.FASTMOVE = function PACKET_ZC_FASTMOVE(fp, end) {
	this.AID = fp.readULong();
	this.targetXpos = fp.readShort();
	this.targetYpos = fp.readShort();
};
PACKET.ZC.FASTMOVE.size = 10;

// 0x8fe
PACKET.ZC.UPDATE_MISSION_HUNT2 = function PACKET_ZC_UPDATE_MISSION_HUNT2(fp, end) {
	this.questCount = ((end - fp.tell()) / 12) | 0; // workaround
	this.hunt = (function (questCount) {
		const out = new Array(questCount);
		for (let i = 0; i < questCount; ++i) {
			out[i] = {};
			out[i].huntID = fp.readULong();
			out[i].mobGID = fp.readULong();
			out[i].maxCount = fp.readShort();
			out[i].huntCount = fp.readShort();
		}
		return out;
	})(this.questCount);
};
PACKET.ZC.UPDATE_MISSION_HUNT2.size = -1;

// 0x8ff
PACKET.ZC.MSG_STATE_CHANGE3 = function PACKET_ZC_MSG_STATE_CHANGE3(fp, end) {
	this.AID = fp.readULong();
	this.index = fp.readShort();
	this.RemainMS = fp.readULong();
	this.val = (function () {
		const count = 3,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.MSG_STATE_CHANGE3.size = 24;

// 0x906
PACKET.ZC.EQUIPWIN_MICROSCOPE_V3 = function PACKET_ZC_EQUIPWIN_MICROSCOPEV3(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.Robe = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.sex = fp.readUChar();
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 28) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].IsIdentified = fp.readUChar();
			out[i].location = fp.readUShort();
			out[i].WearState = fp.readUShort();
			out[i].IsDamaged = fp.readUChar();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
		}
		return out;
	})();
};
PACKET.ZC.EQUIPWIN_MICROSCOPE_V3.size = -1;

// 0x907
PACKET.CZ.INVENTORY_TAB = function PACKET_CZ_INVENTORY_TAB() {
	this.item_index = 0;
	this.favorite = 0;
};
PACKET.CZ.INVENTORY_TAB.prototype.build = function () {
	const pkt_len = 5;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x907);
	pkt_buf.writeShort(this.item_index);
	pkt_buf.writeByte(this.favorite ? 1 : 0);
	return pkt_buf;
};

// 0x908
PACKET.ZC.ITEM_FAVORITE = function PACKET_ZC_ITEM_FAVORITE(fp, end) {
	this.index = fp.readUShort();
	this.favorite = fp.readUChar();
};
PACKET.ZC.ITEM_FAVORITE.size = 5;

// 0x90f
PACKET.ZC.NOTIFY_NEWENTRY7 = function PACKET_ZC_NOTIFY_NEWENTRY7(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.hp = fp.readLong();
	this.maxhp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_NEWENTRY7.size = -1;

// 0x914
PACKET.ZC.NOTIFY_MOVEENTRY8 = function PACKET_ZC_NOTIFY_MOVEENTRY8(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.moveStartTime = fp.readULong();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.hp = fp.readLong();
	this.maxhp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_MOVEENTRY8.size = -1;

// 0x915
PACKET.ZC.NOTIFY_STANDENTRY9 = function PACKET_ZC_NOTIFY_STANDENTRY9(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.hp = fp.readLong();
	this.maxhp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_STANDENTRY9.size = -1;

// 0x977
PACKET.ZC.NOTIFY_MONSTER_HP = function PACKET_ZC_NOTIFY_MONSTER_HP(fp, end) {
	this.AID = fp.readULong();
	this.hp = fp.readULong();
	this.maxhp = fp.readULong();
};
PACKET.ZC.NOTIFY_MONSTER_HP.size = 14;

// 0xa36
PACKET.ZC.HP_INFO_TINY = function PACKET_ZC_HP_INFO_TINY(fp, end) {
	this.GID = fp.readULong();
	this.hp = fp.readUChar();
};
PACKET.ZC.HP_INFO_TINY.size = 7;

// 0x97a
PACKET.ZC.ALL_QUEST_LIST_V2 = function PACKET_ZC_ALL_QUEST_LIST_V2(fp, end) {
	this.questCount = fp.readLong();
	this.QuestList = (function (questCount) {
		const out = new Array(questCount);
		for (let i = 0; i < questCount; ++i) {
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].active = fp.readUChar();
			out[i].quest_svrTime = fp.readULong();
			out[i].quest_endTime = fp.readULong();
			out[i].count = fp.readShort();
			out[i].hunt = new Array(out[i].count);
			for (let j = 0; j < out[i].count; j++) {
				out[i].hunt[j] = {};
				out[i].hunt[j].mobGID = fp.readULong();
				out[i].hunt[j].huntCount = fp.readShort();
				out[i].hunt[j].maxCount = fp.readShort();
				out[i].hunt[j].mobName = fp.readString(NAME_LENGTH);
			}
		}
		return out;
	})(this.questCount);
};
PACKET.ZC.ALL_QUEST_LIST_V2.size = -1;

// 0x983
PACKET.ZC.MSG_STATE_CHANGE4 = function PACKET_ZC_MSG_STATE_CHANGE4(fp, end) {
	this.index = fp.readShort();
	this.AID = fp.readULong();
	this.state = fp.readUChar();
	this.TotalMS = fp.readULong();
	this.RemainMS = fp.readULong();
	this.val = (function () {
		const count = 3,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.MSG_STATE_CHANGE4.size = 29;

// 0x984
PACKET.ZC.MSG_STATE_CHANGE5 = function PACKET_ZC_MSG_STATE_CHANGE5(fp, end) {
	this.AID = fp.readULong();
	this.index = fp.readShort();
	this.TotalMS = fp.readULong();
	this.RemainMS = fp.readULong();
	this.val = (function () {
		const count = 3,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.MSG_STATE_CHANGE5.size = 28;

// 0x988
PACKET.ZC.NOTIFY_CLAN_CONNECTINFO = function PACKET_ZC_NOTIFY_CLAN_CONNECTINFO(fp, end) {
	this.membersOnline = fp.readShort();
	this.membersTotal = fp.readShort();
};
PACKET.ZC.NOTIFY_CLAN_CONNECTINFO.size = 6;

// 0x989
PACKET.ZC.ACK_CLAN_LEAVE = function PACKET_ZC_ACK_CLAN_LEAVE(fp, end) {};
PACKET.ZC.ACK_CLAN_LEAVE.size = 2;

// 0x98a
PACKET.ZC.CLANINFO = function PACKET_ZC_CLANINFO(fp, end) {
	this.clanId = fp.readLong();
	this.name = fp.readString(NAME_LENGTH);
	this.master = fp.readString(NAME_LENGTH);
	this.territory = fp.readString(MAP_NAME_LENGTH_EXT);
	this.allyCount = fp.readUChar();
	this.antagonistCount = fp.readUChar();
	this.allyList = new Array(this.allyCount);
	for (let i = 0; i < this.allyCount; i++) {
		this.allyList[i] = fp.readString(NAME_LENGTH);
	}
	this.antagonistList = new Array(this.antagonistCount);
	for (let i = 0; i < this.antagonistCount; i++) {
		this.antagonistList[i] = fp.readString(NAME_LENGTH);
	}
};
PACKET.ZC.CLANINFO.size = -1;

// 0x98d
PACKET.CZ.CLAN_CHAT = function PACKET_CZ_CLAN_CHAT() {
	this.msg = '';
};
PACKET.CZ.CLAN_CHAT.prototype.build = function () {
	const pkt_len = 2 + 2 + this.msg.length + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x98d);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x98e
PACKET.ZC.NOTIFY_CLAN_CHAT = function PACKET_ZC_NOTIFY_CLAN_CHAT(fp, end) {
	this.memberName = fp.readString(NAME_LENGTH);
	this.message = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_CLAN_CHAT.size = -1;

// 0x990
PACKET.ZC.ITEM_PICKUP_ACK5 = function PACKET_ZC_ITEM_PICKUP_ACK5(fp, end) {
	this.index = fp.readUShort();
	this.count = fp.readUShort();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
	this.location = fp.readLong();
	this.type = fp.readUChar();
	this.result = fp.readUChar();
	this.HireExpireDate = fp.readLong();
	this.bindOnEquipType = fp.readUShort();
};
PACKET.ZC.ITEM_PICKUP_ACK5.size = 31;

// 0x991
PACKET.ZC.NORMAL_ITEMLIST4 = function PACKET_ZC_NORMAL_ITEMLIST4(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 24) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readULong();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].PlaceETCTab = flag & 2;
		}
		return out;
	})();
};
PACKET.ZC.NORMAL_ITEMLIST4.size = -1;

// 0x992
PACKET.ZC.EQUIPMENT_ITEMLIST4 = function PACKET_ZC_EQUIPMENT_ITEMLIST4(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 31) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.EQUIPMENT_ITEMLIST4.size = -1;

// 0x993
PACKET.ZC.CART_NORMAL_ITEMLIST4 = function PACKET_ZC_CART_NORMAL_ITEMLIST4(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 24) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readULong();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].PlaceETCTab = flag & 2;
		}
		return out;
	})();
};
PACKET.ZC.CART_NORMAL_ITEMLIST4.size = -1;

// 0x994
PACKET.ZC.CART_EQUIPMENT_ITEMLIST4 = function PACKET_ZC_CART_EQUIPMENT_ITEMLIST4(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 31) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.CART_EQUIPMENT_ITEMLIST4.size = -1;

// 0x995
PACKET.ZC.STORE_NORMAL_ITEMLIST4 = function PACKET_ZC_STORE_NORMAL_ITEMLIST4(fp, end) {
	this.Name = fp.readString(NAME_LENGTH);
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 24) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readULong();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].PlaceETCTab = flag & 2;
		}
		return out;
	})();
};
PACKET.ZC.STORE_NORMAL_ITEMLIST4.size = -1;

// 0x996
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST4 = function PACKET_ZC_STORE_EQUIPMENT_ITEMLIST4(fp, end) {
	this.Name = fp.readString(NAME_LENGTH);
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 31) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST4.size = -1;

// 0x997
PACKET.ZC.EQUIPWIN_MICROSCOPE_V4 = function PACKET_ZC_EQUIPWIN_MICROSCOPE_V4(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.Robe = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.sex = fp.readUChar();
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 31) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.EQUIPWIN_MICROSCOPE_V4.size = -1;

// 0x999
PACKET.ZC.ACK_WEAR_EQUIP_V5 = function PACKET_ZC_ACK_WEAR_EQUIP_V5(fp, end) {
	this.index = fp.readUShort();
	this.wearLocation = fp.readULong();
	this.viewid = fp.readUShort();
	this.result = !fp.readUChar();
};
PACKET.ZC.ACK_WEAR_EQUIP_V5.size = 0;

// 0x99a
PACKET.ZC.ACK_TAKEOFF_EQUIP_V5 = function PACKET_ZC_ACK_TAKEOFF_EQUIP_V5(fp, end) {
	this.index = fp.readUShort();
	this.wearLocation = fp.readULong();
	this.result = !fp.readUChar();
};
PACKET.ZC.ACK_TAKEOFF_EQUIP_V5.size = 9;

// 0x99b
PACKET.ZC.MAPPROPERTY_R2 = function PACKET_ZC_MAPPROPERTY_R2(fp, end) {
	this.type = fp.readShort();
	this.flag = fp.readLong();
};
PACKET.ZC.MAPPROPERTY_R2.size = 8;

// 0x99d
PACKET.HC.ACCEPT_ENTER_NEO_UNION_LIST = function PACKET_HC_ACCEPT_ENTER_NEO_UNION_LIST(fp, end) {
	this.charInfo = PACKETVER.parseCharInfo(fp, end);
};
PACKET.HC.ACCEPT_ENTER_NEO_UNION_LIST.size = -1;

// 0x99f
PACKET.ZC.SKILL_ENTRY4 = function PACKET_ZC_SKILL_ENTRY4(fp, end) {
	this.AID = fp.readULong();
	this.creatorAID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.job = fp.readULong();
	this.range = fp.readChar();
	this.isVisible = fp.readUChar();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.SKILL_ENTRY4.size = -1;

// 0x9a0
PACKET.HC.CHARLIST_NOTIFY = function PACKET_HC_CHARLIST_NOTIFY(fp, end) {
	this.TotalCnt = fp.readLong();
	if (PACKETVER.value >= 20151001 && PACKETVER.value < 20180103) {
		this.charSlots = fp.readLong();
	}
};
PACKET.HC.CHARLIST_NOTIFY.size = PACKETVER.value >= 20151001 && PACKETVER.value < 20180103 ? 10 : 6;

// 0x9a1
PACKET.CH.CHARLIST_REQ = function PACKET_CH_CHARLIST_REQ() {};
PACKET.CH.CHARLIST_REQ.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9a1);
	return pkt_buf;
};
PACKET.CH.CHARLIST_REQ.size = 2;

// 0x9a5
PACKET.AC.REFUSE_LOGIN3 = function PACKET_AC_REFUSE_LOGIN3(fp, end) {
	this.ErrorCode = fp.readUChar();
	this.blockDate = fp.readULong();
};
PACKET.AC.REFUSE_LOGIN3.size = 7;

// 0x9a6
PACKET.ZC.BANKING_CHECK = function PACKET_ZC_BANKING_CHECK(fp, end) {
	this.money = fp.readLong();
	this.reason = fp.readShort();
};
PACKET.ZC.BANKING_CHECK.size = 12;

// 0x9a7
PACKET.CZ.REQ_BANKING_DEPOSIT = function PACKET_CZ_REQ_BANKING_DEPOSIT() {
	this.AID = 0;
	this.money = 0;
};
PACKET.CZ.REQ_BANKING_DEPOSIT.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9a7);
	pkt_buf.writeLong(this.AID);
	pkt_buf.writeLong(this.money);

	return pkt_buf;
};

// 0x9a8
PACKET.ZC.ACK_BANKING_DEPOSIT = function PACKET_ZC_ACK_BANKING_DEPOSIT(fp, end) {
	this.reason = fp.readShort();
	this.money = fp.readLong();
	this.money2 = fp.readLong();
	this.zeny = fp.readLong();
};
PACKET.ZC.ACK_BANKING_DEPOSIT.size = 16;

// 0x9a9
PACKET.CZ.REQ_BANKING_WITHDRAW = function PACKET_CZ_REQ_BANKING_WITHDRAW() {
	this.AID = 0;
	this.money = 0;
};
PACKET.CZ.REQ_BANKING_WITHDRAW.prototype.build = function () {
	const pkt_len = 2 + 4 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9a9);
	pkt_buf.writeLong(this.AID);
	pkt_buf.writeLong(this.money);

	return pkt_buf;
};

// 0x9aa
PACKET.ZC.ACK_BANKING_WITHDRAW = function PACKET_ZC_ACK_BANKING_WITHDRAW(fp, end) {
	this.reason = fp.readShort();
	this.money = fp.readLong();
	this.money2 = fp.readLong();
	this.zeny = fp.readLong();
};
PACKET.ZC.ACK_BANKING_WITHDRAW.size = 16;

// 0x9ab
PACKET.CZ.REQ_BANKING_CHECK = function PACKET_CZ_REQ_BANKING_CHECK() {
	this.AID = 0;
};
PACKET.CZ.REQ_BANKING_CHECK.prototype.build = function () {
	const pkt_len = 6;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9ab);
	pkt_buf.writeULong(this.AID); // Write the aid

	return pkt_buf;
};

// 0x9b6
PACKET.CZ.REQ_BANK_OPEN = function PACKET_CZ_REQ_BANK_OPEN() {
	this.AID = 0;
};
PACKET.CZ.REQ_BANK_OPEN.prototype.build = function () {
	const pkt_len = 6;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9b6);
	pkt_buf.writeULong(this.AID); // Write the aid

	return pkt_buf;
};

// 0x9b7
PACKET.ZC.ACK_OPEN_BANKING = function PACKET_ZC_ACK_OPEN_BANKING(fp, end) {
	this.unknown = fp.readUShort();
};
PACKET.ZC.ACK_OPEN_BANKING.size = 4;

// 0x9b8
PACKET.CZ.REQ_BANK_CLOSE = function PACKET_CZ_REQ_BANK_CLOSE() {
	this.AID = 0;
};
PACKET.CZ.REQ_BANK_CLOSE.prototype.build = function () {
	const pkt_len = 6;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9b8);
	pkt_buf.writeULong(this.AID); // Write the aid

	return pkt_buf;
};

// 0x9b9
PACKET.ZC.ACK_CLOSE_BANKING = function PACKET_ZC_ACK_CLOSE_BANKING(fp, end) {
	this.unknown = fp.readUShort();
};
PACKET.ZC.ACK_CLOSE_BANKING.size = 4;

// 0x9ca
PACKET.ZC.SKILL_ENTRY5 = function PACKET_ZC_SKILL_ENTRY5(fp, end) {
	this.AID = fp.readULong();
	this.creatorAID = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.job = fp.readULong();
	this.range = fp.readChar();
	this.isVisible = fp.readUChar();
	this.level = fp.readUChar();
	this.msg = fp.readString(end - fp.tell());
};
PACKET.ZC.SKILL_ENTRY5.size = -1;

// 0x9cb
PACKET.ZC.USE_SKILL2 = function PACKET_ZC_USE_SKILL2(fp, end) {
	this.SKID = fp.readUShort();
	this.level = fp.readLong();
	this.targetAID = fp.readULong();
	this.srcAID = fp.readULong();
	this.result = fp.readUChar();
};
PACKET.ZC.USE_SKILL2.size = 17;

// 0x9db
PACKET.ZC.NOTIFY_MOVEENTRY10 = function PACKET_ZC_NOTIFY_MOVEENTRY10(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.AID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readUShort();
	this.weapon = fp.readULong();
	this.accessory = fp.readUShort();
	this.moveStartTime = fp.readULong();
	this.accessory2 = fp.readUShort();
	this.accessory3 = fp.readUShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readUShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.maxhp = fp.readLong();
	this.hp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_MOVEENTRY10.size = -1;

// 0x9dc
PACKET.ZC.NOTIFY_NEWENTRY10 = function PACKET_ZC_NOTIFY_NEWENTRY10(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.AID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.hp = fp.readLong();
	this.maxhp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_NEWENTRY10.size = -1;

// 0x9dd
PACKET.ZC.NOTIFY_STANDENTRY10 = function PACKET_ZC_NOTIFY_STANDENTRY10(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.AID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.hp = fp.readLong();
	this.maxhp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.body = fp.readShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_STANDENTRY10.size = -1;

// 0x9df
PACKET.ZC.ACK_WHISPER2 = function PACKET_ZC_ACK_WHISPER2(fp, end) {
	this.result = fp.readUChar();
	this.unknown = fp.readULong(); // AID ?
};
PACKET.ZC.ACK_WHISPER2.size = 7;

// 0x09e5
PACKET.ZC.DELETEITEM_FROM_MCSTORE2 = function PACKET_ZC_DELETEITEM_FROM_MCSTORE2(fp, end) {
	this.index = fp.readShort();
	this.count = fp.readShort();
	this.CID = fp.readULong();
	this.date = fp.readULong();
	this.zeny = fp.readULong();
};
PACKET.ZC.DELETEITEM_FROM_MCSTORE2.size = 18;

/** RodEx **/

/** Icon **/

// 0x9e7
PACKET.ZC.RODEX_ICON = function PACKET_ZC_RODEX_ICON(fp, end) {
	this.show = fp.readUChar();
};
PACKET.ZC.RODEX_ICON.size = 3;

/** Mail List **/

// 0x9f0
PACKET.ZC.ACK_RODEX_LIST = function PACKET_ZC_ACK_RODEX_LIST(fp, end) {
	this.openType = fp.readUChar();
	this.count = fp.readUChar();
	this.IsEnd = fp.readUChar();
	this.MailList = new Array();
	for (let i = 0; i < this.count; i++) {
		const Mail = {};
		Mail.openType = this.openType;
		Mail.MailID = fp.readUInt64();
		Mail.Isread = fp.readUChar();
		Mail.type = fp.readUChar();
		Mail.SenderName = fp.readString(NAME_LENGTH);
		Mail.regDateTime = fp.readULong();
		Mail.expireDateTime = fp.readULong();
		Mail.Titlelength = fp.readShort();
		Mail.title = fp.readString(Mail.Titlelength);
		this.MailList.push(Mail);
	}
};
PACKET.ZC.ACK_RODEX_LIST.size = -1;

// 0xa70
PACKET.CZ.RANDOM_COMBINE_ITEM_UI_CLOSE = function PACKET_CZ_RANDOM_COMBINE_ITEM_UI_CLOSE() {};
PACKET.CZ.RANDOM_COMBINE_ITEM_UI_CLOSE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xa70);
	return pkt_buf;
};

// 0xa7d
PACKET.ZC.ACK_RODEX_LIST2 = function PACKET_ZC_ACK_RODEX_LIST2(fp, end) {
	this.openType = fp.readUChar();
	this.count = fp.readUChar();
	this.IsEnd = fp.readUChar();
	this.MailList = new Array();
	for (let i = 0; i < this.count; i++) {
		const Mail = {};
		Mail.openType = this.openType;
		Mail.MailID = fp.readUInt64();
		Mail.Isread = fp.readUChar();
		Mail.type = fp.readUChar();
		Mail.SenderName = fp.readString(NAME_LENGTH);
		Mail.regDateTime = fp.readULong();
		Mail.expireDateTime = fp.readULong();
		Mail.Titlelength = fp.readShort();
		Mail.title = fp.readString(Mail.Titlelength);
		this.MailList.push(Mail);
	}
};
PACKET.ZC.ACK_RODEX_LIST2.size = -1;

// 0xac2
PACKET.ZC.ACK_RODEX_LIST3 = function PACKET_ZC_ACK_RODEX_LIST3(fp, end) {
	this.IsEnd = fp.readUChar();
	this.MailList = new Array();
	const len = end - fp.tell();
	for (let i = 0; i < len; i += 41) {
		const Mail = {};
		Mail.openType = fp.readUChar();
		Mail.MailID = fp.readUInt64();
		Mail.Isread = fp.readUChar();
		Mail.type = fp.readUChar();
		Mail.SenderName = fp.readString(NAME_LENGTH);
		Mail.expireDateTime = fp.readULong();
		Mail.Titlelength = fp.readShort();
		Mail.title = fp.readString(Mail.Titlelength);
		i += Mail.Titlelength;
		this.MailList.push(Mail);
	}
};
PACKET.ZC.ACK_RODEX_LIST3.size = -1;

// 0xaca
PACKET.AC.LOGIN_TAREN_REFUSE = function PACKET_AC_LOGIN_TAREN_REFUSE(fp, end) {
	this.ErrorCode = fp.readUChar();
};
PACKET.AC.LOGIN_TAREN_REFUSE.size = 3;

// 0xacd
PACKET.AC.LOGIN_TAREN_REFUSE2 = function PACKET_AC_LOGIN_TAREN_REFUSE2(fp, end) {
	this.ErrorCode = fp.readUChar();
	this.blockDate = fp.readBinaryString(20);
};
PACKET.AC.LOGIN_TAREN_REFUSE2.size = 23;

// 0x0b5a
PACKET.ZC.GRADE_ENCHANT_MATERIAL_LIST = function PACKET_ZC_GRADE_ENCHANT_MATERIAL_LIST(fp, end) {
	this.index = fp.readShort();
	this.success_chance = fp.readLong();
	this.blessing_info = {
		id: fp.readLong(),
		amount: fp.readLong(),
		max_blessing: fp.readLong(),
		bonus: fp.readLong()
	};
	this.protect_itemid = fp.readLong();
	this.protect_amount = fp.readLong();
	const material_size = 17;
	this.materialList = [];

	while (fp.tell() + material_size <= end) {
		const mat = {};
		mat.itemId = fp.readLong();
		mat.amount = fp.readLong();
		mat.price = fp.readLong();
		mat.downgrade = fp.readLong();
		mat.breakable = fp.readChar();

		this.materialList.push(mat);
	}
};
PACKET.ZC.GRADE_ENCHANT_MATERIAL_LIST.size = -1;

// 0xb5b
PACKET.CZ.GRADE_ENCHANT_REQ = function PACKET_CZ_GRADE_ENCHANT_REQ() {
	this.index = 0;
	this.material_index = 0;
	this.blessing_flag = 0;
	this.blessing_amount = 0;
	this.protect_flag = 0;
};
PACKET.CZ.GRADE_ENCHANT_REQ.prototype.build = function () {
	const pkt_len = 2 + 2 + 4 + 1 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0xb5b);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeLong(this.material_index);
	pkt_buf.writeChar(this.blessing_flag);
	pkt_buf.writeLong(this.blessing_amount);
	pkt_buf.writeChar(this.protect_flag);
	return pkt_buf;
};

// 0xb5c
PACKET.CZ.GRADE_ENCHANT_CLOSE_UI = function PACKET_CZ_GRADE_ENCHANT_CLOSE_UI() {};
PACKET.CZ.GRADE_ENCHANT_CLOSE_UI.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0xb5c);
	return pkt_buf;
};

// 0xb5d
PACKET.ZC.GRADE_ENCHANT_ACK = function PACKET_ZC_GRADE_ENCHANT_ACK(fp, end) {
	this.index = fp.readShort();
	this.grade = fp.readShort();
	this.result = fp.readLong();
};
PACKET.ZC.GRADE_ENCHANT_ACK.size = 10;

// 0xb5e
PACKET.ZC.GRADE_ENCHANT_BROADCAST_RESULT = function PACKET_ZC_GRADE_ENCHANT_BROADCAST_RESULT(fp, end) {
	this.char_name = fp.readString(NAME_LENGTH);
	this.itemId = fp.readULong();
	this.grade = fp.readShort();
	this.status = fp.readChar();
};
PACKET.ZC.GRADE_ENCHANT_BROADCAST_RESULT.size = -1;

// 0xb5f
PACKET.ZC.ACK_RODEX_LIST4 = function PACKET_ZC_ACK_RODEX_LIST4(fp, end) {
	this.IsEnd = fp.readUChar();
	this.MailList = new Array();
	const len = end - fp.tell();
	for (let i = 0; i < len; i += 45) {
		const Mail = {};
		Mail.openType = fp.readUChar();
		Mail.MailID = fp.readUInt64();
		Mail.Isread = fp.readUChar();
		Mail.type = fp.readUChar();
		Mail.SenderName = fp.readString(NAME_LENGTH);
		Mail.expireDateTime = fp.readULong();
		Mail.Titlelength = fp.readShort();
		Mail.title = fp.readString(Mail.Titlelength);
		Mail.unknown = fp.readULong();
		i += Mail.Titlelength;
		this.MailList.push(Mail);
	}
};
PACKET.ZC.ACK_RODEX_LIST4.size = -1;

/** Failed to Retrieve Mail List **/

// 0xAC3
PACKET.ZC.ACK_FAILED_ALL_RODEX_LIST = function PACKET_ZC_ACK_FAILED_ALL_RODEX_LIST(fp, end) {};
PACKET.ZC.ACK_FAILED_ALL_RODEX_LIST.size = 2;

/** READ MAIL **/

// 0x9eb
PACKET.ZC.ACK_READ_RODEX = function PACKET_ZC_ACK_READ_RODEX(fp, end) {
	this.openType = fp.readUChar();
	this.MailID = fp.readUInt64();
	this.TextcontentsLength = fp.readShort();
	this.zeny = fp.readUInt64();
	this.ItemCnt = fp.readUChar();
	this.Textcontent = fp.readString(this.TextcontentsLength);
	this.ItemList = new Array();
	const option = new Struct('short index', 'short value', 'char param');
	for (let i = 0; i < this.ItemCnt; i++) {
		const Item = {};
		Item.count = fp.readUShort();
		Item.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.IsIdentified = fp.readUChar();
		Item.IsDamaged = fp.readUChar();
		Item.RefiningLevel = fp.readUChar();
		Item.slot = {};
		Item.slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.location = fp.readLong();
		Item.type = fp.readUChar();
		Item.viewSprite = fp.readUShort();
		Item.bindOnEquipType = fp.readUShort();
		Item.Options = [];
		Item.Options[1] = fp.readStruct(option);
		Item.Options[2] = fp.readStruct(option);
		Item.Options[3] = fp.readStruct(option);
		Item.Options[4] = fp.readStruct(option);
		Item.Options[5] = fp.readStruct(option);
		this.ItemList.push(Item);
	}
};
PACKET.ZC.ACK_READ_RODEX.size = -1;

// 0xb63
PACKET.ZC.ACK_READ_RODEX2 = function PACKET_ZC_ACK_READ_RODEX2(fp, end) {
	this.openType = fp.readUChar();
	this.MailID = fp.readUInt64();
	this.TextcontentsLength = fp.readShort();
	this.zeny = fp.readUInt64();
	this.ItemCnt = fp.readUChar();
	this.Textcontent = fp.readString(this.TextcontentsLength);
	this.ItemList = new Array();
	const option = new Struct('short index', 'short value', 'char param');
	for (let i = 0; i < this.ItemCnt; i++) {
		const Item = {};
		Item.count = fp.readUShort();
		Item.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.IsIdentified = fp.readUChar();
		Item.IsDamaged = fp.readUChar();
		Item.slot = {};
		Item.slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
		Item.location = fp.readLong();
		Item.type = fp.readUChar();
		Item.viewSprite = fp.readUShort();
		Item.bindOnEquipType = fp.readUShort();
		Item.Options = [];
		Item.Options[1] = fp.readStruct(option);
		Item.Options[2] = fp.readStruct(option);
		Item.Options[3] = fp.readStruct(option);
		Item.Options[4] = fp.readStruct(option);
		Item.Options[5] = fp.readStruct(option);
		Item.RefiningLevel = fp.readUChar();
		Item.grade = fp.readUChar();
		this.ItemList.push(Item);
	}
};
PACKET.ZC.ACK_READ_RODEX2.size = -1;

/** Delete Mail Result **/

// 0x9f6 - only on success
PACKET.ZC.ACK_DELETE_RODEX = function PACKET_ZC_ACK_DELETE_RODEX(fp, end) {
	this.openType = fp.readUChar();
	this.MailID = fp.readUInt64();
};
PACKET.ZC.ACK_DELETE_RODEX.size = 11;

/** Sent Result **/

// 0x9ed
PACKET.ZC.ACK_SEND_RODEX = function PACKET_ZC_ACK_SEND_RODEX(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_SEND_RODEX.size = 3;

/** Retrieve Result **/

// 0x9f2
PACKET.ZC.ACK_ZENY_FROM_RODEX = function PACKET_ZC_ACK_ZENY_FROM_RODEX(fp, end) {
	this.MailID = fp.readUInt64();
	this.openType = fp.readUChar();
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_ZENY_FROM_RODEX.size = 12;

// 0x9f4
PACKET.ZC.ACK_ITEM_FROM_RODEX = function PACKET_ZC_ACK_ITEM_FROM_RODEX(fp, end) {
	this.MailID = fp.readUInt64();
	this.openType = fp.readUChar();
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_ITEM_FROM_RODEX.size = 12;

/** Add item to Mail Result **/

// 0xa05
PACKET.ZC.ACK_ADD_ITEM_RODEX = function PACKET_ZC_ACK_ADD_ITEM_RODEX(fp, end) {
	this.result = fp.readUChar();
	const option = new Struct('short index', 'short value', 'char param');

	this.index = fp.readShort();
	this.count = fp.readShort();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.type = fp.readUChar();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.weight = fp.readShort();
	this.favorite = fp.readUChar();
	this.location = fp.readULong();
};
PACKET.ZC.ACK_ADD_ITEM_RODEX.size = PACKETVER.value >= 20181121 ? 63 : 53;

// 0xb3f
PACKET.ZC.ACK_ADD_ITEM_RODEX2 = function PACKET_ZC_ACK_ADD_ITEM_RODEX2(fp, end) {
	this.result = fp.readUChar();
	const option = new Struct('short index', 'short value', 'char param');

	this.index = fp.readShort();
	this.count = fp.readShort();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.type = fp.readUChar();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.slot = {};
	this.slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.weight = fp.readShort();
	this.favorite = fp.readUChar();
	this.location = fp.readULong();
	this.RefiningLevel = fp.readUChar();
	this.grade = fp.readUChar();
};
PACKET.ZC.ACK_ADD_ITEM_RODEX2.size = PACKETVER.value >= 20181121 ? 64 : 54;

/** Remove Item from Mail Result **/

// 0xa07
PACKET.ZC.ACK_REMOVE_RODEX_ITEM = function PACKET_ZC_ACK_REMOVE_RODEX_ITEM(fp, end) {
	this.result = fp.readUChar();
	this.index = fp.readUShort();
	this.count = fp.readUShort();
	this.weight = fp.readUShort();
};
PACKET.ZC.ACK_REMOVE_RODEX_ITEM.size = 9;

/** Write Request Result **/

//0xa12
PACKET.ZC.ACK_OPEN_WRITE_RODEX = function PACKET_ZC_ACK_OPEN_WRITE_RODEX(fp, end) {
	this.receiveName = fp.readString(NAME_LENGTH);
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_OPEN_WRITE_RODEX.size = 27;

/*
 * TODO:
 * 0A32: ZC_OPEN_RODEX_THROUGH_NPC_ONLY
 * 0B96: ZC_RODEX_BLOCK_RECEIVE
 * 0B99: ZC_RODEX_RETURN_ACK
 */

// 0x9e8
PACKET.CZ.OPEN_RODEXBOX = function PACKET_CZ_OPEN_RODEXBOX() {
	this.openType = 0;
	this.MailID = 0;
};
PACKET.CZ.OPEN_RODEXBOX.prototype.build = function () {
	const pkt_len = 2 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9e8);
	pkt_buf.writeUChar(this.openType);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	return pkt_buf;
};

// 0x9e9
PACKET.CZ.CLOSE_RODEXBOX = function PACKET_CZ_CLOSE_RODEXBOX() {};
PACKET.CZ.CLOSE_RODEXBOX.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x9e9);
	return pkt_buf;
};

// 0x9ea
PACKET.CZ.REQ_READ_RODEX = function PACKET_CZ_REQ_READ_RODEX() {
	this.openType = 0;
	this.MailID = 0;
};
PACKET.CZ.REQ_READ_RODEX.prototype.build = function () {
	const pkt_len = 2 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9ea);
	pkt_buf.writeUChar(this.openType);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	return pkt_buf;
};

// 0x9ee
PACKET.CZ.REQ_NEXT_RODEX = function PACKET_CZ_REQ_NEXT_RODEX() {
	this.openType = 0;
	this.MailID = 0;
};
PACKET.CZ.REQ_NEXT_RODEX.prototype.build = function () {
	const pkt_len = 2 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9ee);
	pkt_buf.writeUChar(this.openType);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	return pkt_buf;
};

// 0x9ef
PACKET.CZ.REQ_REFRESH_RODEX = function PACKET_CZ_REQ_REFRESH_RODEX() {
	this.openType = 0;
	this.MailID = 0;
};
PACKET.CZ.REQ_REFRESH_RODEX.prototype.build = function () {
	const pkt_len = 2 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9ef);
	pkt_buf.writeUChar(this.openType);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	return pkt_buf;
};

// 0x9f1
PACKET.CZ.REQ_ZENY_FROM_RODEX = function PACKET_CZ_REQ_ZENY_FROM_RODEX() {
	this.MailID = 0;
	this.openType = 0;
};
PACKET.CZ.REQ_ZENY_FROM_RODEX.prototype.build = function () {
	const pkt_len = 2 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9f1);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	pkt_buf.writeUChar(this.openType);
	return pkt_buf;
};

// 0x9f3
PACKET.CZ.REQ_ITEM_FROM_RODEX = function PACKET_CZ_REQ_ITEM_FROM_RODEX() {
	this.MailID = 0;
	this.openType = 0;
};
PACKET.CZ.REQ_ITEM_FROM_RODEX.prototype.build = function () {
	const pkt_len = 2 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9f3);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	pkt_buf.writeUChar(this.openType);
	return pkt_buf;
};

// 0x9f5
PACKET.CZ.REQ_DELETE_RODEX = function PACKET_CZ_REQ_DELETE_RODEX() {
	this.openType = 0;
	this.MailID = 0;
};
PACKET.CZ.REQ_DELETE_RODEX.prototype.build = function () {
	const pkt_len = 2 + 9;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9f5);
	pkt_buf.writeUChar(this.openType);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	return pkt_buf;
};

// 0xa03
PACKET.CZ.REQ_CANCEL_WRITE_RODEX = function PACKET_CZ_REQ_CANCEL_WRITE_RODEX() {};
PACKET.CZ.REQ_CANCEL_WRITE_RODEX.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0xa03);
	return pkt_buf;
};

// 0xa04
PACKET.CZ.REQ_ADD_ITEM_RODEX = function PACKET_CZ_REQ_ADD_ITEM_RODEX() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.REQ_ADD_ITEM_RODEX.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0xa04);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeShort(this.count);
	return pkt_buf;
};

// 0xa06
PACKET.CZ.REQ_REMOVE_RODEX_ITEM = function PACKET_CZ_REQ_REMOVE_RODEX_ITEM() {
	this.index = 0;
	this.count = 0;
};
PACKET.CZ.REQ_REMOVE_RODEX_ITEM.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0xa06);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeShort(this.count);
	return pkt_buf;
};

// 0xa08
PACKET.CZ.REQ_OPEN_WRITE_RODEX = function PACKET_CZ_REQ_OPEN_WRITE_RODEX() {
	this.name = '';
};
PACKET.CZ.REQ_OPEN_WRITE_RODEX.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0xa08);
	pkt_buf.writeString(this.name, 24);
	return pkt_buf;
};

// 0xa15
PACKET.ZC.GOLDPCCAFE_POINT = function PACKET_ZC_GOLDPCCAFE_POINT(fp, end) {
	this.isActive = fp.readUChar();
	this.mode = fp.readUChar();
	this.point = fp.readULong();
	this.playedTime = fp.readULong();
};
PACKET.ZC.GOLDPCCAFE_POINT.size = 12;

// 0xa16
PACKET.CZ.DYNAMICNPC_CREATE_REQUEST = function PACKET_CZ_DYNAMICNPC_CREATE_REQUEST() {
	this.name = '';
};
PACKET.CZ.DYNAMICNPC_CREATE_REQUEST.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xa16);
	pkt_buf.writeString(this.name, NAME_LENGTH);

	return pkt_buf;
};

// 0xa17
PACKET.ZC.DYNAMICNPC_CREATE_RESULT = function PACKET_ZC_DYNAMICNPC_CREATE_RESULT(fp, end) {
	this.result = fp.readULong();
};
PACKET.ZC.DYNAMICNPC_CREATE_RESULT.size = 6;

// 0xac0
PACKET.CZ.OPEN_ALL_RODEX = function PACKET_CZ_OPEN_ALL_RODEX() {
	this.MailID = 0;
	this.MailReturnID = 0;
	this.MailAccountID = 0;
};
PACKET.CZ.OPEN_ALL_RODEX.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xac0);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	pkt_buf.writeULong(this.MailReturnID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	pkt_buf.writeULong(this.MailAccountID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	return pkt_buf;
};

// 0xac1
PACKET.CZ.UPDATE_ALL_RODEX = function PACKET_CZ_UPDATE_ALL_RODEX() {
	this.MailID = 0;
	this.MailReturnID = 0;
	this.MailAccountID = 0;
};
PACKET.CZ.UPDATE_ALL_RODEX.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xac1);
	pkt_buf.writeULong(this.MailID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	pkt_buf.writeULong(this.MailReturnID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	pkt_buf.writeULong(this.MailAccountID);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	return pkt_buf;
};

// 0x9ec
PACKET.CZ.REQ_SEND_RODEX = function PACKET_CZ_REQ_SEND_RODEX() {
	this.receiver = '';
	this.sender = '';
	this.zeny = 0;
	this.Titlelength = 0;
	this.Bodylength = 0;
	this.title = '';
	this.body = '';
};
PACKET.CZ.REQ_SEND_RODEX.prototype.build = function () {
	const pkt_len = 2 + 66 + this.Titlelength + this.Bodylength;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9ec);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.receiver, 24);
	pkt_buf.writeString(this.sender, 24);
	pkt_buf.writeULong(this.zeny);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	pkt_buf.writeUShort(this.Titlelength);
	pkt_buf.writeUShort(this.Bodylength);
	pkt_buf.writeString(this.title);
	pkt_buf.writeString(this.body);
	return pkt_buf;
};

// 0xa6e
PACKET.CZ.REQ_SEND_RODEX2 = function PACKET_CZ_REQ_SEND_RODEX2() {
	this.receiver = '';
	this.sender = '';
	this.zeny = 0;
	this.Titlelength = 0;
	this.Bodylength = 0;
	this.CharID = 0;
	this.title = '';
	this.body = '';
};
PACKET.CZ.REQ_SEND_RODEX2.prototype.build = function () {
	const pkt_len = 2 + 66 + this.Titlelength + this.Bodylength;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xa6e);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.receiver, 24);
	pkt_buf.writeString(this.sender, 24);
	pkt_buf.writeULong(this.zeny);
	pkt_buf.writeULong(0); // TODO check or convert to 8 bytes (UINT64)
	pkt_buf.writeUShort(this.Titlelength);
	pkt_buf.writeUShort(this.Bodylength);
	pkt_buf.writeULong(this.CharID);
	pkt_buf.writeString(this.title);
	pkt_buf.writeString(this.body);
	return pkt_buf;
};

// 0xa13
PACKET.CZ.CHECK_RECEIVE_CHARACTER_NAME = function PACKET_CZ_CHECK_RECEIVE_CHARACTER_NAME() {
	this.name = '';
};
PACKET.CZ.CHECK_RECEIVE_CHARACTER_NAME.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xa13);
	pkt_buf.writeString(this.name, 24);
	return pkt_buf;
};

// 0xb97
PACKET.CZ.CHECK_RODEX_RECEIVE = function PACKET_CZ_CHECK_RODEX_RECEIVE() {
	this.name = '';
	this.unknown = 1;
};
PACKET.CZ.CHECK_RODEX_RECEIVE.prototype.build = function () {
	const pkt_len = 2 + 24 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb97);
	pkt_buf.writeString(this.name, 24);
	pkt_buf.writeUChar(this.unknown);
	return pkt_buf;
};

/*
 * TODO:
 * 0B98: RODEX_RETURN
 */

// 0xa14
PACKET.ZC.CHECK_RECEIVE_CHARACTER_NAME = function PACKET_ZC_CHECK_RECEIVE_CHARACTER_NAME(fp, end) {
	this.CharID = fp.readULong();
	this.Job = fp.readUShort();
	this.level = fp.readUShort();
};
PACKET.ZC.CHECK_RECEIVE_CHARACTER_NAME.size = 10;

// 0xa51
PACKET.ZC.CHECK_RECEIVE_CHARACTER_NAME2 = function PACKET_ZC_CHECK_RECEIVE_CHARACTER_NAME2(fp, end) {
	this.CharID = fp.readULong();
	this.Job = fp.readUShort();
	this.level = fp.readUShort();
	this.name = fp.readString(NAME_LENGTH);
};
PACKET.ZC.CHECK_RECEIVE_CHARACTER_NAME2.size = 34;

// 0x9f7
PACKET.ZC.PROPERTY_HOMUN2 = function PACKET_ZC_PROPERTY_HOMUN2(fp, end) {
	this.szName = fp.readString(NAME_LENGTH); // <name>.24B
	this.bModified = fp.readUChar(); // <modified>.B
	this.nLevel = fp.readShort(); // <level>.W
	this.nFullness = fp.readShort(); // <hunger>.W
	this.nRelationship = fp.readShort(); // <intimacy>.W

	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort(); // <equip id>.W

	this.atk = fp.readShort(); // <atk>.W
	this.Matk = fp.readShort(); // <matk>.W
	this.hit = fp.readShort(); // <hit>.W
	this.critical = fp.readShort(); // <crit>.W
	this.def = fp.readShort(); // <def>.W
	this.Mdef = fp.readShort(); // <mdef>.W
	this.flee = fp.readShort(); // <flee>.W
	this.aspd = fp.readShort(); // <aspd>.W // todo wrong

	this.hp = fp.readLong(); // <hp>.L
	this.maxHP = fp.readLong(); // <max hp>.L
	this.sp = fp.readShort(); // <sp>.W
	this.maxSP = fp.readShort(); // <max sp>.W
	this.exp = fp.readLong(); // <exp>.L
	this.maxEXP = fp.readLong(); // <max exp>.L
	this.SKPoint = fp.readShort(); // <skill points>.W
	this.ATKRange = fp.readShort(); // <atk range>.W
};
PACKET.ZC.PROPERTY_HOMUN2.size = 75;

// 0x9f8
PACKET.ZC.ALL_QUEST_LIST_V3 = function PACKET_ZC_ALL_QUEST_LIST_V3(fp, end) {
	this.questCount = fp.readLong();
	this.QuestList = (function (questCount) {
		const out = new Array(questCount);
		for (let i = 0; i < questCount; ++i) {
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].active = fp.readUChar();
			out[i].quest_svrTime = fp.readULong();
			out[i].quest_endTime = fp.readULong();
			out[i].count = fp.readShort();
			out[i].hunt = new Array(out[i].count);
			for (let j = 0; j < out[i].count; j++) {
				out[i].hunt[j] = {};
				out[i].hunt[j].huntID = fp.readULong();
				out[i].hunt[j].mobType = fp.readULong();
				out[i].hunt[j].mobGID = fp.readULong();
				out[i].hunt[j].lvlMin = fp.readShort();
				out[i].hunt[j].lvlMax = fp.readShort();
				out[i].hunt[j].huntCount = fp.readShort();
				out[i].hunt[j].maxCount = fp.readShort();
				out[i].hunt[j].mobName = fp.readString(NAME_LENGTH);
			}
		}
		return out;
	})(this.questCount);
};
PACKET.ZC.ALL_QUEST_LIST_V3.size = -1;

// 0x9f9
PACKET.ZC.ADD_QUEST2 = function PACKET_ZC_ADD_QUEST2(fp, end) {
	this.questID = fp.readULong();
	this.active = fp.readUChar();
	this.quest_svrTime = fp.readLong();
	this.quest_endTime = fp.readLong();
	this.count = fp.readShort();
	this.hunt = (function (count) {
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].huntID = fp.readULong();
			out[i].mobType = fp.readULong();
			out[i].mobGID = fp.readULong();
			out[i].lvlMin = fp.readShort();
			out[i].lvlMax = fp.readShort();
			out[i].huntCount = fp.readShort();
			out[i].mobName = fp.readString(NAME_LENGTH);
		}
		return out;
	})(this.count);
};
PACKET.ZC.ADD_QUEST2.size = 143;

// 0x9fa
PACKET.ZC.UPDATE_MISSION_HUNT3 = function PACKET_ZC_UPDATE_MISSION_HUNT3(fp, end) {
	this.questCount = fp.readShort();
	this.hunt = (function () {
		const count = ((end - fp.tell()) / 12) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].huntID = fp.readULong();
			out[i].maxCount = fp.readShort();
			out[i].huntCount = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.UPDATE_MISSION_HUNT3.size = -1;

// 0x9fb
PACKET.CZ.PET_EVOLUTION = function PACKET_CZ_PET_EVOLUTION() {
	this.evolutionPetEggITID = 0;
};
PACKET.CZ.PET_EVOLUTION.prototype.build = function () {
	const isNew = PACKETVER.value >= 20181121;
	const pkt_len = 2 + 2 + (isNew ? 4 : 2);
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x9fb);
	pkt_buf.writeShort(pkt_len);
	if (isNew) {
		pkt_buf.writeULong(this.evolutionPetEggITID);
	} else {
		pkt_buf.writeUShort(this.evolutionPetEggITID);
	}
	return pkt_buf;
};

// 0x9fc
PACKET.ZC.PET_EVOLUTION_RESULT = function PACKET_ZC_PET_EVOLUTION_RESULT(fp, end) {
	this.result = fp.readULong();
};
PACKET.ZC.PET_EVOLUTION_RESULT.size = 6;

// 0x9fd
PACKET.ZC.NOTIFY_MOVEENTRY11 = function PACKET_ZC_NOTIFY_MOVEENTRY11(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.AID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readUShort();
	this.weapon = fp.readULong();
	if (PACKETVER.value >= 20181121) {
		this.shield = fp.readULong();
	}
	this.accessory = fp.readUShort();
	this.moveStartTime = fp.readULong();
	this.accessory2 = fp.readUShort();
	this.accessory3 = fp.readUShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readUShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.MoveData = fp.readPos2();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.maxhp = fp.readLong();
	this.hp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.body = fp.readUShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_MOVEENTRY11.size = -1;

// 0x9fe
PACKET.ZC.NOTIFY_NEWENTRY11 = function PACKET_ZC_NOTIFY_NEWENTRY11(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.AID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	if (PACKETVER.value >= 20181121) {
		this.shield = fp.readLong();
	}
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.hp = fp.readLong();
	this.maxhp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.body = fp.readShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_NEWENTRY11.size = -1;

// 0x9ff
PACKET.ZC.NOTIFY_STANDENTRY11 = function PACKET_ZC_NOTIFY_STANDENTRY11(fp, end) {
	this.objecttype = fp.readUChar();
	this.GID = fp.readULong();
	this.AID = fp.readULong();
	this.speed = fp.readShort();
	this.bodyState = fp.readShort();
	this.healthState = fp.readShort();
	this.effectState = fp.readLong();
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.weapon = fp.readLong();
	if (PACKETVER.value >= 20181121) {
		this.shield = fp.readLong();
	}
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.headDir = fp.readShort();
	this.Robe = fp.readShort();
	this.GUID = fp.readULong();
	this.GEmblemVer = fp.readShort();
	this.honor = fp.readShort();
	this.virtue = fp.readLong();
	this.isPKModeON = fp.readUChar();
	this.sex = fp.readUChar();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.state = fp.readUChar();
	this.clevel = fp.readShort();
	this.font = fp.readShort();
	this.hp = fp.readLong();
	this.maxhp = fp.readLong();
	this.isBoss = fp.readUChar();
	this.body = fp.readShort();
	this.name = fp.readString(end - fp.tell());
};
PACKET.ZC.NOTIFY_STANDENTRY11.size = -1;

// 0xa00
PACKET.ZC.SHORTCUT_KEY_LIST_V3 = function PACKET_ZC_SHORTCUT_KEY_LIST_V3(fp, end) {
	fp.seek(0x1, SEEK_CUR);
	this.ShortCutKey = (function () {
		const count = 38;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].isSkill = fp.readChar();
			out[i].ID = fp.readULong();
			out[i].count = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.SHORTCUT_KEY_LIST_V3.size = 269;

// 0xa07
PACKET.ZC.ACK_REMOVE_ITEM_MAIL = function ACK_REMOVE_ITEM_MAIL(fp, end) {
	this.success = fp.readChar();
	this.index = fp.readUShort();
	this.amount = fp.readUShort();
	this.weight_ = fp.readShort();
};
PACKET.ZC.ACK_REMOVE_ITEM_MAIL.size = 9;

// 0xa08
PACKET.CZ.REQ_OPEN_WRITE_MAIL = function PACKET_CZ_REQ_OPEN_WRITE_MAIL() {
	this.receiver = '';
};
PACKET.CZ.REQ_OPEN_WRITE_MAIL.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x64);
	pkt_buf.writeString(this.receiver);
	return pkt_buf;
};

// 0xa09
PACKET.ZC.ADD_EXCHANGE_ITEM3 = function PACKET_ZC_ADD_EXCHANGE_ITEM3(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.ITID = fp.readUShort();
	this.type = fp.readUChar();
	this.count = fp.readLong();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();

	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
};
PACKET.ZC.ADD_EXCHANGE_ITEM3.size = 45;

// 0xa0a
PACKET.ZC.ADD_ITEM_TO_STORE3 = function PACKET_ZC_ADD_ITEM_TO_STORE3(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.index = fp.readShort();
	this.count = fp.readLong();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.type = fp.readUChar();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();

	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
};
PACKET.ZC.ADD_ITEM_TO_STORE3.size = PACKETVER.value >= 20181121 ? 57 : 47;

// 0xa0b
PACKET.ZC.ADD_ITEM_TO_CART3 = function PACKET_ZC_ADD_ITEM_TO_CART3(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.index = fp.readShort();
	this.count = fp.readLong();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.type = fp.readUChar();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();

	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
};
PACKET.ZC.ADD_ITEM_TO_CART3.size = PACKETVER.value >= 20181121 ? 57 : 47;

//0xa0c
PACKET.ZC.ITEM_PICKUP_ACK6 = function PACKET_ZC_ITEM_PICKUP_ACK6(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.index = fp.readUShort();
	this.count = fp.readUShort();
	this.ITID = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readUShort();
	this.slot.card2 = fp.readUShort();
	this.slot.card3 = fp.readUShort();
	this.slot.card4 = fp.readUShort();
	this.location = fp.readLong();
	this.type = fp.readUChar();
	this.result = fp.readUChar();
	this.HireExpireDate = fp.readLong();
	this.bindOnEquipType = fp.readUShort();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
};
PACKET.ZC.ITEM_PICKUP_ACK6.size = 56;

//0xa0d
PACKET.ZC.EQUIPMENT_ITEMLIST5 = function PACKET_ZC_EQUIPMENT_ITEMLIST5(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 57) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();

			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			out[i].nRandomOptionCnt = fp.readChar();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.EQUIPMENT_ITEMLIST5.size = -1;

//0xa0f
PACKET.ZC.CART_EQUIPMENT_ITEMLIST5 = function PACKET_ZC_CART_EQUIPMENT_ITEMLIST5(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 57) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();

			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			out[i].nRandomOptionCnt = fp.readChar();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.CART_EQUIPMENT_ITEMLIST5.size = -1;

// 0xa10
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST5 = function PACKET_ZC_STORE_EQUIPMENT_ITEMLIST5(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.Name = fp.readString(NAME_LENGTH);
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 57) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			out[i].isOption = fp.readChar();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.STORE_EQUIPMENT_ITEMLIST5.size = -1;

// 0xa18
PACKET.ZC.ACCEPT_ENTER3 = function PACKET_ZC_ACCEPT_ENTER3(fp, end) {
	this.startTime = fp.readULong();
	this.PosDir = fp.readPos();
	this.xSize = fp.readUChar();
	this.ySize = fp.readUChar();
	this.font = fp.readShort();
	this.sex = fp.readUChar();
};
PACKET.ZC.ACCEPT_ENTER3.size = 14;

// 0xa23
PACKET.ZC.ALL_ACH_LIST = function PACKET_ZC_ALL_ACH_LIST(fp, end) {
	this.total_achievements = fp.readLong();
	this.total_points = fp.readLong();
	this.rank = fp.readShort();
	this.current_rank_points = fp.readLong();
	this.next_rank_points = fp.readLong();
	this.ach_list = new Array();

	while (fp.tell() < end) {
		const achievement = { ach_id: 0, completed: 0, objective: new Array(), completed_at: 0, reward: 0 };
		achievement.ach_id = fp.readLong();
		achievement.completed = fp.readUChar();
		for (let j = 0; j < 10; j++) {
			const objective_id = fp.readLong();
			if (objective_id > 0) achievement.objective.push(objective_id);
		}
		achievement.completed_at = fp.readLong();
		achievement.reward = fp.readUChar();
		if (achievement.ach_id > 0) this.ach_list.push(achievement);
	}
};
PACKET.ZC.ALL_ACH_LIST.size = -1;

// 0xa24
PACKET.ZC.ACH_UPDATE = function PACKET_ZC_ACH_UPDATE(fp, end) {
	this.total_points = fp.readLong();
	this.rank = fp.readShort();
	this.current_rank_points = fp.readLong();
	this.next_rank_points = fp.readLong();
	this.ach_list = new Array();
	const achievement = { ach_id: 0, completed: 0, objective: new Array(), completed_at: 0, reward: 0 };
	achievement.ach_id = fp.readLong();
	achievement.completed = fp.readUChar();
	for (let j = 0; j < 10; j++) {
		const objective_id = fp.readLong();
		if (objective_id > 0) achievement.objective.push(objective_id);
	}
	achievement.completed_at = fp.readLong();
	achievement.reward = fp.readUChar();
	if (achievement.ach_id > 0) this.ach_list.push(achievement);
};
PACKET.ZC.ACH_UPDATE.size = 66;

// 0xa25
PACKET.CZ.REQ_ACH_REWARD = function PACKET_CZ_REQ_ACH_REWARD() {
	this.ach_id = 0;
};
PACKET.CZ.REQ_ACH_REWARD.prototype.build = function () {
	const pkt_buf = new BinaryWriter(6);
	pkt_buf.writeShort(0xa25);
	pkt_buf.writeULong(this.ach_id);
	return pkt_buf;
};
PACKET.CZ.REQ_ACH_REWARD.size = 6;

// 0xa26
PACKET.ZC.REQ_ACH_REWARD_ACK = function PACKET_ZC_REQ_ACH_REWARD_ACK(fp, end) {
	this.failed = fp.readUChar();
	this.ach_id = fp.readLong();
};
PACKET.ZC.REQ_ACH_REWARD_ACK.size = 7;

// 0xa27
PACKET.ZC.RECOVERY2 = function PACKET_ZC_RECOVERY2(fp, end) {
	this.varID = fp.readShort();
	this.amount = fp.readLong();
};
PACKET.ZC.RECOVERY2.size = 8;

// 0xa28
PACKET.ZC.ACK_OPENSTORE2 = function PACKET_ZC_ACK_OPENSTORE2(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_OPENSTORE2.size = 3;

// 0xa2d
PACKET.ZC.EQUIPWIN_MICROSCOPE_V5 = function PACKET_ZC_EQUIPWIN_MICROSCOPE_V5(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.Robe = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.sex = fp.readUChar();
	const option = new Struct('short index', 'short value', 'char param');
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 57) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readUShort();
			out[i].slot.card2 = fp.readUShort();
			out[i].slot.card3 = fp.readUShort();
			out[i].slot.card4 = fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			out[i].nRandomOptionCnt = fp.readChar();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			const flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.EQUIPWIN_MICROSCOPE_V5.size = -1;

// 0xa30
PACKET.ZC.ACK_REQNAMEALL2 = function PACKET_ZC_ACK_REQNAMEALL2(fp, end) {
	this.AID = fp.readULong();
	this.CName = fp.readString(NAME_LENGTH);
	this.PName = fp.readString(NAME_LENGTH);
	this.GName = fp.readString(NAME_LENGTH);
	this.RName = fp.readString(NAME_LENGTH);
	this.TitleID = fp.readULong(); //maybe change in future
};
PACKET.ZC.ACK_REQNAMEALL2.size = 106;

// 0x0a2f
PACKET.ZC.ACK_CHANGE_TITLE = function (fp, end) {
	this.result = fp.readUChar();
	this.title_id = fp.readULong();
};
PACKET.ZC.ACK_CHANGE_TITLE.size = 7;

// 0x0a2e
PACKET.CZ.REQ_CHANGE_TITLE = function () {
	this.title_id = 0;
};
PACKET.CZ.REQ_CHANGE_TITLE.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x0a2e);
	pkt_buf.writeULong(this.title_id);
	return pkt_buf;
};
PACKET.CZ.REQ_CHANGE_TITLE.size = 6;

// 0xa37
PACKET.ZC.ITEM_PICKUP_ACK7 = function PACKET_ZC_ITEM_PICKUP_ACK7(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');
	this.index = fp.readUShort();
	this.count = fp.readUShort();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	this.slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.location = fp.readLong();
	this.type = fp.readUChar();
	this.result = fp.readUChar();
	this.HireExpireDate = fp.readLong();
	this.bindOnEquipType = fp.readUShort();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.favorite = fp.readUChar();
	this.look = fp.readUShort();
};
PACKET.ZC.ITEM_PICKUP_ACK7.size = PACKETVER.value >= 20181121 ? 69 : 59;

// 0xa39
PACKET.CH.MAKE_CHAR3 = function PACKET_CH_MAKE_CHAR3() {
	this.name = '';
	this.CharNum = 0;
	this.headPal = 0;
	this.head = 0;
	this.Job = 0;
	this.Sex = 0;
};
PACKET.CH.MAKE_CHAR3.prototype.build = function () {
	const pkt_len = 2 + 24 + 1 + 2 + 2 + 2 + 2 + 1; // Total 36 bytes
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xa39);
	pkt_buf.writeString(this.name, 24);
	pkt_buf.writeUChar(this.CharNum);
	pkt_buf.writeShort(this.headPal);
	pkt_buf.writeShort(this.head);
	pkt_buf.writeShort(this.Job);
	// "Unknown" occupies 2 bytes with a value of 0
	pkt_buf.writeUChar(0);
	pkt_buf.writeUChar(0);
	pkt_buf.writeUChar(this.Sex);

	return pkt_buf;
};

// 0xa41
PACKET.ZC.SKILL_SCALE = function PACKET_ZC_SKILL_SCALE(fp, end) {
	this.AID = fp.readULong();
	this.SKID = fp.readUShort();
	this.level = fp.readShort();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.startTime = fp.readULong();
};
PACKET.ZC.SKILL_SCALE.size = 18;

// 0xa43
PACKET.ZC.ADD_MEMBER_TO_GROUP3 = function PACKET_ZC_ADD_MEMBER_TO_GROUP3(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.role = fp.readULong();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.state = fp.readUChar();
	this.groupName = fp.readString(NAME_LENGTH);
	this.characterName = fp.readString(NAME_LENGTH);
	this.mapName = fp.readBinaryString(16);
	this.ItemPickupRule = fp.readUChar();
	this.ItemDivisionRule = fp.readUChar();
};
PACKET.ZC.ADD_MEMBER_TO_GROUP3.size = 85;

// 0xa44
PACKET.ZC.GROUP_LIST2 = function PACKET_ZC_GROUP_LIST2(fp, end) {
	this.groupName = fp.readBinaryString(NAME_LENGTH);
	this.groupInfo = (function () {
		const count = ((end - fp.tell()) / 50) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AID = fp.readULong();
			out[i].characterName = fp.readBinaryString(NAME_LENGTH);
			out[i].mapName = fp.readBinaryString(16);
			out[i].role = fp.readUChar();
			out[i].state = fp.readUChar();
			out[i].class_ = fp.readShort();
			out[i].baseLevel = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.GROUP_LIST2.size = -1;

// 0xa4e
PACKET.ZC.RANDOM_COMBINE_ITEM_UI_OPEN = function PACKET_ZC_RANDOM_COMBINE_ITEM_UI_OPEN(fp, end) {
	this.itemId = PACKETVER.value >= 20181121 ? fp.readLong() : fp.readShort();
};
PACKET.ZC.RANDOM_COMBINE_ITEM_UI_OPEN.size = PACKETVER.value >= 20181121 ? 6 : 4;

// 0xa4f
PACKET.CZ.REQ_RANDOM_COMBINE_ITEM = function PACKET_CZ_REQ_RANDOM_COMBINE_ITEM() {
	this.itemId = 0;
	this.items = [];
};
PACKET.CZ.REQ_RANDOM_COMBINE_ITEM.prototype.build = function () {
	const pkt_itemIdSize = PACKETVER.value >= 20181121 ? 4 : 2;
	const pkt_len = 2 + 2 + pkt_itemIdSize + this.items.length * 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xa4f);
	pkt_buf.writeShort(pkt_len);
	if (PACKETVER.value >= 20181121) {
		pkt_buf.writeLong(this.itemId);
	} else {
		pkt_buf.writeUShort(this.itemId);
	}
	for (let i = 0; i < this.items.length; i++) {
		pkt_buf.writeShort(this.items[i].index);
		pkt_buf.writeShort(this.items[i].count);
	}

	return pkt_buf;
};

// 0xa50
PACKET.ZC.ACK_RANDOM_COMBINE_ITEM = function PACKET_ZC_ACK_RANDOM_COMBINE_ITEM(fp, end) {
	this.result = fp.readShort();
};
PACKET.ZC.ACK_RANDOM_COMBINE_ITEM.size = 4;

// 0xa68
PACKET.CZ.UI_OPEN = function PACKET_CZ_UI_OPEN() {
	this.UIType = 0;
};
PACKET.CZ.UI_OPEN.prototype.build = function () {
	const pkt_buf = new BinaryWriter(3);
	pkt_buf.writeShort(0xa68);
	pkt_buf.writeUChar(this.UIType);
	return pkt_buf;
};

//0xae0
PACKET.AC.REFUSE_LOGIN_USA = function PACKET_AC_REFUSE_LOGIN_USA(fp, end) {
	this.ErrorCode = fp.readULong();
	this.Error = fp.readULong();
	this.blockDate = fp.readBinaryString(20);
};
PACKET.AC.REFUSE_LOGIN_USA.size = 30;

//0xae2
PACKET.ZC.UI_OPEN = function PACKET_ZC_UI_OPEN(fp, end) {
	this.ui_type = fp.readByte();
	this.data = fp.readULong();
};
PACKET.ZC.UI_OPEN.size = 7;

// 0xb9a
PACKET.ZC.UI_OPEN_V3 = function PACKET_ZC_UI_OPEN_V3(fp, end) {
	this.ui_type = fp.readByte();
	this.data = fp.readUInt64();
};
PACKET.ZC.UI_OPEN_V3.size = 11;

// 0xb9b
PACKET.CZ.REQUEST_RANDOM_ENCHANT = function PACKET_CZ_REQUEST_RANDOM_ENCHANT() {
	this.enchant_group = 0;
	this.index = 0;
};
PACKET.CZ.REQUEST_RANDOM_ENCHANT.prototype.build = function () {
	const pkt_buf = new BinaryWriter(12);
	pkt_buf.writeShort(0x0b9b);
	pkt_buf.writeUInt64(this.enchant_group);
	pkt_buf.writeShort(this.index);
	return pkt_buf;
};

// 0xb9c
PACKET.CZ.REQUEST_PERFECT_ENCHANT = function PACKET_CZ_REQUEST_PERFECT_ENCHANT() {
	this.enchant_group = 0;
	this.index = 0;
	this.ITID = 0;
};
PACKET.CZ.REQUEST_PERFECT_ENCHANT.prototype.build = function () {
	const pkt_buf = new BinaryWriter(16);
	pkt_buf.writeShort(0x0b9c);
	pkt_buf.writeUInt64(this.enchant_group);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeULong(this.ITID);
	return pkt_buf;
};

// 0xb9d
PACKET.CZ.REQUEST_UPGRADE_ENCHANT = function PACKET_CZ_REQUEST_UPGRADE_ENCHANT() {
	this.enchant_group = 0;
	this.index = 0;
	this.slot = 0;
};
PACKET.CZ.REQUEST_UPGRADE_ENCHANT.prototype.build = function () {
	const pkt_buf = new BinaryWriter(14);
	pkt_buf.writeShort(0x0b9d);
	pkt_buf.writeUInt64(this.enchant_group);
	pkt_buf.writeShort(this.index);
	pkt_buf.writeShort(this.slot);
	return pkt_buf;
};

// 0xb9e
PACKET.CZ.REQUEST_RESET_ENCHANT = function PACKET_CZ_REQUEST_RESET_ENCHANT() {
	this.enchant_group = 0;
	this.index = 0;
};
PACKET.CZ.REQUEST_RESET_ENCHANT.prototype.build = function () {
	const pkt_buf = new BinaryWriter(12);
	pkt_buf.writeShort(0x0b9e);
	pkt_buf.writeUInt64(this.enchant_group);
	pkt_buf.writeShort(this.index);
	return pkt_buf;
};

// 0xb9f
PACKET.ZC.RESPONSE_ENCHANT = function PACKET_ZC_RESPONSE_ENCHANT(fp, end) {
	this.msgId = fp.readLong();
	this.ITID = fp.readULong();
};
PACKET.ZC.RESPONSE_ENCHANT.size = 10;

// 0xba0
PACKET.CZ.CLOSE_UI_ENCHANT = function PACKET_CZ_CLOSE_UI_ENCHANT() {};
PACKET.CZ.CLOSE_UI_ENCHANT.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);
	pkt_buf.writeShort(0x0ba0);
	return pkt_buf;
};

// 0xa84
PACKET.ZC.GUILD_INFO3 = function PACKET_ZC_GUILD_INFO3(fp, end) {
	this.GDID = fp.readLong();
	this.level = fp.readLong();
	this.userNum = fp.readLong();
	this.maxUserNum = fp.readLong();
	this.userAverageLevel = fp.readLong();
	this.exp = fp.readLong();
	this.maxExp = fp.readLong();
	this.point = fp.readLong();
	this.honor = fp.readLong();
	this.virtue = fp.readLong();
	this.emblemVersion = fp.readLong();
	this.guildname = fp.readString(NAME_LENGTH);
	this.manageLand = fp.readString(16);
	this.zeny = fp.readLong();
	this.masterAID = fp.readLong();
	this.masterName = this.masterAID; // TODO char_id to name
};
PACKET.ZC.GUILD_INFO3.size = 114 - 20; // - <master name>.24B + <master char id>.L

// 0xa89
PACKET.ZC.STORE_ASSISTANT_ENTRY = function PACKET_ZC_STORE_ASSISTANT_ENTRY(fp, end) {
	this.GID = fp.readULong();
	this.job = fp.readShort();
	this.unknown = fp.readShort();
	this.PosDir = [fp.readShort(), fp.readShort(), 4];
	this.sex = fp.readUChar();
	this.head = fp.readShort();
	this.weapon = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.shield = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.Robe = fp.readShort();
	this.name = fp.readBinaryString(NAME_LENGTH);

	// fill necessary vars to render a player
	this.speed = 0;
	this.objecttype = 0;
	this.bodyState = 0;
	this.healthState = 0;
	this.effectState = 0;
	this.headDir = 0;
	this.GUID = 0;
	this.GEmblemVer = 0;
	this.honor = 0;
	this.virtue = 0;
	this.isPKModeON = 0;
	this.xSize = 0;
	this.ySize = 0;
	this.state = 0;
	this.clevel = 0;
};
PACKET.ZC.STORE_ASSISTANT_ENTRY.size = PACKETVER.value >= 20181121 ? 61 : 57;

// 0xa8a
PACKET.ZC.STORE_ASSISTANT_DISAPPEAR = function PACKET_ZC_STORE_ASSISTANT_DISAPPEAR(fp, end) {
	this.GID = fp.readULong();
	this.type = 0; // out of sight
};
PACKET.ZC.STORE_ASSISTANT_DISAPPEAR.size = 6;

// 0xa95
PACKET.ZC.CONFIG_NOTIFY2 = function PACKET_ZC_CONFIG_NOTIFY2(fp, end) {
	this.show_eq_flag = fp.readUChar();
	this.call_flag = fp.readUChar();
};
PACKET.ZC.CONFIG_NOTIFY2.size = 4;

// 0xa96
PACKET.ZC.ADD_EXCHANGE_ITEM4 = function PACKET_ZC_ADD_EXCHANGE_ITEM4(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.type = fp.readUChar();
	this.count = fp.readLong();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.slot = {};
	if (PACKETVER.value >= 20181121) {
		this.slot.card1 = fp.readULong();
		this.slot.card2 = fp.readULong();
		this.slot.card3 = fp.readULong();
		this.slot.card4 = fp.readULong();
	} else {
		this.slot.card1 = fp.readUShort();
		this.slot.card2 = fp.readUShort();
		this.slot.card3 = fp.readUShort();
		this.slot.card4 = fp.readUShort();
	}
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.location = fp.readULong();
	this.viewSprite = fp.readUShort();
};
PACKET.ZC.ADD_EXCHANGE_ITEM4.size = PACKETVER.value >= 20181121 ? 51 : 61;

// 0xa97
PACKET.CZ.REQ_WEAR_SWITCHEQUIP_ADD = function PACKET_CZ_REQ_WEAR_SWITCHEQUIP_ADD() {
	this.index = 0;
	this.wearLocation = 0;
};
PACKET.CZ.REQ_WEAR_SWITCHEQUIP_ADD.prototype.build = function () {
	const pkt_buf = new BinaryWriter(8);

	pkt_buf.writeShort(0xa97);
	pkt_buf.writeUShort(this.index);
	pkt_buf.writeULong(this.wearLocation);
	return pkt_buf;
};

// 0xa98
PACKET.ZC.REQ_WEAR_SWITCHEQUIP_ADD_RESULT = function PACKET_ZC_REQ_WEAR_SWITCHEQUIP_ADD_RESULT(fp, end) {
	this.index = fp.readUShort();
	this.location = fp.readULong();
	this.flag = PACKETVER.value > 20170502 ? fp.readUShort() : fp.readULong();
};
PACKET.ZC.REQ_WEAR_SWITCHEQUIP_ADD_RESULT.size = PACKETVER.value > 20170502 ? 10 : 12;

// 0xa99
PACKET.CZ.REQ_WEAR_SWITCHEQUIP_REMOVE = function PACKET_CZ_REQ_WEAR_SWITCHEQUIP_REMOVE() {
	this.index = 0;
	this.wearLocation = 0;
};
PACKET.CZ.REQ_WEAR_SWITCHEQUIP_REMOVE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(4);

	pkt_buf.writeShort(0xa99);
	pkt_buf.writeUShort(this.index);
	return pkt_buf;
};

// 0xa9a
PACKET.ZC.REQ_WEAR_SWITCHEQUIP_REMOVE_RESULT = function PACKET_ZC_REQ_WEAR_SWITCHEQUIP_REMOVE_RESULT(fp, end) {
	this.index = fp.readUShort();
	this.location = fp.readULong();
	this.flag = fp.readUShort();
};
PACKET.ZC.REQ_WEAR_SWITCHEQUIP_REMOVE_RESULT.size = 10;

//0xa9b
PACKET.ZC.SEND_SWAP_EQUIPITEM_INFO = function PACKET_ZC_SEND_SWAP_EQUIPITEM_INFO(fp, end) {
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / 6) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].position = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.SEND_SWAP_EQUIPITEM_INFO.size = -1;

// 0xa9c - Equip switch request packet
PACKET.CZ.REQ_FULLSWITCH = function PACKET_CZ_REQ_FULLSWITCH() {};
PACKET.CZ.REQ_FULLSWITCH.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);

	pkt_buf.writeShort(0xa9c); // Packet ID for equip switch request
	return pkt_buf;
};

// 0xa9d - Equip switch reply packet
PACKET.ZC.REQ_FULLSWITCH_RESULT = function PACKET_ZC_REQ_FULLSWITCH_RESULT(fp, end) {
	this.failed = fp.readUShort();
};
PACKET.ZC.REQ_FULLSWITCH_RESULT.size = 4;

// 0xaa0
PACKET.ZC.OPEN_REFINING_UI = function PACKET_ZC_OPEN_REFINING_UI(fp, end) {};
PACKET.ZC.OPEN_REFINING_UI.size = 2;

// 0xaa1
PACKET.CZ.REFINING_SELECT_ITEM = function PACKET_CZ_REFINING_SELECT_ITEM() {
	this.index = 0;
};
PACKET.CZ.REFINING_SELECT_ITEM.prototype.build = function () {
	const pkt_buf = new BinaryWriter(4);

	pkt_buf.writeShort(0xaa1);
	pkt_buf.writeShort(this.index);
	return pkt_buf;
};

// 0xaa2
PACKET.ZC.REFINING_MATERIAL_LIST = function PACKET_ZC_REFINING_MATERIAL_LIST(fp, end) {
	this.itemIndex = fp.readShort();
	this.blacksmithBlessing = fp.readChar();
	this.MaterialInfo = (function () {
		const size = PACKETVER.value >= 20181121 ? 9 : 7;
		const count = ((end - fp.tell()) / size) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].itemId = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].chance = fp.readChar();
			out[i].zeny = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.REFINING_MATERIAL_LIST.size = -1;

// 0xaa3
PACKET.CZ.REQ_REFINING = function PACKET_CZ_REQ_REFINING() {
	this.index = 0;
	this.itemId = 0;
	this.blacksmithBlessing = 0;
};
PACKET.CZ.REQ_REFINING.prototype.build = function () {
	const pkt_len = PACKETVER.value >= 20181121 ? 9 : 7;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xaa3);
	pkt_buf.writeShort(this.index);
	if (PACKETVER.value >= 20181121) {
		pkt_buf.writeULong(this.itemId);
	} else {
		pkt_buf.writeUShort(this.itemId);
	}
	pkt_buf.writeChar(this.blacksmithBlessing);
	return pkt_buf;
};

// 0xaa4
PACKET.CZ.CLOSE_REFINING_UI = function PACKET_CZ_CLOSE_REFINING_UI() {};
PACKET.CZ.CLOSE_REFINING_UI.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);

	pkt_buf.writeShort(0xaa4);
	return pkt_buf;
};

// 0xaa5
PACKET.ZC.MEMBERMGR_INFO2 = function PACKET_ZC_MEMBERMGR_INFO2(fp, end) {
	this.memberInfo = (function () {
		const count = ((end - fp.tell()) / 34) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AID = fp.readULong();
			out[i].GID = fp.readULong();
			out[i].HeadType = fp.readShort();
			out[i].HeadPalette = fp.readShort();
			out[i].Sex = fp.readShort();
			out[i].Job = fp.readShort();
			out[i].Level = fp.readShort();
			out[i].MemberExp = fp.readLong();
			out[i].CurrentState = fp.readLong();
			out[i].GPositionID = fp.readLong();
			out[i].Memo = '';
			out[i].CharName = ''; // todo char_id to char_name
			out[i].LastLogin = fp.readLong();
		}
		return out;
	})();
};
PACKET.ZC.MEMBERMGR_INFO2.size = -1;

// 0xaa8
PACKET.ZC.CONFIG_NOTIFY3 = function PACKET_ZC_CONFIG_NOTIFY3(fp, end) {
	this.show_eq_flag = fp.readUChar();
	this.call_flag = fp.readUChar();
	this.pet_autofeeding_flag = fp.readUChar();
};
PACKET.ZC.CONFIG_NOTIFY3.size = 5;

// 0xab2
PACKET.ZC.GROUP_ISALIVE = function PACKET_ZC_GROUP_ISALIVE(fp, end) {
	this.AID = fp.readULong();
	this.isDead = fp.readChar();
};
PACKET.ZC.GROUP_ISALIVE.size = 7;

// 0xab4
PACKET.ZC.RANDOM_UPGRADE_ITEM_UI_OPEN = function PACKET_ZC_RANDOM_UPGRADE_ITEM_UI_OPEN(fp, end) {
	this.itemId = PACKETVER.value >= 20181121 ? fp.readLong() : fp.readShort();
};
PACKET.ZC.RANDOM_UPGRADE_ITEM_UI_OPEN.size = PACKETVER.value >= 20181121 ? 6 : 4;

// 0xab5
PACKET.CZ.RANDOM_UPGRADE_ITEM_UI_CLOSE = function PACKET_CZ_RANDOM_UPGRADE_ITEM_UI_CLOSE() {};
PACKET.CZ.RANDOM_UPGRADE_ITEM_UI_CLOSE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xab5);
	return pkt_buf;
};

// 0xab6
PACKET.CZ.REQ_RANDOM_UPGRADE_ITEM = function PACKET_CZ_REQ_RANDOM_UPGRADE_ITEM() {
	this.itemId = 0;
	this.item_index = 0;
};
PACKET.CZ.REQ_RANDOM_UPGRADE_ITEM.prototype.build = function () {
	const pkt_itemIdSize = PACKETVER.value >= 20181121 ? 4 : 2;
	const pkt_index = 2;
	const pkt_len = 2 + pkt_itemIdSize + pkt_index;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xab6);
	if (PACKETVER.value >= 20181121) {
		pkt_buf.writeULong(this.itemId);
	} else {
		pkt_buf.writeUShort(this.itemId);
	}
	pkt_buf.writeUShort(this.item_index);

	return pkt_buf;
};

// 0xab7
PACKET.ZC.ACK_RANDOM_UPGRADE_ITEM = function PACKET_ZC_ACK_RANDOM_UPGRADE_ITEM(fp, end) {
	this.result = fp.readShort();
};
PACKET.ZC.ACK_RANDOM_UPGRADE_ITEM.size = 4;

// 0xac4
PACKET.AC.ACCEPT_LOGIN3 = function PACKET_AC_ACCEPT_LOGIN3(fp, end) {
	this.AuthCode = fp.readLong();
	this.AID = fp.readULong();
	this.userLevel = fp.readULong();
	this.lastLoginIP = fp.readULong();
	this.lastLoginTime = fp.readBinaryString(26);
	this.Sex = fp.readUChar();
	if (PACKETVER.value >= 20170315) {
		this.webAuthToken = fp.readBinaryString(17);
	}
	let pkt_len = 32;
	if (PACKETVER.value >= 20170315) {
		pkt_len = 160;
	}
	this.ServerList = (function () {
		const count = ((end - fp.tell()) / pkt_len) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].ip = fp.readULong();
			out[i].port = fp.readUShort();
			out[i].name = fp.readString(20);
			out[i].usercount = fp.readUShort();
			out[i].state = fp.readUShort();
			out[i].property = fp.readUShort();
			if (PACKETVER.value >= 20170315) {
				out[i].serverAddress = fp.readBinaryString(128);
				if (out[i].serverAddress && out[i].serverAddress.includes(':')) {
					[out[i].ip, out[i].port] = out[i].serverAddress.split(':');
					out[i].ip =
						out[i].ip.split('.').reduceRight((ip, octet, j) => ip + (parseInt(octet, 10) << (8 * j)), 0) >>>
						0;
					out[i].port = parseInt(out[i].port, 10);
				}
			}
		}
		return out;
	})();
};
PACKET.AC.ACCEPT_LOGIN3.size = -1;

// 0xac5
PACKET.HC.NOTIFY_ZONESVR2 = function PACKET_HC_NOTIFY_ZONESVR2(fp, end) {
	this.GID = fp.readULong();
	this.mapName = fp.readBinaryString(16);
	this.addr = {};
	this.addr.ip = fp.readULong();
	this.addr.port = fp.readUShort();

	if (PACKETVER.value >= 20170315) {
		this.serverAddress = fp.readBinaryString(128);
		if (this.serverAddress && this.serverAddress.includes(':')) {
			[this.addr.ip, this.addr.port] = this.serverAddress.split(':');
			this.addr.ip =
				this.addr.ip.split('.').reduceRight((ip, octet, i) => ip + (parseInt(octet, 10) << (8 * i)), 0) >>> 0;
			this.addr.port = parseInt(this.addr.port, 10);
		}
	}
};
PACKET.HC.NOTIFY_ZONESVR2.size = 156;

// 0xac7
PACKET.ZC.NPCACK_SERVERMOVE2 = function PACKET_ZC_NPCACK_SERVERMOVE2(fp, end) {
	this.mapName = fp.readBinaryString(16);
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.addr = {};
	this.addr.ip = fp.readULong();
	this.addr.port = fp.readUShort();

	if (PACKETVER.value >= 20170315) {
		this.serverAddress = fp.readBinaryString(128);
		if (this.serverAddress && this.serverAddress.includes(':')) {
			[this.addr.ip, this.addr.port] = this.serverAddress.split(':');
			this.addr.ip =
				this.addr.ip.split('.').reduceRight((ip, octet, i) => ip + (parseInt(octet, 10) << (8 * i)), 0) >>> 0;
			this.addr.port = parseInt(this.addr.port, 10);
		}
	}
};
PACKET.ZC.NPCACK_SERVERMOVE2.size = 156;

// 0xacb
PACKET.ZC.LONGLONGPAR_CHANGE = function PACKET_ZC_LONGLONGPAR_CHANGE(fp, end) {
	this.varID = fp.readUShort();
	this.amount = fp.readInt64();
};
PACKET.ZC.LONGLONGPAR_CHANGE.size = 12;

// 0xacc
PACKET.ZC.NOTIFY_EXP2 = function PACKET_ZC_NOTIFY_EXP2(fp, end) {
	this.AID = fp.readULong();
	this.amount = fp.readLong();
	this.amount2 = fp.readLong();
	this.varID = fp.readUShort();
	this.expType = fp.readShort();
};
PACKET.ZC.NOTIFY_EXP2.size = 18;

// 0xa3b
PACKET.ZC.HAT_EFFECT = function PACKET_ZC_HAT_EFFECT(fp, end) {
	this.GID = fp.readULong();
	this.enabled = fp.readChar(); // Always 1
	this.hatEffectIDs = (function () {
		const count = ((end - fp.tell()) / 2) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.HAT_EFFECT.size = -1;

// 0xada
PACKET.ZC.BROADCAST_ITEMREFINING_RESULT = function PACKET_ZC_BROADCAST_ITEMREFINING_RESULT(fp, end) {
	this.charName = fp.readString(NAME_LENGTH);
	this.itemId = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.refineLevel = fp.readChar();
	this.status = fp.readChar();
};
PACKET.ZC.BROADCAST_ITEMREFINING_RESULT.size = PACKETVER.value >= 20181121 ? 32 : 30;

// 0xadc
PACKET.ZC.CONFIG_NOTIFY4 = function PACKET_ZC_CONFIG_NOTIFY4(fp, end) {
	this.show_eq_flag = fp.readUChar();
	this.call_flag = fp.readUChar();
	this.pet_autofeeding_flag = fp.readUChar();
	this.homunculus_autofeeding_flag = fp.readUChar();
};
PACKET.ZC.CONFIG_NOTIFY4.size = 6;

// 0xadd
PACKET.ZC.ITEM_FALL_ENTRY3 = function PACKET_ZC_ITEM_FALL_ENTRY3(fp, end) {
	this.ITAID = fp.readULong();
	this.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.type = fp.readUShort();
	this.IsIdentified = fp.readUChar();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.subX = fp.readUChar();
	this.subY = fp.readUChar();
	this.count = fp.readUShort();
	this.showdropeffect = fp.readUChar();
	this.dropeffectmode = fp.readShort();
};
PACKET.ZC.ITEM_FALL_ENTRY3.size = PACKETVER.value >= 20181121 ? 24 : 22;

// 0xade
PACKET.ZC.RECOVER_PENALTY_OVERWEIGHT = function PACKET_ZC_RECOVER_PENALTY_OVERWEIGHT(fp, end) {
	this.percentage = fp.readULong();
};
PACKET.ZC.RECOVER_PENALTY_OVERWEIGHT.size = 6;

// 0xadf
PACKET.ZC.ACK_REQNAMEALL3 = function PACKET_ZC_ACK_REQNAMEALL3(fp, end) {
	this.AID = fp.readULong(); // account ID
	this.GID = fp.readULong(); // group ID
	this.CName = fp.readString(NAME_LENGTH); // name
	this.Title = fp.readString(NAME_LENGTH); // title (guild name)
};
PACKET.ZC.ACK_REQNAMEALL3.size = 58;

// 0xae4
PACKET.ZC.ADD_MEMBER_TO_GROUP4 = function PACKET_ZC_ADD_MEMBER_TO_GROUP4(fp, end) {
	this.AID = fp.readULong();
	this.GID = fp.readULong();
	this.role = fp.readULong();
	this.class_ = fp.readShort();
	this.baseLevel = fp.readShort();
	this.xPos = fp.readShort();
	this.yPos = fp.readShort();
	this.state = fp.readUChar();
	this.groupName = fp.readString(NAME_LENGTH);
	this.characterName = fp.readString(NAME_LENGTH);
	this.mapName = fp.readBinaryString(16);
	this.ItemPickupRule = fp.readUChar();
	this.ItemDivisionRule = fp.readUChar();
};
PACKET.ZC.ADD_MEMBER_TO_GROUP4.size = 89;

// 0xae5
PACKET.ZC.GROUP_LIST3 = function PACKET_ZC_GROUP_LIST3(fp, end) {
	this.groupName = fp.readBinaryString(NAME_LENGTH);
	this.groupInfo = (function () {
		const count = ((end - fp.tell()) / 54) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AID = fp.readULong();
			out[i].GID = fp.readULong();
			out[i].characterName = fp.readBinaryString(NAME_LENGTH);
			out[i].mapName = fp.readBinaryString(16);
			out[i].role = fp.readUChar();
			out[i].state = fp.readUChar();
			out[i].class_ = fp.readUShort();
			out[i].baseLevel = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.GROUP_LIST3.size = -1;

// 0xaef
PACKET.CZ.REQ_CHECK_ATTENDANCE = function PACKET_CZ_REQ_CHECK_ATTENDANCE() {};
PACKET.CZ.REQ_CHECK_ATTENDANCE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);

	pkt_buf.writeShort(0xaef);
	return pkt_buf;
};

// 0xaf4
PACKET.CZ.USE_SKILL_TOGROUND3 = function PACKET_CZ_USE_SKILL_TOGROUND3() {
	this.selectedLevel = 0;
	this.SKID = 0;
	this.xPos = 0;
	this.yPos = 0;
	this.unknown = 0;
};
PACKET.CZ.USE_SKILL_TOGROUND3.prototype.build = function () {
	const pkt = new BinaryWriter(11);

	pkt.writeShort(0x0af4);
	pkt.writeShort(this.selectedLevel, true);
	pkt.writeShort(this.SKID, true);
	pkt.writeShort(this.xPos, true);
	pkt.writeShort(this.yPos, true);
	pkt.writeUChar(this.unknown, true);
	return pkt;
};

// 0x0af6 - ZC_ACK_RANKING (PACKETVER >= 20190731, unified ranking response with char IDs)
// rankType: 0=Blacksmith, 1=Alchemist, 2=Taekwon, 3=PK
PACKET.ZC.ACK_RANKING2 = function PACKET_ZC_ACK_RANKING2(fp, end) {
	this.rankType = fp.readShort();
	this.CharID = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
	this.Point = (function () {
		const count = 10,
			out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = fp.readLong();
		}
		return out;
	})();
	this.myPoints = fp.readLong();
};
PACKET.ZC.ACK_RANKING2.size = 88;

// 0xaf7
PACKET.ZC.ACK_REQNAME_BYGID2 = function PACKET_ZC_ACK_REQNAME_BYGID2(fp, end) {
	this.flag = fp.readUShort();
	this.GID = fp.readULong();
	this.CName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.ACK_REQNAME_BYGID2.size = 32;

// 0xafb
PACKET.ZC.AUTOSPELLLIST2 = function PACKET_ZC_AUTOSPELLLIST2(fp, end) {
	const count = (end - fp.tell()) >> 2;
	const out = new Array(count);
	for (let i = 0; i < count; ++i) {
		out[i] = fp.readLong();
	}
	this.SKID = out;
};
PACKET.ZC.AUTOSPELLLIST2.size = -1;

// 0xafe
PACKET.ZC.UPDATE_MISSION_HUNT4 = function PACKET_ZC_UPDATE_MISSION_HUNT4(fp, end) {
	this.questCount = fp.readShort();
	this.hunt = (function () {
		const count = ((end - fp.tell()) / 16) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].huntID = fp.readULong();
			out[i].huntIDCount = fp.readULong();
			out[i].maxCount = fp.readShort();
			out[i].huntCount = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.UPDATE_MISSION_HUNT4.size = -1;

// 0xaff
PACKET.ZC.ALL_QUEST_LIST_V4 = function PACKET_ZC_ALL_QUEST_LIST_V4(fp, end) {
	this.questCount = fp.readLong();
	this.QuestList = (function (questCount) {
		const count = questCount;
		const out = new Array(questCount);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].questID = fp.readULong();
			out[i].active = fp.readUChar();
			out[i].quest_svrTime = fp.readULong();
			out[i].quest_endTime = fp.readULong();
			out[i].count = fp.readShort();
			out[i].hunt = new Array(out[i].count);
			for (let j = 0; j < out[i].count; j++) {
				out[i].hunt[j] = {};
				out[i].hunt[j].huntID = fp.readULong();
				out[i].hunt[j].huntIDCount = fp.readULong();
				out[i].hunt[j].mobType = fp.readULong();
				out[i].hunt[j].mobGID = fp.readULong();
				out[i].hunt[j].lvlMin = fp.readShort();
				out[i].hunt[j].lvlMax = fp.readShort();
				out[i].hunt[j].huntCount = fp.readShort();
				out[i].hunt[j].maxCount = fp.readShort();
				out[i].hunt[j].mobName = fp.readString(NAME_LENGTH);
			}
		}
		return out;
	})(this.questCount);
};
PACKET.ZC.ALL_QUEST_LIST_V4.size = -1;

// 0xb02
PACKET.AC.REFUSE_LOGIN_EX = function PACKET_AC_REFUSE_LOGIN_EX(fp, end) {
	this.ErrorCode = fp.readULong();
	this.blockDate = fp.readBinaryString(20);
};
PACKET.AC.REFUSE_LOGIN_EX.size = 26;

//0xb03
PACKET.ZC.EQUIPWIN_MICROSCOPE_V6 = function PACKET_ZC_EQUIPWIN_MICROSCOPE_V6(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.Robe = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.body2 = fp.readShort();
	this.sex = fp.readUChar();
	const option = new Struct('short index', 'short value', 'char param');
	const item_size = 67;
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / item_size) | 0;
		const out = new Array(count);
		let flag;
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readULong();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readULong();
			out[i].slot.card2 = fp.readULong();
			out[i].slot.card3 = fp.readULong();
			out[i].slot.card4 = fp.readULong();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			out[i].nRandomOptionCnt = fp.readChar();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.EQUIPWIN_MICROSCOPE_V6.size = -1;

// 0xb05
PACKET.ZC.STORE_ASSISTANT_ENTRY_V2 = function PACKET_ZC_STORE_ASSISTANT_ENTRY_V2(fp, end) {
	this.GID = fp.readULong();
	this.job = fp.readShort();
	this.unknown = fp.readShort();
	this.PosDir = [fp.readShort(), fp.readShort(), 4];
	this.sex = fp.readUChar();
	this.head = fp.readShort();
	this.weapon = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.shield = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.Robe = fp.readShort();
	this.name = fp.readBinaryString(NAME_LENGTH);
	this.unknown2 = fp.readShort();

	// fill necessary vars to render a player
	this.speed = 0;
	this.objecttype = 0;
	this.bodyState = 0;
	this.healthState = 0;
	this.effectState = 0;
	this.headDir = 0;
	this.GUID = 0;
	this.GEmblemVer = 0;
	this.honor = 0;
	this.virtue = 0;
	this.isPKModeON = 0;
	this.xSize = 0;
	this.ySize = 0;
	this.state = 0;
	this.clevel = 0;
};
PACKET.ZC.STORE_ASSISTANT_ENTRY_V2.size = PACKETVER.value >= 20181121 ? 63 : 59;

//0xb08
PACKET.ZC.SPLIT_SEND_ITEMLIST_SET = function PACKET_SPLIT_SEND_ITEMLIST_SET(fp, end) {
	this.invType = fp.readUChar();
	this.name = fp.readBinaryString(end - fp.tell());
};
PACKET.ZC.SPLIT_SEND_ITEMLIST_SET.size = -1;

//0xb09
PACKET.ZC.SPLIT_SEND_ITEMLIST_NORMAL = function PACKET_ZC_SPLIT_SEND_ITEMLIST_NORMAL(fp, end) {
	this.invType = fp.readUChar();

	const item_size = PACKETVER.value >= 20181121 ? 34 : 24;
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / item_size) | 0;
		const out = new Array(count);
		let flag;
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].count = fp.readShort();
			out[i].WearState = fp.readULong();
			out[i].slot = {};
			out[i].slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].PlaceETCTab = flag & 2;
		}
		return out;
	})();
};
PACKET.ZC.SPLIT_SEND_ITEMLIST_NORMAL.size = -1;

//0xb0a
PACKET.ZC.SPLIT_SEND_ITEMLIST_EQUIP = function PACKET_ZC_SPLIT_SEND_ITEMLIST_EQUIP(fp, end) {
	this.invType = fp.readUChar();
	const option = new Struct('short index', 'short value', 'char param');
	const item_size = PACKETVER.value >= 20181121 ? 67 : 57;
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / item_size) | 0;
		const out = new Array(count);
		let flag;
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].type = fp.readUChar();

			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			out[i].nRandomOptionCnt = fp.readChar();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.SPLIT_SEND_ITEMLIST_EQUIP.size = -1;

//0xb0b
PACKET.ZC.SPLIT_SEND_ITEMLIST_RESULT = function PACKET_ZC_SPLIT_SEND_ITEMLIST_RESULT(fp, end) {
	this.invType = fp.readUChar();
	this.flag = fp.readUChar();
};
PACKET.ZC.SPLIT_SEND_ITEMLIST_RESULT.size = 4;

// 0xb0c
PACKET.ZC.ADD_QUEST3 = function PACKET_ZC_ADD_QUEST3(fp, end) {
	this.questID = fp.readULong();
	this.active = fp.readUChar();
	this.quest_svrTime = fp.readLong();
	this.quest_endTime = fp.readLong();
	this.count = fp.readShort();
	this.hunt = (function (count) {
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].huntID = fp.readULong();
			out[i].huntIDCount = fp.readULong();
			out[i].mobType = fp.readULong();
			out[i].mobGID = fp.readULong();
			out[i].lvlMin = fp.readShort();
			out[i].lvlMax = fp.readShort();
			out[i].huntCount = fp.readShort();
			out[i].mobName = fp.readString(NAME_LENGTH);
		}
		return out;
	})(this.count);
};
PACKET.ZC.ADD_QUEST3.size = 155;

// 0xb0f
PACKET.CZ.NPC_BARTER_MARKET_PURCHASE = function PACKET_CZ_NPC_BARTER_MARKET_PURCHASE() {
	this.itemList = [];
};
PACKET.CZ.NPC_BARTER_MARKET_PURCHASE.prototype.build = function () {
	const item_size = PACKETVER.value >= 20181121 ? 14 : 12;
	const pkt_len = 4 + this.itemList.length * item_size;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb0f);
	pkt_buf.writeShort(pkt_len);

	for (let i = 0; i < this.itemList.length; ++i) {
		if (PACKETVER.value >= 20181121) {
			pkt_buf.writeULong(this.itemList[i].itemId);
		} else {
			pkt_buf.writeUShort(this.itemList[i].itemId);
		}
		pkt_buf.writeULong(this.itemList[i].amount);
		pkt_buf.writeUShort(this.itemList[i].invIndex);
		pkt_buf.writeULong(this.itemList[i].shopIndex);
	}

	return pkt_buf;
};

// 0xb1b
// this means that player is allowed to do actions
PACKET.ZC.NOTIFY_ACTORINIT2 = function PACKET_ZC_NOTIFY_ACTORINIT2(fp, end) {};
PACKET.ZC.NOTIFY_ACTORINIT2.size = 2;

// 0xb12
PACKET.CZ.NPC_BARTER_MARKET_CLOSE = function PACKET_CZ_NPC_BARTER_MARKET_CLOSE() {};
PACKET.CZ.NPC_BARTER_MARKET_CLOSE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);
	pkt_buf.writeShort(0xb12);
	return pkt_buf;
};

// 0xb14
PACKET.CZ.REQ_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE = function PACKET_CZ_REQ_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE() {};
PACKET.CZ.REQ_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);
	pkt_buf.writeShort(0xb14);
	return pkt_buf;
};

// 0xb15
PACKET.ZC.ACK_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE = function PACKET_ZC_ACK_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE(fp, end) {
	this.result = fp.readUChar();
	this.itemId = fp.readULong();
};
PACKET.ZC.ACK_OPEN_MSGBOX_EXTEND_BODYITEM_SIZE.size = 7;

// 0xb16
PACKET.CZ.REQ_EXTEND_BODYITEM_SIZE = function PACKET_CZ_REQ_EXTEND_BODYITEM_SIZE() {};
PACKET.CZ.REQ_EXTEND_BODYITEM_SIZE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);
	pkt_buf.writeShort(0xb16);
	return pkt_buf;
};

// 0xb17
PACKET.ZC.ACK_EXTEND_BODYITEM_SIZE = function PACKET_ZC_ACK_EXTEND_BODYITEM_SIZE(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_EXTEND_BODYITEM_SIZE.size = 3;

// 0xb18
PACKET.ZC.EXTEND_BODYITEM_SIZE = function PACKET_ZC_EXTEND_BODYITEM_SIZE(fp, end) {
	this.type = fp.readUShort();
};
PACKET.ZC.EXTEND_BODYITEM_SIZE.size = 4;

// 0xb19
PACKET.CZ.CLOSE_MSGBOX_EXTEND_BODYITEM_SIZE = function PACKET_CZ_CLOSE_MSGBOX_EXTEND_BODYITEM_SIZE() {};
PACKET.CZ.CLOSE_MSGBOX_EXTEND_BODYITEM_SIZE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);
	pkt_buf.writeShort(0xb19);
	return pkt_buf;
};

// 0xb1c
PACKET.CZ.PING_LIVE = function PACKET_CZ_PING_LIVE() {};
PACKET.CZ.PING_LIVE.prototype.build = function () {
	const pkt_buf = new BinaryWriter(2);
	pkt_buf.writeShort(0xb1c);
	return pkt_buf;
};

// 0xb1d
PACKET.ZC.PING_LIVE = function PACKET_ZC_PING_LIVE(fp, end) {};
PACKET.ZC.PING_LIVE.size = 2;

// 0xb20
PACKET.ZC.SHORTCUT_KEY_LIST_V4 = function PACKET_ZC_SHORTCUT_KEY_LIST_V4(fp, end) {
	this.rotate = fp.readUChar();
	this.tab = fp.readUShort();
	this.ShortCutKey = (function () {
		const count = 38;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].isSkill = fp.readChar();
			out[i].ID = fp.readULong();
			out[i].count = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.SHORTCUT_KEY_LIST_V4.size = 271;

// 0xb21
PACKET.CZ.SHORTCUT_KEY_CHANGE2 = function PACKET_CZ_SHORTCUT_KEY_CHANGE2() {
	this.Index = 0;
	this.ShortCutKey = {};
	this.tab = 0;
};
PACKET.CZ.SHORTCUT_KEY_CHANGE2.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 1 + 4 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb21);
	pkt_buf.writeShort(this.tab);
	pkt_buf.writeUShort(this.Index);
	pkt_buf.writeChar(this.ShortCutKey.isSkill);
	pkt_buf.writeULong(this.ShortCutKey.ID);
	pkt_buf.writeShort(this.ShortCutKey.count);
	return pkt_buf;
};

// 0xb27
PACKET.ZC.GUILD_AGIT_INFO = function PACKET_ZC_GUILD_AGIT_INFO(fp, end) {
	this.castle_list = fp.readString(end - fp.tell()); // check this
};
PACKET.ZC.GUILD_AGIT_INFO.size = -1;

// 0xb2a
PACKET.ZC.ADD_SKILL2 = function PACKET_ZC_ADD_SKILL2(fp, end) {
	this.data = {};
	this.data.SKID = fp.readUShort();
	this.data.type = fp.readLong();
	this.data.level = fp.readShort();
	this.data.spcost = fp.readShort();
	this.data.attackRange = fp.readShort();
	this.data.upgradable = fp.readUChar();
};
PACKET.ZC.ADD_SKILL2.size = 15;

// 0xb2f
PACKET.ZC.PROPERTY_HOMUN3 = function PACKET_ZC_PROPERTY_HOMUN3(fp, end) {
	this.szName = fp.readString(NAME_LENGTH); // <name>.24B
	this.bModified = fp.readUChar(); // <modified>.B
	this.nLevel = fp.readShort(); // <level>.W
	this.nFullness = fp.readShort(); // <hunger>.W
	this.nRelationship = fp.readShort(); // <intimacy>.W

	this.atk = fp.readShort(); // <atk>.W
	this.Matk = fp.readShort(); // <matk>.W
	this.hit = fp.readShort(); // <hit>.W
	this.critical = fp.readShort(); // <crit>.W
	this.def = fp.readShort(); // <def>.W
	this.Mdef = fp.readShort(); // <mdef>.W
	this.flee = fp.readShort(); // <flee>.W
	this.aspd = fp.readShort(); // <aspd>.W // todo wrong

	this.hp = fp.readLong(); // <hp>.L
	this.maxHP = fp.readLong(); // <max hp>.L
	this.sp = fp.readShort(); // <sp>.W
	this.maxSP = fp.readShort(); // <max sp>.W
	this.exp = fp.readLong(); // <exp>.L
	this.maxEXP = fp.readLong(); // <max exp>.L
	this.SKPoint = fp.readShort(); // <skill points>.W
	this.ATKRange = fp.readShort(); // <atk range>.W
};
PACKET.ZC.PROPERTY_HOMUN3.size = 73;

// 0xb31
PACKET.ZC.ADD_SKILL3 = function PACKET_ZC_ADD_SKILL3(fp, end) {
	this.data = {};
	this.data.SKID = fp.readUShort();
	this.data.type = fp.readLong();
	this.data.level = fp.readShort();
	this.data.spcost = fp.readShort();
	this.data.attackRange = fp.readShort();
	this.data.upgradable = fp.readUChar();
	this.data.level2 = fp.readShort();
};
PACKET.ZC.ADD_SKILL3.size = 17;

// 0xb32
PACKET.ZC.SKILLINFO_LIST2 = function PACKET_ZC_SKILLINFO_LIST2(fp, end) {
	this.skillList = (function () {
		const count = ((end - fp.tell()) / 15) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].SKID = fp.readShort();
			out[i].type = fp.readLong();
			out[i].level = fp.readShort();
			out[i].spcost = fp.readShort();
			out[i].attackRange = fp.readShort();
			out[i].upgradable = fp.readChar();
			out[i].level2 = fp.readShort();
		}
		return out;
	})();
};
PACKET.ZC.SKILLINFO_LIST2.size = -1;

// 0xb33
PACKET.ZC.SKILLINFO_UPDATE3 = function PACKET_ZC_SKILLINFO_UPDATE3(fp, end) {
	this.SKID = fp.readShort();
	this.type = fp.readLong();
	this.level = fp.readShort();
	this.spcost = fp.readShort();
	this.attackRange = fp.readShort();
	this.upgradable = fp.readChar();
	this.level2 = fp.readShort();
};
PACKET.ZC.SKILLINFO_UPDATE3.size = 17;

//0xb37
PACKET.ZC.EQUIPWIN_MICROSCOPE_V7 = function PACKET_ZC_EQUIPWIN_MICROSCOPE_V7(fp, end) {
	this.characterName = fp.readString(NAME_LENGTH);
	this.job = fp.readShort();
	this.head = fp.readShort();
	this.accessory = fp.readShort();
	this.accessory2 = fp.readShort();
	this.accessory3 = fp.readShort();
	this.Robe = fp.readShort();
	this.headpalette = fp.readShort();
	this.bodypalette = fp.readShort();
	this.body2 = fp.readShort();
	this.sex = fp.readUChar();
	const option = new Struct('short index', 'short value', 'char param');
	const item_size = 68;
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / item_size) | 0;
		const out = new Array(count);
		let flag;
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readULong();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].slot = {};
			out[i].slot.card1 = fp.readULong();
			out[i].slot.card2 = fp.readULong();
			out[i].slot.card3 = fp.readULong();
			out[i].slot.card4 = fp.readULong();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			out[i].nRandomOptionCnt = fp.readChar();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			out[i].RefiningLevel = fp.readUChar();
			out[i].enchantgrade = fp.readUChar();
			flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.EQUIPWIN_MICROSCOPE_V7.size = -1;

//0xb39
PACKET.ZC.SPLIT_SEND_ITEMLIST_EQUIP2 = function PACKET_ZC_SPLIT_SEND_ITEMLIST_EQUIP2(fp, end) {
	this.invType = fp.readUChar();
	const option = new Struct('short index', 'short value', 'char param');
	const item_size = 68;
	this.ItemInfo = (function () {
		const count = ((end - fp.tell()) / item_size) | 0;
		const out = new Array(count);
		let flag;
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readULong();
			out[i].type = fp.readUChar();
			out[i].location = fp.readULong();
			out[i].WearState = fp.readULong();
			out[i].slot = {};
			out[i].slot.card1 = fp.readULong();
			out[i].slot.card2 = fp.readULong();
			out[i].slot.card3 = fp.readULong();
			out[i].slot.card4 = fp.readULong();
			out[i].HireExpireDate = fp.readLong();
			out[i].bindOnEquipType = fp.readUShort();
			out[i].wItemSpriteNumber = fp.readUShort();
			out[i].nRandomOptionCnt = fp.readChar();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			out[i].RefiningLevel = fp.readUChar();
			out[i].enchantgrade = fp.readUChar();
			flag = fp.readUChar();
			out[i].IsIdentified = flag & 1;
			out[i].IsDamaged = flag & 2;
			out[i].PlaceETCTab = flag & 4;
		}
		return out;
	})();
};
PACKET.ZC.SPLIT_SEND_ITEMLIST_EQUIP2.size = -1;

// 0xb25 - not imlemented on rAthena
PACKET.ZC.PAR_4JOB_CHANGE = function PACKET_ZC_PAR_4JOB_CHANGE(fp, end) {
	this.varID = fp.readUShort();
	this.count = fp.readUShort();
};
PACKET.ZC.PAR_4JOB_CHANGE.size = 6;

// 0xb3d
PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC3 = function PACKET_ZC_PC_PURCHASE_ITEMLIST_FROMMC3(fp, end) {
	this.AID = fp.readULong();
	this.UniqueID = fp.readULong();
	this.itemList = (function () {
		const count = (end - fp.tell()) / (4 + 2 + 2 + 1 + 4 + 1 + 1 + 16 + 25 + 4 + 2 + 1 + 1); //Item options 25 bytes, (location viewSprite), itemId use Long now, grade
		const out = new Array(count);

		const option = new Struct('short index', 'short value', 'char param');

		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].count = fp.readShort();
			out[i].index = fp.readShort();
			out[i].type = fp.readUChar();
			out[i].ITID = fp.readULong();
			out[i].IsIdentified = fp.readUChar();
			out[i].IsDamaged = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readULong();
			out[i].slot.card2 = fp.readULong();
			out[i].slot.card3 = fp.readULong();
			out[i].slot.card4 = fp.readULong();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			out[i].location = fp.readULong();
			out[i].viewSprite = fp.readUShort();
			out[i].RefiningLevel = fp.readUChar();
			out[i].grade = fp.readUChar();
		}
		return out;
	})();
};
PACKET.ZC.PC_PURCHASE_ITEMLIST_FROMMC3.size = -1;

// 0xb40
PACKET.ZC.PC_PURCHASE_MYITEMLIST2 = function PACKET_ZC_PC_PURCHASE_MYITEMLIST2(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.AID = fp.readULong();
	this.itemList = (function () {
		const len = 4 + 2 + 2 + 1 + 4 + 1 + 1 + 16 + 25 + 1 + 1;

		const count = ((end - fp.tell()) / len) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].price = fp.readLong();
			out[i].index = fp.readShort();
			out[i].count = fp.readShort();
			out[i].type = fp.readUChar();
			out[i].ITID = fp.readULong();
			out[i].IsIdentified = fp.readUChar();
			out[i].IsDamaged = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readULong();
			out[i].slot.card2 = fp.readULong();
			out[i].slot.card3 = fp.readULong();
			out[i].slot.card4 = fp.readULong();
			out[i].Options = [];
			out[i].Options[1] = fp.readStruct(option);
			out[i].Options[2] = fp.readStruct(option);
			out[i].Options[3] = fp.readStruct(option);
			out[i].Options[4] = fp.readStruct(option);
			out[i].Options[5] = fp.readStruct(option);
			out[i].RefiningLevel = fp.readUChar();
			out[i].grade = fp.readUChar();
		}
		return out;
	})();
};
PACKET.ZC.PC_PURCHASE_MYITEMLIST2.size = -1;

// 0xb41
PACKET.ZC.ITEM_PICKUP_ACK8 = function PACKET_ZC_ITEM_PICKUP_ACK8(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');
	this.index = fp.readUShort();
	this.count = fp.readUShort();
	this.ITID = fp.readULong();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readULong();
	this.slot.card2 = fp.readULong();
	this.slot.card3 = fp.readULong();
	this.slot.card4 = fp.readULong();
	this.location = fp.readLong();
	this.type = fp.readUChar();
	this.result = fp.readUChar();
	this.HireExpireDate = fp.readLong();
	this.bindOnEquipType = fp.readUShort();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.favorite = fp.readUChar();
	this.look = fp.readUShort();
	this.RefiningLevel = fp.readUChar();
	this.enchantgrade = fp.readUChar();
};
PACKET.ZC.ITEM_PICKUP_ACK8.size = 70;

// 0xb42
PACKET.ZC.ADD_EXCHANGE_ITEM5 = function PACKET_ZC_ADD_EXCHANGE_ITEM5(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.ITID = fp.readULong();
	this.type = fp.readUChar();
	this.count = fp.readLong();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readULong();
	this.slot.card2 = fp.readULong();
	this.slot.card3 = fp.readULong();
	this.slot.card4 = fp.readULong();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.location = fp.readULong();
	this.viewSprite = fp.readUShort();
	this.RefiningLevel = fp.readUChar();
	this.grade = fp.readUChar();
};
PACKET.ZC.ADD_EXCHANGE_ITEM5.size = 62;

// 0xb43
PACKET.ZC.CHANGE_ITEM_OPTION = function PACKET_ZC_CHANGE_ITEM_OPTION(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.index = fp.readShort();
	this.isDamaged = fp.readChar();
	this.slot = {};
	this.slot.card1 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card2 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card3 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.slot.card4 = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.refiningLevel = fp.readUChar();
	this.enchantgrade = fp.readUChar();
};
PACKET.ZC.CHANGE_ITEM_OPTION.size = PACKETVER.value >= 20181121 ? 48 : 44;

// 0xb44
PACKET.ZC.ADD_ITEM_TO_STORE4 = function PACKET_ZC_ADD_ITEM_TO_STORE4(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.index = fp.readShort();
	this.count = fp.readLong();
	this.ITID = fp.readULong();
	this.type = fp.readUChar();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readULong();
	this.slot.card2 = fp.readULong();
	this.slot.card3 = fp.readULong();
	this.slot.card4 = fp.readULong();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.RefiningLevel = fp.readUChar();
	this.grade = fp.readUChar();
};
PACKET.ZC.ADD_ITEM_TO_STORE4.size = 58;

// 0xb45
PACKET.ZC.ADD_ITEM_TO_CART4 = function PACKET_ZC_ADD_ITEM_TO_CART4(fp, end) {
	const option = new Struct('short index', 'short value', 'char param');

	this.index = fp.readShort();
	this.count = fp.readLong();
	this.ITID = fp.readULong();
	this.type = fp.readUChar();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readULong();
	this.slot.card2 = fp.readULong();
	this.slot.card3 = fp.readULong();
	this.slot.card4 = fp.readULong();
	this.Options = [];
	this.Options[1] = fp.readStruct(option);
	this.Options[2] = fp.readStruct(option);
	this.Options[3] = fp.readStruct(option);
	this.Options[4] = fp.readStruct(option);
	this.Options[5] = fp.readStruct(option);
	this.RefiningLevel = fp.readUChar();
	this.grade = fp.readUChar();
};
PACKET.ZC.ADD_ITEM_TO_CART4.size = 58;

// 0xb4e
PACKET.ZC.NPC_MARKET_PURCHASE_RESULT2 = function PACKET_ZC_NPC_MARKET_PURCHASE_RESULT2(fp, end) {
	this.result = fp.readUShort();
	this.itemList = (function () {
		// Determine item size based on PACKETVER
		const item_size = PACKETVER.value >= 20181121 ? 10 : 8; // Adjust sizes based on nameid (4 or 2 bytes)
		const count = ((end - fp.tell()) / item_size) | 0; // Calculate item count
		const out = new Array(count);

		for (let i = 0; i < count; ++i) {
			out[i] = {};
			// Parse fields with conditional handling for nameid
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort(); // uint32 or uint16
			out[i].qty = fp.readUShort();
			out[i].price = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.NPC_MARKET_PURCHASE_RESULT2.size = -1;

// 0xb57
PACKET.CZ.NPC_EXPANDED_BARTER_MARKET_PURCHASE = function PACKET_CZ_NPC_EXPANDED_BARTER_MARKET_PURCHASE() {
	this.itemList = [];
};
PACKET.CZ.NPC_EXPANDED_BARTER_MARKET_PURCHASE.prototype.build = function () {
	const item_size = PACKETVER.value >= 20181121 ? 12 : 10;
	const pkt_len = 4 + this.itemList.length * item_size;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x0b57);
	pkt_buf.writeShort(pkt_len);

	for (let i = 0; i < this.itemList.length; ++i) {
		if (PACKETVER.value >= 20181121) {
			pkt_buf.writeULong(this.itemList[i].itemId);
		} else {
			pkt_buf.writeUShort(this.itemList[i].itemId);
		}
		pkt_buf.writeULong(this.itemList[i].shopIndex);
		pkt_buf.writeULong(this.itemList[i].amount);
	}

	return pkt_buf;
};

// 0xb58
PACKET.CZ.NPC_EXPANDED_BARTER_MARKET_CLOSE = function PACKET_CZ_NPC_EXPANDED_BARTER_MARKET_CLOSE() {};
PACKET.CZ.NPC_EXPANDED_BARTER_MARKET_CLOSE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb58);
	return pkt_buf;
};

// 0xb59
PACKET.CZ.GRADE_ENCHANT_SELECT_EQUIPMENT = function PACKET_CZ_GRADE_ENCHANT_SELECT_EQUIPMENT() {
	this.index = 0;
};
PACKET.CZ.GRADE_ENCHANT_SELECT_EQUIPMENT.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb59);
	pkt_buf.writeUChar(this.index);
	return pkt_buf;
};

// 0xb65
PACKET.ZC.REPAIRITEMLIST2 = function PACKET_ZC_REPAIRITEMLIST2(fp, end) {
	this.itemList = (function () {
		const count = ((end - fp.tell()) / 13) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].index = fp.readShort();
			out[i].ITID = fp.readULong();
			out[i].RefiningLevel = fp.readUChar();
			out[i].slot = {};
			out[i].slot.card1 = fp.readULong();
			out[i].slot.card2 = fp.readULong();
			out[i].slot.card3 = fp.readULong();
			out[i].slot.card4 = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.REPAIRITEMLIST2.size = -1;

// 0xb67
PACKET.ZC.ITEM_PICKUP_PARTY2 = function PACKET_ZC_ITEM_PICKUP_PARTY2(fp, end) {
	this.AID = fp.readULong();
	this.ITID = fp.readULong();
	this.IsIdentified = fp.readUChar();
	this.IsDamaged = fp.readUChar();
	this.slot = {};
	this.slot.card1 = fp.readULong();
	this.slot.card2 = fp.readULong();
	this.slot.card3 = fp.readULong();
	this.slot.card4 = fp.readULong();
	this.location = fp.readUShort();
	this.itemType = fp.readUChar();
	this.RefiningLevel = fp.readUChar();
	this.grade = fp.readUChar();
};
PACKET.ZC.ITEM_PICKUP_PARTY2.size = 33;

// 0b7a
PACKET.ZC.NPC_MARKET_OPEN2 = function PACKET_ZC_NPC_MARKET_OPEN2(fp, end) {
	this.itemList = (function () {
		// Determine item size based on PACKETVER
		const item_size = PACKETVER.value >= 20181121 ? 19 : 17; // Adjust sizes based on nameid (4 or 2 bytes)
		const count = ((end - fp.tell()) / item_size) | 0; // Calculate item count
		const out = new Array(count);

		for (let i = 0; i < count; ++i) {
			out[i] = {};
			// Parse fields with conditional handling for nameid
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort(); // uint32 or uint16
			out[i].type = fp.readUChar();
			out[i].price = fp.readULong();
			out[i].qty = fp.readULong();
			out[i].weight = fp.readUShort();
			out[i].location = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.NPC_MARKET_OPEN2.size = -1;

// 0xb72
PACKET.HC.ACCEPT_ENTER_NEO_UNION_LIST2 = function PACKET_HC_ACCEPT_ENTER_NEO_UNION_LIST2(fp, end) {
	this.charInfo = PACKETVER.parseCharInfo(fp, end);
};
PACKET.HC.ACCEPT_ENTER_NEO_UNION_LIST2.size = -1;

// 0xb76
PACKET.ZC.PROPERTY_HOMUN4 = function PACKET_ZC_PROPERTY_HOMUN4(fp, end) {
	this.szName = fp.readString(NAME_LENGTH); // <name>.24B
	this.bModified = fp.readUChar(); // <modified>.B
	this.nLevel = fp.readShort(); // <level>.W
	this.nFullness = fp.readShort(); // <hunger>.W
	this.nRelationship = fp.readShort(); // <intimacy>.W

	this.atk = fp.readShort(); // <atk>.W
	this.Matk = fp.readShort(); // <matk>.W
	this.hit = fp.readShort(); // <hit>.W
	this.critical = fp.readShort(); // <crit>.W
	this.def = fp.readShort(); // <def>.W
	this.Mdef = fp.readShort(); // <mdef>.W
	this.flee = fp.readShort(); // <flee>.W
	this.aspd = fp.readShort(); // <aspd>.W // todo wrong

	this.hp = fp.readLong(); // <hp>.L
	this.maxHP = fp.readLong(); // <max hp>.L
	this.sp = fp.readLong(); // <sp>.L
	this.maxSP = fp.readLong(); // <max sp>.L
	this.exp = fp.readLong(); // <exp>.L
	this.maxEXP = fp.readLong(); // <max exp>.L
	this.SKPoint = fp.readShort(); // <skill points>.W
	this.ATKRange = fp.readShort(); // <atk range>.W
};
PACKET.ZC.PROPERTY_HOMUN4.size = 77;

// 0xb77
PACKET.ZC.PC_PURCHASE_ITEMLIST2 = function PACKET_ZC_PC_PURCHASE_ITEMLIST2(fp, end) {
	this.itemList = (function () {
		const item_size = 19;
		const count = ((end - fp.tell()) / item_size) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].ITID = fp.readULong();
			out[i].price = fp.readLong();
			out[i].discountprice = fp.readLong();
			out[i].type = fp.readUChar();
			out[i].viewSprite = fp.readUShort();
			out[i].location = fp.readULong();
		}
		return out;
	})();
};
PACKET.ZC.PC_PURCHASE_ITEMLIST2.size = -1;

// 0xb78
PACKET.ZC.NPC_BARTER_MARKET_ITEMINFO = function PACKET_ZC_NPC_BARTER_MARKET_ITEMINFO(fp, end) {
	this.itemList = (function () {
		const item_size = PACKETVER.value >= 20210203 ? 31 : PACKETVER.value >= 20181121 ? 25 : 21;
		const count = ((end - fp.tell()) / item_size) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].type = fp.readUChar();
			out[i].amount = fp.readULong();
			out[i].currencyITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			out[i].currencyamount = fp.readULong();
			out[i].weight = fp.readULong();
			out[i].index = fp.readULong();
			out[i].viewSprite = PACKETVER.value >= 20210203 ? fp.readUShort() : 0;
			out[i].location = PACKETVER.value >= 20210203 ? fp.readULong() : 0;
		}
		return out;
	})();
};
PACKET.ZC.NPC_BARTER_MARKET_ITEMINFO.size = -1;

// 0xb79
PACKET.ZC.NPC_EXPANDED_BARTER_MARKET_ITEMINFO = function PACKET_ZC_NPC_EXPANDED_BARTER_MARKET_ITEMINFO(fp, end) {
	// Ensure 'this' is properly bound and initialized
	const self = this;

	self.items_count = fp.readLong(); // Assign items_count to 'self' properly
	self.itemList = (function () {
		const item_size =
			PACKETVER.value >= 20250402 ? 36 : PACKETVER.value >= 20210203 ? 32 : PACKETVER.value >= 20181121 ? 26 : 24; // size of the `sub` structure
		const sub2_size = PACKETVER.value >= 20181121 ? 12 : 10; // size of the `sub2` structure
		const items = [];

		for (let i = 0; i < self.items_count; ++i) {
			if (fp.tell() + item_size > end) {
				console.error('Attempted to read beyond packet bounds');
				break;
			}

			const item = {};
			item.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
			item.type = fp.readUShort();
			item.amount = fp.readULong();
			item.weight = fp.readULong();
			item.index = fp.readULong();
			item.price = fp.readULong();
			item.viewSprite = PACKETVER.value >= 20210203 ? fp.readUShort() : 0;
			item.location = PACKETVER.value >= 20210203 ? fp.readULong() : 0;
			item.currency_count = fp.readULong();
			item.RefiningLevel = PACKETVER.value >= 20250402 ? fp.readULong() : 0;

			item.currencyList = [];
			for (let j = 0; j < item.currency_count; ++j) {
				if (fp.tell() + sub2_size > end) {
					console.error('Attempted to read beyond packet bounds in currencies');
					break;
				}

				const currency = {};
				currency.ITID = PACKETVER.value >= 20181121 ? fp.readULong() : fp.readUShort();
				currency.refine_level = fp.readUShort();
				currency.amount = fp.readULong();
				currency.type = fp.readUShort();
				item.currencyList.push(currency);
			}

			items.push(item);
		}

		return items;
	})();
};
PACKET.ZC.NPC_EXPANDED_BARTER_MARKET_ITEMINFO.size = -1;

// 0xb7b
PACKET.ZC.GUILD_INFO4 = function PACKET_ZC_GUILD_INFO4(fp, end) {
	this.GDID = fp.readLong();
	this.level = fp.readLong();
	this.userNum = fp.readLong();
	this.maxUserNum = fp.readLong();
	this.userAverageLevel = fp.readLong();
	this.exp = fp.readLong();
	this.maxExp = fp.readLong();
	this.point = fp.readLong();
	this.honor = fp.readLong();
	this.virtue = fp.readLong();
	this.emblemVersion = fp.readLong();
	this.guildname = fp.readString(NAME_LENGTH);
	this.manageLand = fp.readString(16);
	this.zeny = fp.readLong();
	this.masterAID = fp.readLong();
	this.masterName = fp.readString(NAME_LENGTH);
};
PACKET.ZC.GUILD_INFO4.size = 118; // - <master name>.24B + <master char id>.L

// 0xb7d
PACKET.ZC.MEMBERMGR_INFO3 = function PACKET_ZC_MEMBERMGR_INFO3(fp, end) {
	this.memberInfo = (function () {
		const count = ((end - fp.tell()) / 58) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].AID = fp.readULong();
			out[i].GID = fp.readULong();
			out[i].HeadType = fp.readShort();
			out[i].HeadPalette = fp.readShort();
			out[i].Sex = fp.readShort();
			out[i].Job = fp.readShort();
			out[i].Level = fp.readShort();
			out[i].MemberExp = fp.readLong();
			out[i].CurrentState = fp.readLong();
			out[i].GPositionID = fp.readLong();
			out[i].LastLogin = fp.readLong();
			out[i].CharName = fp.readString(NAME_LENGTH);
			out[i].Memo = '';
		}
		return out;
	})();
};
PACKET.ZC.MEMBERMGR_INFO3.size = -1;

// 0xb6f
PACKET.HC.ACCEPT_MAKECHAR = function PACKET_HC_ACCEPT_MAKECHAR(fp, end) {
	this.charinfo = PACKETVER.parseCharInfo(fp, end)[0];
};
PACKET.HC.ACCEPT_MAKECHAR.size = 0;

//0xb8d
PACKET.ZC.REPUTE_INFO = function PACKET_ZC_REPUTE_INFO(fp, end) {
	this.success = fp.readUChar();
	const repute_size = 16;
	this.reputeInfo = (function () {
		const count = ((end - fp.tell()) / repute_size) | 0;
		const out = new Array(count);
		for (let i = 0; i < count; ++i) {
			out[i] = {};
			out[i].type = fp.readInt64();
			out[i].points = fp.readInt64();
		}
		return out;
	})();
};
PACKET.ZC.REPUTE_INFO.size = -1;

// 0xb8e
PACKET.ZC_REPUTE_OPEN = function PACKET_ZC_REPUTE_OPEN(fp, end) {
	this.table = fp.readInt64();
	this.type = fp.readULong();
	this.mode = fp.readULong();
};
PACKET.ZC_REPUTE_OPEN.size = 18;

// 0xb8f
PACKET.ZC.OPEN_REFORM_UI = function PACKET_ZC_OPEN_REFORM_UI(fp, end) {
	this.ITID = fp.readLong();
};
PACKET.ZC.OPEN_REFORM_UI.size = 6;

// 0xb90
PACKET.CZ.CLOSE_REFORM_UI = function PACKET_CZ_CLOSE_REFORM_UI() {};
PACKET.CZ.CLOSE_REFORM_UI.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb90);
	return pkt_buf;
};

// 0xb91
PACKET.CZ.ITEM_REFORM = function PACKET_CZ_ITEM_REFORM() {
	this.ITID = 0;
	this.index = 0;
};
PACKET.CZ.ITEM_REFORM.prototype.build = function () {
	const pkt_len = 2 + 4 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xb91);
	pkt_buf.writeLong(this.ITID);
	pkt_buf.writeShort(this.index);
	return pkt_buf;
};

// 0xb92
PACKET.ZC.ITEM_REFORM_ACK = function PACKET_ZC_ITEM_REFORM_ACK(fp, end) {
	this.index = fp.readShort();
	this.result = fp.readChar();
};
PACKET.ZC.ITEM_REFORM_ACK.size = 5;

// 0xba4
PACKET.ZC.PROPERTY_HOMUN5 = function PACKET_ZC_PROPERTY_HOMUN5(fp, end) {
	this.szName = fp.readString(NAME_LENGTH); // <name>.24B
	this.bModified = fp.readUChar(); // <modified>.B
	this.nLevel = fp.readShort(); // <level>.W
	this.nFullness = fp.readShort(); // <hunger>.W
	this.nRelationship = fp.readShort(); // <intimacy>.W

	this.atk = fp.readShort(); // <atk>.W
	this.Matk = fp.readShort(); // <matk>.W
	this.hit = fp.readShort(); // <hit>.W
	this.critical = fp.readShort(); // <crit>.W
	this.def = fp.readShort(); // <def>.W
	this.Mdef = fp.readShort(); // <mdef>.W
	this.flee = fp.readShort(); // <flee>.W
	this.aspd = fp.readShort(); // <aspd>.W // todo wrong

	this.hp = fp.readLong(); // <hp>.L
	this.maxHP = fp.readLong(); // <max hp>.L
	this.sp = fp.readLong(); // <sp>.L
	this.maxSP = fp.readLong(); // <max sp>.L
	this.exp = fp.readUInt64(); // <exp>.L
	this.maxEXP = fp.readUInt64(); // <max exp>.L
	this.SKPoint = fp.readShort(); // <skill points>.W
	this.ATKRange = fp.readShort(); // <atk range>.W
};
PACKET.ZC.PROPERTY_HOMUN5.size = 85;

// 0xba5
PACKET.ZC.HO_PAR_CHANGE2 = function PACKET_ZC_HO_PAR_CHANGE2(fp, end) {
	this.param = fp.readUShort();
	this.value = fp.readUInt64();
};
PACKET.ZC.HO_PAR_CHANGE2.size = 10;

/**
 * GM COMMANDS
 */

// 0x99
PACKET.CZ.BROADCAST = function PACKET_CZ_BROADCAST() {
	this.msg = '';
};
PACKET.CZ.BROADCAST.prototype.build = function () {
	const pkt_len = 2 + 2 + this.msg.length + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x99);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x19c
PACKET.CZ.LOCALBROADCAST = function PACKET_CZ_LOCALBROADCAST() {
	this.msg = '';
};
PACKET.CZ.LOCALBROADCAST.prototype.build = function () {
	const pkt_len = 2 + 2 + this.msg.length + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x19c);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.msg);
	return pkt_buf;
};

// 0x140
PACKET.CZ.MOVETO_MAP = function PACKET_CZ_MOVETO_MAP() {
	this.mapName = '';
	this.xPos = 0;
	this.yPos = 0;
};
PACKET.CZ.MOVETO_MAP.prototype.build = function () {
	const pkt_len = 2 + 16 + 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x140);
	pkt_buf.writeBinaryString(this.mapName, 16);
	pkt_buf.writeShort(this.xPos);
	pkt_buf.writeShort(this.yPos);
	return pkt_buf;
};

// 0x1bd
PACKET.CZ.RECALL_GID = function PACKET_CZ_RECALL_GID() {
	this.CharacterName = '';
};
PACKET.CZ.RECALL_GID.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1bd);
	pkt_buf.writeString(this.CharacterName, 24);
	return pkt_buf;
};

// 0x1bc
PACKET.CZ.RECALL = function PACKET_CZ_RECALL() {
	this.AccountName = '';
};
PACKET.CZ.RECALL.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1bc);
	pkt_buf.writeString(this.AccountName, 24);
	return pkt_buf;
};

// 0x19d
PACKET.CZ.CHANGE_EFFECTSTATE = function PACKET_CZ_CHANGE_EFFECTSTATE() {
	this.EffectState = 0;
};
PACKET.CZ.CHANGE_EFFECTSTATE.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x19d);
	pkt_buf.writeLong(this.EffectState);
	return pkt_buf;
};

// 0xcc
PACKET.CZ.DISCONNECT_CHARACTER = function PACKET_CZ_DISCONNECT_CHARACTER() {
	this.AID = 0;
};
PACKET.CZ.DISCONNECT_CHARACTER.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xcc);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};

// 0xce
PACKET.CZ.DISCONNECT_ALL_CHARACTER = function PACKET_CZ_DISCONNECT_ALL_CHARACTER() {};
PACKET.CZ.DISCONNECT_ALL_CHARACTER.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0xce);
	return pkt_buf;
};

// 0x13f
PACKET.CZ.ITEM_CREATE = function PACKET_CZ_ITEM_CREATE() {
	this.itemName = '';
};
PACKET.CZ.ITEM_CREATE.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x13f);
	pkt_buf.writeString(this.itemName, 24);
	return pkt_buf;
};

// 0x197
PACKET.CZ.RESET = function PACKET_CZ_RESET() {
	this.type = 0;
};
PACKET.CZ.RESET.prototype.build = function () {
	const pkt_len = 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x197);
	pkt_buf.writeShort(this.type);
	return pkt_buf;
};

// 0x1ba
PACKET.CZ.REMOVE_AID = function PACKET_CZ_REMOVE_AID() {
	this.AccountName = '';
};
PACKET.CZ.REMOVE_AID.prototype.build = function () {
	const pkt_len = 2 + 24;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x1ba);
	pkt_buf.writeString(this.AccountName, 24);
	return pkt_buf;
};

// 0x198
PACKET.CZ.CHANGE_MAPTYPE = function PACKET_CZ_CHANGE_MAPTYPE() {
	this.xPos = 0;
	this.yPos = 0;
	this.type = 0;
};
PACKET.CZ.CHANGE_MAPTYPE.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x198);
	pkt_buf.writeShort(this.xPos);
	pkt_buf.writeShort(this.yPos);
	pkt_buf.writeShort(this.type);
	return pkt_buf;
};

// 0xb1a // New "Use Skill" packet. Fixes issues with Casting.
PACKET.ZC.USESKILL_ACK3 = function PACKET_ZC_USESKILL_ACK3(fp, end) {
	this.AID = fp.readULong(); // src id
	this.targetID = fp.readULong(); // dst id
	this.xPos = fp.readShort(); // x
	this.yPos = fp.readShort(); // y
	this.SKID = fp.readUShort(); // skill id
	this.property = fp.readULong(); // property (element)
	this.delayTime = fp.readULong(); // delaytime
	this.disposable = fp.readUChar(); // is disposable
	this.attackMT = fp.readULong(); // attackMT
};
PACKET.ZC.USESKILL_ACK3.size = 32;

// 0x0a52
PACKET.CZ.REQ_UPLOAD_MACRO_DETECTOR = function PACKET_CZ_REQ_UPLOAD_MACRO_DETECTOR() {
	this.answer = '';
	this.imageSize = 0;
};
PACKET.CZ.REQ_UPLOAD_MACRO_DETECTOR.prototype.build = function () {
	const pkt_len = 2 + 16 + 2;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x0a52);
	pkt_buf.writeString(this.answer, 16);
	pkt_buf.writeUShort(this.imageSize);
	return pkt_buf;
};
PACKET.CZ.REQ_UPLOAD_MACRO_DETECTOR.size = 20;

// 0x0a53
PACKET.ZC.ACK_UPLOAD_MACRO_DETECTOR = function PACKET_ZC_ACK_UPLOAD_MACRO_DETECTOR(fp, end) {
	this.captchaKey = fp.readBinaryString(4);
	this.captchaFlag = fp.readLong();
};
PACKET.ZC.ACK_UPLOAD_MACRO_DETECTOR.size = 10;

// 0x0a54
PACKET.CZ.UPLOAD_MACRO_DETECTOR_CAPTCHA = function PACKET_CZ_UPLOAD_MACRO_DETECTOR_CAPTCHA() {
	this.captchaKey = '';
	this.imageData = null;
};
PACKET.CZ.UPLOAD_MACRO_DETECTOR_CAPTCHA.prototype.build = function () {
	const imgLen = this.imageData ? this.imageData.byteLength : 0;
	const pkt_len = 2 + 2 + 4 + imgLen;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x0a54);
	pkt_buf.writeShort(pkt_len);
	pkt_buf.writeString(this.captchaKey, 4);
	if (imgLen) {
		pkt_buf.writeBuffer(this.imageData);
	}
	return pkt_buf;
};
PACKET.CZ.UPLOAD_MACRO_DETECTOR_CAPTCHA.size = -1;

// 0x0a55
PACKET.ZC.COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA = function PACKET_ZC_COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA(
	fp,
	end
) {};
PACKET.ZC.COMPLETE_UPLOAD_MACRO_DETECTOR_CAPTCHA.size = 2;

// 0x0a56
PACKET.CZ.REQ_APPLY_MACRO_DETECTOR = function PACKET_CZ_REQ_APPLY_MACRO_DETECTOR() {
	this.AID = 0;
};
PACKET.CZ.REQ_APPLY_MACRO_DETECTOR.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x0a56);
	pkt_buf.writeULong(this.AID);
	return pkt_buf;
};
PACKET.CZ.REQ_APPLY_MACRO_DETECTOR.size = 6;

// 0x0a57
PACKET.ZC.ACK_APPLY_MACRO_DETECTOR = function PACKET_ZC_ACK_APPLY_MACRO_DETECTOR(fp, end) {
	this.status = fp.readLong();
};
PACKET.ZC.ACK_APPLY_MACRO_DETECTOR.size = 6;

// 0x0a58
PACKET.ZC.APPLY_MACRO_DETECTOR = function PACKET_ZC_APPLY_MACRO_DETECTOR(fp, end) {
	this.imageSize = fp.readUShort();
	this.captchaKey = fp.readBinaryString(4);
};
PACKET.ZC.APPLY_MACRO_DETECTOR.size = 8;

// 0x0a59
PACKET.ZC.APPLY_MACRO_DETECTOR_CAPTCHA = function PACKET_ZC_APPLY_MACRO_DETECTOR_CAPTCHA(fp, end) {
	this.captchaKey = fp.readBinaryString(4);
	const count = end - fp.tell();
	this.imageData = new Uint8Array(count);
	for (let i = 0; i < count; ++i) {
		this.imageData[i] = fp.readUByte();
	}
};
PACKET.ZC.APPLY_MACRO_DETECTOR_CAPTCHA.size = -1;

// 0x0a5a
PACKET.CZ.COMPLETE_APPLY_MACRO_DETECTOR_CAPTCHA = function PACKET_CZ_COMPLETE_APPLY_MACRO_DETECTOR_CAPTCHA() {};
PACKET.CZ.COMPLETE_APPLY_MACRO_DETECTOR_CAPTCHA.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a5a);
	return pkt_buf;
};
PACKET.CZ.COMPLETE_APPLY_MACRO_DETECTOR_CAPTCHA.size = 2;

// 0x0a5b
PACKET.ZC.REQ_ANSWER_MACRO_DETECTOR = function PACKET_ZC_REQ_ANSWER_MACRO_DETECTOR(fp, end) {
	this.retryCount = fp.readUChar();
	this.timeout = fp.readLong();
};
PACKET.ZC.REQ_ANSWER_MACRO_DETECTOR.size = 7;

// 0x0a5c
PACKET.CZ.ACK_ANSWER_MACRO_DETECTOR = function PACKET_CZ_ACK_ANSWER_MACRO_DETECTOR() {
	this.answer = '';
};
PACKET.CZ.ACK_ANSWER_MACRO_DETECTOR.prototype.build = function () {
	const pkt_len = 2 + 16;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a5c);
	pkt_buf.writeString(this.answer, 16);
	return pkt_buf;
};
PACKET.CZ.ACK_ANSWER_MACRO_DETECTOR.size = 18;

// 0x0a5d
PACKET.ZC.CLOSE_MACRO_DETECTOR = function PACKET_ZC_CLOSE_MACRO_DETECTOR(fp, end) {
	this.status = fp.readLong();
};
PACKET.ZC.CLOSE_MACRO_DETECTOR.size = 6;

// 0x0a69
PACKET.CZ.REQ_PREVIEW_MACRO_DETECTOR = function PACKET_CZ_REQ_PREVIEW_MACRO_DETECTOR() {
	this.captchaID = 0;
};
PACKET.CZ.REQ_PREVIEW_MACRO_DETECTOR.prototype.build = function () {
	const pkt_len = 2 + 4;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a69);
	pkt_buf.writeLong(this.captchaID);
	return pkt_buf;
};
PACKET.CZ.REQ_PREVIEW_MACRO_DETECTOR.size = 6;

// 0x0a6a
PACKET.ZC.ACK_PREVIEW_MACRO_DETECTOR = function PACKET_ZC_ACK_PREVIEW_MACRO_DETECTOR(fp, end) {
	this.captchaFlag = fp.readLong();
	this.imageSize = fp.readUShort();
	this.captchaKey = fp.readBinaryString(4);
};
PACKET.ZC.ACK_PREVIEW_MACRO_DETECTOR.size = 12;

// 0x0a6b
PACKET.ZC.PREVIEW_MACRO_DETECTOR_CAPTCHA = function PACKET_ZC_PREVIEW_MACRO_DETECTOR_CAPTCHA(fp, end) {
	this.captchaKey = fp.readBinaryString(4);
	const count = end - fp.tell();
	this.imageData = new Uint8Array(count);
	for (let i = 0; i < count; ++i) {
		this.imageData[i] = fp.readUByte();
	}
};
PACKET.ZC.PREVIEW_MACRO_DETECTOR_CAPTCHA.size = -1;

// 0x0a6c
PACKET.CZ.REQ_PLAYER_AID_IN_RANGE = function PACKET_CZ_REQ_PLAYER_AID_IN_RANGE() {
	this.xPos = 0;
	this.yPos = 0;
	this.RadiusRange = 0;
};
PACKET.CZ.REQ_PLAYER_AID_IN_RANGE.prototype.build = function () {
	const pkt_len = 2 + 2 + 2 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a6c);
	pkt_buf.writeShort(this.xPos);
	pkt_buf.writeShort(this.yPos);
	pkt_buf.writeChar(this.RadiusRange);
	return pkt_buf;
};
PACKET.CZ.REQ_PLAYER_AID_IN_RANGE.size = 7;

// 0x0a6d
PACKET.ZC.ACK_PLAYER_AID_IN_RANGE = function PACKET_ZC_ACK_PLAYER_AID_IN_RANGE(fp, end) {
	this.AID = [];
	const count = (end - fp.tell()) / 4;
	for (let i = 0; i < count; ++i) {
		this.AID.push(fp.readULong());
	}
};
PACKET.ZC.ACK_PLAYER_AID_IN_RANGE.size = -1;

// Roulette Packets

// 0xA19 - CZ_REQ_OPEN_ROULETTE
PACKET.CZ.REQ_OPEN_ROULETTE = function PACKET_CZ_REQ_OPEN_ROULETTE() {};
PACKET.CZ.REQ_OPEN_ROULETTE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a19);
	return pkt_buf;
};

// 0xA1A - ZC_ACK_OPEN_ROULETTE
PACKET.ZC.ACK_OPEN_ROULETTE = function PACKET_ZC_ACK_OPEN_ROULETTE(fp, end) {
	this.result = fp.readChar(); // 1 byte - Result (0 = success, 1 = failed)
	this.serial = fp.readLong(); // 4 bytes - Serial
	this.step = fp.readChar(); // 1 byte - Step
	this.idx = fp.readChar(); // 1 byte - Idx
	this.additionItemID = fp.readUShort(); // 2 bytes - AdditionItemID
	this.goldPoint = fp.readLong(); // 4 bytes - Gold points
	this.silverPoint = fp.readLong(); // 4 bytes - Silver points
	this.bronzePoint = fp.readLong(); // 4 bytes - Bronze points
};
PACKET.ZC.ACK_OPEN_ROULETTE.size = 23;

// 0xA1B - CZ_REQ_ROULETTE_INFO (request roulette item list)
PACKET.CZ.REQ_ROULETTE_INFO = function PACKET_CZ_REQ_ROULETTE_INFO() {};
PACKET.CZ.REQ_ROULETTE_INFO.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a1b);
	return pkt_buf;
};

// 0xA1C - ZC_ACK_ROULETTE_INFO (roulette item list)
PACKET.ZC.ACK_ROULETTE_INFO = function PACKET_ZC_ACK_ROULETTE_INFO(fp, end) {
	fp.readUShort(); // 2 bytes - packet length
	this.serial = fp.readULong(); // 4 bytes - RouletteSerial
	this.items = [];
	// Read items until end of packet (42 items max)
	while (fp.tell() < end) {
		const item = {};
		item.row = fp.readUShort(); // 2 bytes
		item.position = fp.readUShort(); // 2 bytes
		item.itemId = fp.readUShort(); // 2 bytes
		item.count = fp.readUShort(); // 2 bytes
		this.items.push(item);
	}
};
PACKET.ZC.ACK_ROULETTE_INFO.size = -1;

// 0xA1D - CZ_REQ_CLOSE_ROULETTE
PACKET.CZ.REQ_CLOSE_ROULETTE = function PACKET_CZ_REQ_CLOSE_ROULETTE() {};
PACKET.CZ.REQ_CLOSE_ROULETTE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a1d);
	return pkt_buf;
};

// 0xA1E - ZC_ACK_CLOSE_ROULETTE
PACKET.ZC.ACK_CLOSE_ROULETTE = function PACKET_ZC_ACK_CLOSE_ROULETTE(fp, end) {
	this.result = fp.readUChar();
};
PACKET.ZC.ACK_CLOSE_ROULETTE.size = 3;

// 0xA1F - CZ_REQ_GENERATE_ROULETTE (spin request)
PACKET.CZ.REQ_GENERATE_ROULETTE = function PACKET_CZ_REQ_GENERATE_ROULETTE() {};
PACKET.CZ.REQ_GENERATE_ROULETTE.prototype.build = function () {
	const pkt_len = 2;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a1f);
	return pkt_buf;
};

// 0xA20 - ZC_ACK_GENERATE_ROULETTE (spin result)
PACKET.ZC.ACK_GENERATE_ROULETTE = function PACKET_ZC_ACK_GENERATE_ROULETTE(fp, end) {
	this.result = fp.readUChar(); // 1 byte - Result
	this.step = fp.readUShort(); // 2 bytes - Step
	this.idx = fp.readUShort(); // 2 bytes - Idx
	this.additionItemID = fp.readUShort(); // 2 bytes - AdditionItemID
	this.remainGold = fp.readLong(); // 4 bytes - RemainGold
	this.remainSilver = fp.readLong(); // 4 bytes - RemainSilver
	this.remainBronze = fp.readLong(); // 4 bytes - RemainBronze
};
PACKET.ZC.ACK_GENERATE_ROULETTE.size = 21;

// 0xA21 - CZ_RECV_ROULETTE_ITEM (request to receive roulette item)
PACKET.CZ.RECV_ROULETTE_ITEM = function PACKET_CZ_RECV_ROULETTE_ITEM() {
	this.condition = 0; // 0 = normal, 1 = losing
};
PACKET.CZ.RECV_ROULETTE_ITEM.prototype.build = function () {
	const pkt_len = 3;
	const pkt_buf = new BinaryWriter(pkt_len);
	pkt_buf.writeShort(0x0a21);
	pkt_buf.writeUChar(this.condition);
	return pkt_buf;
};

// 0xA22 - ZC_RECV_ROULETTE_ITEM (receive roulette item result)
PACKET.ZC.RECV_ROULETTE_ITEM = function PACKET_ZC_RECV_ROULETTE_ITEM(fp, end) {
	this.result = fp.readUChar(); // 1 byte - Result (0=success, 1=failed, 2=overcount, 3=overweight)
	this.additionItemID = fp.readUShort(); // 2 bytes - AdditionItemID
};
PACKET.ZC.RECV_ROULETTE_ITEM.size = 5;

// 0x097f - ZC_SELECTCART
PACKET.ZC.SELECTCART = function PACKET_ZC_SELECTCART(fp, end) {
	this.identity = fp.readLong();
	this.typeList = [];
	while (fp.tell() < end) {
		this.typeList.push(fp.readUChar());
	}
};
PACKET.ZC.SELECTCART.size = -1;

// 0x0980 - CZ_SELECTCART
PACKET.CZ.SELECTCART = function PACKET_CZ_SELECTCART() {
	this.identity = 0;
	this.type = 0;
};
PACKET.CZ.SELECTCART.prototype.build = function () {
	const pkt_len = 2 + 4 + 1;
	const pkt_buf = new BinaryWriter(pkt_len);

	pkt_buf.writeShort(0x0980);
	pkt_buf.writeLong(this.identity);
	pkt_buf.writeUChar(this.type);
	return pkt_buf;
};

/**
 * Export
 */
export default PACKET;
