import { describe, it, expect } from 'vitest';
import RSM from 'Loaders/Model.js';

/**
 * Helper: write a null-terminated string of fixed length.
 */
function writeFixedStr(view, offset, str, len) {
    for (let i = 0; i < len; i++) {
        view.setUint8(offset + i, i < str.length ? str.charCodeAt(i) : 0);
    }
}

/**
 * Build a synthetic RSM v1.4 binary (based on coin_j_01 structure).
 *
 * Layout (v < 2.2 path):
 *   "GRSM"(4) + major(1) + minor(1) + animLen(4) + shadeType(4) + alpha(1, v>=1.4)
 *   reserved(16) + textureCount(4) + textures(40 each)
 *   mainNodeName(40)
 *   nodeCount(4)
 *   Per node (v < 2.2):
 *     name(40) + parentname(40) + nodeTexCount(4) + nodeTextures(long each, v<2.3)
 *     mat3[9](36) + offset[3](12) + pos[3](12) + rotangle(4) + rotaxis[3](12) + scale[3](12)
 *     vertexCount(4) + vertices(12 each)
 *     tvertexCount(4) + tvertices(12 each, v>=1.2: 4 ubytes + 2 floats)
 *     faceCount(4) + faces(per face: 6 ushorts + 1 ushort + 1 ushort + 1 long + 1 long(v>=1.2) = 28)
 *     scaleKeyFrameCount(4, v>=1.6) + scaleKeyFrames — SKIPPED for v1.4
 *     rotKeyframeCount(4) + rotKeyframes(20 each) — always read
 *   posKeyframeCount(4, v<1.6) + posKeyframes(20 each) — read by RSM.load() for v1.4
 *   volumeboxCount(4) + volumeboxes
 */
