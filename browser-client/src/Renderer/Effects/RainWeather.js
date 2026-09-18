/**
 * Renderer/Effects/RainWeather.js
 *
 * Weather rain effect (EF_RAIN).
 * Procedural multi‑layer rain designed for aesthetic depth:
 * - Looping emitter pinned to the owning entity (usually local player).
 * - Coherent, slowly‑varying wind so streaks slant consistently.
 * - Three depth layers (far/mid/near) with different speeds, sizes, and opacity.
 * - Alpha fades by fall progress (not raw time) to avoid popping.
 * - Added procedural audio system for Rain and Thunder (Web Audio API).
 * - Added lightning visual effect using the existing overlay system.
 */

import Renderer from 'Renderer/Renderer.js';
import MapRenderer from 'Renderer/MapRenderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Camera from 'Renderer/Camera.js';
import Preferences from 'Preferences/Audio.js';
import Session from 'Engine/SessionStorage.js';
import _puddleVs from './RainWeather.vs?raw';
import _puddleFs from './RainWeather.fs?raw';
import WebGL from 'Utils/WebGL.js';

const RAG_TICK_MS = 25;
const FADEOUT_TAIL_MS = 1000 * RAG_TICK_MS;

// Emission control.
const EMIT_PER_TICK = 10;
const EMIT_STOP_BEFORE_END_MS = 160 * RAG_TICK_MS;
const MAX_DROPS = 1100;

// Spawn volume around player (in map cells). Large enough to cover the view.
const SCATTER_RADIUS_CELLS = 70;
const SPAWN_HEIGHT_MIN_CELLS = 22;
const SPAWN_HEIGHT_MAX_CELLS = 42;

// Base wind, slowly varying over time (cells per rag tick).
const WIND_STRENGTH_BASE = 0.14;
const WIND_STRENGTH_VAR = 0.06;
const WIND_ANGLE_MAX_RAD = 0.55; // ~31deg sideways

// Thunderstorm
const THUNDER_MIN_INTERVAL = 5000;
const THUNDER_MAX_INTERVAL = 60000;
const FLASH_FADE_IN = 50;
const FLASH_FADE_OUT = 300;
const RAIN_VOLUME = 0.03;

// Depth layers: far/mid/near.
// Each layer defines relative look and speed.
const LAYERS = [
	{
		weight: 0.55,
		speedTick: [0.35, 0.55],
		widthPx: [1.0, 1.8],
		lengthPx: [22, 36],
		alphaScale: 0.12,
		brightness: [0.45, 0.65]
	},
	{
		weight: 0.3,
		speedTick: [0.55, 0.85],
		widthPx: [1.6, 2.6],
		lengthPx: [36, 55],
		alphaScale: 0.22,
		brightness: [0.65, 0.85]
	},
	{
		weight: 0.15,
		speedTick: [0.85, 1.25],
		widthPx: [2.3, 3.4],
		lengthPx: [55, 78],
		alphaScale: 0.35,
		brightness: [0.8, 1.0]
	}
];

// Precompute CDF for layer picking.
const _layerCDF = (function () {
	const out = [];
	let acc = 0;
	for (let i = 0; i < LAYERS.length; i++) {
		acc += LAYERS[i].weight;
		out.push(acc);
	}
	return out;
})();

// Puddle settings (Water accumulation system)
const MAX_PUDDLES = 600; // Increased limit to prevent popping
const PUDDLE_INITIAL_SIZE = 20.0;
const PUDDLE_MAX_SIZE = 230.0;
const PUDDLE_GROWTH_PER_DROP = 4.5;
const PUDDLE_ALPHA_MAX = 0.45;
const PUDDLE_FADE_SPEED = 0.00004; // Alpha per ms (faster drying)
const PUDDLE_SEED_CHANCE = 0.06; // Lowered to favor accumulation over spam

// Procedural raindrop sprite (1x16 alpha‑gradient texture stretched into streaks).
let _dropFrame = null;
// Full-screen grey filter (1x1 texture).
let _filterFrame = null;
// Ground splash texture (16x16 soft disc).
let _splashFrame = null;
const SPLASH_LIFE_MS = 220;
const SPLASH_SIZE_PX = [10, 18];
const SPLASH_ALPHA = 0.45;

// Puddle texture (64x64 deformed disc).
let _puddleFrame = null;

// Shader resources
let _puddleProgram = null;
let _puddleBuffer = null;

// SINGLETON STATE
let _instance = null;
let _mapName = '';
let _isStopping = false;

