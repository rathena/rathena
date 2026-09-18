/**
 * Engine/Replay/ReplayPlayer.js
 *
 * Ragnarok Online Replay System — Orchestrator / State Machine
 *
 * Pipeline:
 *   IDLE → LOADING_REPLAY → APPLYING_SESSION → LOADING_MAP
 *        → PLAYING_INITIAL_DATA → INITIAL_DATA_COMPLETE
 *        → PLAYING_PACKET_STREAM → REPLAY_FINISHED
 */

import DB from 'DB/DBManager.js';
import Network from 'Network/NetworkManager.js';
import MapRenderer from 'Renderer/MapRenderer.js';
import Session from 'Engine/SessionStorage.js';
import MapEngine from 'Engine/MapEngine.js';
import Player from 'Renderer/Entity/Player.js';
import BasicInfo from 'UI/Components/BasicInfo/BasicInfo.js';
import WinStats from 'UI/Components/WinStats/WinStats.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import CartItems from 'UI/Components/CartItems/CartItems.js';
import { ReplayState, replayLog } from 'Engine/Replay/ReplayTypes.js';
import ReplaySocket from './ReplaySocket.js';
import ReplayParser from './ReplayParser.js';

export default class ReplayPlayer {
	constructor() {
		this.parser = null;
		this.socket = null;

		// Playback controls
		this.playing = false;
		this.speed = 1.0;
		this.startTime = 0;
		this.lastTickTime = 0;
		this.durationMs = 0;

		// State machine
		this._state = ReplayState.IDLE;

		// Buffers populated by load()
		this._sessionData = null;
		this._itemsBuffer = null;
		this._petBuffer = null;
		this._statusBuffer = null;
		this._efstListBuffer = [];
		this._initialBuffer = []; // Array of { type, typeName, chunks }
		this._packetStreamBuffer = []; // Array of { id, time, length, data, packetId }

		// Playback cursors
		this._initialGroupIndex = 0;
		this._initialChunkIndex = 0;
		this._streamChunkIndex = 0;
		this._firstStreamTime = 0;
		this._logicalTime = 0;

		// Bound tick handler
		this._onTick = this.tick.bind(this);
		this._animate = false;

		// DB retry counter
		this._retryCount = 0;
	}

	async load(file) {
		this._setState(ReplayState.LOADING_REPLAY);
		replayLog('[Replay] Loading RRF...');

		const buffer = await file.arrayBuffer();
		this.parser = new ReplayParser(buffer);
		const result = this.parser.parse();

		this._sessionData = result.sessionBuffer;
		this._itemsBuffer = result.itemsBuffer;
		this._petBuffer = result.petBuffer;
		this._statusBuffer = result.statusBuffer;
		this._efstListBuffer = result.efstListBuffer || [];
		this._initialBuffer = result.initialBuffer;
		this._packetStreamBuffer = result.packetStreamBuffer;
		this.durationMs = result.durationMs || 0;

		this._setState(ReplayState.IDLE);

		// Nothing to play back: report it here so the caller can tell the user,
		// instead of loading the map and reporting a completed playback
		if (this._packetStreamBuffer.length === 0) {
			throw new Error('The replay contains no recorded packets, the file is truncated or corrupt.');
		}
	}

	start() {
		if (!this.parser || !this._sessionData) {
			console.error('[ReplayPlayer] No replay loaded');
			throw new Error('No replay loaded');
		}

		// Wait for DB to be ready
		if (!DB.isLoaded) {
			if (!DB.startedLazyInit) {
				DB.lazyInit();
				DB.startedLazyInit = true;
			}
			this._retryCount = (this._retryCount || 0) + 1;
			if (this._retryCount > 600) {
				console.error('[Replay] Failed loading databases.');
				this._retryCount = 0;
				DB.startedLazyInit = false;
				return;
			}
			setTimeout(() => this.start(), 100);
			return;
		}
		this._retryCount = 0;
		DB.startedLazyInit = false;

		// Reset cursors
		this._initialGroupIndex = 0;
		this._initialChunkIndex = 0;
		this._streamChunkIndex = 0;
		this.playing = true;
		this.startTime = 0;
		this.lastTickTime = Date.now();

		// Install mock socket
		Network.setSocketFactory((host, port) => {
			this.socket = new ReplaySocket(host, port);
			return this.socket;
		});

		Network.close();

		// Apply session data BEFORE loading map
		this._applySession();
	}

