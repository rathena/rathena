import { describe, it, expect } from 'vitest';
import ACT from 'Loaders/Action.js';

/**
 * Helper: write a null-terminated string of fixed length into a DataView.
 */
function writeFixedString(view, offset, str, len) {
    for (let i = 0; i < len; i++) {
        view.setUint8(offset + i, i < str.length ? str.charCodeAt(i) : 0);
    }
}

/**
 * Build a synthetic ACT v2.5 binary.
 *
 * Layout:
 *   "AC"(2) + minor(1) + major(1)
 *   actionCount(2) + 10 unknown bytes
 *   Per action: animationCount(4)
 *     Per animation: 32 unknown bytes + layerCount(4)
 *       Per layer (v2.5): pos[2](8) + index(4) + is_mirror(4) + color[4](4) +
 *                          scaleX(4) + scaleY(4) + angle(4) + spr_type(4) + width(4) + height(4) = 44 bytes
 *       sound(4) + posCount(4) [+ per pos: 4+4+4+4 = 16]
 *   soundCount(4) + sounds(40 each)
 *   delays: float per action
 *
 * @param {number[]} framesPerAction - array of frame counts per action
 * @param {number} soundCount
 */
function buildMinimalACT(framesPerAction, soundCount) {
    const actionCount = framesPerAction.length;
    const layersPerFrame = 1;

    // Calculate size
    let size = 2 + 1 + 1;        // header
    size += 2 + 10;               // actionCount + unknown
    for (let a = 0; a < actionCount; a++) {
        size += 4; // animationCount
        for (let f = 0; f < framesPerAction[a]; f++) {
            size += 32;           // unknown bytes per animation
            size += 4;            // layerCount
            // v2.5 layer: 4+4+4+4+1+1+1+1+4+4+4+4+4+4 = 44 bytes
            size += layersPerFrame * (4 + 4 + 4 + 4 + 1 + 1 + 1 + 1 + 4 + 4 + 4 + 4 + 4 + 4);
            size += 4;            // sound index
            size += 4;            // pos count (v>=2.3)
            // 0 pos entries
        }
    }
    size += 4;                    // soundCount
    size += soundCount * 40;      // sound strings
    size += actionCount * 4;      // delays (v>=2.2)

    const buf = new ArrayBuffer(size);
    const view = new DataView(buf);
    let off = 0;

    // Header: "AC"
    view.setUint8(off++, 0x41); // 'A'
    view.setUint8(off++, 0x43); // 'C'
    view.setUint8(off++, 5);    // minor: version = 5/10 + 2 = 2.5
    view.setUint8(off++, 2);    // major

    // Action count + 10 unknown bytes
    view.setUint16(off, actionCount, true); off += 2;
    off += 10; // skip unknown

    // Actions
    for (let a = 0; a < actionCount; a++) {
        const animCount = framesPerAction[a];
        view.setUint32(off, animCount, true); off += 4;

        for (let f = 0; f < animCount; f++) {
            off += 32; // unknown bytes

            // Layer count
            view.setUint32(off, layersPerFrame, true); off += 4;

            for (let l = 0; l < layersPerFrame; l++) {
                // pos[0], pos[1]
                view.setInt32(off, 0, true); off += 4;
                view.setInt32(off, 0, true); off += 4;
                // index
                view.setInt32(off, 0, true); off += 4;
                // is_mirror
                view.setInt32(off, 0, true); off += 4;
                // color RGBA (v>=2.0): 4 UBytes
                view.setUint8(off++, 255);
                view.setUint8(off++, 255);
                view.setUint8(off++, 255);
                view.setUint8(off++, 255);
                // scaleX (v>=2.0)
                view.setFloat32(off, 1.0, true); off += 4;
                // scaleY (v>2.3 reads separate, v<=2.3 copies scaleX) — v2.5 reads separate
                view.setFloat32(off, 1.0, true); off += 4;
                // angle
                view.setInt32(off, 0, true); off += 4;
                // spr_type
                view.setInt32(off, 0, true); off += 4;
                // width, height (v>=2.5)
                view.setInt32(off, 24, true); off += 4;
                view.setInt32(off, 24, true); off += 4;
            }

            // sound (v>=2.0)
            view.setInt32(off, -1, true); off += 4;
            // pos count (v>=2.3)
            view.setInt32(off, 0, true); off += 4;
        }
    }

    // Sounds (v>=2.1)
    view.setInt32(off, soundCount, true); off += 4;
    for (let s = 0; s < soundCount; s++) {
        writeFixedString(view, off, `sound_${s}.wav`, 40);
        off += 40;
    }

    // Delays (v>=2.2): float per action
    for (let a = 0; a < actionCount; a++) {
        view.setFloat32(off, 6.0, true); off += 4; // 6.0 * 25 = 150
    }

    return buf;
}

describe('ACT Loader', () => {
    it('rejects invalid header', () => {
        const buf = new ArrayBuffer(20);
        expect(() => new ACT(buf)).toThrow('Incorrect header');
    });

    it('parses synthetic ACT v2.5 correctly', () => {
        // 3 actions with 2, 3, 1 frames
        const data = buildMinimalACT([2, 3, 1], 2);
        const act = new ACT(data);
        expect(act.header).toBe('AC');
        expect(act.version).toBeCloseTo(2.5);
        expect(act.actions.length).toBe(3);
    });

    it('reads sounds (version >= 2.1)', () => {
        const data = buildMinimalACT([1, 1], 4);
        const act = new ACT(data);
        expect(act.sounds.length).toBe(4);
    });

    it('reads delays per action (version >= 2.2)', () => {
        const data = buildMinimalACT([2, 2, 2], 0);
        const act = new ACT(data);
        for (let i = 0; i < act.actions.length; i++) {
            expect(act.actions[i].delay).toBeDefined();
            expect(typeof act.actions[i].delay).toBe('number');
        }
    });

    it('has correct frame counts per action group', () => {
        const frameCounts = [4, 4, 3, 3, 2];
        const data = buildMinimalACT(frameCounts, 1);
        const act = new ACT(data);
        for (let i = 0; i < frameCounts.length; i++) {
            expect(act.actions[i].animations.length).toBe(frameCounts[i]);
        }
    });

    it('each animation has layers array', () => {
        const data = buildMinimalACT([2], 0);
        const act = new ACT(data);
        const firstAnim = act.actions[0].animations[0];
        expect(Array.isArray(firstAnim.layers)).toBe(true);
        expect(firstAnim.layers.length).toBe(1);
        const layer = firstAnim.layers[0];
        expect(layer).toHaveProperty('index');
        expect(layer).toHaveProperty('pos');
    });

    it('compile() returns valid structure', () => {
        const data = buildMinimalACT([1, 1], 1);
        const act = new ACT(data);
        const compiled = act.compile();
        expect(compiled).toBeDefined();
        expect(compiled.actions).toBeDefined();
        expect(compiled.sounds).toBeDefined();
    });
});