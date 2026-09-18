/**
 * Renderer/Map/Ground.js
 *
 * Rendering ground
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import WebGL from 'Utils/WebGL.js';
import Texture from 'Utils/Texture.js';
import Preferences from 'Preferences/Map.js';
import Configs from 'Core/Configs.js';
import _vertexShader from './Ground.vs?raw';
import _fragmentShader from './Ground.fs?raw';

const procCanvas = document.createElement('canvas');
const procCtx = procCanvas.getContext('2d', { willReadFrequently: true });

/**
 * @var {WebGLProgram}
 */
let _program = null;

/**
 * @var {WebGBLuffer}
 */
let _buffer = null;

/**
 * @var {WebGLTexture}
 */
let _lightmap = null;

/**
 * @var {WebGLTexture}
 */
let _tileColor = null;

/**
 * @var {WebGLTexture}
 */
let _textureAtlas = null;

/**
 * @var {WebGLTexture}
 */
let _shadowMap = null;

/**
 * @var {number} total vertices count
 */
let _vertCount = 0;

/**
 * @var {number} Ground width
 */
let _width = 0;

/**
 * @var {number} Ground height
 */
let _height = 0;

/**
 * Render ground
 *
 * @param {object} gl context
 * @param {mat4} modelView
 * @param {mat4} projection
 * @param {mat3} normalMat
 * @param {object} fog structure
 * @param {object} light structure
 */
function render(gl, modelView, projection, normalMat, fog, light) {
	const uniform = _program.uniform;
	const attribute = _program.attribute;

	gl.useProgram(_program);

	// Bind matrix
	gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
	gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

	// Bind light
	gl.uniform3fv(uniform.uLightDirection, light.direction);
	gl.uniform1f(uniform.uLightOpacity, light.opacity);
	gl.uniform3fv(uniform.uLightAmbient, light.ambient);
	gl.uniform3fv(uniform.uLightDiffuse, light.diffuse);
	gl.uniform3fv(uniform.uLightEnv, light.env);

	// Render lightmap ?
	gl.uniform1i(uniform.uLightMapUse, Preferences.lightmap);
	gl.uniform1i(uniform.uPosterize, Preferences.smoothlight === 0);
	gl.uniform1i(uniform.uGammaCorrection, Preferences.smoothlight === 2);

	// Fog settings
	gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
	gl.uniform1f(uniform.uFogNear, fog.near);
	gl.uniform1f(uniform.uFogFar, fog.far);
	gl.uniform3fv(uniform.uFogColor, fog.color);

	// Enable all attributes
	gl.enableVertexAttribArray(attribute.aPosition);
	gl.enableVertexAttribArray(attribute.aVertexNormal);
	gl.enableVertexAttribArray(attribute.aTextureCoord);
	gl.enableVertexAttribArray(attribute.aLightmapCoord);
	gl.enableVertexAttribArray(attribute.aTileColorCoord);

	gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

	// Link attribute
	gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 12 * 4, 0);
	gl.vertexAttribPointer(attribute.aVertexNormal, 3, gl.FLOAT, false, 12 * 4, 3 * 4);
	gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 12 * 4, 6 * 4);
	gl.vertexAttribPointer(attribute.aLightmapCoord, 2, gl.FLOAT, false, 12 * 4, 8 * 4);
	gl.vertexAttribPointer(attribute.aTileColorCoord, 2, gl.FLOAT, false, 12 * 4, 10 * 4);

	// Texture Atlas
	gl.activeTexture(gl.TEXTURE0);
	gl.bindTexture(gl.TEXTURE_2D, _textureAtlas);
	gl.uniform1i(uniform.uDiffuse, 0);

	// LightMap
	gl.activeTexture(gl.TEXTURE1);
	gl.bindTexture(gl.TEXTURE_2D, _lightmap);
	gl.uniform1i(uniform.uLightmap, 1);

	// Tile Color
	gl.activeTexture(gl.TEXTURE2);
	gl.bindTexture(gl.TEXTURE_2D, _tileColor);
	gl.uniform1i(uniform.uTileColor, 2);

	// Send mesh
	gl.drawArrays(gl.TRIANGLES, 0, _vertCount);

	// Is it needed ?
	gl.disableVertexAttribArray(attribute.aPosition);
	gl.disableVertexAttribArray(attribute.aVertexNormal);
	gl.disableVertexAttribArray(attribute.aTextureCoord);
	gl.disableVertexAttribArray(attribute.aLightmapCoord);
	gl.disableVertexAttribArray(attribute.aTileColorCoord);

	// Restore the active texture unit. The lightmap and tile-color binds above
	// leave it on TEXTURE2, and every later pass that calls bindTexture without
	// selecting a unit first then binds to 2 while its shader samples 0 - which
	// still holds _textureAtlas. That is why sky clouds rendered as the map's
	// own ground textures, and why hovering walkable ground hid it: GridSelector
	// sets TEXTURE0 itself, and only runs when the mouse hits the ground.
	gl.activeTexture(gl.TEXTURE0);
}