	pause() {
		this.playing = false;
	}

	resume() {
		this.playing = true;
		this._syncTimeOrigin();
	}

	stop() {
		this.playing = false;
		this._animate = false;
		Network.setSocketFactory(null);

		if (this.socket) {
			// Network.close() detaches the socket first, so no disconnect dialog pops up
			Network.close();
			this.socket = null;
		}

		if (this._state !== ReplayState.REPLAY_FINISHED) {
			this._setState(ReplayState.IDLE);
		}
	}

	setSpeed(speed) {
		this.speed = speed;
		this._syncTimeOrigin();
	}

	/**
	 * Recompute the wall clock origin so playback continues from the logical time
	 * reached so far. Inverse of the formula used in _tickPacketStream().
	 */
	_syncTimeOrigin() {
		const firstStreamTime = this._firstStreamTime || 0;
		const logicalTime = Math.max(this._logicalTime || 0, firstStreamTime);
		this.startTime = Date.now() - (logicalTime - firstStreamTime) / this.speed;
	}

	_setState(newState) {
		this._state = newState;
	}

	/**
	 * Applies session data directly to Session and Session.Entity.
	 * Called BEFORE MapEngine.init so map name and character are available.
	 */
	_applySession() {
		this._setState(ReplayState.APPLYING_SESSION);

		const s = this._sessionData;
		const charName = s.characterName || 'Replay';
		const sex = s.sex === 0 || s.sex === 1 ? s.sex : 0;
		const aid = s.AID || 0;
		const gid = s.GID || aid;

		// Reset Session.Entity completely using a new Player instance so no stale data
		// from previous sessions leaks in, and all Entity methods and prototype properties exist.
		const playerInitData = {
			name: charName,
			sex,
			job: s.job !== undefined ? s.job : 0,
			clevel: s.level !== undefined ? s.level : s.clevel || 1,
			joblevel: s.joblevel !== undefined ? s.joblevel : 1,
			exp: s.exp || 0,
			exp_next: s.exp_next || 0,
			job_exp: s.job_exp || 0,
			job_exp_next: s.job_exp_next || 0,
			str: s.str || 1,
			agi: s.agi || 1,
			vit: s.vit || 1,
			int: s.int || 1,
			dex: s.dex || 1,
			luk: s.luk || 1,
			str_bonus: s.str_bonus || s.plusStr || 0,
			agi_bonus: s.agi_bonus || s.plusAgi || 0,
			vit_bonus: s.vit_bonus || s.plusVit || 0,
			int_bonus: s.int_bonus || s.plusInt || 0,
			dex_bonus: s.dex_bonus || s.plusDex || 0,
			luk_bonus: s.luk_bonus || s.plusLuk || 0,
			money: s.money || 0,
			weight: s.weight || 0,
			max_weight: s.max_weight || 0,
			speed: s.speed || 150,
			attack_speed: s.attack_speed || s.aspd || 300,
			head: s.head || 0,
			weapon: s.weapon || 0,
			shield: s.shield || 0,
			bodypalette: s.bodypalette || 0,
			headpalette: s.headpalette || 0,
			accessory: s.accessory || 0,
			accessory2: s.accessory2 || 0,
			accessory3: s.accessory3 || 0,
			robe: s.robe || 0,
			AID: aid,
			GID: gid
		};

		// Session globals
		Session.AID = aid;
		Session.GID = gid;
		Session.Sex = sex;
		Session.zeny = s.money || 0;
		Session.hasCart = s.hasCart || false;
		Session.CartNum = s.CartNum || 0;

		// Pet data in Session
		if (s.pet && s.pet.aid) {
			Session.petId = s.pet.aid;
			Session.pet = s.pet;
		} else {
			Session.petId = 0;
			Session.pet = {};
		}

		// Instantiate player entity
		Session.Entity = new Player(playerInitData);

		// Synchronize display properties
		Session.Entity.display.name = charName;
		Session.Entity.sex = sex;
		Session.Entity._sex = sex;
		Session.Entity.job = playerInitData.job;
		Session.Entity._job = playerInitData.job;
		Session.Entity.clevel = playerInitData.clevel;
		Session.Entity.level = playerInitData.clevel;
		Session.Entity.joblevel = playerInitData.joblevel;
		Session.Entity.money = playerInitData.money;
		Session.Entity.weight = playerInitData.weight;
		Session.Entity.max_weight = playerInitData.max_weight;
		Session.Entity.speed = playerInitData.speed;
		Session.Entity.attack_speed = playerInitData.attack_speed;

		// Stats
		Session.Entity.str = playerInitData.str;
		Session.Entity.agi = playerInitData.agi;
		Session.Entity.vit = playerInitData.vit;
		Session.Entity.int = playerInitData.int;
		Session.Entity.dex = playerInitData.dex;
		Session.Entity.luk = playerInitData.luk;
		Session.Entity.str_bonus = playerInitData.str_bonus;
		Session.Entity.agi_bonus = playerInitData.agi_bonus;
		Session.Entity.vit_bonus = playerInitData.vit_bonus;
		Session.Entity.int_bonus = playerInitData.int_bonus;
		Session.Entity.dex_bonus = playerInitData.dex_bonus;
		Session.Entity.luk_bonus = playerInitData.luk_bonus;

		// Appearance
		Session.Entity.head = playerInitData.head;
		Session.Entity._head = playerInitData.head;
		Session.Entity.headpalette = playerInitData.headpalette;
		Session.Entity._headpalette = playerInitData.headpalette;
		Session.Entity.bodypalette = playerInitData.bodypalette;
		Session.Entity._bodypalette = playerInitData.bodypalette;
		Session.Entity.weapon = playerInitData.weapon;
		Session.Entity._weapon = playerInitData.weapon;
		Session.Entity.shield = playerInitData.shield;
		Session.Entity._shield = playerInitData.shield;
		Session.Entity.accessory = playerInitData.accessory;
		Session.Entity._accessory = playerInitData.accessory;
		Session.Entity.accessory2 = playerInitData.accessory2;
		Session.Entity._accessory2 = playerInitData.accessory2;
		Session.Entity.accessory3 = playerInitData.accessory3;
		Session.Entity._accessory3 = playerInitData.accessory3;
		Session.Entity.robe = playerInitData.robe;

		// Life (HP / SP)
		const hp = s.hp !== undefined ? s.hp : s.maxHp !== undefined ? s.maxHp : 100;
		const maxHp = s.maxHp !== undefined ? s.maxHp : hp;
		const sp = s.sp !== undefined ? s.sp : s.maxSp !== undefined ? s.maxSp : 100;
		const maxSp = s.maxSp !== undefined ? s.maxSp : sp;

		Session.Entity.life.hp = hp;
		Session.Entity.life.hp_max = maxHp;
		Session.Entity.life.sp = sp;
		Session.Entity.life.sp_max = maxSp;

		// EffectState & Option
		const option = s.effectState !== undefined ? s.effectState : s.option || 0;
		Session.Entity.effectState = option;
		Session.Entity._effectState = option;
		Session.Entity.option = option;

		// Cart
		Session.Entity.hasCart = Session.hasCart;
		Session.Entity.CartNum = Session.CartNum;

		// Initial start coordinates & direction
		this._mapName = s.mapName || 'prontera.rsw';
		this._startX = s.startX || 0;
		this._startY = s.startY || 0;
		this._startDir = s.startDir !== undefined ? s.startDir : 4;

		Session.Entity.position[0] = this._startX;
		Session.Entity.position[1] = this._startY;
		Session.Entity.position[2] = 0;
		Session.Entity.direction = this._startDir;

		replayLog(
			`[Replay] Session applied — name: ${charName}, sex: ${Session.Entity.sex}, head: ${Session.Entity.head}, job: ${Session.Entity.job}, map: ${this._mapName}, HP: ${hp}/${maxHp}, SP: ${sp}/${maxSp}`
		);

		this._loadMap();
	}

