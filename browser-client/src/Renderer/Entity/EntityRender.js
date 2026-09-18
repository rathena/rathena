/**
 * Renderer/EntityRender.js
 *
 * Entity rendering pipeline — WebGL sprite rendering, action animation frame
 * calculation, picking, shadow rendering, palette binding, and 3D GR2 model attachments.
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

/**
 * EntityRender mixin
 * Adds rendering methods (render, renderEntityGUI, renderShadow, isVisible, depth) to Entity.
 *
 * @mixin EntityRender
 * @property {function(mat4, mat4): void} render Render WebGL entity sprite / 3D model
 * @property {function(mat4, mat4): void} renderEntityGUI Render entity health bar, speech bubble, and nameplate
 * @property {function(mat4, mat4): void} renderShadow Render entity ground shadow
 * @property {function(): boolean} isVisible Check if entity is currently visible
 */

import glMatrix from 'Utils/gl-matrix.js';
import Camera from 'Renderer/Camera.js';
import Client from 'Core/Client.js';
import StatusConst from 'DB/Status/StatusState.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import Ground from 'Renderer/Map/Ground.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Water from 'Renderer/Map/Water.js';
import Session from 'Engine/SessionStorage.js';
import DB from 'DB/DBManager.js';
import GraphicsSettings from 'Preferences/Graphics.js';
import GR2ModelRenderer from 'Renderer/GR2/GR2ModelRenderer.js';

/**
 * Load dependencies
 */
// Official client uses: floor(dist * 0.37 * 4 / motionSpeed) % motionCount
// where dist is in the client's internal move units (not map-cell units).
// In roBrowser, `entity.walk.dist` is accumulated in *map cells*.
// We scale cells -> client-like units so the stride cadence matches.
//
// roBrowser stores .act delays in milliseconds (loader multiplies by 25ms ticks),
// while the official formula uses delays in 25ms ticks. Converting gives:
// frame = floor(distCells * stepSize * 0.37 * 4 * 25 / actDelayMs)
// stepSize ≈ 4.6 maps cell-units to the official internal move units.
const WALK_STEP_SIZE = 4.6;
const WALK_DIST_TO_MOTION = WALK_STEP_SIZE * 0.37 * 4 * 25; // ≈ 170.2

/**
 * Render an Entity
 *
 * @param {mat4} modelView
 * @param {mat4} projection
 */
function render(modelView, projection) {
	// GR2 3D body: attach (or re-attach on a body change) an instance the renderer draws in
	// the map pass. The billboard body + shadow are suppressed below while this.gr2 is set.
	// `path` and `this.gr2` are both path strings; JS `!==` on strings is value equality
	// (string interning is irrelevant), so this correctly fires only when the model file
	// actually changed -- detach the old instance and let the block below attach the new one.
	if (this.gr2Model && this.gr2Model.path !== this.gr2) {
		GR2ModelRenderer.detach(this.gr2Model);
		this.gr2Model = null;
	}
	if (this.gr2 && !this.gr2Model) {
		this.gr2Model = GR2ModelRenderer.attach(this);
	}
	// A supported model whose file 404'd or decode threw is now flagged missing: drop the (invisible)
	// 3D path and re-resolve the body, which falls back to the Poring sprite via isMissing in
	// UpdateBody. Runs once -- UpdateBody clears this.gr2, so the guard is false next frame.
	if (this.gr2 && GR2ModelRenderer.isMissing(this.gr2)) {
		if (this.gr2Model) {
			GR2ModelRenderer.detach(this.gr2Model);
			this.gr2Model = null;
		}
		this.job = this._job;
	}

	// Process action
	this.animations.process();

	// Always process walk. It will decide it for itself if it is walking or not and handles it accordingly.
	this.walkProcess();
	this.entitiesWalkProcess(); // falcon/wug

	this.boundingRect.x1 = Infinity;
	this.boundingRect.y1 = -Infinity;
	this.boundingRect.x2 = -Infinity;
	this.boundingRect.y2 = Infinity;

	// Render it only if visible
	if (this.effectColor[3]) {
		this.renderEntity();
		this.attachments.render(Date.now());
	}

	// Update character UI (life, dialog, etc.)
	renderGUI(this, modelView, projection);
}

/**
 * Calculate zIndex and render UI elements
 *
 * @param {Entity}
 * @param {mat4} modelView
 * @param {mat4} projection
 */
