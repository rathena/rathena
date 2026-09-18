/**
 * Engine/Replay/ReplayParser.js
 *
 * Ragnarok Online Replay Format (.rrf) Parser
 * Specification derived from adsonpleal/rrfparser reference implementation.
 */

import BinaryReader from 'Utils/BinaryReader.js';
import ItemType from 'DB/Items/ItemType.js';
import EquipmentLocation from 'DB/Items/EquipmentLocation.js';
import {
	ContainerType,
	ContainerTypeNames,
	ItemChunkKind,
	ItemRecordTag,
	ReplayOpCodes,
	ReplayOpCodeName,
	replayLog
} from 'Engine/Replay/ReplayTypes.js';

// 100 byte prefix string + 12 bytes of version/signature/timestamp
const HEADER_BYTES = 112;
const DESCRIPTOR_BYTES = 10;
const DESCRIPTOR_COUNT = 24;

export default class ReplayParser {
	constructor(fileBuffer) {
		this.fp = new BinaryReader(fileBuffer);
		this.header = {};
		this.containers = [];
		this.size = fileBuffer.byteLength;
	}

	parse() {
		this.readHeader();
		replayLog('[ReplayParser] Header parsed:', this.header);

		if (this.header.version === 5) {
			this.readContainersV5();
			// this.dumpContainers();
		} else {
			console.warn(`[ReplayParser] Unsupported version: ${this.header.version}`);
		}

		return this.buildBuffers();
	}

	readHeader() {
		if (this.size < HEADER_BYTES) {
			throw new Error(`Replay file is truncated: ${this.size} bytes, expected at least ${HEADER_BYTES}`);
		}

		this.fp.seek(0, 0);

		// 100-byte header prefix string (e.g. "<< Ragnarok Replay File Version...")
		this.header.rawPrefix = this.fp.readBinaryString(100);

		this.header.version = this.fp.readUByte();
		this.header.sig = this.fp.readBinaryString(3);

		this.header.recordedAt = {
			year: this.fp.readShort(),
			month: this.fp.readUByte(),
			day: this.fp.readUByte()
		};
		this.header.dateUnused = this.fp.readUByte();

		this.header.recordedAt.hour = this.fp.readUByte();
		this.header.recordedAt.minute = this.fp.readUByte();
		this.header.recordedAt.second = this.fp.readUByte();

		this.header.date = this.header.recordedAt;
		this.keys = this.deriveKeys(this.header.recordedAt);
	}

	deriveKeys(d) {
		const buf = new ArrayBuffer(4);
		const view = new DataView(buf);

		view.setInt16(0, d.year, true);
		view.setUint8(2, d.month);
		view.setUint8(3, d.day);
		const k1 = view.getInt32(0, true) >> 5;

		view.setUint8(0, 0);
		view.setUint8(1, d.hour);
		view.setUint8(2, d.minute);
		view.setUint8(3, d.second);
		const k2 = view.getInt32(0, true) >> 3;

		return { k1, k2 };
	}

	decryptChunk(data, size) {
		const out = new Uint8Array(data.length);
		out.set(data);

		const view = new DataView(out.buffer, out.byteOffset, out.byteLength);
		const wordCount = Math.floor(size / 4);

		for (let cursor = 0; cursor < wordCount; cursor++) {
			const old = view.getInt32(cursor * 4, true);
			const xorVal = Math.imul(this.keys.k1 + cursor + 1, this.keys.k2);
			view.setInt32(cursor * 4, old ^ xorVal, true);
		}

		return out;
	}

	readContainersV5() {
		const tableOffset = HEADER_BYTES;
		const tableEnd = tableOffset + DESCRIPTOR_COUNT * DESCRIPTOR_BYTES;

		if (this.size < tableEnd) {
			throw new Error(`Replay descriptor table is truncated: ${this.size} bytes, expected at least ${tableEnd}`);
		}

		for (let i = 0; i < DESCRIPTOR_COUNT; i++) {
			this.fp.seek(tableOffset + i * DESCRIPTOR_BYTES, 0);
			const type = this.fp.readUShort();
			const declaredLength = this.fp.readLong();
			const offset = this.fp.readLong();

			let realLength = declaredLength;
			if (realLength === 0 && offset > 0) {
				realLength = this.size - offset;
			}

			const container = {
				type,
				declaredLength,
				offset,
				realLength,
				data: []
			};

			this.containers.push(container);

			if (offset === 0 && declaredLength === 0) {
				continue;
			}

			if (offset < 0 || offset >= this.size || realLength <= 0 || offset + realLength > this.size) {
				console.warn(
					`[ReplayParser] Skipping out of bounds container ${ContainerTypeNames[type] || type} at ${offset} (${realLength} bytes)`
				);
				continue;
			}

			this.fp.seek(offset, 0);
			const body = new Uint8Array(this.fp.buffer, offset, realLength);

			if (type === ContainerType.PacketStream) {
				this.parsePacketStream(container, body);
			} else {
				this.parseGenericContainer(container, body, declaredLength);
			}
		}
	}