/**
 * Prepare lightmap and send it to GPU
 * Create a lightmap image with size power of two
 *
 * @param {object} gl context
 * @param {object} lightmap
 * @param {number} size
 */
function initLightmap(gl, lightmap, size) {
	const width = WebGL.toPowerOfTwo(Math.round(Math.sqrt(size)) * 8);
	const height = WebGL.toPowerOfTwo(Math.ceil(Math.sqrt(size)) * 8);
	const enableMipmap = Configs.get('enableMipmap');

	if (!_lightmap) {
		_lightmap = gl.createTexture();
	}

	// Send texture to GPU
	gl.bindTexture(gl.TEXTURE_2D, _lightmap);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, lightmap);

	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	if (enableMipmap) {
		gl.generateMipmap(gl.TEXTURE_2D);
	}
}

/**
 * Prepare Tile Color and send it to GPU
 *
 * @param {object} gl
 * @param {Uint8Array} tilescolor
 * @param {number} width
 * @param {number} height
 */
function initTileColor(gl, tilescolor, width, height) {
	if (WebGL.isWebGL2(gl)) {
		// WebGL2 can handle NPOT textures without performance hit (10x faster!)
		initTileColor2(gl, tilescolor, width, height);
		return;
	}

	const enableMipmap = Configs.get('enableMipmap');

	if (procCanvas.width !== width || procCanvas.height !== height) {
		procCanvas.width = width;
		procCanvas.height = height;
	}

	const imageData = procCtx.createImageData(width, height);
	const data = imageData.data;
	const count = data.length;

	// Set Image pixel
	for (let i = 0; i < count; ++i) {
		data[i] = tilescolor[i];
	}
	procCtx.putImageData(imageData, 0, 0);

	// Build Image with power of two texture * 2 (to smooth)
	const atlasWidth = WebGL.toPowerOfTwo(width);
	const atlasHeight = WebGL.toPowerOfTwo(height);
	const smooth = document.createElement('canvas');
	smooth.width = atlasWidth;
	smooth.height = atlasHeight;
	const ctx = smooth.getContext('2d');

	ctx.fillStyle = 'black';
	ctx.fillRect(0, 0, atlasWidth, atlasHeight);
	ctx.drawImage(procCanvas, 0, 0, atlasWidth, atlasHeight);
	// Send texture to GPU
	if (!_tileColor) {
		_tileColor = gl.createTexture();
	}

	gl.bindTexture(gl.TEXTURE_2D, _tileColor);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, smooth);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	if (enableMipmap) {
		gl.generateMipmap(gl.TEXTURE_2D);
	}
}

/**
 * Prepare Tile Color and send it to GPU (WebGL2 version)
 *
 * @param {object} gl
 * @param {Uint8Array} tilescolor
 * @param {number} width
 * @param {number} height
 */
function initTileColor2(gl, tilescolor, width, height) {
	if (!_tileColor) {
		_tileColor = gl.createTexture();
	}

	gl.bindTexture(gl.TEXTURE_2D, _tileColor);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, tilescolor);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
}

/**
 * Prepare textures and send it to GPU
 * Create a texture atlas where we put all textures to avoid drawcall and optimize perfs
 *
 * @param {Object} gl context
 * @param {Array} textures 's filename
 */
