import { describe, it, expect } from 'vitest';
import SPR from 'Loaders/Sprite.js';

/**
 * Encode pixel data as SPR RLE.
 * RLE rule: byte 0x00 followed by count means "repeat 0x00 count times".
 * Non-zero bytes are literal.
 */
function encodeRLE(pixelData) {
    const out = [];
    let i = 0;
    while (i < pixelData.length) {
        const c = pixelData[i];
        if (c === 0) {
            // Count consecutive zeros
            let count = 0;
            while (i < pixelData.length && pixelData[i] === 0 && count < 255) {
                count++;
                i++;
            }
            out.push(0, count);
        } else {
            out.push(c);
            i++;
        }
    }
    return new Uint8Array(out);
}

/**
 * Build a synthetic SPR v2.1 binary.
 * Based on real SPR structure: "SP"(2) + minor(1) + major(1) + indexed_count(2) + rgba_count(2)
 * Per indexed RLE frame: width(2) + height(2) + rleSize(2) + rleData
 * Palette: 1024 bytes at end of file.
 *
 * @param {Array<{w:number, h:number, pixels?:Uint8Array}>} frames - frame dimensions and optional pixel data
 */
function buildMinimalSPR(frames) {
    // Build RLE data for each frame
    const rleFrames = frames.map(f => {
        let pixels;
        if (f.pixels) {
            pixels = f.pixels;
        } else {
            pixels = new Uint8Array(f.w * f.h);
            for (let j = 0; j < pixels.length; j++) {
                pixels[j] = (j % 254) + 1; // non-zero to avoid RLE runs (simple case)
            }
        }
        return { w: f.w, h: f.h, rle: encodeRLE(pixels) };
    });

    // Calculate total size
    let dataSize = 2 + 1 + 1 + 2 + 2; // header
    for (const f of rleFrames) {
        dataSize += 2 + 2 + 2 + f.rle.length; // width + height + rleSize + rleData
    }
    dataSize += 1024; // palette

    const buf = new ArrayBuffer(dataSize);
    const view = new DataView(buf);
    const bytes = new Uint8Array(buf);
    let off = 0;

    // "SP"
    view.setUint8(off++, 0x53); // 'S'
    view.setUint8(off++, 0x50); // 'P'
    view.setUint8(off++, 1);    // minor: version = 1/10 + 2 = 2.1
    view.setUint8(off++, 2);    // major
    view.setUint16(off, frames.length, true); off += 2; // indexed_count
    view.setUint16(off, 0, true); off += 2;             // rgba_count (v > 1.1)

    // Write RLE frames
    for (const f of rleFrames) {
        view.setUint16(off, f.w, true); off += 2;
        view.setUint16(off, f.h, true); off += 2;
        view.setUint16(off, f.rle.length, true); off += 2;
        bytes.set(f.rle, off); off += f.rle.length;
    }

    // Palette at end (256 RGBA entries = 1024 bytes)
    for (let i = 0; i < 256; i++) {
        bytes[dataSize - 1024 + i * 4 + 0] = i;       // R
        bytes[dataSize - 1024 + i * 4 + 1] = 255 - i;  // G
        bytes[dataSize - 1024 + i * 4 + 2] = 128;       // B
        bytes[dataSize - 1024 + i * 4 + 3] = 255;       // A
    }

    return buf;
}

