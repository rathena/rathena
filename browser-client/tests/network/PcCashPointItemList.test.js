import { describe, it, expect, beforeEach } from 'vitest';
import PACKET from 'Network/PacketStructure.js';
import PACKETVER from 'Network/PacketVerManager.js';
import BinaryReader from 'Utils/BinaryReader.js';

const HEADER = 8; // KafraPoint + CashPoint

function buildItems(items, { wideId, extended }) {
	const idSize = wideId ? 4 : 2;
	const itemLen = 9 + idSize + (extended ? 7 : 0);
	const buf = new ArrayBuffer(HEADER + items.length * itemLen);
	const view = new DataView(buf);
	view.setUint32(0, 111, true);
	view.setUint32(4, 222, true);
	let pos = HEADER;
	for (const item of items) {
		view.setInt32(pos, item.price, true);
		view.setInt32(pos + 4, item.discountprice, true);
		view.setUint8(pos + 8, item.type);
		if (wideId) {
			view.setUint32(pos + 9, item.ITID, true);
		} else {
			view.setUint16(pos + 9, item.ITID, true);
		}
		pos += 9 + idSize;
		if (extended) {
			view.setUint16(pos, item.viewSprite, true);
			view.setInt32(pos + 2, item.location, true);
			view.setUint8(pos + 6, 0);
			pos += 7;
		}
	}
	return buf;
}

function parse(buf) {
	const fp = new BinaryReader(buf);
	return new PACKET.ZC.PC_CASH_POINT_ITEMLIST(fp, buf.byteLength);
}

function makeItems(n, extended) {
	return Array.from({ length: n }, (_, i) => ({
		price: 1000 + i,
		discountprice: 500 + i,
		type: i % 5,
		ITID: 501 + i,
		...(extended ? { viewSprite: 10 + i, location: 1 << (i % 30) } : {})
	}));
}

function expectItems(pkt, items, extended) {
	expect(pkt.KafraPoint).toBe(111);
	expect(pkt.CashPoint).toBe(222);
	expect(pkt.itemList).toHaveLength(items.length);
	items.forEach((item, i) => {
		const got = pkt.itemList[i];
		expect(got.price).toBe(item.price);
		expect(got.discountprice).toBe(item.discountprice);
		expect(got.type).toBe(item.type);
		expect(got.ITID).toBe(item.ITID);
		if (extended) {
			expect(got.viewSprite).toBe(item.viewSprite);
			expect(got.location).toBe(item.location);
		} else {
			expect(got.viewSprite).toBeUndefined();
		}
	});
}

describe('PACKET.ZC.PC_CASH_POINT_ITEMLIST', () => {
	describe('PACKETVER >= 20181121 (13-byte base entries)', () => {
		beforeEach(() => {
			PACKETVER.value = 20221005;
		});

		it.each([0, 1, 13, 18, 20, 22, 40, 54])('parses %i base entries', (n) => {
			const items = makeItems(n, false);
			expectItems(parse(buildItems(items, { wideId: true, extended: false })), items, false);
		});

		it.each([1, 7, 18, 20])('parses %i extended entries', (n) => {
			const items = makeItems(n, true);
			expectItems(parse(buildItems(items, { wideId: true, extended: true })), items, true);
		});

		it('never reads past the end of the packet', () => {
			const buf = buildItems(makeItems(3, false), { wideId: true, extended: false });
			const truncated = buf.slice(0, buf.byteLength - 5);
			const pkt = parse(truncated);
			expect(pkt.itemList).toHaveLength(2);
			expect(pkt.itemList.every(Boolean)).toBe(true);
		});
	});

	describe('PACKETVER < 20181121 (11-byte base entries)', () => {
		beforeEach(() => {
			PACKETVER.value = 20130618;
		});

		it.each([0, 1, 18, 20, 36])('parses %i base entries', (n) => {
			const items = makeItems(n, false);
			expectItems(parse(buildItems(items, { wideId: false, extended: false })), items, false);
		});

		it.each([1, 5, 13, 20])('parses %i extended entries', (n) => {
			const items = makeItems(n, true);
			expectItems(parse(buildItems(items, { wideId: false, extended: true })), items, true);
		});
	});
});
