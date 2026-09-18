/**
 * Renderer/SpriteRenderer.js
 *
 * Rendering sprite in 2D or 3D context
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Camera from './Camera.js';
import _vertexShader from './SpriteRenderer.vs?raw';
import _fragmentShader from './SpriteRenderer.fs?raw';

/**
 * Import
 */
const mat4 = glMatrix.mat4;

/**
 * Render in 3D mode
 */
function RenderCanvas3D(isBlendModeOne) {
	// Nothing to render ?
	if (!this.image.texture || !this.color[3]) {
		return;
	}

	// gl.uniform* seems to be expensive
	// cache values to avoid flooding the GPU and reducing perf.

	const uniform = _program.uniform;
	const gl = _gl;
	const use_pal = this.image.palette !== null;

	if (isBlendModeOne) {
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE);
	} else if (isBlendModeOne === false) {
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}

	if (this.shadow !== _shadow) {
		gl.uniform1f(uniform.uShadow, (_shadow = this.shadow));
	}
	gl.uniform3fv(uniform.uSpriteRendererPosition, this.position);

	// Palette
	if (use_pal) {
		gl.activeTexture(gl.TEXTURE1);
		gl.bindTexture(gl.TEXTURE_2D, this.image.palette);
		gl.uniform2fv(uniform.uTextSize, this.image.size);
		gl.activeTexture(gl.TEXTURE0);
	}

	if (_usepal !== use_pal) {
		gl.uniform1i(uniform.uUsePal, (_usepal = use_pal));
	}

	if (this.depth !== _depth) {
		gl.uniform1f(uniform.uSpriteRendererDepth, (_depth = this.depth));
	}

	const disableDepthCorrection = !!this.disableDepthCorrection;
	if (_disableDepthCorrection !== disableDepthCorrection) {
		_disableDepthCorrection = disableDepthCorrection;
		gl.uniform1i(uniform.uDisableDepthCorrection, disableDepthCorrection);
	}

	gl.uniform1f(uniform.uSpriteRendererZindex, this.zIndex++);
	// Rotate
	if (this.angle !== _angle) {
		_angle = this.angle;

		mat4.identity(_matrix);
		if (_angle) {
			mat4.rotateZ(_matrix, _matrix, (-_angle / 180) * Math.PI);
		}

		gl.uniformMatrix4fv(uniform.uSpriteRendererAngle, false, _matrix);
	}

	_offset[0] = (this.offset[0] / 175.0) * this.xSize;
	_offset[1] = (this.offset[1] / 175.0) * this.ySize - 0.5;
	_size[0] = (this.size[0] / 175.0) * this.xSize;
	_size[1] = (this.size[1] / 175.0) * this.ySize;

	gl.uniform4fv(uniform.uSpriteRendererColor, this.color);
	gl.uniform2fv(uniform.uSpriteRendererSize, _size);
	gl.uniform2fv(uniform.uSpriteRendererOffset, _offset);
	gl.uniform1i(uniform.uIsRGBA, this.sprite ? this.sprite.type : 1);

	// Avoid binding the new texture 150 times if it's the same.
	if (_groupId !== _lastGroupId || _texture !== this.image.texture) {
		_lastGroupId = _groupId;
		gl.bindTexture(gl.TEXTURE_2D, (_texture = this.image.texture));
	}
	gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
}

/**
 * Render in 2D
 */
