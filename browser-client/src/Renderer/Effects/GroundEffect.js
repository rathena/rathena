import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Client from 'Core/Client.js';
import Altitude from 'Renderer/Map/Altitude.js';
import _vertexShader from './GroundEffect.vs?raw';
import _fragmentShader from './GroundEffect.fs?raw';

let _texture;
let _program;
const mat4 = glMatrix.mat4;
const _matrix = mat4.create();

class GroundEffect {
	constructor(posX, posY) {
		this.x = posX;
		this.y = posY;
		this.position = new Int16Array([posX, posY, 1]);
		this.size = 5;
	}

	init(gl) {
		const plane = Altitude.generatePlane(this.x, this.y, this.size);
		this.buffer = gl.createBuffer();
		this.vertCount = plane.length / 5;
		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
		gl.bufferData(gl.ARRAY_BUFFER, plane, gl.STATIC_DRAW);
		this.ready = true;
	}

	free(gl) {
		gl.deleteBuffer(this.buffer);
		this.ready = false;
	}

	render(gl, tick) {
		const attribute = _program.attribute;
		gl.bindBuffer(gl.ARRAY_BUFFER, this.buffer);
		gl.vertexAttribPointer(attribute.aPosition, 3, gl.FLOAT, false, 5 * 4, 0);
		gl.vertexAttribPointer(attribute.aTextureCoord, 2, gl.FLOAT, false, 5 * 4, 3 * 4);
		gl.drawArrays(gl.TRIANGLES, 0, this.vertCount);
	}

	static init(gl) {
		_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
		Client.loadFile('data/texture/effect/pok2.tga', buffer => {
			WebGL.texture(gl, buffer, texture => {
				_texture = texture;
				this.ready = true;
			});
		});
		this.color = new Float32Array(4);
		this.color[0] = 1.0;
		this.color[1] = 1.0;
		this.color[2] = 1.0;
		this.color[3] = 0.4;
	}

	static free(gl) {
		if (_texture) {
			gl.deleteTexture(_texture);
			_texture = null;
		}

		if (_program) {
			gl.deleteProgram(_program);
			_program = null;
		}

		this.ready = false;
	}

	static beforeRender(gl, modelView, projection, fog, tick) {
		const uniform = _program.uniform;
		const attribute = _program.attribute;

		gl.depthMask(false);
		mat4.identity(_matrix);
		gl.useProgram(_program);
		gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
		gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);
		gl.uniformMatrix4fv(uniform.uRotationMat, false, _matrix);
		gl.uniform1i(uniform.uFogUse, fog.use && fog.exist);
		gl.uniform1f(uniform.uFogNear, fog.near);
		gl.uniform1f(uniform.uFogFar, fog.far);
		gl.uniform3fv(uniform.uFogColor, fog.color);
		gl.uniform4fv(uniform.uSpriteRendererColor, this.color);
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture(gl.TEXTURE_2D, _texture);
		gl.uniform1i(uniform.uDiffuse, 0);
		gl.enableVertexAttribArray(attribute.aPosition);
		gl.enableVertexAttribArray(attribute.aTextureCoord);
	}

	static afterRender(gl) {
		gl.depthMask(true);
		gl.disableVertexAttribArray(_program.attribute.aPosition);
		gl.disableVertexAttribArray(_program.attribute.aTextureCoord);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}
}
GroundEffect.renderBeforeEntities = false;
export default GroundEffect;
