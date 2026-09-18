/**
 * Renderer/Effects/WaterfallEffect.js
 *
 * Rendering Waterfall effect
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Alison Serafim
 */

import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import _vertexShader from './WaterfallEffect.vs?raw';
import _fragmentShader from './WaterfallEffect.fs?raw';
import _particleVertexShader from './WaterfallParticle.vs?raw';
import _particleFragmentShader from './WaterfallParticle.fs?raw';

const mat4 = glMatrix.mat4;
const _matrix = mat4.create();

// Client units are 1/5 of a world unit.
const UNIT = 1 / 5;
const LAYER_COUNT = 4;
const SEGMENT_COUNT = 5;
const TEXTURE_COUNT = 3;
const SEGMENT_HEIGHT = 40 * UNIT;
const EFFECT_TICK_MS = 24;
const OPACITY = 80 / 255;

// Spray puffs cycle through 38 client units at 0.05 units per tick.
const PARTICLE_TEXTURE = 'data/texture/effect/freeze_a_small.bmp';
const PARTICLE_CYCLE_MS = (38 / 0.05) * EFFECT_TICK_MS;
const PARTICLE_SIZE = 6 * Math.SQRT1_2 * UNIT;
const PARTICLE_COLOR = [0.65, 1.0, 0.75];
const PARTICLE_FLOATS = 4;
const PARTICLE_CORNERS = new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1]);

let _program;
let _particleProgram;
const _textureCache = new Map();
const _vertices = new Float32Array(20);

function textureFiles(textureSet) {
	const files = [];
	for (let index = 1; index <= TEXTURE_COUNT; index++) {
		files.push(`data/texture/effect/waterfall${textureSet}${index}.tga`);
	}
	files.push(PARTICLE_TEXTURE);
	return files;
}

function loadTextures(gl, textureSet, effect) {
	let cache = _textureCache.get(textureSet);
	if (!cache) {
		const files = textureFiles(textureSet);
		cache = {
			textures: new Array(files.length),
			waiters: new Set(),
			ready: false,
			active: true
		};
		_textureCache.set(textureSet, cache);

		files.forEach((file, index) => {
			Client.loadFile(file, buffer => {
				WebGL.texture(gl, buffer, texture => {
					if (!cache.active) {
						gl.deleteTexture(texture);
						return;
					}

					cache.textures[index] = texture;
					if (cache.textures.filter(Boolean).length === files.length) {
						cache.ready = true;
						cache.waiters.forEach(waiter => {
							waiter.textures = cache.textures;
							waiter.ready = true;
						});
						cache.waiters.clear();
					}
				});
			});
		});
	}

	if (cache.ready) {
		effect.textures = cache.textures;
		effect.ready = true;
	} else {
		cache.waiters.add(effect);
	}

	return cache;
}

function getStyle(variant) {
	const small = variant.includes('small');
	const dark = variant.includes('dark');
	return {
		width: (small ? 18 : 36) * UNIT,
		particleCount: small ? 320 : 640,
		particleSpread: (small ? 12 : 22) * UNIT,
		textureSet: dark ? 3 : 1
	};
}

function buildParticleSeeds(count, spread) {
	const seeds = new Float32Array(count * PARTICLE_FLOATS);
	for (let i = 0; i < count; i++) {
		seeds[i * PARTICLE_FLOATS] = (Math.random() * 2 - 1) * spread;
		seeds[i * PARTICLE_FLOATS + 1] = (Math.random() * 2 - 1) * 5 * UNIT;
		seeds[i * PARTICLE_FLOATS + 2] = Math.random();
		seeds[i * PARTICLE_FLOATS + 3] = Math.random() * Math.PI * 2;
	}
	return seeds;
}

function setVertex(index, x, y, z, u, v) {
	const offset = index * 5;
	_vertices[offset] = x;
	_vertices[offset + 1] = y;
	_vertices[offset + 2] = z;
	_vertices[offset + 3] = u;
	_vertices[offset + 4] = v;
}

/**
 * WaterfallEffect constructor
 *
 * @param {object} effect parameters
 * @param {object} EF_Inst_Par parameters
 * @param {object} EF_Init_Par parameters
 */
