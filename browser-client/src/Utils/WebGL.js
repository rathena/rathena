/**
 * Utils/WebGL.js
 *
 * WebGL Helper function
 *
 * Trying to define here some functions related to webgl.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Texture from 'Utils/Texture.js';
import Configs from 'Core/Configs.js';

/**
 * Get WebGL Context
 *
 * @param {object} canvas element
 * @param {object} parameters
 *
 * @return {object} webgl context
 */
export function getContext(canvas, parameters) {
	let gl = null;
	let i;

	// Default options
	if (!parameters) {
		parameters = {
			alpha: false,
			depth: true,
			stencil: false,
			antialias: true,
			premultipliedAlpha: false,
			preserveDrawingBuffer: true
		};
	}

	// Prefer a high-performance context when available
	if (parameters.powerPreference === undefined) {
		parameters.powerPreference = 'high-performance';
	}

	// WebGL2 only (fallback to experimental-webgl2 name if needed)
	const names = ['webgl2', 'experimental-webgl2'];
	const count = names.length;

	// Find the context
	if (canvas.getContext) {
		for (i = 0; i < count; ++i) {
			try {
				gl = canvas.getContext(names[i], parameters);
				if (gl) {
					break;
				}
			} catch (e) {
				console.error(e);
			}
		}
	}

	// :(
	if (!gl) {
		throw new Error('WebGL::getContext() - WebGL2 is required but not available.');
	}

	return gl;
}

/**
 * Compile Webgl shader (fragment and vertex)
 *
 * @param {object} gl context
 * @param {string} source
 * @param {number} type (fragment or shader constant)
 */
export function compileShader(gl, source, type) {
	let error;

	// Ensure #version is first token by trimming leading whitespace/BOM
	if (source && source.charCodeAt(0) === 0xfeff) {
		source = source.slice(1);
	}
	source = source.replace(/^\s+/, '');

	// Compile shader
	const shader = gl.createShader(type);
	gl.shaderSource(shader, source);
	gl.compileShader(shader);

	// Is there an error ?
	if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
		error = gl.getShaderInfoLog(shader);
		gl.deleteShader(shader);

		// More descriptive error
		const typeStr = type === gl.VERTEX_SHADER ? 'Vertex' : 'Fragment';
		throw new Error('WebGL::CompileShader() - Fail to compile ' + typeStr + ' shader: ' + error);
	}

	return shader;
}

/**
 * Create a Program from a webgl shader
 *
 * @param {object} gl context
 * @param {string} vertexShader
 * @param {string} fragmentShader
 */
export function createShaderProgram(gl, vertexShader, fragmentShader) {
	let shaderProgram, vs, fs, attrib, uniform, i, count, error;

	// Compile shader and attach them
	try {
		shaderProgram = gl.createProgram();
		vs = compileShader(gl, vertexShader, gl.VERTEX_SHADER);
		fs = compileShader(gl, fragmentShader, gl.FRAGMENT_SHADER);

		gl.attachShader(shaderProgram, vs);
		gl.attachShader(shaderProgram, fs);
		gl.linkProgram(shaderProgram);

		// Is there an error
		if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
			error = gl.getProgramInfoLog(shaderProgram);
			gl.deleteProgram(shaderProgram);
			gl.deleteShader(vs);
			gl.deleteShader(fs);

			throw new Error('WebGL::CreateShaderProgram() - Fail to link shaders : ' + error);
		}
	} catch (e) {
		console.error('Critical WebGL Shader Error:', e);
		// Clean up if partial
		if (shaderProgram) {
			gl.deleteProgram(shaderProgram);
		}
		if (vs) {
			gl.deleteShader(vs);
		}
		if (fs) {
			gl.deleteShader(fs);
		}
		throw e;
	}

	// Get back attributes
	count = gl.getProgramParameter(shaderProgram, gl.ACTIVE_ATTRIBUTES);
	shaderProgram.attribute = {};

	for (i = 0; i < count; i++) {
		attrib = gl.getActiveAttrib(shaderProgram, i);
		shaderProgram.attribute[attrib.name] = gl.getAttribLocation(shaderProgram, attrib.name);
	}

	// Get back uniforms
	count = gl.getProgramParameter(shaderProgram, gl.ACTIVE_UNIFORMS);
	shaderProgram.uniform = {};

	for (i = 0; i < count; i++) {
		uniform = gl.getActiveUniform(shaderProgram, i);
		shaderProgram.uniform[uniform.name] = gl.getUniformLocation(shaderProgram, uniform.name);
	}

	return shaderProgram;
}