	parsePacketStream(container, body) {
		const view = new DataView(body.buffer, body.byteOffset, body.byteLength);
		let ptr = 0;
		while (ptr + 10 <= body.byteLength) {
			const id = view.getInt32(ptr, true);
			const time = view.getInt32(ptr + 4, true);
			const length = view.getUint16(ptr + 8, true);
			const dataStart = ptr + 10;
			const dataEnd = dataStart + length;
			if (dataEnd > body.byteLength) {
				break;
			}
			const encrypted = body.subarray(dataStart, dataEnd);
			const decrypted = this.decryptChunk(encrypted, length);
			const packetId = decrypted.length >= 2 ? decrypted[0] | (decrypted[1] << 8) : 0;

			container.data.push({ id, time, length, data: decrypted, packetId });
			ptr = dataEnd;
		}
	}

	parseGenericContainer(container, body, declaredLength) {
		if (declaredLength <= 0) {
			return;
		}

		const decrypted = this.decryptChunk(body, declaredLength);
		const view = new DataView(decrypted.buffer, decrypted.byteOffset, decrypted.byteLength);

		let ptr = 0;
		while (ptr + 6 <= declaredLength) {
			const id = view.getInt16(ptr, true);
			const length = view.getInt32(ptr + 2, true);
			const dataStart = ptr + 6;
			const dataEnd = dataStart + length;
			if (length < 0 || dataEnd > decrypted.byteLength) {
				break;
			}

			const data = decrypted.subarray(dataStart, dataEnd);
			container.data.push({ id, length, data });

			ptr = dataEnd;
		}
	}

	buildBuffers() {
		const sessionBuffer = this.buildSessionBuffer();
		const itemsBuffer = this.buildItemsBuffer();
		const petBuffer = this.buildPetBuffer();
		const statusBuffer = this.buildStatusBuffer();
		const efstListBuffer = this.buildEfstListBuffer();

		// Merge status & pet into sessionBuffer
		Object.assign(sessionBuffer, statusBuffer);
		if (petBuffer) {
			sessionBuffer.pet = petBuffer;
		}

		const initialBuffer = [];
		const initialTypes = [
			ContainerType.InitialPackets,
			ContainerType.InitialEntities,
			ContainerType.InitialFloorItems
		];

		for (const container of this.containers) {
			if (!initialTypes.includes(container.type)) {
				continue;
			}
			if (container.data.length === 0) {
				continue;
			}
			const typeName = ContainerTypeNames[container.type] || `Unknown(${container.type})`;
			// Filter out empty bracket markers (len 0) and keep valid packet data
			const chunks = container.data
				.filter(chunk => chunk.data && chunk.data.byteLength > 0)
				.map(chunk => chunk.data);

			if (chunks.length > 0) {
				initialBuffer.push({ type: container.type, typeName, chunks });
				replayLog(`[Replay] Loaded ${typeName} (${container.type}): ${chunks.length} packets`);
			}
		}

		const packetStreamContainer = this.containers.find(c => c.type === ContainerType.PacketStream);
		const packetStreamBuffer = packetStreamContainer ? packetStreamContainer.data : [];
		replayLog(`[Replay] Packet stream loaded: ${packetStreamBuffer.length} chunks`);

		// Determine true duration in ms (larger of stored length in chunk 970 and last packet timestamp)
		let lastPacketTime = 0;
		if (packetStreamBuffer.length > 0) {
			lastPacketTime = packetStreamBuffer[packetStreamBuffer.length - 1].time || 0;
		}
		const durationMs = Math.max(sessionBuffer.durationMs || 0, lastPacketTime);
		sessionBuffer.durationMs = durationMs;

		return {
			header: this.header,
			sessionBuffer,
			itemsBuffer,
			petBuffer,
			statusBuffer,
			efstListBuffer,
			initialBuffer,
			packetStreamBuffer,
			durationMs
		};
	}