class WaterfallEffect {
	constructor(effect, instance, init) {
		const style = getStyle(effect.variant);
		this.position = instance.position;
		this.startTick = instance.startTick;
		this.width = style.width;
		this.particleCount = style.particleCount;
		this.particleSpread = style.particleSpread;
		this.textureSet = style.textureSet;
		this.textures = [];
		this.textureCache = null;
		this.buffer = null;
		this.cornerBuffer = null;
		this.seedBuffer = null;
		this.vertical = effect.vertical;
		this.ready = false;
		this.needInit = true;
	}

	/**
	 * Initialize WebGL resources
	 *
	 * @param {WebGLRenderingContext} gl
	 */
	init(gl) {
		this.buffer = gl.createBuffer();
		this.cornerBuffer = gl.createBuffer();
		this.seedBuffer = gl.createBuffer();
		gl.bindBuffer(gl.ARRAY_BUFFER, this.cornerBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, PARTICLE_CORNERS, gl.STATIC_DRAW);
		gl.bindBuffer(gl.ARRAY_BUFFER, this.seedBuffer);
		gl.bufferData(gl.ARRAY_BUFFER, buildParticleSeeds(this.particleCount, this.particleSpread), gl.STATIC_DRAW);
		this.textureCache = loadTextures(gl, this.textureSet, this);
	}

	render(gl, tick) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;
		mat4.identity(_matrix);
		// Map effect positions carry the sprite offsets applied by MapRenderer (-0.5 x/z, +1 altitude);
		// revert them so the mesh sits exactly on the RSW position like models do.
		mat4.translate(_matrix, _matrix, [this.position[0] + 0.5, 1 - this.position[2], this.position[1] + 0.5]);
		if (this.vertical) {
			mat4.rotateY(_matrix, _matrix, Math.PI / 2);
		}