describe('SPR Loader', () => {
    it('rejects invalid header', () => {
        const buf = new ArrayBuffer(20);
        expect(() => new SPR(buf)).toThrow('Incorrect header');
    });

    it('parses synthetic SPR v2.1 correctly', () => {
        // Based on real values: 9 indexed frames, 0 RGBA, frame[0]=54x52
        const frameDims = [
            { w: 54, h: 52 }, { w: 30, h: 40 }, { w: 20, h: 25 },
            { w: 48, h: 44 }, { w: 36, h: 30 }, { w: 42, h: 38 },
            { w: 28, h: 32 }, { w: 50, h: 46 }, { w: 34, h: 28 }
        ];
        const data = buildMinimalSPR(frameDims);
        const spr = new SPR(data);
        expect(spr.header).toBe('SP');
        expect(spr.version).toBeCloseTo(2.1);
        expect(spr.indexed_count).toBe(9);
        expect(spr.rgba_count).toBe(0);
        expect(spr.frames.length).toBe(9);
        expect(spr.rgba_index).toBe(9);
    });

    it('has palette of 1024 bytes (version > 1.0)', () => {
        const data = buildMinimalSPR([{ w: 4, h: 4 }]);
        const spr = new SPR(data);
        expect(spr.palette).toBeInstanceOf(Uint8Array);
        expect(spr.palette.length).toBe(1024);
    });

    it('first frame has correct dimensions (RLE decoded)', () => {
        const data = buildMinimalSPR([{ w: 54, h: 52 }, { w: 10, h: 10 }]);
        const spr = new SPR(data);
        expect(spr.frames[0].width).toBe(54);
        expect(spr.frames[0].height).toBe(52);
    });

    it('all frames have valid dimensions', () => {
        const frameDims = [
            { w: 54, h: 52 }, { w: 30, h: 40 }, { w: 20, h: 25 }
        ];
        const data = buildMinimalSPR(frameDims);
        const spr = new SPR(data);
        for (let i = 0; i < spr.frames.length; i++) {
            expect(spr.frames[i].width).toBeGreaterThan(0);
            expect(spr.frames[i].height).toBeGreaterThan(0);
            expect(spr.frames[i].data).toBeDefined();
        }
    });

    it('compile() returns valid structure', () => {
        const data = buildMinimalSPR([{ w: 8, h: 8 }, { w: 4, h: 4 }]);
        const spr = new SPR(data);
        const compiled = spr.compile();
        expect(compiled).toBeDefined();
        expect(compiled.frames).toBeDefined();
        expect(compiled.palette).toBeDefined();
    });

    it('decodes RLE zero-runs correctly', () => {
        // Build pixel data with mixed zero-runs and literals to exercise
        // all three RLE decoder paths in Sprite.js:122-135:
        //   1. Non-zero literal byte
        //   2. Zero byte + count > 0 (run of zeros)
        //   3. Zero byte + count = 0 (literal zero, edge case)
        const w = 8;
        const h = 4;
        const pixels = new Uint8Array(w * h);
        // Row 0: all non-zero literals
        for (let x = 0; x < w; x++) pixels[x] = x + 1;
        // Row 1: run of 8 zeros (exercises path 2: 0x00 + count=8)
        // pixels[8..15] already 0
        // Row 2: mixed — literal, zeros, literal
        pixels[16] = 5;
        // pixels[17..20] = 0 (run of 4 zeros)
        pixels[21] = 10;
        pixels[22] = 20;
        pixels[23] = 30;
        // Row 3: starts with zeros
        // pixels[24..29] = 0 (run of 6 zeros)
        pixels[30] = 42;
        pixels[31] = 99;

        const data = buildMinimalSPR([{ w, h, pixels }]);
        const spr = new SPR(data);

        expect(spr.frames.length).toBe(1);
        expect(spr.frames[0].width).toBe(w);
        expect(spr.frames[0].height).toBe(h);

        const decoded = spr.frames[0].data;
        expect(decoded.length).toBe(w * h);

        // Verify row 0: non-zero literals preserved
        for (let x = 0; x < w; x++) {
            expect(decoded[x]).toBe(x + 1);
        }
        // Verify row 1: zero-run decoded correctly
        for (let x = 0; x < w; x++) {
            expect(decoded[w + x]).toBe(0);
        }
        // Verify row 2: mixed pattern
        expect(decoded[16]).toBe(5);
        expect(decoded[17]).toBe(0);
        expect(decoded[18]).toBe(0);
        expect(decoded[19]).toBe(0);
        expect(decoded[20]).toBe(0);
        expect(decoded[21]).toBe(10);
        expect(decoded[22]).toBe(20);
        expect(decoded[23]).toBe(30);
        // Verify row 3: zero-run then literals
        for (let x = 24; x < 30; x++) {
            expect(decoded[x]).toBe(0);
        }
        expect(decoded[30]).toBe(42);
        expect(decoded[31]).toBe(99);
    });
});