	buildSessionBuffer() {
		const buf = {};

		const readU8 = chunk =>
			new DataView(chunk.data.buffer, chunk.data.byteOffset, chunk.data.byteLength).getUint8(0);
		const readU16 = chunk =>
			new DataView(chunk.data.buffer, chunk.data.byteOffset, chunk.data.byteLength).getUint16(0, true);
		const readU32 = chunk =>
			new DataView(chunk.data.buffer, chunk.data.byteOffset, chunk.data.byteLength).getUint32(0, true);

		const readStringById = (container, id) => {
			const ch = container.data.find(c => c.id === id);
			if (!ch || ch.data.byteLength === 0) {
				return null;
			}
			return this.readString(ch.data);
		};

		const readU32ById = (container, id) => {
			const ch = container.data.find(c => c.id === id);
			if (!ch || ch.data.byteLength < 4) {
				return null;
			}
			return readU32(ch);
		};

		const readU16ById = (container, id) => {
			const ch = container.data.find(c => c.id === id);
			if (!ch || ch.data.byteLength < 2) {
				return null;
			}
			return readU16(ch);
		};

		const readU8ById = (container, id) => {
			const ch = container.data.find(c => c.id === id);
			if (!ch || ch.data.byteLength < 1) {
				return null;
			}
			return readU8(ch);
		};

		// Container 2: ReplayData
		const replayData = this.containers.find(c => c.type === ContainerType.ReplayData);
		if (replayData) {
			const sexVal = readU32ById(replayData, 963) ?? readU8ById(replayData, 963);
			if (sexVal !== null) {
				buf.sex = sexVal === 0 || sexVal === 1 ? sexVal : 0;
			}

			// Name & Map may be id-tagged (964/965) or positional chunks [4] / [5]
			let name = readStringById(replayData, 964);
			if (!name && replayData.data.length > 4 && replayData.data[4].data?.byteLength > 0) {
				name = this.readString(replayData.data[4].data);
			}
			if (name) {
				buf.characterName = name;
			}

			let mapName = readStringById(replayData, 965);
			if (!mapName && replayData.data.length > 5 && replayData.data[5].data?.byteLength > 0) {
				mapName = this.readString(replayData.data[5].data);
			}
			if (mapName) {
				if (!mapName.endsWith('.rsw')) {
					mapName += '.rsw';
				}
				buf.mapName = mapName;
			}

			const gx = readU32ById(replayData, 967);
			if (gx !== null) {
				buf.startX = gx;
			}

			const gy = readU32ById(replayData, 968);
			if (gy !== null) {
				buf.startY = gy;
			}

			const dir = readU32ById(replayData, 969) ?? readU8ById(replayData, 969);
			if (dir !== null) {
				buf.startDir = dir;
			}

			const lengthMs = readU32ById(replayData, 970);
			if (lengthMs !== null) {
				buf.durationMs = lengthMs;
			}
		}

		// Container 3: Session
		const sessionContainer = this.containers.find(c => c.type === ContainerType.Session);
		if (sessionContainer) {
			if (!buf.characterName) {
				const charName = readStringById(sessionContainer, 964);
				if (charName) {
					buf.characterName = charName;
				}
			}

			const aid = readU32ById(sessionContainer, 1010);
			if (aid !== null) {
				buf.AID = aid;
				buf.GID = aid;
			}

			const gid = readU32ById(sessionContainer, 1011);
			if (gid !== null) {
				buf.GID = gid;
			}

			const job = readU32ById(sessionContainer, 1014);
			if (job !== null) {
				buf.job = job;
			}

			const exp = readU32ById(sessionContainer, 1015);
			if (exp !== null) {
				buf.exp = exp;
			}

			const level = readU32ById(sessionContainer, 1016);
			if (level !== null) {
				buf.level = level;
				buf.clevel = level;
			}

			const jobPoint = readU32ById(sessionContainer, 1017);
			if (jobPoint !== null) {
				buf.jobPoint = jobPoint;
			}

			const nextExp = readU32ById(sessionContainer, 1018);
			if (nextExp !== null) {
				buf.exp_next = nextExp;
			}

			const jobLevel = readU32ById(sessionContainer, 1019);
			if (jobLevel !== null) {
				buf.joblevel = jobLevel;
			}

			const skillPoint = readU32ById(sessionContainer, 1020);
			if (skillPoint !== null) {
				buf.skillPoint = skillPoint;
			}

			// Base Stats (allocated)
			const str = readU32ById(sessionContainer, 1024);
			if (str !== null) {
				buf.str = str;
			}

			const agi = readU32ById(sessionContainer, 1025);
			if (agi !== null) {
				buf.agi = agi;
			}

			const vit = readU32ById(sessionContainer, 1026);
			if (vit !== null) {
				buf.vit = vit;
			}

			const intVal = readU32ById(sessionContainer, 1027);
			if (intVal !== null) {
				buf.int = intVal;
			}

			const dex = readU32ById(sessionContainer, 1028);
			if (dex !== null) {
				buf.dex = dex;
			}

			const luk = readU32ById(sessionContainer, 1029);
			if (luk !== null) {
				buf.luk = luk;
			}

			// Plus / Bonus Stats
			const plusStr = readU32ById(sessionContainer, 1030);
			if (plusStr !== null) {
				buf.plusStr = plusStr;
				buf.str_bonus = plusStr;
			}

			const plusAgi = readU32ById(sessionContainer, 1031);
			if (plusAgi !== null) {
				buf.plusAgi = plusAgi;
				buf.agi_bonus = plusAgi;
			}

			const plusVit = readU32ById(sessionContainer, 1032);
			if (plusVit !== null) {
				buf.plusVit = plusVit;
				buf.vit_bonus = plusVit;
			}

			const plusInt = readU32ById(sessionContainer, 1033);
			if (plusInt !== null) {
				buf.plusInt = plusInt;
				buf.int_bonus = plusInt;
			}

			const plusDex = readU32ById(sessionContainer, 1034);
			if (plusDex !== null) {
				buf.plusDex = plusDex;
				buf.dex_bonus = plusDex;
			}

			const plusLuk = readU32ById(sessionContainer, 1035);
			if (plusLuk !== null) {
				buf.plusLuk = plusLuk;
				buf.luk_bonus = plusLuk;
			}

			// ASPD / ATK / DEF / Hit / Flee / Critical
			const aspd = readU32ById(sessionContainer, 1036);
			if (aspd !== null) {
				buf.aspd = aspd;
				buf.attack_speed = aspd;
			}

			const atk = readU32ById(sessionContainer, 1037);
			if (atk !== null) {
				buf.atk = atk;
			}

			const mdef = readU32ById(sessionContainer, 1038);
			if (mdef !== null) {
				buf.mdef = mdef;
			}

			const plusAspd = readU32ById(sessionContainer, 1039);
			if (plusAspd !== null) {
				buf.plusAspd = plusAspd;
			}

			const def = readU32ById(sessionContainer, 1040);
			if (def !== null) {
				buf.def = def;
			}

			const plusDef = readU32ById(sessionContainer, 1041);
			if (plusDef !== null) {
				buf.plusDef = plusDef;
			}

			const refiningPower = readU32ById(sessionContainer, 1042);
			if (refiningPower !== null) {
				buf.refiningPower = refiningPower;
			}

			const maxMatk = readU32ById(sessionContainer, 1043);
			if (maxMatk !== null) {
				buf.maxMatk = maxMatk;
			}

			const minMatk = readU32ById(sessionContainer, 1044);
			if (minMatk !== null) {
				buf.minMatk = minMatk;
			}

			const plusMdef = readU32ById(sessionContainer, 1045);
			if (plusMdef !== null) {
				buf.plusMdef = plusMdef;
			}

			const hit = readU32ById(sessionContainer, 1046);
			if (hit !== null) {
				buf.hit = hit;
			}

			const flee = readU32ById(sessionContainer, 1047);
			if (flee !== null) {
				buf.flee = flee;
			}

			const crit = readU32ById(sessionContainer, 1048);
			if (crit !== null) {
				buf.crit = crit;
			}

			const plusFlee = readU32ById(sessionContainer, 1049);
			if (plusFlee !== null) {
				buf.plusFlee = plusFlee;
			}

			const equipArrowIndex = readU16ById(sessionContainer, 1050);
			if (equipArrowIndex !== null) {
				buf.equipArrowIndex = equipArrowIndex;
			}

			const money = readU32ById(sessionContainer, 1051);
			if (money !== null) {
				buf.money = money;
			}

			const speed = readU32ById(sessionContainer, 1052);
			if (speed !== null) {
				buf.speed = speed;
			}

			const honor = readU32ById(sessionContainer, 1053);
			if (honor !== null) {
				buf.honor = honor;
			}

			const nextJobExp = readU32ById(sessionContainer, 1054);
			if (nextJobExp !== null) {
				buf.job_exp_next = nextJobExp;
			}

			const jobExp = readU32ById(sessionContainer, 1055);
			if (jobExp !== null) {
				buf.job_exp = jobExp;
			}

			const virtue = readU32ById(sessionContainer, 1056);
			if (virtue !== null) {
				buf.virtue = virtue;
			}

			// Appearance & Visuals
			const hairStyle = readU32ById(sessionContainer, 1060);
			if (hairStyle !== null) {
				buf.head = hairStyle;
			}

			const weapon = readU32ById(sessionContainer, 1061);
			if (weapon !== null) {
				buf.weapon = weapon;
			}

			const shield = readU32ById(sessionContainer, 1062);
			if (shield !== null) {
				buf.shield = shield;
			}

			const clothesColor = readU32ById(sessionContainer, 1063);
			if (clothesColor !== null) {
				buf.bodypalette = clothesColor;
			}

			const hairColor = readU32ById(sessionContainer, 1064);
			if (hairColor !== null) {
				buf.headpalette = hairColor;
			}

			const accessory = readU32ById(sessionContainer, 1065);
			if (accessory !== null) {
				buf.accessory = accessory;
			}

			const accessory2 = readU32ById(sessionContainer, 1066);
			if (accessory2 !== null) {
				buf.accessory2 = accessory2;
			}

			const accessory3 = readU32ById(sessionContainer, 1067);
			if (accessory3 !== null) {
				buf.accessory3 = accessory3;
			}

			const option = readU32ById(sessionContainer, 1070);
			if (option !== null) {
				buf.effectState = option;
				buf.option = option;
			}

			const robe = readU32ById(sessionContainer, 1071);
			if (robe !== null) {
				buf.robe = robe;
			}

			// Cart info
			const cartCurCount = readU16ById(sessionContainer, 1086);
			if (cartCurCount !== null) {
				buf.cartCurCount = cartCurCount;
			}

			const cartMaxCount = readU16ById(sessionContainer, 1087);
			if (cartMaxCount !== null) {
				buf.cartMaxCount = cartMaxCount;
			}

			const cartCurWeight = readU32ById(sessionContainer, 1088);
			if (cartCurWeight !== null) {
				buf.cartCurWeight = cartCurWeight;
			}

			const cartMaxWeight = readU32ById(sessionContainer, 1089);
			if (cartMaxWeight !== null) {
				buf.cartMaxWeight = cartMaxWeight;
			}

			// Cart capacity, not its content, tells whether the character owns a cart
			buf.hasCart = (buf.cartMaxCount || 0) > 0 || (buf.cartMaxWeight || 0) > 0;
		}

		return buf;
	}