function ensureDropFrame(gl) {
	if (_dropFrame && _dropFrame.texture && gl.isTexture(_dropFrame.texture)) {
		return;
	}

	_dropFrame = null;
	const tex = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, tex);

	// Build a 1x16 vertical gradient (top transparent → bright core → soft tail).
	const h = 16;
	const data = new Uint8Array(h * 4);
	for (let i = 0; i < h; i++) {
		const t = i / (h - 1);
		// Ramp up to full alpha, then taper slightly.
		let a = Math.min(1, t / 0.7);
		if (t > 0.7) {
			a *= 1 - ((t - 0.7) / 0.3) * 0.35;
		}
		const alphaByte = Math.max(0, Math.min(255, Math.floor(a * 255)));
		const idx = i * 4;
		data[idx + 0] = 255;
		data[idx + 1] = 255;
		data[idx + 2] = 255;
		data[idx + 3] = alphaByte;
	}

	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

	_dropFrame = { texture: tex, width: 1, height: h, type: 1 };
}

function ensureFilterFrame(gl) {
	if (_filterFrame && _filterFrame.texture && gl.isTexture(_filterFrame.texture)) {
		return;
	}
	_filterFrame = null;

	const tex = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, tex);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, new Uint8Array([255, 255, 255, 255]));
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

	_filterFrame = { texture: tex, width: 1, height: 1, type: 1 };
}

function ensureSplashFrame(gl) {
	if (_splashFrame && _splashFrame.texture && gl.isTexture(_splashFrame.texture)) {
		return;
	}
	_splashFrame = null;

	const w = 16,
		h = 16;
	const data = new Uint8Array(w * h * 4);
	const cx = (w - 1) / 2;
	const cy = (h - 1) / 2;
	const maxR = Math.min(cx, cy);

	for (let y = 0; y < h; y++) {
		for (let x = 0; x < w; x++) {
			const dx = x - cx;
			const dy = y - cy;
			const r = Math.sqrt(dx * dx + dy * dy) / maxR;
			let a = 0;
			if (r <= 1) {
				// Soft disc with slight ring emphasis.
				a = Math.exp(-r * r * 2.8);
				const ring = Math.exp(-Math.pow((r - 0.65) / 0.12, 2));
				a = Math.max(a, ring * 0.9);
			}
			const alphaByte = Math.max(0, Math.min(255, Math.floor(a * 255)));
			const idx = (y * w + x) * 4;
			data[idx + 0] = 255;
			data[idx + 1] = 255;
			data[idx + 2] = 255;
			data[idx + 3] = alphaByte;
		}
	}

	const tex = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, tex);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

	_splashFrame = { texture: tex, width: w, height: h, type: 1 };
}

function ensurePuddleFrame(gl) {
	if (_puddleFrame && _puddleFrame.texture && gl.isTexture(_puddleFrame.texture)) {
		return;
	}
	_puddleFrame = null;

	const w = 64,
		h = 64;
	const data = new Uint8Array(w * h * 4);
	const cx = (w - 1) / 2;
	const cy = (h - 1) / 2;
	const maxR = Math.min(cx, cy) * 0.85;

	for (let y = 0; y < h; y++) {
		for (let x = 0; x < w; x++) {
			const dx = x - cx;
			const dy = y - cy;
			const dist = Math.sqrt(dx * dx + dy * dy);
			const angle = Math.atan2(dy, dx);

			// Procedural deformation for "organic" look
			const deform =
				1.0 + 0.15 * Math.sin(angle * 5) + 0.08 * Math.sin(angle * 3 + 1.5) + 0.05 * Math.cos(angle * 11);
			const r = dist / (maxR * deform);

			let a = 0;
			if (r <= 1.0) {
				// Soft concave falloff for pool look
				a = Math.pow(1.0 - r, 0.7);
			}

			const alphaByte = Math.max(0, Math.min(255, Math.floor(a * 255)));
			const idx = (y * w + x) * 4;
			// Deep wet-blue/dark tone
			data[idx + 0] = 18;
			data[idx + 1] = 32;
			data[idx + 2] = 55;
			data[idx + 3] = alphaByte;
		}
	}

	const tex = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, tex);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, data);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

	_puddleFrame = { texture: tex, width: w, height: h, type: 1 };
}

function initPuddleShader(gl) {
	if (_puddleProgram) {
		return;
	}
	_puddleProgram = WebGL.createShaderProgram(gl, _puddleVs, _puddleFs);

	// Generate horizontal quad buffer (XZ plane)
	const vertices = new Float32Array([
		// Triangle 1 (pos: x, y, z, tex: u, v)
		-0.5, 0.0, -0.5, 0.0, 0.0, 0.5, 0.0, -0.5, 1.0, 0.0, -0.5, 0.0, 0.5, 0.0, 1.0,
		// Triangle 2
		0.5, 0.0, -0.5, 1.0, 0.0, 0.5, 0.0, 0.5, 1.0, 1.0, -0.5, 0.0, 0.5, 0.0, 1.0
	]);
	_puddleBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, _puddleBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
}

