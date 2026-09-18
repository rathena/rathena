/**
 * Renderer/Effects/StrEffect.js
 *
 * Rendering Str File object
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import _vertexShader from './StrEffect.vs?raw';
import _fragmentShader from './StrEffect.fs?raw';
import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';

const mat4 = glMatrix.mat4;

/**
 * Look up table D3DX => OPENGL
 */
const D3DBLEND = {};

/**
 * @var {WebGLProgram}
 */
let _program = null;

/**
 * @var {WebGLBuffer}
 */
let _buffer = null;

/**
 * @var {float[16]} buffer
 */
const _bufferData = new Float32Array(4 * 4);

/**
 * @var mat4 matrix to generate rotation
 */
const _matrix = mat4.create();

/**
 * @var {number} last angle
 */
let _lastAngle = -1;

// Pixel to world conversion for attachment offsets
const PIXEL_TO_WORLD_Z = 1.0 / 5.0;

const anim = {
	frame: 0,
	type: 0,
	aniframe: 0,
	anitype: 0,
	srcalpha: 1,
	destalpha: 1,
	mtpreset: 0,
	delay: 0.0,
	angle: 0.0,
	color: new Float32Array(4),
	pos: new Float32Array(2),
	uv: new Float32Array(8),
	xy: new Float32Array(8)
};

// Helper to copy animation state for caching
function copyAnim(src) {
	return {
		type: src.type,
		aniframe: src.aniframe,
		anitype: src.anitype,
		srcalpha: src.srcalpha,
		destalpha: src.destalpha,
		mtpreset: src.mtpreset,
		delay: src.delay,
		angle: src.angle,
		color: new Float32Array(src.color),
		pos: new Float32Array(src.pos),
		uv: new Float32Array(src.uv),
		xy: new Float32Array(src.xy)
	};
}

/**
 * Calculate effect animation base on the keyframe structure.
 *
 * @param {STRLayer} layer
 * @param {number} key index
 * @param {object} animation structure
 */
function calculateAnimation(layer, keyIndex, result) {
	let lastFrame = 0;
	let lastSource = 0;
	let fromId = -1,
		toId = -1;

	const animations = layer.animations;

	// Animations are sorted by the loader
	for (let i = 0; i < layer.anikeynum; ++i) {
		if (animations[i].frame <= keyIndex) {
			if (animations[i].type === 0) {
				fromId = i;
			}
			if (animations[i].type === 1) {
				toId = i;
			}
		}
		lastFrame = Math.max(lastFrame, animations[i].frame);

		if (animations[i].type === 0) {
			lastSource = Math.max(lastSource, animations[i].frame);
		}
	}

	// Nothing to render
	if (fromId < 0 || (toId < 0 && lastFrame < keyIndex)) {
		return false;
	}

	const from = animations[fromId];
	const to = animations[toId];
	const delta = keyIndex - from.frame;
	result.srcalpha = from.srcalpha;
	result.destalpha = from.destalpha;

	// Static frame (or frame that can't be updated)
	if (toId !== fromId + 1 || to.frame !== from.frame) {
		// No other source
		if (to && lastSource <= from.frame) {
			return false;
		}

		result.angle = from.angle;
		result.aniframe = from.aniframe;

		result.color.set(from.color);
		result.pos.set(from.pos);
		result.uv.set(from.uv);
		result.xy.set(from.xy);

		return true;
	}

	// Morph animation
	result.color[0] = from.color[0] + to.color[0] * delta;
	result.color[1] = from.color[1] + to.color[1] * delta;
	result.color[2] = from.color[2] + to.color[2] * delta;
	result.color[3] = from.color[3] + to.color[3] * delta;

	result.uv[0] = from.uv[0] + to.uv[0] * delta;
	result.uv[1] = from.uv[1] + to.uv[1] * delta;
	result.uv[2] = from.uv[2] + to.uv[2] * delta;
	result.uv[3] = from.uv[3] + to.uv[3] * delta;
	result.uv[4] = from.uv[4] + to.uv[4] * delta;
	result.uv[5] = from.uv[5] + to.uv[5] * delta;
	result.uv[6] = from.uv[6] + to.uv[6] * delta;
	result.uv[7] = from.uv[7] + to.uv[7] * delta;

	result.xy[0] = from.xy[0] + to.xy[0] * delta;
	result.xy[1] = from.xy[1] + to.xy[1] * delta;
	result.xy[2] = from.xy[2] + to.xy[2] * delta;
	result.xy[3] = from.xy[3] + to.xy[3] * delta;
	result.xy[4] = from.xy[4] + to.xy[4] * delta;
	result.xy[5] = from.xy[5] + to.xy[5] * delta;
	result.xy[6] = from.xy[6] + to.xy[6] * delta;
	result.xy[7] = from.xy[7] + to.xy[7] * delta;

	result.angle = from.angle + to.angle * delta;
	result.pos[0] = from.pos[0] + to.pos[0] * delta;
	result.pos[1] = from.pos[1] + to.pos[1] * delta;

	switch (to.anitype) {
		default: // bug fix
			result.aniframe = 0;
			break;

		case 1: // normal
			result.aniframe = from.aniframe + to.aniframe * delta;
			break;

		case 2: // Stop at end
			result.aniframe = Math.min(from.aniframe + to.delay * delta, layer.texcnt - 1);
			break;

		case 3: // Repeat
			result.aniframe = (from.aniframe + to.delay * delta) % layer.texcnt;
			break;

		case 4: // play reverse infinitly
			result.aniframe = (from.aniframe - to.delay * delta) % layer.texcnt;
			break;
	}

	return true;
}

