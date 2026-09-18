/**
 * Controls/EntityControl.js
 *
 * Entity class
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import glMatrix from 'Utils/gl-matrix.js';
import PathFinding from 'Utils/PathFinding.js';
import DB from 'DB/DBManager.js';
import KEYS from 'Controls/KeyEventHandler.js';
import Mouse from 'Controls/MouseEventHandler.js';
import Preferences from 'Preferences/Controls.js';
import Camera from 'Renderer/Camera.js';
import Session from 'Engine/SessionStorage.js';
import Friends from 'Engine/MapEngine/Friends.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import Network from 'Network/NetworkManager.js';
import Cursor from 'UI/CursorManager.js';
import InputBox from 'UI/Components/InputBox/InputBox.js';
import ChatRoom from 'UI/Components/ChatRoom/ChatRoom.js';
import ContextMenu from 'UI/Components/ContextMenu/ContextMenu.js';
import Pet from 'UI/Components/PetInformations/PetInformations.js';
import Trade from 'UI/Components/Trade/Trade.js';
import NpcBox from 'UI/Components/NpcBox/NpcBox.js';
import Altitude from 'Renderer/Map/Altitude.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import CaptchaSelector from 'UI/Components/Captcha/CaptchaSelector.js';
import Guild from 'Engine/MapEngine/Guild.js';
import PartyFriends from 'UI/Components/PartyFriends/PartyFriends.js';
import Group from 'Engine/MapEngine/Group.js';
import HomunInformations from 'UI/Components/HomunInformations/HomunInformations.js';
import MercenaryInformations from 'UI/Components/MercenaryInformations/MercenaryInformations.js';

/**
 * Import
 */
const mat4 = glMatrix.mat4;
const vec2 = glMatrix.vec2;
const _matrix = mat4.create();

class EntityControl {
	/*
	 * When mouse is over
	 */
	static onMouseOver() {
		const Entity = this.constructor;

		switch (this.objecttype) {
			case Entity.TYPE_PET:
				if (!Camera.action.active) {
					Cursor.setType(Cursor.ACTION.DEFAULT);
				}
				break;

			case Entity.TYPE_PC:
			case Entity.TYPE_ELEM:
			case Entity.TYPE_HOM:
			case Entity.TYPE_MERC: {
				if ((KEYS.SHIFT === true || Preferences.noshift === true) && this !== Session.Entity) {
					if (!Camera.action.active) {
						Cursor.setType(Cursor.ACTION.ATTACK);
					}
					break;
				}

				const action = this.canAttackEntity() ? Cursor.ACTION.ATTACK : Cursor.ACTION.DEFAULT;
				Cursor.setType(action);
				break;
			}

			case Entity.TYPE_MOB:
			case Entity.TYPE_UNIT:
			case Entity.TYPE_NPC_ABR:
			case Entity.TYPE_NPC_BIONIC:
				Cursor.setType(Cursor.ACTION.ATTACK);
				break;

			case Entity.TYPE_NPC:
			case Entity.TYPE_NPC2:
				//check if already talk to NPC
				if (!NpcBox.ui || !NpcBox.ui.is(':visible')) {
					Cursor.setType(Cursor.ACTION.TALK, true);
				}
				break;

			case Entity.TYPE_WARP:
				Cursor.setType(Cursor.ACTION.WARP);
				return;

			case Entity.TYPE_ITEM:
				Cursor.setType(Cursor.ACTION.PICK, true, 0);
				break;
		}

		switch (this.display.load) {
			// Ask for the name
			case this.display.TYPE.NONE: {
				this.display.load = this.display.TYPE.LOADING;

				let pkt;
				if (PACKETVER.value >= 20180307) {
					pkt = new PACKET.CZ.REQNAME2();
				} else {
					pkt = new PACKET.CZ.REQNAME();
				}
				pkt.AID = this.GID;
				Network.sendPacket(pkt);
				break;
			}
			// Nothing yet
			case this.display.TYPE.LOADING:
				break;

			// Display the name
			case this.display.TYPE.COMPLETE:
				mat4.multiply(_matrix, Camera.projection, this.matrix);
				this.display.render(_matrix);
				this.display.add();
				break;
		}
	}

	/**
	 * When mouse is not over yet
	 */
	static onMouseOut() {
		if (!Camera.action.active) {
			Cursor.setType(Cursor.ACTION.DEFAULT);
		} else {
			Cursor.setType(Cursor.ACTION.ROTATE);
		}

		if (this !== this.constructor.Manager.getFocusEntity()) {
			this.display.display = false;
			this.display.remove();
		}
	}