function randRange(min, max) {
	return min + Math.random() * (max - min);
}

function pickLayerIndex() {
	const r = Math.random() * _layerCDF[_layerCDF.length - 1];
	for (let i = 0; i < _layerCDF.length; i++) {
		if (r <= _layerCDF[i]) {
			return i;
		}
	}
	return _layerCDF.length - 1;
}

function computeGlobalWind(tick) {
	// Slow oscillation: changes over ~10s.
	const t = tick * 0.00012;
	const strengthTick = WIND_STRENGTH_BASE + WIND_STRENGTH_VAR * Math.sin(t * 2.3);
	const angleCam = Math.sin(t * 1.1) * WIND_ANGLE_MAX_RAD;
	const camXTick = Math.cos(angleCam) * strengthTick;
	const camYTick = Math.sin(angleCam) * strengthTick * 0.35;

	// Convert camera‑space wind to world‑space so on‑screen direction stays consistent
	// even when the camera rotates.
	const yawRad = (((Camera.angle && Camera.angle[1]) || 0) * Math.PI) / 180;
	const cosYaw = Math.cos(yawRad);
	const sinYaw = Math.sin(yawRad);
	const xTick = camXTick * cosYaw + camYTick * sinYaw;
	const yTick = -camXTick * sinYaw + camYTick * cosYaw;
	return {
		camXTick: camXTick,
		camYTick: camYTick,
		xTick: xTick,
		yTick: yTick,
		xMs: xTick / RAG_TICK_MS,
		yMs: yTick / RAG_TICK_MS
	};
}

/**
 * Compute the on‑screen tilt for a raindrop so the streak aligns with its
 * projected movement direction under the current camera pitch/yaw.
 *
 * We transform the drop velocity into the same "map coords" used by the
 * SpriteRenderer Project() helper (x, -z, y), then into view space via
 * Camera.modelView. The streak axis is SpriteRenderer +Y, so the angle is
 * atan2(vx_view, vy_view).
 */
function computeDropTilt(vx, vy, speedMs) {
	// Velocity in world coords (cells/ms). Falling decreases world Z.
	const vz = -speedMs;

	// Convert to map coords expected by Camera.modelView / Project().
	const mx = vx;
	const my = -vz; // = speedMs, positive when falling down.
	const mz = vy;

	const m = Camera.modelView;
	if (!m) {
		return 0;
	}

	// View‑space velocity (w=0 ignores translation).
	const vxView = m[0] * mx + m[4] * my + m[8] * mz;
	const vyView = m[1] * mx + m[5] * my + m[9] * mz;

	if (Math.abs(vxView) < 0.000001 && Math.abs(vyView) < 0.000001) {
		return 0;
	}

	return (Math.atan2(vxView, vyView) * 180) / Math.PI;
}

class RainWeatherEffect {
	constructor(Params) {
		this.effectID = Params.Inst.effectID;
		this.ownerAID = Params.Init.ownerAID;
		this.startTick = Params.Inst.startTick;
		this.endTick = Params.Inst.endTick; // -1 for infinite

		this.lastEmitTick = this.startTick;
		this.drops = [];
		this.splashes = [];
		this.puddles = [];

		this._wind = { xTick: 0, yTick: 0, xMs: 0, yMs: 0 };

		// --- Thunderstorm ---
		this.nextLightningTime = Date.now() + randRange(THUNDER_MIN_INTERVAL, THUNDER_MAX_INTERVAL);
		this.isFlashing = false;
		this.flashStartTime = 0;
		this.flashMultiCount = 1; // Double thunders
		this.audioCtx = null;
		this.rainNode = null;
		this.audioResumed = false;

		// Try start audio feature
		try {
			const AudioContext = window.AudioContext || window.webkitAudioContext;
			if (AudioContext) {
				this.audioCtx = new AudioContext();
				this.initRainSound();
			}
		} catch (e) {
			console.warn('RainWeather: Web Audio API is not supported', e);
		}

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
		if (_instance && !_instance.needCleanUp) {
			if (_instance.endTick > 0) {
				_instance.endTick = -1;
				_instance.lastEmitTick = now;
			}
			return _instance;
		}

		_instance = new RainWeatherEffect(Params);
		_mapName = currentMap;
		return _instance;
	}