const renderGUI = (function renderGUIClosure() {
	const mat4 = glMatrix.mat4;
	const vec4 = glMatrix.vec4;
	const _matrix = mat4.create();
	const _vector = vec4.create();
	const _pickMatrix = mat4.create();

	return function _renderGUI(entity, modelView, projection) {
		// Move to camera
		_vector[0] = entity.position[0] + 0.5;
		_vector[1] = -entity.position[2];
		_vector[2] = entity.position[1] + 0.5;
		mat4.translate(_matrix, modelView, _vector);

		// Set-up Spherical billboard
		_matrix[0] = 1.0;
		_matrix[1] = 0.0;
		_matrix[2] = 0.0;
		_matrix[4] = 0.0;
		_matrix[5] = 1.0;
		_matrix[6] = 0.0;
		_matrix[8] = 0.0;
		_matrix[9] = 0.0;
		_matrix[10] = 1.0;

		// Project to screen
		mat4.multiply(_matrix, projection, _matrix);

		if (entity.effectColor[3] && entity._job !== 139) {
			// Same billboard as above, lifted the way the sprite itself is.
			_vector[0] = entity.position[0] + 0.5;
			_vector[1] = -(entity.position[2] + SPRITE_LIFT);
			_vector[2] = entity.position[1] + 0.5;
			mat4.translate(_pickMatrix, modelView, _vector);
			_pickMatrix[0] = 1.0;
			_pickMatrix[1] = 0.0;
			_pickMatrix[2] = 0.0;
			_pickMatrix[4] = 0.0;
			_pickMatrix[5] = 1.0;
			_pickMatrix[6] = 0.0;
			_pickMatrix[8] = 0.0;
			_pickMatrix[9] = 0.0;
			_pickMatrix[10] = 1.0;
			mat4.multiply(_pickMatrix, projection, _pickMatrix);
			calculateBoundingRect(entity, _pickMatrix);
		}

		// Get depth for rendering order
		_vector[0] = 0.0;
		_vector[1] = 0.0;
		_vector[2] = 0.0;
		_vector[3] = 1.0;

		vec4.transformMat4(_vector, _vector, _matrix);
		entity.depth = _vector[3];

		// Display UI
		if (entity.life.display) {
			entity.life.render(_matrix);
		}
		if (entity.emblem.display) {
			entity.emblem.render(_matrix);
		}
		if (entity.display.display) {
			entity.display.render(_matrix);
		}
		if (entity.dialog.display) {
			entity.dialog.render(_matrix);
		}
		if (entity.cast.display) {
			entity.cast.render(_matrix);
		}
		if (entity.room.display) {
			entity.room.render(_matrix);
		}
	};
})();

/**
 * Calculate Bounding Rectangle
 *
 * @param {Entity}
 * @param {mat4}
 */
/**
 * Vertical lift renderEntity applies to every sprite before drawing it.
 * The picking rectangle has to use it too, or it lands below the sprite.
 */
const SPRITE_LIFT = 0.2;

const calculateBoundingRect = (function calculateBoundingRectClosure() {
	const vec4 = glMatrix.vec4;
	const size = glMatrix.vec2.create();
	const vector = vec4.create();
	const out = vec4.create();

	function projectPoint(x, y, matrix) {
		vector[0] = x;
		vector[1] = y;
		vector[2] = 0.0;
		vector[3] = 1.0;

		vec4.transformMat4(out, vector, matrix);

		out[3] = out[3] === 0.0 ? 1.0 : 1.0 / out[3];
		out[0] *= out[3];
		out[1] *= out[3];
	}

	return function _calculateBoundingRect(entity, matrix) {
		let tmp;

		const fx = entity.xSize / 175;
		const fy = entity.ySize / 175;

		size[0] = window.innerWidth * 0.5;
		size[1] = window.innerHeight * 0.5;

		const rect = entity.boundingRect;
		const minSize = entity.objecttype === entity.constructor.TYPE_ITEM ? 30 : 60;

		// GR2 3D body: renderEntity early-returns for GR2, so no sprite body accumulates a rect.
		// Use the screen-space box the GR2 renderer projected from the model AABB this frame so the
		// hover/click test matches the visible model instead of the tiny default sprite box.
		const gr2Model = entity.gr2Model;
		if (gr2Model && gr2Model.screenRect) {
			const sr = gr2Model.screenRect;
			rect.x1 = sr.x1;
			rect.y1 = sr.y1;
			rect.x2 = sr.x2;
			rect.y2 = sr.y2;
			if (rect.x2 - rect.x1 < minSize) {
				rect.x1 = (rect.x1 + rect.x2) * 0.5 - minSize * 0.5;
				rect.x2 = rect.x1 + minSize;
			}
			if (rect.y2 - rect.y1 < minSize) {
				rect.y1 = (rect.y1 + rect.y2) * 0.5 - minSize * 0.5;
				rect.y2 = rect.y1 + minSize;
			}
			return;
		}

		// No body ? Default picking (sprite 110 for example)
		if (rect.x1 === Infinity || rect.x2 === -Infinity || rect.y1 === -Infinity || rect.y2 === Infinity) {
			rect.x1 = -25;
			rect.x2 = +25;
			rect.y1 = +45;
			rect.y2 = 0;
		}

		// Swap x1 and x2 if needed
		if (rect.x1 > rect.x2) {
			tmp = rect.x1;
			rect.x1 = rect.x2;
			rect.x2 = tmp;
		}

		// Top left
		projectPoint(rect.x1 * fx, rect.y1 * fy, matrix);
		rect.x1 = size[0] + size[0] * out[0];
		rect.y1 = size[1] - size[1] * out[1];

		// Bottom right
		projectPoint(rect.x2 * fx, rect.y2 * fy, matrix);
		rect.x2 = size[0] + size[0] * out[0];
		rect.y2 = size[1] - size[1] * out[1];

		// Cap it to minSize
		if (rect.x2 - rect.x1 < minSize) {
			rect.x1 = (rect.x1 + rect.x2) * 0.5 - minSize * 0.5;
			rect.x2 = rect.x1 + minSize;
		}

		if (rect.y2 - rect.y1 < minSize) {
			rect.y1 = (rect.y1 + rect.y2) * 0.5 - minSize * 0.5;
			rect.y2 = rect.y1 + minSize;
		}
	};
})();

/**
 * Render Entity
 */