	buildStatusBuffer() {
		const buf = {};
		const statusContainer = this.containers.find(c => c.type === ContainerType.Status);
		if (!statusContainer) {
			return buf;
		}

		const readU32 = chunk =>
			new DataView(chunk.data.buffer, chunk.data.byteOffset, chunk.data.byteLength).getUint32(0, true);

		const readU32ById = (container, id) => {
			const ch = container.data.find(c => c.id === id);
			if (!ch || ch.data.byteLength < 4) {
				return null;
			}
			return readU32(ch);
		};

		// Bonus stats (chunks 2010..2015)
		const bonusStr = readU32ById(statusContainer, 2010);
		if (bonusStr !== null) {
			buf.str_bonus = bonusStr;
		}

		const bonusAgi = readU32ById(statusContainer, 2011);
		if (bonusAgi !== null) {
			buf.agi_bonus = bonusAgi;
		}

		const bonusVit = readU32ById(statusContainer, 2012);
		if (bonusVit !== null) {
			buf.vit_bonus = bonusVit;
		}

		const bonusInt = readU32ById(statusContainer, 2013);
		if (bonusInt !== null) {
			buf.int_bonus = bonusInt;
		}

		const bonusDex = readU32ById(statusContainer, 2014);
		if (bonusDex !== null) {
			buf.dex_bonus = bonusDex;
		}

		const bonusLuk = readU32ById(statusContainer, 2015);
		if (bonusLuk !== null) {
			buf.luk_bonus = bonusLuk;
		}

		// Weight (chunks 2017/2018)
		const curWeight = readU32ById(statusContainer, 2017);
		if (curWeight !== null) {
			buf.weight = curWeight;
		}

		const maxWeight = readU32ById(statusContainer, 2018);
		if (maxWeight !== null) {
			buf.max_weight = maxWeight;
		}

		// HP / SP (chunks 2029..2032)
		const hp = readU32ById(statusContainer, 2029);
		if (hp !== null) {
			buf.hp = hp;
		}

		const maxHp = readU32ById(statusContainer, 2030);
		if (maxHp !== null) {
			buf.maxHp = maxHp;
		}

		const sp = readU32ById(statusContainer, 2031);
		if (sp !== null) {
			buf.sp = sp;
		}

		const maxSp = readU32ById(statusContainer, 2032);
		if (maxSp !== null) {
			buf.maxSp = maxSp;
		}

		return buf;
	}

