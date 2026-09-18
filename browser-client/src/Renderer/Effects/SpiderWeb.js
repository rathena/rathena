import WebGL from 'Utils/WebGL.js';
import { FlatTexture } from 'Renderer/Effects/Tiles.js';
import _vertexShader from './SpiderWeb.vs?raw';
import _fragmentShader from './SpiderWeb.fs?raw';

let _lpNum = 0;

class SpiderWeb extends FlatTexture('data/texture/effect/spiderweb.tga', 128) {
	static createShaderProgram(gl) {
		return WebGL.createShaderProgram(gl, _vertexShader, _fragmentShader);
	}

	constructor() {
		super(...arguments);
		this.ix = ++_lpNum;
	}

	render(gl, tick) {
		const oddEven = this.ix % 2 === 0 ? Math.PI : 0;
		const sizeMult = Math.sin(oddEven + tick / (540 * Math.PI));
		gl.uniform3fv(this.constructor._program.uniform.uPosition, this.position);
		gl.uniform1f(this.constructor._program.uniform.uSize, 1.5 + 0.05 * sizeMult);
		gl.bindBuffer(gl.ARRAY_BUFFER, this.constructor._buffer);
		gl.drawArrays(gl.TRIANGLES, 0, 6);
	}
}
export default SpiderWeb;