	_loadMap() {
		this._setState(ReplayState.LOADING_MAP);

		// ACCEPT_ENTER must be sent after socket connects so MapEngine can call
		// MapRenderer.setMap() and start the loading sequence.
		this._acceptEnterSent = false;
		// Track if map loading has actually started (setMap called)
		this._mapLoadStarted = false;

		MapEngine.init('127.0.0.1', 6900, this._mapName);

		// Start animation loop
		this._animate = true;
		this.lastTickTime = Date.now();
		this._onTick();
	}

	/**
	 * Synthesizes the ZC_ACCEPT_ENTER (0x0073) packet that MapEngine needs
	 * to register the player entity and trigger the full map initialisation sequence.
	 */
	_sendAcceptEnter() {
		const pkt = new Uint8Array(11);
		const view = new DataView(pkt.buffer);
		view.setUint16(0, 0x0073, true);
		view.setUint32(2, Date.now(), true);

		const x = this._startX;
		const y = this._startY;
		const dir = this._startDir;
		const p = (x << 14) | (y << 4) | (dir & 0x0f);
		view.setUint8(6, (p >> 16) & 0xff);
		view.setUint8(7, (p >> 8) & 0xff);
		view.setUint8(8, p & 0xff);

		this._pushPacket(pkt);
	}

