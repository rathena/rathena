import { describe, it, expect } from 'vitest';
import RSW from 'Loaders/World.js';

/**
 * Helper: write a null-terminated string of fixed length.
 */
function writeFixedStr(view, offset, str, len) {
    for (let i = 0; i < len; i++) {
        view.setUint8(offset + i, i < str.length ? str.charCodeAt(i) : 0);
    }
}

/**
 * Build a synthetic RSW v2.1 binary (based on ma_zif01 values).
 *
 * Layout:
 *   "GRSW"(4) + major(1) + minor(1)
 *   ini(40) + gnd(40) + gat(40) + src(40) (v>=1.4)
 *   Water: level(4) + type(4) + waveHeight(4) + waveSpeed(4) + wavePitch(4) + animSpeed(4) (v>=1.8)
 *   Light: longitude(4) + latitude(4) + diffuse[3](12) + ambient[3](12) + opacity(4) (v>=1.5,1.7)
 *   Ground: top(4) + bottom(4) + left(4) + right(4) (v>=1.6)
 *   Objects: count(4) + per object data
 *
 * @param {object} opts
 */
function buildMinimalRSW(opts = {}) {
    const modelCount = opts.models ?? 0;

    // Model object size (v>=1.3):
    // name(40) + animType(4) + animSpeed(4) + blockType(4) + filename(80) + nodename(80) +
    // position[3](12) + rotation[3](12) + scale[3](12) = 248 bytes
    // Plus the object type tag (4 bytes)
    const modelObjSize = 4 + 40 + 4 + 4 + 4 + 80 + 80 + 12 + 12 + 12;

    let size = 4 + 1 + 1;              // header
    size += 40 * 4;                     // ini + gnd + gat + src
    size += 4 + 4 + 4 + 4 + 4 + 4;     // water (v>=1.3,1.8,1.9)
    size += 4 + 4 + 12 + 12 + 4;       // light (v>=1.5,1.7)
    size += 4 * 4;                      // ground (v>=1.6)
    size += 4;                          // object count
    size += modelCount * modelObjSize;

    const buf = new ArrayBuffer(size);
    const view = new DataView(buf);
    let off = 0;

    // "GRSW"
    [0x47, 0x52, 0x53, 0x57].forEach(b => view.setUint8(off++, b));
    view.setInt8(off++, 2);  // major: version = 2 + 1/10 = 2.1
    view.setInt8(off++, 1);  // minor

    // Sub files
    writeFixedStr(view, off, 'test.ini', 40); off += 40;
    writeFixedStr(view, off, 'test_map.gnd', 40); off += 40;
    writeFixedStr(view, off, 'test_map.gat', 40); off += 40;
    writeFixedStr(view, off, 'test_map.rsw', 40); off += 40; // src (v>=1.4)

    // Water (v>=1.3): level
    view.setFloat32(off, 0, true); off += 4; // level raw=0 → 0/5=0
    // Water (v>=1.8): type, waveHeight, waveSpeed, wavePitch
    view.setInt32(off, 0, true); off += 4;                    // type
    view.setFloat32(off, 1.0, true); off += 4;                // waveHeight raw=1.0 → 1.0/5=0.2
    view.setFloat32(off, 2.0, true); off += 4;                // waveSpeed
    view.setFloat32(off, 50.0, true); off += 4;               // wavePitch
    // Water (v>=1.9): animSpeed
    view.setInt32(off, 3, true); off += 4;

    // Light (v>=1.5)
    view.setInt32(off, 45, true); off += 4;                   // longitude
    view.setInt32(off, 45, true); off += 4;                   // latitude
    view.setFloat32(off, 1.0, true); off += 4;                // diffuse R
    view.setFloat32(off, 1.0, true); off += 4;                // diffuse G
    view.setFloat32(off, 1.0, true); off += 4;                // diffuse B
    view.setFloat32(off, 0.3, true); off += 4;                // ambient R
    view.setFloat32(off, 0.3, true); off += 4;                // ambient G
    view.setFloat32(off, 0.3, true); off += 4;                // ambient B
    // Light (v>=1.7)
    view.setFloat32(off, 0.5, true); off += 4;                // opacity

    // Ground (v>=1.6)
    view.setInt32(off, 0, true); off += 4;                    // top
    view.setInt32(off, 0, true); off += 4;                    // bottom
    view.setInt32(off, -500, true); off += 4;                 // left
    view.setInt32(off, 500, true); off += 4;                  // right

    // Objects
    view.setInt32(off, modelCount, true); off += 4;
    for (let m = 0; m < modelCount; m++) {
        view.setInt32(off, 1, true); off += 4; // type = model
        writeFixedStr(view, off, `model_${m}`, 40); off += 40; // name
        view.setInt32(off, 0, true); off += 4;   // animType
        view.setFloat32(off, 1.0, true); off += 4; // animSpeed
        view.setInt32(off, 0, true); off += 4;   // blockType
        writeFixedStr(view, off, `model_${m}.rsm`, 80); off += 80; // filename
        writeFixedStr(view, off, `node_${m}`, 80); off += 80; // nodename
        // position[3], rotation[3], scale[3]
        for (let v = 0; v < 9; v++) {
            view.setFloat32(off, v < 6 ? 0 : 5.0, true); off += 4; // scale=5.0/5=1.0
        }
    }

    return buf;
}

