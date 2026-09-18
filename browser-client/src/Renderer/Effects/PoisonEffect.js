/**
 * Renderer/Effects/PoisonEffect.js
 * Just a purple screen
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author AoShinHo
 */

import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Session from 'Engine/SessionStorage.js';

let _filterFrame = null;
let _active = false;

function ensureFilterFrame(gl) {
	if (_filterFrame && _filterFrame.texture && gl.isTexture(_filterFrame.texture)) {
		return;
	}
	const tex = gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, tex);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, new Uint8Array([255, 255, 255, 255]));
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
	_filterFrame = { texture: tex, width: 1, height: 1, type: 1 };
}

class PoisonEffect {
	static render(gl, modelView, projection, fog) {
		if (!_active || !Session.Entity) {
			return;
		}

		ensureFilterFrame(gl);

		SpriteRenderer.sprite = _filterFrame;
		SpriteRenderer.image.texture = _filterFrame.texture;

		SpriteRenderer.position[0] = Session.Entity.position[0];
		SpriteRenderer.position[1] = Session.Entity.position[1];
		SpriteRenderer.position[2] = Session.Entity.position[2];

		SpriteRenderer.color[0] = 0.5;
		SpriteRenderer.color[1] = 0.0;
		SpriteRenderer.color[2] = 0.5;
		SpriteRenderer.color[3] = 0.15;

		SpriteRenderer.size[0] = 4000;
		SpriteRenderer.size[1] = 4000;
		SpriteRenderer.runWithDepth(false, false, true, () => {
			SpriteRenderer.render(false);
		});
	}

	static setActive(bool) {
		_active = bool;
	}

	static isActive() {
		return _active;
	}
}
export default PoisonEffect;
