/**
 * Renderer/Effects/PokJukWeatherEffect.js
 *
 * Fireworks Effect (PokJuk) adapted from C++ CEffectPrim::PrimPokJuk.
 * Logic:
 * 1. Ascent Phase: The projectile rises with negative gravity.
 * 2. Explosion Phase: Upon reaching the time limit, the particles expand 360 degrees.
 * 3. Tail: Maintains previous positions to draw the ascent trail.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import MapRenderer from 'Renderer/MapRenderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Session from 'Engine/SessionStorage.js';
import Sound from 'Audio/SoundManager.js';

// SINGLETON STATE
let _instance = null;
let _mapName = '';
let _whiteTexture = null;

// Explosion height in game units
const EXPLOSION_ALTITUDE = 8.0;
const PARTICLE_SIZE = 6;

const FIRE_LIFE_MS = 50;
const EXPLOSION_LIFE_MS = 1000;

class PokJukWeatherEffect {
	constructor(Params) {
		this.fireworks = [];
		this.init(Params);
	}

	static isActive() {
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

		// Ensure the internal texture exists before drawing
		_instance.createInternalTexture(gl);

		gl.blendFunc(gl.SRC_ALPHA, gl.ONE); // Additive blending

		SpriteRenderer.runWithDepth(false, false, true, function () {
			_instance.render(gl, tick);
		});

		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}

	static startOrRestart(Params) {
		const currentMap = MapRenderer.currentMap;

		if (_mapName !== currentMap || !_instance) {
			_instance = new PokJukWeatherEffect(Params);
			_mapName = currentMap;
		}

		return _instance;
	}

	createInternalTexture(gl) {
		if (_whiteTexture) {
			return;
		}

		const canvas = document.createElement('canvas');
		canvas.width = PARTICLE_SIZE;
		canvas.height = PARTICLE_SIZE;
		const ctx = canvas.getContext('2d');

		const center = PARTICLE_SIZE / 2;
		const radius = center - 1;

		ctx.clearRect(0, 0, PARTICLE_SIZE, PARTICLE_SIZE);

		const gradient = ctx.createRadialGradient(center, center, 0, center, center, radius);
		gradient.addColorStop(0, 'rgba(255, 255, 255, 1)');
		gradient.addColorStop(0.7, 'rgba(255, 255, 255, 0.8)');
		gradient.addColorStop(1, 'rgba(255, 255, 255, 0)');

		ctx.beginPath();
		ctx.arc(center, center, radius, 0, Math.PI * 2);
		ctx.fillStyle = gradient;
		ctx.fill();

		_whiteTexture = gl.createTexture();
		gl.bindTexture(gl.TEXTURE_2D, _whiteTexture);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);

		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	}

	init(Params) {
		if (!Session.Entity) {
			return;
		}
		const pPos = Session.Entity.position;

		// Spawn random number of fireworks (1 or 3)
		const numFireworks = Math.floor(Math.random() * 3) + 1;
		for (let ec = 0; ec < numFireworks; ec++) {
			const firework = this.createFirework(pPos);
			firework.process -= ec * 100;
			this.fireworks.push(firework);
		}
	}

	createFirework() {
		const pPos = Session.Entity.position;
		return {
			process: -(600 + Math.floor(Math.random() * 400)),
			// Start near player
			posNow: [pPos[0] + (Math.random() * 20 - 10), pPos[1] + (Math.random() * 20 - 10), pPos[2]],
			horizontalDrift: [Math.random() * 0.02 - 0.01, Math.random() * 0.02 - 0.01],
			startAlt: pPos[2],
			alpha: 0,
			colorType: Math.floor(Math.random() * 5),
			state: 0, // 0: Ascent, 1: Explosion
			particles: [],
			size: PARTICLE_SIZE,
			arcDirection: Math.random() > 0.5 ? 1 : -1, // 1 = right, -1 = left
			arcAmplitude: 3 + Math.random() * 2,
			arcPhase: 0,
			explosionTimer: 0,
			lastWaveTime: 0,
			isExploding: false
		};
	}

	render(gl, tick) {
		for (let i = 0; i < this.fireworks.length; i++) {
			const fw = this.fireworks[i];
			this.updateFirework(fw);
			this.drawFirework(fw);
		}
	}

	updateFirework(fw) {
		fw.process++;

		// Launch sound
		if (fw.process === 1) {
			Sound.play('effect/\xc6\xf8\xc1\xd7.wav');
		}

		if (fw.process > 0) {
			if (fw.state === 0) {
				// ASCENT PHASE
				fw.posNow[2] += 0.04;
				const ascentProgress = Math.max(0, Math.min(1, (fw.posNow[2] - fw.startAlt) / EXPLOSION_ALTITUDE));
				const arcProgress = Math.pow(Math.max(0, (ascentProgress - 0.3) / 0.7), 2);
				fw.posNow[0] += arcProgress * fw.arcAmplitude * fw.arcDirection * 0.02;
				fw.alpha = Math.floor(Math.min(ascentProgress * 255 * 2, 255));

				// Explode based on altitude relative to start position
				if (fw.posNow[2] - fw.startAlt > EXPLOSION_ALTITUDE) {
					fw.state = 1;
					fw.isExploding = true;
					fw.explosionTimer = 0;
					fw.lastWaveTime = -50;
					// Generate burst particles
					for (let p = 0; p < 30; p++) {
						this.addParticleWave(fw, true);
					}
				}
			} else if (fw.state === 1) {
				// EXPLOSION PHASE
				fw.explosionTimer += 16;
				if (fw.explosionTimer < EXPLOSION_LIFE_MS) {
					if (fw.explosionTimer - fw.lastWaveTime >= FIRE_LIFE_MS) {
						const colorType = Math.floor(Math.random() * 5);
						for (let i = 0; i < 15; i++) {
							this.addParticleWave(fw, false, colorType);
						}
						for (let i = 0; i < 60; i++) {
							this.addParticleWave(fw, true, colorType);
						}
						fw.lastWaveTime = fw.explosionTimer;
					}
				}
				let active = 0;
				for (let j = 0; j < fw.particles.length; j++) {
					const p = fw.particles[j];
					if (p.alpha <= 0) {
						continue;
					}

					p.pos[0] += p.vel[0];
					p.pos[1] += p.vel[1];
					p.pos[2] += p.vel[2];
					p.vel[2] -= 0.003; // Light gravity
					p.alpha -= p.fadeSpeed || 2; // Fade speed
					active++;
				}

				if (active === 0 && fw.explosionTimer > EXPLOSION_LIFE_MS) {
					Object.assign(fw, this.createFirework());
				}
			}
		}
	}

	addParticleWave(fw, isGunpowder, colorType) {
		const phi = Math.random() * Math.PI * 2;
		const theta = Math.random() * Math.PI;
		const speed = isGunpowder ? 0.02 + Math.random() * 0.04 : 0.08 + Math.random() * 0.07;

		const zOffset = isGunpowder ? 0.5 : 0;

		fw.particles.push({
			pos: [fw.posNow[0], fw.posNow[1], fw.posNow[2] + zOffset],
			vel: [
				Math.sin(theta) * Math.cos(phi) * speed,
				Math.sin(theta) * Math.sin(phi) * speed,
				Math.cos(theta) * speed
			],
			alpha: 255,
			size: isGunpowder ? fw.size * 0.4 : fw.size * (0.6 + Math.random() * 0.6),
			colorType: isGunpowder ? -1 : colorType,
			fadeSpeed: isGunpowder ? 6 : 3
		});
	}

	drawFirework(fw) {
		if (fw.process <= 0) {
			return;
		}

		if (fw.state === 0) {
			this.renderParticle(fw.posNow, fw.alpha, 255, 255, 255, fw.size);
		} else {
			for (let i = 0; i < fw.particles.length; i++) {
				const p = fw.particles[i];
				if (p.alpha > 0) {
					let r, g, b;
					switch (p.colorType) {
						case 0:
							r = 100;
							g = 160;
							b = 255;
							break; // Blue
						case 1:
							r = 255;
							g = 100;
							b = 100;
							break; // Red
						case 2:
							r = 100;
							g = 255;
							b = 100;
							break; // Green
						case 3:
							r = 255;
							g = 255;
							b = 130;
							break; // Yellow
						case 4:
							r = 255;
							g = 130;
							b = 255;
							break; // Pink
						default:
							r = 255;
							g = 255;
							b = 255;
							break; // White (Gunpowder)
					}
					this.renderParticle(p.pos, p.alpha, r, g, b, fw.size);
				}
			}
		}
	}

	renderParticle(pos, alpha, r, g, b, size) {
		SpriteRenderer.position[0] = pos[0];
		SpriteRenderer.position[1] = pos[1];
		SpriteRenderer.position[2] = pos[2];

		SpriteRenderer.color.set([r / 255, g / 255, b / 255, alpha / 255]);
		SpriteRenderer.size[0] = size;
		SpriteRenderer.size[1] = size;

		// Use the procedurally generated texture
		SpriteRenderer.image.texture = _whiteTexture;
		SpriteRenderer.render(false);
	}

	static stop() {
		_instance = null;
		_mapName = '';
	}
}
export default PokJukWeatherEffect;
