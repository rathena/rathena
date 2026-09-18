/**
 * Renderer/Camera.js
 *
 * Camera class
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import KEYS from 'Controls/KeyEventHandler.js';
import Mouse from 'Controls/MouseEventHandler.js';
import Events from 'Core/Events.js';
import Preferences from 'Preferences/Camera.js';
import Session from 'Engine/SessionStorage.js';
import glMatrix from 'Utils/gl-matrix.js';
import Configs from 'Core/Configs.js';
import DB from 'DB/DBManager.js';

const { mat4, mat3, vec2, vec3 } = glMatrix;
const _position = vec3.create();

/**
 * @const {number} camera min-max constants
 */
const C_MIN_ZOOM = 1;
const C_MAX_ZOOM = 5;

const C_MIN_V_ANGLE_ISOMETRIC = 190;
const C_MAX_V_ANGLE_ISOMETRIC = 270;

const C_THIRDPERSON_TRESHOLD_ZOOM = 1;
const C_MIN_V_ANGLE_3RDPERSON = 175;
const C_MAX_V_ANGLE_3RDPERSON = 270;

const C_MIN_V_ANGLE_1STPERSON = 90;
const C_MAX_V_ANGLE_1STPERSON = 270;

const C_QUAKE_MULT = 0.1;

let _pending = false;

/**
 * Camera Namespace
 */
class Camera {
	/**
	 * Projection matrix
	 * @type {mat4}
	 */
	static projection = mat4.create();

	/**
	 * ModelView matrix
	 * @type {mat4}
	 */
	static modelView = mat4.create();

	/**
	 * Normal matrix
	 * @type {mat3}
	 */
	static normalMat = mat3.create();

	/**
	 * @type {number} zoom
	 */
	static zoom = Preferences.zoom;

	/**
	 * @type {number} zoomFinal
	 */
	static zoomFinal = Preferences.zoom;

	/**
	 * @type {vec2} angle rotation
	 */
	static angle = vec2.create();

	/**
	 * @type {vec2} angle final rotation
	 */
	static angleFinal = vec2.create();

	/**
	 * @type {vec3}
	 */
	static position = vec3.create();

	/**
	 * @type {vec3} point the camera orbits around, in world (mesh) space
	 */
	static focus = vec3.create();

	/**
	 * @type {Entity} Entity currently attached by the camera
	 */
	static target = null;

	/**
	 * @type {number}
	 */
	static lastTick = 0;

	/**
	 * @type {number} camera min-max variables
	 */
	static MIN_ZOOM = C_MIN_ZOOM;
	static MAX_ZOOM = C_MAX_ZOOM;
	static MIN_V_ANGLE = C_MIN_V_ANGLE_ISOMETRIC;
	static MAX_V_ANGLE = C_MAX_V_ANGLE_ISOMETRIC;

	/**
	 * @type {number} Camera direction
	 */
	static direction = 0;
	static altitudeFrom = 0;
	static altitudeTo = -65;
	static altitudeRange = 15;
	static rotationFrom = -360;
	static rotationTo = 360;
	static range = 230; //240;
	static zoomStep = 15;
	static zoomStepMult = 1;

	/**
	 * @type {string} current map
	 */
	static currentMap = '';

	// Indoor Params
	static indoorRotationFrom = -60;
	static indoorRotationTo = -25;
	static indoorRange = 240;

	/**
	 * @type {number} camera zoom indoor
	 */
	static MAX_ZOOM_INDOOR = 2.5;

	/**
	 * @type {number} min camera altitude indoor
	 */
	static MIN_ALTITUDE_INDOOR = 220;

	/**
	 * @type {number} max camera altitude indoor
	 */
	static MAX_ALTITUDE_INDOOR = 240;

	static enable3RDPerson = false;
	static enable1STPerson = false;

	static state = -1;

	static states = {
		isometric: 0,
		third_person: 1,
		first_person: 2
	};

	/**
	 * @type {object} Camera action informations (right click)
	 */
	static action = {
		active: false,
		tick: 0,
		x: 0,
		y: 0
	};

	static quakes = [];

	/**
	 * Save the camera settings
	 */
	static save() {
		// Save camera settings after 3 seconds
		if (!_pending) {
			Events.setTimeout(save, 3000);
			_pending = true;
		}
	}

	/**
	 * Attach player
	 *
	 * @param {object} target - Entity player to attach
	 */
	static setTarget(target) {
		this.target = target;
	}

	/**
	 * Get camera latitude
	 *
	 * @return {number} latitude
	 */
	static getLatitude() {
		return this.angle[0] - 180.0;
	}