	/**
	 * When clicking on an Entity
	 *
	 */
	static onMouseDown() {
		const Entity = this.constructor;
		let pkt;

		switch (this.objecttype) {
			case Entity.TYPE_PET:
			case Entity.TYPE_HOM:
			case Entity.TYPE_MERC:
				break;

			case Entity.TYPE_ITEM:
				Cursor.setType(Cursor.ACTION.PICK, true, 2);
				Session.Entity.lookTo(this.position[0], this.position[1]);
				if (PACKETVER.value >= 20180307) {
					pkt = new PACKET.CZ.ITEM_PICKUP2();
				} else {
					pkt = new PACKET.CZ.ITEM_PICKUP();
				}
				pkt.ITAID = this.GID;

				// Too far, walking to it
				if (vec2.distance(Session.Entity.position, this.position) > 2) {
					Session.moveAction = pkt;

					if (PACKETVER.value >= 20180307) {
						pkt = new PACKET.CZ.REQUEST_MOVE2();
					} else {
						pkt = new PACKET.CZ.REQUEST_MOVE();
					}
					pkt.dest[0] = Mouse.world.x;
					pkt.dest[1] = Mouse.world.y;
					Network.sendPacket(pkt);

					return true;
				}
				Network.sendPacket(pkt);

				return true;

			case Entity.TYPE_NPC:
			case Entity.TYPE_NPC2:
				//check if already talk to NPC
				if (!NpcBox.ui || !NpcBox.ui.is(':visible')) {
					pkt = new PACKET.CZ.CONTACTNPC();
					pkt.NAID = this.GID;
					pkt.type = 1; // 1 for NPC in Aegis
					Network.sendPacket(pkt);

					// Update look
					Session.Entity.lookTo(this.position[0], this.position[1]);
					if (PACKETVER.value >= 20180307) {
						pkt = new PACKET.CZ.CHANGE_DIRECTION2();
					} else {
						pkt = new PACKET.CZ.CHANGE_DIRECTION();
					}
					pkt.headDir = Session.Entity.headDir;
					pkt.dir = Session.Entity.direction;
					Network.sendPacket(pkt);

					//Update Cursor
					Cursor.setType(Cursor.ACTION.DEFAULT);
				}
				return true;

			case Entity.TYPE_WARP: {
				const i = 1;
				const out = [];
				let j, x, y;
				PathFinding.search(
					Session.Entity.position[0] | 0,
					Session.Entity.position[1] | 0,
					this.position[0] | 0,
					this.position[1] | 0,
					i,
					out
				);
				for (j = out.length; j > 1; j -= 2) {
					x = out[j - 2];
					y = out[j - 1];
					if (Altitude.getCellType(x, y) & Altitude.TYPE.WALKABLE) {
						break;
					}
				}

				if (PACKETVER.value >= 20180307) {
					pkt = new PACKET.CZ.REQUEST_MOVE2();
				} else {
					pkt = new PACKET.CZ.REQUEST_MOVE();
				}
				pkt.dest[0] = x;
				pkt.dest[1] = y;
				Network.sendPacket(pkt);
				return true;
			}
			case Entity.TYPE_PC: {
				if (Session.captchaGetIdOnEntityClick) {
					CaptchaSelector.addPlayer(this.GID);
				}
				return true;
			}
		}

		return false;
	}

	/**
	 * Stop clicking on an entity
	 */
	static onMouseUp() {}

