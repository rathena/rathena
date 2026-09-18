/**
 * Renderer/Renderer.js
 *
 * Rendering sprite in 2D or 3D context
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import WebGL from 'Utils/WebGL.js';
import glMatrix from 'Utils/gl-matrix.js';
import Configs from 'Core/Configs.js';
import GraphicsSettings from 'Preferences/Graphics.js';
import Events from 'Core/Events.js';
import Background from 'UI/Background.js';
import Cursor from 'UI/CursorManager.js';
import Mouse from 'Controls/MouseEventHandler.js';
import Camera from 'Renderer/Camera.js';
import Session from 'Engine/SessionStorage.js';
import PostProcess from 'Renderer/Effects/PostProcess.js';
import MemoryManager from 'Core/MemoryManager.js';

const { mat4 } = glMatrix;

/**
 * Shime for requestAnimationFrame
 */
const _requestAnimationFrame =
	window.requestAnimationFrame ||
	window.webkitRequestAnimationFrame ||
	window.mozRequestAnimationFrame ||
	window.oRequestAnimationFrame ||
	window.msRequestAnimationFrame ||
	function (callback) {
		return window.setTimeout(callback, 1000 / 60);
	};
/**
 * Shime for cancelAnimationFrame
 */
const _cancelAnimationFrame =
	window.cancelAnimationFrame ||
	window.webkitCancelAnimationFrame ||
	window.mozCancelAnimationFrame ||
	window.oCancelAnimationFrame ||
	window.msCancelAnimationFrame ||
	function (updateId) {
		window.clearTimeout(updateId);
	};

/**
 * Renderer Namespace
 */
class Renderer {
	/**
	 * @type {boolean} Rendering ?
	 */
	static rendering = false;

	/**
	 * @type {HTMLCanvasElement}
	 */
	static canvas = document.createElement('canvas');

	/**
	 * @type {WebGLRenderingContext}
	 */
	static gl = null;

	/**
	 * @type {boolean} true when using a WebGL2 context
	 */
	static isWebGL2 = false;

	/**
	 * @type {boolean} flag for context lost state
	 */
	static contextLost = false;

	/**
	 * @type {HTMLElement} overlay for context loss message
	 */
	static errorOverlay = null;

	/**
	 * @type {number} screen width
	 */
	static width = 0;

	/**
	 * @type {number} screen height
	 */
	static height = 0;

	/**
	 * @type {number} store the last time the windows was resize (to avoid to resize the context on each 16ms)
	 */
	static resizeTimeOut = 0;

	/**
	 * @type {number} unique identifier of the current render callback (can be used for cancelAnimationFrame/clearInterval)
	 */
	static updateId = 0;

	/**
	 * @type {number} frame rate limit
	 */
	static frameLimit = GraphicsSettings.fpslimit;

	/**
	 * @type {number} game tick
	 */
	static tick = 0;

	/**
	 * @type {number} vertical field of view in degrees
	 */
	static vFov = 15.0;

	/**
	 * @type {function[]} callbacks to execute
	 */
	static renderCallbacks = [];

	/**
	 * Initialize renderer
	 */
	static init(param) {
		if (!this.gl) {
			// Fixed viewport: see `body.ro-viewport` in UI/Common.css
			document.body.classList.add('ro-viewport');

			this.canvas.className = 'ro-scene';
			this.canvas.style.position = 'absolute';
			this.canvas.style.top = '0px';
			this.canvas.style.left = '0px';
			this.canvas.style.zIndex = 0;

			// Context Loss Listeners
			this.canvas.addEventListener('webglcontextlost', this.onContextLost.bind(this), false);
			this.canvas.addEventListener('webglcontextrestored', this.onContextRestored.bind(this), false);

			this.gl = WebGL.getContext(this.canvas, param);
			this.isWebGL2 = WebGL.isWebGL2(this.gl);

			if (typeof console !== 'undefined' && console.info) {
				console.info('Renderer using WebGL ' + (this.isWebGL2 ? '2' : '1') + ' context');
			}

			window.addEventListener('resize', this.onResize.bind(this));
			window.addEventListener('contextmenu', event => {
				event.preventDefault();
			});

			// Listen for visual viewport changes (mobile input focus zoom, pinch zoom)
			// to keep the canvas and camera in sync.
			if (window.visualViewport) {
				window.visualViewport.addEventListener('resize', this.onResize.bind(this));
			}

			this.render(null);
			this.resize();
		}

		const gl = this.gl;
		this.isWebGL2 = WebGL.isWebGL2(gl);

		gl.clearDepth(1.0);
		gl.enable(gl.DEPTH_TEST);
		gl.depthFunc(gl.LEQUAL);

		gl.enable(gl.BLEND);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
	}