	buildPetBuffer() {
		const compContainer = this.containers.find(c => c.type === ContainerType.Companions);
		if (!compContainer) {
			return null;
		}

		const readU32 = chunk =>
			new DataView(chunk.data.buffer, chunk.data.byteOffset, chunk.data.byteLength).getUint32(0, true);

		const readU32ById = id => {
			const ch = compContainer.data.find(c => c.id === id);
			if (!ch || ch.data.byteLength < 4) {
				return null;
			}
			return readU32(ch);
		};

		const readStringById = id => {
			const ch = compContainer.data.find(c => c.id === id);
			if (!ch || ch.data.byteLength === 0) {
				return '';
			}
			return this.readString(ch.data);
		};

		const aid = readU32ById(ReplayOpCodes.PetGid);
		if (!aid) {
			return null; // No pet out at recording start
		}

		const name = readStringById(ReplayOpCodes.PetName);
		const viewRaw = readU32ById(ReplayOpCodes.PetJob) ?? 0;
		const level = readU32ById(ReplayOpCodes.PetLevel) ?? 1;
		const hunger = readU32ById(ReplayOpCodes.PetFullness) ?? 0;
		const intimacy = readU32ById(ReplayOpCodes.PetRelation) ?? 0;

		return {
			aid,
			name,
			view: viewRaw === 0xffffffff ? -1 : viewRaw,
			job: viewRaw === 0xffffffff ? -1 : viewRaw,
			level,
			hunger,
			intimacy
		};
	}