const RenderCanvas2D = (function RenderCanvas2DClosure() {
	let imageData;

	const canvas = document.createElement('canvas');
	const ctx = canvas.getContext('2d');
	canvas.width = 20;
	canvas.height = 20;
	imageData = ctx.createImageData(canvas.width, canvas.height);

	return function () {
		// Nothing to render
		if (this.sprite.width <= 0 || this.sprite.height <= 0) {
			return;
		}

		let scale_x, scale_y;
		let x, y;
		let r, g, b, a, inRow, outRow;

		scale_x = 1.0;
		scale_y = 1.0;
		const _x = _pos[0] + this.offset[0];
		const _y = _pos[1] + this.offset[1] - 0.5 * 35; // middle of cell
		const pal = this.palette;
		const frame = this.sprite;
		const width = frame.width;
		const height = frame.height;

		_size.set(this.size);

		// Mirror feature
		if (_size[0] < 0) {
			scale_x *= -1;
			_size[0] *= -1;
		}

		if (_size[1] < 0) {
			scale_y *= -1;
			_size[1] *= -1;
		}

		// Resize canvas from memory
		if (width > canvas.width || height > canvas.height) {
			canvas.width = width;
			canvas.height = height;
			imageData = ctx.createImageData(width, height);
		}

		const input = frame.data;
		const color = this.color; // [r, g, b, a] as floats 0..1
		const outputWidth = canvas.width;

		// Use 32-bit view for the output buffer (ImageData)
		// WHY: Writing a single 32-bit value per pixel is faster than 4 separate byte writes.
		const output32 = new Uint32Array(imageData.data.buffer);

		// Pre-calculate color multipliers for 32-bit assembly
		// Avoid repeated array lookups inside the inner loop.
		const r_mul = color[0],
			g_mul = color[1],
			b_mul = color[2],
			a_mul = color[3];

		// Fast path: no color modulation (identity)
		const isColorIdentity = r_mul === 1 && g_mul === 1 && b_mul === 1 && a_mul === 1;

		// RGBA images
		if (this.sprite.type === 1) {
			/**
			 * OLD LOGIC: Per-channel RGBA modulation using byte array access.
			 *            4 loads + 4 stores + multiplications per pixel.
			 * NEW LOGIC: Reads and writes pixels as a single 32-bit integer.
			 *            Uses bitwise extraction and assembly with optional color modulation.
			 *            1 load + 1 store per pixel in the fast path.
			 * Reduces memory writes and bounds checks inside the inner loop.
			 */
			const input32 = new Uint32Array(input.buffer);

			for (y = 0; y < height; ++y) {
				outRow = y * outputWidth;
				inRow = y * width;

				for (x = 0; x < width; ++x) {
					const pixel = input32[inRow + x];
					if (pixel === 0) {
						// Transparent skip behavior due n*0 = 0
						output32[outRow + x] = 0;
						continue;
					}

					if (isColorIdentity) {
						// Fast path: no color modulation.
						// Copy the precompiled RGBA pixel directly due n*1 = n.
						output32[outRow + x] = pixel;
					} else {
						// Extract RGBA components from packed 32-bit pixel.
						// Note: In Little Endian, 0xAABBGGRR is stored as [R, G, B, A] in memory.
						r = (pixel & 0xff) * r_mul;
						g = ((pixel >> 8) & 0xff) * g_mul;
						b = ((pixel >> 16) & 0xff) * b_mul;
						a = ((pixel >> 24) & 0xff) * a_mul;
						output32[outRow + x] = (a << 24) | (b << 16) | (g << 8) | r;
					}
				}
			}
		}

		// Palettes
		else {
			// Pre-calculate a color-modulated 32-bit palette for this frame.
			// WHY: Avoid per-pixel palette lookups and color multiplications.
			// Cost: O(256) setup, O(pixels) usage.
			const pal32 = new Uint32Array(256);
			for (let i = 0; i < 256; i++) {
				if (i === 0) {
					// Transparent skip behavior due n*0 = 0
					pal32[i] = 0;
					continue;
				}
				const pIdx = i * 4;
				r = (pal[pIdx + 0] * r_mul) | 0;
				g = (pal[pIdx + 1] * g_mul) | 0;
				b = (pal[pIdx + 2] * b_mul) | 0;
				a = (255 * a_mul) | 0;
				// Store in LE format [R, G, B, A] -> 0xAABBGGRR
				pal32[i] = (a << 24) | (b << 16) | (g << 8) | r;
			}

			for (y = 0; y < height; ++y) {
				outRow = y * outputWidth;
				inRow = y * width;
				for (x = 0; x < width; ++x) {
					// Fast palette lookup: single array access and single 32-bit write.
					// OLD: Per-channel palette reads and multiplications per pixel.
					// NEW: O(1) lookup using precomputed 32-bit palette.
					output32[outRow + x] = pal32[input[inRow + x]];
				}
			}
		}

		// Insert into the canvas
		ctx.putImageData(imageData, 0, 0, 0, 0, width, height);

		// Render sprite in context
		_ctx.save();
		_ctx.translate(_x | 0, _y | 0);
		_ctx.rotate((this.angle / 180) * Math.PI);
		_ctx.scale(scale_x, scale_y);
		_ctx.drawImage(canvas, 0, 0, width, height, -_size[0] >> 1, -_size[1] >> 1, _size[0] | 0, _size[1] | 0);
		_ctx.restore();
	};
})();

