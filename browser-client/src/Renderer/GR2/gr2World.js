/**
 * Renderer/GR2/gr2World.js
 *
 * The GR2 world matrix — port of the sandbox `gr2BuilderRobrowser` (draw path
 * reconciled to roBrowser's /5 referential). Byte-cited from the client 3dmob draw
 * path (mars26 fcn.00b2ad20 / ver12 fcn.00567e70): Rx(+pi/2) . Ry((dir+180)) . S . T,
 * with the model's own granny InitialPlacement (ipRow) applied innermost.
 *
 * HANDEDNESS. There is deliberately NO model-space X reflection here. A granny-RH
 * model reaches the screen correctly through exactly ONE reflection: either a Y-up
 * camera with an in-model flipX . Rx(-pi/2), OR a Y-INVERTING camera with a clean
 * Rx(+pi/2) and no flipX — the two are equivalent, and stacking both mirrors the
 * model. roBrowser's camera already inverts world Y on screen (see computeGroundOffset
 * below / Camera.js "inversed Y-Z axis"), so that inversion IS the RH->LH bridge and
 * the world matrix stays a clean, reflection-free Rx . Ry . S . T (det +1). (The
 * sandbox, which keeps a plain Y-up camera, carries the flipX in-model instead.)
 *
 * MATRIX CONVENTION. The chain is built row-vector (D3D, v' = v.M) with the shared
 * `mul` helper (gr2Math.js), then the flat array is handed to GL / gl-matrix as-is. A row-major
 * array reinterpreted column-major IS its transpose, and column-vector M.v with the
 * transpose equals row-vector v.M — so `mat4.multiply(out, Camera.modelView, world)`
 * (gl-matrix is column-major) reproduces the sandbox `multiply4(view, model)` with no
 * extra transpose. Do NOT transpose the output.
 *
 * /5 REFERENTIAL (build-7). roBrowser lands the RO world at /5 (World/Altitude/Ground
 * divide position + scale + heights by 5). The GR2 geometry scales by RB_UNIT_SCALE
 * (0.2 = 1.0u / 5.0u) so the model reads 1.0-cell-sized; the world POSITION is left in the
 * caller's referential (co-located with the /5 ground) plus a +0.5 cell-CENTRE offset — the
 * roBrowser convention every 2D visual uses to reach the cell middle from the raw corner (see
 * RB_PLACEMENT_OFS). The model origin lands on the cell centre, co-located with roBrowser's 2D
 * sprite/shadow. See re-ro-clients-asm/sandbox/CORRESPONDENCE.md (build-7).
 *
 * YAW. `((dir + 180) / 180) * pi` — the +180 is asm-cited (mars26 fcn.00b2ad20.asm:160
 * addss 180.0 / ver12 fcn.00408b70). The sandbox leaves the offset vs roBrowser's own
 * Camera.direction remap (floor((angle[1]+22.5)/45)%8, rendered dir = (Camera.direction
 * + entity.direction) mod 8) UNRESOLVED by design — both sandbox halves share worldCore
 * so they coincide with no offset. Here it is pinned empirically on the live map (see the
 * baked defaults in worldCore) and will thread into the Entity direction remap.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 */

import { mul, trans, IDENTITY_ROW } from 'Renderer/GR2/gr2Math.js';

/** Geometry scale under the /5 referential (0.2 = 1.0u / 5.0u). */
export const RB_UNIT_SCALE = 0.2;

/**
 * Horizontal cell offset on X,Z. +0.5 = the CELL CENTRE — where every roBrowser 2D visual lands.
 * A raw integer entity position (x,y) is the cell CORNER; roBrowser reaches the cell middle by
 * adding +0.5: the sprite body + shadow in SpriteRenderer.vs ("xyz = x(-z)y + middle of cell
 * (0.5)"), Altitude.getCellHeight ("middle of the cell", +0.5), and the name/HP overlay
 * (EntityRender.js:93-95, position+0.5). The GR2 draw path bypasses SpriteRenderer.vs, so it must
 * add the +0.5 itself — otherwise the model sits half a cell off (on the corner) from the 2D
 * shadow it is read against and from the GridSelector cell quad ([x,x+1]x[y,y+1], centre
 * (x+0.5,y+0.5)). The flag mesh is centred at model X=0/Y=0, so +0.5
 * drops its central pillar on the cell centre.
 *
 * NOT the old -0.4 straddle: that was 0.2*-2.0 from the -2.0 (0xe316c8) in the DEAD CGrannyPc
 * draw fcn.00c03e10. The live flag draw C3dGrannyGameActor fcn.00b2ad20 reads a RAW origin
 * [esi+0x10] -> PostMultTranslate with no draw-time offset — but that origin is ITSELF the cell
 * CENTRE: the client anchors actors at the cell middle (confirmed against the official client
 * reference shot — the flag sits centred on its cell) and bakes the half-cell (+2.5 world = +0.5
 * cell) into the stored world position at cell->world conversion time. That is why the offset
 * surfaces at NO client consumer: the draw feeds the origin raw, the 2D sprite projector
 * (fcn.00539820/fcn.00539650) reads it raw, and the ground sampler fcn.0042c700 interpolates
 * height by the raw sub-cell fraction (0x42c7a0) and adds no +0.5 of its own — the fraction is
 * already 0.5. roBrowser takes the mirror approach: it stores the raw integer corner and adds
 * +0.5 at each consumer (SpriteRenderer.vs, Altitude.getCellHeight, EntityRender overlay, and here).
 * Same cell centre, offset applied at a different pipeline stage — so +0.5 faithfully reproduces
 * the client's centre anchor, it is not merely a roBrowser-internal convention. (The exact +2.5
 * immediate inside the client's cell->world conversion is a documented reverse-engineering stall:
 * the setter is unreachable statically from both the packet dispatcher and the actor factory.)
 */