	/**
	 * Focus the entity
	 */
	static onFocus() {
		const Entity = this.constructor;
		const main = Session.Entity;
		let pkt;

		switch (this.objecttype) {
			case Entity.TYPE_PC:
			case Entity.TYPE_ELEM:
			case Entity.TYPE_HOM:
				// TODO: add check for PVP/WOE mapflag
				if (KEYS.SHIFT === false && Preferences.noshift === false && !this.canAttackEntity()) {
					if (!Camera.action.active) {
						Cursor.setType(Cursor.ACTION.DEFAULT);
					}
					if (!Session.TouchTargeting && !Session.autoFollow) {
						break;
					}
				}
			// no break intended.

			case Entity.TYPE_MOB:
			case Entity.TYPE_UNIT:
			case Entity.TYPE_NPC_ABR:
			case Entity.TYPE_NPC_BIONIC:
				// Start rendering the lock on arrow
				this.attachments.add({
					uid: 'lockon',
					spr: 'data/sprite/cursors.spr',
					act: 'data/sprite/cursors.act',
					frame: Cursor.ACTION.LOCK,
					repeat: true,
					depth: 10.0
				});

				if (!Session.TouchTargeting && !Session.autoFollow) {
					const out = [];
					const count = PathFinding.search(
						main.position[0] | 0,
						main.position[1] | 0,
						this.position[0] | 0,
						this.position[1] | 0,
						main.attack_range + 1,
						out
					);

					// Can't attack
					if (!count) {
						return true;
					}

					if (main.isOverWeight) {
						ChatBox.addText(DB.getMessage(243), ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
						return true;
					}

					// send look to target
					main.lookTo(this.position[0], this.position[1]);

					if (PACKETVER.value >= 20180307) {
						pkt = new PACKET.CZ.CHANGE_DIRECTION2();
					} else {
						pkt = new PACKET.CZ.CHANGE_DIRECTION();
					}
					pkt.headDir = main.headDir;
					pkt.dir = main.direction;
					Network.sendPacket(pkt);

					if (PACKETVER.value >= 20180307) {
						pkt = new PACKET.CZ.REQUEST_ACT2();
					} else {
						pkt = new PACKET.CZ.REQUEST_ACT();
					}
					pkt.action = 7;
					pkt.targetGID = this.GID;

					// in range send packet
					if (count < 2) {
						Network.sendPacket(pkt);
						return true;
					}

					// Move to entity
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

				return true;
		}

		return false;
	}

	/**
	 * Lost focus on entity
	 */
	static onFocusEnd() {
		const Entity = this.constructor;

		switch (this.objecttype) {
			case Entity.TYPE_PC:
			case Entity.TYPE_ELEM:
			case Entity.TYPE_HOM:
			case Entity.TYPE_MOB:
			case Entity.TYPE_UNIT:
			case Entity.TYPE_NPC_ABR:
			case Entity.TYPE_NPC_BIONIC:
				if (Entity.Manager.getFocusEntity()) {
					Network.sendPacket(new PACKET.CZ.CANCEL_LOCKON());
				}
		}

		// Stop displaying name
		this.display.display = false;
		this.display.remove();

		// Stop rendering t he lock on arrow
		this.attachments.remove('lockon');
	}

	/**
	 * Open entity room (chat room, shop, ...)
	 */
	static onRoomEnter() {
		let pkt;
		const Room = this.room.constructor;

		switch (this.room.type) {
			case Room.Type.SELL_SHOP:
				pkt = new PACKET.CZ.REQ_CLICK_TO_BUYING_STORE();
				pkt.makerAID = this.room.id;
				Network.sendPacket(pkt);
				break;

			case Room.Type.BUY_SHOP:
				pkt = new PACKET.CZ.REQ_BUY_FROMMC();
				pkt.AID = this.room.id;
				Network.sendPacket(pkt);
				break;

			case Room.Type.PUBLIC_CHAT:
				pkt = new PACKET.CZ.REQ_ENTER_ROOM();
				pkt.roomID = this.room.id;
				pkt.passwd = '';
				Network.sendPacket(pkt);

				/* Prepare the chat room UI */
				ChatRoom.type = 1; //public
				ChatRoom.title = this.room.title;
				ChatRoom.limit = this.room.limit;
				ChatRoom.count = this.room.count;
				break;

			case Room.Type.PRIVATE_CHAT: {
				pkt = new PACKET.CZ.REQ_ENTER_ROOM();
				pkt.roomID = this.room.id;

				InputBox.append();
				InputBox.setType('pass', false);
				const self = this;
				InputBox.onSubmitRequest = pass => {
					InputBox.remove();
					pkt.passwd = pass;
					Network.sendPacket(pkt);

					/* Prepare the chat room UI */
					ChatRoom.type = 0; //private
					ChatRoom.title = self.room.title;
					ChatRoom.limit = self.room.limit;
					ChatRoom.count = self.room.count;
				};
				return;
			}
		}
	}

	/**
	 * When clicking on an Entity
	 *
	 */
	static onContextMenu() {
		const Entity = this.constructor;
		const entity = this;

		switch (this.objecttype) {
			case Entity.TYPE_PET:
				if (Session.petId === this.GID) {
					ContextMenu.remove();
					ContextMenu.append();
					ContextMenu.addElement(DB.getMessage(596), Pet.ui.show.bind(Pet.ui)); // check pet status
					ContextMenu.addElement(DB.getMessage(592), Pet.reqPetFeed); // Feed pet
					ContextMenu.addElement(DB.getMessage(593), Pet.reqPetAction); // performance
					ContextMenu.addElement(DB.getMessage(595), Pet.reqUnEquipPet); // unequip accessory
					ContextMenu.addElement(DB.getMessage(594), Pet.reqBackToEgg); // return to egg shell
				}
				break;

			case Entity.TYPE_PC:
				/// TODO: complete it :
				/// - check for admin action (kick, mute, ...)

				ContextMenu.remove();
				ContextMenu.append();

				// Check equipment
				ContextMenu.addElement(DB.getMessage(1360).replace('%s', this.display.name), () => {
					Equipment.onCheckPlayerEquipment(entity.GID); // simple version (MapEngine/Item.js)
				});

				// Trade option
				ContextMenu.addElement(DB.getMessage(87).replace('%s', this.display.name), () => {
					Trade.reqExchange(entity.GID, entity.display.name);
				});

				// Guild features
				if (Session.hasGuild) {
					if (Session.guildRight & 0x01 && !this.GUID) {
						// Send (%s) a Guild invitation
						ContextMenu.addElement(DB.getMessage(382).replace('%s', this.display.name), () => {
							Guild.requestPlayerInvitation(entity.GID);
						});
					}

					if (Session.isGuildMaster && this.GUID && Session.Entity.GUID !== this.GUID) {
						ContextMenu.nextGroup();

						// Set this guild as an Alliance
						ContextMenu.addElement(DB.getMessage(399).replace('%s', this.display.name), () => {
							Guild.requestAlliance(entity.GID);
						});

						// Set this guild as an Antagonist
						ContextMenu.addElement(DB.getMessage(403).replace('%s', this.display.name), () => {
							Guild.requestHostility(entity.GID);
						});
					}
				}

				// Open 1:1Chat
				ContextMenu.addElement(DB.getMessage(360), () => {
					PartyFriends.onOpenChat1to1(entity.display.name);
				});

				if (!Friends.isFriend(this.display.name)) {
					ContextMenu.nextGroup();
					ContextMenu.addElement(DB.getMessage(358), () => {
						Friends.addFriend(entity.display.name);
					});
				}

				if (Session.hasParty && Session.isPartyLeader) {
					ContextMenu.nextGroup();
					ContextMenu.addElement(DB.getMessage(88).replace('%s', this.display.name), () => {
						Group.onRequestInvitation(entity.GID, entity.display.name);
					});
				}

				//ContextMenu.nextGroup();
				//ContextMenu.addElement( DB.getMessage(315), blockUserPrivateMessage);
				break;

			case Entity.TYPE_HOM:
				if (Session.homunId === this.GID) {
					ContextMenu.remove();
					ContextMenu.append();
					ContextMenu.addElement('View Status', () => {
						HomunInformations.ui.toggle();
					});
					ContextMenu.addElement('Feed', () => {
						HomunInformations.reqHomunFeed();
					});
					if (localStorage.getItem('HOM_AGGRESSIVE') == 0) {
						ContextMenu.addElement('Assist', () => {
							HomunInformations.toggleAggressive();
						});
					} else {
						ContextMenu.addElement('Stand By', () => {
							HomunInformations.toggleAggressive();
						});
					}
				}
				break;

			case Entity.TYPE_MERC:
				if (Session.mercId === this.GID) {
					ContextMenu.remove();
					ContextMenu.append();
					ContextMenu.addElement('View Status', () => {
						MercenaryInformations.ui.toggle();
					});
					if (localStorage.getItem('MER_AGGRESSIVE') == 0) {
						ContextMenu.addElement('Assist', () => {
							MercenaryInformations.toggleAggressive();
						});
					} else {
						ContextMenu.addElement('Stand By', () => {
							MercenaryInformations.toggleAggressive();
						});
					}
				}
				break;
		}

		return false;
	}

	static canAttackEntity() {
		if (this === Session.Entity) {
			return false;
		}
		// Show attack cursor on non-party members (PvP)
		else if (Session.mapState.isPVP) {
			if (Session.hasParty && PartyFriends.isGroupMember(this.display.name)) {
				return false;
			}
			return true;
		}
		// Show attack cursor on non-guild members (GvG)
		else if (Session.mapState.isGVG) {
			if (
				(Session.Entity.GUID > 0 && this.GUID !== Session.Entity.GUID) ||
				(this.GUID == 0 && this !== Session.Entity)
			) {
				// 0 = no guild, can be attacked by anyone
				return true;
			}
		}
		return false;
	}
}

/**
 * Export
 */
export default function Init() {
	this.onMouseOver = EntityControl.onMouseOver;
	this.onMouseOut = EntityControl.onMouseOut;
	this.onMouseDown = EntityControl.onMouseDown;
	this.onMouseUp = EntityControl.onMouseUp;
	this.onFocus = EntityControl.onFocus;
	this.onFocusEnd = EntityControl.onFocusEnd;
	this.onRoomEnter = EntityControl.onRoomEnter;
	this.onContextMenu = EntityControl.onContextMenu;
	this.canAttackEntity = EntityControl.canAttackEntity;
}