/**
 * @type {WebGLProgram}
 */
let _program = null;

/**
 * @type {WebGLBuffer}
 */
let _buffer = null;

/**
 * @type {CanvasRenderingContext2D} canvas context
 */
let _ctx = null;

/**
 * @type {WebGLRenderingContext} 3d context
 */
let _gl = null;

/**
 * @type {number} group id
 */
let _groupId = 0;

/**
 * @type {number} last group id
 */
let _lastGroupId = 0;

/**
 * @type {number} last shadow used
 */
let _shadow = null;

/**
 * @type {number} last rotation angle used
 */
let _angle = null;

/**
 * @type {number} last depth operation
 */
let _depth = null;

/**
 * @type {boolean} cached disable depth correction state
 */
let _disableDepthCorrection = false;

/**
 * @type {boolean} cached depth mask state
 */
let _depthMask = true;

/**
 * @type {boolean} cached depth test state
 */
let _depthTest = true;

/**
 * @type {object} last texture used
 */
let _texture = null;

/**
 * @type {boolean} do we use palette ?
 */
let _usepal = null;

/**
 * @const {Int16Array} position in 2D canvas
 */
const _pos = new Int16Array(2);

/**
 * @const {Float32Array} last generated matrix (used for rotation)
 */
const _matrix = new Float32Array(4 * 4);

/**
 * @const {Float32Array} sprite size
 */
const _size = new Float32Array(2);

/**
 * @const {Float32Array} sprite offset position
 */
const _offset = new Float32Array(2);

/**
 * Sprite Renderer NameSpace
 */
class SpriteRenderer {
	/**
	 * @type {function} functions to use to render
	 */
	static render = null;

	/**
	 * @type {number} sprite shadow (mult * color)
	 */
	static shadow = 1.0;

	/**
	 * @type {number} sprite angle rotation
	 */
	static angle = 0;

	/**
	 * @type {number} depth
	 */
	static depth = 0.0;

	/**
	 * @type {Float32Array} sprite position in 3D world
	 */
	static position = new Float32Array(3);

	/**
	 * @type {Float32Array} sprite color (color * color)
	 */
	static color = new Float32Array(4);

	/**
	 * @type {Float32Array} sprite size
	 */
	static size = new Float32Array(2);

	/**
	 * @type {Float32Array} sprite offset position
	 */
	static offset = new Float32Array(2);

	/**
	 * @type {object} sprite image information
	 */
	static image = {
		texture: null,
		palette: null,
		size: new Float32Array(2)
	};

	/**
	 * @type {object} sprite imageData (for 2D context)
	 */
	static sprite = null;

	/**
	 * @type {object} sprite palette (for 2D context)
	 */
	static palette = null;

	/**
	 * @type {number} groupid used (avoid draw call)
	 */
	static groupId = 0;

	/**
	 * @type {boolean} disable depth correction (ray-plane) for current draw
	 */
	static disableDepthCorrection = false;

	/**
	 * @type {number} width unity
	 */
	static xSize = 5;

	/**
	 * @type {number} height unity
	 */
	static ySize = 5;

	/**
	 * Initialize SpriteRenderer Renderer
	 *
	 * @param {object} gl context
	 */
	static init(gl) {
		if (!_buffer) {
			_buffer = gl.createBuffer();
			gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
			gl.bufferData(
				gl.ARRAY_BUFFER,
				new Float32Array([
					-0.5, +0.5, 0.0, 0.0, +0.5, +0.5, 1.0, 0.0, -0.5, -0.5, 0.0, 1.0, +0.5, -0.5, 1.0, 1.0
				]),
				gl.STATIC_DRAW
			);
		}

		if (!_program) {
			_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
		}
	}

