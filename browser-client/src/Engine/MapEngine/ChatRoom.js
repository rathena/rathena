/**
 * Engine/MapEngine/ChatRoom.js
 *
 * Chat Room Handler
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import EntityManager from 'Renderer/EntityManager.js';
import ChatRoomCreate from 'UI/Components/ChatRoomCreate/ChatRoomCreate.js';
import ChatRoom from 'UI/Components/ChatRoom/ChatRoom.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Session from 'Engine/SessionStorage.js';

/**
 * Request a chat room
 * PACKET.CZ.CREATE_CHATROOM
 */
ChatRoomCreate.requestRoom = function requestRoom() {
	if (ChatRoomCreate.editMode) {
		ChatRoom.changeChatRoom(this.title, this.limit, this.type, this.password);
		ChatRoomCreate.editMode = false;
	} else {
		const pkt = new PACKET.CZ.CREATE_CHATROOM();
		pkt.size = this.limit;
		pkt.type = this.type;
		pkt.passwd = this.password;
		pkt.title = this.title;
		Network.sendPacket(pkt);
	}
};

/**
 * Request a change in the chat room
 * PACKET.CZ.CHANGE_CHATROOM (0xde)
 * @param {string} title
 * @param {number} limit
 * @param {number} type - 0=private, 1=public
 * @param {string} password
 */
ChatRoom.changeChatRoom = function changeChatRoom(title, limit, type, password) {
	const pkt = new PACKET.CZ.CHANGE_CHATROOM();
	pkt.size = limit;
	pkt.type = type;
	pkt.passwd = password || '';
	pkt.title = title;
	Network.sendPacket(pkt);
};

/**
 * Request to change the role of a member (transfer leadership)
 * PACKET.CZ.REQ_ROLE_CHANGE (0xe0)
 * @param {number} role - 0=owner, 1=normal
 * @param {string} name
 */
ChatRoom.requestRoleChange = function requestRoleChange(role, name) {
	const pkt = new PACKET.CZ.REQ_ROLE_CHANGE();
	pkt.role = role;
	pkt.name = name;
	Network.sendPacket(pkt);
};

/**
 * Request to expel a member from current chatroom
 * PACKET.CZ.REQ_EXPEL_MEMBER (0xe2)
 * @param {string} name
 */
ChatRoom.requestExpelMember = function requestExpelMember(name) {
	const pkt = new PACKET.CZ.REQ_EXPEL_MEMBER();
	pkt.name = name;
	Network.sendPacket(pkt);
};

/**
 * Request exit from current chatroom
 * PACKET.CZ.EXIT_ROOM
 */
ChatRoom.exitRoom = function exitRoom() {
	const pkt = new PACKET.CZ.EXIT_ROOM();
	Network.sendPacket(pkt);
};

/**
 * Response from the server if the chat creating was succesful or not.
 * @param {object} pkt - PACKET.ZC.ACK_CREATE_CHATROOM
 */