const renderEntity = (function renderEntityClosure() {
	const _position = new Int32Array(2);

	return function _renderEntity() {
		if (this.hideEntity) {
			return;
		}

		// Update shadow
		SpriteRenderer.shadow = Ground.getShadowFactor(this.position[0], this.position[1]);
		SpriteRenderer.zIndex = 1;

		const animation = this.animation;
		const Entity = this.constructor;
		_position[0] = 0;
		_position[1] = 0;

		// Animation change ! Get it now
		if (animation.save && animation.delay < Date.now()) {
			this.setAction(animation.save);
		}

		// GR2 3D body: the model is drawn by GR2ModelRenderer in the map pass. Suppress the 2D
		// billboard body AND the shadow blob (Phase 0: no blob under a GR2). Name / lifebar /
		// emblem overlays render in renderGUI (called separately) and are kept.
		// Returning here also skips attachments.renderBefore() below -- intentional: the current
		// GR2 set (guardians/emperium/flag/treasure) carries no sprite attachments. attachments.render()
		// (the always-behind pass, called from the map-pass entry above) still runs on the parent.
		if (this.gr2) {
			return;
		}

		// Avoid look up, render as IDLE all not supported frames
		const action = this.action < 0 ? this.ACTION.IDLE : this.action;
		const direction = (Camera.direction + this.direction + 8) % 8;
		const behind = direction > 1 && direction < 6;
		const self = this;

		// Render shadow (shadow isn't render when player is sit or dead).
		if (action !== this.ACTION.DIE && action !== this.ACTION.SIT && this.job !== 45 && !this.hideShadow) {
			SpriteRenderer.zIndex = -1;
			// Shadow is base on gat height
			SpriteRenderer.position[0] = this.position[0];
			SpriteRenderer.position[1] = this.position[1];
			SpriteRenderer.position[2] = Altitude.getCellHeight(this.position[0], this.position[1]) + 0.05;

			// Keep shadow on ground: depth test on, depth write off, disabledepthcorrection
			SpriteRenderer.runWithDepth(true, false, false, function () {
				renderElement(self, self.files.shadow, 'shadow', _position, false);
			});
		}

		SpriteRenderer.position.set(self.position);
		// Render attachments that should appear behind the entity first
		this.attachments.renderBefore(Date.now());
		SpriteRenderer.zIndex = 1;

		// ------------------------------------------------------------------
		// ENTITY RENDER PIPELINE
		//
		// - Write depth (3D world occluders)
		// - Use depth correction (isometric projection)
		// - Use zIndex only for same-entity layering
		//
		// ------------------------------------------------------------------
		switch (self.objecttype) {
			case Entity.TYPE_PC:
			case Entity.TYPE_MERC:
				SpriteRenderer.position[2] = SpriteRenderer.position[2] + 0.2;

				// Main sprite pass:
				// depthTest  = resolve world occlusion
				// depthWrite = allow entity to occlude others
				// depthCorrection ENABLED (required for isometric depth)
				SpriteRenderer.runWithDepth(true, true, false, function () {
					let cartidx;
					// madogear don't appear costumes
					const RIDING_STATUS =
						self.effectState &
							(StatusConst.EffectState.RIDING |
								StatusConst.EffectState.DRAGON1 |
								StatusConst.EffectState.DRAGON2 |
								StatusConst.EffectState.DRAGON3 |
								StatusConst.EffectState.DRAGON4 |
								StatusConst.EffectState.DRAGON5 |
								StatusConst.EffectState.WUGRIDER) || self.allRidingState;

					function robeCorrection(lookingFront) {
						if (self.robe > 0 && self.robeHeight && self.bodyHeight) {
							const HEAD_SIZE = 64;
							const COMPENSATION = 25;
							if (
								self.robeHeight +
									(self.action === self.ACTION.SIT
										? 0
										: RIDING_STATUS && lookingFront
											? COMPENSATION * 2
											: COMPENSATION) >
								self.bodyHeight + HEAD_SIZE
							) {
								if (self.action === self.ACTION.SIT) {
									return lookingFront ? -450 : RIDING_STATUS ? 1 : -100;
								}
								return lookingFront ? -300 : RIDING_STATUS ? 100 : 1;
							}
						}
						if (self.action === self.ACTION.SIT) {
							return lookingFront ? -100 : 550;
						}
						return lookingFront ? 1 : 400;
					}

					// Shield is behind on some position, seems to be hardcoded by the client
					if (self.shield && behind) {
						SpriteRenderer.runWithDepth(true, false, false, function () {
							SpriteRenderer.zIndex = -1;
							renderElement(self, self.files.shield, 'shield', _position, true);
						});
					}

					if (!(direction > 2 && direction < 6)) {
						// looking front
						if (Session.Playing == true && self.hasCart == true) {
							cartidx = DB.isSuperNovice(self._job) ? 0 : self.CartNum;

							// Draw Cart
							SpriteRenderer.runWithDepth(true, false, false, function () {
								SpriteRenderer.zIndex = -1;
								renderElement(self, self.files.cart_shadow, 'cartshadow', _position, false);
								SpriteRenderer.zIndex = 1;
								renderElement(self, self.files.cart[cartidx], 'cart', _position, false);
							});
						}

						// Draw Robe
						if (self.robe > 0) {
							SpriteRenderer.zIndex = robeCorrection(true);
							renderElement(self, self.files.robe, 'robe', _position, true);
						}
					}

					SpriteRenderer.zIndex = 150;
					if (
						RIDING_STATUS &&
						!(direction > 2 && direction < 6) &&
						(self.action === self.ACTION.SIT || self.action === self.ACTION.WALK)
					) {
						SpriteRenderer.zIndex *= 2;
					}

					// Draw Body
					renderElement(self, self.files.body, 'body', _position, true);

					// Isometric Projection Body Offset
					let bodyZOffset = 250;
					if (RIDING_STATUS) {
						bodyZOffset *= 2;
						if (self.action === self.ACTION.WALK && !(direction > 2 && direction < 6)) {
							bodyZOffset += 250;
						}
					}

					SpriteRenderer.zIndex = bodyZOffset + 50;

					// Draw Head
					renderElement(self, self.files.head, 'head', _position, false);

					// TB_Layer_Priority = {Default_Mid = 100, Default_Top = 200, Default_Bottom = 300, Default_Robe = 400 }
					// Values*10 reference https://github.com/zhad3/zrenderer/blob/c10b337dfb9d44e33b146551191b3398630823b5/source/sprite.d#L84

					// Hat Middle
					if (self.accessory3 > 0 && self.accessory3 !== self.accessory) {
						// accessory already rendered, avoid render same item again
						SpriteRenderer.zIndex = bodyZOffset + 100;
						renderElement(self, self.files.accessory3, 'head', _position, false);
					}

					// Hat Top
					if (
						self.accessory2 > 0 &&
						self.accessory2 !== self.accessory &&
						self.accessory2 !== self.accessory3
					) {
						// accessory and accessory3 already rendered, avoid render same item again
						SpriteRenderer.zIndex = bodyZOffset + 200;
						renderElement(self, self.files.accessory2, 'head', _position, false);
					}

					// Hat Bottom
					if (self.accessory > 0) {
						SpriteRenderer.zIndex = bodyZOffset + 300;
						renderElement(self, self.files.accessory, 'head', _position, false);
					}

					if (direction > 2 && direction < 6) {
						// looking back

						// Draw Robe
						if (self.robe > 0) {
							SpriteRenderer.zIndex = bodyZOffset + robeCorrection(false);
							renderElement(self, self.files.robe, 'robe', _position, true);
						}

						// Draw Cart
						if (Session.Playing == true && self.hasCart == true) {
							SpriteRenderer.zIndex = bodyZOffset + 500;
							cartidx = DB.isSuperNovice(self._job) ? 0 : self.CartNum;
							SpriteRenderer.runWithDepth(true, false, false, function () {
								renderElement(self, self.files.cart_shadow, 'cartshadow', _position, false);
								renderElement(self, self.files.cart[cartidx], 'cart', _position, false);
							});
						}
					}

					// Draw Weapon
					if (self.weapon > 0) {
						SpriteRenderer.zIndex = bodyZOffset + 250;
						renderElement(self, self.files.weapon, 'weapon', _position, true);
						renderElement(self, self.files.weapon_trail, 'weapon_trail', _position, true);
					}

					if (self.shield > 0 && !behind) {
						SpriteRenderer.zIndex = bodyZOffset + 300;
						renderElement(self, self.files.shield, 'shield', _position, true);
					}
				});
				break;
			case Entity.TYPE_HOM:
				SpriteRenderer.position[2] = SpriteRenderer.position[2] + 0.2;
				SpriteRenderer.zIndex = 150;
				SpriteRenderer.runWithDepth(true, true, false, function () {
					renderElement(self, self.files.body, 'body', _position, true);
				});
				break;
			case Entity.TYPE_FALCON:
				SpriteRenderer.position[2] = SpriteRenderer.position[2] + 0.2;
				SpriteRenderer.zIndex = 1000;
				SpriteRenderer.runWithDepth(true, true, false, function () {
					renderElement(self, self.files.body, 'body', _position, true);
				});
				break;
			default:
				SpriteRenderer.position[2] = SpriteRenderer.position[2] + 0.2;
				SpriteRenderer.zIndex = 150;
				// Non-player entities:
				// - Do not write depth to avoid breaking PC occlusion and internal layer issues
				// - Still use depth test for correct ordering
				// (submerged bodies get their depth from renderWaterDepth, just before the water pass)
				self.waterDepthFrame = null;
				SpriteRenderer.runWithDepth(true, false, false, function () {
					renderElement(self, self.files.body, 'body', _position, true);
				});
				break;
		}
		SpriteRenderer.zIndex = 1;
	};
})();