		gl.uniformMatrix4fv(uniform.uModelMat, false, _matrix);
		const elapsed = tick - this.startTick;
		const process = Math.floor(elapsed / EFFECT_TICK_MS);

		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
		gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 20, 0);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 20, 12);

		for (let layer = 0; layer < LAYER_COUNT; layer++) {
			const speed = 80 - layer * 13;
			const scroll = ((process % speed) * SEGMENT_HEIGHT) / speed;
			const crop = scroll / SEGMENT_HEIGHT;
			const phase = Math.floor((process % (TEXTURE_COUNT * speed)) / speed);
			const halfWidth = (this.width + layer * UNIT) / 2;
			const depth = (layer - 1) * UNIT;

			for (let segment = 0; segment < SEGMENT_COUNT; segment++) {
				const top = scroll - segment * SEGMENT_HEIGHT;
				const bottom = top - SEGMENT_HEIGHT;
				let visibleTop = top;
				let visibleBottom = bottom;
				let vTop = 0;
				let vBottom = 1;

				if (segment === 0) {
					visibleTop = top + (bottom - top) * crop;
					vTop = crop;
				} else if (segment === SEGMENT_COUNT - 1) {
					visibleBottom = top + (bottom - top) * crop;
					vBottom = crop;
				}

				setVertex(0, -halfWidth, -visibleBottom, depth, 0, vBottom);
				setVertex(1, halfWidth, -visibleBottom, depth, 1, vBottom);
				setVertex(2, -halfWidth, -visibleTop, depth, 0, vTop);
				setVertex(3, halfWidth, -visibleTop, depth, 1, vTop);
				gl.bufferData(gl.ARRAY_BUFFER, _vertices, gl.DYNAMIC_DRAW);
				gl.bindTexture(gl.TEXTURE_2D, this.textures[(segment + phase) % TEXTURE_COUNT]);
				gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
			}
		}

		this.renderParticles(gl, elapsed);
	}

	/**
	 * Additive spray puffs rising from the pool at the base of the fall,
	 * drawn as camera-facing instanced quads.
	 */
	renderParticles(gl, elapsed) {
		const uniform = _particleProgram.uniform;
		const attribute = _particleProgram.attribute;

		gl.useProgram(_particleProgram);
		gl.uniformMatrix4fv(uniform.uModelMat, false, _matrix);
		gl.uniform1f(uniform.uTime, elapsed / PARTICLE_CYCLE_MS);
		gl.uniform1f(uniform.uSize, PARTICLE_SIZE);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE);

		gl.enableVertexAttribArray(attribute.aCorner);
		gl.enableVertexAttribArray(attribute.aSeed);
		gl.bindBuffer(gl.ARRAY_BUFFER, this.cornerBuffer);
		gl.vertexAttribPointer(attribute.aCorner, 2, gl.FLOAT, false, 0, 0);
		gl.bindBuffer(gl.ARRAY_BUFFER, this.seedBuffer);
		gl.vertexAttribPointer(attribute.aSeed, PARTICLE_FLOATS, gl.FLOAT, false, 0, 0);
		gl.vertexAttribDivisor(attribute.aSeed, 1);

		gl.bindTexture(gl.TEXTURE_2D, this.textures[TEXTURE_COUNT]);
		gl.drawArraysInstanced(gl.TRIANGLE_STRIP, 0, 4, this.particleCount);

		gl.vertexAttribDivisor(attribute.aSeed, 0);
		gl.disableVertexAttribArray(attribute.aCorner);
		gl.disableVertexAttribArray(attribute.aSeed);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		gl.useProgram(_program);
		gl.enableVertexAttribArray(_program.attribute.aPosition);
		gl.enableVertexAttribArray(_program.attribute.aTextureCoord);
	}

	/**
	 * Free WebGL resources
	 *
	 * @param {WebGLRenderingContext} gl
	 */
	free(gl) {
		if (this.textureCache) {
			this.textureCache.waiters.delete(this);
			this.textureCache = null;
		}
		[this.buffer, this.cornerBuffer, this.seedBuffer].forEach(buffer => {
			if (buffer) {
				gl.deleteBuffer(buffer);
			}
		});
		this.buffer = null;
		this.cornerBuffer = null;
		this.seedBuffer = null;
		this.textures = [];
		this.ready = false;
	}

	/**
	 * Called before rendering all effects of this type
	 *
	 * @param {WebGLRenderingContext} gl
	 * @param {mat4} modelView
	 * @param {mat4} projection
	 * @param {object} fog
	 * @param {number} tick
	 * @param {object} entity
	 */
	static beforeRender(gl, modelView, projection, fog) {
		const fogUse = fog.use && fog.exist;

		gl.useProgram(_particleProgram);
		gl.uniformMatrix4fv(_particleProgram.uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(_particleProgram.uniform.uProjectionMat, false, projection);
		gl.uniform1i(_particleProgram.uniform.uFogUse, fogUse);
		gl.uniform1f(_particleProgram.uniform.uFogNear, fog.near);
		gl.uniform1f(_particleProgram.uniform.uFogFar, fog.far);
		gl.uniform1i(_particleProgram.uniform.uTexture, 0);
		gl.uniform3fv(_particleProgram.uniform.uColor, PARTICLE_COLOR);

		const uniform = _program.uniform;
		const attribute = _program.attribute;
		gl.useProgram(_program);
		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);
		gl.uniform1i(uniform.uFogUse, fogUse);
		gl.uniform1f(uniform.uFogNear, fog.near);
		gl.uniform1f(uniform.uFogFar, fog.far);
		gl.uniform3fv(uniform.uFogColor, fog.color);
		gl.uniform1f(uniform.uOpacity, OPACITY);
		gl.uniform1i(uniform.uTexture, 0);
		gl.enable(gl.DEPTH_TEST);
		gl.enable(gl.BLEND);
		gl.depthMask(false);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);
	}

	/**
	 * Called after rendering all effects of this type
	 *
	 * @param {WebGLRenderingContext} gl
	 */
	static afterRender(gl) {
		gl.depthMask(true);
		gl.disableVertexAttribArray(_program.attribute.aPosition);
		gl.disableVertexAttribArray(_program.attribute.aTextureCoord);
	}

	/**
	 * Initialize the effect type
	 *
	 * @param {WebGLRenderingContext} gl
	 */
	static init(gl) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
		_particleProgram = WebGL.createShaderProgram(gl, _particleVertexShader, _particleFragmentShader);
		this.ready = true;
	}

	/**
	 * Free resources for this effect type
	 *
	 * @param {WebGLRenderingContext} gl
	 */
	static free(gl) {
		_textureCache.forEach(cache => {
			cache.active = false;
			cache.textures.forEach(texture => {
				if (texture) {
					gl.deleteTexture(texture);
				}
			});
			cache.waiters.clear();
		});
		_textureCache.clear();

		if (_program) {
			gl.deleteProgram(_program);
		}
		if (_particleProgram) {
			gl.deleteProgram(_particleProgram);
		}
		_program = null;
		_particleProgram = null;
		this.ready = false;
		this.needInit = true;
	}
}
WaterfallEffect.renderBeforeEntities = false;
WaterfallEffect.needInit = true;

export default WaterfallEffect;