	static renderAll(gl, modelView, projection, fog, tick) {
		if (!_instance) {
			return;
		}

		// Clean up if map changed
		if (_mapName !== MapRenderer.currentMap) {
			_instance = null;
			return;
		}

		this.beforeRender(gl, modelView, projection, fog);

		SpriteRenderer.runWithDepth(false, false, true, () => {
			_instance.render(gl, tick, false);
		});

		if (_instance.needCleanUp) {
			_instance.free();
			_instance = null;
		}

		this.afterRender(gl);
	}

	static renderPuddles(gl, modelView, projection, fog, tick) {
		if (!_instance || !_instance.puddles.length) {
			return;
		}

		this.beforeRender(gl, modelView, projection, fog);
		_instance.render(gl, tick, true);
		this.afterRender(gl);
	}

	static stop(ownerAID, tick) {
		if (!_instance) {
			return;
		}

		const now = tick || Renderer.tick;
		if (_instance.endTick === -1) {
			_isStopping = true;
			_instance.endTick = now + FADEOUT_TAIL_MS;
		}
	}

	initRainSound() {
		if (!this.audioCtx) {
			return;
		}

		if (!Preferences.Sound.play) {
			return;
		}

		const ctx = this.audioCtx;
		const bufferSize = 2 * ctx.sampleRate;
		const buffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
		const output = buffer.getChannelData(0);

		// Generate white noise
		for (let i = 0; i < bufferSize; i++) {
			output[i] = Math.random() * 2 - 1;
		}

		const whiteNoise = ctx.createBufferSource();
		whiteNoise.buffer = buffer;
		whiteNoise.loop = true;

		// Low pass filter
		const rainFilter = ctx.createBiquadFilter();
		rainFilter.type = 'lowpass';
		rainFilter.frequency.value = 400;

		const gainNode = ctx.createGain();
		gainNode.gain.value = RAIN_VOLUME * Preferences.Sound.volume;

		whiteNoise.connect(rainFilter);
		rainFilter.connect(gainNode);
		gainNode.connect(ctx.destination);

		whiteNoise.start(0);
		this.rainNode = whiteNoise;
	}

	triggerRainDropSound() {
		if (!this.audioCtx) {
			return;
		}

		if (!Preferences.Sound.play) {
			return;
		}

		const ctx = this.audioCtx;

		if (ctx.state === 'suspended') {
			ctx.resume();
		}

		const osc = ctx.createOscillator();
		const gain = ctx.createGain();
		const now = ctx.currentTime;

		osc.frequency.value = randRange(1000, 1200); // Plink Frequency
		osc.type = 'sine';

		osc.connect(gain);
		gain.connect(ctx.destination);

		const duration = 0.01; // Plink duration
		const peakVolume = randRange(0.1, RAIN_VOLUME) * Preferences.Sound.volume;

		gain.gain.setValueAtTime(0, now);
		// Fast attach
		gain.gain.linearRampToValueAtTime(peakVolume, now + 0.001);
		gain.gain.exponentialRampToValueAtTime(0.001, now + duration);

		// Start and End
		osc.start(now);
		osc.stop(now + duration + 0.01);

		//clean afteruse
		setTimeout(
			() => {
				try {
					osc.disconnect();
					gain.disconnect();
				} catch (e) {
					/* Ignore */
				}
			},
			duration * 1000 + 50
		);
	}

	triggerThunderSound() {
		if (!this.audioCtx) {
			return;
		}

		if (!Preferences.Sound.play) {
			return;
		}

		// Browser can block audio interactions, try to resume it.
		if (this.audioCtx.state === 'suspended') {
			this.audioCtx.resume();
		}

		const ctx = this.audioCtx;
		const bufferSize = ctx.sampleRate * 2;
		const buffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
		const data = buffer.getChannelData(0);

		for (let i = 0; i < bufferSize; i++) {
			data[i] = (Math.random() * 2 - 1) * Math.exp(-i / bufferSize);
		}

		const source = ctx.createBufferSource();
		source.buffer = buffer;

		const filter = ctx.createBiquadFilter();
		filter.type = 'lowpass';
		filter.frequency.value = 300;

		// add gain to thunderstorm
		const thunderGain = ctx.createGain();
		thunderGain.gain.setValueAtTime(0.8 * Preferences.Sound.volume, ctx.currentTime);
		thunderGain.gain.exponentialRampToValueAtTime(0.01 * Preferences.Sound.volume, ctx.currentTime + 1.5);

		source.connect(filter);
		filter.connect(thunderGain);
		thunderGain.connect(ctx.destination);

		source.start();
	}