	/**
	 * Set screen quake
	 *
	 * @param {number} Start tick
	 * @param {number} Duration
	 * @param {number} X axis amount
	 * @param {number} Y axis amount
	 * @param {number} Z axis amount
	 */
	static setQuake(start, duration, xAmt, yAmt, zAmt) {
		const quake = {};
		quake.startTick = start;
		quake.duration = duration || 650;
		quake.sideQuake = xAmt || 1.0;
		quake.latitudeQuake = yAmt || 0.2;
		quake.zoomQuake = zAmt || 0.24;
		quake.active = true;

		this.quakes.push(quake);
	}

	/**
	 * Set screen quake
	 *
	 * @param {number} Start tick
	 * @param {number} Duration
	 * @param {number} X axis amount
	 * @param {number} Y axis amount
	 * @param {number} Z axis amount
	 */
	static processQuake(tick) {
		for (let i = 0; i < this.quakes.length; i++) {
			if (this.quakes[i].active) {
				if (this.quakes[i].startTick <= tick) {
					if (this.quakes[i].startTick + this.quakes[i].duration > tick) {
						/*var step = (tick - this.quakes[i].startTick) / this.quakes[i].duration;*/ // UNUSED

						this.position[0] +=
							((Math.random() * 5 - 2.5) / 10 + this.quakes[i].sideQuake) *
							Math.cos(this.angle[1] * (Math.PI / 180)) *
							C_QUAKE_MULT;
						this.position[1] +=
							((Math.random() * 5 - 2.5) / 10 + this.quakes[i].sideQuake) *
							-Math.sin(this.angle[1] * (Math.PI / 180)) *
							C_QUAKE_MULT;
						this.quakes[i].sideQuake *= -1;

						this.zoom += ((Math.random() * 5 - 2.5) / 10 + this.quakes[i].zoomQuake) * C_QUAKE_MULT;
						this.quakes[i].zoomQuake *= -1;

						this.angle[0] += ((Math.random() * 5 - 2.5) / 15 + this.quakes[i].latitudeQuake) * C_QUAKE_MULT;
						this.quakes[i].latitudeQuake *= -1;
					} else {
						this.quakes[i].active = false;
					}
				}
			} else {
				this.quakes.splice(i, 1);
			}
		}
	}

	/**
	 * Initialize Camera
	 */
	static init() {
		Camera.enable3RDPerson = Configs.get('ThirdPersonCamera', false);
		Camera.enable1STPerson = Configs.get('FirstPersonCamera', false);
		Camera.MAX_ZOOM = Configs.get('CameraMaxZoomOut', C_MAX_ZOOM);

		this.lastTick = Date.now();

		this.angle[0] = this.range % 360.0; //240.0;
		this.angle[1] = this.rotationFrom % 360.0;
		this.angleFinal[0] = this.range % 360.0;
		this.angleFinal[1] = this.rotationFrom % 360.0;

		this.position[0] = -this.target.position[0];
		this.position[1] = -this.target.position[1];
		this.position[2] = this.target.position[2];

		this.altitudeRange = this.altitudeTo - this.altitudeFrom;

		if (this.enable1STPerson) {
			this.MIN_ZOOM = 0;
		} else if (this.enable3RDPerson) {
			this.MIN_ZOOM = 0.2;
		} else {
			this.MIN_ZOOM = C_MIN_ZOOM;
		}

		// This may cause circular dependency if treated synchronously in ESM.
		// But since this is a method, we assume dependencies are ready later.
		import('Renderer/MapRenderer.js').then(MapRenderer => {
			this.currentMap = MapRenderer.default.currentMap;
			if (DB.isIndoor(this.currentMap)) {
				this.zoomFinal = Preferences.indoorZoom || 125;
				this.angleFinal[0] = 230;
				this.angleFinal[1] = -40;
			} else {
				this.zoomFinal = Preferences.zoom || 125;
			}
		});
	}

	/**
	 * Rotate the camera
	 *
	 * @param {boolean} active - is mouse down ?
	 */
	static rotate(active) {
		const action = this.action;
		const tick = Date.now();

		if (!active) {
			action.active = false;
			return;
		}

		// Check for double click (reset angle and zoom)
		if (
			action.tick + 500 > tick &&
			Math.abs(action.x - Mouse.screen.x) < 10 && // Check the mouse position to avoid bug while rotating
			Math.abs(action.y - Mouse.screen.y) < 10
		) {
			// to fast the camera...

			if (KEYS.SHIFT) {
				if (DB.isIndoor(this.currentMap)) {
					this.angleFinal[0] = +this.indoorRange;
				} else {
					this.angleFinal[0] = +this.range;
				}
			}
			if (KEYS.CTRL) {
				this.zoomFinal = 125.0;
			} else {
				if (DB.isIndoor(this.currentMap)) {
					this.angleFinal[1] = this.indoorRotationTo;
				} else {
					this.angleFinal[1] = 0.0;
				}
			}
		}

		// Save position and tick (for double click)
		action.x = Mouse.screen.x;
		action.y = Mouse.screen.y;
		action.tick = tick;
		action.active = true;
	}