/**
 * Depth-only redraw of the body for entities standing in water, so the water
 * pass (drawn after entities, depth tested) covers only the submerged part.
 * Runs after every entity has been drawn, with colour writes disabled by the
 * caller, so the written depth cannot hide other sprites. Replays the exact
 * layers the colour pass drew this frame (`waterDepthFrame`), so no animation,
 * sound or trail state is touched. Only set for the non-player body pass;
 * entity types that already write depth never get a frame.
 */
function renderWaterDepth() {
	const frame = this.waterDepthFrame;

	if (!frame || this.hideEntity || !this.effectColor[3]) {
		return;
	}

	if (!Water.isSubmerged(this.position[0], this.position[1])) {
		return;
	}

	const self = this;
	SpriteRenderer.position.set(this.position);
	SpriteRenderer.position[2] = SpriteRenderer.position[2] + 0.2;
	SpriteRenderer.zIndex = 150;
	SpriteRenderer.runWithDepth(true, true, false, function () {
		for (let i = 0, count = frame.layers.length; i < count; ++i) {
			self.renderLayer(frame.layers[i], frame.spr, frame.pal, frame.size, frame.position, 'body', false);
		}
	});
	SpriteRenderer.zIndex = 1;
}

/**
 * Render second body (BL_DOUBLE_BODY + EF_MAKEBLUR)
 * @param {Entity} entity
 * @param {Array} layers
 * @param {string} spr
 * @param {string} pal
 * @param {object} files
 * @param {string} type
 * @param {number[]} _position
 * @param {object} options
 */
