import { describe, it, expect } from 'vitest';
import GR2Loader from 'Loaders/GR2Loader.js';

/**
 * Build a synthetic granny-ro-js `parsed` object: two textured meshes (a body and an
 * emblem "Plane01"), plus a matless proxy submesh that packModel must drop. The skeleton
 * has 3 bones; each mesh binds a subset of them by name so we can assert local->global remap.
 */
function buildParsed() {
    const skeleton = { bones: [{ name: 'root' }, { name: 'mid' }, { name: 'tip' }] };

    // Body: 2 verts. boneBindings [mid, root] => local->global [1, 0].
    const body = {
        name: 'Object08',
        vertexCount: 2,
        positions: [[0, 0, 0], [1, 2, 3]],
        normals: [[1, 0, 0], [0, 0, 1]],
        uvs: [[0, 0], [1, 1]],
        indices: [0, 1, 0],
        boneBindings: [{ name: 'mid' }, { name: 'root' }],
        // vert0: weighted to local 0 (=> global 1) and local 1 (=> global 0), unnormalized sum 2.
        // vert1: no weights => rigid, must bind fully to l2g[0] (=> global 1) with weight 1.
        vertexWeights: [[{ boneIndex: 0, weight: 1 }, { boneIndex: 1, weight: 1 }], []],
        materials: [{ textureFile: 'flag.tga' }]
    };

    // Emblem: 1 vert. Name matches /Plane/i => emblem flag true.
    const emblem = {
        name: 'Plane01',
        vertexCount: 1,
        positions: [[5, 5, 5]],
        normals: [[0, 1, 0]],
        uvs: [[0.5, 0.5]],
        indices: [0],
        boneBindings: [{ name: 'tip' }],
        vertexWeights: [[{ boneIndex: 0, weight: 1 }]],
        materials: [{ textureFile: 'emblem.bmp' }]
    };

    // Matless proxy: no textured material => dropped.
    const proxy = {
        name: 'Bip01 Proxy',
        vertexCount: 1,
        positions: [[0, 0, 0]],
        normals: [[0, 0, 1]],
        uvs: [[0, 0]],
        indices: [0],
        boneBindings: [{ name: 'root' }],
        vertexWeights: [[{ boneIndex: 0, weight: 1 }]],
        materials: []
    };

    return { skeletons: [skeleton], meshes: [body, emblem, proxy] };
}

