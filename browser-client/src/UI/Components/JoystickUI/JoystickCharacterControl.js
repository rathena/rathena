/**
 * UI/Components/JoystickUI/JoystickCharacterControl.js
 *
 * Handles character-specific actions triggered by joystick input,
 * including movement calculation, auto-attacking targeted entities,
 * and item pickup logic.
 *
 * @author AoShinHo
 */

import Session from 'Engine/SessionStorage.js';
import EntityManager from 'Renderer/EntityManager.js';
import Network from 'Network/NetworkManager.js';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import glMatrix from 'Vendors/gl-matrix.js';
import Camera from 'Renderer/Camera.js';
import PathFinding from 'Utils/PathFinding.js';
import Target from './JoystickTargetService.js';

const direction = glMatrix.vec2.create();
const rotate = glMatrix.mat2.create();

function move(x, y) {
	const player = Session.Entity;
	if (!player) {
		return;
	}

	direction[0] = x;
	direction[1] = y;

	glMatrix.mat2.identity(rotate);
	glMatrix.mat2.rotate(rotate, rotate, ((-Camera.direction * 45) / 180) * Math.PI);
	glMatrix.vec2.transformMat2(direction, direction, rotate);

	const nx = Math.round(player.position[0] + direction[0] * 3);
	const ny = Math.round(player.position[1] + direction[1] * 3);

	const movePacket = PACKETVER.value >= 20180307 ? new PACKET.CZ.REQUEST_MOVE2() : new PACKET.CZ.REQUEST_MOVE();

	movePacket.dest[0] = nx;
	movePacket.dest[1] = ny;

	Network.sendPacket(movePacket);
}

function attack() {
	const Player = Session.Entity;
	if (!Player) {
		return;
	}

	const target = Target.getEntity();
	if (!target) {
		return;
	}

	Target.focus(target);

	const entityFocus = EntityManager.getFocusEntity();
	if (!entityFocus) {
		return;
	}

	let pkt;
	const out = [];
	const count = PathFinding.search(
		Player.position[0] | 0,
		Player.position[1] | 0,
		entityFocus.position[0] | 0,
		entityFocus.position[1] | 0,
		Player.attack_range + 1,
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

	if (count < Player.attack_range + 1) {
		Network.sendPacket(pkt);
		return true;
	}

	Session.moveAction = pkt;

	if (PACKETVER.value >= 20180307) {
		pkt = new PACKET.CZ.REQUEST_MOVE2();
	} else {
		pkt = new PACKET.CZ.REQUEST_MOVE();
	}
	pkt.dest[0] = out[(count - 1) * 2];
	pkt.dest[1] = out[(count - 1) * 2 + 1];
	Network.sendPacket(pkt);
}

function pickUp() {
	const Player = Session.Entity;
	if (!Player) {
		return;
	}

	const item = EntityManager.getClosestEntity(Player, EntityManager.TYPE_ITEM);
	if (!item) {
		return;
	}

	const pkt = PACKETVER.value >= 20180307 ? new PACKET.CZ.ITEM_PICKUP2() : new PACKET.CZ.ITEM_PICKUP();

	pkt.ITAID = item.GID;
	Network.sendPacket(pkt);
}
export default {
	attack: attack,
	pickUp: pickUp,
	move: move
};