function renderSecondBody(entity, layers, spr, pal, files, type, _position, options = {}) {
	// options:
	// - options.enableHalo: boolean  -> halo type Assumptio (BL_DOUBLE_BODY)
	// - options.enableTrail: boolean -> trail behind the character (EF_MAKEBLUR)
	// - options.trailLength: number  -> number of ghosts to keep
	// - options.blurType: number     -> 1 (standard) EF_MAKEBLUR , 3 (spaced) - EF_MAKEBLUR3, 4 (once), 5 (attack only) - EF_MAKEBLUR5
	const { enableHalo = false, enableTrail = false, trailLength = 5, blurType = 1 } = options;

	if (!enableHalo && !enableTrail) {
		return;
	}

	if (type === 'shadow' || type === 'cartshadow') {
		return;
	}

	// -------------------------
	// 1) Halo (second body) - Assumptio
	// -------------------------
	if (enableHalo) {
		const originalR = entity.effectColor[0];
		const originalG = entity.effectColor[1];
		const originalB = entity.effectColor[2];
		const originalA = entity.effectColor[3];

		entity.effectColor[0] = 1.0;
		entity.effectColor[1] = 1.0;
		entity.effectColor[2] = 1.0;
		entity.effectColor[3] = 0.6;

		// Bigger, visible contour
		const haloScale = 1.15;
		const zIndex = SpriteRenderer.zIndex;
		SpriteRenderer.zIndex = zIndex - 100;

		SpriteRenderer.runWithDepth(true, false, false, function () {
			for (let i = 0; i < layers.length; ++i) {
				const oldX = layers[i].scale[0];
				const oldY = layers[i].scale[1];
				layers[i].scale[0] = oldX * haloScale;
				layers[i].scale[1] = oldY * haloScale;
				entity.renderLayer(layers[i], spr, pal, files.size, _position, type, true);
				layers[i].scale[0] = oldX;
				layers[i].scale[1] = oldY;
			}
		});

		SpriteRenderer.zIndex = zIndex;

		// restore
		entity.effectColor[0] = originalR;
		entity.effectColor[1] = originalG;
		entity.effectColor[2] = originalB;
		entity.effectColor[3] = originalA;
	}

	// -------------------------
	// 2) Trail - After-images (MakeBlur variants)
	// -------------------------
	if (enableTrail) {
		// Initialize trail storage for each element type
		if (!entity._trailData) {
			entity._trailData = {};
		}
		if (!entity._trailData[type]) {
			entity._trailData[type] = {
				snapshots: [],
				lastTick: 0
			};
		}

		const trail = entity._trailData[type];
		const now = Date.now();

		// Determine blur type: 1 (standard), 3 (10f), 4 (once), 5 (10f, attack only)
		const interval = entity.isFastMoving ? 30 : blurType === 3 || blurType === 5 ? 560 : 80; // Fast moves capture at 30ms interval
		const maxLen =
			blurType === 4 ? 1 : GraphicsSettings.performanceMode ? Math.floor(trailLength / 2) : trailLength;

		let shouldCapture = false;

		// Snapshot logic
		if (blurType === 1 || blurType === 3) {
			shouldCapture =
				(entity.action === entity.ACTION.WALK || entity.isFastMoving) && now - trail.lastTick > interval;
		} else if (blurType === 4) {
			shouldCapture = trail.snapshots.length === 0;
		} else if (blurType === 5) {
			const isCombat = [
				entity.ACTION.ATTACK,
				entity.ACTION.ATTACK1,
				entity.ACTION.ATTACK2,
				entity.ACTION.ATTACK3,
				entity.ACTION.SKILL
			].includes(entity.action);
			shouldCapture = (isCombat || entity.isFastMoving) && now - trail.lastTick > interval;
		}

		if (shouldCapture) {
			trail.snapshots.unshift({
				position: glMatrix.vec3.clone(entity.position),
				tick: now,
				layers: layers, // Reference current animation frames
				spr: spr,
				pal: pal,
				_position: new Int32Array(_position),
				color: [...entity.effectColor]
			});
			trail.lastTick = now;

			if (trail.snapshots.length > maxLen) {
				trail.snapshots.pop();
			}
		}

		if (trail.snapshots.length) {
			const originalPos = glMatrix.vec3.clone(SpriteRenderer.position);
			const originalZ = SpriteRenderer.zIndex;
			const originalColor = new Float32Array(entity.effectColor);

			SpriteRenderer.runWithDepth(true, false, false, function () {
				const duration = blurType === 4 || blurType === 3 ? 800 : 400; // Static image lasts longer
				for (let idx = 0; idx < trail.snapshots.length; idx++) {
					const snp = trail.snapshots[idx];
					const age = now - snp.tick;

					if (age > duration && blurType !== 4) {
						continue;
					}

					let alpha = Math.max(0, 0.4 * (1.0 - age / duration));
					if (alpha <= 0.05 && blurType !== 4) {
						continue;
					}
					if (blurType === 3) alpha *= 2;

					// Set Colors based on mode
					if (blurType !== 1) {
						// MakeBlur3/4/5: White (255, 255, 255)
						entity.effectColor[0] = 1.0;
						entity.effectColor[1] = 1.0;
						entity.effectColor[2] = 1.0;
					}
					entity.effectColor[3] = alpha;

					// Move SpriteRenderer base to ghost world position
					SpriteRenderer.position.set(snp.position);
					SpriteRenderer.zIndex = originalZ - 10 - idx;

					// Render captured layers
					for (let i = 0; i < snp.layers.length; ++i) {
						entity.renderLayer(snp.layers[i], snp.spr, snp.pal, files.size, snp._position, type, false);
					}
				}
			});

			// Restore global/entity state
			SpriteRenderer.position.set(originalPos);
			SpriteRenderer.zIndex = originalZ;
			entity.effectColor.set(originalColor);
		}
	}
}

