import { describe, it, expect } from 'vitest';
import GND from 'Loaders/Ground.js';

/**
 * Build a synthetic GND binary.
 *
 * Layout:
 *   "GRGN"(4) + major(1) + minor(1) + width(4) + height(4) + zoom(4)
 *   Textures: count(4) + nameLen(4) + names(nameLen each)
 *   Lightmaps: count(4) + per_cell_x(4) + per_cell_y(4) + size_cell(4) + data(count*per_cell*4)
 *   Tiles: count(4) + tile data (40 bytes each)
 *   Surfaces: width*height * (4 floats + 3 longs = 28 bytes)
 *   Water: only if version >= 1.8
 *
 * @param {number} width
 * @param {number} height
 * @param {object} opts
 */
function buildMinimalGND(width, height, opts = {}) {
    const major = opts.major ?? 1;
    const minor = opts.minor ?? 7; // version 1.7
    const zoom = opts.zoom ?? 10.0;
    const textureCount = opts.textureCount ?? 0;
    const tileCount = opts.tileCount ?? 0;
    const lightmapCount = opts.lightmapCount ?? 0;
    const surfaceCount = width * height;

    // Lightmap params
    const per_cell_x = 8;
    const per_cell_y = 8;
    const size_cell = 1; // per_cell = 8*8*1 = 64
    const per_cell = per_cell_x * per_cell_y * size_cell;

    let size = 4 + 1 + 1 + 4 + 4 + 4; // header
    // Textures
    size += 4 + 4; // count + nameLen
    size += textureCount * 40; // texture names (40 bytes each)
    // Lightmaps
    size += 4 + 4 + 4 + 4; // count + per_cell_x + per_cell_y + size_cell
    size += lightmapCount * per_cell * 4;
    // Tiles
    size += 4; // tile count
    size += tileCount * (8 * 4 + 2 + 2 + 4); // 8 floats + texIdx(2) + light(2) + color(4) = 40
    // Surfaces
    size += surfaceCount * (4 * 4 + 3 * 4); // 4 floats + 3 longs = 28

    const buf = new ArrayBuffer(size);
    const view = new DataView(buf);
    let off = 0;

    // "GRGN"
    [0x47, 0x52, 0x47, 0x4E].forEach(b => view.setUint8(off++, b));
    view.setUint8(off++, major);
    view.setUint8(off++, minor);
    view.setUint32(off, width, true); off += 4;
    view.setUint32(off, height, true); off += 4;
    view.setFloat32(off, zoom, true); off += 4;

    // Textures
    view.setUint32(off, textureCount, true); off += 4;
    view.setUint32(off, 40, true); off += 4; // nameLen
    off += textureCount * 40;

    // Lightmaps
    view.setInt32(off, lightmapCount, true); off += 4;
    view.setInt32(off, per_cell_x, true); off += 4;
    view.setInt32(off, per_cell_y, true); off += 4;
    view.setInt32(off, size_cell, true); off += 4;
    off += lightmapCount * per_cell * 4;

    // Tiles
    view.setUint32(off, tileCount, true); off += 4;
    off += tileCount * 40;

    // Surfaces
    for (let i = 0; i < surfaceCount; i++) {
        view.setFloat32(off, -5.0, true); off += 4; // h1
        view.setFloat32(off, -5.0, true); off += 4; // h2
        view.setFloat32(off, -5.0, true); off += 4; // h3
        view.setFloat32(off, -5.0, true); off += 4; // h4
        view.setInt32(off, -1, true); off += 4;      // tile_up
        view.setInt32(off, -1, true); off += 4;      // tile_front
        view.setInt32(off, -1, true); off += 4;      // tile_right
    }

    return buf;
}

describe('GND Loader', () => {
    it('rejects invalid header', () => {
        const buf = new ArrayBuffer(20);
        expect(() => new GND(buf)).toThrow('Invalid header');
    });

    it('parses synthetic GND (based on ma_zif01 values) correctly', () => {
        const data = buildMinimalGND(30, 30);
        const gnd = new GND(data);
        expect(gnd.version).toBeCloseTo(1.7);
        expect(gnd.width).toBe(30);
        expect(gnd.height).toBe(30);
        expect(typeof gnd.zoom).toBe('number');
    });

    it('has correct surface count', () => {
        const data = buildMinimalGND(30, 30);
        const gnd = new GND(data);
        expect(gnd.surfaces.length).toBe(900); // 30 * 30
    });

    it('does not have water (version 1.7 < 1.8)', () => {
        const data = buildMinimalGND(4, 4);
        const gnd = new GND(data);
        expect(gnd.water).toBeUndefined();
    });

    it('tiles array is defined', () => {
        const data = buildMinimalGND(4, 4);
        const gnd = new GND(data);
        expect(gnd.tiles).toBeDefined();
        expect(Array.isArray(gnd.tiles)).toBe(true);
    });
});