	spawnDrop(spawnTick) {
		if (!Session.Entity) {
			return;
		}

		const layerIndex = pickLayerIndex();
		const layer = LAYERS[layerIndex];

		const px = Session.Entity.position[0];
		const py = Session.Entity.position[1];

		const theta = Math.random() * Math.PI * 2;
		const radius = Math.random() * SCATTER_RADIUS_CELLS;
		const ox = Math.cos(theta) * radius;
		const oy = Math.sin(theta) * radius;

		// Per-drop speed / size.
		const speedTick = randRange(layer.speedTick[0], layer.speedTick[1]);
		const speedMs = speedTick / RAG_TICK_MS;
		const widthPx = randRange(layer.widthPx[0], layer.widthPx[1]);
		const lengthPx = randRange(layer.lengthPx[0], layer.lengthPx[1]);

		// Shift spawn upwind exactly enough to cancel out downwind drift during fall.
		// This keeps rain centered on the player regardless of wind strength.
		const spawnHeight = randRange(SPAWN_HEIGHT_MIN_CELLS, SPAWN_HEIGHT_MAX_CELLS);
		const driftTimeTicks = spawnHeight / speedTick;
		const upwindX = -this._wind.xTick * driftTimeTicks;
		const upwindY = -this._wind.yTick * driftTimeTicks;

		const x = px + ox + upwindX;
		const y = py + oy + upwindY;

		const groundZ = Altitude.getCellHeight(x, y);
		const z = groundZ + spawnHeight;

		// Small per-drop wind jitter around global wind.
		const windJitterXMs = (Math.random() - 0.5) * 0.002;
		const windJitterYMs = (Math.random() - 0.5) * 0.002;

		// Per-drop tilt jitter (base tilt computed each frame for camera correctness).
		const tiltJitter = randRange(-4, 4);

		this.drops.push({
			spawnTick: spawnTick,
			x: x,
			y: y,
			z: z,
			spawnZ: z,
			groundZ: groundZ,
			speedMs: speedMs,
			windJitterXMs: windJitterXMs,
			windJitterYMs: windJitterYMs,
			tiltJitter: tiltJitter,
			widthPx: widthPx,
			lengthPx: lengthPx,
			alphaScale: layer.alphaScale,
			brightness: randRange(layer.brightness[0], layer.brightness[1]),
			flicker: Math.random() * 1000
		});

		if (this.drops.length > MAX_DROPS) {
			this.drops.splice(0, this.drops.length - MAX_DROPS);
		}
	}

	updatePuddle(x, y, z, tick) {
		let bestPuddle = null;
		let bestDistSq = Infinity;

		// Find if the drop hit any existing puddle
		for (let i = 0; i < this.puddles.length; i++) {
			const p = this.puddles[i];
			const dx = p.x - x;
			const dy = p.y - y;
			const distSq = dx * dx + dy * dy;

			// Radius of impact (minimum 1.5 cells to allow growth of small puddles)
			const impactRadius = Math.max(1.5, (p.size / 35.0) * 0.8);

			if (distSq < impactRadius * impactRadius) {
				if (distSq < bestDistSq) {
					bestDistSq = distSq;
					bestPuddle = p;
				}
			}
		}

		if (bestPuddle) {
			// Grow and darken if hit directly
			bestPuddle.size = Math.min(PUDDLE_MAX_SIZE, bestPuddle.size + PUDDLE_GROWTH_PER_DROP);
			bestPuddle.alpha = Math.min(PUDDLE_ALPHA_MAX, bestPuddle.alpha + 0.02);

			// Accumulate time: each drop adds 1.5s of life, capped at 10s max into the future
			const currentLife = Math.max(0, (bestPuddle.nextDryTick || tick) - tick);
			const newLife = Math.min(10000, currentLife + 1500);
			bestPuddle.nextDryTick = tick + newLife;

			bestPuddle.lastUpdate = tick;
		} else if (Math.random() < PUDDLE_SEED_CHANCE) {
			// Avoid seeding too close to ANY existing puddle (prevent stacking/darkening)
			let tooClose = false;
			for (let i = 0; i < this.puddles.length; i++) {
				const p = this.puddles[i];
				const dx = p.x - x;
				const dy = p.y - y;
				if (dx * dx + dy * dy < PUDDLE_MAX_SIZE / 4) {
					// 12 cells avoidance radius
					tooClose = true;
					// Optionally give a small boost to the nearby puddle instead
					p.size = Math.min(PUDDLE_MAX_SIZE, p.size + PUDDLE_GROWTH_PER_DROP * 0.5);
					break;
				}
			}

			if (!tooClose) {
				// Seed a new puddle spot
				if (this.puddles.length >= MAX_PUDDLES) {
					// Strategy: Instead of removing the oldest, remove the one with lowest visibility (smallest/faintest)
					let worstIdx = 0;
					let minScore = Infinity;
					for (let i = 0; i < this.puddles.length; i++) {
						const score = this.puddles[i].size * this.puddles[i].alpha;
						if (score < minScore) {
							minScore = score;
							worstIdx = i;
						}
					}
					this.puddles.splice(worstIdx, 1);
				}
				this.puddles.push({
					x: x,
					y: y,
					z: z,
					size: PUDDLE_INITIAL_SIZE,
					alpha: 0.12,
					angle: Math.random() * Math.PI * 2,
					lastUpdate: tick,
					nextDryTick: tick + 3500 // Initial life
				});
			}
		}
	}