describe('GR2Loader.packModel', () => {
    it('drops matless proxy submeshes', () => {
        const { meshes } = GR2Loader.packModel(buildParsed());
        expect(meshes).toHaveLength(2);
        expect(meshes.map(m => m.name)).toEqual(['Object08', 'Plane01']);
    });

    it('reports the global bone count', () => {
        const { boneCount } = GR2Loader.packModel(buildParsed());
        expect(boneCount).toBe(3);
    });

    it('remaps mesh-local bone bindings to global skeleton indices', () => {
        const { meshes } = GR2Loader.packModel(buildParsed());
        const body = meshes[0];
        // vert0 local bones 0,1 => global 1 (mid), 0 (root).
        expect(body.bidx[0]).toBe(1);
        expect(body.bidx[1]).toBe(0);
    });

    it('binds rigid (weightless) vertices fully to the first bound bone', () => {
        const { meshes } = GR2Loader.packModel(buildParsed());
        const body = meshes[0];
        // vert1 had no weights => bidx = l2g[0] = global 1, weight 1.
        expect(body.bidx[4]).toBe(1);
        expect(body.bw[4]).toBeCloseTo(1, 6);
    });

    it('normalizes the top-4 weights to sum 1', () => {
        const { meshes } = GR2Loader.packModel(buildParsed());
        const body = meshes[0];
        const sum = body.bw[0] + body.bw[1] + body.bw[2] + body.bw[3];
        expect(sum).toBeCloseTo(1, 6);
        // vert0 had equal unnormalized weights => 0.5 / 0.5.
        expect(body.bw[0]).toBeCloseTo(0.5, 6);
        expect(body.bw[1]).toBeCloseTo(0.5, 6);
    });

    it('passes normals through unit-length', () => {
        const { meshes } = GR2Loader.packModel(buildParsed());
        for (const m of meshes) {
            for (let i = 0; i < m.vcount; i++) {
                const x = m.nrm[i * 3];
                const y = m.nrm[i * 3 + 1];
                const z = m.nrm[i * 3 + 2];
                expect(Math.hypot(x, y, z)).toBeCloseTo(1, 6);
            }
        }
    });

    it('emits attribute arrays that are 16-float interleavable (16 * vcount)', () => {
        const { meshes } = GR2Loader.packModel(buildParsed());
        for (const m of meshes) {
            const N = m.vcount;
            expect(m.bind).toHaveLength(N * 3);
            expect(m.nrm).toHaveLength(N * 3);
            expect(m.uv).toHaveLength(N * 2);
            expect(m.bidx).toHaveLength(N * 4);
            expect(m.bw).toHaveLength(N * 4);
            const total = m.bind.length + m.nrm.length + m.uv.length + m.bidx.length + m.bw.length;
            expect(total).toBe(N * 16);
            expect(m.indices).toBeInstanceOf(Uint16Array);
        }
    });

    it('flags the Plane* mesh as the emblem', () => {
        const { meshes } = GR2Loader.packModel(buildParsed());
        expect(meshes[0].emblem).toBe(false);
        expect(meshes[1].emblem).toBe(true);
    });

    it('throws when the model has no skeleton', () => {
        expect(() => GR2Loader.packModel({ skeletons: [], meshes: [] })).toThrow(/no skeleton/);
    });
});

describe('GR2Loader.ipRowFromTransform', () => {
    const IDENTITY = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1];

    it('returns identity when the transform is absent', () => {
        expect(Array.from(GR2Loader.ipRowFromTransform(undefined))).toEqual(IDENTITY);
    });

    it('returns identity when flags is 0', () => {
        const M = GR2Loader.ipRowFromTransform({ flags: 0, position: [9, 9, 9] });
        expect(Array.from(M)).toEqual(IDENTITY);
    });

    it('applies HAS_POSITION INVERTED (the InitialPlacement is undone to render at the actor origin)', () => {
        const M = GR2Loader.ipRowFromTransform({ flags: 1, position: [5, 6, 7] });
        // Row-vector translation carries -p: a model authored off-origin is re-centred onto
        // its cell (Kguardian90_7 IP [0.045,-7.253,0] -> +7.253 forward, aligning it with siblings).
        expect(M[12]).toBeCloseTo(-5, 6);
        expect(M[13]).toBeCloseTo(-6, 6);
        expect(M[14]).toBeCloseTo(-7, 6);
        // Linear part stays identity.
        expect(M[0]).toBeCloseTo(1, 6);
        expect(M[5]).toBeCloseTo(1, 6);
        expect(M[10]).toBeCloseTo(1, 6);
    });

    it('applies HAS_ORIENTATION quat Rx(+90) (treasurebox case)', () => {
        const s = Math.SQRT1_2; // 0.70710678
        const M = GR2Loader.ipRowFromTransform({ flags: 2, orientation: [s, 0, 0, s] });
        // Row-vector Rx(+90): [1,0,0, 0,0,1, 0,-1,0].
        const expected = [1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1];
        for (let i = 0; i < 16; i++) {
            expect(M[i]).toBeCloseTo(expected[i], 6);
        }
    });

    it('applies HAS_SCALESHEAR as the 3x3 linear block', () => {
        const M = GR2Loader.ipRowFromTransform({ flags: 4, scaleShear: [2, 0, 0, 0, 3, 0, 0, 0, 4] });
        expect(M[0]).toBeCloseTo(2, 6);
        expect(M[5]).toBeCloseTo(3, 6);
        expect(M[10]).toBeCloseTo(4, 6);
    });
});