function onCreateRoomResult(pkt) {
	switch (pkt.result) {
		// Success
		case 0:
			ChatRoom.title = ChatRoomCreate.title;
			ChatRoom.limit = ChatRoomCreate.limit;
			ChatRoom.type = ChatRoomCreate.type;
			ChatRoom.count = 1;
			ChatRoom.members = [Session.Entity.display.name];
			ChatRoom.owner = Session.Entity.display.name;
			ChatRoom.append();

			ChatBox.addText(DB.getMessage(64), ChatBox.TYPE.BLUE, ChatBox.FILTER.PUBLIC_LOG);
			break;

		// Room limit exceeded
		case 1:
			ChatBox.addText(DB.getMessage(65), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;

		// Same room already exists
		case 2:
			ChatBox.addText(DB.getMessage(66), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
			break;
	}
}

/**
 *
 * @param {object} pkt - PACKET.ZC.REFUSE_ENTER_ROOM
 *  0 = room full
 *  1 = wrong password
 *  2 = kicked
 *  3 = success (no message)
 *  4 = no enough zeny
 *  5 = too low level
 *  6 = too high level
 *  7 = unsuitable job class
 */
function onEnterRoomResult(pkt) {
	let error = 67;
	switch (pkt.result) {
		// full
		case 0:
			error = 67;
			break;
		case 1:
			error = 7;
			break;
		case 2:
			error = 68;
			break;
		case 3:
			return; //don't show anything!
		case 4:
			error = 55;
			break;
		case 5:
			error = 432;
			break;
		case 6:
			error = 433;
			break;
		case 7:
			error = 434;
			break;
	}

	ChatBox.addText(DB.getMessage(error), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
}

/**
 * Notify a entry of a new member
 * @param {object} pkt - PACKET_ZC_MEMBER_NEWENTRY
 */
function onMemberJoin(pkt) {
	ChatRoom.count = pkt.curcount;
	ChatRoom.members.push(pkt.name);

	ChatRoom.updateChat();
	ChatRoom.message(DB.getMessage(179).replace('%s', pkt.name), 'join');
}

/**
 * Change room owner
 * @param {object} pkt - PACKET.ZC.ROLE_CHANGE
 */
function onRoleChange(pkt) {
	// The server will send two of this packets!
	// One to remove the ownership and one to add ownership, we dont need the first packet !

	if (pkt.role === 0) {
		ChatRoom.owner = pkt.name;
		ChatRoom.updateChat();
	}
}

/**
 * Member exit
 * @param {object} pkt - PACKET.ZC.MEMBER_EXIT
 */
function onMemberLeave(pkt) {
	// Seems like the server send us we are disconnect,
	// we do not care.
	if (!ChatRoom.isOpen) {
		return;
	}

	ChatRoom.count = pkt.curcount;
	ChatRoom.removeMember(pkt.name);
	ChatRoom.updateChat();

	// Leave the room
	if (pkt.type === 0) {
		ChatRoom.message(DB.getMessage(180).replace('%s', pkt.name), 'leave');
	}

	// Kick out of the room
	else {
		ChatRoom.message(DB.getMessage(181).replace('%s', pkt.name), 'leave');
	}
}

/**
 * Change chat room properties
 * @param {object} pkt - PACKET.ZC.CHANGE_CHATROOM
 */
function onRoomUpdate(pkt) {
	// TODO: switch chat owner (AID-roomID).
	ChatRoom.limit = pkt.maxcount;
	ChatRoom.count = pkt.curcount;
	ChatRoom.type = pkt.type;
	ChatRoom.title = pkt.title;
	ChatRoom.updateChat();
}

/**
 * Enter a room
 * @param {object} pkt - PACKET.ZC.ENTER_ROOM
 */
function onRoomEnter(pkt) {
	//this.roomID       = fp.readULong();
	const count = pkt.memberList.length;
	ChatRoom.members = new Array(count);

	for (let i = 0; i < count; ++i) {
		if (pkt.memberList[i].role === 0) {
			ChatRoom.owner = pkt.memberList[i].name;
		}
		ChatRoom.members[i] = pkt.memberList[i].name;
	}

	// Remove room
	EntityManager.forEach(entity => {
		if (entity.room.id === pkt.roomID) {
			entity.room.remove();
			return false;
		}
		return true;
	});

	ChatRoom.count = count;
	ChatRoom.append();
}

/**
 * Initialize
 */
export default function MainEngine() {
	Network.hookPacket(PACKET.ZC.ACK_CREATE_CHATROOM, onCreateRoomResult);
	//Network.hookPacket(PACKET.ZC.ROOM_NEWENTRY,     Display); //This is holded up at Entity.js
	Network.hookPacket(PACKET.ZC.CHANGE_CHATROOM, onRoomUpdate);
	//Network.hookPacket(PACKET.ZC.DESTROY_ROOM,      Destroy); //This is holded up at Entity.js
	Network.hookPacket(PACKET.ZC.ENTER_ROOM, onRoomEnter);
	Network.hookPacket(PACKET.ZC.MEMBER_NEWENTRY, onMemberJoin);
	Network.hookPacket(PACKET.ZC.ROLE_CHANGE, onRoleChange);
	Network.hookPacket(PACKET.ZC.MEMBER_EXIT, onMemberLeave);
	Network.hookPacket(PACKET.ZC.REFUSE_ENTER_ROOM, onEnterRoomResult);
}