	/**
	 * Process action when right click is down
	 */
	static processMouseAction() {
		// Rotate Z
		if (KEYS.SHIFT) {
			this.angleFinal[0] += ((Mouse.screen.y - this.action.y) / Mouse.screen.height) * 300;
			if (DB.isIndoor(this.currentMap)) {
				this.angleFinal[0] = Math.max(this.angleFinal[0], this.MIN_ALTITUDE_INDOOR);
				this.angleFinal[0] = Math.min(this.angleFinal[0], this.MAX_ALTITUDE_INDOOR);
			} else {
				this.angleFinal[0] = Math.max(this.angleFinal[0], this.MIN_V_ANGLE);
				this.angleFinal[0] = Math.min(this.angleFinal[0], this.MAX_V_ANGLE);
			}
		}

		// Zoom
		else if (KEYS.CTRL) {
			this.zoomFinal -= (Mouse.screen.y - this.action.y) * ((this.zoomStep * this.zoomStepMult) / 10);
			if (DB.isIndoor(this.currentMap)) {
				this.zoomFinal = Math.min(this.zoomFinal, Math.abs(this.altitudeRange) * this.MAX_ZOOM_INDOOR);
			} else {
				this.zoomFinal = Math.min(this.zoomFinal, Math.abs(this.altitudeRange) * this.MAX_ZOOM);
			}
			this.zoomFinal = Math.max(this.zoomFinal, Math.abs(this.altitudeRange) * this.MIN_ZOOM);
		}

		// Rotate
		else {
			this.angleFinal[1] -= ((Mouse.screen.x - this.action.x) / Mouse.screen.width) * 720;

			if (this.angle[1] > 180 && this.angleFinal[1] > 180) {
				this.angle[1] -= 360;
				this.angleFinal[1] -= 360;
			} else if (this.angle[1] < -180 && this.angleFinal[1]) {
				this.angle[1] += 360;
				this.angleFinal[1] += 360;
			}

			if (DB.isIndoor(this.currentMap)) {
				this.angleFinal[1] = Math.max(this.angleFinal[1], this.indoorRotationFrom);
				this.angleFinal[1] = Math.min(this.angleFinal[1], this.indoorRotationTo);
			} else {
				this.angleFinal[1] = Math.max(this.angleFinal[1], this.rotationFrom);
				this.angleFinal[1] = Math.min(this.angleFinal[1], this.rotationTo);
			}

			if (this.state == this.states.first_person || this.state == this.states.third_person) {
				this.angleFinal[0] += ((Mouse.screen.y - this.action.y) / Mouse.screen.height) * 300;
				this.angleFinal[0] = Math.max(this.angleFinal[0], this.MIN_V_ANGLE);
				this.angleFinal[0] = Math.min(this.angleFinal[0], this.MAX_V_ANGLE);
			}
		}

		// Update last check
		this.action.x = +Mouse.screen.x;
		this.action.y = +Mouse.screen.y;
		this.updateState();
		this.save();
	}

	/**
	 * Process a MouseWheel, zoom.
	 *
	 * @param {number} delta (zoom)
	 */
	static setZoom(delta) {
		if (delta) {
			this.zoomFinal += delta * this.zoomStep * this.zoomStepMult;
			if (DB.isIndoor(this.currentMap)) {
				this.zoomFinal = Math.min(this.zoomFinal, Math.abs(this.altitudeRange) * this.MAX_ZOOM_INDOOR);
			} else {
				this.zoomFinal = Math.min(this.zoomFinal, Math.abs(this.altitudeRange) * this.MAX_ZOOM);
			}

			this.zoomFinal = Math.max(this.zoomFinal, Math.abs(this.altitudeRange) * this.MIN_ZOOM);
			this.updateState();
			this.save();
		}
	}