	buildEfstListBuffer() {
		const efstContainer = this.containers.find(c => c.type === ContainerType.EfstList);
		if (!efstContainer) {
			return [];
		}

		const result = [];
		for (const chunk of efstContainer.data) {
			if (!chunk.data || chunk.data.byteLength < 4) {
				continue;
			}
			const view = new DataView(chunk.data.buffer, chunk.data.byteOffset, 4);
			const efstId = view.getUint32(0, true);
			if (efstId > 0 && efstId <= 3000) {
				result.push(efstId);
			}
		}

		return result;
	}

	buildItemsBuffer() {
		const itemsContainer = this.containers.find(c => c.type === ContainerType.Items);
		if (!itemsContainer) {
			return { inventory: [], cart: [], equipped: [], equippedCostume: [] };
		}

		const out = {
			inventory: [],
			cart: [],
			equipped: [],
			equippedCostume: []
		};

		const SKIP_CHUNKS = new Set([4602, 4604, 4605, 4606]);
		const RECORD_SIZES = [221, 172];
		const NAMEID_OFFSET = 104;

		const detectRecordSize = (view, byteLength) => {
			const plausible = id => id > 0 && id < 5000000;
			for (const size of RECORD_SIZES) {
				if (byteLength < size || byteLength % size !== 0) {
					continue;
				}
				let anyValid = false;
				let ok = true;
				for (let r = 0; r < byteLength / size; r++) {
					const id = view.getInt32(r * size + NAMEID_OFFSET, true);
					if (id === 0) {
						continue;
					}
					if (!plausible(id)) {
						ok = false;
						break;
					}
					anyValid = true;
				}
				if (ok && anyValid) {
					return size;
				}
			}
			return 0;
		};

		const readTlvFields = (view, base, recordSize) => {
			const fields = new Map();
			let o = 0;
			while (o + 6 <= recordSize) {
				const tag = view.getUint16(base + o, true);
				const length = view.getUint32(base + o + 2, true);
				if (o + 6 + length > recordSize) {
					return null;
				}
				fields.set(tag, { offset: o + 6, length });
				o += 6 + length;
			}
			return o === recordSize ? fields : null;
		};

		const readTlvNumber = (view, base, fields, tag, fallback = 0) => {
			const field = fields.get(tag);
			if (!field) {
				return fallback;
			}
			switch (field.length) {
				case 1:
					return view.getUint8(base + field.offset);
				case 2:
					return view.getUint16(base + field.offset, true);
				case 4:
					return view.getInt32(base + field.offset, true);
				default:
					return fallback;
			}
		};

		const parseItemChunk = (chunk, targetArray) => {
			if (!chunk.data || chunk.data.byteLength === 0) {
				return;
			}
			const view = new DataView(chunk.data.buffer, chunk.data.byteOffset, chunk.data.byteLength);
			const recordSize = detectRecordSize(view, chunk.data.byteLength);
			if (recordSize === 0) {
				console.warn(
					`[ReplayParser] Unrecognized item record layout in chunk ${chunk.id} (${chunk.data.byteLength} bytes), items skipped`
				);
				return;
			}

			const recordCount = Math.floor(chunk.data.byteLength / recordSize);
			for (let r = 0; r < recordCount; r++) {
				const base = r * recordSize;
				const fields = readTlvFields(view, base, recordSize);

				let slot = 0;
				let equipped = 0;
				let qty = 0;
				let itemId = 0;
				let refine = 0;
				let grade = 0;
				const cards = [0, 0, 0, 0];
				const options = [];

				if (fields) {
					slot = readTlvNumber(view, base, fields, ItemRecordTag.POS, 0);
					equipped = readTlvNumber(view, base, fields, ItemRecordTag.WEAR_STATE, 0);
					qty = readTlvNumber(view, base, fields, ItemRecordTag.QTY, 0);
					itemId = readTlvNumber(view, base, fields, ItemRecordTag.NAMEID, 0);
					refine = readTlvNumber(view, base, fields, ItemRecordTag.REFINE, 0);
					grade = readTlvNumber(view, base, fields, ItemRecordTag.GRADE, 0);

					const cardsField = fields.get(ItemRecordTag.CARDS);
					if (cardsField && cardsField.length >= 16) {
						for (let c = 0; c < 4; c++) {
							cards[c] = view.getInt32(base + cardsField.offset + c * 4, true);
						}
					}

					const optField = fields.get(ItemRecordTag.OPTIONS);
					if (optField && optField.length >= 5) {
						const optCount = Math.min(5, Math.floor(optField.length / 5));
						for (let o = 0; o < optCount; o++) {
							const optOffset = base + optField.offset + o * 5;
							const optId = view.getUint16(optOffset, true);
							if (optId > 0) {
								const optVal = view.getInt16(optOffset + 2, true);
								const optParam = view.getUint8(optOffset + 4);
								options.push({ id: optId, value: optVal, param: optParam });
							}
						}
					}
				} else {
					// Fallback to absolute offsets
					slot = view.getInt16(base + 22, true);
					equipped = view.getInt32(base + 42, true);
					qty = view.getInt16(base + 52, true);
					for (let c = 0; c < 4; c++) {
						cards[c] = view.getInt32(base + 82 + c * 4, true);
					}
					itemId = view.getInt32(base + 104, true);
					refine = view.getUint8(base + 134);

					if (recordSize >= 221) {
						for (let o = 0; o < 5; o++) {
							const optOffset = base + 190 + o * 5;
							const optId = view.getUint16(optOffset, true);
							if (optId > 0) {
								const optVal = view.getInt16(optOffset + 2, true);
								const optParam = view.getUint8(optOffset + 4);
								options.push({ id: optId, value: optVal, param: optParam });
							}
						}
					}
				}

				if (itemId > 0) {
					let itemType = ItemType.ETC;
					if (
						chunk.id === ItemChunkKind.EQUIPPED_COSTUME ||
						equipped &
							(EquipmentLocation.COSTUME_HEAD_TOP |
								EquipmentLocation.COSTUME_HEAD_MID |
								EquipmentLocation.COSTUME_HEAD_BOTTOM |
								EquipmentLocation.COSTUME_ROBE |
								EquipmentLocation.COSTUME_FLOOR |
								EquipmentLocation.SHADOW_ARMOR |
								EquipmentLocation.SHADOW_WEAPON |
								EquipmentLocation.SHADOW_SHIELD |
								EquipmentLocation.SHADOW_SHOES |
								EquipmentLocation.SHADOW_R_ACCESSORY_SHADOW |
								EquipmentLocation.SHADOW_L_ACCESSORY_SHADOW)
					) {
						itemType = ItemType.SHADOWGEAR;
					} else if (equipped & EquipmentLocation.WEAPON) {
						itemType = ItemType.WEAPON;
					} else if (equipped & EquipmentLocation.AMMO) {
						itemType = ItemType.AMMO;
					} else if (equipped > 0) {
						itemType = ItemType.ARMOR;
					} else if (refine > 0 || grade > 0) {
						itemType = ItemType.ARMOR;
					} else if (itemId >= 501 && itemId <= 600) {
						itemType = ItemType.HEALING;
					} else if (itemId >= 601 && itemId <= 700) {
						itemType = ItemType.USABLE;
					} else if (itemId >= 4001 && itemId <= 4700) {
						itemType = ItemType.CARD;
					} else if (itemId >= 1101 && itemId <= 2100) {
						itemType = ItemType.WEAPON;
					} else if (itemId >= 2101 && itemId <= 2999) {
						itemType = ItemType.ARMOR;
					}

					targetArray.push({
						index: slot + 2,
						ITID: itemId,
						count: qty > 0 ? qty : 1,
						type: itemType,
						IsIdentified: true,
						IsDamaged: false,
						PlaceETCTab: false,
						WearState: equipped,
						location: equipped,
						RefiningLevel: refine,
						enchantgrade: grade,
						slot: {
							card1: cards[0] || 0,
							card2: cards[1] || 0,
							card3: cards[2] || 0,
							card4: cards[3] || 0
						},
						cards: [cards[0] || 0, cards[1] || 0, cards[2] || 0, cards[3] || 0],
						Options: options.map(opt => ({
							index: opt.id,
							value: opt.value,
							param: opt.param
						})),
						HireExpireDate: 0,
						bindOnEquipType: 0,
						wItemSpriteNumber: 0,
						// Aliases for compatibility
						itemId,
						qty: qty > 0 ? qty : 1,
						slotIndex: slot,
						refine,
						grade,
						options
					});
				}
			}
		};

		for (const chunk of itemsContainer.data) {
			if (SKIP_CHUNKS.has(chunk.id)) {
				continue;
			}
			switch (chunk.id) {
				case ItemChunkKind.INVENTORY:
					parseItemChunk(chunk, out.inventory);
					break;
				case ItemChunkKind.CART:
				case ItemChunkKind.CART_MIRROR:
					if (out.cart.length === 0) {
						parseItemChunk(chunk, out.cart);
					}
					break;
				case ItemChunkKind.EQUIPPED:
					parseItemChunk(chunk, out.equipped);
					break;
				case ItemChunkKind.EQUIPPED_COSTUME:
					parseItemChunk(chunk, out.equippedCostume);
					break;
			}
		}

		return out;
	}

