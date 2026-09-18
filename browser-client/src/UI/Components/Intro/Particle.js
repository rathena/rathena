/**
 * UI/Components/Intro/Particles.js
 *
 * Manage Intro Particles
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * @Constructor
 */
class Particle {
	constructor(width, height) {
		this.rmax = 10;
		this.xmax = 5;
		this.ymax = 2;

		this.reset(width, height);
	}

	/**
	 * Cross Browser animation Frame
	 */
	static requestRender =
		window.requestAnimationFrame ||
		window.webkitRequestAnimationFrame ||
		window.mozRequestAnimationFrame ||
		window.oRequestAnimationFrame ||
		window.msRequestAnimationFrame ||
		function (callback) {
			window.setTimeout(callback, 1000 / 60);
		};

	/**
	 * Initialize particles
	 *
	 * @param {number} count - particles count
	 * @param {HTMLCanvasElement} canvas
	 * @param {function} [onReady] - callback when background image is loaded
	 */
	static init(count, canvas, onReady) {
		this.list = new Array(count);

		this.width = canvas.width;
		this.height = canvas.height;
		this.canvas = canvas;
		this.ctx = canvas.getContext('2d');
		this.ctx.globalCompositeOperation = 'lighter';

		this.bg = new Image();
		this.bg.decoding = 'async';
		this.bg.src = new URL('./images/background.jpg', import.meta.url).href;
		this.bg.onload = function () {
			this.ready = true;
			if (onReady) onReady();
		};
		this.bg.onerror = function () {
			if (onReady) onReady();
		};

		for (let i = 0; i < count; ++i) {
			this.list[i] = new Particle(this.width, this.height);
		}

		this.process = true;
		this.tick = Date.now();
		this.render();
	}

	/**
	 * Stop particles
	 */
	static stop() {
		this.process = false;
	}

	/**
	 * Render particles
	 */
	static render() {
		const now = Date.now();
		// Throttle to ~60 FPS
		const interval = 1000 / 60;
		if (this._lastRenderTime && now - this._lastRenderTime < interval) {
			if (this.process) {
				this.requestRender.call(window, this.render.bind(this), this.canvas);
			}
			return;
		}

		let i, count;

		this.ctx.clearRect(0, 0, this.width, this.height);
		if (this.bg.ready) {
			this.ctx.globalCompositeOperation = 'source-over';
			this.ctx.drawImage(this.bg, 0, 0);
			this.ctx.globalCompositeOperation = 'lighter';
		}

		for (i = 0, count = this.list.length; i < count; ++i) {
			this.list[i].update(now - this.tick, 0, 0, this.width, this.height);
			this.list[i].render(this.ctx);
		}

		this.tick = now;
		if (this.process) {
			this.requestRender.call(window, this.render.bind(this), this.canvas);
		}
		this._lastRenderTime = now;
	}

	/**
	 * Reset particles informations
	 *
	 * @param {number} width
	 * @param {number} height
	 */
	reset(width, height) {
		this.x = width * Math.random();
		this.y = height * Math.random();
		this.r = (this.rmax - 1) * Math.random() + 1;

		this.c = Math.floor(255 * Math.random());

		this.dx = Math.random() * this.xmax * 2 - this.xmax;
		this.dy = Math.random() * this.ymax * 2 - this.ymax;
		this.dc = Math.random() > 0.5 ? 1 : -1;

		this.life = 200 * (this.r / this.rmax);
		this.ratio = Math.random() * this.life;

		this.stop = Math.random() * 0.2 + 0.4;
		this.t = Math.random() + 1;
	}

	/**
	 * Render particle
	 *
	 * @param {Canvas2DContext}
	 */
	render(ctx) {
		const opacity = Math.floor((1 - this.ratio / this.life) * 100) * 0.01;
		const radius = this.r * opacity;
		const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, Math.max(radius, 0.5));

		ctx.beginPath();
		ctx.arc(this.x, this.y, this.r, 0, Math.PI * 2, true);
		ctx.closePath();

		gradient.addColorStop(0.0, 'rgba(255,255,255,' + opacity + ')');
		gradient.addColorStop(this.stop, 'rgba(' + (this.c | 0) + ',100,255,' + opacity * 0.6 + ')');
		gradient.addColorStop(1.0, 'rgba(' + (this.c | 0) + ',100,255,0)');

		ctx.fillStyle = gradient;
		ctx.fill();
	}

	/**
	 * Move particle in a rectangle
	 *
	 * @param {number} time elapsed
	 * @param {number} x
	 * @param {number} y
	 * @param {number} width
	 * @param {number} height
	 */
	update(tick, x, y, width, height) {
		const f = (tick / 60) * 2; // speed / 2

		this.x += Math.cos((this.ratio / this.life) * this.dx * 2) * f;
		this.y += Math.sin((this.ratio / this.life) * this.dy * 2) * f;
		this.c += this.dc * f;

		this.ratio += this.t * f;
		if (this.ratio <= 0 || this.ratio >= this.life) {
			this.t *= -1;
		}

		// Bound
		if (this.x - this.r > width || this.x + this.r < 0 || this.y - this.r > height || this.y + this.r < 0) {
			this.reset(width, height);
		}

		if (this.c >= 255) {
			this.dc = -1;
		} else if (this.c <= 0) {
			this.dc = 1;
		}
	}
}
/**
 * Export class
 */
export default Particle;