/**
 * Render each Entity elements (body, head, hats, ...)
 *
 * @param {Entity}
 * @param {object} files {spr, act, pal}
 * @param {string} type
 * @param {vec2}   position (reference)
 * @param {boolean} is_main - true if it's the main element (body)
 */
const renderElement = (function renderElementClosure() {
	const _position = new Int32Array(2);

	return function _renderElement(entity, files, type, position, is_main) {
		let isBlendModeOne = false;

		// Nothing to render
		if (typeof files === 'undefined' || !files.spr || !files.act) {
			return;
		}

		// Get back sprite and act
		const spr = Client.loadFile(files.spr);
		const act = Client.loadFile(files.act);

		// Not loaded yet
		if (!spr || !act) {
			return;
		}

		// If palette, load palette, else get back sprite palette
		const pal = (files.pal && Client.loadFile(files.pal)) || spr;

		// Obtain animations from the action and direction.
		const action =
			act.actions[
				(entity.action * 8 + // Action
					((Camera.direction + entity.direction + 8) % 8)) % // Direction
					act.actions.length
			]; // Avoid overflow on action (ex: if there is just one action)

		// Find animation
		const animation_id = calcAnimation(entity, action, type, Date.now() - entity.animation.tick);
		const animation = action.animations[animation_id];
		const layers = animation.layers;

		// Play sound
		if (animation.sound > -1) {
			entity.sound.play(act.sounds[animation.sound], entity.action, animation_id);
		}

		_position[0] = 0;
		_position[1] = 0;

		if (animation.pos.length && !is_main) {
			_position[0] = position[0] - animation.pos[0].x;
			_position[1] = position[1] - animation.pos[0].y;
		}

		if (type === 'cart' || type === 'cartshadow') {
			const direction = (Camera.direction + entity.direction + 8) % 8;

			switch (direction) {
				case 0:
					{
						_position[0] = 0;
						_position[1] = -30;
					}
					break;
				case 1:
					_position[0] = 30;
					_position[1] = -10;
					break;
				case 2:
					_position[0] = 40;
					_position[1] = 0;
					break;
				case 3:
					_position[0] = 30;
					_position[1] = 10;
					break;
				case 4:
					{
						_position[0] = 0;
						_position[1] = 20;
					}
					break;
				case 5:
					_position[0] = -30;
					_position[1] = 10;
					break;
				case 6:
					_position[0] = -40;
					_position[1] = 0;
					break;
				case 7:
					_position[0] = -30;
					_position[1] = -10;
					break;
			}
		}

		// Check if special body effect and enable the correct blend mode
		const hasBerserkOrMarionette =
			(type !== 'shadow' && entity.getOpt3(StatusConst.Status.BERSERK)) ||
			entity.getOpt3(StatusConst.Status.MARIONETTE);

		if (hasBerserkOrMarionette) {
			isBlendModeOne = true;
		}

		// Trail / Blur variations
		const isBUNSIN = entity.getOpt3(StatusConst.Status.NJ_BUNSINJYUTSU);
		const isHALLUCINATIONWALK = entity.getOpt3(StatusConst.Status.HALLUCINATIONWALK);
		const isENERGYCOAT = entity.getOpt3(StatusConst.Status.ENERGYCOAT);
		const isQUICKEN = entity.virtue & StatusConst.OPT3.QUICKEN;
		const isOVERTHRUST = entity.getOpt3(StatusConst.Status.OVERTHRUST);
		const isEXPLOSIONSPIRITS = entity.getOpt3(StatusConst.Status.EXPLOSIONSPIRITS);
		const isBERSERK = entity.getOpt3(StatusConst.Status.BERSERK);

		renderSecondBody(entity, layers, spr, pal, files, type, _position, {
			enableHalo: entity.getOpt3(StatusConst.Status.ASSUMPTIO) || !!entity._enableHalo,
			enableTrail:
				isENERGYCOAT ||
				isBUNSIN ||
				isHALLUCINATIONWALK ||
				isQUICKEN ||
				isOVERTHRUST ||
				isEXPLOSIONSPIRITS ||
				isBERSERK ||
				!!entity._fastMoveTrail ||
				!!entity._enableTrail,
			blurType: isBUNSIN ? 5 : isHALLUCINATIONWALK ? 3 : entity._blurType || 1
		});

		// Render all frames
		for (let i = 0, count = layers.length; i < count; ++i) {
			entity.renderLayer(layers[i], spr, pal, files.size, _position, type, isBlendModeOne);
		}

		if (is_main && type === 'body' && entity.waterDepthFrame === null) {
			const frame =
				entity._waterDepthFrameBuffer || (entity._waterDepthFrameBuffer = { position: new Int32Array(2) });
			frame.layers = layers;
			frame.spr = spr;
			frame.pal = pal;
			frame.size = files.size;
			frame.position.set(_position);
			entity.waterDepthFrame = frame;
		}

		// Save reference
		if (is_main && animation.pos.length) {
			position[0] = animation.pos[0].x;
			position[1] = animation.pos[0].y;
		}
	};
})();

