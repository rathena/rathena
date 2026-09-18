import { describe, it, expect } from 'vitest';
import STR from 'Loaders/Str.js';

/**
 * Size of one STRAnimation in bytes:
 *   frame(4) + type(4) + pos[2](8) + uv[8](32) + xy[8](32) +
 *   aniframe(4) + anitype(4) + delay(4) + angle(4) + color[4](16) +
 *   srcalpha(4) + destalpha(4) + mtpreset(4) = 124 bytes
 */
const ANIM_SIZE = 124;

/**
 * Build a synthetic STR v0x94 binary.
 *
 * Layout:
 *   "STRM"(4) + version(4) + fps(4) + maxKey(4) + layernum(4) + 16 unknown bytes
 *   Per layer: texcnt(4) + textures(128 each) + anikeynum(4) + animations(ANIM_SIZE each)
 *
 * @param {object} opts
 * @param {number} opts.fps
 * @param {number} opts.maxKey
 * @param {Array<{textures: number, keyframes: number}>} opts.layers
 */
function buildMinimalSTR(opts) {
    const { fps, maxKey, layers } = opts;
    const layernum = layers.length;

    let size = 4 + 4 + 4 + 4 + 4 + 16; // header + unknown
    for (const l of layers) {
        size += 4;                    // texcnt
        size += l.textures * 128;     // texture names
        size += 4;                    // anikeynum
        size += l.keyframes * ANIM_SIZE;
    }

    const buf = new ArrayBuffer(size);
    const view = new DataView(buf);
    let off = 0;

    // "STRM"
    [0x53, 0x54, 0x52, 0x4D].forEach(b => view.setUint8(off++, b));
    view.setUint32(off, 0x94, true); off += 4; // version
    view.setUint32(off, fps, true); off += 4;
    view.setUint32(off, maxKey, true); off += 4;
    view.setUint32(off, layernum, true); off += 4;
    off += 16; // unknown

    // Layers
    for (const l of layers) {
        view.setInt32(off, l.textures, true); off += 4;
        // Texture names (128 bytes each, fill with "tex\0...")
        for (let t = 0; t < l.textures; t++) {
            const name = `tex${t}.tga`;
            for (let c = 0; c < 128; c++) {
                view.setUint8(off + c, c < name.length ? name.charCodeAt(c) : 0);
            }
            off += 128;
        }
        // Keyframes
        view.setInt32(off, l.keyframes, true); off += 4;
        for (let k = 0; k < l.keyframes; k++) {
            // frame
            view.setInt32(off, k, true); off += 4;
            // type
            view.setUint32(off, 0, true); off += 4;
            // pos[2]
            view.setFloat32(off, 0, true); off += 4;
            view.setFloat32(off, 0, true); off += 4;
            // uv[8]
            for (let u = 0; u < 8; u++) { view.setFloat32(off, 0, true); off += 4; }
            // xy[8]
            for (let x = 0; x < 8; x++) { view.setFloat32(off, 0, true); off += 4; }
            // aniframe, anitype, delay, angle
            view.setFloat32(off, 0, true); off += 4;
            view.setUint32(off, 0, true); off += 4;
            view.setFloat32(off, 0, true); off += 4;
            view.setFloat32(off, 0, true); off += 4;
            // color[4]
            for (let c = 0; c < 4; c++) { view.setFloat32(off, 255, true); off += 4; }
            // srcalpha, destalpha, mtpreset
            view.setUint32(off, 0, true); off += 4;
            view.setUint32(off, 0, true); off += 4;
            view.setUint32(off, 0, true); off += 4;
        }
    }

    return buf;
}

describe('STR Loader', () => {
    it('rejects invalid header', () => {
        const buf = new ArrayBuffer(20);
        expect(() => new STR(buf)).toThrow('Incorrect header');
    });

    it('parses synthetic STR correctly', () => {
        // Based on real note_1.str values: fps=60, maxKey=90, 16 layers
        const layers = [];
        layers.push({ textures: 0, keyframes: 0 }); // layer 0
        for (let i = 1; i <= 6; i++) layers.push({ textures: 1, keyframes: 3 });
        layers.push({ textures: 1, keyframes: 2 }); // layer 7
        layers.push({ textures: 1, keyframes: 3 }); // layer 8
        for (let i = 9; i <= 15; i++) layers.push({ textures: 1, keyframes: 2 });

        const data = buildMinimalSTR({ fps: 60, maxKey: 90, layers });
        const str = new STR(data);
        expect(str.header).toBe('STRM');
        expect(str.version).toBe(0x94);
        expect(str.fps).toBe(60);
        expect(str.maxKey).toBe(90);
        expect(str.layernum).toBe(16);
    });

    it('has correct number of layers', () => {
        const layers = [
            { textures: 0, keyframes: 0 },
            { textures: 1, keyframes: 2 },
            { textures: 1, keyframes: 1 }
        ];
        const data = buildMinimalSTR({ fps: 30, maxKey: 10, layers });
        const str = new STR(data);
        expect(str.layers.length).toBe(3);
    });

    it('layers have valid structure', () => {
        const layers = [
            { textures: 1, keyframes: 2 },
            { textures: 0, keyframes: 0 }
        ];
        const data = buildMinimalSTR({ fps: 60, maxKey: 20, layers });
        const str = new STR(data);
        for (let i = 0; i < str.layers.length; i++) {
            expect(str.layers[i]).toBeDefined();
            expect(str.layers[i].texname).toBeDefined();
            expect(str.layers[i].animations).toBeDefined();
        }
    });
});