describe('RSW Loader', () => {
    it('rejects invalid header', () => {
        const buf = new ArrayBuffer(20);
        expect(() => new RSW(buf)).toThrow('Invalid header');
    });

    it('parses synthetic RSW v2.1 correctly', () => {
        const data = buildMinimalRSW({ models: 1 });
        const rsw = new RSW(data);
        expect(rsw.files.gnd).toBeDefined();
        expect(rsw.files.gat).toBeDefined();
        expect(rsw.water).toBeDefined();
        expect(rsw.light).toBeDefined();
    });

    it('reads water properties (version 2.1 >= 1.8)', () => {
        const data = buildMinimalRSW();
        const rsw = new RSW(data);
        expect(rsw.water.level).toBeCloseTo(0);
        expect(rsw.water.type).toBe(0);
        expect(rsw.water.waveHeight).toBeCloseTo(0.2); // raw 1.0 / 5
        expect(rsw.water.waveSpeed).toBeCloseTo(2);
        expect(rsw.water.wavePitch).toBeCloseTo(50);
        expect(rsw.water.animSpeed).toBe(3);
    });

    it('reads light properties (version >= 1.5)', () => {
        const data = buildMinimalRSW();
        const rsw = new RSW(data);
        expect(rsw.light.longitude).toBe(45);
        expect(rsw.light.latitude).toBe(45);
        expect(rsw.light.diffuse).toEqual([1, 1, 1]);
        expect(rsw.light.ambient[0]).toBeCloseTo(0.3);
        expect(rsw.light.ambient[1]).toBeCloseTo(0.3);
        expect(rsw.light.ambient[2]).toBeCloseTo(0.3);
        expect(rsw.light.opacity).toBeCloseTo(0.5);
    });

    it('reads ground frustum (version >= 1.6)', () => {
        const data = buildMinimalRSW();
        const rsw = new RSW(data);
        expect(rsw.ground.top).toBe(0);
        expect(rsw.ground.bottom).toBe(0);
        expect(rsw.ground.left).toBe(-500);
        expect(rsw.ground.right).toBe(500);
    });

    it('reads 1 model object', () => {
        const data = buildMinimalRSW({ models: 1 });
        const rsw = new RSW(data);
        expect(rsw.models.length).toBe(1);
    });

    it('compile() returns valid structure', () => {
        const data = buildMinimalRSW({ models: 1 });
        const rsw = new RSW(data);
        const compiled = rsw.compile();
        expect(compiled).toBeDefined();
        expect(compiled.water).toBeDefined();
        expect(compiled.light).toBeDefined();
    });
});