/**
 * Get animation delay
 * TODO: search how works the delay on walk and aspd.
 *
 * @param {string} type
 * @param {string} entity
 * @param {object} act
 * @returns {number} delay
 */
function getAnimationDelay(type, entity, act) {
	if (type === 'body' && entity.action === entity.ACTION.WALK) {
		return (act.delay / 150) * entity.walk.speed;
	}

	// Delay on attack (fallback when animation.speed is not explicitly set)
	// Uses the ACT file delay directly (matching official C++ client m_motionSpeed = actRes->GetDelay(action)).
	// Player normal attacks already provide animation.speed = pkt.attackMT / m_attackMotion in onEntityAttack.
	if (
		entity.action === entity.ACTION.ATTACK ||
		entity.action === entity.ACTION.ATTACK1 ||
		entity.action === entity.ACTION.ATTACK2 ||
		entity.action === entity.ACTION.ATTACK3
	) {
		if (act && act.delay && act.delay > 0) {
			return act.delay;
		}
		if (entity.attack_speed && act && act.animations && act.animations.length > 0) {
			return Math.max(entity.attack_speed / act.animations.length, 100);
		}
		return 150;
	}

	return (act && act.delay) || 150;
}

/**
 * Calculate animations
 */
function calcAnimation(entity, act, type, tick) {
	// Fix for shadow
	if (type === 'shadow' || type === 'cartshadow') {
		return 0;
	}

	// To avoid look up
	const ACTION = entity.ACTION;
	const action = entity.action;
	const animation = entity.animation;
	let animCount = act.animations.length;
	const animSize = animCount;
	const animLastIndex = animSize - 1;
	const isIdle = action === ACTION.IDLE || action === ACTION.SIT;
	let delay = getAnimationDelay(type, entity, act);
	let headDir = 0;
	let anim = 0;

	//overrides
	if (animation.length) {
		animCount = animation.length;
	}
	if (animation.speed) {
		delay = animation.speed;
	}

	if (type === 'cart' && isIdle) {
		return 0;
	}

	// Get rid of doridori

	if (
		(type === 'body' || type === 'robe') &&
		(entity.objecttype === entity.constructor.TYPE_PC || entity.objecttype === entity.constructor.TYPE_MERC) &&
		isIdle
	) {
		if (entity.headDir <= animLastIndex) {
			return entity.headDir;
		}
		return animLastIndex;
	}

	// If hat/hair, divide to 3 since there is doridori include
	// TODO: fixed, just on IDLE and SIT ?
	if (
		type === 'head' &&
		(entity.objecttype === entity.constructor.TYPE_PC || entity.objecttype === entity.constructor.TYPE_MERC) &&
		isIdle
	) {
		animCount = Math.floor(animCount / 3);
		headDir = entity.headDir <= animLastIndex ? entity.headDir : animLastIndex;
	}

	// Don't play, so stop at the current frame.
	if (animation.play === false) {
		anim += animCount * headDir; // get rid of doridori
		anim += animation.frame; // set frame
		anim %= animSize; // avoid overflow

		return anim;
	}

	// Distance-driven walk animation to keep motion synced to ground distance.
	// Adapted from official client: floor(dist * stepSize * 0.37 * 4 / (delay/24)) % motionCount
	// In roBrowser, `entity.walk.dist` is in map cells and WALK_DIST_TO_MOTION encodes
	// stepSize * 0.37 * 4 * 24, so we divide by act.delay (ms).
	// Apply to all sprite parts so hats/weapons stay in phase with the body.
	if (action === ACTION.WALK && entity.walk && entity.objecttype !== entity.constructor.TYPE_FALCON) {
		let motionCount = animCount || 1;
		if (animation.length) {
			motionCount = animation.length;
		}
		motionCount = Math.max(motionCount, 1);

		// motionSpeed is the per-action delay from the .act file.
		// Cache a shared walk phase per render tick so all parts stay in step.
		const motionSpeed = Math.max(act.delay || 1, 1);
		let phase;
		const nowTick = Date.now();
		const havePhaseThisTick =
			entity.walk._motionPhaseTick === nowTick && typeof entity.walk._motionPhase === 'number';
		const isBodyPart = type === 'body';

		// Body is authoritative for motionSpeed; recompute when we render it.
		if (!havePhaseThisTick || isBodyPart) {
			phase = (entity.walk.dist * WALK_DIST_TO_MOTION) / motionSpeed;
			entity.walk._motionPhase = phase;
			entity.walk._motionPhaseTick = nowTick;
		} else {
			phase = entity.walk._motionPhase;
		}

		let motion = Math.floor(phase);
		motion %= motionCount;
		motion += motionCount * headDir; // keep head dir offset handling consistent
		motion += animation.frame;
		motion %= animSize;

		return motion;
	}

	// Repeatable
	if (animation.repeat) {
		anim = Math.floor(tick / delay);

		entity.sound.freeOnAnimationEnd(anim, animCount);

		anim %= animCount;
		anim += animCount * headDir; // get rid of doridori
		anim += animation.frame; // don't forget the previous frame
		anim %= animSize; // avoid overflow

		return anim;
	}

	// No repeat
	anim = Math.min((tick / delay) | 0, animCount ? animCount - 1 : 0);

	anim %= animCount;
	anim += animCount * headDir; // get rid of doridori
	anim += animation.frame; // previous frame
	anim %= animSize; // avoid overflow

	const lastFrame = animation.frame + animCount - 1;

	if (type === 'body' && ((tick / delay) | 0) >= animCount - 1) {
		animation.frame = anim = Math.min(lastFrame, animSize - 1);
		animation.play = false;
		if (animation.next) {
			entity.setAction(animation.next);
		}
	}

	return Math.min(anim, animSize - 1);
}