	render(gl, tick, puddlesOnly = false) {
		if (!Session.Entity) {
			return;
		}

		ensureDropFrame(gl);
		ensureFilterFrame(gl);
		ensureSplashFrame(gl);
		ensurePuddleFrame(gl);
		initPuddleShader(gl);

		if (puddlesOnly) {
			this.renderPuddles(gl, tick);
			return;
		}

		if (!_dropFrame) {
			return;
		}

		// Update global wind for this frame.
		this._wind = computeGlobalWind(tick);

		// --- ThunderStorm system (Visual and Sound) ---
		const now = Date.now();

		// check if need to flash on thunderstorm
		if (!this.isFlashing && now >= this.nextLightningTime) {
			this.triggerThunderSound();
			this.isFlashing = true;
			this.flashStartTime = now;
			// 40% chance of double storm
			this.flashMultiCount = Math.random() > 0.6 ? 2 : 1;
			this.nextLightningTime = now + randRange(THUNDER_MIN_INTERVAL, THUNDER_MAX_INTERVAL);
		}

		// common grey filter
		let overlayR = 0.65;
		let overlayG = 0.67;
		let overlayB = 0.7;
		let overlayA = 0.14;

		// flash filter
		if (this.isFlashing) {
			const elapsed = now - this.flashStartTime;
			let flashAlpha = 0;

			if (this.flashMultiCount === 2) {
				// --- Double Flash (Strobe) ---
				// 0-80ms: First flash is faster
				if (elapsed < 80) {
					flashAlpha = 0.6 * (1 - elapsed / 80);
				}
				// 80-160ms: darker (dramatic pause)
				else if (elapsed < 160) {
					flashAlpha = 0;
				}
				// 160ms+: Another Flash (Main)
				else {
					const elapsed2 = elapsed - 160;
					if (elapsed2 < FLASH_FADE_IN) {
						flashAlpha = (elapsed2 / FLASH_FADE_IN) * 0.8;
					} else if (elapsed2 < FLASH_FADE_IN + FLASH_FADE_OUT) {
						const p = (elapsed2 - FLASH_FADE_IN) / FLASH_FADE_OUT;
						flashAlpha = 0.8 * (1 - p);
					} else {
						this.isFlashing = false;
					}
				}
			} else {
				// --- FLASH (Default) ---
				if (elapsed < FLASH_FADE_IN) {
					// Fade In
					flashAlpha = (elapsed / FLASH_FADE_IN) * 0.8;
				} else if (elapsed < FLASH_FADE_IN + FLASH_FADE_OUT) {
					// Fade Out
					const p = (elapsed - FLASH_FADE_IN) / FLASH_FADE_OUT;
					flashAlpha = 0.8 * (1 - p);
				} else {
					// end
					this.isFlashing = false;
				}
			}

			if (this.isFlashing) {
				// Thunder collor white blue
				overlayR = 0.9;
				overlayG = 0.95;
				overlayB = 1.0;
				overlayA = flashAlpha;
			}
		}

		// Renderize filter (Thunderstorm)
		if (_filterFrame) {
			// skip rain fade out to visual storm
			if (this.endTick > 0 && !this.isFlashing) {
				const tail = Math.max(0, Math.min(1, (this.endTick - tick) / FADEOUT_TAIL_MS));
				overlayA *= tail;
			}

			SpriteRenderer.image.palette = null;
			SpriteRenderer.sprite = _filterFrame;
			SpriteRenderer.image.texture = _filterFrame.texture;
			SpriteRenderer.position[0] = Session.Entity.position[0];
			SpriteRenderer.position[1] = Session.Entity.position[1];
			SpriteRenderer.position[2] = Session.Entity.position[2];
			SpriteRenderer.zIndex = -1000;

			SpriteRenderer.color[0] = overlayR;
			SpriteRenderer.color[1] = overlayG;
			SpriteRenderer.color[2] = overlayB;
			SpriteRenderer.color[3] = overlayA;

			SpriteRenderer.angle = 0;
			SpriteRenderer.size[0] = 6000;
			SpriteRenderer.size[1] = 6000;
			SpriteRenderer.offset[0] = 0;
			SpriteRenderer.offset[1] = 0;
			SpriteRenderer.render(false);
		}

		// Effect lifetime / emission gating.
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
			const ticksToEmit = Math.floor((tick - this.lastEmitTick) / RAG_TICK_MS);
			if (ticksToEmit > 0) {
				for (let i = 0; i < ticksToEmit; i++) {
					const emitTick = this.lastEmitTick + i * RAG_TICK_MS;
					if (_isStopping) {
						break;
					}
					for (let e = 0; e < EMIT_PER_TICK; e++) {
						this.spawnDrop(emitTick);
					}
				}
				this.lastEmitTick += ticksToEmit * RAG_TICK_MS;
			}
		}