	/**
	 * Handle WebGL Context Loss
	 * stops the render loop and notifies the user
	 */
	static onContextLost(event) {
		event.preventDefault(); // Prevent default browser behavior (which is not restoring)
		this.contextLost = true;
		console.warn('[Renderer] WebGL Context Lost! Pausing rendering.');

		// Free cached image data — GPU textures are already gone,
		// clean JS-side cache so assets reload fresh on context restore
		MemoryManager.forceClean(null, /\.(bmp|jpg|png|spr|pal|str)$/i);

		this.stop();

		// Create/Show overlay message
		if (!this.errorOverlay) {
			this.errorOverlay = document.createElement('div');
			this.errorOverlay.style.cssText =
				'position:fixed; top:0; left:0; width:100%; height:100%; background:rgba(0,0,0,0.85); color:white; display:flex; flex-direction:column; justify-content:center; align-items:center; z-index:10000; text-align:center;';
			this.errorOverlay.innerHTML =
				'<h2 style="color:#ff6b6b; margin-bottom:10px;">Graphics Context Lost</h2><p>The browser lost connection to the GPU.</p><p style="font-size:0.9em; opacity:0.8;">Attempting to restore automatically...</p>';
			document.body.appendChild(this.errorOverlay);
		} else {
			this.errorOverlay.style.display = 'flex';
		}
	}

	/**
	 * Handle WebGL Context Restoration
	 * Re-initializes WebGL state and resumes rendering
	 */
	static onContextRestored(event) {
		console.info('[Renderer] WebGL Context Restored! Re-initializing...');
		this.contextLost = false;

		if (this.errorOverlay) {
			this.errorOverlay.style.display = 'none';
		}

		const gl = this.gl;

		// Re-detect capabilities
		this.isWebGL2 = WebGL.isWebGL2(gl);

		// Reset Global GL State
		gl.clearDepth(1.0);
		gl.enable(gl.DEPTH_TEST);
		gl.depthFunc(gl.LEQUAL);
		gl.enable(gl.BLEND);
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

		// Restart post process active modules
		PostProcess.restartModules(gl);

		// Trigger resize to reset viewport and framebuffers
		this.resize();

		// Resume Render Loop
		this.render();
	}

	/**
	 * Show renderer
	 */
	static show() {
		if (!this.canvas.parentNode) {
			document.body.appendChild(this.canvas);
		}
	}

	/**
	 * Remove renderer
	 */
	static remove() {
		if (this.canvas.parentNode) {
			document.body.removeChild(this.canvas);
		}
	}

	/**
	 * Get back WebGL Context
	 */
	static getContext() {
		return this.gl;
	}

	/**
	 * Ask for resizing the window, avoid flooding the function (can flood the context), wait for 500ms each time
	 */
	static onResize() {
		Events.clearTimeout(this.resizeTimeOut);
		this.resizeTimeOut = Events.setTimeout(this.resize.bind(this), 500);
	}

	/**
	 * Resizing window
	 */
	static resize() {
		// Don't resize if context is lost
		if (this.contextLost || !this.gl) {
			return;
		}

		let width, height;
		const dpr = window.devicePixelRatio || 1;

		width = window.innerWidth || document.body.offsetWidth;
		height = window.innerHeight || document.body.offsetHeight;

		Mouse.screen.width = this.width = width;
		Mouse.screen.height = this.height = height;

		const quality = Configs.get('quality', 100) / 100;
		width *= quality;
		height *= quality;

		this.canvas.width = width * dpr;
		this.canvas.height = height * dpr;
		this.canvas.style.width = this.width + 'px';
		this.canvas.style.height = this.height + 'px';

		try {
			this.gl.viewport(0, 0, width * dpr, height * dpr);
		} catch (e) {
			console.error('Viewport resize failed', e);
		}

		mat4.perspective(this.vFov, width / height, 1, 1000, Camera.projection);

		Background.resize(this.width, this.height);

		// Updates FBO sizes
		PostProcess.recreateFbo(this.gl, width * dpr, height * dpr);

		// Circular dependency handled with dynamic import
		import('UI/UIManager.js').then(module => {
			module.default.fixResizeOverflow(this.width, this.height);
		});
	}