/**
 * Render ROSprite Render
 *
 * @param {object} layer structure
 * @param {object} spr sprite structure
 * @param {object} pal palette structure
 * @param {float}  sprite size
 * @param {Array} pos [x,y] where to render the sprite
 * @param {string} type
 * @param {boolean} isBlendModeOne
 */
function renderLayer(layer, spr, pal, size, pos, type, isBlendModeOne) {
	// If there is nothing to render
	if (layer.index < 0) {
		return;
	}

	SpriteRenderer.image.palette = null;
	SpriteRenderer.sprite = spr.frames[layer.index];
	SpriteRenderer.palette = pal.palette;

	let index = layer.index + 0;
	const is_rgba = layer.spr_type === 1 || spr.rgba_index === 0;

	if (!is_rgba) {
		SpriteRenderer.image.palette = pal.texture;
		SpriteRenderer.image.size[0] = 2 * spr.frames[index].width;
		SpriteRenderer.image.size[1] = 2 * spr.frames[index].height;
	}

	// RGBA is at the end of the spr.
	else if (layer.spr_type === 1) {
		index += spr.old_rgba_index;
	}

	if (!spr.frames[index]) {
		console.warn('[renderLayer] - Frame index ' + index + ' not found in sprite');
		return;
	}

	const frame = spr.frames[index];
	let width = frame.width;
	let height = frame.height;

	// Apply the scale
	width *= layer.scale[0] * size;
	height *= layer.scale[1] * size;

	// Get the entity bounding rect
	if (type === 'body') {
		const w = (frame.originalWidth * layer.scale[0] * size) / 2;
		const h = (frame.originalHeight * layer.scale[1] * size) / 2;

		this.boundingRect.x1 = Math.min(this.boundingRect.x1, layer.pos[0] + pos[0] - w);
		this.boundingRect.y1 = Math.max(this.boundingRect.y1, -(layer.pos[1] + pos[1]) + h);
		this.boundingRect.x2 = Math.max(this.boundingRect.x2, layer.pos[0] + pos[0] + w);
		this.boundingRect.y2 = Math.min(this.boundingRect.y2, -(layer.pos[1] + pos[1]) - h);
		this.bodyHeight = height;
	}

	if (type === 'robe') {
		this.robeHeight = height + layer.pos[1];
	}

	// Image inverted
	if (layer.is_mirror) {
		width = -width;
	}

	// copy color
	if (type !== 'shadow') {
		SpriteRenderer.color[0] = layer.color[0] * this.effectColor[0];
		SpriteRenderer.color[1] = layer.color[1] * this.effectColor[1];
		SpriteRenderer.color[2] = layer.color[2] * this.effectColor[2];
		SpriteRenderer.color[3] = layer.color[3] * this.effectColor[3];
	} else {
		SpriteRenderer.color[0] = layer.color[0];
		SpriteRenderer.color[1] = layer.color[1];
		SpriteRenderer.color[2] = layer.color[2];
		SpriteRenderer.color[3] = layer.color[3];
	}

	// apply disapear
	if (this.remove_tick) {
		SpriteRenderer.color[3] *= 1 - (Date.now() - this.remove_tick) / this.remove_delay;
	}

	// Store shader info
	SpriteRenderer.angle = layer.angle;
	SpriteRenderer.size[0] = width;
	SpriteRenderer.size[1] = height;
	SpriteRenderer.offset[0] = layer.pos[0] + pos[0];
	SpriteRenderer.offset[1] = layer.pos[1] + pos[1];
	SpriteRenderer.xSize = this.xSize;
	SpriteRenderer.ySize = this.ySize;
	SpriteRenderer.image.texture = frame.texture;

	// Draw Sprite
	SpriteRenderer.render(isBlendModeOne);
}

/**
 * Export
 */
export default function Init() {
	this.render = render;
	this.renderLayer = renderLayer;
	this.renderEntity = renderEntity;
	this.renderWaterDepth = renderWaterDepth;
	this.waterDepthFrame = undefined;
	this._waterDepthFrameBuffer = null;
}