/**
 * StrEffect constructor
 *
 * @param {string} str effect file
 * @param {Array} effect position
 * @param {string} start tick
 * @param {string} texturePath
 */
class StrEffect {
	constructor(filename, position, startTick, texturePath) {
		this.filename = filename;
		this.startTick = startTick;
		this.position = position;
		this.texturePath = texturePath;

		// If can't render it, just remove it.
		Client.loadFile(
			this.filename,
			null,
			() => {
				this.needCleanUp = true;
			},
			{ texturePath }
		);
	}

	/**
	 * Preparing for render
	 *
	 * @param {object} webgl context
	 */
	init(gl) {
		this.ready = true;
	}

	/**
	 * Destroying data
	 *
	 * @param {object} webgl context
	 */
	free(gl) {
		this.ready = false;
	}

	/**
	 * Render in 3D effect
	 *
	 * @param {object} gl
	 * @param {number} tick
	 */
	render(gl, tick) {
		let layer;
		let i, keyIndex;

		// Follow entity position for attachments
		if (this.ownerEntity && this.ownerEntity.position) {
			this.position = this.ownerEntity.position;
			this.ownerDirection = this.ownerEntity.direction;
			if (this._Params && this._Params.Inst) {
				this._Params.Inst.position = this.position;
			}
		}

		const strFile = Client.loadFile(this.filename, null, null, { texturePath: this.texturePath });

		// Not loaded yet
		if (strFile === null) {
			return;
		}

		keyIndex = ((tick - this.startTick) / 1000) * strFile.fps;

		// Loop persistent effects
		if (this.persistent && strFile && strFile.maxKey) {
			keyIndex = keyIndex % strFile.maxKey;
		}

		// Cache for blank frame handling in hat effects
		if (!this._lastValidAnim) {
			this._lastValidAnim = {};
		}

		let anyFreshFrame = false;

		for (i = 0; i < strFile.layernum; i++) {
			layer = strFile.layers[i];

			if (layer.materials.length) {
				if (calculateAnimation(layer, keyIndex, anim)) {
					if (layer.materials[anim.aniframe | 0]) {
						this.renderAnimation(gl, layer.materials[anim.aniframe | 0], anim);
						anyFreshFrame = true;
						// Cache for hat effect blank frame handling
						if (this.persistent && this.ownerEntity) {
							this._lastValidAnim[i] = {
								anim: copyAnim(anim),
								material: layer.materials[anim.aniframe | 0]
							};
						}
					}
				} else if (this.persistent && this.ownerEntity && this._lastValidAnim[i]) {
					// Use cached frame for blank frames in hat effects
					this.renderAnimation(gl, this._lastValidAnim[i].material, this._lastValidAnim[i].anim);
				}
			}
		}

		// Reset hat effect animation when in blank region to avoid gaps
		if (this.persistent && this.ownerEntity && !anyFreshFrame && strFile.maxKey > 0) {
			this.startTick = tick;
			this._lastValidAnim = {};
		}

		// animation ended
		if (keyIndex >= strFile.maxKey) {
			this.needCleanUp = true;
		}
	}

	/**
	 * Setup geometries, send data to GPU
	 *
	 * @param {object} webgl context
	 * @param {glTexture} webgl texture
	 * @param {StrAnimation} animation object
	 */
	renderAnimation(gl, material, animat) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;

		// Hat effects: Scale with entity size
		let sizeScale = 1.0;
		if (this.ownerEntity) {
			sizeScale = (this.ownerEntity.xSize + this.ownerEntity.ySize) / 2 / 5;
		}

		// Update geometries (apply size scaling only for attachments)
		_bufferData[0] = animat.xy[0] * sizeScale;
		_bufferData[1] = animat.xy[4] * sizeScale;
		_bufferData[2] = 0; //anim.uv[0];
		_bufferData[3] = 0; //anim.uv[1];

		_bufferData[4] = animat.xy[1] * sizeScale;
		_bufferData[5] = animat.xy[5] * sizeScale;
		_bufferData[6] = 1; //anim.uv[2];
		_bufferData[7] = 0; //anim.uv[3];

		_bufferData[8] = animat.xy[3] * sizeScale;
		_bufferData[9] = animat.xy[7] * sizeScale;
		_bufferData[10] = 0; //anim.uv[4];
		_bufferData[11] = 1; //anim.uv[5];