	toNetworkPacket(chunk) {
		return chunk.data;
	}

	readString(uint8array) {
		let length = 0;
		while (length < uint8array.length && uint8array[length] !== 0) {
			length++;
		}
		const decoder = new TextDecoder('euc-kr');
		return decoder.decode(uint8array.subarray(0, length));
	}

	dumpContainers() {
		console.log('=== REPLAY CONTAINERS DUMP ===');
		for (let i = 0; i < this.containers.length; i++) {
			const container = this.containers[i];
			const containerName = ContainerTypeNames[container.type] || `Unknown(${container.type})`;

			console.log(
				`\n--- Container [${i}] Type: ${container.type} (${containerName}) | DeclaredLen: ${container.declaredLength} | Offset: ${container.offset} | RealLen: ${container.realLength} | Chunks: ${container.data.length} ---`
			);

			container.data.forEach((chunk, index) => {
				const data = chunk.data;
				const size = data ? data.byteLength : 0;
				const opName = chunk.id !== undefined ? ReplayOpCodeName[chunk.id] || '' : '';
				const packetInfo =
					chunk.packetId !== undefined
						? ` | PacketID: 0x${chunk.packetId.toString(16).padStart(4, '0')} (${chunk.packetId})`
						: '';
				const chunkHeader = `Chunk [${index}] ID: ${chunk.id ?? 'N/A'}${opName ? ` (${opName})` : ''}${packetInfo}${chunk.time !== undefined ? ` | Time: ${chunk.time}` : ''} | Data Size: ${size}`;

				console.log(chunkHeader);

				if (!data || size === 0) {
					console.log('  [Empty Content]');
					return;
				}

				if (size === 1) {
					const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
					const val = view.getUint8(0);
					console.log(`  Char: ${val} (0x${val.toString(16).padStart(2, '0')})`);
				} else if (size === 2) {
					const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
					const val = view.getUint16(0, true);
					console.log(`  uint16: ${val} (0x${val.toString(16).padStart(4, '0')})`);
				} else if (size === 4) {
					const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
					const val = view.getUint32(0, true);
					console.log(`  uint32: ${val} (0x${val.toString(16).padStart(8, '0')})`);
				} else if (size === 8) {
					const view = new DataView(data.buffer, data.byteOffset, data.byteLength);
					const val = view.getBigUint64(0, true);
					console.log(`  uint64: ${val} (0x${val.toString(16).padStart(16, '0')})`);
				}

				this.dumpHex(data);
			});
		}
		console.log('=== END OF REPLAY CONTAINERS DUMP ===');
	}

	dumpHex(uint8Array) {
		const lines = [];
		for (let offset = 0; offset < uint8Array.length; offset += 16) {
			const chunk = uint8Array.subarray(offset, offset + 16);

			const hexParts = [];
			const asciiParts = [];

			for (let i = 0; i < 16; i++) {
				if (i < chunk.length) {
					const byte = chunk[i];
					hexParts.push(byte.toString(16).padStart(2, '0'));
					asciiParts.push(byte >= 32 && byte <= 126 ? String.fromCharCode(byte) : '.');
				} else {
					hexParts.push('  ');
					asciiParts.push(' ');
				}
			}

			const hexStr = hexParts.slice(0, 8).join(' ') + '  ' + hexParts.slice(8, 16).join(' ');
			const asciiStr = asciiParts.join('');
			const offsetStr = offset.toString(16).padStart(4, '0');

			lines.push(`  ${offsetStr}: ${hexStr}  |${asciiStr}|`);
		}
		console.log(lines.join('\n'));
	}
}