	/**
	 * Synthesizes a ZC_MSG_STATE_CHANGE (0x0196) packet for an active EFST buff.
	 */
	_sendStateChange(statusId, isOn = true) {
		const pkt = new Uint8Array(9);
		const view = new DataView(pkt.buffer);
		view.setUint16(0, 0x0196, true);
		view.setUint16(2, statusId, true);
		view.setUint32(4, Session.AID, true);
		view.setUint8(8, isOn ? 1 : 0);
		this._pushPacket(pkt);
	}

	tick() {
		if (this._animate) {
			requestAnimationFrame(this._onTick);
		}

		if (!this.playing || !this.socket || !this.socket.connected) {
			this.lastTickTime = Date.now();
			return;
		}

		const now = Date.now();

		switch (this._state) {
			case ReplayState.LOADING_MAP:
				// Step 1: Send ACCEPT_ENTER as soon as socket connects
				if (!this._acceptEnterSent) {
					// Force a full map load, otherwise MapRenderer.setMap() takes the
					// local teleport path and never toggles `loading`
					MapRenderer.currentMap = '';
					this._sendAcceptEnter();
					this._acceptEnterSent = true;
					this._mapLoadStarted = false;
				}
				// Step 2: Once MapRenderer.loading becomes true, map started loading
				if (this._acceptEnterSent && MapRenderer.loading) {
					this._mapLoadStarted = true;
				}
				// Step 3: Once MapRenderer.loading becomes false AFTER it was true,
				// map has fully loaded — transition to initial data
				if (this._mapLoadStarted && !MapRenderer.loading) {
					this._mapLoadStarted = false;
					this._setState(ReplayState.PLAYING_INITIAL_DATA);
				}
				// Freeze stream time while map is loading
				this.startTime += now - this.lastTickTime;
				this.lastTickTime = now;
				break;

			case ReplayState.PLAYING_INITIAL_DATA:
				this.lastTickTime = now;
				this._tickInitialData();
				break;

			case ReplayState.INITIAL_DATA_COMPLETE:
				this._setState(ReplayState.PLAYING_PACKET_STREAM);
				this.startTime = Date.now();
				this.lastTickTime = now;
				this._firstStreamTime = this._packetStreamBuffer.length > 0 ? this._packetStreamBuffer[0].time : 0;
				this._logicalTime = this._firstStreamTime;
				break;

			case ReplayState.PLAYING_PACKET_STREAM:
				this.lastTickTime = now;
				this._tickPacketStream(now);
				break;

			default:
				this.lastTickTime = now;
				break;
		}
	}