		for (let d = this.drops.length - 1; d >= 0; d--) {
			const drop = this.drops[d];

			// Ended or hit ground.
			if (drop.z <= drop.groundZ + 0.05) {
				// Spawn a small splash on impact (biased toward nearer layers).
				if (_splashFrame && Math.random() < drop.alphaScale * 0.6) {
					this.splashes.push({
						x: drop.x,
						y: drop.y,
						z: drop.groundZ + 0.02,
						startTick: tick,
						lifeMs: SPLASH_LIFE_MS,
						maxSizePx: randRange(SPLASH_SIZE_PX[0], SPLASH_SIZE_PX[1]),
						angle: drop.angle * 0.2
					});
					if (this.splashes.length > 300) {
						this.splashes.splice(0, this.splashes.length - 300);
					}
					this.triggerRainDropSound(); // call water sound
				}

				// Update/Spawn puddle on impact
				this.updatePuddle(drop.x, drop.y, drop.groundZ, tick);

				this.drops.splice(d, 1);
				continue;
			}

			const dt = tick - (drop._lastTick || drop.spawnTick);
			drop._lastTick = tick;

			// Update fall + coherent wind drift. Drift is recomputed from the current
			// global wind so on-screen slant stays stable under camera rotation.
			const windXMs = this._wind.xMs + (drop.windJitterXMs || 0);
			const windYMs = this._wind.yMs + (drop.windJitterYMs || 0);

			drop.z -= drop.speedMs * dt;
			drop.x += windXMs * dt;
			drop.y += windYMs * dt;

			// Progress through fall (0..1).
			const denom = drop.spawnZ - drop.groundZ;
			let progress = denom > 0.0001 ? (drop.spawnZ - drop.z) / denom : 0;
			if (progress < 0) {
				progress = 0;
			}
			if (progress > 1) {
				progress = 1;
			}

			// Fade by progress to avoid popping.
			let alpha = drop.alphaScale;
			if (progress < 0.08) {
				alpha *= progress / 0.08;
			} else if (progress > 0.85) {
				alpha *= (1 - progress) / 0.15;
			}

			// Gentle shimmer for life.
			alpha *= 0.9 + 0.1 * Math.sin((tick + drop.flicker) * 0.015);

			SpriteRenderer.position[0] = drop.x;
			SpriteRenderer.position[1] = drop.y;
			SpriteRenderer.position[2] = drop.z;
			SpriteRenderer.zIndex = 0;

			SpriteRenderer.image.palette = null;
			SpriteRenderer.sprite = _dropFrame;
			SpriteRenderer.image.texture = _dropFrame.texture;
			SpriteRenderer.color[0] = 0.6 * drop.brightness;
			SpriteRenderer.color[1] = 0.72 * drop.brightness;
			SpriteRenderer.color[2] = 1.0 * drop.brightness;
			SpriteRenderer.color[3] = alpha;
			SpriteRenderer.angle = computeDropTilt(windXMs, windYMs, drop.speedMs) + (drop.tiltJitter || 0);
			SpriteRenderer.size[0] = drop.widthPx;
			SpriteRenderer.size[1] = drop.lengthPx;
			SpriteRenderer.offset[0] = 0;
			SpriteRenderer.offset[1] = 0;

			SpriteRenderer.render(false);
		}