function initTextures(gl, textures) {
	// Find texture size
	const count = textures.length;
	const textureCols = Math.round(Math.sqrt(count));
	const atlasWidth = WebGL.toPowerOfTwo(textureCols * 258);
	const atlasHeight = WebGL.toPowerOfTwo(Math.ceil(Math.sqrt(count)) * 258);

	if (procCanvas.width !== atlasWidth || procCanvas.height !== atlasHeight) {
		procCanvas.width = atlasWidth;
		procCanvas.height = atlasHeight;
	}

	procCtx.clearRect(0, 0, atlasWidth, atlasHeight);
	let loaded = 0;

	function onTextureCompleteBuildAtlas(success, index) {
		if (success) {
			const x = (index % textureCols) * 258;
			const y = Math.floor(index / textureCols) * 258;
			procCtx.drawImage(this, x + 0, y + 0, 258, 258); // generate border
			procCtx.drawImage(this, x + 1, y + 1, 256, 256);
		}

		if (++loaded === count) {
			onTextureAtlasComplete(gl, procCanvas);
		}
	}

	// Fetch all images, and draw them in a mega-texture
	for (let i = 0; i < count; ++i) {
		Texture.load(textures[i], onTextureCompleteBuildAtlas, i);
	}
}

/**
 * Send the texture atlas to GPU
 *
 * @param {object} gl
 * @param {object} atlas - canvas texture
 */
function onTextureAtlasComplete(gl, atlas) {
	// Bind to GPU
	if (!_textureAtlas) {
		_textureAtlas = gl.createTexture();
	}

	gl.bindTexture(gl.TEXTURE_2D, _textureAtlas);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, atlas);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);

	const enableMipmap = Configs.get('enableMipmap');
	if (enableMipmap) {
		gl.generateMipmap(gl.TEXTURE_2D);
	}
}

/**
 * Prepare ground data
 *
 * @param {object} gl context
 * @param {object} data - ground
 */
function init(gl, data) {
	_vertCount = data.meshVertCount;
	_width = data.width;
	_height = data.height;
	_shadowMap = data.shadowMap;

	// Bind buffer, sending mesh to GPU
	if (!_buffer) {
		_buffer = gl.createBuffer();
	}

	// Link program	if not loaded
	if (!_program) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
	}

	gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
	gl.bufferData(gl.ARRAY_BUFFER, data.mesh, gl.STATIC_DRAW);

	// Send lightmap to GPU
	initLightmap(gl, data.lightmap, data.lightmapSize);

	// Send Tile color to GPU
	initTileColor(gl, data.tileColor, data.width, data.height);

	// Send textures to GPU
	initTextures(gl, data.textures);
}

/**
 * Clean texture/buffer from memory
 *
 * @param {object} gl context
 */
function free(gl) {
	if (_lightmap) {
		gl.deleteTexture(_lightmap);
		_lightmap = null;
	}

	if (_tileColor) {
		gl.deleteTexture(_tileColor);
		_tileColor = null;
	}

	if (_textureAtlas) {
		gl.deleteTexture(_textureAtlas);
		_textureAtlas = null;
	}

	if (_buffer) {
		gl.deleteBuffer(_buffer);
		_buffer = null;
	}

	_shadowMap = null;
	_vertCount = 0;
}

/**
 * Return shadow factor
 *
 * @param {number} x
 * @param {number} y
 * @return {number} shadow factor
 */
function getShadowFactor(x, y) {
	// Map not loadead yet
	if (!_shadowMap) {
		return 1.0;
	}

	let _x,
		_y,
		factor = 0;

	// Player is at cell center
	x += 0.5;
	y += 0.5;

	// Get index
	_x = Math.floor(x / 2) * 8;
	_y = Math.floor(y / 2) * 8;

	// Add floor percent
	_x += Math.min((x & 1 ? 4 : 0) + Math.floor((x % 1) * 4), 6);
	_y += Math.min((y & 1 ? 4 : 0) + Math.floor((y % 1) * 4), 6);

	// Smooth shadowmap
	for (y = -3; y < 3; ++y) {
		for (x = -3; x < 3; ++x) {
			factor += _shadowMap[_x + x + (_y + y) * _width * 8];
		}
	}

	// Get back value
	return factor / (6 * 6) / 255;
}

/**
 * Export
 */
export default {
	init: init,
	free: free,
	render: render,
	getShadowFactor: getShadowFactor
};