	/**
	 * Drains initial data (InitialPackets → InitialEntities → InitialFloorItems)
	 * and applies initial Efst buffs, items, and UI stat updates.
	 */
	_tickInitialData() {
		for (const group of this._initialBuffer) {
			for (const chunk of group.chunks) {
				const data = chunk.data || chunk;
				this._pushPacket(data);
			}
		}

		// Inject initial buffs (EfstList from Container 18)
		if (this._efstListBuffer && this._efstListBuffer.length > 0) {
			for (const efstId of this._efstListBuffer) {
				this._sendStateChange(efstId, true);
			}
		}

		// If initial items exist from Container 8, add them using Inventory.getUI().addItem
		if (this._itemsBuffer) {
			const inventoryUI = Inventory?.getUI ? Inventory.getUI() : null;
			if (inventoryUI && typeof inventoryUI.addItem === 'function') {
				// Clear any previous inventory list entries
				if (inventoryUI.list) {
					inventoryUI.list.length = 0;
				}
				if (inventoryUI.equippedItems) {
					inventoryUI.equippedItems.length = 0;
				}

				// 1. Add inventory bag items (WearState = 0)
				if (this._itemsBuffer.inventory?.length > 0) {
					for (const item of this._itemsBuffer.inventory) {
						inventoryUI.addItem(item);
					}
				}

				// 2. Add equipped items (WearState > 0) — addItemSub automatically equips them in Equipment window
				if (this._itemsBuffer.equipped?.length > 0) {
					for (const item of this._itemsBuffer.equipped) {
						inventoryUI.addItem(item);
					}
				}

				// 3. Add costume / shadow gear (WearState > 0)
				if (this._itemsBuffer.equippedCostume?.length > 0) {
					for (const item of this._itemsBuffer.equippedCostume) {
						inventoryUI.addItem(item);
					}
				}
			}

			// Add cart items
			const cartUI = CartItems?.getUI ? CartItems.getUI() : null;
			if (cartUI && this._itemsBuffer.cart?.length > 0) {
				if (typeof cartUI.addItem === 'function') {
					if (cartUI.list) {
						cartUI.list.length = 0;
					}
					for (const item of this._itemsBuffer.cart) {
						cartUI.addItem(item);
					}
				} else if (typeof cartUI.setItems === 'function') {
					cartUI.setItems(this._itemsBuffer.cart);
				}
			}
		}

		// Update UI components with character session values
		if (Session.Entity) {
			if (BasicInfo?.getUI()?.update) {
				BasicInfo.getUI().update('blvl', Session.Entity.clevel);
				BasicInfo.getUI().update('jlvl', Session.Entity.joblevel);
				BasicInfo.getUI().update('zeny', Session.Entity.money);
				BasicInfo.getUI().update('name', Session.Entity.display.name);
				BasicInfo.getUI().update('job', Session.Entity.job);
				BasicInfo.getUI().update('hp', Session.Entity.life.hp, Session.Entity.life.hp_max);
				BasicInfo.getUI().update('sp', Session.Entity.life.sp, Session.Entity.life.sp_max);
				BasicInfo.getUI().update('weight', Session.Entity.weight, Session.Entity.max_weight);
			}

			if (WinStats?.getUI()?.update) {
				WinStats.getUI().update('str', Session.Entity.str);
				WinStats.getUI().update('agi', Session.Entity.agi);
				WinStats.getUI().update('vit', Session.Entity.vit);
				WinStats.getUI().update('int', Session.Entity.int);
				WinStats.getUI().update('dex', Session.Entity.dex);
				WinStats.getUI().update('luk', Session.Entity.luk);
				WinStats.getUI().update('str2', Session.Entity.str_bonus);
				WinStats.getUI().update('agi2', Session.Entity.agi_bonus);
				WinStats.getUI().update('vit2', Session.Entity.vit_bonus);
				WinStats.getUI().update('int2', Session.Entity.int_bonus);
				WinStats.getUI().update('dex2', Session.Entity.dex_bonus);
				WinStats.getUI().update('luk2', Session.Entity.luk_bonus);
			}
		}

		this._setState(ReplayState.INITIAL_DATA_COMPLETE);
	}

	/**
	 * Time-based packet injection from the PacketStream buffer.
	 */
	_tickPacketStream(now) {
		const logicalTime = (now - this.startTime) * this.speed + (this._firstStreamTime || 0);
		this._logicalTime = logicalTime;

		while (this._streamChunkIndex < this._packetStreamBuffer.length) {
			const chunk = this._packetStreamBuffer[this._streamChunkIndex];
			if (chunk.time <= logicalTime) {
				this._pushPacket(chunk.data);
				this._streamChunkIndex++;
			} else {
				break;
			}
		}

		if (this._streamChunkIndex >= this._packetStreamBuffer.length) {
			replayLog('[Replay] Replay finished');
			this._setState(ReplayState.REPLAY_FINISHED);
			this.stop();
		}
	}

	_getCurrentStreamChunk() {
		return this._packetStreamBuffer[this._streamChunkIndex] || null;
	}

	/**
	 * Injects a raw network packet buffer into the NetworkManager via the mock socket.
	 */
	_pushPacket(data) {
		if (!this.socket || !this.socket.connected) {
			return;
		}
		try {
			this.socket.push(data);
		} catch (e) {
			console.error('[Replay] Packet injection error:', e);
		}
	}
}
