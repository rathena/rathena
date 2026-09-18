import { describe, it, expect } from 'vitest';
import GAT from 'Loaders/Altitude.js';

/**
 * Build a synthetic GAT binary based on real format structure.
 * Layout: "GRAT"(4) + major(1) + minor(1) + width(4) + height(4)
 * Per cell: 4 floats (heights) + 1 uint32 (type) = 20 bytes
 *
 * @param {number} width
 * @param {number} height
 * @param {object} opts - optional overrides
 */
function buildMinimalGat(width, height, opts = {}) {
    const major = opts.major ?? 1;
    const minor = opts.minor ?? 2; // version 1.2
    const headerSize = 4 + 1 + 1 + 4 + 4;
    const cellCount = width * height;
    const buf = new ArrayBuffer(headerSize + cellCount * 20);
    const view = new DataView(buf);
    let offset = 0;

    // "GRAT"
    [0x47, 0x52, 0x41, 0x54].forEach(b => view.setUint8(offset++, b));
    view.setUint8(offset++, major);
    view.setUint8(offset++, minor);
    view.setUint32(offset, width, true); offset += 4;
    view.setUint32(offset, height, true); offset += 4;

    // Cell types: 0=walkable, 1=non-walkable, 3=water
    const types = [0, 1, 3, 0, 5, 0];
    for (let i = 0; i < cellCount; i++) {
        const h = (i % 10) * 1.5; // varied heights
        view.setFloat32(offset, h, true); offset += 4;
        view.setFloat32(offset, h + 0.1, true); offset += 4;
        view.setFloat32(offset, h + 0.2, true); offset += 4;
        view.setFloat32(offset, h + 0.3, true); offset += 4;
        view.setUint32(offset, types[i % types.length], true); offset += 4;
    }
    return buf;
}

describe('GAT Loader', () => {
    it('parses a minimal 2x2 gat file', () => {
        const buf = buildMinimalGat(2, 2);
        const gat = new GAT(buf);
        expect(gat.width).toBe(2);
        expect(gat.height).toBe(2);
        expect(gat.version).toBeCloseTo(1.2);
        expect(gat.cells.length).toBe(2 * 2 * 5);
    });

    it('rejects invalid header', () => {
        const buf = new ArrayBuffer(14);
        expect(() => new GAT(buf)).toThrow('Invalid header');
    });
});

describe('GAT Loader with synthetic fixture (based on ma_zif01 values)', () => {
    it('parses 60x60 grid correctly', () => {
        const data = buildMinimalGat(60, 60);
        const gat = new GAT(data);
        expect(gat.version).toBeCloseTo(1.2);
        expect(gat.width).toBe(60);
        expect(gat.height).toBe(60);
        expect(gat.cells.length).toBe(60 * 60 * 5); // 18000
    });

    it('cells contain valid height and type data', () => {
        const data = buildMinimalGat(60, 60);
        const gat = new GAT(data);
        // Check first cell has 5 values (4 heights + 1 type)
        for (let i = 0; i < 5; i++) {
            expect(typeof gat.cells[i]).toBe('number');
            expect(isNaN(gat.cells[i])).toBe(false);
        }
        // Type values should be valid (from TYPE_TABLE)
        for (let i = 0; i < gat.width * gat.height; i++) {
            const type = gat.cells[i * 5 + 4];
            expect(type).toBeDefined();
        }
    });

    it('compile() returns valid structure', () => {
        const data = buildMinimalGat(4, 4);
        const gat = new GAT(data);
        const compiled = gat.compile();
        expect(compiled.width).toBe(4);
        expect(compiled.height).toBe(4);
        expect(compiled.cells.length).toBe(4 * 4 * 5);
    });
});