	static updateState() {
		if (this.enable1STPerson && this.zoomFinal == 0) {
			if (this.state != this.states.first_person) {
				this.MIN_V_ANGLE = C_MIN_V_ANGLE_1STPERSON;
				this.MAX_V_ANGLE = C_MAX_V_ANGLE_1STPERSON;
				this.zoomStepMult = 0.3;
				this.state = this.states.first_person;
				if (Session.Entity) {
					Session.Entity.hideEntity = true;
				}
				import('Renderer/Renderer.js').then(Renderer => {
					Renderer.default.vFov = 50;
					Renderer.default.resize();
				});
			}
		} else if (
			this.enable3RDPerson &&
			this.zoomFinal < Math.abs(this.altitudeRange) * C_THIRDPERSON_TRESHOLD_ZOOM
		) {
			if (this.state != this.states.third_person) {
				this.MIN_V_ANGLE = C_MIN_V_ANGLE_3RDPERSON;
				this.MAX_V_ANGLE = C_MAX_V_ANGLE_3RDPERSON;
				this.zoomStepMult = 0.3;
				this.state = this.states.third_person;
				if (Session.Entity) {
					Session.Entity.hideEntity = false;
				}
				import('Renderer/Renderer.js').then(Renderer => {
					Renderer.default.vFov = 30;
					Renderer.default.resize();
				});
			}
		} else {
			if (this.state != this.states.isometric) {
				this.MIN_V_ANGLE = C_MIN_V_ANGLE_ISOMETRIC;
				this.MAX_V_ANGLE = C_MAX_V_ANGLE_ISOMETRIC;
				this.zoomStepMult = 1;
				this.state = this.states.isometric;
				if (Session.Entity) {
					Session.Entity.hideEntity = false;
				}
				import('Renderer/Renderer.js').then(Renderer => {
					Renderer.default.vFov = 15;
					Renderer.default.resize();
				});
			}
		}
	}

	/**
	 * Update the camera
	 *
	 * @param {number} tick
	 */
	static update(tick) {
		const lerp = Math.min((tick - this.lastTick) * 0.006, 1.0);
		this.lastTick = tick;

		// Update camera from mouse movement
		if (this.action.x !== -1 && this.action.y !== -1 && this.action.active) {
			this.processMouseAction();
		}

		// Screen quake
		this.processQuake(tick);

		// Move Camera
		if (Preferences.smooth && this.state != this.states.first_person) {
			this.position[0] += (-this.target.position[0] - this.position[0]) * lerp;
			this.position[1] += (-this.target.position[1] - this.position[1]) * lerp;
			this.position[2] += (this.target.position[2] - this.position[2]) * lerp;
		} else {
			this.position[0] = -this.target.position[0];
			this.position[1] = -this.target.position[1];
			this.position[2] = this.target.position[2];
		}

		// Zoom
		this.zoom += (this.zoomFinal - this.zoom) * lerp * 2.0;

		let zOffset = 0;
		if (this.state == this.states.first_person) {
			zOffset = 2;
		} else if (
			this.state == this.states.third_person &&
			this.zoomFinal < Math.abs(this.altitudeRange) * C_THIRDPERSON_TRESHOLD_ZOOM
		) {
			zOffset = 1.5;
		}

		// Angle
		this.angle[0] += (this.angleFinal[0] - this.angle[0]) * lerp * 2.0;
		this.angle[1] += (this.angleFinal[1] - this.angle[1]) * lerp * 2.0;
		this.angle[0] %= 360;
		this.angle[1] %= 360;

		// Find Camera direction (for NPC direction)
		this.direction = Math.floor((this.angle[1] + 22.5) / 45) % 8;

		// Calculate new modelView mat
		const matrix = this.modelView;
		mat4.identity(matrix);
		mat4.translateZ(matrix, (this.altitudeFrom - this.zoom) / 2);
		mat4.rotateX(matrix, matrix, (this.angle[0] / 180) * Math.PI);
		mat4.rotateY(matrix, matrix, (this.angle[1] / 180) * Math.PI);

		// Center of the cell and inversed Y-Z axis
		_position[0] = this.position[0] - 0.5;
		_position[1] = this.position[2] + zOffset;
		_position[2] = this.position[1] - 0.5;
		mat4.translate(matrix, matrix, _position);
		vec3.negate(this.focus, _position);

		mat4.toInverseMat3(matrix, this.normalMat);
		mat3.transpose(this.normalMat, this.normalMat);
	}
}

function save() {
	_pending = false;
	if (!DB.isIndoor(Camera.currentMap)) {
		Preferences.zoom = Camera.zoomFinal;
	} else {
		Preferences.indoorZoom = Camera.zoomFinal;
	}
	Preferences.save();
}

/**
 * Export
 */
export default Camera;
