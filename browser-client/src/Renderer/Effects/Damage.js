/**
 * Renderer/Effects/Damage.js
 *
 * Rendering damage particles
 * TODO: Create a particle class to manage the process
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import WebGL from 'Utils/WebGL.js';
import Client from 'Core/Client.js';
import Configs from 'Core/Configs.js';
import Sprite from 'Loaders/Sprite.js';
import Action from 'Loaders/Action.js';
import Renderer from 'Renderer/Renderer.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import MapPreferences from 'Preferences/Map.js';
import DB from 'DB/DBManager.js';
import Sound from 'Audio/SoundManager.js';
import EffectManager from 'Renderer/EffectManager.js';
import MemoryManager from 'Core/MemoryManager.js';
import Entity from 'Renderer/Entity/Entity.js';
import GraphicsSettings from 'Preferences/Graphics.js';
import PACKETVER from 'Network/PacketVerManager.js';

// Load dependencies
const EndureSound = 'player_metal.wav';
const dpr = window.devicePixelRatio || 1;

const procCanvas = document.createElement('canvas');
const procCtx = procCanvas.getContext('2d', { willReadFrequently: true });

let _skin = 0;
const _damageSkins = {
	0: {
		// DT_Default
		BaseNumber: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/\xbc\xfd\xc0\xda.spr',
		BaseMsg: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/msg.spr',
		BaseBlue: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/bluemsg.spr'
	},
	1: {
		// DT_NewNumber
		BaseNumber: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/NewNumber.spr',
		BaseMsg: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/NewNumber_MSG.spr',
		BaseBlue: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/NewNumber_BMSG.spr'
	},
	2: {
		// DT_Han
		BaseNumber: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/Han.spr',
		BaseMsg: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/Han_MSG.spr',
		BaseBlue: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/Han_BMSG.spr'
	},
	3: {
		// DT_Invi
		BaseNumber: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/NewNumberH.spr',
		BaseMsg: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/NewNumberH_MSG.spr',
		BaseBlue: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/NewNumberH_BMSG.spr'
	}
};

const _loadedSkinsData = {};

// Damage suffix status
let _enableSuffix = false;

/**
 * @var {string} Sprite of the damage sprite
 */
const _msgNames = {
	0: 'miss',
	1: 'guard',
	2: 'crit',
	3: 'critbg',
	4: 'luckybg',
	5: 'lucky'
};

/**
 * @var {Damage[]} List of damages
 */
const _list = [];

/**
 * @var {object} frame descriptor for the RGBA damage textures (no palette)
 */
const _rgbaFrame = { type: 1 };

/**
 * @var previus combo time
 */
const prevCombo = [];

/**
 * Damage Namespace
 */
class Damage {
	constructor() {
		this.entity = null;
		this.startTick = 0;
		this.type = 0;
		this.color = new Float32Array(4);
		this.delay = 1500;
		this.texture = null;
		this.width = 0;
		this.height = 0;
	}

