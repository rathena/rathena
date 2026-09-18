import WebGL from 'Utils/WebGL.js';
import _vertexShader from './FlatColorTile.vs?raw';
import _fragmentShader from './FlatColorTile.fs?raw';

const _cache = {};

export default function (name, spec) {
	let _program, _buffer;
	if (_cache[name]) {
		return _cache[name];
	}
	if (spec.a === undefined) {
		spec.a = 0.5;
	}

	[spec.r, spec.g, spec.b, spec.a].forEach(function (value) {
		if (value === undefined) {
			throw new Error('FlatColorTile: need to pass r, g, b, a');
		}
		if (value > 1.0 || value < 0.0) {
			throw new Error('FlatColorTile: r, g, b, a need to be 0.0-1.0');
		}
	});

	class FlatColorTile {
		constructor(pos, startTick) {
			this.position = pos;
		}

		init(gl) {
			this.ready = true;
		}

		free(gl) {
			this.ready = false;
		}

		render(gl, tick) {
			if (_program === undefined) {
				return;
			} // temporal hotfix to avoid crash
			gl.uniform3fv(_program.uniform.uPosition, this.position);
			gl.uniform1f(_program.uniform.uSize, 0.5);
			gl.uniform4fv(_program.uniform.uColor, [spec.r, spec.g, spec.b, spec.a]);

			gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
			gl.drawArrays(gl.TRIANGLES, 0, 6);
		}

		static init(gl) {
			_program = WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
			_buffer = gl.createBuffer();

			gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);
			gl.bufferData(
				gl.ARRAY_BUFFER,
				new Float32Array([
					-1.0, -1.0, 0.0, 0.0, +1.0, -1.0, 1.0, 0.0, +1.0, +1.0, 1.0, 1.0, +1.0, +1.0, 1.0, 1.0, -1.0, +1.0,
					0.0, 1.0, -1.0, -1.0, 0.0, 0.0
				]),
				gl.STATIC_DRAW
			);
			FlatColorTile.ready = true;
		}

		static free(gl) {
			if (_program) {
				gl.deleteProgram(_program);
				_program = null;
			}

			if (_buffer) {
				gl.deleteBuffer(_buffer);
			}

			this.ready = false;
		}

		static beforeRender(gl, modelView, projection, fog, tick) {
			const uniform = _program.uniform;
			const attribute = _program.attribute;

			gl.useProgram(_program);

			gl.uniformMatrix4fv(uniform.uModelViewMat, false, modelView);
			gl.uniformMatrix4fv(uniform.uProjectionMat, false, projection);

			gl.enableVertexAttribArray(attribute.aPosition);

			gl.bindBuffer(gl.ARRAY_BUFFER, _buffer);

			gl.vertexAttribPointer(attribute.aPosition, 2, gl.FLOAT, false, 4 * 4, 0);
		}

		static afterRender(gl) {
			gl.disableVertexAttribArray(_program.attribute.aPosition);
		}
	}

	FlatColorTile._uid = name;
	FlatColorTile.renderBeforeEntities = true;
	_cache[name] = FlatColorTile;

	return FlatColorTile;
}
