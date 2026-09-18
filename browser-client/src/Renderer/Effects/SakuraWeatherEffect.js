/**
 * Renderer/Effects/SakuraWeather.js
 *
 * Weather effect for Sakura (Cherry Blossom) and Maple leaves.
 * Adapted from C++ CRagEffect::Sakura/Maple and CEffectPrim::PrimSakura.
 * * Logic features:
 * - Spawns less frequently than snow.
 * - leaves sway back and forth (sine wave movement) rather than falling straight.
 * - Maple leaves fall significantly slower than Sakura petals.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import Client from 'Core/Client.js';
import Renderer from 'Renderer/Renderer.js';
import MapRenderer from 'Renderer/MapRenderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Session from 'Engine/SessionStorage.js';

// Constants based on RO Client behavior
const RAG_TICK_MS = 25;
const FADEOUT_TAIL_MS = 1000 * RAG_TICK_MS;

// Emission settings (Reduced quantity compared to snow)
// Snow is 2 per tick. Sakura/Maple is roughly 1 every 2 calls in C++, so ~1 per 150ms.
const EMIT_INTERVAL_MS = 150;
const EMIT_STOP_BEFORE_END_MS = 160 * RAG_TICK_MS;

// Lifetime
const LEAVE_LIFE_MS = 600 * RAG_TICK_MS; // Lasts a bit longer to allow slow falling
const LEAVE_FADEIN_MS = 20 * RAG_TICK_MS;
const LEAVE_FADEOUT_START_MS = (LEAVE_LIFE_MS * 4) / 5;

// Spatial constants
const SCATTER_RADIUS_CELLS = 70;
const SPAWN_HEIGHT_MIN_CELLS = 32;
const SPAWN_HEIGHT_MAX_CELLS = 42;

// Effect IDs
const _EF_SAKURA = 163;
const EF_MAPLE = 333;

// Paths
const PATH_SAKURA = 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/sakura01';
const PATH_MAPLE = 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xb4\xdc\xc7\xb3';

// SINGLETON STATE
let _instance = null;
let _mapName = '';
let _isStopping = false;

// Logic from CEffectPrim::PrimSakura to update sway angles
function updateSwayAngle(current, target) {
	if (target > current) {
		current += 3;
		if (current > 359) {
			current = 359;
		}
		if (current > target) {
			// Reached target, pick new random lower target
			target = Math.random() * current;
		}
	} else {
		current -= 2;
		if (current < 0) {
			current = 0;
		}
		if (current <= target) {
			// Reached target, pick new random higher target
			target = 359 - Math.random() * current;
		}
	}
	return { c: current, t: target };
}

// Helper function for rendering layers
function renderLayer(layer, spr, pal, sizeScale, pos, alpha) {
	if (layer.index < 0) {
		return;
	}

	SpriteRenderer.image.palette = null;
	SpriteRenderer.sprite = spr.frames[layer.index];
	SpriteRenderer.palette = pal.palette;

	let index = layer.index;
	const is_rgba = layer.spr_type === 1 || spr.rgba_index === 0;

	if (!is_rgba) {
		SpriteRenderer.image.palette = pal.texture;
		SpriteRenderer.image.size[0] = 2 * spr.frames[index].width;
		SpriteRenderer.image.size[1] = 2 * spr.frames[index].height;
	} else if (layer.spr_type === 1) {
		index += spr.old_rgba_index;
	}

	const frame = spr.frames[index];
	let width = frame.width * layer.scale[0] * sizeScale;
	const height = frame.height * layer.scale[1] * sizeScale;

	if (layer.is_mirror) {
		width = -width;
	}

	SpriteRenderer.color.set([layer.color[0], layer.color[1], layer.color[2], layer.color[3] * alpha]);

	SpriteRenderer.angle = layer.angle;
	SpriteRenderer.size[0] = width;
	SpriteRenderer.size[1] = height;
	SpriteRenderer.offset[0] = layer.pos[0] + pos[0];
	SpriteRenderer.offset[1] = layer.pos[1] + pos[1];
	SpriteRenderer.xSize = 5;
	SpriteRenderer.ySize = 5;
	SpriteRenderer.image.texture = frame.texture;

	SpriteRenderer.render(false);
}

class SakuraWeatherEffect {
	constructor(Params) {
		this.effectID = Params.Inst.effectID;
		this.startTick = Params.Inst.startTick;
		this.endTick = Params.Inst.endTick;

		this.lastEmitTick = this.startTick;
		this.leaves = [];
		this.isMaple = this.effectID === EF_MAPLE;

		this.spr = null;
		this.act = null;

		this.ready = true;
		this.needCleanUp = false;
	}

	static isActive() {
		return _instance;
	}

	static beforeRender(gl, modelView, projection, fog) {
		SpriteRenderer.shadow = 1;
		SpriteRenderer.angle = 0;
		SpriteRenderer.offset[0] = 0;
		SpriteRenderer.offset[1] = 0;
		SpriteRenderer.image.palette = null;
		SpriteRenderer.color.set([1, 1, 1, 1]);
		SpriteRenderer.depth = 0;
		SpriteRenderer.zIndex = 0;
	}

	static afterRender(gl) {
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}

	static startOrRestart(Params) {
		const now = Params.Inst.startTick || Renderer.tick;
		const currentMap = MapRenderer.currentMap;

		if (_mapName !== currentMap) {
			_instance = null;
			_mapName = currentMap;
		}
		_isStopping = false;

		// If instance exists and is valid
		if (_instance && !_instance.needCleanUp) {
			// Check if we are switching from Sakura to Maple or vice versa
			const isNewMaple = Params.Inst.effectID === EF_MAPLE;
			if (_instance.isMaple !== isNewMaple) {
				_instance = null; // Force recreate if type changed
			} else {
				if (_instance.endTick > 0) {
					_instance.endTick = -1;
					_instance.lastEmitTick = now;
				}
				return _instance;
			}
		}

		_instance = new SakuraWeatherEffect(Params);
		_mapName = currentMap;
		return _instance;
	}

	static renderAll(gl, modelView, projection, fog, tick) {
		if (!_instance) {
			return;
		}

		if (_mapName !== MapRenderer.currentMap) {
			_instance = null;
			return;
		}

		this.beforeRender(gl, modelView, projection, fog);

		SpriteRenderer.runWithDepth(false, false, true, () => {
			_instance.render(gl, tick);
		});

		if (_instance.needCleanUp) {
			_instance.free();
			_instance = null;
		}
		this.afterRender(gl);
	}

	static stop(ownerAID, tick) {
		if (!_instance) {
			return;
		}
		const now = tick || Renderer.tick;
		if (_instance.endTick === -1) {
			_instance.endTick = now + FADEOUT_TAIL_MS;
			_isStopping = true;
		}
	}

	spawnLeave(spawnTick) {
		if (!Session.Entity) {
			return;
		}

		const px = Session.Entity.position[0];
		const py = Session.Entity.position[1];

		// Random position around player
		const theta = Math.random() * Math.PI * 2;
		const radius = Math.random() * SCATTER_RADIUS_CELLS;
		const x = px + Math.cos(theta) * radius;
		const y = py + Math.sin(theta) * radius;

		const groundZ = Altitude.getCellHeight(x, y);
		const spawnHeight = SPAWN_HEIGHT_MIN_CELLS + Math.random() * (SPAWN_HEIGHT_MAX_CELLS - SPAWN_HEIGHT_MIN_CELLS);
		const z = groundZ + spawnHeight;

		// Speed
		// Sakura: (random(2)+2)*0.1f -> 0.2 to 0.3 internal units
		// Maple:  (random(4)+2)*0.03f -> 0.06 to 0.18 internal units (Much slower)
		const fallSpeedBase = this.isMaple ? (2 + Math.random() * 4) * 0.03 : (2 + Math.random() * 2) * 0.1;

		// Convert to roBrowser scale
		// Snow was 0.1 cells/tick.
		const fallSpeed = fallSpeedBase * 0.5; // Tweak this multiplier to taste

		// Sway Amplitude Factors
		// Sakura: X=0.24, Z=0.30
		// Maple:  X=0.12, Z=0.15
		const swayFactorX = this.isMaple ? 0.12 : 0.24;
		const swayFactorY = this.isMaple ? 0.15 : 0.3;

		this.leaves.push({
			spawnTick: spawnTick,
			x: x,
			y: y,
			z: z,
			baseX: x, // Center of the sway
			baseY: y, // Center of the sway
			size: 0.5 + Math.random() * 0.3,

			// Movement Props
			speed: fallSpeed,
			swayFacX: swayFactorX,
			swayFacY: swayFactorY,

			// Angle logic from C++
			// angX = current angle, targetX = destination angle
			angX: Math.random() * 360,
			targetX: Math.random() * 360,
			angY: Math.random() * 360, // Maps to C++ Z axis
			targetY: Math.random() * 360,

			_lastTick: spawnTick
		});
	}

	render(gl, tick) {
		if (!Session.Entity) {
			return;
		}

		const path = this.isMaple ? PATH_MAPLE : PATH_SAKURA;
		const spr = Client.loadFile(path + '.spr', null, null, { to_rgba: true });
		const act = Client.loadFile(path + '.act');

		if (!spr || !act) {
			return;
		}

		// Emission
		let remaining = Infinity;
		if (this.endTick > 0) {
			remaining = this.endTick - tick;
			if (remaining <= 0) {
				this.needCleanUp = true;
				return;
			}
		}

		const allowEmit = remaining > EMIT_STOP_BEFORE_END_MS;
		if (allowEmit) {
			// Check if enough time passed for next spawn
			if (tick - this.lastEmitTick >= EMIT_INTERVAL_MS) {
				if (!_isStopping) {
					// Spawn 1 leave per interval (reduced quantity)
					this.spawnLeave(tick);
				}
				this.lastEmitTick = tick;
			}
		}

		// Update & Render leaves
		const action = act.actions[0];
		const frameDelay = Math.max(action.delay || 150, 1);
		// Maple acts often have only 1 frame simple rotation, Sakura has animation.
		const frameCount = action.animations.length || 1;

		for (let f = this.leaves.length - 1; f >= 0; f--) {
			const leave = this.leaves[f];
			const age = tick - leave.spawnTick;

			if (age >= LEAVE_LIFE_MS) {
				this.leaves.splice(f, 1);
				continue;
			}

			// 1. Update Sway Angles
			const resX = updateSwayAngle(leave.angX, leave.targetX);
			leave.angX = resX.c;
			leave.targetX = resX.t;

			const resY = updateSwayAngle(leave.angY, leave.targetY);
			leave.angY = resY.c;
			leave.targetY = resY.t;

			// 2. Calculate Position
			// Vertical Fall
			const dt = tick - leave._lastTick;
			leave.z -= leave.speed * (dt / RAG_TICK_MS); // Scale speed by delta

			// Horizontal Sway (Sinusoidal offset from base position)
			// RO: vecB_pre.x += factor * sin(angle)
			// This accumulates in C++. Here we can just calculate offset from the "falling line".
			// However, C++ logic adds to the position vector directly.
			// Let's emulate accumulation or simple offset. Simple offset is smoother for web.
			// But to match the "drifting" feel, we drift the BaseX/Y.

			const radX = (leave.angX * Math.PI) / 180;
			const radY = (leave.angY * Math.PI) / 180;

			// Drift the center slightly based on wind (Sway Factor)
			const driftX = leave.swayFacX * Math.sin(radX);
			const driftY = leave.swayFacY * Math.sin(radY);

			// Apply drift to current position
			leave.x += driftX * 0.1; // Scale down for smoothness
			leave.y += driftY * 0.1;

			leave._lastTick = tick;

			// 3. Render
			let alpha = 1.0;

			let alphaCap = 1.0;
			if (!this.isMaple) {
				alphaCap = 0.5;
			}

			if (age < LEAVE_FADEIN_MS) {
				alpha = (age / LEAVE_FADEIN_MS) * alphaCap;
			} else if (age > LEAVE_FADEOUT_START_MS) {
				alpha = Math.max(
					0,
					(1 - (age - LEAVE_FADEOUT_START_MS) / (LEAVE_LIFE_MS - LEAVE_FADEOUT_START_MS)) * alphaCap
				);
			}

			SpriteRenderer.position[0] = leave.x;
			SpriteRenderer.position[1] = leave.y;
			SpriteRenderer.position[2] = leave.z;
			SpriteRenderer.zIndex = 0;

			const frameIndex = Math.floor(age / frameDelay) % frameCount;
			const animation = action.animations[frameIndex];
			const layers = animation.layers;
			const pal = spr;
			const pos = [0, 0];

			for (let l = 0; l < layers.length; l++) {
				renderLayer(layers[l], spr, pal, leave.size, pos, alpha);
			}
		}
	}

	free() {
		this.ready = false;
		this.leaves = [];
	}
}

export default SakuraWeatherEffect;