	/**
	 * Initialize 3D Context
	 *
	 * @param {object} gl context
	 * @param {mat4} modelView
	 * @param {mat4} projection
	 * @param {object} fog structure
	 */
	static bind3DContext(gl, modelView, projection, fog) {
		const attribute = _program.attribute;
		const uniform = _program.uniform;

		gl.useProgram(_program);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);
		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uViewModelMat, false, mat4.invert(_matrix, modelView));

		// Fog settings
		gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
		gl.uniform1f(uniform.uFogNear, fog.near);
		gl.uniform1f(uniform.uFogFar, fog.far);
		gl.uniform3fv(uniform.uFogColor, fog.color);

		// Textures. Select unit 0 explicitly: uDiffuse samples it, but a previous
		// pass may have left the active unit elsewhere, and only the palette path
		// below happens to reset it - so palette-less sprites (clouds) would bind
		// their texture to whatever unit was current.
		gl.activeTexture(gl.TEXTURE0);
		gl.uniform1i(uniform.uDiffuse, 0);
		gl.uniform1i(uniform.uPalette, 1);

		// Camera position for billboarding
		gl.uniform1f(uniform.uCameraZoom, Camera.zoom);
		gl.uniform1f(uniform.uCameraLatitude, Camera.getLatitude());
		gl.uniform1i(uniform.uDisableDepthCorrection, (_disableDepthCorrection = false));

		// Enable all attributes
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);
		gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

		// Link attribute
		gl.vertexAttribPointer(attribute.aPosition, 2, gl.FLOAT, false, 4 * 4, 0);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 4 * 4, 2 * 4);

		// Binding 3D context
		this.render = RenderCanvas3D;
		this.xSize = 5;
		this.ySize = 5;

		_gl = gl;
		_depthMask = true;
		_groupId++;
	}

	/**
	 * Unbind 3D Context
	 *
	 * @param {object} gl context
	 */
	static unbind(gl) {
		const attribute = _program.attribute;

		gl.disableVertexAttribArray(attribute.aPosition);
		gl.disableVertexAttribArray(attribute.aTextureCoord);
	}

	/**
	 * Prepare to render on 2D context.
	 *
	 * @param {object} ctx canvas context
	 * @param {number} x position
	 * @param {number} y position
	 */
	static bind2DContext(ctx, x, y) {
		_ctx = ctx;
		_pos[0] = x;
		_pos[1] = y;

		this.render = RenderCanvas2D;
		this.xSize = 5;
		this.ySize = 5;
	}

	/**
	 * Executes a render block with an isolated depth state.
	 *
	 * Temporarily overrides depth test, depth write mask, and depth correction
	 * flags, executes the given function, then restores the previous GL state.
	 *
	 * This prevents depth state leakage between render passes and ensures
	 * deterministic layered rendering.
	 *
	 * @param {boolean} use depthTest enable.
	 * @param {boolean} use depthMask enable.
	 * @param {boolean} use disableDepthCorrection enable.
	 * @param {function} execute function with this parameters before restore gl state.
	 */
	static runWithDepth(depthTest, depthMask, depthCorrection, fn) {
		if (!_gl) {
			fn();
			return;
		}

		const prevDepthTest = _depthTest;
		const prevDepthMask = _depthMask;
		const prevDepthCorrection = this.disableDepthCorrection;

		if (_depthTest !== depthTest) {
			_depthTest = depthTest;

			if (depthTest) {
				_gl.enable(_gl.DEPTH_TEST);
			} else {
				_gl.disable(_gl.DEPTH_TEST);
			}
		}

		if (_depthMask !== depthMask) {
			_depthMask = depthMask;
			_gl.depthMask(depthMask);
		}

		if (this.disableDepthCorrection !== depthCorrection) {
			this.disableDepthCorrection = depthCorrection;
		}

		try {
			fn();
		} finally {
			if (_depthTest !== prevDepthTest) {
				_depthTest = prevDepthTest;

				if (prevDepthTest) {
					_gl.enable(_gl.DEPTH_TEST);
				} else {
					_gl.disable(_gl.DEPTH_TEST);
				}
			}

			if (_depthMask !== prevDepthMask) {
				_depthMask = prevDepthMask;
				_gl.depthMask(prevDepthMask);
			}

			if (this.disableDepthCorrection !== prevDepthCorrection) {
				this.disableDepthCorrection = prevDepthCorrection;
			}
		}
	}
}

/**
 * Export
 */
export default SpriteRenderer;