export const RB_PLACEMENT_OFS = 0.5;

// Row-vector 4x4 helpers (D3DXMatrixRotationX / Y / Scaling). `mul`, `trans` and IDENTITY_ROW
// are shared with the loader via gr2Math.js; these three are GR2-world-only.
const _rotX = a => {
	const s = Math.sin(a);
	const c = Math.cos(a);
	return [1, 0, 0, 0, 0, c, s, 0, 0, -s, c, 0, 0, 0, 0, 1];
};
const _rotY = a => {
	const s = Math.sin(a);
	const c = Math.cos(a);
	return [c, 0, -s, 0, 0, 1, 0, 0, s, 0, c, 0, 0, 0, 0, 1];
};
const _scale = s => [s, 0, 0, 0, 0, s, 0, 0, 0, 0, s, 0, 0, 0, 0, 1];

// Effective yaw offset in degrees: the asm-cited +180 (mars26 fcn.00b2ad20.asm:160 addss 180.0
// / ver12 fcn.00408b70) plus a live-map +180 reconciliation against roBrowser's own
// Camera.direction remap — 360 total. Exposed via opts.yawOffsetDeg for re-tuning other eras.
const YAW_BASE_DEG = 360;

/**
 * worldCore(dir, pos) -> the reflection-free world core Rx(rxSign*pi/2) . Ry(yaw) . S . T
 * at the /5 referential (geometry x0.2, +0.5 cell-centre offset). Position is fed unchanged.
 */
export function worldCore(dir, pos, opts) {
	const o = opts || {};
	// Orientation reconciled on the live map against roBrowser's Y-INVERTING camera (which now
	// carries the sole granny-RH -> render-LH reflection; there is no in-model flipX). rxSign +1
	// (pitch), yaw offset YAW_BASE_DEG. The opts stay exposed for re-tuning other eras/models.
	const rxSign = o.rxSign != null ? o.rxSign : 1;
	const yawOffset = o.yawOffsetDeg != null ? o.yawOffsetDeg : YAW_BASE_DEG;
	const yaw = ((dir + yawOffset) / 180) * Math.PI;
	let m = mul(_rotX((rxSign * Math.PI) / 2), _rotY(yaw));
	m = mul(m, _scale(RB_UNIT_SCALE));
	m = mul(m, trans(pos[0] + RB_PLACEMENT_OFS, pos[1] + (o.heightOffset || 0), pos[2] + RB_PLACEMENT_OFS));
	return m;
}

/**
 * computeGroundOffset(meshes, ipRow, opts) -> the heightOffset that puts the model's base flush on
 * the terrain. roBrowser's render inverts world Y on screen (proven on the live map: a model placed
 * at world Y ABOVE the ground draws BELOW it / occluded), so the base — the part that must touch the
 * ground line — is the world-Y MAX vertex. We ground that: no vertex ends up above the ground's
 * world Y, so nothing renders under the terrain. Yaw-independent (uses the world matrix's Y-row).
 *
 * The model's granny InitialPlacement (ipRow) MUST be threaded here so grounding runs in the SAME
 * frame the draw uses (`v . ipRow . worldCore`). A non-identity IP (e.g. the treasurebox's 90 deg X
 * rotation, or the guardians' Y translation) reorients/shifts the mesh; grounding with a null IP
 * would compute the base in the wrong frame and the model floats / sinks / lies flat on the ground.
 */
export function computeGroundOffset(meshes, ipRow, opts) {
	const m = buildWorld(0, [0, 0, 0], ipRow, opts);
	const a = m[1];
	const b = m[5];
	const c = m[9];
	let max = -Infinity;
	for (let k = 0; k < meshes.length; k++) {
		const bind = meshes[k].bind;
		const vc = meshes[k].vcount;
		for (let i = 0; i < vc; i++) {
			const y = a * bind[i * 3] + b * bind[i * 3 + 1] + c * bind[i * 3 + 2];
			if (y > max) {
				max = y;
			}
		}
	}
	return max === -Infinity ? 0 : -max;
}

/**
 * buildWorld(dir, pos, ipRow) -> Float32Array(16) GR2 world matrix.
 * Chain (row-vector, IP innermost): v . ipRow . worldCore(dir, pos) — a clean,
 * reflection-free Rx . Ry . S . T (det +1). The granny-RH -> render-LH reflection is
 * provided by roBrowser's Y-inverting camera, not by this matrix (see HANDEDNESS above).
 * Hand the result straight to gl-matrix `mat4.multiply(out, view, world)` (see MATRIX
 * CONVENTION above) — it is already the column-major world matrix.
 *
 * @param {number} dir - actor facing fed into Ry((dir+180))
 * @param {number[]} pos - world position [x, y, z] (caller referential, left unscaled)
 * @param {number[]} [ipRow] - the model's granny InitialPlacement row-vector mat4 (identity for flags)
 */
export function buildWorld(dir, pos, ipRow, opts) {
	const o = opts || {};
	const ip = ipRow || IDENTITY_ROW;
	return new Float32Array(mul(ip, worldCore(dir, pos, o)));
}