		_bufferData[12] = animat.xy[2] * sizeScale;
		_bufferData[13] = animat.xy[6] * sizeScale;
		_bufferData[14] = 1; //anim.uv[6];
		_bufferData[15] = 1; //anim.uv[7];

		if (animat.angle !== _lastAngle) {
			mat4.identity(_matrix);
			mat4.rotateZ(_matrix, _matrix, (-animat.angle / 180) * Math.PI);
			_lastAngle = animat.angle;
		}

		const spriteOffset = new Float32Array(2);
		let verticalBase = 0.5;

		if (this.ownerEntity) {
			// Attachment: center at (320,320), apply offsets, scale with entity
			spriteOffset[0] = animat.pos[0] - 320 + (this.xOffset || 0) * PIXEL_TO_WORLD_Z * 35 * sizeScale;
			// flip Y offset only
			spriteOffset[1] = animat.pos[1] - 320 - (this.yOffset || 0) * PIXEL_TO_WORLD_Z * 35 * sizeScale;
			verticalBase = 0.0;
		} else {
			spriteOffset[0] = animat.pos[0] - 320;
			spriteOffset[1] = animat.pos[1] - 320;
		}

		// Send effect parameters
		gl.uniform4fv(uniform.uSpriteColor, animat.color);
		gl.uniform2fv(uniform.uSpriteOffset, spriteOffset);
		gl.uniform1f(uniform.uVerticalBase, verticalBase);
		gl.uniform3fv(uniform.uSpritePosition, this.position);

		gl.uniformMatrix4fv(uniform.uSpriteAngle, false, _matrix);

		// Send new buffer
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
		gl.bufferSubData(gl.ARRAY_BUFFER, 0, _bufferData);

		// Link attribute
		gl.vertexAttribPointer(attribute.aPosition, 2, gl.FLOAT, false, 4 * 4, 0 * 4);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 4 * 4, 2 * 4);

		// Send texture and data
		gl.blendFunc(D3DBLEND[animat.srcalpha], D3DBLEND[animat.destalpha]);
		gl.bindTexture(gl.TEXTURE_2D, material);
		gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
	}

	/**
	 * Initialize StrEffect
	 *
	 * @param {object} gl context
	 */
	static init(gl) {
		if (!_buffer) {
			_buffer = gl.createBuffer();
			gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
			gl.bufferData(gl.ARRAY_BUFFER, _bufferData.byteLength, gl.DYNAMIC_DRAW);
		}

		if (!_program) {
			_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
		}

		D3DBLEND[1] = gl.ZERO;
		D3DBLEND[2] = gl.ONE;
		D3DBLEND[3] = gl.SRC_COLOR;
		D3DBLEND[4] = gl.ONE_MINUS_SRC_COLOR;
		D3DBLEND[5] = gl.SRC_ALPHA;
		D3DBLEND[6] = gl.ONE_MINUS_SRC_ALPHA;
		D3DBLEND[7] = gl.DST_ALPHA;
		D3DBLEND[8] = gl.ONE_MINUS_DST_ALPHA;
		D3DBLEND[9] = gl.DST_COLOR;
		D3DBLEND[10] = gl.ONE_MINUS_DST_COLOR;
		D3DBLEND[11] = gl.SRC_ALPHA_SATURATE;
		D3DBLEND[14] = gl.CONSTANT_COLOR;
		D3DBLEND[15] = gl.ONE_MINUS_CONSTANT_ALPHA;

		this.ready = true;
	}

	/**
	 * Destroy Effect
	 *
	 * @param {object} webgl context
	 */
	static free(gl) {
		if (_program) {
			gl.deleteProgram(_program);
			_program = null;
		}

		if (_buffer) {
			gl.deleteBuffer(_buffer);
			_buffer = null;
		}

		this.ready = false;
	}

	/**
	 * Bind context
	 *
	 * @param {object} gl context
	 * @param {mat4} modelview
	 * @param {mat4} projection
	 * @param {object} fog structure
	 * @param {number} tick
	 */
	static beforeRender(gl, modelView, projection, fog, tick) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;

		gl.depthMask(false);
		gl.useProgram(_program);
		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

		// Fog settings
		gl.uniform1f(uniform.uFogNear, fog.near * 100);
		gl.uniform1f(uniform.uFogFar, fog.far * 150);
		gl.uniform3fv(uniform.uFogColor, fog.color);

		// Textures
		gl.uniform1i(uniform.uDiffuse, 0);

		// Enable all attributes
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);

		gl.activeTexture(gl.TEXTURE0);
	}

	/**
	 * Unbind 3D Context
	 *
	 * @param {object} gl context
	 */
	static afterRender(gl) {
		gl.depthMask(true);

		gl.disableVertexAttribArray(_program.attribute.aPosition);
		gl.disableVertexAttribArray(_program.attribute.aTextureCoord);

		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}
}
/**
 * Export
 */
export default StrEffect;
