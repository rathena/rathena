import WebGL from 'Utils/WebGL.js';
import Client from 'Core/Client.js';
import SpriteRenderer from 'Renderer/SpriteRenderer.js';
import EntityManager from 'Renderer/EntityManager.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Camera from 'Renderer/Camera.js';
import Entity from 'Renderer/Entity/Entity.js';

function randBetween(minimum, maximum) {
	return parseFloat(Math.min(minimum + Math.random() * (maximum - minimum), maximum).toFixed(3));
}

const blendMode = {};
let _soulStrikeFirstEffect = null;

class ThreeDEffect {
	constructor(effect, EF_Inst_Par, EF_Init_Par) {
		const position = EF_Inst_Par.position;
		const otherPosition = EF_Inst_Par.otherPosition;
		const startTick = EF_Inst_Par.startTick;
		const endTick = EF_Inst_Par.endTick;
		const AID = EF_Init_Par.ownerAID;

		this.AID = AID;
		this.ownerEntity = EF_Init_Par.ownerEntity;
		this.otherEntity = EF_Init_Par.otherEntity;

		this.textureName = effect.file;

		this.textureNameList = Array.isArray(effect.fileList) ? effect.fileList : [];
		this.frameDelay = !isNaN(effect.frameDelay) ? effect.frameDelay : 10;

		this.zIndex = effect.zIndex ? effect.zIndex : 0;
		this.fadeOut = effect.fadeOut ? true : false;
		this.fadeIn = effect.fadeIn ? true : false;
		this.shadowTexture = effect.shadowTexture ? true : false;
		this.spriteName = effect.spriteName;
		this.absoluteSpriteName = effect.absoluteSpriteName;
		this.spriteRessource = null;
		this.playSprite = effect.playSprite ? true : false;
		this.sprDelay = effect.sprDelay ? effect.sprDelay : 0;
		if (effect.rotatePosX > 0) {
			this.rotatePosX = effect.rotatePosX;
		} else {
			this.rotatePosX = 0;
		}
		if (effect.rotatePosY > 0) {
			this.rotatePosY = effect.rotatePosY;
		} else {
			this.rotatePosY = 0;
		}
		if (effect.rotatePosZ > 0) {
			this.rotatePosZ = effect.rotatePosZ;
		} else {
			this.rotatePosZ = 0;
		}
		if (effect.nbOfRotation > 0) {
			this.nbOfRotation = effect.nbOfRotation;
		} else {
			this.nbOfRotation = 1;
		}

		this.rotateLate = effect.rotateLate > 0 ? effect.rotateLate : 0;
		this.rotateLate += effect.rotateLateDelta ? effect.rotateLateDelta * EF_Inst_Par.duplicateID : 0;

		this.rotationClockwise = effect.rotationClockwise ? true : false;
		this.sparkling = effect.sparkling ? true : false;
		if (effect.sparkNumber > 0) {
			this.sparkNumber = effect.sparkNumber;
		} else {
			if (effect.sparkNumberRand) {
				this.sparkNumber = randBetween(effect.sparkNumberRand[0], effect.sparkNumberRand[1]);
			} else {
				this.sparkNumber = 1;
			}
		}

		// When true, draw as an overlay: no depth test and no ray-plane correction.
		this.overlay = effect.overlay ? true : false;

		this.alphaMax = !isNaN(effect.alphaMax) ? Math.max(Math.min(effect.alphaMax, 1), 0) : 1;
		this.alphaMax = Math.max(
			Math.min(
				this.alphaMax + (!isNaN(effect.alphaMaxDelta) ? effect.alphaMaxDelta * EF_Inst_Par.duplicateID : 0),
				1
			),
			0
		);

		if (effect.red) {
			this.red = effect.red;
		} else {
			this.red = 1;
		}
		if (effect.green) {
			this.green = effect.green;
		} else {
			this.green = 1;
		}
		if (effect.blue) {
			this.blue = effect.blue;
		} else {
			this.blue = 1;
		}
		this.position = position;
		if (effect.posxStart) {
			this.posxStart = effect.posxStart;
		} else {
			this.posxStart = 0;
		}
		if (effect.posxEnd) {
			this.posxEnd = effect.posxEnd;
		} else {
			this.posxEnd = 0;
		}
		if (effect.posx) {
			this.posxStart = effect.posx;
			this.posxEnd = effect.posx;
		}
		if (effect.posxRand) {
			this.posxStart = randBetween(-effect.posxRand, effect.posxRand);
			this.posxEnd = this.posxStart;
		}
		if (effect.posxRandDiff) {
			this.posxStart = randBetween(-effect.posxRandDiff, effect.posxRandDiff);
			this.posxEnd = randBetween(-effect.posxRandDiff, effect.posxRandDiff);
		}
		if (effect.posxStartRand) {
			const posxStartRandMiddle = effect.posxStartRandMiddle ? effect.posxStartRandMiddle : 0;
			this.posxStart = randBetween(
				posxStartRandMiddle - effect.posxStartRand,
				posxStartRandMiddle + effect.posxStartRand
			);
		}
		if (effect.posxEndRand) {
			const posxEndRandMiddle = effect.posxEndRandMiddle ? effect.posxEndRandMiddle : 0;
			this.posxEnd = randBetween(posxEndRandMiddle - effect.posxEndRand, posxEndRandMiddle + effect.posxEndRand);
		}
		this.posxSmooth = effect.posxSmooth ? true : false;
		if (effect.posyStart) {
			this.posyStart = effect.posyStart;
		} else {
			this.posyStart = 0;
		}
		if (effect.posyEnd) {
			this.posyEnd = effect.posyEnd;
		} else {
			this.posyEnd = 0;
		}
		if (effect.posy) {
			this.posyStart = effect.posy;
			this.posyEnd = effect.posy;
		}
		if (effect.posyRand) {
			this.posyStart = randBetween(-effect.posyRand, effect.posyRand);
			this.posyEnd = this.posyStart;
		}
		if (effect.posyRandDiff) {
			this.posyStart = randBetween(-effect.posyRandDiff, effect.posyRandDiff);
			this.posyEnd = randBetween(-effect.posyRandDiff, effect.posyRandDiff);
		}
		if (effect.posyStartRand) {
			const posyStartRandMiddle = effect.posyStartRandMiddle ? effect.posyStartRandMiddle : 0;
			this.posyStart = randBetween(
				posyStartRandMiddle - effect.posyStartRand,
				posyStartRandMiddle + effect.posyStartRand
			);
		}
		if (effect.posyEndRand) {
			const posyEndRandMiddle = effect.posyEndRandMiddle ? effect.posyEndRandMiddle : 0;
			this.posyEnd = randBetween(posyEndRandMiddle - effect.posyEndRand, posyEndRandMiddle + effect.posyEndRand);
		}
		this.posySmooth = effect.posySmooth ? true : false;
		if (effect.poszStart) {
			this.poszStart = effect.poszStart;
		} else {
			this.poszStart = 0;
		}
		if (effect.poszEnd) {
			this.poszEnd = effect.poszEnd;
		} else {
			this.poszEnd = 0;
		}
		if (effect.posz) {
			this.poszStart = effect.posz;
			this.poszEnd = effect.posz;
		}
		if (effect.poszRand) {
			this.poszStart = randBetween(-effect.poszRand, effect.poszRand);
			this.poszEnd = this.poszStart;
		}
		if (effect.poszRandDiff) {
			this.poszStart = randBetween(-effect.poszRandDiff, effect.poszRandDiff);
			this.poszEnd = randBetween(-effect.poszRandDiff, effect.poszRandDiff);
		}
		if (effect.poszStartRand) {
			const poszStartRandMiddle = effect.poszStartRandMiddle ? effect.poszStartRandMiddle : 0;
			this.poszStart = randBetween(
				poszStartRandMiddle - effect.poszStartRand,
				poszStartRandMiddle + effect.poszStartRand
			);
		}
		if (effect.poszEndRand) {
			const poszEndRandMiddle = effect.poszEndRandMiddle ? effect.poszEndRandMiddle : 0;
			this.poszEnd = randBetween(poszEndRandMiddle - effect.poszEndRand, poszEndRandMiddle + effect.poszEndRand);
		}

		this.xOffset = !isNaN(effect.xOffset) ? effect.xOffset : 0;
		this.yOffset = !isNaN(effect.yOffset) ? effect.yOffset : 0;
		this.zOffset = !isNaN(effect.zOffset) ? effect.zOffset : 0;

		this.zOffsetStart = !isNaN(effect.zOffsetStart) ? effect.zOffsetStart : 0;
		this.zOffsetEnd = !isNaN(effect.zOffsetEnd) ? effect.zOffsetEnd : 0;
		this.arc = !isNaN(effect.arc) ? effect.arc : 0;
		this.retreat = !isNaN(effect.retreat) ? effect.retreat : 0;

		this.poszSmooth = effect.poszSmooth ? true : false;
		if (effect.fromSrc) {
			const randStart = [
				effect.posxStartRand ? randBetween(-effect.posxStartRand, effect.posxStartRand) : 0,
				effect.posyStartRand ? randBetween(-effect.posyStartRand, effect.posyStartRand) : 0,
				effect.poszStartRand ? randBetween(-effect.poszStartRand, effect.poszStartRand) : 0
			];
			const randEnd = [
				effect.posxEndRand ? randBetween(-effect.posxEndRand, effect.posxEndRand) : 0,
				effect.posyEndRand ? randBetween(-effect.posyEndRand, effect.posyEndRand) : 0,
				effect.poszEndRand ? randBetween(-effect.poszEndRand, effect.poszEndRand) : 0
			];

			this.posxStart = 0 + this.xOffset + randStart[0];
			this.posxEnd = otherPosition[0] - position[0] + this.xOffset + randEnd[0];
			this.posyStart = 0 + this.yOffset + randStart[1];
			this.posyEnd = otherPosition[1] - position[1] + this.yOffset + randEnd[1];
			this.poszStart = 0 + this.zOffset + this.zOffsetStart + randStart[2];
			this.poszEnd = otherPosition[2] - position[2] + this.zOffset + this.zOffsetEnd + randEnd[2];
		}
		if (effect.toSrc) {
			const randStart = [
				effect.posxStartRand ? randBetween(-effect.posxStartRand, effect.posxStartRand) : 0,
				effect.posyStartRand ? randBetween(-effect.posyStartRand, effect.posyStartRand) : 0,
				effect.poszStartRand ? randBetween(-effect.poszStartRand, effect.poszStartRand) : 0
			];
			const randEnd = [
				effect.posxEndRand ? randBetween(-effect.posxEndRand, effect.posxEndRand) : 0,
				effect.posyEndRand ? randBetween(-effect.posyEndRand, effect.posyEndRand) : 0,
				effect.poszEndRand ? randBetween(-effect.poszEndRand, effect.poszEndRand) : 0
			];

			this.posxStart = otherPosition[0] - position[0] + this.xOffset + randStart[0];
			this.posxEnd = 0 + this.xOffset + randEnd[0];
			this.posyStart = otherPosition[1] - position[1] + this.yOffset + randStart[1];
			this.posyEnd = 0 + this.yOffset + randEnd[1];
			this.poszStart = otherPosition[2] - position[2] + this.zOffset + this.zOffsetStart + randStart[2];
			this.poszEnd = 0 + this.zOffset + this.zOffsetEnd + randEnd[2];
		}

		if (effect.size) {
			this.sizeStartX = effect.size;
			this.sizeStartY = effect.size;
			this.sizeEndX = effect.size;
			this.sizeEndY = effect.size;
		} else {
			this.sizeStartX = 1;
			this.sizeStartY = 1;
			this.sizeEndX = 1;
			this.sizeEndY = 1;
		}

		if (effect.sizeDelta) {
			this.sizeStartX += effect.sizeDelta * EF_Inst_Par.duplicateID;
			this.sizeStartY += effect.sizeDelta * EF_Inst_Par.duplicateID;
			this.sizeEndX += effect.sizeDelta * EF_Inst_Par.duplicateID;
			this.sizeEndY += effect.sizeDelta * EF_Inst_Par.duplicateID;
		}

		if (effect.sizeStart) {
			this.sizeStartX = effect.sizeStart;
			this.sizeStartY = effect.sizeStart;
		}
		if (effect.sizeEnd) {
			this.sizeEndX = effect.sizeEnd;
			this.sizeEndY = effect.sizeEnd;
		}
		if (effect.sizeX) {
			this.sizeStartX = effect.sizeX;
			this.sizeEndX = effect.sizeX;
		}
		if (effect.sizeY) {
			this.sizeStartY = effect.sizeY;
			this.sizeEndY = effect.sizeY;
		}
		if (effect.sizeStartX) {
			this.sizeStartX = effect.sizeStartX;
		}
		if (effect.sizeStartY) {
			this.sizeStartY = effect.sizeStartY;
		}
		if (effect.sizeEndX) {
			this.sizeEndX = effect.sizeEndX;
		}
		if (effect.sizeEndY) {
			this.sizeEndY = effect.sizeEndY;
		}
		if (effect.sizeRand) {
			this.sizeStartX = effect.size + randBetween(-effect.sizeRand, effect.sizeRand);
			this.sizeStartY = this.sizeStartX;
			this.sizeEndX = this.sizeStartX;
			this.sizeEndY = this.sizeStartX;
		}
		if (effect.sizeRandX) {
			const sizeRandXMiddle = effect.sizeRandXMiddle ? effect.sizeRandXMiddle : 100;
			this.sizeStartX = randBetween(sizeRandXMiddle - effect.sizeRandX, sizeRandXMiddle + effect.sizeRandX);
			this.sizeEndX = this.sizeStartX;
		}
		if (effect.sizeRandY) {
			const sizeRandYMiddle = effect.sizeRandYMiddle ? effect.sizeRandYMiddle : 100;
			this.sizeStartY = randBetween(sizeRandYMiddle - effect.sizeRandY, sizeRandYMiddle + effect.sizeRandY);
			this.sizeEndY = this.sizeStartY;
		}
		this.sizeSmooth = effect.sizeSmooth ? true : false;
		this.angle = effect.angle ? effect.angle : 0;
		this.rotate = effect.rotate ? true : false;
		this.toAngle = effect.toAngle ? effect.toAngle : 0;

		this.startTick = startTick;
		this.endTick = endTick;

		this.blendMode = effect.blendMode;

		if (effect.rotateToTarget) {
			this.rotateToTarget = true;
			const x = this.posxEnd - this.posxStart;
			const y = this.posyEnd - this.posyStart;
			this.angle += 90 - Math.atan2(y, x) * (180 / Math.PI);
		}

		this.rotateWithCamera = effect.rotateWithCamera ? true : false;

		if (effect.soulStrikePattern || effect.drainPattern) {
			if (!EF_Inst_Par.duplicateID || effect.drainPattern) {
				let hitIndex = Math.floor((this.startTick / effect.duration) % 5);

				if (effect.drainPattern) {
					hitIndex = EF_Inst_Par.duplicateID + 1;
				}

				const offsetAngle = hitIndex * 72;
				const offsetRadius = 2;

				const offsetX = Math.cos((offsetAngle * Math.PI) / 180) * offsetRadius;
				const offsetY = Math.sin((offsetAngle * Math.PI) / 180) * offsetRadius;

				this._soulOffsetX = offsetX;
				this._soulOffsetY = offsetY;
				this._soulRetreat =
					(this.retreat || 0) * (1 + hitIndex * 0.2) + (effect.drainPattern ? randBetween(0, 2) : 0);
				this._soulArc = (this.arc || 0) * (1 + hitIndex * 0.1) + (effect.drainPattern ? randBetween(0, 2) : 0);
				this._soulAngle = offsetAngle;
				_soulStrikeFirstEffect = this;
			} else {
				const firstEffect = _soulStrikeFirstEffect;
				if (firstEffect) {
					this._soulOffsetX = firstEffect._soulOffsetX;
					this._soulOffsetY = firstEffect._soulOffsetY;
					this._soulRetreat = firstEffect._soulRetreat;
					this._soulArc = firstEffect._soulArc;
					this._soulAngle = firstEffect._soulAngle;
				}
			}

			this.posxStart = (this.posxStart || 0) + this._soulOffsetX;
			this.posyStart = (this.posyStart || 0) + this._soulOffsetY;
			this.retreat = this._soulRetreat;
			this.arc = this._soulArc;
			this.angle = (this.angle || 0) + this._soulAngle;
		}
	}