	/**
	 * Rendering scene
	 */
	static _render(time) {
		// Stop if context is lost
		if (this.contextLost) {
			return;
		}

		// time: DOMHighResTimeStamp (from rAF) or undefined if fallback
		const now = typeof time === 'number' ? time : Date.now();

		// bind cache for scheduling (avoid new bind each frame)
		if (!this._renderBound) {
			this._renderBound = this._render.bind(this);
		}

		// initialize lastFrameTime on first run
		if (typeof this._lastFrameTime === 'undefined' || this._lastFrameTime === 0) {
			this._lastFrameTime = now;
			// ensure tick has a sane initial value
			if (!this.tick) {
				this.tick = Date.now();
			}
		}

		// Force 60 FPS cap on UI-only screens (char select)
		// where there is no 3D scene and uncapped FPS wastes resources
		if (!Session.Playing && (this.frameLimit <= 0 || this.frameLimit > 60)) {
			this.frameLimit = 60;
		} else if (Session.Playing && this.frameLimit !== GraphicsSettings.fpslimit) {
			this.frameLimit = GraphicsSettings.fpslimit;
		}

		// Throttle when frameLimit > 0
		if (this.frameLimit > 0) {
			const interval = 1000 / this.frameLimit;
			const elapsed = now - this._lastFrameTime;

			if (elapsed < interval) {
				// Not enough time elapsed for next allowed frame — schedule next rAF and exit
				this.updateId = _requestAnimationFrame(this._renderBound);
				return;
			}

			// Advance lastFrameTime preserving alignment (avoid time drift)
			// keep lastFrameTime at nearest interval boundary
			this._lastFrameTime = now - (elapsed % interval);
		} else {
			// No limit => run every rAF
			this._lastFrameTime = now;
		}

		// Use Date.now for serverTick and Events processing, to keep existing behavior intact
		const newTick = Date.now();

		// Increment serverTick with delta
		Session.serverTick += newTick - this.tick;

		// Update engine tick
		this.tick = newTick;

		// Execute events
		Events.process(this.tick);

		// Execute render callbacks
		let i, count;
		for (i = 0, count = this.renderCallbacks.length; i < count; ++i) {
			try {
				this.renderCallbacks[i](this.tick, this.gl);
			} catch (e) {
				// Defensive: a single callback shouldn't break the whole loop
				console.error('[Renderer] render callback error', e);

				// Memory Pressure Detection / Fallback
				// If we hit an error during rendering, check if it's OOM or Context Lost related
				if (this.gl.isContextLost()) {
					// Handled by event listener, but break loop now
					return;
				}

				// Basic Heuristic: If errors persist or explicitly OOM
				// We could disable bloom/fancy effects here to recover
				if (GraphicsSettings.bloom) {
					console.warn('[Renderer] Disabling bloom due to render error (potential resource pressure)');
					GraphicsSettings.bloom = false;
				}
			}
		}

		Cursor.render(this.tick);

		// Schedule next frame
		this.updateId = _requestAnimationFrame(this._renderBound);
	}

	/**
	 * Start rendering
	 */
	static render(fn) {
		if (fn && !this.renderCallbacks.includes(fn)) {
			this.renderCallbacks.push(fn);
		}

		if (!this.rendering) {
			this.rendering = true;

			// Cancel any previous interval/animation to be safe
			try {
				clearInterval(this.updateId);
			} catch (e) {
				console.error(e);
			}
			try {
				_cancelAnimationFrame(this.updateId);
			} catch (e) {
				console.error(e);
			}

			// Reset timing helpers so first rAF initializes cleanly
			this._lastFrameTime = 0;
			this._renderBound = this._render.bind(this);

			// Start loop with requestAnimationFrame (safer & sync with browser)
			this.updateId = _requestAnimationFrame(this._renderBound);
		}
	}

	/**
	 * Stop rendering
	 */
	static stop(fn) {
		// No callback specified, remove all
		if (!arguments.length) {
			this.renderCallbacks.length = 0;
			this.rendering = false; // Ensure rendering flag is cleared
			try {
				_cancelAnimationFrame(this.updateId);
			} catch (e) {
				console.error(e);
			}
			return;
		}
		const pos = this.renderCallbacks.indexOf(fn);
		if (pos > -1) {
			this.renderCallbacks.splice(pos, 1);
		}
	}
}

/**
 * Export
 */
export default Renderer;
