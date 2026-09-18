/**
 * Renderer/Effects/PostProcess.js
 *
 * Manages the Post-Processing pipeline using a Ping-Pong buffer architecture.
 * Ensures the main scene is rendered at full resolution before applying effects.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import GraphicsSettings from 'Preferences/Graphics.js';

let _effects = [];
let _activeEffects = [];

// Ping-Pong Buffers (Full Resolution)
let _readFbo = null;
let _writeFbo = null;

class PostProcess {
	/**
	 * Register module in pass priority order and init
	 * @param {ShaderModule} module - Post Process Modular effect.
	 * @param {WebGLRenderingContext} gl - The WebGL context.
	 */
	static register(module, gl) {
		if (!module.program || !module.isActive || !module.init || !module.render || !module.clean) {
			console.error('[PostProcess] Incorrect modular Post-Process format registered - please Fix');
			return;
		}
		_effects.push(module);
		module.init(gl);
	}

	/**
	 * Prepare the pipeline for the scene rendering.
	 * Always binds a full-resolution buffer to ensure the 3D scene is sharp.
	 * @param {WebGLRenderingContext} gl - The WebGL context.
	 */
	static prepare(gl) {
		_activeEffects = _effects.filter(e => e.isActive());

		// Ensure global buffers exist and match canvas size
		this.validateBuffers(gl);

		if (_activeEffects.length > 0) {
			// Render the scene into the write buffer (which becomes read buffer in .render())
			PostProcess.beforeRenderPass(gl, _writeFbo);
		} else {
			// No effects? Render directly to screen
			PostProcess.beforeRenderPass(gl, null);
		}
	}

	/**
	 * Executes the post-processing pipeline using Ping-Pong swapping.
	 * @param {WebGLRenderingContext} gl - The WebGL context.
	 */
	static render(gl) {
		if (_activeEffects.length === 0) {
			return;
		}

		// The buffer we just drew the 3D scene into (_writeFbo) becomes the source (_readFbo)
		this.swapBuffers();

		for (let i = 0; i < _activeEffects.length; i++) {
			const effect = _activeEffects[i];
			const isLast = i === _activeEffects.length - 1;

			// Destination: Screen (null) if last, otherwise the next offscreen buffer
			const targetFbo = isLast ? null : _writeFbo;

			// Render the effect: Source (Read) -> Effect Logic -> Destination (Write)
			effect.render(gl, _readFbo.texture, targetFbo);

			// If not the last effect, swap buffers so the output becomes the input for the next one
			if (!isLast) {
				this.swapBuffers();
			}
		}
	}
	/**
	 * Set up the FBO and viewport for the next render pass
	 * @param {WebGLRenderingContext} gl - The WebGL context.
	 * @param {Object} outputFbo - The FBO to render to.
	 */
	static beforeRenderPass(gl, outputFbo) {
		if (outputFbo !== null) {
			gl.bindFramebuffer(gl.FRAMEBUFFER, outputFbo.framebuffer);
			gl.viewport(0, 0, outputFbo.width, outputFbo.height);
		} else {
			gl.bindFramebuffer(gl.FRAMEBUFFER, null);
			gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
		}
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	}

	/**
	 * Cleans up bindings
	 */
	static afterRenderPass(gl) {
		gl.useProgram(null);
		gl.bindBuffer(gl.ARRAY_BUFFER, null);
		gl.bindFramebuffer(gl.FRAMEBUFFER, null);
		gl.bindTexture(gl.TEXTURE_2D, null);
	}

	/**
	 * Swaps the read and write FBO references.
	 */
	static swapBuffers() {
		const temp = _readFbo;
		_readFbo = _writeFbo;
		_writeFbo = temp;
	}

	/**
	 * Ensures Ping-Pong buffers are created and resized if necessary.
	 */
	static validateBuffers(gl) {
		const scale = GraphicsSettings.performanceMode ? 0.75 : 1.0;
		const scaledWidth = Math.floor(gl.canvas.width * scale);
		const scaledHeight = Math.floor(gl.canvas.height * scale);

		if (!_readFbo || _readFbo.width !== scaledWidth || _readFbo.height !== scaledHeight) {
			_readFbo = this.createFbo(gl, scaledWidth, scaledHeight, _readFbo);
			_writeFbo = this.createFbo(gl, scaledWidth, scaledHeight, _writeFbo);
		}
	}

	/**
	 * restart Modules when crashs
	 */
	static restartModules(gl) {
		_activeEffects.forEach(module => {
			module.clean(gl);
			module.init(gl);
		});
		_activeEffects = [];

		// Physically delete Ping-Pong buffers from GPU memory
		if (_readFbo) {
			if (gl.isTexture(_readFbo.texture)) {
				gl.deleteTexture(_readFbo.texture);
			}
			if (gl.isRenderbuffer(_readFbo.rbo)) {
				gl.deleteRenderbuffer(_readFbo.rbo);
			}
			if (gl.isFramebuffer(_readFbo.framebuffer)) {
				gl.deleteFramebuffer(_readFbo.framebuffer);
			}
		}

		if (_writeFbo) {
			if (gl.isTexture(_writeFbo.texture)) {
				gl.deleteTexture(_writeFbo.texture);
			}
			if (gl.isRenderbuffer(_writeFbo.rbo)) {
				gl.deleteRenderbuffer(_writeFbo.rbo);
			}
			if (gl.isFramebuffer(_writeFbo.framebuffer)) {
				gl.deleteFramebuffer(_writeFbo.framebuffer);
			}
		}

		_readFbo = null;
		_writeFbo = null;
	}

	/**
	 * Recreates the FBO when the window size changes
	 */
	static recreateFbo(gl, width, height) {
		const scale = GraphicsSettings.performanceMode ? 0.75 : 1.0;
		const scaledWidth = Math.floor(width * scale);
		const scaledHeight = Math.floor(height * scale);

		// Recreate global buffers
		_readFbo = this.createFbo(gl, scaledWidth, scaledHeight, _readFbo);
		_writeFbo = this.createFbo(gl, scaledWidth, scaledHeight, _writeFbo);

		// Notify modules to recreate their internal buffers (if any)
		_effects.forEach(module => {
			if (module.recreateFbo) {
				module.recreateFbo(gl, scaledWidth, scaledHeight);
			}
		});
	}

	/**
	 * Clean current registered modules
	 */
	static clean(gl) {
		_effects.forEach(module => module.clean(gl));
		_effects = [];
		_activeEffects = [];

		// Physically delete Ping-Pong buffers from GPU memory
		if (_readFbo) {
			if (gl.isTexture(_readFbo.texture)) {
				gl.deleteTexture(_readFbo.texture);
			}
			if (gl.isRenderbuffer(_readFbo.rbo)) {
				gl.deleteRenderbuffer(_readFbo.rbo);
			}
			if (gl.isFramebuffer(_readFbo.framebuffer)) {
				gl.deleteFramebuffer(_readFbo.framebuffer);
			}
		}

		if (_writeFbo) {
			if (gl.isTexture(_writeFbo.texture)) {
				gl.deleteTexture(_writeFbo.texture);
			}
			if (gl.isRenderbuffer(_writeFbo.rbo)) {
				gl.deleteRenderbuffer(_writeFbo.rbo);
			}
			if (gl.isFramebuffer(_writeFbo.framebuffer)) {
				gl.deleteFramebuffer(_writeFbo.framebuffer);
			}
		}
		_readFbo = null;
		_writeFbo = null;
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
	}

	/**
	 * Creates or validates an existing FBO by comparing dimensions.
	 * @param {WebGLRenderingContext} gl
	 * @param {number} width - Desired width
	 * @param {number} height - Desired height
	 * @param {Object} fbo - Current FBO object (if it exists)
	 * @param {number} downsampleFactor - Multiplier for resolution (default 1.0)
	 * @returns {Object|null} New FBO object or the current one if still valid
	 */
	static createFbo(gl, width, height, fbo, downsampleFactor = 1.0) {
		const targetWidth = Math.floor(width * downsampleFactor);
		const targetHeight = Math.floor(height * downsampleFactor);

		try {
			if (!fbo || fbo.width !== targetWidth || fbo.height !== targetHeight) {
				return this.createFramebuffer(gl, targetWidth, targetHeight, fbo);
			}
			return fbo;
		} catch (e) {
			console.error('Failed to create PostProcess FBOs:', e);
			return null;
		}
	}

	/**
	 * Physically creates the Framebuffer, Texture, and Renderbuffer in WebGL.
	 * @param {WebGLRenderingContext} gl
	 * @param {number} width
	 * @param {number} height
	 * @param {Object} oldfbo - Old object for resource cleanup
	 * @returns {Object|null}
	 */
	static createFramebuffer(gl, width, height, oldfbo) {
		try {
			if (oldfbo) {
				// Free old resources to prevent memory leaks
				if (gl.isTexture(oldfbo.texture)) {
					gl.deleteTexture(oldfbo.texture);
				}
				if (gl.isRenderbuffer(oldfbo.rbo)) {
					gl.deleteRenderbuffer(oldfbo.rbo);
				}
				if (gl.isFramebuffer(oldfbo.framebuffer)) {
					gl.deleteFramebuffer(oldfbo.framebuffer);
				}
			}

			const fbo = gl.createFramebuffer();
			gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);

			// Setup Texture where the scene will be drawn
			const texture = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_2D, texture);
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, width, height, 0, gl.RGB, gl.UNSIGNED_BYTE, null);

			// Parameters to avoid distortions
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

			gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);

			// Setup Renderbuffer for depth testing (Z-buffer)
			const rbo = gl.createRenderbuffer();
			gl.bindRenderbuffer(gl.RENDERBUFFER, rbo);
			gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, width, height);
			gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, rbo);

			// Validate Framebuffer state
			const status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
			if (status !== gl.FRAMEBUFFER_COMPLETE) {
				throw new Error('WebGL::createFramebuffer() - Incomplete Framebuffer! Status: ' + status);
			}

			// Clean up bindings
			gl.bindFramebuffer(gl.FRAMEBUFFER, null);
			gl.bindTexture(gl.TEXTURE_2D, null);
			gl.bindRenderbuffer(gl.RENDERBUFFER, null);

			return {
				framebuffer: fbo,
				texture: texture,
				rbo: rbo,
				width: width,
				height: height
			};
		} catch (e) {
			console.error('WebGL::createFramebuffer failed (likely OOM or context loss):', e);
			// Clean up partially created resources
			gl.bindFramebuffer(gl.FRAMEBUFFER, null);
			gl.bindTexture(gl.TEXTURE_2D, null);
			gl.bindRenderbuffer(gl.RENDERBUFFER, null);
			return null;
		}
	}
}
export default PostProcess;