	/**
	 * Convert sprite to image Data
	 * @param {object} gl - WebGL context
	 */
	static init(gl) {
		const confChange = !(_enableSuffix === Configs.get('enableDmgSuffix'));

		_enableSuffix = Configs.get('enableDmgSuffix');

		const totalSkins = Object.keys(_damageSkins).length;

		const num_count = _enableSuffix ? 12 : 10;

		if (PACKETVER.value < 20220821 && Object.keys(_loadedSkinsData).length > 0 && !confChange) {
			return;
		}

		if (Object.keys(_loadedSkinsData).length === totalSkins && !confChange) {
			return;
		}

		Object.keys(_damageSkins).forEach(skinIdStr => {
			const skinId = parseInt(skinIdStr, 10);
			const currentSkin = _damageSkins[skinId];

			if (_loadedSkinsData[skinId] && !confChange) {
				return;
			}

			if (PACKETVER.value < 20220821 && Object.keys(_loadedSkinsData).length > 0) {
				return;
			}

			_loadedSkinsData[skinId] = {
				numbers: new Array(num_count),
				msg: {},
				msgBlue: {}
			};

			const skinData = _loadedSkinsData[skinId];

			Client.getFiles(
				[
					currentSkin.BaseNumber,
					currentSkin.BaseMsg,
					currentSkin.BaseBlue,
					currentSkin.BaseNumber.replace('.spr', '.act')
				],
				(numbers, msg, bluemsg, numbersAct) => {
					let sprNumbers, sprMsg, sprBlue, actNumbers;
					const enableMipmap = Configs.get('enableMipmap');

					// Load it properly later using webgl
					MemoryManager.remove(gl, msg);

					try {
						sprNumbers = new Sprite(numbers);
						sprMsg = new Sprite(msg);
						sprBlue = new Sprite(bluemsg);
						actNumbers = new Action(numbersAct);
					} catch (e) {
						console.error('Damage::init() - ' + e.message);
						return;
					}

					skinData.Scale = [];
					if (
						actNumbers.actions &&
						actNumbers.actions[0] &&
						actNumbers.actions[0].animations &&
						actNumbers.actions[0].animations[0] &&
						actNumbers.actions[0].animations[0].layers &&
						actNumbers.actions[0].animations[0].layers[0]
					) {
						skinData.Scale = actNumbers.actions[0].animations[0].layers[0].scale || [1.0, 1.0];
					}

					// Create SpriteSheet
					for (let i = 0; i < skinData.numbers.length; ++i) {
						skinData.numbers[i] = sprNumbers.getCanvasFromFrame(i);
					}

					for (let i = 0; i < 6; i++) {
						//msg.spr miss crit lucky...

						const source = sprMsg.getCanvasFromFrame(i);
						const canvas = document.createElement('canvas');
						const ctx = canvas.getContext('2d');

						canvas.width = WebGL.toPowerOfTwo(source.width) * dpr;
						canvas.height = WebGL.toPowerOfTwo(source.height) * dpr;
						ctx.drawImage(
							source,
							(canvas.width - source.width) / 2,
							(canvas.height - source.height) / 2,
							source.width,
							source.height
						);

						skinData.msg[_msgNames[i]] = {
							texture: gl.createTexture(),
							canvas: canvas
						};

						gl.bindTexture(gl.TEXTURE_2D, skinData.msg[_msgNames[i]].texture);
						gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
						gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
						gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
						gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
						gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
						if (enableMipmap) {
							gl.generateMipmap(gl.TEXTURE_2D);
						}
					}

					for (let i = 0; i < 6; i++) {
						//bluemsg.spr miss crit lucky...

						const source = sprBlue.getCanvasFromFrame(i);
						const canvas = document.createElement('canvas');
						const ctx = canvas.getContext('2d');

						canvas.width = WebGL.toPowerOfTwo(source.width) * dpr;
						canvas.height = WebGL.toPowerOfTwo(source.height) * dpr;
						ctx.drawImage(
							source,
							(canvas.width - source.width) / 2,
							(canvas.height - source.height) / 2,
							source.width,
							source.height
						);

						skinData.msgBlue[_msgNames[i]] = {
							texture: gl.createTexture(),
							canvas: canvas
						};

						gl.bindTexture(gl.TEXTURE_2D, skinData.msgBlue[_msgNames[i]].texture);
						gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);
						gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
						gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
						gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
						gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
						if (enableMipmap) {
							gl.generateMipmap(gl.TEXTURE_2D);
						}
					}
				}
			);
		});
	}

	/**
	 * Add Damage to the scene
	 *
	 * @param {number} damage
	 * @param {Entity} entity to receive the damage
	 * @param {number} tick
	 * @param {number} type - Damage|Heal
	 */
	static add(damage, entity, tick, weapon, type) {
		_skin = GraphicsSettings.damageSkin || 0;

		if (_skin == 3) {
			return;
		}

		if (PACKETVER.value < 20220821) {
			_skin = 0;
		}

		let skinData = _loadedSkinsData[_skin];
		let numbersData, msgData, msgBlueData;
		if (skinData) {
			numbersData = skinData.numbers;
			msgData = skinData.msg;
			msgBlueData = skinData.msgBlue;
		} else {
			if (_loadedSkinsData[0]) {
				skinData = _loadedSkinsData[0];
				numbersData = skinData.numbers;
				msgData = skinData.msg;
				msgBlueData = skinData.msgBlue;
			} else {
				console.warn('Damage::add() - No Damage Skin Loaded.');
				return;
			}
		}

		// Can not display negative damages.
		// Need to wait the client to load damage sprite
		if (damage < 0 || !numbersData[0] || !msgData.miss || !msgBlueData.miss) {
			return;
		}

		// Can't render floating points
		damage = Math.floor(damage);

		const PADDING = 2;
		let i, count;
		let frame;

		let suffix = null;

		if (_enableSuffix) {
			// Check for large numbers and convert accordingly
			if (damage >= 100000000) {
				damage = Math.floor(damage / 1000000);
				suffix = 11; // 'M'
			} else if (damage >= 1000000) {
				damage = Math.floor(damage / 1000);
				suffix = 10; // 'K'
			}
		}

		const numbers = damage.toString().split('');

		// Add suffix to numbers if it exists
		if (suffix !== null) {
			numbers.push(suffix);
		}

		let width = 0;
		let height = 0;
		const gl = Renderer.gl;

		const obj = new Damage();

		obj.type = type || (damage ? Damage.TYPE.DAMAGE : Damage.TYPE.MISS);
		if (entity.objecttype === entity.constructor.TYPE_PC) {
			obj.type |= Damage.TYPE.ENEMY;
		}

		obj.color = [1.0, 1.0, 1.0, 1.0]; // for sprite renderer
		obj.delay = 1500;
		obj.startTick = tick;
		obj.entity = entity;

		if (obj.type & Damage.TYPE.SP) {
			// blue
			obj.color[0] = 0.13;
			obj.color[1] = 0.19;
			obj.color[2] = 0.75;
		} else if (obj.type & Damage.TYPE.HEAL) {
			// green
			obj.color[0] = 0.0;
			obj.color[1] = 1.0;
			obj.color[2] = 0.0;
		} else if (obj.type & Damage.TYPE.ENEMY) {
			// red
			obj.color[0] = 1.0;
			obj.color[1] = 0.0;
			obj.color[2] = 0.0;
		} else if (obj.type & Damage.TYPE.COMBO) {
			// yellow
			obj.color[0] = 0.9;
			obj.color[1] = 0.9;
			obj.color[2] = 0.15;
			obj.delay = 3000;
		} else if (obj.type & Damage.TYPE.CRIT) {
			// yellow
			obj.color[0] = 0.9;
			obj.color[1] = 0.9;
			obj.color[2] = 0.15;

			// Add CRIT background
			const bgObj = new Damage();
			bgObj.type = Damage.TYPE.CRIT;
			bgObj.color = [0.66, 0.66, 0.66, 1.0];
			bgObj.delay = 1500;
			bgObj.startTick = tick;
			bgObj.entity = entity;
			bgObj.texture = msgData.critbg.texture;
			bgObj.width = msgData.critbg.canvas.width * 0.6;
			bgObj.height = msgData.critbg.canvas.height * 0.6;
			bgObj.offset = [0.0, -6.0];
			bgObj.isDisposable = false;
			_list.push(bgObj);

			// Add hit effect
			const EF_Init_Par = {
				effectId: 1,
				ownerAID: entity.GID,
				position: entity.position,
				startTick: tick
			};
			EffectManager.spam(EF_Init_Par);
		} else if (obj.type & Damage.TYPE.COMBO_B) {
			// white
			obj.color[0] = 1.0;
			obj.color[1] = 1.0;
			obj.color[2] = 1.0;

			// Add Blue CRIT background
			const bgObj = new Damage();
			bgObj.type = obj.type;
			bgObj.color = [0.66, 0.66, 0.66, 1.0];
			bgObj.delay = 1500;
			bgObj.startTick = tick;
			bgObj.entity = entity;
			bgObj.texture = msgBlueData.critbg.texture;
			bgObj.width = msgBlueData.critbg.canvas.width * 0.6;
			bgObj.height = msgBlueData.critbg.canvas.height * 0.6;
			bgObj.offset = [0.0, -6.0];
			bgObj.isDisposable = false;
			_list.push(bgObj);
		} else {
			// white
			obj.color[0] = 1.0;
			obj.color[1] = 1.0;
			obj.color[2] = 1.0;
		}

		// Miss
		if (!damage) {
			if (MapPreferences.miss) {
				obj.texture = msgData.miss.texture;
				obj.width = msgData.miss.canvas.width;
				obj.height = msgData.miss.canvas.height;
				obj.isDisposable = false;
				_list.push(obj);
			}
			return;
		}

		// Calculate canvas width and height
		for (i = 0, count = numbers.length; i < count; ++i) {
			frame = numbersData[numbers[i]];
			width += frame.width + PADDING;
			height = Math.max(height, frame.height);
		}

		// Set canvas size (pow of 2 for webgl).
		const finalWidth = WebGL.toPowerOfTwo(width) * dpr;
		const finalHeight = WebGL.toPowerOfTwo(height) * dpr;

		if (procCanvas.width !== finalWidth || procCanvas.height !== finalHeight) {
			procCanvas.width = finalWidth;
			procCanvas.height = finalHeight;
		} else {
			procCtx.clearRect(0, 0, finalWidth, finalHeight);
		}

		// find where to start to get the image at the center
		const start_x = (finalWidth - width) >> 1;
		const start_y = (finalHeight - height) >> 1;

		// build texture
		width = 0;
		for (i = 0, count = numbers.length; i < count; ++i) {
			frame = numbersData[numbers[i]];
			procCtx.drawImage(frame, start_x + width, start_y + ((height - frame.height) >> 1));
			width += frame.width + PADDING;
		}

		const texture = gl.createTexture();

		const enableMipmap = Configs.get('enableMipmap');
		gl.bindTexture(gl.TEXTURE_2D, texture);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, procCanvas);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
		if (enableMipmap) {
			gl.generateMipmap(gl.TEXTURE_2D);
		}

		obj.texture = texture;
		obj.width = finalWidth;
		obj.height = finalHeight;
		obj.isDisposable = true;

		let hitSound;
		if (entity.objecttype === Entity.TYPE_PC) {
			hitSound = DB.getJobHitSound(entity._job);
		} else {
			if (weapon || weapon === 0) {
				hitSound = DB.getWeaponHitSound(weapon);
			}
		}

		if (hitSound) {
			obj.soundFile = hitSound;
		}

		_list.push(obj);
	}

	/**
	 * Remove damages from map, clean up memory
	 *
	 * @param {object} gl context
	 */
	static free(gl) {
		_list.forEach(item => {
			if (item.isDisposable) {
				gl.deleteTexture(item.texture);
			}
		});

		_list.length = 0;
	}

	/**
	 * Rendering damages on maps
	 *
	 * @param {object} gl context
	 * @param {mat4} modelView
	 * @param {mat4} projection
	 * @param {object} fog structure
	 * @param {number} tick - game tick
	 */
	// Render all damages.
	static render(gl, modelView, projection, fog, tick) {
		// Nothing to render exiting
		if (!_list.length) {
			return;
		}

		// Init program
		SpriteRenderer.bind3DContext(gl, modelView, projection, fog);

		// Base parameters
		SpriteRenderer.shadow = 1.0;
		SpriteRenderer.angle = 0;
		SpriteRenderer.image.palette = null;
		SpriteRenderer.sprite = _rgbaFrame;

		let i, count, perc;
		let damage;
		let size;

		const skinData = _loadedSkinsData[_skin];

		// Render all list
		for (i = 0, count = _list.length; i < count; ++i) {
			damage = _list[i];

			// Not now.
			if (damage.startTick > tick) {
				continue;
			}

			// Remove it from list, time passed.
			if (damage.startTick + damage.delay < tick) {
				if (damage.isDisposable) {
					gl.deleteTexture(damage.texture);
				}
				_list.splice(i, 1);
				count--;
				i--;
				continue;
			}

			perc = (tick - damage.startTick) / damage.delay;

			// Combo title
			if (damage.type & Damage.TYPE.COMBO || damage.type & Damage.TYPE.COMBO_B) {
				//Combo title need to remove if new one come up
				if (damage.startTick < prevCombo[damage.entity.GID]) {
					continue;
				}
				prevCombo[damage.entity.GID] = damage.startTick;

				// TODO: fix it
				size = Math.min(perc, 0.05) * 70;

				// Remove it
				if (!(damage.type & Damage.TYPE.COMBO_FINAL) && perc > 0.15) {
					damage.startTick = 0;
				}

				SpriteRenderer.position[0] = damage.entity.position[0];
				SpriteRenderer.position[1] = damage.entity.position[1];
				SpriteRenderer.position[2] = damage.entity.position[2] + 7 + perc;
			}

			// Damage
			else if (damage.type & Damage.TYPE.DAMAGE || damage.type & Damage.TYPE.CRIT) {
				size = (1 - perc) * 4;

				const motionType = GraphicsSettings.damageMotion || 0;

				// Base Z arc (Bounce)
				const zArc = Math.sin(-Math.PI / 2 + Math.PI * (0.5 + perc * 1.5)) * 5;

				switch (motionType) {
					case 1: // Left (Drift X Negative)
						SpriteRenderer.position[0] = damage.entity.position[0] - perc * 4;
						SpriteRenderer.position[1] = damage.entity.position[1];
						break;
					case 2: // Top (Vertical - No horizontal drift)
						SpriteRenderer.position[0] = damage.entity.position[0];
						SpriteRenderer.position[1] = damage.entity.position[1];
						break;
					case 3: // Right (Drift X Positive)
						SpriteRenderer.position[0] = damage.entity.position[0] + perc * 4;
						SpriteRenderer.position[1] = damage.entity.position[1];
						break;
					case 0: // Default (Diagonal)
					default:
						SpriteRenderer.position[0] = damage.entity.position[0] + perc * 4;
						SpriteRenderer.position[1] = damage.entity.position[1] - perc * 4;
						break;
				}

				SpriteRenderer.position[2] = damage.entity.position[2] + 2 + zArc;
				if (damage.soundFile) {
					if (damage.type & Damage.TYPE.ENDURE) {
						Sound.playPosition(EndureSound, damage.entity.position);
					}

					Sound.playPosition(damage.soundFile, damage.entity.position);
					delete damage.soundFile;
				}
			}

			// Heal
			else if (damage.type & Damage.TYPE.HEAL) {
				size = Math.max((1 - perc * 2) * 3, 0.8);
				SpriteRenderer.position[0] = damage.entity.position[0];
				SpriteRenderer.position[1] = damage.entity.position[1];
				SpriteRenderer.position[2] = damage.entity.position[2] + 2 + (perc < 0.4 ? 0 : (perc - 0.4) * 5);
			}

			// Miss
			else if (damage.type & Damage.TYPE.MISS) {
				perc = (tick - damage.startTick) / 800;
				size = 0.5;
				SpriteRenderer.position[0] = damage.entity.position[0];
				SpriteRenderer.position[1] = damage.entity.position[1];
				SpriteRenderer.position[2] = damage.entity.position[2] + 3.5 + perc * 7;
			}

			// Miss
			else if (damage.type & Damage.TYPE.LUCKY) {
				perc = (tick - damage.startTick) / 800;
				size = 0.5;
				SpriteRenderer.position[0] = damage.entity.position[0];
				SpriteRenderer.position[1] = damage.entity.position[1];
				SpriteRenderer.position[2] = damage.entity.position[2] + 3.5 + perc * 7;
			}

			SpriteRenderer.size[0] = damage.width * size;
			SpriteRenderer.size[1] = damage.height * size;

			if (skinData && skinData.Scale) {
				SpriteRenderer.size[0] *= skinData.Scale[0];
				SpriteRenderer.size[1] *= skinData.Scale[1];
			}

			damage.color[3] = 1.0 - perc;

			if (damage.offset) {
				SpriteRenderer.offset[0] = damage.offset[0];
				SpriteRenderer.offset[1] = damage.offset[1];
			} else {
				SpriteRenderer.offset[0] = 0;
				SpriteRenderer.offset[1] = 0;
			}

			SpriteRenderer.depth = i;

			SpriteRenderer.color.set(damage.color);
			SpriteRenderer.image.texture = damage.texture;
			SpriteRenderer.runWithDepth(false, false, true, () => {
				SpriteRenderer.render();
			});
		}

		// Restore GL state
		SpriteRenderer.unbind(gl);
	}
}
/**
 * Damage type constant
 */
Damage.TYPE = {
	HEAL: 1 << 0,
	MISS: 1 << 1,
	DAMAGE: 1 << 2,
	ENEMY: 1 << 3,
	COMBO: 1 << 4,
	COMBO_FINAL: 1 << 5,
	SP: 1 << 6,
	CRIT: 1 << 7,
	LUCKY: 1 << 8,
	ENDURE: 1 << 9,
	COMBO_B: 1 << 10
};

/**
 * Export
 */
export default Damage;