	init(gl) {
		this.loadedTextures = 0;
		this.textureList = [];
		if (this.textureNameList.length > 0) {
			const textureCount = this.textureNameList.length;

			for (let i = 0; i < textureCount; i++) {
				Client.loadFile(`data/texture/${this.textureNameList[i]}`, buffer => {
					WebGL.texture(gl, buffer, texture => {
						this.textureList[i] = texture;
						this.loadedTextures++;

						if (this.loadedTextures == textureCount) {
							this.ready = true;
						}
					});
				});
			}
		} else if (this.textureName) {
			Client.loadFile(`data/texture/${this.textureName}`, buffer => {
				WebGL.texture(gl, buffer, texture => {
					this.texture = texture;
					this.ready = true;
				});
			});
		} else {
			this.ready = true;
		}
	}

	free(gl) {
		this.ready = false;
	}

	render(gl, tick) {
		if (this.startTick > tick) {
			return;
		} //not yet

		if (this.blendMode > 0 && this.blendMode < 16) {
			gl.blendFunc(gl.SRC_ALPHA, blendMode[this.blendMode]);
		} else {
			gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		}

		const start = tick - this.startTick;
		const end = this.endTick - this.startTick;
		let steps = (start / end) * 100;

		if (steps > 100) {
			steps = 100;
		}

		if (!this.spriteRessource) {
			if (this.shadowTexture) {
				this.spriteRessource = Client.loadFile('data/sprite/shadow.spr');
				this.actRessource = Client.loadFile('data/sprite/shadow.act');
			} else if (this.absoluteSpriteName) {
				this.spriteRessource = Client.loadFile(`${this.absoluteSpriteName}.spr`, null, null, { to_rgba: true });
				this.actRessource = Client.loadFile(`${this.absoluteSpriteName}.act`, null, null, { to_rgba: true });
			} else if (this.spriteName) {
				this.spriteRessource = Client.loadFile(`data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/${this.spriteName}.spr`);
				this.actRessource = Client.loadFile(`data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/${this.spriteName}.act`);
			}
		}

		if (this.spriteRessource) {
			this.texture = this.spriteRessource.frames[0].texture;
		}

		if (this.textureList.length > 0) {
			const frame = Math.floor((tick - this.startTick) / this.frameDelay) % this.textureList.length;
			this.texture = this.textureList[frame];
		}

		SpriteRenderer.image.texture = this.texture;
		SpriteRenderer.zIndex = this.zIndex;

		let posDelta = 0;

		if (this.rotatePosX > 0) {
			posDelta =
				this.rotatePosX *
				Math.cos((steps * 3.5 * this.nbOfRotation * Math.PI) / 180 - (this.rotateLate * Math.PI) / 2);
			if (this.rotationClockwise) {
				posDelta = -1 * posDelta;
			}
		} else {
			if (this.posxSmooth) {
				if (this.posxStart != this.posxEnd) {
					const csJ = steps * 0.09 + 1;
					const csK = Math.log10(csJ);
					const csL = this.posxEnd - this.posxStart;
					const csM = this.posxStart;
					const csN = csK * csL + csM;
					posDelta = csN;
				} else {
					posDelta = this.posxStart;
				}
			} else {
				if (this.posxStart != this.posxEnd) {
					const csL = (this.posxEnd - this.posxStart) / 100;
					const csM = this.posxStart;
					const csN = steps * csL + csM;
					posDelta = csN;
				} else {
					posDelta = this.posxStart;
				}
			}
		}
		SpriteRenderer.position[0] = this.position[0] + posDelta;
		posDelta = 0;

		if (this.rotatePosY > 0) {
			posDelta =
				this.rotatePosY *
				Math.sin((steps * 3.5 * this.nbOfRotation * Math.PI) / 180 - (this.rotateLate * Math.PI) / 2);
		} else {
			if (this.posySmooth) {
				if (this.posyStart != this.posyEnd) {
					const csJ = steps * 0.09 + 1;
					const csK = Math.log10(csJ);
					const csL = this.posyEnd - this.posyStart;
					const csM = this.posyStart;
					const csN = csK * csL + csM;
					posDelta = csN;
				} else {
					posDelta = this.posyStart;
				}
			} else {
				if (this.posyStart != this.posyEnd) {
					const csL = (this.posyEnd - this.posyStart) / 100;
					const csM = this.posyStart;
					const csN = steps * csL + csM;
					posDelta = csN;
				} else {
					posDelta = this.posyStart;
				}
			}
		}
		SpriteRenderer.position[1] = this.position[1] + posDelta;
		posDelta = 0;

		if (this.retreat !== 0) {
			let linearX, linearY;

			if (this.posxStart != this.posxEnd) {
				linearX = steps * ((this.posxEnd - this.posxStart) / 100) + this.posxStart;
			} else {
				linearX = this.posxStart;
			}

			if (this.posyStart != this.posyEnd) {
				linearY = steps * ((this.posyEnd - this.posyStart) / 100) + this.posyStart;
			} else {
				linearY = this.posyStart;
			}

			let dx = this.posxEnd - this.posxStart;
			let dy = this.posyEnd - this.posyStart;
			const dist = Math.sqrt(dx * dx + dy * dy);

			if (dist > 0.001) {
				dx = dx / dist;
				dy = dy / dist;
				const retreatFactor = Math.sin((steps * Math.PI) / 100) * this.retreat;
				linearX = linearX - dx * retreatFactor;
				linearY = linearY - dy * retreatFactor;
			}
			SpriteRenderer.position[0] = this.position[0] + linearX;
			SpriteRenderer.position[1] = this.position[1] + linearY;
		}

		if (this.poszSmooth) {
			if (this.poszStart != this.poszEnd) {
				const csJ = steps * 0.09 + 1;
				const csK = Math.log10(csJ);
				const csL = this.poszEnd - this.poszStart;
				const csM = this.poszStart;
				const csN = csK * csL + csM;
				posDelta = csN;
			} else {
				posDelta = this.poszStart;
			}
		} else {
			if (this.poszStart != this.poszEnd) {
				const csL = (this.poszEnd - this.poszStart) / 100;
				const csM = this.poszStart;
				const csN = steps * csL + csM;
				posDelta = csN;
			} else {
				posDelta = this.poszStart;
			}
		}

		if (this.arc !== 0) {
			posDelta += this.arc * Math.sin((steps * Math.PI) / 100);
		}

		SpriteRenderer.position[2] = this.position[2] + posDelta;

		if (this.shadowTexture) {
			SpriteRenderer.position[2] = Altitude.getCellHeight(SpriteRenderer.position[0], SpriteRenderer.position[1]);
		}

		let alpha = this.alphaMax;

		if (this.fadeIn && start < end / 4) {
			alpha = (start * this.alphaMax) / (end / 4);
		} else if (this.fadeOut && start > end / 2 + end / 4) {
			alpha = ((end - start) * this.alphaMax) / (end / 4);
		} else if (this.sparkling) {
			alpha = this.alphaMax * ((Math.cos((steps * 11 * this.sparkNumber * Math.PI) / 180) + 1) / 2);
		}

		if (alpha < 0) {
			alpha = 0.0;
		} else if (alpha > 1) {
			alpha = 1.0;
		}

		SpriteRenderer.color[3] = alpha;
		SpriteRenderer.color[0] = this.red;
		SpriteRenderer.color[1] = this.green;
		SpriteRenderer.color[2] = this.blue;
		let sizeX, sizeY;

		if (this.sizeSmooth) {
			if (this.sizeEndX != this.sizeStartX) {
				const csJ = steps * 0.09 + 1;
				const csK = Math.log10(csJ);
				const csL = this.sizeEndX - this.sizeStartX;
				const csM = this.sizeStartX;
				const csN = csK * csL + csM;
				sizeX = csN;
			} else {
				sizeX = this.sizeStartX;
			}
			if (this.sizeEndY != this.sizeStartY) {
				const ctf = steps * 0.09 + 1;
				const ctg = Math.log10(ctf);
				const cth = this.sizeEndY - this.sizeStartY;
				const cti = this.sizeStartY;
				const ctj = ctg * cth + cti;
				sizeY = ctj;
			} else {
				sizeY = this.sizeStartY;
			}
		} else {
			if (this.sizeEndX != this.sizeStartX) {
				const csL = (this.sizeEndX - this.sizeStartX) / 100;
				const csM = this.sizeStartX;
				const csN = steps * csL + csM;
				sizeX = csN;
			} else {
				sizeX = this.sizeStartX;
			}
			if (this.sizeEndY != this.sizeStartY) {
				const cth = (this.sizeEndY - this.sizeStartY) / 100;
				const cti = this.sizeStartY;
				const ctj = steps * cth + cti;
				sizeY = ctj;
			} else {
				sizeY = this.sizeStartY;
			}
		}

		SpriteRenderer.size[0] = sizeX;
		SpriteRenderer.size[1] = sizeY;

		if (this.rotate) {
			const angleStep = (this.toAngle - this.angle) / 100;
			const startAngle = this.angle;
			const angle = steps * angleStep + startAngle;
			SpriteRenderer.angle = this.rotateWithCamera ? angle + Camera.angle[1] : angle;
		} else {
			SpriteRenderer.angle = this.rotateWithCamera ? this.angle + Camera.angle[1] : this.angle;
		}

		if (this.actRessource && this.spriteRessource) {
			let entity = this.ownerEntity;
			if (!entity) {
				entity = new Entity();
				entity.GID = this.AID;
				entity.position = this.position;
				entity.objecttype = entity.constructor.TYPE_EFFECT;
				entity.hideShadow = true;
				EntityManager.add(entity);
				this.ownerEntity = entity;
			}
			const actions =
				this.actRessource.actions[
					(entity.action * 8 + ((Camera.direction + entity.direction + 8) % 8)) %
						this.actRessource.actions.length
				];
			let animations;
			const delay = this.sprDelay || actions.delay;
			if (this.playSprite) {
				animations =
					actions.animations[Math.floor((tick - this.startTick) / delay) % actions.animations.length];
			} else {
				animations = actions.animations[0];
			}
			const layers = animations.layers;
			let i = 0;
			const layercount = layers.length;
			do {
				let renderer;

				if (i == 0) {
					renderer = SpriteRenderer;
				} else {
					renderer = Object.assign({}, SpriteRenderer);
				}

				// Reset channels every layer to effect global color
				renderer.color[0] = this.red;
				renderer.color[1] = this.green;
				renderer.color[2] = this.blue;
				renderer.color[3] = alpha;

				const layer = layers[i];
				const ctE = new Int16Array(2);
				ctE[0] = 0;
				ctE[1] = 0;
				if (animations.pos.length) {
					ctE[0] = this.position[0] - animations.pos[0].x;
					ctE[1] = this.position[1] - animations.pos[0].y;
				}
				renderer.image.texture = this.spriteRessource.frames[layer.index].texture;
				renderer.image.palette = null;
				renderer.sprite = this.spriteRessource.frames[layer.index];
				renderer.palette = this.spriteRessource.palette;
				let ctF = layer.index + 0;
				const ctG = layer.spr_type === 1 || this.spriteRessource.rgba_index === 0;
				if (!ctG) {
					renderer.image.palette = this.spriteRessource.texture;
					renderer.image.size[0] = this.spriteRessource.frames[ctF].width;
					renderer.image.size[1] = this.spriteRessource.frames[ctF].height;
				} else if (layer.spr_type === 1) {
					ctF += this.spriteRessource.old_rgba_index;
				}
				const frame = this.spriteRessource.frames[ctF];
				let width = frame.width;
				let height = frame.height;
				width *= (layer.scale[0] / 100) * sizeX;
				height *= (layer.scale[1] / 100) * sizeY;
				if (layer.is_mirror) {
					width = -width;
				}
				renderer.color[0] *= layer.color[0];
				renderer.color[1] *= layer.color[1];
				renderer.color[2] *= layer.color[2];
				renderer.color[3] *= layer.color[3];
				if (!this.rotateToTarget) {
					renderer.angle += layer.angle;
				}
				renderer.offset[0] = layer.pos[0] + ctE[0];
				renderer.offset[1] = layer.pos[1] + ctE[1];
				renderer.size[0] = width;
				renderer.size[1] = height;

				// default true, false, false
				SpriteRenderer.runWithDepth(this.overlay === false, false, this.overlay === true, () => {
					SpriteRenderer.render();
				});

				++i;
			} while (i < layercount);
		} else {
			// default true, false, false
			SpriteRenderer.runWithDepth(this.overlay === false, false, this.overlay === true, () => {
				SpriteRenderer.render();
			});
		}

		this.needCleanUp = this.endTick < tick;
	}