		// Render splashes after drops so they sit on top of the ground.
		if (_splashFrame && this.splashes.length) {
			for (let s = this.splashes.length - 1; s >= 0; s--) {
				const splash = this.splashes[s];
				const splashAge = tick - splash.startTick;
				if (splashAge >= splash.lifeMs) {
					this.splashes.splice(s, 1);
					continue;
				}

				const sp = splashAge / splash.lifeMs;
				const splashAlpha = SPLASH_ALPHA * (1 - sp);
				const splashSize = splash.maxSizePx * (0.6 + sp * 0.8);

				SpriteRenderer.image.palette = null;
				SpriteRenderer.sprite = _splashFrame;
				SpriteRenderer.image.texture = _splashFrame.texture;
				SpriteRenderer.position[0] = splash.x;
				SpriteRenderer.position[1] = splash.y;
				SpriteRenderer.position[2] = splash.z;
				SpriteRenderer.zIndex = 0;
				SpriteRenderer.color[0] = 0.75;
				SpriteRenderer.color[1] = 0.8;
				SpriteRenderer.color[2] = 0.9;
				SpriteRenderer.color[3] = splashAlpha;
				SpriteRenderer.angle = splash.angle;
				SpriteRenderer.size[0] = splashSize;
				SpriteRenderer.size[1] = splashSize;
				SpriteRenderer.offset[0] = 0;
				SpriteRenderer.offset[1] = 0;
				SpriteRenderer.render(false);
			}
		}
	}

	renderPuddles(gl, tick) {
		if (_puddleFrame && this.puddles.length) {
			const self = this;
			SpriteRenderer.runWithDepth(true, false, false, () => {
				const uniform = _puddleProgram.uniform;
				const attribute = _puddleProgram.attribute;

				gl.useProgram(_puddleProgram);
				gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

				// Prepare common uniforms
				gl.uniformMatrix4fv(uniform.uModelViewMat, false, Camera.modelView);
				gl.uniformMatrix4fv(uniform.uProjectionMat, false, Camera.projection);

				// Fog
				const fog = MapRenderer.fog;
				gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
				gl.uniform1f(uniform.uFogNear, fog.near);
				gl.uniform1f(uniform.uFogFar, fog.far);
				gl.uniform3fv(uniform.uFogColor, fog.color);

				gl.activeTexture(gl.TEXTURE0);
				gl.bindTexture(gl.TEXTURE_2D, _puddleFrame.texture);
				gl.uniform1i(uniform.uDiffuse, 0);

				// Pass time for procedural ripples
				gl.uniform1f(uniform.uTime, tick * 0.001);

				// Pass camera info for Fresnel reflections (Platinum look)
				gl.uniform3fv(uniform.uCameraPos, Camera.position);

				gl.bindBuffer(gl.ARRAY_BUFFER, _puddleBuffer);
				gl.enableVertexAttribArray(attribute.aPosition);
				gl.enableVertexAttribArray(attribute.aTextureCoord);
				gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 5 * 4, 0);
				gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 5 * 4, 3 * 4);

				for (let p = self.puddles.length - 1; p >= 0; p--) {
					const puddle = self.puddles[p];
					// Evaporate slowly if not hit by rain for a while
					// Each drop adds 1.5s of life up to a hard cap of 10s.
					if (tick > (puddle.nextDryTick || tick)) {
						const dt = tick - (puddle._lastTick || tick);
						// Larger puddles dry out slower but start shrinking sooner
						const evaporationRate = PUDDLE_FADE_SPEED / (1.0 + puddle.size * 0.003);
						const fade = evaporationRate * dt;

						puddle.alpha -= fade;
						// Shrink all the way to nothing (fadeout + encolher)
						puddle.size -= fade * 8500.0;
					}
					puddle._lastTick = tick;

					// Remove only when practically invisible or shrunk to nothing
					if (puddle.size <= 0.1 || puddle.alpha <= 0.0001) {
						self.puddles.splice(p, 1);
						continue;
					}

					// Uniforms for this puddle
					gl.uniform3f(uniform.uWorldPosition, puddle.x, puddle.y, puddle.z + 0.01);
					gl.uniform2f(uniform.uSize, puddle.size, puddle.size);
					gl.uniform1f(uniform.uAngle, puddle.angle);
					gl.uniform4f(uniform.uColor, 1.0, 1.0, 1.0, puddle.alpha);
					gl.uniform1f(uniform.uZIndex, 1);

					gl.drawArrays(gl.TRIANGLES, 0, 6);
				}

				gl.disableVertexAttribArray(attribute.aPosition);
				gl.disableVertexAttribArray(attribute.aTextureCoord);

				// Restore blend mode for subsequent rendering
				gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
			});
		}
	}

	free() {
		// Clean thunder audio
		if (this.audioCtx) {
			try {
				if (this.rainNode) {
					this.rainNode.stop();
					this.rainNode = null;
				}
				this.audioCtx.close();
				this.audioCtx = null;
			} catch (e) {
				console.log(e);
			}
		}
		this.drops = [];
		this.splashes = [];
		this.puddles = [];
		this.ready = false;
	}
}

RainWeatherEffect.renderBeforeEntities = false;

export default RainWeatherEffect;