function buildMinimalRSM() {
    const nodeName = 'test_node';
    const textureName = 'test\\texture.bmp';
    const vertCount = 4;
    const tvertCount = 4;
    const faceCount = 2;

    // Calculate size
    let size = 4 + 1 + 1 + 4 + 4 + 1; // header + animLen + shadeType + alpha
    size += 16;                          // reserved
    size += 4 + 40;                      // textureCount(1) + texture name
    size += 40;                          // mainNodeName
    size += 4;                           // nodeCount

    // Node
    size += 40 + 40;                     // name + parentname
    size += 4;                           // nodeTexCount
    size += 1 * 4;                       // 1 texture index (long)
    size += 9 * 4;                       // mat3
    size += 3 * 4;                       // offset
    size += 3 * 4;                       // pos
    size += 4;                           // rotangle
    size += 3 * 4;                       // rotaxis
    size += 3 * 4;                       // scale
    size += 4 + vertCount * 12;          // vertices
    size += 4 + tvertCount * (4 + 8);    // tvertices (v>=1.2: 4 ubytes + 2 floats)
    size += 4 + faceCount * (6 * 2 + 2 + 2 + 4 + 4); // faces (v>=1.2 adds smoothGroup)
    // scaleKeyFrames: SKIPPED for v1.4 (only read when v>=1.6)
    size += 4;                           // rotKeyframeCount (always read in Node constructor)
    size += 4;                           // posKeyframeCount (read by RSM.load() when v<1.6)
    size += 4;                           // volumeboxCount

    const buf = new ArrayBuffer(size);
    const view = new DataView(buf);
    let off = 0;

    // "GRSM"
    [0x47, 0x52, 0x53, 0x4D].forEach(b => view.setUint8(off++, b));
    view.setInt8(off++, 1);   // major: version = 1 + 4/10 = 1.4
    view.setInt8(off++, 4);   // minor
    view.setInt32(off, 32000, true); off += 4; // animLen
    view.setInt32(off, 2, true); off += 4;     // shadeType = SMOOTH
    view.setUint8(off++, 255);                 // alpha raw=255 → 255/255=1.0

    off += 16; // reserved

    // Textures
    view.setInt32(off, 1, true); off += 4;
    writeFixedStr(view, off, textureName, 40); off += 40;

    // Main node name
    writeFixedStr(view, off, nodeName, 40); off += 40;

    // Node count
    view.setInt32(off, 1, true); off += 4;

    // --- Node ---
    writeFixedStr(view, off, nodeName, 40); off += 40;     // name
    writeFixedStr(view, off, '', 40); off += 40;            // parentname (empty = root)

    // Node texture count + indices
    view.setInt32(off, 1, true); off += 4;
    view.setInt32(off, 0, true); off += 4; // texture index 0

    // mat3 (identity)
    const identity3 = [1, 0, 0, 0, 1, 0, 0, 0, 1];
    for (const v of identity3) { view.setFloat32(off, v, true); off += 4; }

    // offset
    for (let i = 0; i < 3; i++) { view.setFloat32(off, 0, true); off += 4; }
    // pos
    for (let i = 0; i < 3; i++) { view.setFloat32(off, 0, true); off += 4; }
    // rotangle
    view.setFloat32(off, 0, true); off += 4;
    // rotaxis
    for (let i = 0; i < 3; i++) { view.setFloat32(off, 0, true); off += 4; }
    // scale
    for (let i = 0; i < 3; i++) { view.setFloat32(off, 1.0, true); off += 4; }

    // Vertices (simple quad)
    view.setInt32(off, vertCount, true); off += 4;
    const verts = [[-1, 0, -1], [1, 0, -1], [1, 0, 1], [-1, 0, 1]];
    for (const v of verts) {
        for (const c of v) { view.setFloat32(off, c, true); off += 4; }
    }

    // Texture vertices (v>=1.2: 4 ubytes color + 2 floats UV)
    view.setInt32(off, tvertCount, true); off += 4;
    const uvs = [[0, 0], [1, 0], [1, 1], [0, 1]];
    for (const uv of uvs) {
        // 4 color bytes
        view.setUint8(off++, 255);
        view.setUint8(off++, 255);
        view.setUint8(off++, 255);
        view.setUint8(off++, 255);
        // u, v
        view.setFloat32(off, uv[0], true); off += 4;
        view.setFloat32(off, uv[1], true); off += 4;
    }

    // Faces (v>=1.2: adds smoothGroup long)
    view.setInt32(off, faceCount, true); off += 4;
    const faceData = [
        { vi: [0, 1, 2], ti: [0, 1, 2] },
        { vi: [0, 2, 3], ti: [0, 2, 3] }
    ];
    for (const f of faceData) {
        for (const v of f.vi) { view.setUint16(off, v, true); off += 2; } // vertidx[3]
        for (const t of f.ti) { view.setUint16(off, t, true); off += 2; } // tvertidx[3]
        view.setUint16(off, 0, true); off += 2;  // texid
        view.setUint16(off, 0, true); off += 2;  // padding
        view.setInt32(off, 0, true); off += 4;    // twoSide
        view.setInt32(off, 0, true); off += 4;    // smoothGroup (v>=1.2)
    }

    // scaleKeyFrames: NOT written (v1.4 < 1.6, parser skips this field)

    // Rotation keyframes: count=0 (always read in Node constructor)
    view.setInt32(off, 0, true); off += 4;

    // posKeyframes: count=0 (read by RSM.load() because v1.4 < 1.6)
    // NOTE: v<1.6 posKeyframes use { data: readFloat() } while v>=2.2 use { Data: readLong() }
    // — this casing/type inconsistency is a pre-existing issue in src/Loaders/Model.js:822 vs :184
    view.setInt32(off, 0, true); off += 4;

    // Volumebox count
    view.setInt32(off, 0, true); off += 4;

    return buf;
}

describe('RSM Loader', () => {
    it('rejects invalid header', () => {
        const buf = new ArrayBuffer(20);
        expect(() => new RSM(buf)).toThrow('Incorrect header');
    });

    it('parses synthetic RSM v1.4 correctly', () => {
        const data = buildMinimalRSM();
        const rsm = new RSM(data);
        expect(rsm.version).toBeCloseTo(1.4);
        expect(rsm.animLen).toBe(32000);
        expect(rsm.shadeType).toBe(2);
        expect(rsm.alpha).toBeCloseTo(1.0); // 255/255
    });

    it('has 1 node with correct name', () => {
        const data = buildMinimalRSM();
        const rsm = new RSM(data);
        expect(rsm.nodes.length).toBe(1);
        expect(rsm.main_node).toBeDefined();
        expect(rsm.main_node.name).toBe('test_node');
    });

    it('has 1 texture', () => {
        const data = buildMinimalRSM();
        const rsm = new RSM(data);
        expect(rsm.textures.length).toBe(1);
    });

    it('main_node has vertices and faces', () => {
        const data = buildMinimalRSM();
        const rsm = new RSM(data);
        expect(rsm.main_node.vertices.length).toBe(4);
        expect(rsm.main_node.faces.length).toBe(2);
    });
});