	static init(gl) {
		blendMode[1] = gl.ZERO;
		blendMode[2] = gl.ONE;
		blendMode[3] = gl.SRC_COLOR;
		blendMode[4] = gl.ONE_MINUS_SRC_COLOR;
		blendMode[5] = gl.DST_COLOR;
		blendMode[6] = gl.ONE_MINUS_DST_COLOR;
		blendMode[7] = gl.SRC_ALPHA;
		blendMode[8] = gl.ONE_MINUS_SRC_ALPHA;
		blendMode[9] = gl.DST_ALPHA;
		blendMode[10] = gl.ONE_MINUS_DST_ALPHA;
		blendMode[11] = gl.CONSTANT_COLOR;
		blendMode[12] = gl.ONE_MINUS_CONSTANT_COLOR;
		blendMode[13] = gl.CONSTANT_ALPHA;
		blendMode[14] = gl.ONE_MINUS_CONSTANT_ALPHA;
		blendMode[15] = gl.SRC_ALPHA_SATURATE;

		this.ready = true;
		this.renderBeforeEntities = false;
	}

	static free(gl) {
		this.ready = false;
	}

	static beforeRender(gl, modelView, projection, fog, tick) {
		SpriteRenderer.bind3DContext(gl, modelView, projection, fog);
		SpriteRenderer.shadow = 1;
		SpriteRenderer.angle = 0;
		SpriteRenderer.size[0] = 100;
		SpriteRenderer.size[1] = 100;
		SpriteRenderer.offset[0] = 0;
		SpriteRenderer.offset[1] = 0;
		SpriteRenderer.image.palette = null;
		SpriteRenderer.color[0] = 1;
		SpriteRenderer.color[1] = 1;
		SpriteRenderer.color[2] = 1;
		SpriteRenderer.color[3] = 1;
		SpriteRenderer.depth = 0;
		SpriteRenderer.zIndex = 0;
	}

	static afterRender(gl) {
		gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
		SpriteRenderer.unbind(gl);
	}
}
export default ThreeDEffect;