/**
 * Webgl Require textures to be power of two size
 *
 * @param {number} num
 * @return {number}
 */
export function toPowerOfTwo(num) {
	return Math.pow(2, Math.ceil(Math.log(num) / Math.log(2)));
}

/**
 * Load an image and push it to GPU
 *
 * @param {object} gl context
 * @param {string} url
 * @param {function} callback once the image is on gpu
 */
export function texture(gl, url, callback) {
	const args = Array.prototype.slice.call(arguments, 3);

	Texture.load(url, function (success) {
		if (!success) {
			return;
		}

		// Defensive: Check if context is lost before trying to create textures
		if (gl.isContextLost()) {
			console.warn('WebGL::texture - context lost, skipping texture creation');
			return;
		}
		try {
			const enableMipmap = Configs.get('enableMipmap');

			const canvas = document.createElement('canvas');
			canvas.width = toPowerOfTwo(this.width);
			canvas.height = toPowerOfTwo(this.height);
			const ctx = canvas.getContext('2d');
			ctx.drawImage(this, 0, 0, canvas.width, canvas.height);

			const _texture = gl.createTexture();
			gl.bindTexture(gl.TEXTURE_2D, _texture);
			gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
			if (enableMipmap) {
				gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
				gl.generateMipmap(gl.TEXTURE_2D);
			} else {
				gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			}

			args.unshift(_texture);
			callback.apply(null, args);
		} catch (e) {
			console.error('WebGL::texture creation error:', e);
		}
	});
}

/**
 * Detect Post-Processing can be Enabled (Bad Combination of Chrome+SoftwareOnly+IntelXE(block-listed by WebGL- crbug.com/41479539))
 * @param {object} gl context
 * @returns {boolean}
 */
export function detectBadWebGL(gl) {
	const renderer = gl.getParameter(gl.RENDERER) || '';
	const vendor = gl.getParameter(gl.VENDOR) || '';
	const ua = navigator.userAgent;

	const isSwiftShader = renderer.toLowerCase().includes('swiftshader');
	const isIntel = vendor.toLowerCase().includes('intel');
	const isChrome = /chrome|chromium/i.test(ua) && !/edg|opr/i.test(ua);

	if (isSwiftShader && isIntel && isChrome) {
		console.warn('[WebGL] PostProcessing disabled due to WebGL compatibility issue:', {
			reason: 'Intel + Chrome + SwiftShader',
			vendor: gl.getParameter(gl.VENDOR),
			renderer: gl.getParameter(gl.RENDERER),
			version: gl.getParameter(gl.VERSION)
		});
	}

	return isSwiftShader && isIntel && isChrome;
}

/**
 * Check if the context is a WebGL2 context
 *
 * @param {object} gl context
 * @return {boolean}
 */
export function isWebGL2(gl) {
	return gl && window['WebGL2RenderingContext'] !== undefined && gl instanceof WebGL2RenderingContext;
}

/**
 * Export
 */
export default {
	getContext: getContext,
	compileShader: compileShader,
	createShaderProgram: createShaderProgram,
	detectBadWebGL: detectBadWebGL,
	toPowerOfTwo: toPowerOfTwo,
	texture: texture,
	isWebGL2: isWebGL2
};
