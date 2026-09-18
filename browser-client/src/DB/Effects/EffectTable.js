/**
 * @module DB/Effects/EffectTable
 *
 * List effects
 * TODO: complete the list, add informations about sound.
 *
 * @author Vincent Thibault
 */

import RainWeatherEffect from 'Renderer/Effects/RainWeather.js';
import SnowWeatherEffect from 'Renderer/Effects/SnowWeather.js';
import SakuraWeatherEffect from 'Renderer/Effects/SakuraWeatherEffect.js';
import Camera from 'Renderer/Camera.js';
import SwirlingAura from 'Renderer/Effects/SwirlingAura.js';
import GroundAura from 'Renderer/Effects/GroundAura.js';
import Level99Bubble from 'Renderer/Effects/Level99Bubble.js';

import SongEffects from 'Renderer/Effects/Songs.js';
import SoundManager from 'Audio/SoundManager.js';
import Events from 'Core/Events.js';
import DB from 'DB/DBManager.js';
import Session from 'Engine/SessionStorage.js';
import CloudWeatherEffect from 'Renderer/Effects/CloudWeatherEffect.js';
import FlatColorTile from 'Renderer/Effects/FlatColorTile.js';
import LockOnTarget from 'Renderer/Effects/LockOnTarget.js';
import LPEffect from 'Renderer/Effects/LPEffect.js';
import MagicRing from 'Renderer/Effects/MagicRing.js';
import MagicTarget from 'Renderer/Effects/MagicTarget.js';
import Poison from 'Renderer/Effects/PoisonEffect.js';
import PokJukWeatherEffect from 'Renderer/Effects/PokJukWeatherEffect.js';
import PropertyGround from 'Renderer/Effects/PropertyGround.js';
import QuadHorn from 'Renderer/Effects/QuadHorn.js';
import Blind from 'Renderer/Effects/Shaders/Blind.js';
import SpiderWeb from 'Renderer/Effects/SpiderWeb.js';
import SpiritSphere from 'Renderer/Effects/SpiritSphere.js';
import WarlockSphere from 'Renderer/Effects/WarlockSphere.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Renderer from 'Renderer/Renderer.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';

const ServiceEffects = SongEffects.ServiceEffects;
const getBragiSpellNote = SongEffects.getBragiSpellNote;
const DissonanceEffects = SongEffects.DissonanceEffects;
const LullabyEffects = SongEffects.LullabyEffects;
const MrKimEffects = SongEffects.MrKimEffects;
const EtChaosEffects = SongEffects.EtChaosEffects;
const DrumEffects = SongEffects.DrumEffects;
const NibelungEffects = SongEffects.NibelungEffects;
const LokiEffects = SongEffects.LokiEffects;
const AbyssEffects = SongEffects.AbyssEffects;
const SiegfiedEffects = SongEffects.SiegfiedEffects;
const WhistleEffects = SongEffects.WhistleEffects;
const SinEffects = SongEffects.SinEffects;
const AppleEffects = SongEffects.AppleEffects;
const UglyEffects = SongEffects.UglyEffects;
const HummingEffects = SongEffects.HummingEffects;
const ForgetEffects = SongEffects.ForgetEffects;
const FortuneEffects = SongEffects.FortuneEffects;
const GospelEffects = SongEffects.GospelEffects;
const FogEffects = SongEffects.FogEffects;
const GravityEffects = SongEffects.GravityEffects;
const EvillandEffects = SongEffects.EvillandEffects;

export default {
	/// Common parameters
	///
	/// - duration:
	///   specifies the duration of the effect
	///
	/// - duplicate:
	///   the EffectManager duplicates the effect this many times
	///
	/// - timeBetweenDupli:
	///   if the duplicate param is specified, then the EffectManager waits for this long between duplicates
	///
	/// - wav:
	///   audio file stored in data/wav/ folder
	///
	/// - attachedEntity:
	///   if set to true, the effect will follow the entity attached

	/// type = STR
	///
	/// - file:
	///   STR file name stored in data/texture/effect/(.*).str
	///
	/// - min:
	///   minify str file stored in data/texture/effect/(.*).str
	///   used when /mineffect is enabled
	///
	/// - rand
	///   replace the %d in the file name with a rand(val1, val2).
	///
	/// - texturePath:
	///   path to look for the textures
	///   instead of the regular data/texture/effect path

	/// type = SPR
	///
	/// - file:
	///   Sprite file name stored in data/sprite/AIANA®/(.*).spr
	///
	/// - head
	///   if set to true, the sprite will be at the character's head
	///
	/// - stopAtEnd
	///   do not remove when animation end
	///
	/// - direction
	///   if set to true, the sprite will inherit character's direction

	/// type = FUNC
	///
	/// - func:
	///   callback to use

	/// type = CYLINDER
	///
	/// - textureName:
	///   name of the targa texture stored in data/texture/effect/(.*).tga
	///
	/// - red, green, blue:
	///   overrides the color values of the texture (0.0 ~ 1.0)
	///
	/// - totalCircleSides:
	///   number of sides of the base "circle" of the cylinder
	///
	/// - circleSides:
	///   number of sides shown out of the totalCircleSides
	///
	/// - repeatTextureX:
	///   how many times the texture is repeated on the side of the cylinder (the whole 360 turn regardless of how many sides are actually visible)
	///   use it to fit the texture to sides or semi-circles
	///
	/// - rotate:
	///   if set to true, the cylinder will continously rotate around it's Y (vertical) axis before any other rotation is applied
	///
	/// - angleX, angleY, angleZ:
	///   rotates the cylinder on the X, Y, Z axis by the set amount of degrees (applied after rotate)
	///
	/// - rotateWithCamera
	///   if set to true the cylinder rotates with the camera on the final Y (vertical) axis after every other rotation is applied
	///
	/// - rotateToTarget
	///   if set to true the notrh side of the cylinder is rotated to face the target
	///
	/// - rotateWithSource
	///   if set to true the notrh side of the cylinder will be rotated to match the source's direction
	///
	/// - topSize:
	///   top radius of the circle
	///
	/// - bottomSize:
	///   bottom radius of the circle
	///
	/// - height:
	///   height of the cylinder
	///
	/// - posX, posY, posZ:
	///   offsets the cylinder's position on it's X, Y, Z axis
	///
	/// - fade:
	///   if set to true the cylinder will fade out at the end of the duration
	///
	/// - alphaMax:
	///   sets the opacity of the cylinder (0.0 ~ 1.0)
	///
	/// - animation:
	///   sets the animation type of the cylinder
	///	 - 1: the height of the cylinder grows to the set height from 0
	///	 - 2: top radius of the cylinder grows to the topSize value from 0
	///	 - 3: the radius of the whole cylinder shrinks to 0
	///	 - 4: the radius of the whole cylinder grows to the set topSize and bottomSize values from 0
	///	 - 5: the height of the cylinder grows to the set height value, and then shrinks back to 0
	///
	/// - blendMode:
	///   sets the webgl blendFunc target mode of the cylinder
	///	 - 1: ZERO
	///	 - 2: ONE (transparent light effect)
	///	 - 3: SRC_COLOR
	///	 - 4: ONE_MINUS_SRC_COLOR
	///	 - 5: DST_COLOR
	///	 - 6: ONE_MINUS_DST_COLOR
	///	 - 7: SRC_ALPHA
	///	 - 8: ONE_MINUS_SRC_ALPHA (default)
	///	 - 9: DST_ALPHA
	///	 - 10: ONE_MINUS_DST_ALPHA
	///	 - 11: CONSTANT_COLOR
	///	 - 12: ONE_MINUS_CONSTANT_COLOR
	///	 - 13: CONSTANT_ALPHA
	///	 - 14: ONE_MINUS_CONSTANT_ALPHA
	///	 - 15: SRC_ALPHA_SATURATE
	///   the webgl blendFunc source mode is always SRC_ALPHA
	///
	/// - repeat
	///   if set to true the effect will play repeatedly until removed

	/// type = 2D
	///
	/// - file:
	///   Texture file name stored in data/texture/(.*.bmp|tga)
	///
	/// - wav:
	///   audio file stored in data/wav/ folder
	///
	/// - red:
	///   if set to >0, overrides the red color of the texture
	///
	/// - green:
	///   if set to >0, overrides the green color of the texture
	///
	/// - blue:
	///   if set to >0, overrides the blue color of the texture
	///
	/// - alphaMax:
	///   sets the opacity of the texture (0.0 ~ 1.0)
	///
	/// - fadeIn:
	///   if set to true the texture will fade in at the beginning of the duration
	///
	/// - fadeOut:
	///   if set to true the texture will fade out at the end of the duration
	///
	/// - fadeIn:
	///   if set to true the texture will fade in at the beginning of the duration
	///
	/// - posx, posy, posz:
	///   sets the relative position of the texture
	///
	/// - posxStart, posyStart, poszStart, posxEnd, posyEnd, poszEnd:
	///   sets the relative starting and ending position of the texture
	///
	/// - posxRand, posyRand, poszRand:
	///   sets a +- range for a random relative position
	///
	/// - posxRandDiff, posyRandDiff, poszRandDiff:
	///   sets a +- range for a random relative starting and ending position. The start and end is 2 different random numbers in the same range.
	///
	/// - posxStartRand, posyStartRand, poszStartRand, posxEndRand, posyEndRand, poszEndRand:
	///   sets a +- range for a random relative starting and ending position
	///
	/// - posxSmooth, posySmooth, poszSmooth:
	///   smoohtes out the movement on an axis
	///
	/// - size, sizeX, sizeY, sizeStart, sizeEnd, sizeStartX, sizeStartY, sizeEndX, sizeEndY:
	/// - sizeRand, sizeRandx, sizeEndY:
	/// - sizeSmooth:
	///   works the same way as positions, but effects the size of the texture
	///
	/// - poszStartRandMiddle, poszEndRandMiddle:
	///   sets the relative middle position of the random range of the starting and ending Z position
	///
	/// - sizeRandXMiddle, sizeRandYMiddle:
	///   sets the middle value for the random range for the size X&Y values
	///
	/// - rotate:
	///   if set to true makes the texture rotate on it's y axis (turn around)
	///
	/// - angle:
	///   sets the starting angle of the texture in degrees (eg: 180=upside down)
	///
	/// - toAngle:
	///   sets the final angle of the texture in degrees (eg: 180=upside down)
	///
	/// - zIndex:
	///   sets the zindex of the texture (closer to the camera or farther)

	/// type = 3D
	///
	/// - file:
	///   Texture file name stored in data/texture/(.*.bmp|tga)
	///
	/// - fileList:
	///   An array of texture file name stored in data/texture/(.*.bmp|tga)
	///
	/// - frameDelay:
	///   For how many ticks (time) one texture is shown when using fileList. Used for "hand animated" effects that are created from multiple textures.
	///
	/// - delayStart:
	///   delays the visual start of this effect entry by the given number of
	///   milliseconds. Sound (wav) is NOT delayed — it plays at the original startTick.
	///
	/// - spriteName:
	///   Sprite file name stored in data/sprite/AIANA®/(.*).spr
	///
	/// - absoluteSpriteName:
	///   Sprite file name stored in the absolute path.spr
	///   eg: data/sprite/npc/skel_archer_arrow.spr
	///
	/// - playSprite:
	///   if set to true plays the sprite animation
	///
	/// - sprDelay:
	///   frame delay for a sprite animation
	///
	/// - wav:
	///   audio file stored in data/wav/ folder
	///
	/// - red:
	///   if set to >0, overrides the red color of the texture
	///
	/// - green:
	///   if set to >0, overrides the green color of the texture
	///
	/// - blue:
	///   if set to >0, overrides the blue color of the texture
	///
	/// - alphaMax:
	///   sets the opacity of the texture (0.0 ~ 1.0)
	///
	/// - alphaMaxDelta:
	///   when using duplicates, increase alpaMax value between duplicates by this amount
	///
	/// - fadeIn:
	///   if set to true the texture will fade in at the beginning of the duration
	///
	/// - fadeOut:
	///   if set to true the texture will fade out at the end of the duration
	///
	/// - fadeIn:
	///   if set to true the texture will fade in at the beginning of the duration
	///
	/// - posx, posy, posz:
	///   sets the relative position of the texture
	///
	/// - posxStart, posyStart, poszStart, posxEnd, posyEnd, poszEnd:
	///   sets the relative starting and ending position of the texture
	///
	/// - posxRand, posyRand, poszRand:
	///   sets a +- range for a random relative position
	///
	/// - posxRandDiff, posyRandDiff, poszRandDiff:
	///   sets a +- range for a random relative starting and ending position. The start and end is 2 different random numbers in the same range.
	///
	/// - posxStartRand, posyStartRand, poszStartRand, posxEndRand, posyEndRand, poszEndRand:
	///   sets a +- range for a random relative starting and ending position
	///
	/// - posxSmooth, posySmooth, poszSmooth:
	///   smoohtes out the movement on an axis
	///
	/// - size, sizeX, sizeY, sizeStart, sizeEnd, sizeStartX, sizeStartY, sizeEndX, sizeEndY:
	/// - sizeRand, sizeRandx, sizeEndY:
	/// - sizeSmooth:
	///   works the same way as positions, but effects the size of the texture
	///
	/// - sizeDelta
	///   when using duplicates, increase sizeDelta value between duplicates by this amount
	///
	/// - posxStartRandMiddle, posyStartRandMiddle, poszStartRandMiddle, posxEndRandMiddle, posyEndRandMiddle, poszEndRandMiddle:
	///   sets the relative middle position of the random range of the starting and ending
	///
	/// - sizeRandXMiddle, sizeRandYMiddle:
	///   sets the middle value for the random range for the size X&Y values
	///
	/// - zOffsetStart:
	///   initial Z offset when using fromSrc or toSrc
	///
	/// - zOffsetEnd:
	///   final Z offset when using fromSrc or toSrc
	///
	/// - arc:
	///   creates an arced trajectory using sine wave
	///   positive values create upward arc, negative values create downward arc
	///
	/// - retreat:
	///   makes projectile retreat backward before moving forward
	///   creates a "pull back" effect in the middle of the trajectory
	///
	/// - rotate:
	///   if set to true makes the texture rotate on it's y axis (turn around)
	///
	/// - rotatePosX, rotatePosY, rotatePosZ:
	///   offsets the rotation axis
	///
	/// - nbOfRotation:
	///   sets the number of rotations
	///
	/// - rotateLate:
	///   sets a delay before the rotation
	///
	/// - rotateLateDelta:
	///   when using duplicates, increase rotateLate value between duplicates by this amount
	///
	/// - rotationClockwise:
	///   if set to true then rotates clockwise
	///
	/// - angle:
	///   sets the starting angle of the texture in degrees (eg: 180=upside down)
	///
	/// - toAngle:
	///   sets the final angle of the texture in degrees (eg: 180=upside down)
	///
	/// - rotateToTarget:
	///   if set to true the upper side of the texture is rotated to face the target
	///
	/// - rotateWithCamera:
	///   if set to true the texture rotates if the camera is rotated makig it's sides always face the same coordinate in the 3D environment
	///
	/// - zIndex:
	///   sets the zindex of the texture (closer to the camera or farther)
	///
	/// - shadowTexture:
	///   if set to true then enables the shadow
	///
	/// - blendMode:
	///   sets the webgl blendFunc target mode of the cylinder
	///	 - 1: ZERO
	///	 - 2: ONE (transparent light effect)
	///	 - 3: SRC_COLOR
	///	 - 4: ONE_MINUS_SRC_COLOR
	///	 - 5: DST_COLOR
	///	 - 6: ONE_MINUS_DST_COLOR
	///	 - 7: SRC_ALPHA
	///	 - 8: ONE_MINUS_SRC_ALPHA (default)
	///	 - 9: DST_ALPHA
	///	 - 10: ONE_MINUS_DST_ALPHA
	///	 - 11: CONSTANT_COLOR
	///	 - 12: ONE_MINUS_CONSTANT_COLOR
	///	 - 13: CONSTANT_ALPHA
	///	 - 14: ONE_MINUS_CONSTANT_ALPHA
	///	 - 15: SRC_ALPHA_SATURATE
	///   the webgl blendFunc source mode is always SRC_ALPHA
	///
	/// - sparkling
	///   if set to true then makes the effect blink/sparkle
	///
	/// - sparkNumber
	///   sets how often the effect blinks/sparkles
	///
	/// - repeat
	///   if set to true the effect will play repeatedly until removed

	// Song requirements
	// (moved to top imports)

	// return { // changed to export default { at the top

	0: [
		{
			//EF_HIT1	Regular hit
			type: '3D',
			duplicate: 4,
			timeBetweenDupli: 0,
			file: 'effect/pok3.tga',
			duration: 300,
			alphaMax: 0.8,
			alphaMin: 0.3,
			fadeIn: true,
			fadeOut: true,
			poszStart: 1,
			posxStart: 0,
			posyStart: 0,
			posxEndRand: 2,
			posyEndRand: 2,
			poszEndRand: 2,
			zIndex: 1,
			size: 10,
			red: 1,
			green: 1,
			blue: 1,
			sizeRand: 20,
			sizeSmooth: true,
			attachedEntity: false,
			sparkling: true
		}
	],

	1: [
		{
			alphaMax: 12,
			angleRand: [0, 35],
			attachedEntity: false,
			duration: 250,
			durationRand: [200, 350],
			fadeOut: true,
			fade: true,
			file: 'effect/lens1.tga',
			sizeEndX: 1,
			sizeRandEndY: [250, 300],
			sizeRandStartX: [25, 40],
			sizeRandStartY: [10, 10],
			circlePattern: true,
			circleOuterSizeRand: [5, 6],
			circleInnerSize: 2.2,
			type: '2D',
			overlay: true,
			zIndex: 1,
			wav: 'effect/ef_hit2'
		},
		{
			alphaMax: 12,
			angleRand: [50, 85],
			attachedEntity: false,
			duration: 250,
			durationRand: [200, 350],
			fadeOut: true,
			fade: true,
			file: 'effect/lens2.tga',
			sizeEndX: 1,
			sizeRandEndY: [250, 300],
			sizeRandStartX: [25, 40],
			sizeRandStartY: [10, 10],
			circlePattern: true,
			circleOuterSizeRand: [5, 6],
			circleInnerSize: 2.2,
			type: '2D',
			overlay: true,
			zIndex: 1
		},
		{
			alphaMax: 12,
			angleRand: [100, 135],
			attachedEntity: false,
			duration: 250,
			durationRand: [200, 350],
			fadeOut: true,
			fade: true,
			file: 'effect/lens1.tga',
			sizeEndX: 1,
			sizeRandEndY: [250, 300],
			sizeRandStartX: [25, 40],
			sizeRandStartY: [10, 10],
			circlePattern: true,
			circleOuterSizeRand: [5, 6],
			circleInnerSize: 2.2,
			type: '2D',
			overlay: true,
			zIndex: 1
		},
		{
			alphaMax: 12,
			angleRand: [150, 185],
			attachedEntity: false,
			duration: 250,
			durationRand: [200, 350],
			fadeOut: true,
			fade: true,
			file: 'effect/lens2.tga',
			sizeEndX: 1,
			sizeRandEndY: [250, 300],
			sizeRandStartX: [25, 40],
			sizeRandStartY: [10, 10],
			circlePattern: true,
			circleOuterSizeRand: [5, 6],
			circleInnerSize: 2.2,
			type: '2D',
			overlay: true,
			zIndex: 1
		},
		{
			alphaMax: 12,
			angleRand: [200, 235],
			attachedEntity: false,
			duration: 250,
			durationRand: [200, 350],
			fadeOut: true,
			fade: true,
			file: 'effect/lens1.tga',
			sizeEndX: 1,
			sizeRandEndY: [250, 300],
			sizeRandStartX: [25, 40],
			sizeRandStartY: [10, 10],
			circlePattern: true,
			circleOuterSizeRand: [5, 6],
			circleInnerSize: 2.2,
			type: '2D',
			overlay: true,
			zIndex: 1
		},
		{
			alphaMax: 12,
			angleRand: [255, 290],
			attachedEntity: false,
			duration: 250,
			durationRand: [200, 350],
			fadeOut: true,
			fade: true,
			file: 'effect/lens2.tga',
			sizeEndX: 1,
			sizeRandEndY: [250, 300],
			sizeRandStartX: [25, 40],
			sizeRandStartY: [10, 10],
			circlePattern: true,
			circleOuterSizeRand: [5, 6],
			circleInnerSize: 2.2,
			type: '2D',
			overlay: true,
			zIndex: 1
		},
		{
			alphaMax: 12,
			angleRand: [300, 335],
			attachedEntity: false,
			duration: 250,
			durationRand: [200, 350],
			fadeOut: true,
			fade: true,
			file: 'effect/lens1.tga',
			sizeEndX: 1,
			sizeRandEndY: [250, 300],
			sizeRandStartX: [25, 40],
			sizeRandStartY: [10, 10],
			circlePattern: true,
			circleOuterSizeRand: [5, 6],
			circleInnerSize: 2.2,
			type: '2D',
			overlay: true,
			zIndex: 1
		},
		{
			alphaMax: 12,
			angleRand: [340, 360],
			attachedEntity: false,
			duration: 250,
			durationRand: [200, 350],
			fadeOut: true,
			fade: true,
			file: 'effect/lens2.tga',
			sizeEndX: 1,
			sizeRandEndY: [250, 300],
			sizeRandStartX: [25, 40],
			sizeRandStartY: [10, 10],
			circlePattern: true,
			circleOuterSizeRand: [5, 6],
			circleInnerSize: 2.2,
			type: '2D',
			overlay: true,
			zIndex: 1
		}
	],

	2: [
		{
			//EF_HIT3	Melee Skill Hit
			type: 'CYLINDER',
			textureName: 'lens2',
			angleX: -90,
			posZ: 1,
			rotateWithCamera: true,
			bottomSize: 0.37,
			topSize: 1,
			height: 4,
			animation: 1,
			fade: true,
			duration: 150,
			alphaMax: 0.8,
			wav: 'effect/ef_hit3',
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'lens2',
			angleX: -90,
			posZ: 1,
			rotateWithCamera: true,
			bottomSize: 0.37,
			topSize: 0.37,
			height: 4,
			animation: 1,
			fade: true,
			duration: 150,
			alphaMax: 0.8,
			wav: 'effect/ef_hit3',
			attachedEntity: true
		}
	],

	3: [
		{
			//EF_HIT4	Melee Skill Hit
			type: 'CYLINDER',
			textureName: 'lens2',
			angleX: -90,
			posZ: 1,
			rotateWithCamera: true,
			bottomSize: 0.15,
			topSize: 1,
			height: 4,
			animation: 1,
			fade: true,
			duration: 150,
			alphaMax: 0.8,
			wav: 'effect/ef_hit4',
			attachedEntity: true
		}
	],

	4: [
		{
			//EF_HIT5	Melee Skill Hit
			alphaMax: 1,
			angle: 90,
			attachedEntity: false,
			duration: 400,
			fadeOut: true,
			file: 'effect/lens2.tga',
			posz: 1,
			rotate: true,
			sizeEndY: 200,
			sizeStartY: 10,
			sizeX: 15,
			toAngle: 0,
			type: '3D',
			wav: 'effect/ef_hit5'
		},
		{
			alphaMax: 1,
			angle: 180,
			attachedEntity: false,
			duration: 400,
			fadeOut: true,
			file: 'effect/lens2.tga',
			posz: 1,
			rotate: true,
			sizeEndY: 200,
			sizeStartY: 10,
			sizeX: 15,
			toAngle: 90,
			type: '3D',
			wav: 'effect/ef_hit5'
		}
	],

	5: [
		{
			//EF_HIT6	Melee Skill Hit
			alphaMax: 1,
			angle: 90,
			attachedEntity: false,
			duration: 400,
			fadeOut: true,
			file: 'effect/lens2.tga',
			posz: 1,
			rotate: true,
			sizeEndY: 150,
			sizeStartY: 10,
			sizeX: 10,
			toAngle: 0,
			type: '2D',
			overlay: true,
			wav: 'effect/ef_hit6'
		},
		{
			alphaMax: 1,
			angle: 180,
			attachedEntity: false,
			duration: 400,
			fadeOut: true,
			file: 'effect/lens2.tga',
			posz: 1,
			rotate: true,
			sizeEndY: 150,
			sizeStartY: 10,
			sizeX: 10,
			toAngle: 90,
			type: '2D',
			overlay: true,
			wav: 'effect/ef_hit6'
		}
	],

	6: [
		{
			//portal - entering the new map	//EF_ENTRY	Being Warped
			//type: 'FUNC',
			//file: 'effect/ring_blue',
			alphaMax: 0.62,
			animation: 1,
			attachedEntity: false,
			blendMode: 8,
			bottomSize: 0.9,
			duration: 500,
			fade: true,
			height: 7.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.9,
			type: 'CYLINDER',
			wav: 'effect/ef_portal'
		},
		{
			alphaMax: 0.62,
			animation: 1,
			attachedEntity: true,
			blendMode: 8,
			bottomSize: 0.85,
			duration: 500,
			fade: true,
			height: 8,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.85,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.8,
			animation: 1,
			attachedEntity: true,
			blendMode: 8,
			bottomSize: 0.9,
			duration: 500,
			fade: true,
			height: 1,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1.5,
			type: 'CYLINDER'
		}
	],

	7: [
		{
			//EF_EXIT	Item Heal effect
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 0.95,
			duration: 2000,
			fade: true,
			height: 10,
			textureName: 'alpha_down',
			topSize: 0.95,
			type: 'CYLINDER',
			wav: '_heal_effect'
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			duration: 1000,
			delayOffset: 400,
			duplicate: 6,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			posxRand: 1.5,
			posyRand: 1.5,
			poszEndRand: 3,
			poszEndRandMiddle: 6,
			poszStart: 0,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			timeBetweenDupli: 80,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			duration: 900,
			delayLate: 200,
			duplicate: 3,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			posxRand: 1,
			posyRand: 1,
			poszEnd: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 0,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			timeBetweenDupli: 200,
			type: '3D',
			zIndex: 1
		}
	],

	8: [
		{
			//EF_WARP	Yellow Ripple Effect
			alphaMax: 0.8,
			animation: 4,
			attachedEntity: true,
			bottomSize: 10,
			duration: 1000,
			duplicate: 4,
			fade: true,
			height: 0,
			posZ: 0.1,
			textureName: 'ring_yellow',
			timeBetweenDupli: 300,
			topSize: 13,
			type: 'CYLINDER'
		}
	],

	9: [
		{
			//EF_ENHANCE	Different Type of Heal
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 0.95,
			duration: 2000,
			fade: true,
			height: 10,
			textureName: 'alpha_down',
			topSize: 0.95,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			duration: 1000,
			delayLate: 500,
			duplicate: 7,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			duration: 1000,
			delayOffset: 400,
			duplicate: 3,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		}
	],

	10: [
		{
			//EF_COIN	Mammonite
			type: 'STR',
			file: 'maemor',
			min: 'memor_min',
			wav: 'effect/ef_coin2',
			attachedEntity: true
		}
	],

	11: [
		{
			//EF_ENDURE	Endure
			type: '3D',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/endure.tga',
			posz: 2,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 200,
			wav: 'effect/ef_endure',
			zIndex: 10
		}
	],

	12: [
		{
			//EF_BEGINSPELL	Yellow cast aura
			type: 'CYLINDER',
			alphaMax: 0.8,
			blendMode: 2,
			animation: 2,
			attachedEntity: true,
			bottomSize: 1,
			//duration: 1000,
			fade: true,
			height: 4,
			rotate: false,
			textureName: 'ring_yellow',
			topSize: 5,
			wav: 'effect/ef_beginspell'
		}
	],

	13: [
		{
			//EF_GLASSWALL	Blue Box
			type: 'STR',
			file: 'effect/safetywall',
			wav: 'effect/ef_glasswall',
			attachedEntity: false
		}
	],

	14: [
		{
			//EF_HEALSP	Blue restoring effect
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 0.95,
			duration: 2000,
			fade: true,
			height: 10,
			rotate: true,
			red: 0.1,
			green: 0.5,
			blue: 1,
			textureName: 'ring_blue',
			topSize: 0.95,
			type: 'CYLINDER',
			wav: '_heal_effect'
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			duration: 1000,
			delayOffset: 400,
			duplicate: 6,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			posxRand: 1.5,
			posyRand: 1.5,
			poszEndRand: 3,
			poszEndRandMiddle: 6,
			poszStart: 0,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			red: 0.1,
			green: 0.5,
			blue: 1,
			timeBetweenDupli: 80,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			duration: 900,
			delayLate: 200,
			duplicate: 3,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			posxRand: 1,
			posyRand: 1,
			poszEnd: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 0,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			red: 0.1,
			green: 0.5,
			blue: 1,
			timeBetweenDupli: 200,
			type: '3D',
			zIndex: 1
		}
	],

	15: [
		{
			//soul strike caster	//EF_SOULSTRIKE	Soul Strike
			alphaMax: 1,
			attachedEntity: true,
			blue: 1,
			duration: 200,
			delayLate: 250,
			//duplicate: -1,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			toSrc: true,
			green: 1,
			poszEnd: 1,
			poszSmooth: true,
			poszStartRand: 5,
			poszStartRandMiddle: 6,
			red: 1,
			size: 50,
			timeBetweenDupli: 150,
			type: '3D',
			wav: 'effect/ef_soulstrike',
			zIndex: 1
		},
		{
			type: '3D',
			duration: 250,
			duplicate: 5,
			timeBetweenDupli: 20,
			absoluteSpriteName: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/particle1',
			playSprite: true,
			toSrc: true,
			rotateToTarget: true,
			sizeStart: 100.0,
			sizeEnd: 500.0,
			zOffsetStart: 3,
			arc: 4.0,
			retreat: 3.0,
			soulStrikePattern: true
		}
	],

	16: [
		{
			//hide and monster body relocation sound	//EF_BASH	Hide
			type: 'CYLINDER',
			textureName: 'alpha_down',
			alphaMax: 0.6,
			duration: 1000,
			rotate: true,
			fade: true,
			angleX: -90,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.1,
			topSize: 2,
			animation: 2,
			zIndex: 1,
			wav: 'effect/ef_bash',
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 1000,
			duplicate: 10,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			rotate: true,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 2.5,
			animation: 2,
			zIndex: 1.1,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 1000,
			duplicate: 8,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			rotate: true,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 4,
			animation: 2,
			zIndex: 1.2,
			attachedEntity: true
		}
	],

	17: [
		{
			//EF_MAGNUMBREAK	Magnum Break
			type: 'CYLINDER',
			alphaMax: 0.7,
			animation: 4,
			attachedEntity: true,
			bottomSize: 4,
			duration: 300,
			fade: true,
			height: 1,
			rotate: true,
			textureName: 'ring_yellow',
			topSize: 6
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.6,
			animation: 4,
			attachedEntity: true,
			bottomSize: 4,
			duration: 300,
			fade: true,
			height: 4,
			rotate: true,
			textureName: '\xb4\xeb\xc6\xf8\xb9\xdf', //´ëÆø¹ß
			topSize: 1,
			wav: 'effect/ef_magnumbreak'
		}
	],

	18: [
		{
			//EF_STEAL	Steal
			alphaMax: 1,
			attachedEntity: false,
			red: 1,
			green: 1,
			blue: 0.85,
			duration: 500,
			duplicate: 7,
			timeBetweenDupli: 0,
			fadeOut: true,
			file: 'effect/pok1.tga',
			posxEndRand: 3.5,
			posyEndRand: 3.5,
			poszEndRand: 1,
			poszEndRandMiddle: 3,
			sizeEnd: 10,
			sizeStart: 200,
			type: '3D',
			zIndex: 10
		},
		{
			wav: 'effect/ef_steal'
		}
	],

	// 19: [{}],	//EF_HIDING	Invalid Effect ID Popup in client

	20: [
		{
			//EF_PATTACK	Envenom/Poison
			alphaMax: 1,
			attachedEntity: true,
			blue: 1,
			duration: 1000,
			duplicate: 10,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok1.tga',
			green: 0.7,
			posxRand: 1,
			posyRand: 1,
			poszEnd: 5,
			poszStart: 0,
			red: 1,
			size: 100,
			sizeRand: 20,
			type: '3D',
			zIndex: 1
		},
		{
			attachedEntity: true,
			wav: 'effect/ef_detoxication' //wav: 'effect/assasin_enchantpoison',
		}
	],

	21: [
		{
			//EF_DETOXICATION	Detoxify
			alphaMax: 1,
			attachedEntity: true,
			blue: 1,
			duration: 1000,
			duplicate: 10,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok1.tga',
			green: 1,
			posxRand: 1,
			posyRand: 1,
			poszEnd: 5,
			poszStart: 0,
			red: 0.7,
			size: 100,
			sizeRand: 20,
			type: '3D',
			zIndex: 1
		},
		{
			attachedEntity: true,
			wav: 'effect/ef_detoxication' //wav: 'effect/assasin_enchantpoison',
		}
	],

	22: [
		{
			//EF_SIGHT	Sight
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			attachedEntity: true,
			blendMode: 8,
			duration: 12200,
			duplicate: 10,
			timeBetweenDupli: 0,
			nbOfRotation: 10,
			posx: -2,
			posz: 4,
			playSprite: true,
			rotateLate: 0.9,
			rotateLateDelta: -0.1,
			rotatePosX: 3,
			rotatePosY: 3,
			rotationClockwise: true,
			size: 30,
			sizeDelta: 10
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			attachedEntity: true,
			blendMode: 8,
			duration: 12200,
			duplicate: 10,
			timeBetweenDupli: 0,
			nbOfRotation: 10,
			posx: -2,
			posz: 4,
			playSprite: true,
			rotateLate: 0.9,
			rotateLateDelta: -0.1,
			rotatePosX: 3,
			rotatePosY: 3,
			rotationClockwise: true,
			size: 60,
			sizeDelta: 20
		},
		{
			wav: 'effect/ef_sight'
		}
	],

	23: [
		{
			//EF_STONECURSE	Stone Curse
			type: 'STR',
			file: 'stonecurse',
			attachedEntity: true
		}
	],

	24: [
		{
			//fireball caster effect (on target effect 49:)	//EF_FIREBALL	Fire Ball
			type: '3D',
			alphaMax: 0.2,
			alphaMaxDelta: 0.2,
			attachedEntity: true,
			duration: 250,
			delayOffset: 160,
			delayOffsetDelta: -40,
			spriteName: 'fireball',
			playSprite: true,
			duplicate: 5,
			timeBetweenDupli: 0,
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			zOffset: 2,
			size: 200,
			zIndex: 1
		},
		{
			wav: 'effect/ef_fireball'
		}
	],

	25: [
		{
			//EF_FIREWALL	Fire Wall
			type: 'STR',
			file: 'firewall%d',
			rand: [1, 2],
			attachedEntity: false
		},
		{
			wav: 'effect/ef_firewall'
		}
	],

	26: [
		{
			//EF_ICEARROW	A sound (a swipe?)
			//type: 'FUNC',
			wav: 'effect/ef_icearrow%d', // Or ef_icearrow2 & ef_icearrow3 . Seems to be random
			rand: [1, 3],
			attachedEntity: false
		}
	],

	27: [
		{
			//Frost diver caster (ice traveling to target)	//EF_FROSTDIVER	Frost Diver (Traveling to Target)
			//type: 'FUNC',
			file: 'effect/ice',
			//wav: 'effect/ef_frostdiver1',
			attachedEntity: false
		}
	],

	28: [
		{
			//Frost Diver target hit	//EF_FROSTDIVER2	Frost Diver (Hitting)
			attachedEntity: true,
			file: 'freeze', //effect/ice.tga
			type: 'STR',
			wav: 'effect/ef_frostdiver2'
		}
	],

	29: [
		{
			//EF_LIGHTBOLT	Lightning Bolt
			attachedEntity: true,
			duplicate: 1,
			file: 'lightning',
			type: 'STR'
		},
		{
			attachedEntity: true,
			file: 'windhit%d',
			rand: [1, 3],
			type: 'STR'
		}
	],

	30: [
		{
			//EF_THUNDERSTORM	Thunder Storm
			type: 'STR',
			file: 'thunderstorm',
			wav: 'effect/magician_thunderstorm',
			attachedEntity: false
		}
	],

	31: [
		{
			//EF_FIREARROW	Weird bubbles launching from feet
			//type: 'FUNC',
			wav: 'effect/ef_firearrow1',
			attachedEntity: true
		}
	],

	32: [
		{
			//EF_NAPALMBEAT	Small clustered explosions
			//type: 'FUNC',
			//file: 'effect/Ao1ß1', // Uses up to Ao1ß8 , so eight files for an animated explosion
			wav: 'effect/ef_napalmbeat',
			attachedEntity: true
		}
	],

	33: [
		{
			//EF_RUWACH	//ruwach
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			attachedEntity: true,
			blendMode: 8,
			duration: 12200,
			duplicate: 8,
			timeBetweenDupli: 0,
			nbOfRotation: 8,
			posx: -2,
			rotateLate: 0.7,
			rotateLateDelta: -0.1,
			rotatePosX: 3,
			rotatePosY: 3,
			rotationClockwise: true,
			size: 0.4,
			sizeDelta: 0.15
		},
		{
			type: '3D',
			spriteName: 'particle2',
			alphaMax: 0.07,
			alphaMaxDelta: 0.07,
			attachedEntity: true,
			blendMode: 2,
			red: 0,
			green: 1,
			blue: 1,
			duration: 12200,
			duplicate: 10,
			timeBetweenDupli: 0,
			nbOfRotation: 8,
			posx: -2,
			posz: 2,
			rotateLate: 0.7,
			rotateLateDelta: -0.1,
			rotatePosX: 3,
			rotatePosY: 3,
			rotationClockwise: true,
			size: 80,
			sizeDelta: 30
		},
		{
			wav: 'effect/ef_ruwach'
		}
	],

	34: [
		{
			//EF_TELEPORTATION	Old Map Exit Animation (unused)
			type: 'CYLINDER',
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 8,
			bottomSize: 0.8,
			duration: 1000,
			fade: true,
			red: 1,
			green: 1,
			blue: 1,
			height: 35,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.7,
			wav: 'effect/ef_teleportation'
		}
	],

	35: [
		{
			//warp portal casting before unit appear	//EF_READYPORTAL	Old Warp Portal (unused)
			alphaMax: 0.6,
			animation: 0,
			attachedEntity: true,
			blue: 1,
			bottomSize: 0.6,
			duration: 25000,
			fade: true,
			green: 0.7,
			height: 15,
			red: 0.7,
			rotate: true,
			textureName: 'alpha_down',
			topSize: 0.6,
			type: 'CYLINDER',
			wav: 'effect/ef_readyportal'
		}
	],

	//36: [{}],	//EF_PORTAL	//warp portal unit

	37: [
		{
			//EF_INCAGILITY	AGI Up
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			delayLate: 500,
			duplicate: 7,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 0.75,
			attachedEntity: true,
			duration: 1000,
			delayOffset: 400,
			duplicate: 3,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			duplicate: 10,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/agi_up.bmp',
			poszEnd: 3,
			poszStart: 0.4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 100,
			sizeY: 45,
			type: '3D',
			wav: 'effect/ef_incagility',
			overlay: true,
			zIndex: 10
		}
	],

	38: [
		{
			//EF_DECAGILITY	AGI Down
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			duplicate: 20,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 1,
			poszStartRand: 1,
			poszStartRandMiddle: 6,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/slow.bmp',
			poszEnd: 0.4,
			poszStart: 2.8,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 100,
			sizeY: 45,
			type: '3D',
			wav: 'effect/ef_decagility',
			zIndex: 10
		}
	],

	39: [
		{
			//EF_AQUA	Aqua Benedicta
			type: 'SPR',
			file: '\xbc\xba\xbc\xf6\xb6\xdf\xb1\xe2',
			wav: 'effect/ef_aqua',
			head: true,
			attachedEntity: true
		}
	],

	40: [
		{
			//EF_SIGNUM	Signum Crucis
			type: 'STR',
			file: 'cross',
			wav: 'effect/ef_signum',
			attachedEntity: true
		}
	],

	41: [
		{
			//EF_ANGELUS	Angelus
			type: 'STR',
			file: 'angelus',
			wav: 'effect/ef_angelus',
			min: 'jong_mini',
			head: true,
			attachedEntity: true
		}
	],

	42: [
		{
			//EF_BLESSING	Blessing
			type: 'SPR',
			attachedEntity: true,
			delayFrame: 30,
			file: '\xc3\xe0\xba\xb9',
			frame: 0,
			repeat: true,
			duration: 1500,
			stackable: true,
			head: true,
			yOffset: -120
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			delayOffset: 300,
			duplicate: 6,
			fadeIn: true,
			fadeOut: true,
			posxRand: 1.2,
			posyRand: 1,
			poszEndRand: 0.5,
			poszEndRandMiddle: 1,
			poszStartRand: 2,
			poszStartRandMiddle: 5.5,
			size: 50,
			sparkling: true,
			sparkNumber: 2,
			spriteName: 'particle6',
			timeBetweenDupli: 0,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			delayOffset: 400,
			duplicate: 6,
			fadeIn: true,
			fadeOut: true,
			posxRand: 1.4,
			posyRand: 1.1,
			poszEndRand: 0.5,
			poszEndRandMiddle: 1,
			poszStartRand: 2,
			poszStartRandMiddle: 5.5,
			size: 50,
			spriteName: 'particle6',
			timeBetweenDupli: 0,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.3,
			attachedEntity: false,
			blendMode: 2,
			blue: 1,
			duration: 2500,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok2.tga',
			green: 0.75,
			posx: 0,
			posy: 0,
			posz: 0,
			red: 0.1,
			size: 140,
			type: '3D',
			wav: 'effect/ef_blessing',
			zIndex: 10
		}
	],

	43: [
		{
			//EF_INCAGIDEX	Dex + Agi Up
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/dex_agi_up.bmp',
			poszEnd: 3,
			poszStart: 0.4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 100,
			sizeY: 45,
			type: '3D',
			wav: 'effect/ef_incagidex',
			zIndex: 10
		}
	],

	44: [
		{
			//EF_SMOKE	Little fog Smoke (prontera chimneys)
			/*type: 'SPR',
			file: '\xb1\xbc\xb6\xd2\xbf\xac\xb1\xe2',
			attachedEntity: false*/

			type: '3D',
			spriteName: '\xb1\xbc\xb6\xd2\xbf\xac\xb1\xe2',

			duplicate: 10,
			timeBetweenDupli: 1000,

			duration: 10000,
			alphaMax: 0.8,
			fadeOut: true,
			fadeIn: false,

			poszStart: 0,
			poszEnd: 20,
			poszSmooth: false,

			posxStartRand: 0,
			posxEndRand: 3,
			posxSmooth: true,

			sizeStart: 70,
			sizeEnd: 300,
			sizeSmooth: true,

			angle: -90,
			toAngle: 0,
			rotate: true,
			rotateWithCamera: true,
			repeat: true,
			delayFrame: 100,

			param0_Func: function (e, v) {
				e.posxEnd = 0.5 + v * 0.01;
				e.size = e.size * (1 + Math.sin(v * 0.5) * 0.5);
			},

			param1_Func: function (e, v) {
				if (v < 5) {
					e.alpha = 1.0;
				}
				e.alpha = e.alpha * (0.9 + Math.random() * 0.1);
			}
		}
	],

	45: [
		{
			// This one is almost invisible, but there are some small white thingies flying around	//EF_FIREFLY	Faint Little Ball Things.
			type: 'FUNC',
			//file: 'sprite/AIANA®/particle1',
			attachedEntity: true
		}
	],

	47: [
		{
			//EF_TORCH	Torch
			type: '3D', // C++: PP_3DPARTICLE
			absoluteSpriteName: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/torch_01', // PT_USEORGARGB
			attachedEntity: true, // C++: m_pos = m_master->m_pos
			playSprite: true,
			duration: 600, // 6 frames 100delay each
			posz: 0.8, // C++: m_deltaPos2.y = -10
			posx: 0.1, // finetune
			size: 100, // C++: m_size (m_param[0]) 1.0f
			angle: 270,
			rotateToTarget: true,
			repeat: true
		}
	],

	49: [
		{
			//EF_FIREHIT	Firebolt/Wall Hits
			type: 'STR',
			file: 'firehit%d',
			wav: 'effect/ef_firehit',
			rand: [1, 3],
			attachedEntity: true
		}
	],

	50: [
		{
			//EF_FIRESPLASHHIT	Spinning Fire Thing
			type: '2D',
			overlay: true,
			duration: 500,
			file: 'effect/firering.tga',
			sizeStart: 10,
			sizeEnd: 300,
			angle: 0,
			toAngle: -360,
			rotate: true,
			fadeOut: true,
			posz: 1,
			attachedEntity: true
		}
	],

	51: [
		{
			// water hit	//EF_COLDHIT	Ice Elemental Hit
			wav: '_hit_fist%d',
			rand: [3, 4],
			attachedEntity: true
		} /*, {
			type: '2D',
			alphaMax: 1,
			angle: 0,
			angleDelta: 40,
			attachedEntity: false,
			duration: 140,
			duplicates: 9,
			fadeOut: true,
			file: 'effect/ice.tga',
			offsetxEnd: 3,
			offsetxStart: 0.5,
			size: 100,
			timeBetweenDupli: 0
		}*/
	],

	52: [
		{
			//EF_WINDHIT	Wind Elemental Hit
			type: 'STR',
			file: 'windhit%d',
			wav: '_hit_fist%d',
			rand: [1, 3],
			attachedEntity: true
		}
	],

	53: [
		{
			//EF_POISONHIT	Puff of Purpulish Smoke?
			type: 'SPR',
			file: 'poisonhit',
			wav: 'effect/ef_poisonattack',
			attachedEntity: false
		}
	],

	54: [
		{
			//EF_BEGINSPELL2	Cast Initiation Aura (Water Element)
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.5,
			height: 30,
			red: 0.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.5,
			height: 1,
			red: 0.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1.3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.6,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.5,
			height: 3,
			red: 0.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 4,
			type: 'CYLINDER',
			wav: 'effect/ef_beginspell'
		}
	],

	55: [
		{
			//EF_BEGINSPELL3	Cast Initiation Aura (Fire Element)
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.4,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.4,
			height: 30,
			red: 1,
			rotate: true,
			textureName: 'ring_red',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.7,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.4,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.4,
			height: 2,
			red: 1,
			rotate: true,
			textureName: 'ring_red',
			topSize: 1.3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.7,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.4,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.4,
			height: 3,
			red: 1,
			rotate: true,
			textureName: 'ring_red',
			topSize: 4,
			type: 'CYLINDER',
			wav: 'effect/ef_beginspell'
		}
	],

	56: [
		{
			//EF_BEGINSPELL4	Cast Initiation Aura (Wind Element)
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.6,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 1,
			height: 30,
			red: 0.6,
			rotate: true,
			textureName: 'ring_white',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.6,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.6,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 1,
			height: 2,
			red: 0.6,
			rotate: true,
			textureName: 'ring_white',
			topSize: 1.3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.6,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.6,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 1,
			height: 3,
			red: 0.6,
			rotate: true,
			textureName: 'ring_white',
			topSize: 4,
			type: 'CYLINDER',
			wav: 'effect/ef_beginspell'
		}
	],

	57: [
		{
			//EF_BEGINSPELL5	Cast Initiation Aura (Earth Element)
			alphaMax: 0.5,
			animation: 1,
			attachedEntity: true,
			blendMode: 0,
			blue: 1,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 1,
			height: 30,
			red: 1,
			rotate: true,
			textureName: 'ring_yellow',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 1,
			animation: 1,
			attachedEntity: true,
			blendMode: 0,
			blue: 1,
			bottomSize: 1.1,
			//duration: 900,
			fade: true,
			green: 1,
			height: 3,
			red: 1,
			rotate: true,
			textureName: 'ring_yellow',
			topSize: 1.2,
			type: 'CYLINDER'
		},
		{
			alphaMax: 1,
			animation: 2,
			attachedEntity: true,
			blendMode: 0,
			blue: 1,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 1,
			height: 3,
			red: 1,
			rotate: true,
			textureName: 'ring_yellow',
			topSize: 4,
			type: 'CYLINDER',
			wav: 'effect/ef_beginspell'
		}
	],

	58: [
		{
			//EF_BEGINSPELL6	Cast Initiation Aura (Holy Element)
			alphaMax: 0.8,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 1,
			height: 4,
			red: 1,
			rotate: true,
			textureName: 'ring_white',
			topSize: 5,
			type: 'CYLINDER',
			wav: 'effect/ef_beginspell'
		}
	],

	59: [
		{
			//EF_BEGINSPELL7	Cast Initiation Aura (Poison Element)
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.8,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.8,
			height: 30,
			red: 0.8,
			rotate: true,
			textureName: 'ring_purple',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.7,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.8,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.8,
			height: 3,
			red: 0.8,
			rotate: true,
			textureName: 'ring_purple',
			topSize: 1.3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.7,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.8,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 0.8,
			height: 3,
			red: 0.8,
			rotate: true,
			textureName: 'ring_purple',
			topSize: 4,
			type: 'CYLINDER',
			wav: 'effect/ef_beginspell'
		}
	],

	60: [
		{
			//EF_LOCKON	Cast target circle
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				this.add(new LockOnTarget(Params.Init.ownerEntity, Params.Inst.startTick, Params.Inst.endTick), Params);
			}
		}
	],

	61: [
		{
			//EF_WARPZONE	Old Warp Portal (NPC Warp, unused)
			alphaMax: 0.3,
			animation: 3,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 2,
			duration: 2800,
			fade: true,
			green: 0.5,
			height: 1.1,
			red: 0.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 3.3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.3,
			animation: 3,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1.9,
			duration: 2800,
			fade: true,
			green: 0.5,
			height: 1.1,
			red: 0.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 3.2,
			type: 'CYLINDER'
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			blue: 1,
			duration: 1000,
			duplicate: 3,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok1.tga',
			green: 1,
			posxStartRand: 3,
			posxStartRandMiddle: 0,
			posyStartRand: 3,
			posyStartRandMiddle: 0,
			poszEndRand: 2,
			poszEndRandMiddle: 2,
			poszStart: 0,
			red: 1,
			size: 100,
			sizeRand: 17,
			type: '3D',
			zIndex: 1
		}
	],

	62: [
		//EF_SIGHTRASHER	Sight Trasher
		// Direction 1: North
		{
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 0,
			posyEnd: -8,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 60,
			sizeEnd: 160,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 0,
			posyEnd: -8,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 20,
			sizeEnd: 260,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		// Direction 2: Northeast
		{
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 5.66,
			posyEnd: -5.66,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 60,
			sizeEnd: 160,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 5.66,
			posyEnd: -5.66,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 20,
			sizeEnd: 260,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		// Direction 3: East
		{
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 8,
			posyEnd: 0,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 60,
			sizeEnd: 160,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 8,
			posyEnd: 0,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 20,
			sizeEnd: 260,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		// Direction 4: Southeast
		{
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 5.66,
			posyEnd: 5.66,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 60,
			sizeEnd: 160,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 5.66,
			posyEnd: 5.66,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 20,
			sizeEnd: 260,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		// Direction 5: South
		{
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 0,
			posyEnd: 8,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 60,
			sizeEnd: 160,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: 0,
			posyEnd: 8,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 20,
			sizeEnd: 260,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		// Direction 6: Southwest
		{
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: -5.66,
			posyEnd: 5.66,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 60,
			sizeEnd: 160,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: -5.66,
			posyEnd: 5.66,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 20,
			sizeEnd: 260,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		// Direction 7: West
		{
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: -8,
			posyEnd: 0,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 60,
			sizeEnd: 160,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: -8,
			posyEnd: 0,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 20,
			sizeEnd: 260,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		// Direction 8: Northwest
		{
			type: '3D',
			shadowTexture: true,
			alphaMax: 0.5,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: -5.66,
			posyEnd: -5.66,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 60,
			sizeEnd: 160,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			type: '3D',
			spriteName: 'sight',
			alphaMax: 123 / 255,
			alphaMaxDelta: 3 / 255,
			duplicate: 4,
			timeBetweenDupli: 100,
			sizeDelta: -60,
			duration: 800,
			attachedEntity: false,
			posxStart: 0,
			posyStart: 0,
			posxEnd: -5.66,
			posyEnd: -5.66,
			poszStart: 2,
			poszEnd: 2,
			sizeStart: 20,
			sizeEnd: 260,
			playSprite: true,
			fadeIn: true,
			fadeOut: true,
			blendMode: 8
		},
		{
			wav: 'effect/wizard_sightrasher'
		}
	],

	64: [
		{
			//EF_ARROWSHOT	Something Like Puruple/Yellow Light Bullet
			type: 'STR',
			file: 'arrowshot',
			attachedEntity: true
		}
	],

	65: [
		{
			//EF_INVENOM	Something Like Absorb of Power
			type: 'STR',
			file: 'invenom',
			wav: 'effect/thief_invenom',
			attachedEntity: true
		}
	],

	66: [
		{
			//EF_CURE	Cure
			type: 'STR',
			file: 'cure',
			wav: 'effect/acolyte_cure',
			min: 'cure_min',
			attachedEntity: true
		}
	],

	67: [
		{
			//EF_PROVOKE	Provoke
			type: 'STR',
			file: 'provoke',
			wav: 'effect/swordman_provoke',
			attachedEntity: true
		}
	],

	68: [
		{
			//EF_MVP	MVP Banner
			type: 'STR',
			file: 'mvp',
			wav: 'effect/st_mvp',
			attachedEntity: true
		}
	],

	69: [
		{
			//EF_SKIDTRAP	Skid Trap
			type: 'STR',
			file: 'skidtrap',
			wav: 'effect/hunter_skidtrap', // or hallucinationwalk ?
			attachedEntity: false
		}
	],

	70: [
		{
			//EF_BRANDISHSPEAR	Brandish Spear
			attachedEntity: false,
			file: 'brandish',
			type: 'STR',
			wav: 'effect/knight_brandish_spear'
		}
	],

	74: [
		{
			//EF_ICEWALL	Ice Wall
			type: 'QuadHorn',
			textureFile: 'effect/ice.tga',
			attachedEntity: false,
			height: [2.8, 3.3],
			offsetX: [0.25, 0.75],
			offsetY: [0.25, 0.75],
			offsetZ: -0.1,
			bottomSize: [0.3, 0.5],
			blendMode: 8,
			rotateY: [1, 360],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 1,
			animationSpeed: 50
		},
		{
			type: 'QuadHorn',
			textureFile: 'effect/ice.tga',
			attachedEntity: false,
			height: [2.3, 2.8],
			offsetX: [0.25, 0.75],
			offsetY: [0.25, 0.75],
			offsetZ: -0.1,
			bottomSize: [0.3, 0.5],
			blendMode: 8,
			rotateY: [1, 360],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 1,
			animationSpeed: 50
		},
		{
			type: 'QuadHorn',
			textureFile: 'effect/ice.tga',
			attachedEntity: false,
			height: [2.5, 2.9],
			offsetX: [0.25, 0.75],
			offsetY: [0.25, 0.75],
			offsetZ: -0.1,
			bottomSize: [0.3, 0.5],
			blendMode: 8,
			rotateY: [1, 360],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 1,
			animationSpeed: 50
		},
		{
			wav: 'effect/wizard_icewall'
		}
	],

	75: [
		{
			//EF_GLORIA	Gloria
			type: 'STR',
			file: 'gloria',
			wav: 'effect/priest_gloria',
			min: 'gloria_min',
			attachedEntity: true
		}
	],

	76: [
		{
			//EF_MAGNIFICAT	Magnificat
			type: 'STR',
			file: 'magnificat',
			wav: 'effect/priest_magnificat',
			min: 'magnificat_min',
			attachedEntity: true
		}
	],

	77: [
		{
			//EF_RESURRECTION	Resurrection
			type: 'STR',
			file: 'resurrection',
			wav: 'effect/priest_resurrection',
			min: 'resurrection_min',
			attachedEntity: true
		}
	],

	78: [
		{
			//EF_RECOVERY	Status Recovery
			type: 'STR',
			file: 'recovery',
			wav: 'effect/priest_recovery',
			attachedEntity: true
		}
	],

	79: [
		{
			//EF_EARTHSPIKE	Earth Spike
			//Main Spike
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 5000,
			height: [0.95, 1.5],
			offsetX: [0.4, 0.6],
			offsetY: [0.4, 0.6],
			offsetZ: -0.1,
			bottomSize: [0.5, 0.6],
			blendMode: 1,
			rotateY: [1, 360],
			rotateZ: [-8.0, 8.0],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 3,
			animationSpeed: 120,
			animationOut: true
		},
		{
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 5000,
			height: [0.2, 0.4],
			offsetX: [0.0, 0.0],
			offsetY: [0.5, 0.5],
			offsetZ: -0.1,
			bottomSize: [0.1, 0.2],
			blendMode: 1,
			rotateY: [1, 360],
			rotateZ: [-15.0, 15.0],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 2,
			animationSpeed: 100,
			animationOut: true
		},
		{
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 5000,
			height: [0.2, 0.4],
			offsetX: [0.0, 0.5],
			offsetY: [0.5, 1.0],
			offsetZ: -0.1,
			bottomSize: [0.1, 0.2],
			blendMode: 1,
			rotateY: [1, 360],
			rotateZ: [-15.0, 15.0],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 2,
			animationSpeed: 100,
			animationOut: true
		},
		{
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 5000,
			height: [0.2, 0.4],
			offsetX: [1.0, 1.2],
			offsetY: [0.5, 0.8],
			offsetZ: -0.1,
			bottomSize: [0.1, 0.2],
			blendMode: 1,
			rotateY: [1, 360],
			rotateZ: [-15.0, 15.0],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 2,
			animationSpeed: 100,
			animationOut: true
		},
		{
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 5000,
			height: [0.2, 0.4],
			offsetX: [0.5, 0.7],
			offsetY: [0.0, -0.2],
			offsetZ: -0.1,
			bottomSize: [0.1, 0.2],
			blendMode: 1,
			rotateY: [1, 360],
			rotateZ: [-15.0, 15.0],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 2,
			animationSpeed: 100,
			animationOut: true
		},
		{
			wav: 'effect/wizard_earthspike',
			attachedEntity: false
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				const duration = 200;
				Camera.setQuake(start, duration);
			}
		}
	],

	80: [
		{
			//spear boomerang hit on target	//EF_SPEARBMR	Spear Boomerang
			//type: 'FUNC',
			wav: 'effect/ef_fireball',
			attachedEntity: true
		}
	],

	81: [
		{
			// default skill sound?	//EF_PIERCE	Skill hit
			wav: 'effect/ef_bash',
			attachedEntity: true
		}
	],

	82: [
		{
			//turn undead caster	//EF_TURNUNDEAD	Turn Undead
			//type: 'FUNC',
			wav: 'effect/ef_bash',
			attachedEntity: true
		}
	],

	83: [
		{
			//EF_SANCTUARY	Sanctuary
			type: 'STR',
			file: 'sanctuary',
			wav: 'effect/priest_sanctuary',
			attachedEntity: true
		}
	],

	84: [
		{
			//EF_IMPOSITIO	Impositio Manus
			type: 'STR',
			file: 'impositio',
			wav: 'effect/priest_impositio',
			attachedEntity: true
		}
	],

	85: [
		{
			//EF_LEXAETERNA	Lex Aeterna
			type: 'STR',
			file: 'lexaeterna',
			wav: 'effect/priest_lexaeterna',
			min: 'lexaeterna_min',
			attachedEntity: true
		}
	],

	86: [
		{
			//EF_ASPERSIO	Aspersio
			type: 'STR',
			file: 'aspersio',
			wav: 'effect/priest_aspersio',
			attachedEntity: true
		}
	],

	87: [
		{
			//EF_LEXDIVINA	Lex Divina
			type: 'STR',
			file: 'lexdivina',
			wav: 'effect/priest_lexdivina',
			attachedEntity: true
		}
	],

	88: [
		{
			//EF_SUFFRAGIUM	Suffragium
			type: 'STR',
			file: 'suffragium',
			wav: 'effect/priest_suffragium',
			min: 'suffragium_min',
			attachedEntity: true
		}
	],

	89: [
		{
			//EF_STORMGUST	Storm Gust
			type: 'STR',
			file: 'stormgust',
			wav: 'effect/wizard_stormgust',
			min: 'storm_min',
			attachedEntity: true
		}
	],

	90: [
		{
			//EF_LORD	Lord of Vermilion
			type: 'STR',
			file: 'lord',
			wav: 'effect/wizard_fire_ivy',
			attachedEntity: true
		}
	],

	91: [
		{
			//EF_BENEDICTIO	B. S. Sacramenti
			type: 'STR',
			file: 'benedictio',
			wav: 'effect/priest_benedictio',
			attachedEntity: true
		}
	],

	92: [
		{
			//EF_METEORSTORM	Meteor Storm
			type: 'STR',
			file: 'meteor%d',
			wav: 'effect/wizard_meteor',
			rand: [1, 4],
			attachedEntity: true
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 600;
				Camera.setQuake(start);
			}
		}
	],

	93: [
		{
			//EF_YUFITEL	Jupitel Thunder (Ball)
			wav: 'effect/hunter_shockwavetrap',
			attachedEntity: false
		},
		{
			type: '3D',
			file: 'effect/thunder_center.bmp',
			duration: 200, // Aprox. 20 frames on C++
			size: 35, // widthSize 3.5f
			toSrc: true, // PT3DTEX_CHKTARGETPOS
			blendMode: 2, // RF_EMISSIVE (Aprox. Additive)
			overlay: true,
			alphaMax: 0.66 // 170/255 on C++
		},
		{
			type: '3D',
			fileList: [
				'effect/thunder_ball_a.bmp',
				'effect/thunder_ball_b.bmp',
				'effect/thunder_ball_c.bmp',
				'effect/thunder_ball_d.bmp',
				'effect/thunder_ball_e.bmp',
				'effect/thunder_ball_f.bmp'
			],
			frameDelay: 10, // m_animSpeed = 1 on C++
			duration: 200, // Aprox. 20 frames on C++
			size: 45, // widthSize 4.5f
			toSrc: true, // PT3DTEX_CHKTARGETPOS
			overlay: true,
			blendMode: 2 // RF_EMISSIVE (Aprox. Additive)
		}
	],

	94: [
		{
			//EF_YUFITELHIT	Jupitel Thunder (Hit)
			type: '3D',
			file: 'effect/thunder_pang.bmp',
			duration: 100, // Aprox. 10 frames on C++
			sizeStart: 0,
			sizeEnd: 25, // widthSpeed 2.5f (Growing up)
			blendMode: 2, // RF_EMISSIVE (Aprox. Additive)
			rotateToTarget: true,
			fadeOut: true,
			overlay: true,
			attachedEntity: true
		},
		{
			type: '3D',
			fileList: [
				'effect/thunder_plazma_blast_a.bmp',
				'effect/thunder_plazma_blast_b.bmp',
				'effect/thunder_ball_d.bmp',
				'effect/thunder_ball_e.bmp',
				'effect/thunder_ball_f.bmp'
			],
			frameDelay: 10,
			duration: 300, // duration 300 on C++ is equal to 3000ms on ROB but it's the total duration of all hits, ROB parse each
			size: 75, // widthSize 7.5f * 10
			blendMode: 2, // RF_EMISSIVE (Aprox. Additive)
			overlay: true,
			attachedEntity: true
		}
	],

	95: [
		{
			//EF_QUAGMIRE	Quagmire
			type: 'STR',
			file: 'quagmire',
			attachedEntity: false
		},
		{
			wav: 'effect/wizard_quagmire'
		}
	],

	96: [
		{
			//firepillar caster	//EF_FIREPILLAR	Fire Pillar
			type: 'STR',
			file: 'firepillar',
			wav: 'effect/wizard_fire_pillar_a',
			attachedEntity: false
		}
	],

	97: [
		{
			//firepillar target hit	//EF_FIREPILLARBOMB	Fire Pillar/Land Mine hit
			type: 'STR',
			file: 'firepillarbomb',
			wav: 'effect/wizard_fire_pillar_b',
			attachedEntity: false
		}
	],

	98: [
		{
			//EF_HASTEUP	Adrenaline Rush
			//type: 'FUNC',
			// This one is pretty messy... it somehow consists of two sprites, one is attached to the Entity, one isnt. additionally it consists of two sounds
			// For the sake of simplicity, I propose just using one sprite and one sound - the _a sound is just some 'intro' while _b is a real effect
			// black_adrenalinerush.wav what is this for?
			wav: 'effect/black_adrenalinerush_b', // The original client plays _a first and then continues with b
			attachedEntity: true
		}
	],

	'98_beforecast': [
		{
			wav: 'effect/black_adrenalinerush_a',
			attachedEntity: true
		}
	],

	99: [
		{
			//EF_FLASHER	Flasher Trap
			//type: 'FUNC',
			// Again two sprites... one attached one not. But here the 'main' sprite is ment to stay a little longer
			wav: 'effect/hunter_flasher',
			attachedEntity: false
		}
	],

	100: [
		{
			//EF_REMOVETRAP	Yellow ball fountain
			//type: 'FUNC',
			wav: 'effect/hunter_removetrap',
			attachedEntity: false
		}
	],

	101: [
		{
			//EF_REPAIRWEAPON	Weapon Repair
			type: 'STR',
			file: 'repairweapon',
			attachedEntity: true
		},
		{
			wav: 'effect/black_weapon_repair_a',
			delayWav: 480
		},
		{
			wav: 'effect/black_weapon_repair_a',
			delayWav: 1320
		}
	],

	102: [
		{
			//EF_CRASHEARTH	Hammerfall
			type: 'STR',
			file: 'crashearth',
			wav: 'effect/black_hammerfall',
			attachedEntity: false
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 350;
				Camera.setQuake(start);
			}
		}
	],

	103: [
		{
			//EF_PERFECTION	Weapon Perfection
			type: 'STR',
			file: 'weaponperfection',
			wav: 'effect/black_weapon_perfection',
			min: 'weaponperfection_min',
			attachedEntity: true
		}
	],

	104: [
		{
			//EF_MAXPOWER	Maximize Power
			type: 'STR',
			file: 'maximizepower',
			min: 'maximize_min',
			attachedEntity: true
		}
	],

	105: [
		{
			//EF_BLASTMINE	empty
			wav: 'effect/hun_anklesnare'
		}
	],

	106: [
		{
			//EF_BLASTMINEBOMB	Blast Mine Trap
			type: 'STR',
			file: 'blastmine',
			wav: 'effect/hunter_blastmine', //hun_blastmine
			attachedEntity: false
		}
	],

	107: [
		{
			//EF_CLAYMORE	Claymore Trap
			type: 'STR',
			file: 'claymore',
			wav: 'effect/hunter_claymoretrap',
			attachedEntity: false
		}
	],

	108: [
		{
			//EF_FREEZING	Freezing Trap
			type: 'STR',
			file: 'freezing',
			wav: 'effect/hunter_freezingtrap',
			attachedEntity: false
		}
	],

	109: [
		{
			//EF_BUBBLE	Bailaban Blue bubble Map Effect
			type: 'STR',
			file: 'bubble%d',
			rand: [1, 4],
			attachedEntity: false
		}
	],

	110: [
		{
			//EF_GASPUSH	Trap Used by Giearth
			type: 'STR',
			file: 'gaspush',
			//wav:  'se_gas_pushhh', // the anthell map already has the sounds placed, no need to add it to the effect as well
			attachedEntity: false
		}
	],

	111: [
		{
			//EF_SPRINGTRAP	Spring Trap
			type: 'STR',
			file: 'spring',
			wav: 'effect/hunter_springtrap',
			attachedEntity: false
		}
	],

	112: [
		{
			//kyrie caster	//EF_KYRIE	Kyrie Eleison
			type: 'STR',
			file: 'kyrie',
			wav: 'effect/priest_kyrie_eleison_a', //on target its priest_kyrie_eleison_b
			min: 'kyrie_min',
			attachedEntity: true
		}
	],

	113: [
		{
			//EF_MAGNUS	Magnus Exorcismus
			type: 'STR',
			file: 'magnus',
			wav: 'effect/priest_magnus',
			attachedEntity: false
		}
	],

	115: [
		{
			//blitzbeat on target hit	//EF_BLITZBEAT	Blitz Beat
			wav: 'effect/hunter_blitzbeat',
			attachedEntity: true
		}
	],

	116: [
		{
			//EF_WATERBALL	   Fling Watersphere
			type: '3D',
			duration: 500,
			fileList: ['effect/water_out_a.bmp', 'effect/water_out_b.bmp', 'effect/water_out_c.bmp'],
			fadeOut: true,
			posxRand: 1.5,
			poszRand: 1.5,
			posyStart: 0,
			posyEnd: 3,
			posySmooth: true,
			size: 30.5,
			rotateWithCamera: true,
			blendMode: 2,
			attachedEntity: true,
			wav: 'effect/wizard_waterball_chulung'
		}
	],

	117: [
		{
			//EF_WATERBALL2	waterball moving to target
			type: '3D',
			duration: 500,
			duplicate: 20,
			timeBetweenDupli: 50,
			absoluteSpriteName: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/waterball',
			playSprite: true,
			fromSrc: true,
			rotateToTarget: true,
			fadeOut: true,
			size: 50.0,
			zOffsetStart: 5,
			zOffsetEnd: 0,
			arc: 7.5,
			retreat: 5.0,
			wav: 'effect/wizard_waterball_chulung'
		}
	],

	119: [
		{
			//EF_DETECTING	Detect
			wav: 'effect/hunter_detecting',
			attachedEntity: true
		}
	],

	120: [
		{
			//EF_CLOAKING	   Cloaking
			wav: 'effect/assasin_cloaking'
		}
	],

	121: [
		{
			//EF_SONICBLOW	// sonic blow caste
			type: '3D',
			blendMode: 2,
			duration: 400,
			alphaMax: 1.0,
			fadeOut: true,
			sizeStart: 100,
			sizeEnd: 300,
			file: 'effect/ring2.bmp', // Original: 'effect/alpha_center.tga',
			wav: 'effect/ef_stonecurse',
			zIndex: 10,
			attachedEntity: true
		}
	],

	122: [
		{
			//EF_SONICBLOWHIT	   Multi hit effect
			//wav:  'effect/assasin_sonicblow' // old

			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				const delay = Params.Inst.startTick - Renderer.tick;

				entity.animations.add(function (tick) {
					if (tick >= delay) {
						entity.direction = (entity.direction + 2) % 8; // Spinning around
						return true;
					}
				});
			}
		}
	],

	123: [
		{
			//grimtooth caster	//EF_GRIMTOOTH	Grimtooth Cast
			wav: 'effect/ef_frostdiver',
			attachedEntity: true
		}
	],

	124: [
		{
			//EF_VENOMDUST	Venom Dust
			type: 'STR',
			file: 'venomdust',
			wav: 'effect/assasin_poisonreact', //effect/assasin_venomdust'
			attachedEntity: false
		}
	],

	126: [
		{
			//EF_POISONREACT	Poison React
			type: 'STR',
			file: 'poisonreact_1st',
			wav: 'effect/assasin_poisonreact',
			attachedEntity: true
		}
	],

	127: [
		{
			//EF_POISONREACT2	Small Posion React
			type: 'STR',
			file: 'poisonreact',
			wav: 'effect/assasin_poisonreact',
			attachedEntity: true
		}
	],

	128: [
		{
			//EF_OVERTHRUST	Over Thrust
			wav: 'effect/black_overthrust',
			attachedEntity: true
		}
	],

	129: [
		{
			//EF_SPLASHER	Venom Splasher Explosion
			type: 'STR',
			file: 'venomsplasher',
			wav: 'effect/assasin_venomsplasher',
			attachedEntity: true
		}
	],

	130: [
		{
			//EF_TWOHANDQUICKEN	Two-Hand Quicken
			type: 'STR',
			file: 'twohand',
			wav: 'effect/knight_twohandquicken',
			head: true,
			attachedEntity: true
		}
	],

	131: [
		{
			//autocounter activate hit	//EF_AUTOCOUNTER	Auto-Counter Hit
			type: 'STR',
			file: 'autocounter',
			wav: 'effect/knight_autocounter',
			attachedEntity: true
		}
	],

	132: [
		//EF_GRIMTOOTHATK	   Grimtooth Hit
		{
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 15000,
			height: 2.5,
			offsetX: 0,
			offsetY: 0.4,
			offsetZ: -0.2,
			bottomSize: 0.15,
			blendMode: 1,
			rotateX: -15,
			rotateY: 0,
			rotateZ: 0,
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 3,
			animationSpeed: 120,
			animationOut: true
		},
		{
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 15000,
			height: 2.5,
			offsetY: 0.5,
			bottomSize: 0.15,
			blendMode: 1,
			rotateX: 5,
			rotateY: 45,
			rotateZ: -15,
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 3,
			animationSpeed: 120,
			animationOut: true
		},
		{
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 15000,
			height: 2.5,
			offsetX: 0,
			offsetY: 0.5,
			offsetZ: 0,
			bottomSize: 0.15,
			blendMode: 1,
			rotateX: 5,
			rotateY: 45,
			rotateZ: 15,
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 3,
			animationSpeed: 120,
			animationOut: true
		}
	],

	133: [
		{
			//EF_FREEZE	Ice Effect (Used by NPCs)
			type: 'STR',
			file: 'freeze',
			attachedEntity: true
		}
	],

	134: [
		{
			//EF_FREEZED	Ice Effect (Used by NPCs)
			type: 'STR',
			file: 'freezed',
			attachedEntity: true
		}
	],

	135: [
		{
			//EF_ICECRASH	Ice Effect (Used by NPCs)
			type: 'STR',
			file: 'icecrash',
			attachedEntity: true
		}
	],

	136: [
		{
			//EF_SLOWPOISON	Slow Poison
			type: 'STR',
			file: 'slowp',
			wav: 'effect/priest_slowpoison',
			attachedEntity: false
		}
	],

	138: [
		{
			//EF_FIREPILLARON	Fire pillar
			type: 'CYLINDER',
			attachedEntity: false,
			topSize: 2.0,
			bottomSize: 1.0,
			textureName: 'magic_red',
			height: 3,
			duration: 5000,
			delayOffset: 1000,
			rotate: true
		},
		{
			type: 'CYLINDER',
			attachedEntity: false,
			topSize: 1.5,
			bottomSize: 0.7,
			textureName: 'magic_red',
			height: 5,
			duration: 5000,
			delayOffset: 1000,
			rotate: true
		},
		{
			type: 'CYLINDER',
			attachedEntity: false,
			topSize: 1.0,
			bottomSize: 0.5,
			textureName: 'magic_red',
			height: 7,
			duration: 5000,
			delayOffset: 1000,
			rotate: true
		}
	],

	139: [
		{
			//EF_SANDMAN	Sandman Trap
			type: 'STR',
			file: 'sandman',
			wav: 'effect/hunter_sandman',
			attachedEntity: false
		}
	],

	140: [
		{
			//EF_REVIVE	Ressurection Aura
			wav: 'effect/priest_resurrection',
			attachedEntity: true
		}
	],

	141: [
		{
			//EF_PNEUMA	Pneuma
			type: 'STR',
			file: 'pneuma%d',
			rand: [1, 3],
			attachedEntity: false
		}
	],

	142: [
		{
			//EF_HEAVENDRIVE
			type: 'FUNC',
			func: function (Params) {
				for (let i = -2; i <= 2; i++) {
					for (let j = -2; j <= 2; j++) {
						const newParams = {
							effect: {
								type: 'QuadHorn',
								textureFile: 'effect/stone.bmp',
								attachedEntity: false,
								duration: 1000,
								height: [0.75, 1.2],
								offsetX: [0.2, 0.2],
								offsetY: [0.2, 0.2],
								offsetZ: -0.1,
								bottomSize: [0.4, 0.7],
								blendMode: 1,
								rotateY: [1, 360],
								rotateZ: [-8.0, 8.0],
								color: [1.0, 1.0, 1.0, 1.0],
								animation: 3,
								animationSpeed: 250,
								animationOut: true
							},
							Init: {},
							Inst: {}
						};
						const newPos = [Params.Init.position[0] + i, Params.Init.position[1] + j];
						Object.assign(newParams.Init, Params.Init);
						newParams.Init.position = [newPos[0], newPos[1], Altitude.getCellHeight(newPos[0], newPos[1])];
						newParams.Init.duration = newParams.effect.duration;
						Object.assign(newParams.Inst, Params.Inst);
						newParams.Inst.position = [newPos[0], newPos[1], Altitude.getCellHeight(newPos[0], newPos[1])];
						newParams.Inst.duration = newParams.effect.duration;
						newParams.Inst.endTick = newParams.Inst.startTick + newParams.effect.duration;
						this.add(new QuadHorn(newParams.effect, newParams.Inst, newParams.Init), newParams);
					}
				}
			}
		},
		{
			wav: 'effect/wizard_earthspike',
			attachedEntity: false
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				const duration = 200;
				Camera.setQuake(start, duration);
			}
		}
	],

	143: [
		{
			//EF_SONICBLOW2	Sonic Blow (Part 2/2) - sonicblow at target
			type: 'STR',
			file: 'sonicblow',
			attachedEntity: true
		}
	],

	144: [
		{
			//EF_BRANDISH2	Brandish Spear Pre-Hit Effect
			type: 'STR',
			file: 'brandish2',
			wav: 'effect/knight_brandish_spear',
			attachedEntity: true
		}
	],

	/*145: [{	//EF_SHOCKWAVE	Shockwave Trap
			type: 'STR',
			file: 'shockwave',
			wav:  'effect/hunter_shockwavetrap',
			attachedEntity: true
		}],*/

	145: [
		{
			type: 'SPR',
			file: 'shockwave',
			wav: 'effect/hunter_shockwavetrap',
			attachedEntity: true
		}
	],

	146: [
		{
			//EF_SHOCKWAVEHIT	Shockwave Trap Hit
			type: 'STR',
			file: 'shockwavehit',
			attachedEntity: true
		}
	],

	147: [
		{
			//EF_EARTHHIT	Pierce Hit
			type: 'STR',
			file: 'earthhit',
			attachedEntity: true
		}
	],

	148: [
		{
			//EF_PIERCESELF	Pierce Cast Animation
			type: 'STR',
			file: 'pierce',
			attachedEntity: true
			// attach animation at middle of body
		}
	],

	149: [
		{
			//EF_BOWLINGSELF	Bowling Bash
			type: 'STR',
			file: 'bowling',
			wav: '_enemy_hit_normal1', //'effect/knight_bowling_bash', fake
			head: true,
			attachedEntity: true
		}
	],

	150: [
		{
			//EF_SPEARSTABSELF	Pierce Cast Animation
			type: 'STR',
			file: 'spearstab',
			wav: '_enemy_hit_normal1',
			attachedEntity: true
			// attach animation at middle of body
		}
	],

	151: [
		{
			//spear boomerang caster	//EF_SPEARBMRSELF	Spear Boomerang Cast
			type: 'STR',
			file: 'spearboomerang',
			wav: 'effect/knight_spear_boomerang',
			head: true,
			attachedEntity: true
		}
	],

	152: [
		{
			//turn undead hit on targer	//EF_HOLYHIT	Turn Undead
			type: 'STR',
			file: 'holyhit',
			attachedEntity: true
		}
	],

	153: [
		{
			//EF_CONCENTRATION	Increase Concentration
			type: 'STR',
			file: 'concentration',
			wav: 'effect/ac_concentration',
			attachedEntity: true
		}
	],

	154: [
		{
			//EF_REFINEOK	Refine Success
			type: 'STR',
			file: 'bs_refinesuccess',
			wav: 'effect/bs_refinesuccess',
			attachedEntity: true
		}
	],

	155: [
		{
			//EF_REFINEFAIL	Refine Fail
			type: 'STR',
			file: 'bs_refinefailed',
			wav: 'effect/bs_refinefailed',
			attachedEntity: true
		}
	],

	//156: [{}],	//EF_JOBCHANGE	   jobchange.str not found error
	//157: [{}],	//EF_LVUP	   levelup.str not found error

	158: [
		{
			//EF_JOBLVUP	Job Level Up
			type: 'STR',
			file: 'joblvup',
			attachedEntity: true
		}
	],

	//159: [{}],	//EF_TOPRANK	   PvP circle
	//160: [{}],	//EF_PARTY	   PvP Party Circle
	161: [
		{
			//EF_RAIN	Rain weather
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				RainWeatherEffect.startOrRestart(Params);
			}
		}
	],
	162: [
		{
			//EF_SNOW	Snow weather
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				SnowWeatherEffect.startOrRestart(Params);
			}
		}
	],

	163: [
		{
			//EF_SAKURA	   White Sakura Leaves
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				SakuraWeatherEffect.startOrRestart(Params);
			}
		}
	],

	//164: [{}],	//EF_STATUS_STATE	   (Nothing)

	165: [
		{
			//Comodo Fireworks Ball	//EF_BANJJAKII	Comodo Fireworks Ball
			type: 'SPR',
			file: '\xc5\xa9\xb8\xae\xbd\xba\xb8\xb6\xbd\xba',
			duration: 1000,
			attachedEntity: true
		}
	],

	166: [
		{
			//EF_MAKEBLUR Energy Coat (Visual Effect)
			type: 'FUNC',
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				if (!entity) {
					return;
				}

				entity._enableTrail = true;
				entity._blurType = 1;

				setTimeout(() => {
					entity._enableTrail = false;
				}, Params.Init.duration || 2000);
			}
		}
	],
	//167: [{}],	//EF_TAMINGSUCCESS	   (Nothing)
	//168: [{}],	//EF_TAMINGFAILED	   (Nothing)

	169: [
		{
			//EF_ENERGYCOAT	Energy Coat Animation
			type: 'STR',
			file: 'energycoat',
			attachedEntity: true
		}
	],

	170: [
		{
			//EF_CARTREVOLUTION	Cart Revolution
			type: 'STR',
			file: 'cartrevolution',
			wav: 'effect/ef_magnumbreak',
			attachedEntity: true
		}
	],

	171: [
		{
			//EF_VENOMDUST2	   Venom Dust Map Unit
			type: '3D',
			spriteName: 'particle3',
			alphaMax: 1,
			attachedEntity: true,
			duration: 100,
			size: 80,
			poszStart: 0,
			poszEnd: 0.5,
			repeat: true
		}
	],

	//172: [{}],	//EF_CHANGEDARK	   Change Element (Dark)
	//173: [{}],	//EF_CHANGEFIRE	   Change Element (Fire)
	//174: [{}],	//EF_CHANGECOLD	   Change Element (Water)
	//175: [{}],	//EF_CHANGEWIND	   Change Element (Wind)
	//176: [{}],	//EF_CHANGEFLAME	   Change Element (Fire)
	//177: [{}],	//EF_CHANGEEARTH	   Change Element (Earth)
	//178: [{}],	//EF_CHAINGEHOLY	   Change Element (Holy)
	//179: [{}],	//EF_CHANGEPOISON	   Change Element (Poison)

	//180: [{}],	//EF_HITDARK	   Dark Elemental Hit

	181: [
		{
			//EF_MENTALBREAK	Mental Breaker
			type: 'STR',
			file: 'mentalbreak',
			attachedEntity: true
		}
	],

	182: [
		{
			//EF_MAGICALATTHIT	Magical Hit
			type: 'STR',
			file: 'magical',
			attachedEntity: true
		}
	],

	183: [
		{
			//EF_SUI_EXPLOSION	Self Destruction
			type: 'STR',
			file: 'sui_explosion',
			attachedEntity: true,
			wav: 'effect/ef_hit2'
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				const duration = 200;
				Camera.setQuake(start, duration);
			}
		}
	],

	//184: [{}],	//EF_DARKATTACK	   Dark Elemental Attack

	185: [
		{
			//EF_SUICIDE	(Nothing)
			type: 'STR',
			file: 'suicide',
			attachedEntity: true
		}
	],

	186: [
		{
			//EF_COMBOATTACK1	Combo Attack 1
			type: 'STR',
			file: 'yunta_1',
			attachedEntity: true
		}
	],

	187: [
		{
			//EF_COMBOATTACK2	Combo Attack 2
			type: 'STR',
			file: 'yunta_2',
			attachedEntity: true
		}
	],

	188: [
		{
			//EF_COMBOATTACK3	Combo Attack 3
			type: 'STR',
			file: 'yunta_3',
			attachedEntity: true
		}
	],

	189: [
		{
			//EF_COMBOATTACK4	Combo Attack 4
			type: 'STR',
			file: 'yunta_4',
			attachedEntity: true
		}
	],

	190: [
		{
			//EF_COMBOATTACK5	Combo Attack 5
			type: 'STR',
			file: 'yunta_5',
			attachedEntity: true
		}
	],

	191: [
		{
			//EF_GUIDEDATTACK	Guided Attack
			type: 'STR',
			file: 'homing',
			attachedEntity: true
		}
	],

	192: [
		{
			//EF_POISONATTACK	Poison Attack
			type: 'STR',
			file: 'poison',
			attachedEntity: true
		}
	],

	193: [
		{
			//EF_SILENCEATTACK	Silence Attack
			type: 'STR',
			file: 'silence',
			attachedEntity: true
		}
	],

	194: [
		{
			//EF_STUNATTACK	Stun Attack
			type: 'STR',
			file: 'stun',
			attachedEntity: true
		}
	],

	195: [
		{
			//EF_PETRIFYATTACK	Petrify Attack
			type: 'STR',
			file: 'stonecurse',
			attachedEntity: true
		}
	],

	//196: [{}],	//EF_CURSEATTACK	   Curse Attack

	197: [
		{
			//EF_SLEEPATTACK	Sleep Attack
			type: 'STR',
			file: 'sleep',
			attachedEntity: true
		}
	],

	//198: [{}],	//EF_TELEKHIT	   (Nothing)

	199: [
		{
			//EF_PONG	Small Popping Bubble Map Effect
			type: 'STR',
			file: 'pong%d',
			rand: [1, 3],
			attachedEntity: false
		}
	],

	200: [
		// EF_LEVEL99 - Normal level 99 Aura (Rising Cone / Swirling Band)
		// Math model from original game:
		// - Ring radius ≈ 3.9, tilt 55°, vertical amplitude 15, 315° sweep
		// - height_k(t) = H·sin(90+9(k−10))·sin(ωt), ω=1°/frame
		// - Spin adds (ec+3)°/frame
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				this.add(
					new SwirlingAura(Params.Init.ownerEntity.position, 'ring_blue.tga', Params.Inst.startTick),
					Params
				);
			}
		}
	],

	201: [
		// EF_LEVEL99_2 - Normal level 99 Aura (Bottom / Ground Ring)
		// Uses GroundAura which renders flat quads on the ground plane
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				this.add(
					new GroundAura(
						Params.Init.ownerEntity.position,
						100, // size (same as original Aura.js)
						15.0, // distance (same as original Aura.js)
						'pikapika2.bmp',
						Params.Inst.startTick
					),
					Params
				);
			}
		}
	],

	202: [
		// EF_LEVEL99_3 - Level 99 aura bubble sparkle (camera-facing diamonds)
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				// Default to blue variant (flag1 = 1) unless overridden
				let flag1 = 1;

				// Allow effect table or caller to override tint variant
				if (typeof Params.effect.flag1 !== 'undefined') {
					flag1 = Params.effect.flag1;
				} else if (Params.Inst && typeof Params.Inst.flag1 !== 'undefined') {
					flag1 = Params.Inst.flag1;
				}

				this.add(
					new Level99Bubble(Params.Init.ownerEntity.position, 'whitelight.tga', Params.Inst.startTick, flag1),
					Params
				);
			}
		}
	],

	203: [
		{
			//EF_GUMGANG	   Fury (Visual Effect)
			//super1..5.bmp
			type: '3D',
			fadeOut: true,
			duration: 2000,
			delayLate: 0,
			file: 'effect/super1.bmp',
			attachedEntity: true
		},
		{
			type: '3D',
			fadeOut: true,
			duration: 2000,
			delayLate: 400,
			file: 'effect/super2.bmp',
			attachedEntity: true
		},
		{
			type: '3D',
			fadeOut: true,
			duration: 2000,
			delayLate: 800,
			file: 'effect/super3.bmp',
			attachedEntity: true
		},
		{
			type: '3D',
			fadeOut: true,
			duration: 2000,
			delayLate: 1200,
			file: 'effect/super4.bmp',
			attachedEntity: true
		},
		{
			type: '3D',
			fadeOut: true,
			duration: 2000,
			delayLate: 1600,
			file: 'effect/super5.bmp',
			attachedEntity: true
		}
	],

	204: [
		{
			//EF_POTION1	Red Herb/Potion
			type: 'STR',
			file: '\xbb\xa1\xb0\xa3\xc6\xf7\xbc\xc7',
			attachedEntity: true
		}
	],

	205: [
		{
			//EF_POTION2	Orange Potion
			type: 'STR',
			file: '\xc1\xd6\xc8\xab\xc6\xf7\xbc\xc7',
			attachedEntity: true
		}
	],

	206: [
		{
			//EF_POTION3	Yellow Herb/Potion
			type: 'STR',
			file: '\xb3\xeb\xb6\xf5\xc6\xf7\xbc\xc7',
			attachedEntity: true
		}
	],

	207: [
		{
			//EF_POTION4	White Herb/Potion
			type: 'STR',
			file: '\xc7\xcf\xbe\xe1\xc6\xf7\xbc\xc7',
			attachedEntity: true
		}
	],

	208: [
		{
			//EF_POTION5	Blue Herb/Potion
			type: 'STR',
			file: '\xc6\xc4\xb6\xf5\xc6\xf7\xbc\xc7',
			wav: 'effect/\xc8\xed\xb1\xe2',
			attachedEntity: true
		}
	],

	209: [
		{
			//EF_POTION6	Green Herb/Potion
			type: 'STR',
			file: '\xc3\xca\xb7\xcf\xc6\xf7\xbc\xc7',
			attachedEntity: true
		}
	],

	210: [
		{
			//EF_POTION7	Yellow Circle Healing Effect
			type: 'STR',
			file: 'fruit',
			wav: '_heal_effect',
			attachedEntity: true
		}
	],

	211: [
		{
			//EF_POTION8	Blue Circle Healing Effect
			type: 'STR',
			file: 'fruit_',
			wav: 'effect/\xc8\xed\xb1\xe2',
			attachedEntity: true
		}
	],

	212: [
		{
			//EF_DARKBREATH	Dark Breath
			type: 'SPR',
			file: 'darkbreath',
			head: true,
			attachedEntity: true
		}
	],

	213: [
		{
			//EF_DEFFENDER	Defender
			type: 'STR',
			file: 'deffender',
			attachedEntity: true
		}
	],

	214: [
		{
			//EF_KEEPING	Keeping
			type: 'STR',
			file: 'keeping',
			attachedEntity: true
		}
	],

	215: [
		{
			//EF_SUMMONSLAVE
			type: 'SPR',
			file: 'smoke',
			attachedEntity: true
		}
	],

	216: [
		{
			//EF_BLOODDRAIN (Dracula / Beast Strafe)
			type: '3D',
			duration: 600,
			duplicate: 5,
			timeBetweenDupli: 0,
			absoluteSpriteName: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/particle1',
			playSprite: true,

			toSrc: true,
			rotateToTarget: true,

			// Blood Red
			red: 1.0,
			green: 0.4,
			blue: 0.4,

			sizeStart: 150.0,
			sizeEnd: 180.0,

			zOffsetStart: 5,
			arc: 3.0,
			retreat: 3.0,
			drainPattern: true
		}
	],

	217: [
		{
			//EF_ENERGYDRAIN (Wizard Soul Drain)
			type: '3D',
			duration: 600,
			duplicate: 5,
			timeBetweenDupli: 0,
			absoluteSpriteName: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/particle1',
			playSprite: true,

			toSrc: true,
			rotateToTarget: true,

			// Dark Blue
			red: 0.4,
			green: 0.4,
			blue: 1.0,

			sizeStart: 150.0,
			sizeEnd: 180.0,

			zOffsetStart: 5,
			arc: 3.0,
			retreat: 3.0,
			drainPattern: true
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					// Wait for drain particles from Part 1 (Soul Drain travel)
					if (tick < 500) {
						return false;
					}

					const time = tick - 500;

					// Part 2: Blue tint progression (200ms duration from official source)
					if (time < 200) {
						// Formula from official source: m_master->SetArgb(-1, 255-(m_stateCnt+50), 255-(m_stateCnt+50), 255)
						const val = (255 - (time + 50)) / 255;
						entity._flashColor[0] = val;
						entity._flashColor[1] = val;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return false;
					}

					// Part 3: Smooth fade back to normal (200ms) for premium feel
					if (time < 400) {
						const progress = (time - 200) / 200;
						const startVal = 5 / 255; // 255 - (200 + 50)
						const val = startVal + (1.0 - startVal) * progress;
						entity._flashColor[0] = val;
						entity._flashColor[1] = val;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return false;
					}

					// Final reset
					entity._flashColor[0] = 1.0;
					entity._flashColor[1] = 1.0;
					entity._flashColor[2] = 1.0;
					entity._flashColor[3] = 1.0;
					entity.recalculateBlendingColor();
					return true;
				});
			}
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				// make target blue
				const entity = Params.Init.otherEntity;
				if (!entity) return true;
				entity.animations.add(function (tick) {
					const time = tick;

					// Part 1: Blue tint progression (200ms duration from official source)
					if (time < 200) {
						// Formula from official source: m_master->SetArgb(-1, 255-(m_stateCnt+50), 255-(m_stateCnt+50), 255)
						const val = (255 - (time + 50)) / 255;
						entity._flashColor[0] = val;
						entity._flashColor[1] = val;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return false;
					}

					// Part 2: Smooth fade back to normal (200ms) for premium feel
					if (time < 400) {
						const progress = (time - 200) / 200;
						const startVal = 5 / 255; // 255 - (200 + 50)
						const val = startVal + (1.0 - startVal) * progress;
						entity._flashColor[0] = val;
						entity._flashColor[1] = val;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return false;
					}

					// Final reset
					entity._flashColor[0] = 1.0;
					entity._flashColor[1] = 1.0;
					entity._flashColor[2] = 1.0;
					entity._flashColor[3] = 1.0;
					entity.recalculateBlendingColor();
					return true;
				});
			}
		}
	],

	218: [
		{
			//EF_POTION_CON	Concentration Potion
			type: 'STR',
			file: '\xc1\xfd\xc1\xdf',
			wav: 'effect/ac_concentration',
			attachedEntity: true
		}
	],

	219: [
		{
			//EF_POTION_	Awakening Potion
			type: 'STR',
			file: '\xb0\xa2\xbc\xba',
			wav: 'effect/ac_concentration',
			attachedEntity: true
		}
	],

	220: [
		{
			//EF_POTION_BERSERK	Berserk Potion
			type: 'STR',
			file: '\xb9\xf6\xbc\xad\xc5\xa9',
			wav: 'effect/ac_concentration',
			attachedEntity: true
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 200;
				const duration = 200;
				Camera.setQuake(start, duration);
			}
		}
	],

	//221: [{}],	//EF_POTIONPILLAR	   Intense light beam

	222: [
		{
			//EF_DEFENDER	   Defender (Crusader)
			type: 'CYLINDER',
			alphaMax: 0.6,
			animation: 1,
			blendMode: 8,
			bottomSize: 1.5,
			topSize: 1.5,
			duration: 3000,
			fade: true,
			height: 10,
			rotate: true,
			textureName: 'ring_black',
			attachedEntity: true
		}
	],

	//223: [{}],	//EF_GANBANTEIN	   Holy Cast Aura
	//224: [{}],	//EF_WIND	   Wind (Map effect)
	//225: [{}],	//EF_VOLCANO	   Volcano casting effect

	226: [
		{
			//EF_GRANDCROSS	   Grand Cross Effect
			type: 'CYLINDER', // Center
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500
		},
		{
			type: 'CYLINDER', // West
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: -1
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: -2
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: -3
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: -4
		},
		{
			type: 'CYLINDER', // East
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: 1
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: 2
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: 3
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: 4
		},
		{
			type: 'CYLINDER', // North
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posY: 1
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posY: 2
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posY: 3
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posY: 4
		},
		{
			type: 'CYLINDER', // South
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posY: -1
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posY: -2
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posY: -3
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posY: -4
		},
		{
			type: 'CYLINDER', // NE
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: 1,
			posY: 1
		},
		{
			type: 'CYLINDER', // SE
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: 1,
			posY: -1
		},
		{
			type: 'CYLINDER', // SW
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: -1,
			posY: -1
		},
		{
			type: 'CYLINDER', // NW
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			topSize: 0.7,
			bottomSize: 0.7,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 45,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: -1,
			posY: 1
		},
		{
			type: 'CYLINDER', // NE arc
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 20,
			circleSides: 5,
			topSize: 3,
			bottomSize: 3,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: -180,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: 3.5,
			posY: 3.5
		},
		{
			type: 'CYLINDER', // SE arc
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 20,
			circleSides: 5,
			topSize: 3,
			bottomSize: 3,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 90,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: 3.5,
			posY: -3.5
		},
		{
			type: 'CYLINDER', // SW arc
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 20,
			circleSides: 5,
			topSize: 3,
			bottomSize: 3,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: 0,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: -3.5,
			posY: -3.5
		},
		{
			type: 'CYLINDER', // NW arc
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: false,
			blendMode: 2,
			totalCircleSides: 20,
			circleSides: 5,
			topSize: 3,
			bottomSize: 3,
			duration: 1000,
			fade: true,
			height: 5,
			rotate: false,
			angleY: -90,
			textureName: 'ring_red',
			duplicate: 3,
			timeBetweenDupli: 500,
			posX: -3.5,
			posY: 3.5
		},
		{
			wav: 'effect/cru_grand cross'
		}
	],

	227: [
		{
			//Intimidate / Snatch	//EF_INTIMIDATE	Snatch
			wav: 'effect/rog_intimidate',
			attachedEntity: true
		}
	],

	228: [
		{
			//EF_CHOOKGI	   (Nothing) - Used for spirit spheres (other classes)
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const spiritNum = Params.Init.spiritNum || 0;
				const Spheres = new SpiritSphere(Params.Init.ownerEntity, spiritNum, false);
				this.add(Spheres, Params);
			}
		}
	],

	229: [
		{
			//EF_CLOUD        Fog
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				CloudWeatherEffect.startOrRestart(Params);
			}
		}
	],

	230: [
		{
			//EF_CLOUD2       Normal Cloud (Under Map Gat)
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				CloudWeatherEffect.startOrRestart(Params);
			}
		}
	],

	//231: [{}],	//EF_MAPPILLAR	   Map Light Pillar Animation 1

	232: [
		{
			//EF_LINELINK	   Sacrifice (Visual Effect)
			type: '3D',
			alphaMax: 0.5,
			angle: 180,
			attachedEntity: true,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 0.9,
			duration: 100,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/alpha_center.tga',
			fromSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			sizeX: 5,
			sizeY: 50,
			zOffset: 1,
			zIndex: 1
		}
	],

	233: [
		{
			//EF_CLOUD3	   Einbroch Industrial Clouds
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				CloudWeatherEffect.startOrRestart(Params);
			}
		}
	],

	234: [
		{
			//EF_SPELLBREAKER	Spell Breaker
			type: 'STR',
			file: 'spell',
			wav: 'effect/sage_spell breake',
			attachedEntity: true
		}
	],

	235: [
		{
			//EF_DISPELL	Dispell
			type: 'STR',
			file: '\xb5\xf0\xbd\xba\xc6\xe7',
			attachedEntity: true
		}
	],

	//236: [{}],	//EF_DELUGE	   Deluge Cast Aura
	//237: [{}],	//EF_VIOLENTGALE	   Violent Gale Cast Aura
	//238: [{}],	//EF_LANDPROTECTOR	   Magnetic Earth Cast Aura
	239: [
		{
			//EF_BOTTOM_VO	   Volcano (Visual Effect)
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				this.add(
					new PropertyGround(Params.Inst.position, 3.0, 1.0, 2, 'ring_red', Params.Inst.startTick),
					Params
				);
			}
		}
	],
	240: [
		{
			//EF_BOTTOM_DE	   Deluge (Visual Effect)
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				this.add(
					new PropertyGround(Params.Inst.position, 3.0, 1.0, 2, 'ring_blue', Params.Inst.startTick),
					Params
				);
			}
		}
	],
	241: [
		{
			//EF_BOTTOM_VI	   Violent Gale (Visual Effect)
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				this.add(
					new PropertyGround(Params.Inst.position, 3.0, 1.0, 2, 'ring_yellow', Params.Inst.startTick),
					Params
				);
			}
		}
	],

	242: [
		{
			//EF_BOTTOM_LA	Magnetic Earth (Visual Effect)
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				this.add(new LPEffect(Params.Inst.position, Params.Inst.startTick), Params);
			}
		}
	],

	//243: [{}],	//EF_FASTMOVE	   (Invalid)

	244: [
		{
			//EF_MAGICROD	Magic Rod
			type: 'STR',
			file: '\xb8\xc5\xc1\xf7\xb7\xce\xb5\xe5',
			wav: 'effect/sage_magic rod',
			attachedEntity: true
		}
	],

	245: [
		{
			//EF_HOLYCROSS	Holy Cross
			type: 'STR',
			file: 'holy_cross',
			wav: 'effect/cru_holy cross',
			attachedEntity: true
		}
	],

	246: [
		{
			//EF_SHIELDCHARGE	Shield Charge
			type: 'STR',
			file: 'shield_charge',
			attachedEntity: true
		}
	],

	//247: [{}],	//EF_MAPPILLAR2	   Map Light Pillar Animation 2

	248: [
		{
			//EF_PROVIDENCE	Resistant Souls
			type: 'STR',
			file: 'providence',
			attachedEntity: true
		}
	],

	249: [
		{
			//EF_SHIELDBOOMERANG	Shield Boomerang
			wav: 'effect/cru_shield boomerang'
		}
	],

	250: [
		{
			//EF_SPEARQUICKEN	   Spear Quicken
			type: 'STR',
			file: 'twohand',
			wav: 'effect/knight_twohandquicken',
			head: true,
			attachedEntity: true
		}
	],

	251: [
		{
			//EF_DEVOTION	Devotion
			type: 'STR',
			file: 'devotion',
			attachedEntity: true
		}
	],

	252: [
		{
			//EF_REFLECTSHIELD	   Reflect Shield
			type: 'CYLINDER',
			alphaMax: 0.6,
			animation: 1,
			blendMode: 8,
			bottomSize: 1.5,
			topSize: 1.5,
			duration: 3000,
			fade: true,
			height: 10,
			rotate: true,
			textureName: 'ring_yellow',
			attachedEntity: true
		}
	],

	253: [
		{
			//EF_ABSORBSPIRITS	   Absorb Spirit Spheres
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1,
			bottomSize: 1.1,
			duration: 1500,
			fade: true,
			height: 15,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1.1,
			type: 'CYLINDER',
			wav: 'effect/\xc8\xed\xb1\xe2'
		},
		{
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1,
			bottomSize: 1,
			duration: 1500,
			fade: true,
			height: 13,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1,
			bottomSize: 1.1,
			duration: 1500,
			fade: true,
			height: 2,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1,
			duration: 1500,
			duplicate: 4,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			posxRand: 1.2,
			posyRand: 1.2,
			poszEndRand: 1,
			poszEndRandMiddle: 8,
			poszStart: 0,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			timeBetweenDupli: 10,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1,
			duration: 1300,
			delayOffset: 400,
			duplicate: 20,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			posxRand: 1.5,
			posyRand: 1.5,
			poszEndRand: 3,
			poszEndRandMiddle: 6,
			poszStart: 0,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			timeBetweenDupli: 10,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1,
			duration: 1100,
			delayLate: 200,
			duplicate: 10,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			posxRand: 1,
			posyRand: 1,
			poszEnd: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 0,
			size: 9,
			sizeRand: 2,
			timeBetweenDupli: 50,
			type: '3D',
			zIndex: 1
		}
	],

	254: [
		{
			//EF_STEELBODY	   Mental Strength (Visual Effect)
			wav: 'effect/mon_\xb1\xdd\xb0\xad\xba\xd2\xb1\xab'
		}
	],

	255: [
		{
			//EF_FLAMELAUNCHER	Elemental Endow (Fire)
			type: 'STR',
			file: 'enc_fire',
			wav: '_enemy_hit_wind1',
			attachedEntity: true
		}
	],

	256: [
		{
			//EF_FROSTWEAPON	Elemental Endow (Water)
			type: 'STR',
			file: 'enc_ice',
			wav: '_enemy_hit_wind1',
			attachedEntity: true
		}
	],

	257: [
		{
			//endow wind on target	//EF_LIGHTNINGLOADER	Elemental Endow (Wind)
			type: 'STR',
			file: 'enc_wind',
			wav: 'effect/_enemy_hit_wind1',
			attachedEntity: true
		}
	],

	258: [
		{
			//EF_SEISMICWEAPON	Elemental Endow (Earth)
			type: 'STR',
			file: 'enc_earth',
			wav: '_enemy_hit_wind1',
			attachedEntity: true
		}
	],

	//259: [{}],	//EF_MAPPILLAR3	   Map Light Pillar Animation 3
	//260: [{}],	//EF_MAPPILLAR4	   Map Light Pillar Animation 4

	261: [
		{
			//fury / critical explosion TODO: combo sounds, super novice fury	//EF_GUMGANG2	Fury Cast Animation
			type: 'CYLINDER',
			alphaMax: 0.5,
			blendMode: 8,
			animation: 4,
			duplicate: 4,
			timeBetweenDupli: 100,
			bottomSize: 2,
			duration: 1500,
			fade: true,
			height: 2,
			rotate: true,
			textureName: 'ring_yellow',
			topSize: 5,
			attachedEntity: true
		},
		{
			wav: 'effect/mon_\xc6\xf8\xb1\xe2',
			attachedEntity: true
		}
	],

	262: [
		{
			//EF_TEIHIT1	   Raging Quadruple Blow
			wav: 'effect/\x6d\x6f\x6e\x5f\xc6\xf8\xb1\xe2',
			type: '3D',
			duplicate: 12,
			timeBetweenDupli: 0,
			file: 'effect/alpha_center.tga',
			delayStart: 250,
			duration: 550,
			alphaMax: 0.8,
			fadeIn: true,
			fadeOut: true,
			posxStart: 0,
			posyStart: 0,
			posxEndRand: 40,
			posyEndRand: 40,
			sizeStartX: 10,
			sizeStartY: 150,
			sizeEndX: 10,
			sizeEndY: 150,
			blendMode: 2,
			attachedEntity: true,
			overlay: true,
			rotateToTarget: true,
			rotateWithCamera: true
		}
	],

	263: [
		{
			//EF_GUMGANG3	   Raging Quadruple Blow 2
			type: 'CYLINDER',
			alphaMax: 0.3,
			blendMode: 8,
			animation: 4,
			duplicate: 4,
			timeBetweenDupli: 100,
			bottomSize: 3,
			duration: 1000,
			fade: true,
			height: 2,
			rotate: true,
			textureName: 'ring_yellow',
			topSize: 6,
			attachedEntity: true
		}
	],

	//264: [{}],	//EF_TEIHIT2	   (Nothing)

	265: [
		{
			//EF_TANJI	   Throw Spirit Sphere
			type: '3D',
			file: 'effect/blue_ivy.bmp',
			alphaMax: 1,
			blendMode: 2,
			duration: 150,
			toSrc: true,
			rotateWithCamera: true,
			rotateToTarget: true,
			angle: 90,
			zOffset: 1,
			size: 50,
			wav: 'effect/mon_\xc5\xba\xc1\xf6\xbd\xc5\xc5\xeb',
			attachedEntity: true
		}
	],

	266: [
		{
			//EF_TEIHIT1X	   Asura Strike (Hit)
			wav: 'effect/\x6d\x6f\x6e\x5f\xbe\xc6\xbc\xf6\xb6\xf3\x20\xc6\xd0\xc8\xb2\xb1\xc7',
			type: '3D',
			duplicate: 24,
			timeBetweenDupli: 0,
			file: 'effect/lens1.tga',
			delayStart: 100,
			duration: 550,
			alphaMax: 0.8,
			fadeIn: true,
			fadeOut: true,
			posxStart: 0,
			posyStart: 0,
			posxEndRand: 40,
			posyEndRand: 40,
			sizeStartX: 10,
			sizeStartY: 150,
			sizeEndX: 10,
			sizeEndY: 150,
			blendMode: 2,
			attachedEntity: true,
			overlay: true,
			rotateToTarget: true,
			rotateWithCamera: true
		}
	],

	267: [
		{
			//EF_CHIMTO	   Occult Impaction
			wav: 'effect/mon_\xc4\xa7\xc5\xf5\xb0\xe6'
		}
	],

	268: [
		{
			//EF_STEALCOIN	Steal Coin
			type: 'STR',
			file: 'steal_coin',
			attachedEntity: true
		}
	],

	269: [
		{
			//EF_STRIPWEAPON	Divest Weapon
			type: 'STR',
			file: 'strip_weapon',
			wav: 'effect/\x74\x5f\xba\xae\xc6\xa8\xb1\xe8',
			attachedEntity: true
		}
	],

	270: [
		{
			//EF_STRIPSHIELD	Divest Shield
			type: 'STR',
			file: 'strip_shield',
			wav: 'effect/\x74\x5f\xba\xae\xc6\xa8\xb1\xe8',
			attachedEntity: true
		}
	],

	271: [
		{
			//EF_STRIPARMOR	Divest Armor
			type: 'STR',
			file: 'strip_armor',
			wav: 'effect/\x74\x5f\xba\xae\xc6\xa8\xb1\xe8',
			attachedEntity: true
		}
	],

	272: [
		{
			//EF_STRIPHELM	Divest Helm
			type: 'STR',
			file: 'strip_helm',
			wav: 'effect/\x74\x5f\xba\xae\xc6\xa8\xb1\xe8',
			attachedEntity: true
		}
	],

	273: [
		{
			//EF_CHAINCOMBO	Raging Quadruple Blow 4
			type: 'STR',
			file: '\xbf\xac\xc8\xaf',
			wav: 'effect/mon_\xbf\xac\xc8\xaf',
			attachedEntity: true
		}
	],

	274: [
		{
			//EF_RG_COIN	   Steal Coin Animation
			type: '2D',
			overlay: true,
			file: 'effect/coin_a.bmp',
			blendMode: 2,
			alphaMax: 0.8,
			duplicate: 30,
			timeBetweenDupli: 50,
			duration: 1500,
			fadeOut: true,
			posxEndRand: 10,
			posyEndRand: 10,
			posz: 2,
			size: 20,
			rotateToTarget: true,
			attachedEntity: true
		},
		{
			wav: 'effect/rog_steal coin'
		}
	],

	275: [
		{
			//backstab on target hit	//EF_BACKSTAP	Back Stab Animation
			wav: 'effect/rog_back stap',
			attachedEntity: true
		}
	],

	276: [
		{
			//Sightless Mind	//EF_TEIHIT3	Sightless Mind (RG_RAID)
			type: '3D',
			duplicate: 20,
			timeBetweenDupli: 0,
			file: 'effect/lens1.tga',
			delayStart: 100,
			duration: 550,
			alphaMax: 0.8,
			fadeIn: true,
			fadeOut: true,
			posxStart: 0,
			posyStart: 0,
			posxEndRand: 40,
			posyEndRand: 40,
			sizeStartX: 10,
			sizeStartY: 150,
			sizeEndX: 10,
			sizeEndY: 150,
			blendMode: 2,
			attachedEntity: true,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			overlay: true,
			rotateToTarget: true,
			rotateWithCamera: true
		}
	],

	'277_ground': [
		{
			// Dissonance //\xb1\xe2\xb7\xf9.tga
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				DissonanceEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	278: [
		{
			//EF_BOTTOM_LULLABY	Lullaby Map Unit
			wav: 'effect/\xc0\xda\xc0\xe5\xb0\xa1', //ŔÚŔĺ°ˇ
			attachedEntity: true
		}
	],

	'278_ground': [
		{
			// Lullaby
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				LullabyEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	279: [
		{
			//EF_BOTTOM_RICHMANKIM	Mr Kim a Rich Man Map Unit
			wav: 'effect/\xb1\xe8\xbc\xad\xb9\xe6\xb5\xb7', //±čĽ­ąćµ·
			attachedEntity: true
		}
	],

	'279_ground': [
		{
			// Mr Kim
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				MrKimEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	280: [
		{
			//EF_BOTTOM_ETERNALCHAOS	Eternal Chaos Map Unit
			wav: 'effect/\xbf\xb5\xbf\xf8\xc0\xc7\x20\xc8\xa5\xb5\xb7', //żµżřŔÇ ČĄµ·
			attachedEntity: true
		}
	],

	'280_ground': [
		{
			// Chaos
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				EtChaosEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	281: [
		{
			//EF_BOTTOM_DRUMBATTLEFIELD	A Drum on the Battlefield Map Unit
			wav: 'effect/\xc0\xfc\xc0\xe5\xc0\xc7', //ŔüŔĺŔÇ
			attachedEntity: true
		}
	],

	'281_ground': [
		{
			// Drum on battlefield
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				DrumEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	282: [
		{
			//EF_BOTTOM_RINGNIBELUNGEN	The Ring Of Nibelungen Map Unit
			wav: 'effect/\xb4\xcf\xba\xa7\xb7\xee\xb0\xd5\xc0\xc7\x20\xb9\xdd\xc1\xf6', //´Ďş§·î°ŐŔÇ ąÝÁö
			attachedEntity: true
		}
	],

	'282_ground': [
		{
			// Ring nibelun
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				NibelungEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	283: [
		{
			//EF_BOTTOM_ROKISWEIL	Loki's Veil Map Unit
			wav: 'effect/\xb7\xce\xc5\xb0', //·ÎĹ°
			attachedEntity: true
		}
	],

	'283_ground': [
		{
			// Loki Veil
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				LokiEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	284: [
		{
			//EF_BOTTOM_INTOABYSS	Into the Abyss Map Unit
			wav: 'effect/\xbd\xc9\xbf\xac\xbc\xd3\xc0\xb8\xb7\xce', //˝Éż¬ĽÓŔ¸·Î
			attachedEntity: true
		}
	],

	'284_ground': [
		{
			// Into abyss
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				AbyssEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	285: [
		{
			//EF_BOTTOM_SIEGFRIED	Invunerable Siegfriend Map Unit
			wav: 'effect/\xba\xd2\xbb\xe7\xbd\xc5', //şŇ»ç˝Ĺ
			attachedEntity: true
		}
	],

	'285_ground': [
		{
			// Invulnerable Sieg
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				SiegfiedEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	286: [
		{
			//EF_BOTTOM_WHISTLE	A Wistle Map Unit
			wav: 'effect/\xb4\xde\xba\xfb\xbc\xbc\xb7\xb9\xb3\xaa\xb5\xa5', //´ŢşűĽĽ·ąłŞµĄ
			attachedEntity: true
		}
	],

	'286_ground': [
		{
			// A whistle
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				WhistleEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	287: [
		{
			//EF_BOTTOM_ASSASSINCROSS	Assassin Cross of Sunset Map Unit
			wav: 'effect/\xbc\xae\xbe\xe7\xc0\xc7\x20\xbe\xee\xbd\xd8\xbd\xc5', //Ľ®ľçŔÇ ľî˝Ř˝Ĺ
			attachedEntity: true
		}
	],

	'287_ground': [
		{
			// Assassin cross
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				SinEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	288: [
		{
			//EF_BOTTOM_POEMBRAGI	A Poem of Bragi Map Unit
			wav: 'effect/\xba\xea\xb6\xf3\xb1\xe2\xc0\xc7\x20\xbd\xc3', //şę¶ó±âŔÇ ˝Ă
			attachedEntity: true
		}
	],

	'288_ground': [
		{
			// Bragi
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				const bragiNoteEffect = getBragiSpellNote();
				self.add(new bragiNoteEffect(Params.Inst.position, Params.Inst.startTick, 2), Params);
			}
		}
	],

	289: [
		{
			//EF_BOTTOM_APPLEIDUN	The Apple Of Idun Map Unit
			wav: 'effect/\xc0\xcc\xb5\xd0\xc0\xc7\x20\xbb\xe7\xb0\xfa',
			//ISO-8859-1:	ÀÌµÐÀÇ »ç°ú
			//Windows 1250: ŔĚµĐŔÇ »ç°ú
			//EUC-KR:		 이둔의 사과
			//Hex:			 \xc0\xcc\xb5\xd0\xc0\xc7\x20\xbb\xe7\xb0\xfa
			//reference for finding encoding
			attachedEntity: true
		}
	],

	'289_ground': [
		{
			// Apple
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				AppleEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	'290_ground': [
		{
			// Ugly
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				UglyEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	291: [
		{
			//EF_BOTTOM_HUMMING	Humming Map Unit
			wav: 'effect/\xc8\xef\xbe\xf3\xb0\xc5\xb8\xb2', //Čďľó°Ĺ¸˛
			attachedEntity: true
		}
	],

	'291_ground': [
		{
			// Humming
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				HummingEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	292: [
		{
			//EF_BOTTOM_DONTFORGETME	Please don't Forget Me Map Unit
			wav: 'effect/\xb3\xaa\xb8\xa6\xc0\xd8\xc1\xf6\xb8\xbb\xbe\xc6\xbf\xe4', //łŞ¸¦ŔŘÁö¸»ľĆżä
			attachedEntity: true
		}
	],

	'292_ground': [
		{
			// Dont forget
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				ForgetEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	293: [
		{
			//EF_BOTTOM_FORTUNEKISS	Fortune's Kiss Map Unit
			wav: 'effect/\xc7\xe0\xbf\xee\xc0\xc7', //ÇŕżîŔÇ
			attachedEntity: true
		}
	],

	'293_ground': [
		{
			// Fortune kiss /ladyluck
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				FortuneEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	294: [
		{
			//EF_BOTTOM_SERVICEFORYOU	Service For You Map Unit
			wav: 'effect/\xb4\xe7\xbd\xc5\xc0\xbb\x20\xc0\xa7\xc7\xd1\x20\xbc\xad\xba\xf1\xbd\xba', //´ç˝ĹŔ» Ŕ§ÇŃ Ľ­şń˝ş
			attachedEntity: true
		}
	],

	'294_ground': [
		{
			// Service
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				ServiceEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	295: [
		{
			//EF_TALK_FROSTJOKE	   Frost Joke
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				if (Params.Init.ownerAID == Session.Entity.GID) {
					const msg = DB.getRandomJoke();
					if (msg) {
						ChatBox.onRequestTalk('', msg, ChatBox.TYPE.PUBLIC);
					}
				}
			}
		}
	],

	296: [
		{
			//EF_TALK_SCREAM	   Scream
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				if (Params.Init.ownerAID == Session.Entity.GID) {
					const msg = DB.getRandomScream();
					if (msg) {
						ChatBox.onRequestTalk('', msg, ChatBox.TYPE.PUBLIC);
					}
				}
			}
		}
	],

	297: [
		{
			//EF_POKJUK	   Fire Works (Visual Effect)
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				PokJukWeatherEffect.startOrRestart(Params);
			}
		}
	],

	298: [
		{
			//EF_THROWITEM	   Acid Terror Animnation
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 300,
			fadeIn: true,
			fadeOut: true,
			file: '\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba/item/\xbf\xb0\xbb\xea\xba\xb4.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 0.5,
			size: 30,
			zOffset: 1,
			zIndex: 1
		}
	],

	//299: [{}],	//EF_THROWITEM2	   (Nothing)

	300: [
		{
			//EF_CHEMICALPROTECTION	   Chemical Protection
			wav: 'apocalips_attack'
		}
	],

	//301: [{}],	//EF_POKJUK_SOUND	   Fire Works (Sound Effect) [this is needed?]

	302: [
		{
			//EF_DEMONSTRATION	Bomb
			type: 'SPR',
			file: '\xb5\xa5\xb8\xf3\xbd\xba\xc6\xae\xb7\xb9\xc0\xcc\xbc\xc7',
			attachedEntity: false
		}
	],

	303: [
		{
			//EF_CHEMICAL2
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 132;
				Camera.setQuake(start);
			}
		}
	],

	304: [
		{
			//teleportation animation	//EF_TELEPORTATION2	Teleportation Animation
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.3,
			duration: 1500,
			fade: true,
			green: 0.6,
			height: 35,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.3,
			type: 'CYLINDER',
			wav: 'effect/ef_teleportation'
		},
		{
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.6,
			duration: 1500,
			fade: true,
			green: 0.6,
			height: 25,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.8,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.8,
			duration: 1500,
			fade: true,
			green: 0.6,
			height: 13,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			duration: 1500,
			fade: true,
			green: 0.6,
			height: 5,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1.3,
			type: 'CYLINDER'
		}
	],

	305: [
		{
			//potion success	//EF_PHARMACY_OK	Pharmacy Success
			type: 'STR',
			file: 'p_success',
			wav: 'effect/p_success',
			attachedEntity: true
		}
	],

	306: [
		{
			//potion failed	//EF_PHARMACY_FAIL	Pharmacy Failed
			type: 'STR',
			file: 'p_failed',
			wav: 'effect/p_failed',
			attachedEntity: true
		}
	],

	//307: [{}],	//EF_FORESTLIGHT	   Forest Light 1

	308: [
		{
			//EF_THROWITEM3	   Throw Stone
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 200,
			fadeIn: true,
			fadeOut: true,
			file: '\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba/item/\xb5\xb9.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 0.5,
			size: 20,
			zOffset: 1,
			zIndex: 1
		}
	],

	309: [
		{
			//EF_FIRSTAID	   First Aid
			type: '2D',
			alphaMax: 0.2,
			blendMode: 2,
			file: 'effect/pikapika2.bmp',
			fadeOut: true,
			duration: 1000,
			size: 100,
			posz: 2,
			wav: '_heal_effect'
		}
	],

	//310: [{}],	//EF_SPRINKLESAND	   Sprinkle Sand

	311: [
		{
			//crazy uproar	//EF_LOUD	Crazy Uproar
			type: 'STR',
			file: 'loud',
			wav: 'effect/\xb0\xed\xbc\xba\xb9\xe6\xb0\xa1',
			attachedEntity: true
		}
	],

	312: [
		{
			//EF_HEAL	   Heal Effect
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.7,
			bottomSize: 0.95,
			duration: 1500,
			fade: true,
			green: 1,
			height: 8,
			red: 0.7,
			rotate: true,
			textureName: 'ring_white',
			topSize: 0.95,
			type: 'CYLINDER',
			wav: '_heal_effect'
		},
		{
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.7,
			bottomSize: 1,
			duration: 1500,
			fade: true,
			green: 1,
			height: 8,
			red: 0.7,
			rotate: true,
			textureName: 'ring_white',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1300,
			delayOffset: 400,
			duplicate: 15,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1.5,
			posyRand: 1.5,
			poszEndRand: 2,
			poszEndRandMiddle: 6,
			poszStart: 0,
			red: 1,
			size: 9,
			sizeRand: 2,
			timeBetweenDupli: 10,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1100,
			delayLate: 200,
			duplicate: 7,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1,
			posyRand: 1,
			poszEnd: 5,
			poszStartRand: 1,
			poszStartRandMiddle: 0,
			red: 1,
			size: 9,
			sizeRand: 2,
			timeBetweenDupli: 50,
			type: '3D',
			zIndex: 1
		}
	],

	313: [
		{
			//EF_HEAL2	   Heal Effect 2
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.7,
			bottomSize: 1.1,
			duration: 1500,
			fade: true,
			green: 1,
			height: 15,
			red: 0.7,
			rotate: true,
			textureName: 'ring_white',
			topSize: 1.1,
			type: 'CYLINDER',
			wav: '_heal_effect'
		},
		{
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.7,
			bottomSize: 1,
			duration: 1500,
			fade: true,
			green: 1,
			height: 13,
			red: 0.7,
			rotate: true,
			textureName: 'ring_white',
			topSize: 1,
			type: 'CYLINDER',
			wav: '_heal_effect'
		},
		{
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.7,
			bottomSize: 1.1,
			duration: 1500,
			fade: true,
			green: 1,
			height: 2,
			red: 0.7,
			rotate: true,
			textureName: 'ring_white',
			topSize: 3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1500,
			duplicate: 4,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1.2,
			posyRand: 1.2,
			poszEndRand: 1,
			poszEndRandMiddle: 8,
			poszStart: 0,
			red: 1,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			timeBetweenDupli: 10,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1300,
			delayOffset: 400,
			duplicate: 20,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1.5,
			posyRand: 1.5,
			poszEndRand: 3,
			poszEndRandMiddle: 6,
			poszStart: 0,
			red: 1,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			timeBetweenDupli: 10,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1100,
			delayLate: 200,
			duplicate: 10,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1,
			posyRand: 1,
			poszEnd: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 0,
			red: 1,
			size: 9,
			sizeRand: 2,
			timeBetweenDupli: 50,
			type: '3D',
			zIndex: 1
		}
	],

	314: [
		{
			//EF_EXIT2	   Old Map Exit effect (Unused)
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.3,
			duration: 1500,
			fade: true,
			green: 0.5,
			height: 35,
			red: 0.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.3,
			type: 'CYLINDER',
			wav: 'effect/ef_teleportation'
		},
		{
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.4,
			duration: 1500,
			fade: true,
			green: 0.5,
			height: 23,
			red: 0.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.6,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.5,
			duration: 1500,
			fade: true,
			green: 0.5,
			height: 5,
			red: 0.5,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.7,
			type: 'CYLINDER'
		}
	],

	315: [
		{
			//EF_GLASSWALL2	Safety Wall
			attachedEntity: true,
			file: 'safetywall',
			repeat: false,
			type: 'STR',
			wav: 'effect/ef_glasswall'
		},
		{
			alphaMax: 0.4,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.5,
			bottomSize: 0.6,
			duration: 50000,
			fade: true,
			green: 0.1,
			height: 7,
			red: 0.5,
			rotate: true,
			textureName: 'alpha_down',
			topSize: 0.6,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.4,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.5,
			bottomSize: 0.65,
			duration: 50000,
			delayOffset: 50,
			fade: true,
			green: 0.1,
			height: 6,
			red: 0.5,
			rotate: true,
			textureName: 'alpha_down',
			topSize: 0.65,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.4,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.5,
			bottomSize: 0.7,
			duration: 50000,
			delayOffset: 100,
			fade: true,
			green: 0.1,
			height: 7,
			red: 0.5,
			rotate: true,
			textureName: 'alpha_down',
			topSize: 0.7,
			type: 'CYLINDER'
		}
	],

	316: [
		{
			//Warp Portal Animation 1	//EF_READYPORTAL2	Warp Portal Animation 1
			alphaMax: 0.4,
			animation: 4,
			attachedEntity: true,
			blendMode: 2,
			renderBeforeEntities: true,
			blue: 1,
			bottomSize: 2.4,
			duration: 500,
			fadeOut: true,
			green: 0.6,
			height: 0.1,
			posZ: 0.1,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			repeat: true,
			repeatDelay: -300,
			topSize: 3.9,
			wav: 'effect/ef_readyportal',
			type: 'CYLINDER'
		}
	],

	317: [
		{
			//Warp Portal Animation 2	//EF_PORTAL2	Warp Portal Animation 2
			alphaMax: 0.4,
			animation: 4,
			attachedEntity: true,
			blendMode: 2,
			renderBeforeEntities: true,
			blue: 1,
			bottomSize: 2.4,
			duration: 500,
			fadeOut: true,
			green: 0.6,
			height: 0.1,
			posZ: 0.1,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			repeat: true,
			repeatDelay: -300,
			topSize: 3.9,
			wav: 'effect/ef_readyportal',
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.3,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.6,
			duration: 25000,
			fade: true,
			green: 0.6,
			height: 15,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.6,
			wav: 'effect/ef_portal',
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.3,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.8,
			duration: 25000,
			fade: true,
			green: 0.6,
			height: 13,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.8,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.5,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			duration: 25000,
			fade: true,
			green: 0.6,
			height: 1,
			posZ: 2,
			red: 0.6,
			rotate: true,
			totalCircleSides: 20,
			circleSides: 10,
			repeatTextureX: 2,
			textureName: 'alpha1',
			topSize: 1,
			type: 'CYLINDER'
		}
	],

	soulink_caster_effect: [
		{
			// todo
			wav: 'effect/\x74\x5f\xba\xae\xc6\xa8\xb1\xe8',
			attachedEntity: false
		}
	],

	soulink_target_effect: [
		{
			// todo
			wav: 'effect/\x74\x5f\xbf\xb5\xc8\xa5', //t_żµČĄ
			attachedEntity: true
		}
	],

	gunslinger_coin: [
		{
			// coin caster
			wav: 'effect/\xc7\xc3\xb8\xb3', //ÇĂ¸ł
			attachedEntity: true
		}
	],

	ef_coldbolt: [
		{
			//coldbolt falling objects
			type: '3D',
			alphaMax: 1,
			angle: 112.5,
			duration: 500,
			file: 'effect/icearrow.tga',
			posxEnd: 0,
			posxStartRandMiddle: 5,
			posxStartRand: 1,
			posyEnd: 0,
			posyStartRandMiddle: 2,
			posyStartRand: 1,
			poszEnd: 0,
			poszStart: 20,
			size: 50,
			zIndex: 1,
			wav: 'effect/ef_icearrow%d',
			rand: [1, 3],
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			alphaMax: 0.7,
			animation: 4,
			attachedEntity: false,
			bottomSize: 3,
			duration: 1000,
			delayLate: 500,
			fade: true,
			height: 0.1,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 5
		}
	],

	ef_firebolt: [
		{
			//firebolt falling objects
			type: '3D',
			alphaMax: 1,
			blendMode: 2,
			angle: 112.5,
			duration: 500,
			fileList: [
				'effect/\xba\xd2\xc8\xad\xbb\xec1.tga',
				'effect/\xba\xd2\xc8\xad\xbb\xec2.tga',
				'effect/\xba\xd2\xc8\xad\xbb\xec3.tga',
				'effect/\xba\xd2\xc8\xad\xbb\xec4.tga',
				'effect/\xba\xd2\xc8\xad\xbb\xec5.tga',
				'effect/\xba\xd2\xc8\xad\xbb\xec6.tga'
				//,'effect/\xba\xd2\xc8\xad\xbb\xec7.tga' //Not used officially
				//,'effect/\xba\xd2\xc8\xad\xbb\xec8.tga'
			],
			frameDelay: 30,
			posxEnd: 0,
			posxStartRandMiddle: 5,
			posxStartRand: 1,
			posyEnd: 0,
			posyStartRandMiddle: 2,
			posyStartRand: 1,
			poszEnd: 0,
			poszStart: 20,
			sizeX: 100,
			sizeY: 50,
			zIndex: 1,
			wav: 'effect/ef_firearrow%d',
			rand: [1, 3],
			attachedEntity: true
		}
	],

	318: [
		{
			//EF_BOTTOM_MAG	Magnus Exorcisimus Map Unit
			alphaMax: 0.2,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 0.7,
			duration: 50000,
			fade: false,
			height: 5,
			rotate: false,
			textureName: 'ring_red',
			topSize: 0.7,
			type: 'CYLINDER',
			angleY: 45
		},
		{
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 0.7,
			duration: 2000,
			fade: true,
			height: 5,
			rotate: false,
			textureName: 'ring_red',
			topSize: 0.7,
			type: 'CYLINDER',
			angleY: 45,
			repeat: true
		}
	],

	319: [
		{
			//EF_BOTTOM_SANC	Sanctuary Map Unit
			alphaMax: 0.3,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.5,
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 0.7,
			duration: 50000,
			fade: false,
			green: 0.9,
			height: 2,
			red: 0.5,
			rotate: false,
			textureName: 'magic_green',
			topSize: 0.7,
			type: 'CYLINDER',
			angleY: 45
		},
		{
			alphaMax: 0.1,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.5,
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 0.7,
			duration: 2000,
			fade: true,
			green: 0.9,
			height: 2,
			red: 0.5,
			rotate: false,
			textureName: 'magic_green',
			topSize: 0.7,
			type: 'CYLINDER',
			angleY: 45,
			repeat: true
		}
	],

	320: [
		{
			//EF_HEAL3	   Offensive Heal
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.95,
			duration: 1000,
			fade: true,
			green: 1,
			height: 10,
			red: 1,
			rotate: true,
			textureName: 'ring_white',
			topSize: 0.95,
			type: 'CYLINDER',
			wav: '_heal_effect'
		},
		{
			alphaMax: 0.2,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			duration: 1000,
			fade: true,
			green: 1,
			height: 9,
			red: 1,
			rotate: true,
			textureName: 'ring_white',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1000,
			delayOffset: 400,
			duplicate: 10,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1.5,
			posyRand: 1.5,
			poszEndRand: 3,
			poszEndRandMiddle: 6,
			poszStart: 0,
			red: 1,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			timeBetweenDupli: 10,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 900,
			delayLate: 200,
			duplicate: 5,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1,
			posyRand: 1,
			poszEnd: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 0,
			red: 1,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 2,
			timeBetweenDupli: 50,
			type: '3D',
			zIndex: 1
		}
	],

	321: [
		{
			//EF_WARPZONE2	   Warp NPC
			alphaMax: 0.4,
			animation: 3,
			renderBeforeEntities: true,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 2,
			duration: 4000,
			duplicate: 4,
			timeBetweenDupli: 1000,
			fade: true,
			red: 0.5,
			green: 0.5,
			blue: 1,
			height: 1.1,
			repeat: true,
			rotate: false,
			textureName: 'ring_blue',
			topSize: 3.3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.4,
			animation: 3,
			renderBeforeEntities: true,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 1.9,
			duration: 4000,
			duplicate: 4,
			timeBetweenDupli: 1000,
			fade: true,
			red: 0.5,
			green: 0.5,
			blue: 1,
			height: 1.1,
			repeat: true,
			rotate: false,
			textureName: 'ring_blue',
			topSize: 3.2,
			type: 'CYLINDER'
		},
		{
			alphaMax: 1.0,
			attachedEntity: true,
			blendMode: 2,
			renderBeforeEntities: true,
			duration: 1000,
			duplicate: 5,
			timeBetweenDupli: 300,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok1.tga',
			red: 0.9,
			green: 1,
			blue: 0.9,
			posxStartRand: 3,
			posxStartRandMiddle: 0,
			posyStartRand: 3,
			posyStartRandMiddle: 0,
			poszEndRand: 2,
			poszEndRandMiddle: 2,
			poszStart: 0,
			repeat: true,
			size: 50,
			type: '3D',
			zIndex: 1
		}
	],
	//322: [{}],	//EF_FORESTLIGHT2	   Forest Light 2
	//323: [{}],	//EF_FORESTLIGHT3	   Forest Light 3
	//324: [{}],	//EF_FORESTLIGHT4	   Forest Light 4
	325: [
		{
			//EF_HEAL4	   Heal Effect 4
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.7,
			bottomSize: 1.1,
			duration: 1500,
			fade: true,
			green: 1,
			height: 18,
			red: 0.7,
			rotate: true,
			textureName: 'ring_white',
			topSize: 1.1,
			type: 'CYLINDER',
			wav: '_heal_effect'
		},
		{
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.7,
			bottomSize: 1,
			duration: 1500,
			fade: true,
			green: 1,
			height: 15,
			red: 0.7,
			rotate: true,
			textureName: 'ring_white',
			topSize: 1,
			type: 'CYLINDER',
			wav: '_heal_effect'
		},
		{
			alphaMax: 0.3,
			animation: 1,
			attachedEntity: true,
			blendMode: 2,
			blue: 0.7,
			bottomSize: 1.1,
			duration: 1500,
			fade: true,
			green: 1,
			height: 3,
			red: 0.7,
			rotate: true,
			textureName: 'ring_white',
			topSize: 3,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1500,
			duplicate: 7,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1.2,
			posyRand: 1.2,
			poszEndRand: 1,
			poszEndRandMiddle: 8,
			poszStart: 0,
			red: 1,
			size: 9,
			sizeRand: 2,
			sparkling: true,
			sparkNumber: 3,
			timeBetweenDupli: 10,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1300,
			delayOffset: 400,
			duplicate: 25,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1.5,
			posyRand: 1.5,
			poszEndRand: 3,
			poszEndRandMiddle: 6,
			poszStart: 0,
			red: 1,
			size: 10,
			sizeRand: 5,
			sparkling: true,
			sparkNumber: 3,
			timeBetweenDupli: 10,
			type: '3D',
			zIndex: 1
		},
		{
			alphaMax: 0.8,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			duration: 1100,
			delayLate: 200,
			duplicate: 15,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			green: 1,
			posxRand: 1,
			posyRand: 1,
			poszEnd: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 0,
			red: 1,
			size: 11,
			sizeRand: 2,
			timeBetweenDupli: 50,
			type: '3D',
			zIndex: 1
		}
	],
	//326: [{}],	//EF_FOOT	   Chase Walk Left Foot
	//327: [{}],	//EF_FOOT2	   Chse Walk Right Foot

	328: [
		{
			//EF_BEGINASURA	   Monk Asura Strike
			type: 'CYLINDER',
			textureName: 'ring_white',
			fade: true,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 1,
			topSize: 4.5,
			height: -4,
			duration: 800
		},
		{
			type: 'CYLINDER',
			textureName: 'ring_white',
			fade: true,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 1,
			topSize: 2.5,
			height: -4,
			duration: 800
		},
		// asura1 — phase 1: appear + shrink
		{
			type: '3D',
			file: 'effect/asura1.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 250,
			sizeEnd: 120,
			sizeSmooth: true,
			posx: -6.0,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 0
		},
		// asura1 — phase 2: grow-back + fade out
		{
			type: '3D',
			file: 'effect/asura1.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 120,
			sizeEnd: 200,
			posx: -6.0,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1200
		},
		// asura2 — phase 1
		{
			type: '3D',
			file: 'effect/asura2.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 250,
			sizeEnd: 120,
			sizeSmooth: true,
			posx: -3.6,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 100
		},
		// asura2 — phase 2
		{
			type: '3D',
			file: 'effect/asura2.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 120,
			sizeEnd: 200,
			posx: -3.6,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1300
		},
		// asura3 — phase 1
		{
			type: '3D',
			file: 'effect/asura3.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 250,
			sizeEnd: 120,
			sizeSmooth: true,
			posx: -1.2,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 200
		},
		// asura3 — phase 2
		{
			type: '3D',
			file: 'effect/asura3.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 120,
			sizeEnd: 200,
			posx: -1.2,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1400
		},
		// asura4 — phase 1
		{
			type: '3D',
			file: 'effect/asura4.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 250,
			sizeEnd: 120,
			sizeSmooth: true,
			posx: 1.2,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 300
		},
		// asura4 — phase 2
		{
			type: '3D',
			file: 'effect/asura4.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 120,
			sizeEnd: 200,
			posx: 1.2,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1500
		},
		// asura5 — phase 1
		{
			type: '3D',
			file: 'effect/asura5.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 250,
			sizeEnd: 120,
			sizeSmooth: true,
			posx: 3.6,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 400
		},
		// asura5 — phase 2
		{
			type: '3D',
			file: 'effect/asura5.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 120,
			sizeEnd: 200,
			posx: 3.6,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1600
		},
		// asura6 — phase 1
		{
			type: '3D',
			file: 'effect/asura6.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 250,
			sizeEnd: 120,
			sizeSmooth: true,
			posx: 6.0,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 500
		},
		// asura6 — phase 2
		{
			type: '3D',
			file: 'effect/asura6.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 120,
			sizeEnd: 200,
			posx: 6.0,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1700
		}
	],

	329: [
		{
			//EF_TRIPLEATTACK	   Triple Strike
			wav: 'effect/ef_hit2'
		},
		{
			wav: 'effect/ef_hit4',
			delayWav: 200
		},
		{
			wav: 'effect/ef_hit2',
			delayWav: 400
		}
	],

	//330: [{}],	//EF_HITLINE	   Combo Finish

	331: [
		{
			//EF_HPTIME	   Natural HP Regeneration
			type: '3D',
			duplicate: 12,
			file: 'effect/pok1.tga',
			delay: 500,
			timeBetweenDupli: 10,
			alphaMax: 0.8,
			fadeOut: true,
			fadeIn: true,
			zIndex: 1,
			size: 30,
			sizeRand: 20,
			red: 0.9,
			green: 1,
			blue: 0.9,
			blendMode: 2,
			poszStartRand: 1.5,
			poszStartRandMiddle: 2,
			poszEndRandMiddle: 5,
			poszEndRand: 1,
			sparkling: true,
			sparkNumberRand: [3, 6],
			posxRand: 0.6,
			posyRand: 0.6,
			attachedEntity: true
		},
		{
			wav: '_heal_effect'
		}
	],

	332: [
		{
			//EF_SPTIME	   Natural SP Regeneration
			type: '3D',
			duplicate: 12,
			file: 'effect/pok1.tga',
			delay: 500,
			timeBetweenDupli: 10,
			alphaMax: 0.8,
			fadeOut: true,
			fadeIn: true,
			zIndex: 1,
			size: 30,
			sizeRand: 20,
			red: 0.9,
			green: 0.9,
			blue: 1,
			blendMode: 2,
			poszStartRand: 1.5,
			poszStartRandMiddle: 2,
			poszEndRandMiddle: 5,
			poszEndRand: 1,
			sparkling: true,
			sparkNumberRand: [3, 6],
			posxRand: 0.6,
			posyRand: 0.6,
			attachedEntity: true
		},
		{
			wav: 'effect/\xc8\xed\xb1\xe2'
		}
	],

	333: [
		{
			//EF_MAPLE	   Autumn Leaves
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				SakuraWeatherEffect.startOrRestart(Params);
			}
		}
	],

	334: [
		{
			//EF_BLIND	   Blind
			type: 'FUNC',
			attachedEntity: false,
			wav: '_blind',
			func: function (Params) {
				Blind.setActive(true);
			}
			/*file: 'effect/fullb.tga',
			file: 'effect/white02.bmp',*/
		}
	],

	335: [
		{
			//EF_POISON	   Poison
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				Poison.setActive(true);
			}
		}
	],

	336: [
		{
			//kyrie eleison / parrying	(when target blocked dmg)	//EF_GUARD	Kyrie Eleison/Parrying Shield
			wav: 'effect/kyrie_guard',
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'guardk',
			blendMode: 2,
			duration: 600,
			fade: true,
			totalCircleSides: 8,
			circleSides: 5,
			repeatTextureX: 5,
			angleY: 112.5,
			bottomSize: 1.5,
			topSize: 1,
			height: 0.7,
			posZ: 2.14,
			alphaMax: 0.6,
			red: 0.91,
			green: 1.0,
			blue: 0.9,
			rotateWithSource: true,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'guardk',
			blendMode: 2,
			duration: 600,
			fade: true,
			totalCircleSides: 8,
			circleSides: 5,
			repeatTextureX: 5,
			angleY: 112.5,
			bottomSize: 1.5,
			topSize: 1.5,
			height: 1.14,
			posZ: 1,
			alphaMax: 0.6,
			red: 0.91,
			green: 1.0,
			blue: 0.9,
			rotateWithSource: true,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'guardk',
			blendMode: 2,
			duration: 600,
			fade: true,
			totalCircleSides: 8,
			circleSides: 5,
			repeatTextureX: 5,
			angleY: 112.5,
			bottomSize: 1,
			topSize: 1.5,
			height: 0.7,
			posZ: 0.3,
			alphaMax: 0.6,
			red: 0.91,
			green: 1.0,
			blue: 0.9,
			rotateWithSource: true,
			attachedEntity: true
		}
	],

	337: [
		{
			//EF_JOBLVUP50	Class Change
			type: 'STR',
			file: 'joblvup',
			attachedEntity: true
		}
	],

	//338: [{}],	//EF_ANGEL2	   Super Novice/Taekwon Level Up Angel

	'339_beforecast': [
		{
			//EF_MAGNUM2	   Spiral Pierce
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;
				entity._flashColor[0] = 1.0;
				entity._flashColor[1] = 1.0;
				entity._flashColor[2] = 1.0;
				entity._flashColor[3] = 1.0;
				entity.recalculateBlendingColor();

				entity.animations.add(function (tick) {
					if (!entity.cast.display) {
						//we don't know cast time here so.. let's hack
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return true;
					}
					entity._flashColor[1] = 0.0 + Math.sin(tick / (6 * Math.PI));
					entity._flashColor[2] = 0.0 + Math.sin(tick / (6 * Math.PI));
					entity.recalculateBlendingColor();
				});
			}
		}
	],

	339: [
		{
			//EF_MAGNUM2	   Spiral Pierce
			wav: 'permeter_attack'
		},
		{
			wav: 'effect/ef_magnumbreak',
			delayWav: 300
		}
	],

	//340: [{}],	//EF_CALLZONE	   (Nothing)
	//341: [{}],	//EF_PORTAL3	   Wedding Warp Portal
	//342: [{}],	//EF_COUPLECASTING	   Wedding Skill
	//343: [{}],	//EF_HEARTCASTING	   Another Merry Skill

	344: [
		{
			//EF_ENTRY2	   Character map entry effect
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.3,
			duration: 1500,
			fade: true,
			green: 0.6,
			height: 35,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.3,
			type: 'CYLINDER',
			wav: 'effect/ef_portal'
		},
		{
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.6,
			duration: 1500,
			fade: true,
			green: 0.6,
			height: 25,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 0.8,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 0.8,
			duration: 1500,
			fade: true,
			green: 0.6,
			height: 13,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.5,
			animation: 5,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			duration: 1500,
			fade: true,
			green: 0.6,
			height: 5,
			red: 0.6,
			rotate: true,
			textureName: 'ring_blue',
			topSize: 1.3,
			type: 'CYLINDER'
		}
	],

	//345: [{}],	//EF_SAINTWING	   Wings (Animated)
	//346: [{}],	//EF_SPHEREWIND	   Like Moonlight But Blue

	347: [
		{
			//EF_COLORPAPER	   Wedding Ceremony
			wav: 'effect/wedding'
		}
	],

	//348: [{}],	//EF_LIGHTSPHERE	   Like 1000 Blade trepassing
	349: [{ type: 'WATERFALL', variant: 'large', vertical: false }], // EF_WATERFALL
	350: [{ type: 'WATERFALL', variant: 'large', vertical: true }], // EF_WATERFALL_90
	351: [{ type: 'WATERFALL', variant: 'small', vertical: false }], // EF_WATERFALL_SMALL
	352: [{ type: 'WATERFALL', variant: 'small', vertical: true }], // EF_WATERFALL_SMALL_90
	353: [{ type: 'WATERFALL', variant: 'dark-large', vertical: false }], // EF_WATERFALL_T2
	354: [{ type: 'WATERFALL', variant: 'dark-large', vertical: true }], // EF_WATERFALL_T2_90
	355: [{ type: 'WATERFALL', variant: 'dark-small', vertical: false }], // EF_WATERFALL_SMALL_T2
	356: [{ type: 'WATERFALL', variant: 'dark-small', vertical: true }], // EF_WATERFALL_SMALL_T2
	//357: [{}],	//EF_MINI_TETRIS	   (Nothing)
	//358: [{}],	//EF_GHOST	   Niflheim Ghost
	//359: [{}],	//EF_BAT	   Niflheim Bat Slow
	//360: [{}],	//EF_BAT2	   Niflheim Bat Fast

	361: [
		{
			//EF_SOULBREAKER	   Soul Destroyer
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.4,
			duration: 500,
			fadeIn: true,
			fadeOut: true,
			zIndex: 10,
			toSrc: true,
			rotateWithCamera: true,
			rotateToTarget: true,
			angle: 90,
			posz: 2,
			sizeStart: 100,
			sizeEnd: 200,
			wav: 'effect/\xb1\xe2\xb0\xf8\xc6\xf7',
			attachedEntity: true
		}
	],

	362: [
		{
			//EF_LEVEL99_4	   Trancendant Level 99 Aura 1
			//effect\\whitelight.tga
			//Level99_3(1)
		}
	],

	363: [
		{
			//EF_VALLENTINE	Valentine Day Heart With Wings
			type: 'SPR',
			file: 'vallentine',
			wav: 'effect/vallentine',
			attachedEntity: true
		}
	],

	//364: [{}],	//EF_VALLENTINE2	   Valentine Day Heart

	365: [
		{
			//EF_PRESSURE	   Gloria Domini
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 500,
			rotate: true,
			angle: 0,
			toAngle: -611,
			poszStart: 20,
			poszEnd: 5,
			size: 100,
			zIndex: 1,
			file: 'effect/cross_old.bmp',
			wav: 'effect/\xc7\xc1\xb7\xb9\xbc\xc5'
		},
		{
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 500,
			delayOffset: 501,
			fadeOut: true,
			angle: -611,
			posz: 5,
			size: 100,
			zIndex: 1,
			file: 'effect/cross_old.bmp'
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 500;
				Camera.setQuake(start);
			}
		}
	],

	366: [
		{
			//martyr's reckoning	//EF_BASH3D	   Martyr's Reckoning
			wav: 'effect/bash3d', // incorrect wav name
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					// Official: Halo (Double Body) is active between tick 20 and 40.
					entity._enableHalo = tick >= 150 && tick <= 450;

					if (tick >= 150 && tick <= 450) {
						// SetArgb(-1, 255, 200, 200) -> Light Red
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 0.78;
						entity._flashColor[2] = 0.78;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
					} else {
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
					}
					return tick > 500;
				});
			}
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			delayStart: 200,
			duplicate: 5,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 4.5,
			animation: 2,
			zIndex: 1.1,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			duplicate: 5,
			delayStart: 200,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 7.2,
			animation: 2,
			zIndex: 1.2,
			attachedEntity: true
		}
	],

	367: [
		{
			//aura blade	//EF_AURABLADE	Aura Blade
			wav: 'effect/\xbf\xc0\xb6\xf3\x20\xba\xed\xb7\xb9\xc0\xcc\xb5\xe5', //żŔ¶ó şí·ąŔĚµĺ
			attachedEntity: true
			//+ on cast small white-magic aura (double)
		}
	],

	368: [
		{
			//EF_REDBODY	Berserk
			wav: 'effect/\xb9\xf6\xbc\xad\xc5\xa9',
			attachedEntity: true
			//shake screen
		}
	],

	369: [
		{
			//EF_LKCONCENTRATION	Concentration
			type: 'STR',
			file: 'twohand',
			wav: 'effect/knight_twohandquicken',
			head: true,
			attachedEntity: true
		}
	],

	370: [
		{
			//EF_BOTTOM_GOSPEL	Gospel Map Unit
			wav: 'effect/\xb0\xa1\xbd\xba\xc6\xe7', //°ˇ˝şĆç
			attachedEntity: true
		}
	],

	'370_ground': [
		{
			// Gospel
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				GospelEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	371: [
		{
			//EF_ANGEL	Level Up
			type: 'STR',
			file: 'angel',
			wav: 'levelup',
			attachedEntity: true
		}
	],

	372: [
		{
			//EF_DEVIL	Death
			type: 'STR',
			file: 'devil',
			attachedEntity: true
		}
	],

	373: [
		{
			//EF_DRAGONSMOKE	House Smoke
			type: 'SPR',
			file: 'poisonhit',
			attachedEntity: true
		}
	],

	374: [
		// EF_BOTTOM_BASILICA
		{
			// F1=1, GI[0] - Alpha 32, Height 14.0
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 2.45,
			topSize: 2.45,
			height: 2.0,
			alphaMax: 32 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			duration: 20000,
			attachedEntity: true
		},
		{
			// F1=1, GI[1] - Alpha 32, Height 14.4
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 2.52,
			topSize: 2.52,
			height: 2.1,
			alphaMax: 32 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 10,
			duration: 20000,
			attachedEntity: true
		},
		{
			// F1=1, GI[2] - Alpha 15, Distance 26.6
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 2.6,
			topSize: 2.6,
			height: 2.0,
			alphaMax: 15 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 26.6,
			duration: 20000,
			attachedEntity: true
		},
		{
			// F1=1, GI[3] - Alpha 15, Distance 79.8 (53.2+26.6)
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 2.6,
			topSize: 2.6,
			height: 2.0,
			alphaMax: 15 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 79.8,
			duration: 20000,
			attachedEntity: true
		}
	],

	//375: [{}],	//EF_ASSUMPTIO	   Assumptio (Visual Effect)
	376: [
		{
			//EF_HITLINE2	   Palm Strike
			wav: 'effect/\xb8\xcd\xc8\xa3\xb0\xe6\xc6\xc4\xbb\xea'
		}
	],
	377: [
		{
			//martyr's reckoning 2	//EF_BASH3D2	   Martyr's Reckoning 2
			wav: 'effect/\x6d\x6f\x6e\x5f\xc6\xf8\xb1\xe2', //mon_¿¬È¯? can be incorrect wav name
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					// Official: Halo (Double Body) is active between tick 5 and 35.
					entity._enableHalo = tick >= 50 && tick <= 350;

					if (tick >= 50 && tick <= 350) {
						// SetArgb(-1, 255, 100, 100) -> Red
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 0.39;
						entity._flashColor[2] = 0.39;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
					} else {
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
					}
					return tick > 400;
				});
			}
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			delayStart: 50,
			duplicate: 8,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 4.5,
			animation: 2,
			zIndex: 1.1,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			duplicate: 8,
			delayStart: 50,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 7.2,
			animation: 2,
			zIndex: 1.2,
			attachedEntity: true
		}
	],

	378: [
		{
			//EF_ENERGYDRAIN2          Energy Drain (1st Part)
			type: '3D',
			duration: 600,
			duplicate: 5,
			timeBetweenDupli: 0,
			absoluteSpriteName: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/particle1',
			playSprite: true,

			fromSrc: true,
			toSrc: true,
			rotateToTarget: true,

			// Cyan (Based on C++ random(81) + 160 simplified here)
			red: 0.8,
			green: 0.8,
			blue: 1.0,

			sizeStart: 160.0,
			sizeEnd: 190.0,

			zOffsetStart: 5,
			arc: 3.0,
			retreat: 3.0,
			drainPattern: true
		}
	],
	379: [
		{
			//EF_TRANSBLUEBODY	   Energy Drain (2nd Part)
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					// Wait for drain particles from Part 1 (Soul Drain travel)
					if (tick < 500) {
						return false;
					}

					const time = tick - 500;

					// Enable halo (BL_DOUBLE_BODY actor)
					entity._enableHalo = true;

					// Part 2: Blue tint progression (200ms duration from official source)
					if (time < 200) {
						// Formula from official source: m_master->SetArgb(-1, 255-(m_stateCnt+50), 255-(m_stateCnt+50), 255)
						const val = (255 - (time + 50)) / 255;
						entity._flashColor[0] = val;
						entity._flashColor[1] = val;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return false;
					}

					// Part 3: Smooth fade back to normal (200ms) for premium feel
					if (time < 400) {
						const progress = (time - 200) / 200;
						const startVal = 5 / 255; // 255 - (200 + 50)
						const val = startVal + (1.0 - startVal) * progress;
						entity._flashColor[0] = val;
						entity._flashColor[1] = val;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return false;
					}

					// Final reset
					entity._enableHalo = false;
					entity._flashColor[0] = 1.0;
					entity._flashColor[1] = 1.0;
					entity._flashColor[2] = 1.0;
					entity._flashColor[3] = 1.0;
					entity.recalculateBlendingColor();
					return true;
				});
			}
		}
	],

	380: [
		{
			//EF_MAGICCRASHER	   Magic Crasher
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					if (tick < 1000) {
						if (tick > 500) {
							entity._flashColor[0] = Math.random();
							entity._flashColor[1] = Math.random();
							entity._flashColor[2] = Math.random();
							entity._flashColor[3] = 0.6;
							entity.recalculateBlendingColor();
						}
					} else {
						entity._flashColor[0] = 1;
						entity._flashColor[1] = 1;
						entity._flashColor[2] = 1;
						entity._flashColor[3] = 1;
						entity.recalculateBlendingColor();
						return true;
					}
				});
			},
			wav: 'effect/\xb8\xc5\xc1\xf7\x20\xc5\xa9\xb7\xa1\xbd\xac'
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 300;
				Camera.setQuake(start);
			}
		}
	],

	//381: [{}],	//EF_LIGHTSPHERE2	   Blue Starburst (Unknown use)

	382: [
		{
			//EF_LIGHTBLADE	(Nothing)
			type: 'SPR',
			file: '\xc7\xd1\xba\xb9\xc3\xb5\xbb\xe7',
			head: true,
			attachedEntity: true
		}
	],

	383: [
		{
			//EF_ENERGYDRAIN3 (Health Conversion)
			type: '3D',
			duration: 600,
			duplicate: 5,
			timeBetweenDupli: 0,
			absoluteSpriteName: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/particle1',
			playSprite: true,

			fromSrc: true,
			toSrc: true,
			rotateToTarget: true,

			// Green
			red: 0.7,
			green: 1.0,
			blue: 0.7,

			sizeStart: 140.0,
			sizeEnd: 170.0,

			zOffsetStart: 5,
			arc: 3.0,
			retreat: 3.0,
			drainPattern: true
		}
	],

	384: [
		{
			//EF_LINELINK2	   Soul Change (Sound Effect)
			wav: 'effect/\xbc\xd2\xbf\xef\x20\xc3\xbc\xc0\xce\xc1\xf6',
			attachedEntity: true
		}
	],

	//385: [{}],	//EF_LINKLIGHT	   Soul Change (Visual Effect)

	386: [
		{
			//EF_TRUESIGHT	   True Sight
			wav: 'effect/hunter_detecting',
			attachedEntity: true
		}
	],

	387: [
		{
			//EF_FALCONASSAULT	   Falcon Assault
			wav: 'effect/hunter_blitzbeat',
			attachedEntity: true
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 300;
				Camera.setQuake(start);
			}
		}
	],

	388: [
		{
			//EF_TRIPLEATTACK2	   Focused Arrow Strike (Sound Effect)
			wav: 'effect/\xbb\xfe\xc7\xc1\xbd\xb4\xc6\xc3',
			attachedEntity: true
		}
	],

	389: [
		{
			//windwalk	//EF_PORTAL4	Wind Walk
			wav: 'effect/\xc0\xa9\xb5\xe5\xbf\xf6\xc5\xa9', //Ŕ©µĺżöĹ©
			attachedEntity: true
		}
	],

	390: [
		{
			//EF_MELTDOWN	Shattering Strike
			type: 'STR',
			file: 'melt',
			attachedEntity: true
		}
	],

	391: [
		{
			//EF_CARTBOOST	Cart Boost
			type: 'STR',
			file: 'cart',
			wav: 'effect/ef_incagility',
			attachedEntity: true
		}
	],

	392: [
		{
			//EF_REJECTSWORD	Reject Sword
			type: 'STR',
			file: 'sword',
			wav: 'effect/kyrie_guard',
			attachedEntity: true
		}
	],

	393: [
		{
			//EF_TRIPLEATTACK3	   Arrow Vulcan
			wav: 'effect/\xbe\xd6\xb7\xce\xbf\xec\x20\xb9\xdf\xc4\xad'
		}
	],

	394: [
		{
			// Moonlit water mill/sheltering bliss	//EF_SPHEREWIND2	Sheltering Bliss
			type: 'FUNC',
			wav: 'effect/\xb4\xde\xba\xfb\xbc\xbc\xb7\xb9\xb3\xaa\xb5\xa5',
			attachedEntity: false,
			func: function (Params) {
				const BlueTile = FlatColorTile('salmon', { r: 0xff / 255, g: 0x8a / 255, b: 0xbb / 255, a: 0.6 });
				this.add(new BlueTile(Params.Inst.position, Params.Inst.startTick), Params);
			}
		}
	],

	//395: [{}],	//EF_LINELINK3	   Marionette Control (Sound Effect)
	//396: [{}],	//EF_PINKBODY	   Marionette Control (Visual Effect)

	397: [
		{
			//EF_LEVEL99_5	   Trancended 99 Aura (Middle)
			//effect\\ring_blue.tga
			//Level99
		}
	],

	398: [
		{
			//EF_LEVEL99_6	   Trancended 99 Aura (Bottom)
			//effect\\pikapika2.bmp
			//Level99_2
		}
	],
	399: [
		{
			//headcrush caster	//EF_BASH3D3	Head Crush
			wav: 'effect/\xc7\xec\xb5\xe5\x20\xc5\xa9\xb7\xaf\xbd\xac', //Çìµå Å©·¯½¬
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					// Enable halo (BL_DOUBLE_BODY actor)
					entity._enableHalo = true;

					if (tick < 200) {
						const val = (255 - (tick + 50)) / 255;
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = val;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return false;
					}

					if (tick < 500) {
						const progress = (tick - 200) / 300;
						const startVal = 5 / 255; // 255 - (200 + 50)
						const val = startVal + (1.0 - startVal) * progress;
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = val;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return false;
					}

					// Final reset
					entity._enableHalo = false;
					entity._flashColor[0] = 1.0;
					entity._flashColor[1] = 1.0;
					entity._flashColor[2] = 1.0;
					entity._flashColor[3] = 1.0;
					entity.recalculateBlendingColor();
					return true;
				});
			}
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			delayStart: 500,
			duplicate: 6,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 4.5,
			animation: 2,
			zIndex: 1.1,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			duplicate: 6,
			delayStart: 500,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 7.2,
			animation: 2,
			zIndex: 1.2,
			attachedEntity: true
		}
	],

	400: [
		{
			//joint beat caster	//EF_BASH3D4	Joint Beat
			wav: 'effect/\xba\xf1\xc6\xae\x20\xc1\xb6\xc0\xce\xc6\xae', // ºñÆ® Á¶ÀÎÆ®
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					// Official: Halo (Double Body) is active between state 20 and 50.
					// Scaling to ms (approx 200ms to 500ms).
					if (tick >= 150 && tick <= 500) {
						entity._enableHalo = true;
					} else {
						entity._enableHalo = false;
					}
					return tick > 600;
				});
			}
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			delayStart: 500,
			duplicate: 6,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 4.5,
			animation: 2,
			zIndex: 1.1,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			duplicate: 6,
			delayStart: 500,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 7.2,
			animation: 2,
			zIndex: 1.2,
			attachedEntity: true
		}
	],

	charge_attack: [
		{
			//charge attack (quest-skill)
			//same effect on target like 'Bash'
			attachedEntity: true
		}
	],

	//401: [{}],	//EF_NAPALMVALCAN	   Napalm Vulcan Sound
	402: [
		//EF_PORTAL5	   Dangerous Soul Collect
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					const time = tick;

					if (time < 200) {
						entity._flashColor[0] = 1.0; // Red
						entity._flashColor[1] = 1.0; // Green
						entity._flashColor[2] = 0.0; // Blue
						entity._flashColor[3] = 0.1; // Alpha (10%)
						entity.recalculateBlendingColor();
						return false;
					}

					// Part 2: Smooth fade back to normal
					if (time < 800) {
						const progress = (time - 200) / 600;
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = 0.0 + 1.0 * progress;
						entity._flashColor[3] = 0.1 + 0.9 * progress;
						entity.recalculateBlendingColor();
						return false;
					}

					// Final reset
					entity._flashColor[0] = 1.0;
					entity._flashColor[1] = 1.0;
					entity._flashColor[2] = 1.0;
					entity._flashColor[3] = 1.0;
					entity.recalculateBlendingColor();
					return true;
				});
			}
		}
	],

	403: [
		{
			//EF_MAGICCRASHER2	   Mind Breaker
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					if (tick < 1000) {
						entity._flashColor[0] = Math.random();
						entity._flashColor[1] = Math.random();
						entity._flashColor[2] = Math.random();
						entity.recalculateBlendingColor();
					} else {
						entity._flashColor[0] = 1;
						entity._flashColor[1] = 1;
						entity._flashColor[2] = 1;
						entity._flashColor[3] = 1;
						entity.recalculateBlendingColor();
						return true;
					}
				});
			},
			wav: 'effect/swordman_provoke'
		}
	],

	404: [
		{
			//EF_BOTTOM_SPIDER	Fiber Lock
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				this.add(new SpiderWeb(Params.Inst.position, Params.Inst.startTick), Params);
			}
		}
	],

	'405_ground': [
		{
			//wall of fog
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				const self = this;
				FogEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	406: [
		{
			//EF_SOULBURN	Soul Burn
			type: 'STR',
			file: '\xbc\xd2\xbf\xef\xb9\xf8',
			attachedEntity: true
		}
	],

	407: [
		{
			//EF_SOULCHANGE	Soul Change
			type: 'STR',
			file: '\xbb\xe7\xb6\xf7\xc8\xbf\xb0\xfa',
			attachedEntity: true
		}
	],

	//408: [{}],	//EF_BABY	   Mom, Dad, I love you! (Baby Skill)

	409: [
		{
			//EF_SOULBREAKER2	   Meteor Assault
			wav: 'effect/\xb8\xde\xc5\xd7\xbf\xc0\x20\xbe\xee\xbd\xe4\xc6\xae'
		},
		{
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.6,
			duration: 500,
			fadeOut: true,
			zIndex: 10,
			sizeStart: 100,
			sizeEnd: 200,
			posxStart: -1,
			posxEnd: -5,
			posyStart: 0,
			posyEnd: 0,
			rotateWithCamera: true,
			angle: 0
		},
		{
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.6,
			duration: 500,
			fadeOut: true,
			zIndex: 10,
			sizeStart: 100,
			sizeEnd: 200,
			posxStart: -0.7,
			posxEnd: -3.53,
			posyStart: -0.7,
			posyEnd: -3.53,
			rotateWithCamera: true,
			angle: -45
		},
		{
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.6,
			duration: 500,
			fadeOut: true,
			zIndex: 10,
			sizeStart: 100,
			sizeEnd: 200,
			posxStart: 0,
			posxEnd: 0,
			posyStart: -1,
			posyEnd: -5,
			rotateWithCamera: true,
			angle: -90
		},
		{
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.6,
			duration: 500,
			fadeOut: true,
			zIndex: 10,
			sizeStart: 100,
			sizeEnd: 200,
			posxStart: 0.7,
			posxEnd: 3.53,
			posyStart: -0.7,
			posyEnd: -3.53,
			rotateWithCamera: true,
			angle: -135
		},
		{
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.6,
			duration: 500,
			fadeOut: true,
			zIndex: 10,
			sizeStart: 100,
			sizeEnd: 200,
			posxStart: 1,
			posxEnd: 5,
			posyStart: 0,
			posyEnd: 0,
			rotateWithCamera: true,
			angle: -180
		},
		{
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.6,
			duration: 500,
			fadeOut: true,
			zIndex: 10,
			sizeStart: 100,
			sizeEnd: 200,
			posxStart: 0.7,
			posxEnd: 3.53,
			posyStart: 0.7,
			posyEnd: 3.53,
			rotateWithCamera: true,
			angle: -225
		},
		{
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.6,
			duration: 500,
			fadeOut: true,
			zIndex: 10,
			sizeStart: 100,
			sizeEnd: 200,
			posxStart: 0,
			posxEnd: 0,
			posyStart: 1,
			posyEnd: 5,
			rotateWithCamera: true,
			angle: -270
		},
		{
			type: '3D',
			file: 'effect/purpleslash.tga',
			alphaMax: 0.6,
			duration: 500,
			fadeOut: true,
			zIndex: 10,
			sizeStart: 100,
			sizeEnd: 200,
			posxStart: -0.7,
			posxEnd: -3.53,
			posyStart: 0.7,
			posyEnd: 3.53,
			rotateWithCamera: true,
			angle: -315
		}
	],

	//410: [{}],	//EF_RAINBOW	   Rainbow
	//411: [{}],	//EF_PEONG	   Leap
	//412: [{}],	//EF_TANJI2	   Like Throw Spirit Sphere
	//413: [{}],	//EF_PRESSEDBODY	   Axe Kick
	//414: [{}],	//EF_SPINEDBODY	   Round Kick
	//415: [{}],	//EF_KICKEDBODY	   Counter Kick
	//416: [{}],	//EF_AIRTEXTURE	   (Nothing)
	//417: [{}],	//EF_HITBODY	   Flash
	//418: [{}],	//EF_DOUBLEGUMGANG	   Warmth Lightning
	//419: [{}],	//EF_REFLECTBODY	   Kaite (Visual Effect)

	420: [
		{
			//EF_BABYBODY	Eswoo (Small) (Visual Effect)
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectSmallTransition(Params) {
				const entity = Params.Init.ownerEntity;
				const xSize = entity.xSize;
				const ySize = entity.ySize;

				entity.animations.add(function (tick) {
					entity.xSize = xSize + (2.5 - xSize) * (Math.min(tick, 300) / 300);
					entity.ySize = ySize + (2.5 - ySize) * (Math.min(tick, 300) / 300);

					return tick > 300;
				});
			}
		}
	],

	421: [
		{
			//EF_BABYBODY2	Eswoo (Alt. Small) (Visual Effect)
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectSmall(Params) {
				const entity = Params.Init.ownerEntity;
				entity.xSize = 2.5;
				entity.ySize = 2.5;
			}
		}
	],

	422: [
		{
			//EF_GIANTBODY	Eswoo (Normal) (Visual Effect)
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBigTransition(Params) {
				const entity = Params.Init.ownerEntity;
				const xSize = entity.xSize;
				const ySize = entity.ySize;

				entity.animations.add(function (tick) {
					entity.xSize = xSize + (7.5 - xSize) * (Math.min(tick, 300) / 300);
					entity.ySize = ySize + (7.5 - ySize) * (Math.min(tick, 300) / 300);

					return tick > 300;
				});
			}
		}
	],

	423: [
		{
			//EF_GIANTBODY2	Eswoo (Alt. Normal) (Visual Effect)
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBig(Params) {
				const entity = Params.Init.ownerEntity;
				entity.xSize = 7.5;
				entity.ySize = 7.5;
			}
		}
	],

	//424: [{}],	//EF_ASURABODY	   Spirit Link (Visual Effect)
	//425: [{}],	//EF_4WAYBODY	   Esma Hit (Visual Effect)

	426: [
		{
			//taekwon sprint collision effect	//EF_QUAKEBODY	Sprint Collision (Visual Effect)
			wav: 'effect/\xba\xb9\xc8\xa3\xb0\xdd', //şąČŁ°Ý
			attachedEntity: true
		}
	],

	//427: [{}],	//EF_ASURABODY_MONSTER	   (Nothing)
	//428: [{}],	//EF_HITLINE3	   (Nothing)
	//429: [{}],	//EF_HITLINE4	   Taekwon Kick Hit 1
	//430: [{}],	//EF_HITLINE5	   Taekwon Kick Hit 2
	//431: [{}],	//EF_HITLINE6	   Taekwon Kick Hit 3
	//432: [{}],	//EF_ELECTRIC	   Solar, Lunar and Stellar Perception (Visual Effect)
	//433: [{}],	//EF_ELECTRIC2	   Solar, Lunar and Stellar Opposition (Visual Effect)
	//434: [{}],	//EF_HITLINE7	   Taekwon Kick Hit 4
	//435: [{}],	//EF_STORMKICK	   Whirlwind Kick
	//436: [{}],	//EF_HALFSPHERE	   White Barrier (Unused)
	//437: [{}],	//EF_ATTACKENERGY	   White barrier 2 (Unused)
	//438: [{}],	//EF_ATTACKENERGY2	   Kaite Reflect Animation
	//439: [{}],	//EF_CHEMICAL3	   Flying Side Kick

	440: [
		{
			//EF_ASSUMPTIO2	Assumptio (Animation)
			type: 'STR',
			file: 'asum',
			wav: 'effect/\xbe\xc6\xbc\xfb\xc7\xc1\xc6\xbc\xbf\xc0',
			attachedEntity: true
		}
	],

	//441: [{}],	//EF_BLUECASTING	   Comfort Skills Cast Aura
	//442: [{}],	//EF_RUN	   Foot Prints caused by Sprint.
	//443: [{}],	//EF_STOPRUN	   (Nothing)

	444: [
		{
			//EF_STOPEFFECT	   Sprint Stop Animation
			wav: 'effect/t_\xc8\xbf\xb0\xfa\xc0\xbd1',
			attachedEntity: true
		}
	],

	445: [
		{
			//high jump caster	//EF_JUMPBODY	High Jump (Jump)
			wav: 'effect/t_\xc8\xb8\xc7\xc72',
			attachedEntity: true
		}
	],

	//446: [{}],	//EF_LANDBODY	   High Jump (Return Down)
	//447: [{}],	//EF_FOOT3	   Running Left Foot
	//448: [{}],	//EF_FOOT4	   Running Right Foot
	//449: [{}],	//EF_TAE_READY	   KA-Spell (1st Part)

	450: [
		{
			// Dark cross	//EF_GRANDCROSS2	Darkcross
		}
	],

	451: [
		{
			//EF_SOULSTRIKE2	   Dark Strike
			alphaMax: 1,
			attachedEntity: true,
			blue: 1,
			duration: 200,
			delayLate: 250,
			//duplicate: -1,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/pok3.tga',
			toSrc: true,
			green: 1,
			poszEnd: 1,
			poszSmooth: true,
			poszStartRand: 5,
			poszStartRandMiddle: 6,
			red: 1,
			size: 50,
			timeBetweenDupli: 150,
			type: '3D',
			wav: 'effect/ef_soulstrike',
			zIndex: 1
		},
		{
			type: '3D',
			duration: 250,
			duplicate: 5,
			timeBetweenDupli: 20,
			absoluteSpriteName: 'data/sprite/\xc0\xcc\xc6\xd1\xc6\xae/particle5',
			playSprite: true,
			toSrc: true,
			rotateToTarget: true,
			sizeStart: 100.0,
			sizeEnd: 500.0,
			zOffsetStart: 3,
			arc: 4.0,
			retreat: 4.0, //soulstrike2 have a little retreat upper
			soulStrikePattern: true
		}
	],

	452: [
		{
			//EF_YUFITELHIT2	Alternative Jupitel Thunder (Hit)
			type: '3D',
			file: 'effect/pokjuk_d.bmp',
			duration: 100, // Aprox. 10 frames on C++
			sizeStart: 0,
			sizeEnd: 25, // widthSpeed 2.5f (Growing up)
			blendMode: 2, // RF_EMISSIVE (Aprox. Additive)
			rotateToTarget: true,
			fadeOut: true,
			overlay: true,
			attachedEntity: true
		},
		{
			type: '3D',
			fileList: [
				'effect/twirl_soft.bmp',
				'effect/thunder_ball_b.bmp',
				'effect/twirl_soft.bmp',
				'effect/thunder_ball_c.bmp',
				'effect/twirl_soft.bmp'
			],
			frameDelay: 10,
			duration: 300, // duration 300 on C++ is equal to 3000ms on ROB but it's the total duration of all hits, ROB parse each
			size: 75, // widthSize 7.5f * 10
			blendMode: 2, // RF_EMISSIVE (Aprox. Additive)
			overlay: true,
			attachedEntity: true
		}
	],

	453: [
		{
			//EF_NPC_STOP	   Paralized
			type: 'SPR',
			file: '\xbd\xba\xc5\xe9',
			attachedEntity: true
		}
	],

	454: [
		{
			//EF_DARKCASTING	   Like Blind
			alphaMax: 0.8,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			blue: 1,
			bottomSize: 1,
			//duration: 900,
			fade: true,
			green: 1,
			height: 4,
			red: 1,
			rotate: true,
			textureName: 'ring_black',
			topSize: 5,
			type: 'CYLINDER',
			wav: 'effect/ef_beginspell'
		}
	],

	//455: [{}],	//EF_GUMGANGNPC	   Another Warmth Lightning

	456: [
		{
			//EF_AGIUP	   Power Up
			wav: 'effect/\x6d\x6f\x6e\x5f\xc6\xf8\xb1\xe2',
			attachedEntity: true
		}
	],

	457: [
		{
			//flying kick on target	//EF_JUMPKICK	Flying Side Kick (2nd Part)
			wav: 'effect/\x74\x5f\xb3\xaf\xb6\xf3\xc2\xf7\xb1\xe2', //t_łŻ¶óÂ÷±â
			attachedEntity: true
		}
	],

	//458: [{}],	//EF_QUAKEBODY2	   Running/Sprint (running into a wall)
	//459: [{}],	//EF_STORMKICK1	   Brown tornado that spins sprite (unused)
	//460: [{}],	//EF_STORMKICK2	   Green tornado (unused)
	//461: [{}],	//EF_STORMKICK3	   Blue tornado (unused)
	//462: [{}],	//EF_STORMKICK4	   Kaupe Dodge Effect
	//463: [{}],	//EF_STORMKICK5	   Kaupe Dodge Effect
	//464: [{}],	//EF_STORMKICK6	   White tornado (unused)
	//465: [{}],	//EF_STORMKICK7	   Purple tornado (unused)
	//466: [{}],	//EF_SPINEDBODY2	   Another Round Kick

	467: [
		{
			//EF_BEGINASURA1	   Warm/Mild Wind (Earth)
			wav: 'effect/t_\xb9\xd9\xb6\xf7\xb9\xe6\xc3\xe2'
		},
		{
			alphaMax: 1,
			blendMode: 2,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 300,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon1.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon1.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 350,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon1.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 400,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon1.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 450,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon1.tga'
		}
	],

	468: [
		{
			//EF_BEGINASURA2	   Warm/Mild Wind (Wind)
			wav: 'effect/t_\xb9\xd9\xb6\xf7\xb9\xe6\xc3\xe2'
		},
		{
			alphaMax: 1,
			blendMode: 2,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 300,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon2.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon2.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 350,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon2.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 400,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon2.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 450,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon2.tga'
		}
	],

	469: [
		{
			//EF_BEGINASURA3	   Warm/Mild Wind (Water)
			wav: 'effect/t_\xb9\xd9\xb6\xf7\xb9\xe6\xc3\xe2'
		},
		{
			alphaMax: 1,
			blendMode: 2,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 300,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon3.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon3.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 350,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon3.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 400,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon3.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 450,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon3.tga'
		}
	],

	470: [
		{
			//EF_BEGINASURA4	   Warm/Mild Wind (Fire)
			wav: 'effect/t_\xb9\xd9\xb6\xf7\xb9\xe6\xc3\xe2'
		},
		{
			alphaMax: 1,
			blendMode: 2,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 300,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon4.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon4.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 350,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon4.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 400,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon4.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 450,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon4.tga'
		}
	],

	471: [
		{
			//EF_BEGINASURA5	   Warm/Mild Wind (Ghost)
			wav: 'effect/t_\xb9\xd9\xb6\xf7\xb9\xe6\xc3\xe2'
		},
		{
			alphaMax: 1,
			blendMode: 2,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 300,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon7.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon7.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 350,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon7.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 400,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon7.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 450,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon7.tga'
		}
	],

	472: [
		{
			//EF_BEGINASURA6	   Warm/Mild Wind (Shadow)
			wav: 'effect/t_\xb9\xd9\xb6\xf7\xb9\xe6\xc3\xe2'
		},
		{
			alphaMax: 1,
			blendMode: 2,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 300,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon5.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon5.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 350,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon5.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 400,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon5.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 450,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon5.tga'
		}
	],

	473: [
		{
			//EF_BEGINASURA7	   Warm/Mild Wind (Holy)
			wav: 'effect/t_\xb9\xd9\xb6\xf7\xb9\xe6\xc3\xe2'
		},
		{
			alphaMax: 1,
			blendMode: 2,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 300,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon6.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon6.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 350,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon6.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 400,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon6.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 450,
			type: '3D',
			zIndex: 10,
			file: 'effect/hanmoon6.tga'
		}
	],

	//474: [{}],	//EF_AURABLADE2	   (Nothing)
	//475: [{}],	//EF_DEVIL1	   Demon of The Sun Moon And Stars (Level 1)
	//476: [{}],	//EF_DEVIL2	   Demon of The Sun Moon And Stars (Level 2)
	//477: [{}],	//EF_DEVIL3	   Demon of The Sun Moon And Stars (Level 3)
	//478: [{}],	//EF_DEVIL4	   Demon of The Sun Moon And Stars (Level 4)
	//479: [{}],	//EF_DEVIL5	   Demon of The Sun Moon And Stars (Level 5)
	//480: [{}],	//EF_DEVIL6	   Demon of The Sun Moon And Stars (Level 6)
	//481: [{}],	//EF_DEVIL7	   Demon of The Sun Moon And Stars (Level 7)
	//482: [{}],	//EF_DEVIL8	   Demon of The Sun Moon And Stars (Level 8)
	//483: [{}],	//EF_DEVIL9	   Demon of The Sun Moon And Stars (Level 9)
	//484: [{}],	//EF_DEVIL10	   Demon of The Sun Moon And Stars (Level 10)
	//485: [{}],	//EF_DOUBLEGUMGANG2	   Mental Strength Lightning but White
	//486: [{}],	//EF_DOUBLEGUMGANG3	   Mental Strength Lightning
	//487: [{}],	//EF_BLACKDEVIL	   Demon of The Sun Moon And Stars Ground Effect
	//488: [{}],	//EF_FLOWERCAST	   Comfort Skills
	//489: [{}],	//EF_FLOWERCAST2	   (Nothing)
	//490: [{}],	//EF_FLOWERCAST3	   (Nothing)

	491: [
		{
			//EF_MOCHI	Element Potions
			type: 'STR',
			file: '\xc2\xfd\xbd\xd2\xb6\xb1',
			attachedEntity: true
		}
	],

	492: [
		{
			//EF_LAMADAN	Cooking Foods
			type: 'STR',
			file: 'ramadan',
			attachedEntity: true
		}
	],

	493: [
		{
			// edp	//EF_EDP	Enchant Deadly Poison
			wav: 'effect/assasin_cloaking',
			//blinking
			attachedEntity: true
		}
	],

	//494: [{}],	//EF_SHIELDBOOMERANG2	   Throwing Tomahawk
	//495: [{}],	//EF_RG_COIN2	   Full Strip Sound

	'496_beforecast': [
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				this.add(new MagicRing(entity, 2.45, 0.8, 2.8, 'ring_jadu', Params.Inst.startTick + 10000), Params);
			}
		}
	],

	496: [
		{
			//EF_GUARD2	  Preserve
			wav: 'effect/black_maximize_power_sword_bic'
		}
	],

	//497: [{}],	//EF_SLIM	   Twilight Alchemy 1
	//498: [{}],	//EF_SLIM2	   Twilight Alchemy 2
	//499: [{}],	//EF_SLIM3	   Twilight Alchemy 3
	//500: [{}],	//EF_CHEMICALBODY	   Player Become Blue with Blue Aura

	501: [
		{
			//EF_CASTSPIN	   Chase Walk Animation
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				const duration = 500;
				const count = 50;
				const delay = duration / count;

				for (let i = 0; i < count; i++) {
					let delta = 1;

					if (i <= 15) {
						delta = 12 * (8 / 360);
					} else if (i <= 30) {
						delta = 18 * (8 / 360);
					} else if (i <= 40) {
						delta = 24 * (8 / 360);
					} else {
						delta = 48 * (8 / 360);
					}

					Events.setTimeout(function () {
						entity.direction = Math.floor(entity.direction + delta) % 8;
					}, delay * i);
				}
			}
		}
	],

	//502: [{}],	//EF_PIERCEBODY	   Player Become Yellow with Yellow Aura
	//503: [{}],	//EF_SOULLINK	   Soul Link Word

	504: [
		{
			//EF_CHOOKGI2	   (Nothing) - Used for spirit spheres (monk classes)
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const spiritNum = Params.Init.spiritNum || 0;
				const Spheres = new SpiritSphere(Params.Init.ownerEntity, spiritNum, false);
				this.add(Spheres, Params);
			}
		}
	],

	//505: [{}],	//EF_MEMORIZE	   Memorize
	//506: [{}],	//EF_SOULLIGHT	   (Nothing)

	507: [
		{
			//Authoritative Badge	//EF_MAPAE	Authoritative Badge
			type: 'STR',
			file: 'mapae',
			wav: 'effect/mapae',
			attachedEntity: true
		}
	],

	508: [
		{
			//EF_ITEMPOKJUK	Fire Cracker
			type: 'STR',
			file: 'itempokjuk',
			wav: 'effect/itempokjuk',
			attachedEntity: true
		}
	],

	509: [
		{
			//EF_05VAL	Valentine Day Hearth (Wings)
			type: 'SPR',
			file: '05vallentine',
			attachedEntity: true
		}
	],

	510: [
		{
			//EF_BEGINASURA11	   Champion Asura Strike
			type: 'CYLINDER',
			textureName: 'ring_white',
			fade: true,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 1,
			topSize: 4.5,
			height: -4,
			duration: 800
		},
		{
			type: 'CYLINDER',
			textureName: 'ring_white',
			fade: true,
			animation: 2,
			attachedEntity: true,
			blendMode: 2,
			bottomSize: 1,
			topSize: 2.5,
			height: -4,
			duration: 800
		},
		// asura11 — phase 1
		{
			type: '3D',
			file: 'effect/asura11.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 300,
			sizeEnd: 150,
			sizeSmooth: true,
			posx: -8.0,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 0
		},
		// asura11 — phase 2
		{
			type: '3D',
			file: 'effect/asura11.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 150,
			sizeEnd: 250,
			posx: -8.0,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1200
		},
		// asura12 — phase 1
		{
			type: '3D',
			file: 'effect/asura12.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 300,
			sizeEnd: 150,
			sizeSmooth: true,
			posx: -4.8,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 100
		},
		// asura12 — phase 2
		{
			type: '3D',
			file: 'effect/asura12.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 150,
			sizeEnd: 250,
			posx: -4.8,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1300
		},
		// asura13 — phase 1
		{
			type: '3D',
			file: 'effect/asura13.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 300,
			sizeEnd: 150,
			sizeSmooth: true,
			posx: -1.6,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 200
		},
		// asura13 — phase 2
		{
			type: '3D',
			file: 'effect/asura13.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 150,
			sizeEnd: 250,
			posx: -1.6,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1400
		},
		// asura14 — phase 1
		{
			type: '3D',
			file: 'effect/asura14.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 300,
			sizeEnd: 150,
			sizeSmooth: true,
			posx: 1.6,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 300
		},
		// asura14 — phase 2
		{
			type: '3D',
			file: 'effect/asura14.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 150,
			sizeEnd: 250,
			posx: 1.6,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1500
		},
		// asura15 — phase 1
		{
			type: '3D',
			file: 'effect/asura15.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 300,
			sizeEnd: 150,
			sizeSmooth: true,
			posx: 4.8,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 400
		},
		// asura15 — phase 2
		{
			type: '3D',
			file: 'effect/asura15.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 150,
			sizeEnd: 250,
			posx: 4.8,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1600
		},
		// asura16 — phase 1
		{
			type: '3D',
			file: 'effect/asura16.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 1200,
			duplicate: 3,
			timeBetweenDupli: 150,
			alphaMaxDelta: -0.25,
			fadeIn: true,
			sizeStart: 300,
			sizeEnd: 150,
			sizeSmooth: true,
			posx: 8.0,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 500
		},
		// asura16 — phase 2
		{
			type: '3D',
			file: 'effect/asura16.tga',
			alphaMax: 1,
			attachedEntity: true,
			duration: 400,
			fadeOut: true,
			sizeStart: 150,
			sizeEnd: 250,
			posx: 8.0,
			posy: 0,
			posz: 4,
			overlay: true,
			red: 0.1,
			green: 0.1,
			blue: 0.1,
			delayStart: 1700
		}
	],
	//511: [{}],	//EF_NIGHT	   (Nothing)

	512: [
		{
			//EF_CHEMICAL2DASH	   Chain Crush Combo
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 132;
				Camera.setQuake(start);
			}
		}
	],

	513: [
		{
			//EF_GROUNDSAMPLE	   Area Cast
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				this.add(
					new MagicTarget(
						Params.Init.skillId,
						Params.Init.position[0],
						Params.Init.position[1],
						Params.Inst.endTick,
						Params.Init.otherEntity
					),
					Params
				);
			}
		}
	],

	//514: [{}],	//EF_GI_EXPLOSION	   Really Big Circle

	515: [
		{
			//EF_CLOUD4	   Airplane Cloud
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				CloudWeatherEffect.startOrRestart(Params);
			}
		}
	],

	516: [
		{
			//EF_CLOUD5	   Thanathos Cloud
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				CloudWeatherEffect.startOrRestart(Params);
			}
		}
	],

	517: [
		{
			//EF_BOTTOM_HERMODE	   (Nothing)
		}
	],

	'517_music': [
		{
			wav: 'effect/\xc7\xec\xb8\xa3\xb8\xf0\xb5\xe5\xc0\xc7\x20\xc1\xf6\xc6\xce\xc0\xcc',
			attachedEntity: true
		}
	],

	//518: [{}],	//EF_CARTTER	   Cart Termination

	519: [
		{
			//speed potion	//EF_ITEMFAST	Speed Down Potion
			type: 'SPR',
			file: 'fast',
			wav: 'effect/fast',
			attachedEntity: true
		}
	],

	//520: [{}],	//EF_SHIELDBOOMERANG3	   Shield Bumerang
	//521: [{}],	//EF_DOUBLECASTBODY	   Player Become Red with Red Aura

	'522_ground': [
		{
			// Gravitation field
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const self = this;
				GravityEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				const duration = 200;
				Camera.setQuake(start, duration);
			}
		}
	],

	523: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot01.tga'
		}
	], //EF_TAROTCARD1	   Tarot Card of Fate (The Fool)
	524: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot02.tga'
		}
	], //EF_TAROTCARD2	   Tarot Card of Fate (The Magician)
	525: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot03.tga'
		}
	], //EF_TAROTCARD3	   Tarot Card of Fate (The High Priestess)
	526: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot04.tga'
		}
	], //EF_TAROTCARD4	   Tarot Card of Fate (The Chariot)
	527: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot05.tga'
		}
	], //EF_TAROTCARD5	   Tarot Card of Fate (Strength)
	528: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot06.tga'
		}
	], //EF_TAROTCARD6	   Tarot Card of Fate (The Lovers)
	529: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot07.tga'
		}
	], //EF_TAROTCARD7	   Tarot Card of Fate (The Wheel of Fortune)
	530: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot08.tga'
		}
	], //EF_TAROTCARD8	   Tarot Card of Fate (The Hanged Man)
	531: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot09.tga'
		}
	], //EF_TAROTCARD9	   Tarot Card of Fate (Death)
	532: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot10.tga'
		}
	], //EF_TAROTCARD10	   Tarot Card of Fate (Temperance)
	533: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot11.tga'
		}
	], //EF_TAROTCARD11	   Tarot Card of Fate (The Devil)
	534: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot12.tga'
		}
	], //EF_TAROTCARD12	   Tarot Card of Fate (The Tower)
	535: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot13.tga'
		}
	], //EF_TAROTCARD13	   Tarot Card of Fate (The Star)
	536: [
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 3000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 70,
			sizeSmooth: true,
			sizeStart: 100,
			type: '3D',
			wav: 'effect/priest_slowpoison',
			zIndex: 10,
			file: 'effect/tarot14.tga'
		}
	], //EF_TAROTCARD14	   Tarot Card of Fate (The Sun)

	537: [
		{
			//EF_ACIDDEMON	   Acid Demonstration
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick + 200;
				Camera.setQuake(start);
			}
		}
	],

	//538: [{}],	//EF_GREENBODY	   Player Become Green with Green Aura
	//539: [{}],	//EF_THROWITEM4	   Throw Random Bottle
	//540: [{}],	//EF_BABYBODY_BACK	   Instant Small->Normal
	//541: [{}],	//EF_THROWITEM5	   (Nothing)
	//542: [{}],	//EF_BLUEBODY	   KA-Spell (1st Part)

	543: [
		{
			//EF_HATED	   Kahii
			wav: 'effect/t_\xba\xb8\xc1\xb6\xb8\xb6\xb9\xfd'
		}
	],

	//544: [{}],	//EF_REDLIGHTBODY	   Warmth Red Sprite
	//545: [{}],	//EF_RO2YEAR	   Sound And... PUFF Client Crash :P
	//546: [{}],	//EF_SMA_READY	   Kaupe

	547: [
		{
			//EF_STIN	   Estin
			wav: 'effect/t_\xbf\xa1\xb3\xca\xc1\xf6\xb9\xe6\xc3\xe2'
		}
	],

	//548: [{}],	//EF_RED_HIT	   Instant Red Sprite
	//549: [{}],	//EF_BLUE_HIT	   Instant Blue Sprite
	//550: [{}],	//EF_QUAKEBODY3	   Another Effect like Running Hit
	//551: [{}],	//EF_SMA	   Effect Like Estun but with Circle
	//552: [{}],	//EF_SMA2	   (Nothing)

	553: [
		{
			//EF_STIN2	   Esma
			duplicate: 5,
			timeBetweenDupli: 200,
			wav: 'effect/t_\xb3\xaf\xb6\xf3\xc2\xf7\xb1\xe2'
		}
	],

	//554: [{}],	//EF_HITTEXTURE	   Large White Cloud

	555: [
		{
			//EF_STIN3	   Estun
			wav: 'effect/t_\xbf\xa1\xb3\xca\xc1\xf6\xb9\xe6\xc3\xe2'
		}
	],

	//556: [{}],	//EF_SMA3	   (Nothing)
	//557: [{}],	//EF_BLUEFALL	   Juperos Energy Waterfall (Horizontal)
	//558: [{}],	//EF_BLUEFALL_90	   Juperos Energy Waterfall (Vertical)
	//559: [{}],	//EF_FASTBLUEFALL	   Juperos Energy Waterfall Fast (Horizontal)
	//560: [{}],	//EF_FASTBLUEFALL_90	   Juperos Energy Waterfall Fast (Vertical)
	//561: [{}],	//EF_BIG_PORTAL	   Juperos Warp
	//562: [{}],	//EF_BIG_PORTAL2	   Juperos Warp

	563: [
		{
			//EF_SCREEN_QUAKE	   Earthquake Effect (Juperos Elevator)
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				const duration = 200;
				Camera.setQuake(start, duration);
			}
		}
	],

	//564: [{}],	//EF_HOMUNCASTING	   Wedding Cast

	565: [
		{
			//EF_HFLIMOON1	Filir Moonlight Lvl 1
			type: 'STR',
			file: 'moonlight_1',
			wav: 'effect/h_moonlight_1',
			attachedEntity: true
		}
	],

	566: [
		{
			//EF_HFLIMOON2	Filir Moonlight Lvl 2
			type: 'STR',
			file: 'moonlight_2',
			wav: 'effect/h_moonlight_2',
			attachedEntity: true
		}
	],

	567: [
		{
			//EF_HFLIMOON3	Filir Moonlight Lvl 3
			type: 'STR',
			file: 'moonlight_3',
			wav: 'effect/h_moonlight_3',
			attachedEntity: true
		}
	],

	568: [
		{
			//homun?	//EF_HO_UP	Another Job Level Up
			type: 'STR',
			file: 'h_levelup',
			attachedEntity: true
		}
	],

	569: [
		{
			//EF_HAMIDEFENCE	Amistr Bulwark
			type: 'STR',
			file: 'defense',
			attachedEntity: true
		}
	],

	570: [
		{
			//EF_HAMICASTLE	Amistr Castling
			type: 'SPR',
			file: '\xc4\xb3\xbd\xbd\xb8\xb5',
			attachedEntity: true
		}
	],

	571: [
		{
			//EF_HAMIBLOOD	Amistr Bloodlust
			type: 'SPR',
			file: '\xba\xed\xb7\xaf\xb5\xe5\xb7\xaf\xbd\xba\xc6\xae',
			attachedEntity: true
		}
	],

	//572: [{}],	//EF_HATED2	   Warmth Soul
	//573: [{}],	//EF_TWILIGHT1	   Twilight Alchemy 1
	//574: [{}],	//EF_TWILIGHT2	   Twilight Alchemy 2
	//575: [{}],	//EF_TWILIGHT3	   Twilight Alchemy 3

	576: [
		{
			//EF_ITEM_THUNDER	Box Effect (Thunder)
			type: 'SPR',
			file: 'item_thunder',
			attachedEntity: true
		}
	],

	577: [
		{
			//EF_ITEM_CLOUD	Box Effect (Cloud)
			type: 'SPR',
			file: 'item_cloud',
			attachedEntity: true
		}
	],

	578: [
		{
			//EF_ITEM_CURSE	Box Effect (Curse)
			type: 'SPR',
			file: 'item_curse',
			attachedEntity: true
		}
	],

	579: [
		{
			//EF_ITEM_ZZZ	Box Effect (Sleep)
			type: 'SPR',
			file: 'item_zzz',
			wav: '_snore',
			attachedEntity: true
		}
	],

	580: [
		{
			//EF_ITEM_RAIN	Box Effect (Rain)
			type: 'SPR',
			file: 'item_rain',
			attachedEntity: true
		}
	],

	//581: [{}],	//EF_ITEM_LIGHT	   Box Effect (Sunlight)
	//582: [{}],	//EF_ANGEL3	   Another Super Novice/Taekwon Angel

	583: [
		{
			//EF_M01	Warmth Hit
			type: 'SPR',
			file: 'm_ef01',
			attachedEntity: true
		}
	],

	584: [
		{
			//EF_M02	Full Buster
			type: 'SPR',
			file: 'm_ef02',
			attachedEntity: true,
			direction: true
		}
	],

	585: [
		{
			//EF_M03	5 Medium Size Explosion
			type: 'SPR',
			file: 'm_ef03',
			attachedEntity: true
		}
	],

	586: [
		{
			//EF_M04	Somatology Lab Mobs Aura
			type: 'SPR',
			file: 'm_ef04',
			attachedEntity: true
		}
	],

	587: [
		{
			//EF_M05	Big Purple Flame
			type: 'SPR',
			file: 'm_ef05',
			wav: 'dragon_breath',
			attachedEntity: true
		}
	],

	588: [
		{
			//EF_M06	Little Red Flame
			type: 'SPR',
			file: 'm_ef06',
			attachedEntity: true
		}
	],

	589: [
		{
			//EF_M07	Eswoo
			type: 'SPR',
			file: 'm_ef07',
			wav: 'effect/t_\xba\xb8\xc1\xb6\xb8\xb6\xb9\xfd',
			attachedEntity: true
		}
	],

	590: [
		{
			//EF_KAIZEL	   Running Stop //Conflicting info, will use it as Kaizel
			wav: 'effect/priest_resurrection'
		}
	],

	//591: [{}],	//EF_KAAHI	   (Nothing)

	592: [
		{
			//EF_CLOUD6
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				CloudWeatherEffect.startOrRestart(Params);
			}
		}
	],

	593: [
		{
			//EF_FOOD01	Food Effect (STR)
			type: 'STR',
			file: 'food_str',
			attachedEntity: true
		}
	],

	594: [
		{
			//EF_FOOD02	Food Effect (INT)
			type: 'STR',
			file: 'food_int',
			attachedEntity: true
		}
	],

	595: [
		{
			//EF_FOOD03	Food Effect (VIT)
			type: 'STR',
			file: 'food_vit',
			attachedEntity: true
		}
	],

	596: [
		{
			//EF_FOOD04	Food Effect (AGI)
			type: 'STR',
			file: 'food_agi',
			attachedEntity: true
		}
	],

	597: [
		{
			//EF_FOOD05	Food Effect (DEX)
			type: 'STR',
			file: 'food_dex',
			attachedEntity: true
		}
	],

	598: [
		{
			//EF_FOOD06	Food Effect (LUK)
			type: 'STR',
			file: 'food_luk',
			attachedEntity: true
		}
	],

	//599: [{}],	//EF_SHRINK	   Cast Time Sound and Flashing Animation on Player

	600: [
		{
			//EF_THROWITEM6	   Throw Venom Knife
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 200,
			fadeIn: true,
			fadeOut: true,
			file: '\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba/item/\xba\xa3\xb3\xd1\xb3\xaa\xc0\xcc\xc7\xc1.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 1,
			size: 30,
			zOffset: 1,
			zIndex: 1
		}
	],

	//601: [{}],	//EF_SIGHT2	   Sight Blaster
	//602: [{}],	//EF_QUAKEBODY4	   Close Confine (Grab Effect)

	603: [
		{
			//EF_FIREHIT2	Spinning fire ball (like 50, but smaller)
			type: 'STR',
			file: 'firehit%d',
			rand: [1, 3],
			attachedEntity: true
		}
	],

	604: [
		{
			//EF_NPC_STOP2	Close Confine (Ground Effect)
			type: 'SPR',
			file: 'cconfine',
			attachedEntity: true,
			wav: 'effect/ef_hit6',
			stopAtEnd: true
		}
	],

	//605: [{}],	//EF_NPC_STOP2_DEL	   (Nothing)

	606: [
		{
			//EF_FVOICE	   Pang Voice (Visual Effect)
			type: 'SPR',
			file: 'fvoice',
			wav: 'amon_ra_die01'
		}
	],

	607: [
		{
			//EF_WINK	   Wink of Charm (Visual Effect)
			type: 'SPR',
			file: 'wink'
		}
	],

	608: [
		{
			//EF_COOKING_OK	Cooking Success
			type: 'STR',
			file: 'cook_suc',
			wav: '_heal_effect',
			attachedEntity: true
		}
	],

	609: [
		{
			//EF_COOKING_FAIL	Cooking Failed
			type: 'STR',
			file: 'cook_fail',
			wav: 'caramel_die',
			attachedEntity: true
		}
	],

	//610: [{ file: 'effect/success.bmp' }],	//EF_TEMP_OK	   Success
	//611: [{ file: 'effect/failed.bmp' }],	//EF_TEMP_FAIL	   Failed

	612: [
		{
			//EF_HAPGYEOK	Korean Words and /no1 Emoticon
			type: 'SPR',
			file: '\xc7\xd5\xb0\xdd\x5f',
			attachedEntity: true
		},
		{
			type: 'STR',
			file: 'itempokjuk',
			wav: 'effect/itempokjuk',
			attachedEntity: true
		}
	],

	613: [
		{
			//EF_THROWITEM7	   Throw Shuriken
			wav: 'effect/\xb4\xd1\xc0\xda\x5f\xb4\xf8\xc1\xf6\xb1\xe2'
		},
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 200,
			fadeIn: true,
			fadeOut: true,
			file: '\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba/item/\xbc\xf6\xb8\xae\xb0\xcb.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 1,
			size: 30,
			zOffset: 1,
			zIndex: 1
		}
	],

	614: [
		{
			//EF_THROWITEM8	   Throw Kunai
			wav: 'effect/\xb4\xd1\xc0\xda\x5f\xb4\xf8\xc1\xf6\xb1\xe2'
		},
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 200,
			fadeIn: true,
			fadeOut: true,
			file: '\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba/item/\xc4\xed\xb3\xaa\xc0\xcc\x5f\xb5\xb6.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 1,
			size: 30,
			zOffset: 1,
			zIndex: 1
		}
	],

	615: [
		{
			//EF_THROWITEM9	   Throw Fumma Shuriken
			wav: 'effect/\xb4\xd1\xc0\xda\x5f\xb4\xf8\xc1\xf6\xb1\xe2'
		},
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 200,
			fadeIn: true,
			fadeOut: true,
			file: '\xc0\xaf\xc0\xfa\xc0\xce\xc5\xcd\xc6\xe4\xc0\xcc\xbd\xba/item/\xc7\xb3\xb8\xb6\x5f\xb3\xfa\xbf\xec.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 1,
			size: 30,
			zOffset: 1,
			zIndex: 1
		}
	],

	616: [
		{
			//EF_THROWITEM10	   Throw Money
			wav: 'effect/\xb4\xd1\xc0\xda\x5f\xb4\xf8\xc1\xf6\xb1\xe2'
		},
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 200,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/coin_a.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 1,
			size: 20,
			zOffset: 1,
			zIndex: 1
		}
	],

	//617: [{}],	//EF_BUNSINJYUTSU	   Illusionary Shadow

	618: [
		{
			//EF_KOUENKA	   Crimson Fire Bolossom
			type: 'STR',
			file: 'firehit',
			wav: 'effect/ef_firearrow%d',
			rand: [1, 3],
			attachedEntity: true
		}
	],

	619: [
		{
			//EF_HYOUSENSOU	   Lightning Spear Of Ice
			type: 'STR',
			file: 'freeze',
			wav: 'effect/ef_icearrow%d',
			rand: [1, 3],
			attachedEntity: true
		}
	],

	//620: [{}],	//EF_BOTTOM_SUITON	   Water Escape Technique

	621: [
		{
			//EF_STIN4	   Wind Blade
			wav: 'effect/\xc7\xb3\xc0\xce'
		}
	],

	622: [
		{
			//EF_THUNDERSTORM2	   Lightning Crash
			type: 'STR',
			file: 'setsudan',
			wav: 'effect/ef_thunderstorm',
			attachedEntity: true
		}
	],

	//623: [{}],	//EF_CHEMICAL4	   Piercing Shot
	//624: [{}],	//EF_STIN5	   Kamaitachi
	//625: [{}],	//EF_MADNESS_BLUE	   Madness Canceller
	//626: [{}],	//EF_MADNESS_RED	   Adjustment

	627: [
		{
			//EF_RG_COIN3	   Disarm (Sound Effect)
			wav: 'effect/\xb5\xf0\xbd\xba\xbe\xcf'
		}
	],

	628: [
		{
			//dust	//EF_BASH3D5	   Dust
			wav: 'effect/bash3d5', // incorrect wav name
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			duplicate: 6,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 4.5,
			animation: 2,
			zIndex: 1.1,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			duplicate: 6,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 7.2,
			animation: 2,
			zIndex: 1.2,
			attachedEntity: true
		}
	],

	629: [
		{
			//EF_CHOOKGI3	   (Nothing) - Used for spirit spheres (gunslinger classes)
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const spiritNum = Params.Init.spiritNum || 0;
				const Spheres = new SpiritSphere(Params.Init.ownerEntity, spiritNum, true);
				this.add(Spheres, Params);
			}
		}
	],

	630: [
		{
			//EF_KIRIKAGE	Shadow Slash
			type: 'SPR',
			file: '\xb1\xd7\xb8\xb2\xc0\xda\xba\xa3\xb1\xe2',
			wav: 'effect/\xb1\xd7\xb8\xb2\xc0\xda\xba\xa3\xb1\xe2',
			attachedEntity: true
		}
	],

	631: [
		{
			//reverse tatami map unit	//EF_TATAMI	Reverse Tatami Map Unit
			type: 'SPR',
			file: '\xb4\xd9\xb4\xd9\xb9\xcc\x20\xb5\xda\xc1\xfd\xb1\xe2',
			wav: 'effect/\xb4\xd9\xb4\xd9\xb9\xcc\xb5\xda\xc1\xfd\xb1\xe2',
			attachedEntity: true
		}
	],

	632: [
		{
			//EF_KASUMIKIRI	Mist Slash
			type: 'SPR',
			file: '\xbe\xc8\xb0\xb3\xba\xa3\xb1\xe2',
			wav: 'effect/\xbe\xc8\xb0\xb3\xba\xa3\xb1\xe2',
			attachedEntity: true
		}
	],

	633: [
		{
			//EF_ISSEN	Final Strike
			type: 'SPR',
			file: '\xc0\xcf\xbc\xb6',
			wav: 'effect/\xc0\xcf\xbc\xb6',
			attachedEntity: true
		}
	],

	634: [
		{
			//EF_KAEN	Crimson Fire Formation
			type: 'SPR',
			file: '\xc8\xad\xbf\xb0\xc1\xf8',
			repeat: true,
			attachedEntity: true
		},
		{
			wav: 'effect/\xc8\xad\xbf\xb0\xc1\xf8'
		}
	],

	635: [
		{
			//EF_BAKU	Dragon Fire Formation
			type: 'STR',
			file: 'fire dragon',
			wav: 'effect/\xc6\xf8\xbf\xb0\xb7\xe6',
			attachedEntity: false
		}
	],

	636: [
		{
			//EF_HYOUSYOURAKU	Falling Ice Pillar
			type: 'STR',
			file: 'icy',
			wav: 'effect/\xba\xf9\xc1\xa4\xb6\xf4',
			attachedEntity: false
		}
	],

	637: [
		{
			//EF_DESPERADO	Desperado
			type: 'SPR',
			file: '\xb5\xa5\xbd\xba\xc6\xe4\xb6\xf3\xb5\xb5',
			wav: 'effect/\xb5\xa5\xbd\xba\xc6\xe4\xb6\xf3\xb5\xb5',
			attachedEntity: true
		}
	],

	638: [
		{
			//EF_LIGHTNING_S	Ground Drift Grenade
			type: 'SPR',
			file: '\xb6\xf3\xc0\xcc\xc6\xae\xb4\xd7\xbd\xba\xc7\xc7\xbe\xee',
			attachedEntity: false
		}
	],

	639: [
		{
			//EF_BLIND_S	Ground Drift Grenade
			type: 'SPR',
			file: '\xba\xed\xb6\xf3\xc0\xce\xb5\xe5\xbd\xba\xc7\xc7\xbe\xee',
			attachedEntity: false
		}
	],

	640: [
		{
			//EF_POISON_S	Ground Drift Grenade
			type: 'SPR',
			file: '\xc6\xf7\xc0\xcc\xc1\xf0\xbd\xba\xc7\xc7\xbe\xee',
			attachedEntity: false
		}
	],

	641: [
		{
			//EF_FREEZING_S	Ground Drift Grenade
			type: 'SPR',
			file: '\xc7\xc1\xb8\xae\xc2\xa1\xbd\xba\xc7\xc7\xbe\xee',
			attachedEntity: false
		}
	],

	642: [
		{
			//EF_FLARE_S	Ground Drift Grenade
			type: 'SPR',
			file: '\xc7\xc3\xb7\xb9\xbe\xee\xbd\xba\xc7\xc7\xbe\xee',
			attachedEntity: false
		}
	],

	643: [
		{
			//EF_RAPIDSHOWER	Rapid Shower
			type: 'SPR',
			file: '\xb7\xa1\xc7\xc7\xb5\xe5\xbb\xfe\xbf\xf6',
			wav: 'effect/\xb7\xa1\xc7\xc7\xb5\xe5\xbb\xfe\xbf\xf6',
			attachedEntity: true
		}
	],

	644: [
		{
			//EF_MAGICALBULLET	Magic Bullet
			type: 'SPR',
			file: '\xb8\xc5\xc1\xf6\xc4\xc3\xba\xd2\xb8\xb4',
			wav: 'effect/\xb8\xc5\xc1\xf6\xc4\xc3\xba\xed\xb8\xb4',
			attachedEntity: true
		}
	],

	645: [
		{
			//EF_SPREADATTACK	Spread Attack
			type: 'SPR',
			file: '\xbd\xba\xc7\xc1\xb7\xb9\xb5\xe5',
			attachedEntity: true,
			direction: true
		}
	],

	646: [
		{
			//EF_TRACKCASTING	Tracking (Shown While Casting)
			type: 'STR',
			file: '\xc6\xae\xb7\xa2\xc5\xb7',
			attachedEntity: true
		}
	],

	647: [
		{
			//EF_TRACKING	Tracking
			type: 'SPR',
			file: '\xc6\xae\xb7\xa1\xc5\xb7',
			attachedEntity: true
		}
	],

	648: [
		{
			//EF_TRIPLEACTION	Triple Action
			type: 'SPR',
			file: '\xc6\xae\xb8\xae\xc7\xc3\xbe\xd7\xbc\xc7',
			wav: 'effect/\xc6\xae\xb8\xae\xc7\xc3\xbe\xd7\xbc\xc7',
			attachedEntity: true
		}
	],

	649: [
		{
			//EF_BULLSEYE	Bull's Eye
			type: 'STR',
			file: '\xba\xd2\xbd\xba\xbe\xc6\xc0\xcc', //oO1o3AAI
			attachedEntity: true
		}
	],

	//650: [{}],	//EF_MAP_MAGICZONE	   Ice Cave Level 4 Circle
	//651: [{}],	//EF_MAP_MAGICZONE2	   Ice Cave Level 4 Big Circle
	//652: [{}],	//EF_DAMAGE1	   Like Regeneration Number but Red with a Sound
	//653: [{}],	//EF_DAMAGE1_2	   Like Regeneration Number but Red
	//654: [{}],	//EF_DAMAGE1_3	   Like Regeneration Number but Purple
	//655: [{}],	//EF_UNDEADBODY	   Mobs Skill (Change Undead Element)
	//656: [{}],	//EF_UNDEADBODY_DEL	   Last animation before Change Undead Element finish
	//657: [{}],	//EF_GREEN_NUMBER	   (Nothing)
	//658: [{}],	//EF_BLUE_NUMBER	   (Nothing)
	//659: [{}],	//EF_RED_NUMBER	   (Nothing)
	//660: [{}],	//EF_PURPLE_NUMBER	   (Nothing)
	//661: [{}],	//EF_BLACK_NUMBER	   (Nothing)
	//662: [{}],	//EF_WHITE_NUMBER	   (Nothing)
	//663: [{}],	//EF_YELLOW_NUMBER	   (Nothing)
	//664: [{}],	//EF_PINK_NUMBER	   (Nothing)
	//665: [{}],	//EF_BUBBLE_DROP	   Little Blue Ball Falling From the Sky

	666: [
		{
			//Earthquake	//EF_NPC_EARTHQUAKE	Earthquake
			type: 'SPR',
			file: '\xbe\xee\xbd\xba\xc4\xf9\xc0\xcc\xc5\xa9',
			wav: 'effect/earth_quake',
			attachedEntity: true
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			duplicate: 3,
			timeBetweenDupli: 35,
			func: function (Params) {
				const start = Params.Inst.startTick;
				Camera.setQuake(start);
			}
		}
	],

	//667: [{}],	//EF_DA_SPACE	   (Nothing)

	668: [
		{
			//EF_DRAGONFEAR	Dragonfear
			type: 'STR',
			file: 'dragon_h',
			wav: 'effect/dragonfear',
			attachedEntity: true
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				Camera.setQuake(start);
			}
		}
	],

	669: [
		{
			//wide bleeding	//EF_BLEEDING	Wide Bleeding
			type: 'STR',
			file: 'wideb',
			wav: 'effect/wideb',
			attachedEntity: true
		}
	],

	670: [
		{
			//EF_WIDECONFUSE	Dragon fear (Visual Effect)
			type: 'STR',
			file: 'dfear',
			wav: 'effect/dragonfear',
			attachedEntity: true
		}
	],

	671: [
		{
			//EF_BOTTOM_RUNNER	   The Japan Earth Symbol (like 'Seven Wind Lv1', but on the ground)
			file: 'effect/hanmoon1.tga'
		}
	],

	672: [
		{
			//EF_BOTTOM_TRANSFER	   The Japan Wind Symbol (like 'Seven Wind Lv2', but on the ground)
			file: 'effect/hanmoon2.tga'
		}
	],

	//673: [{}],	//EF_CRYSTAL_BLUE	   Map turns Blue (like Soul Link)

	674: [
		{
			// evil land
			type: 'FUNC',
			attachedEntity: false,
			//file: 'status-curse',
			func: function (Params) {
				const self = this;
				EvillandEffects.forEach(function (effect) {
					self.add(new effect(Params.Inst.position, Params.Inst.startTick), Params);
				});
			}
		}
	],

	675: [
		{
			//EF_GUARD3	   Like Parrying/Kyrie Eleison barrier but Yellow with small Cross in every barrier piece
			wav: 'effect/kyrie_guard',
			attachedEntity: true
		}
	],

	677: [
		{
			//EF_CRITICALWOUND	Critical Wounds/Bleeding Attack
			type: 'STR',
			file: 'cwound',
			attachedEntity: true
		}
	],

	//678: [{}],	//EF_GREEN99_3	   White 99 Aura Bubbles
	//679: [{}],	//EF_GREEN99_5	   Green Aura (Middle)
	//680: [{}],	//EF_GREEN99_6	   Green Aura (Bottom)
	//681: [{}],	//EF_MAPSPHERE	   Dimensional Gorge Map Effect

	682: [
		{
			//EF_POK_LOVE	   I Love You Banner
			type: 'SPR',
			file: '\xc6\xf8\xc1\xd7\x5f\xb7\xaf\xba\xea',
			attachedEntity: true
		},
		{
			type: 'STR',
			file: 'itempokjuk',
			wav: 'effect/itempokjuk',
			attachedEntity: true
		}
	],
	683: [
		{
			//EF_POK_WHITE	   Happy White Day Banner
			type: 'SPR',
			file: '\xc6\xf8\xc1\xd7\x5f\xc8\x5f\xc0\xcc\xc6\xae\xb5\xa5\xc0\xcc',
			attachedEntity: true
		},
		{
			type: 'STR',
			file: 'itempokjuk',
			wav: 'effect/itempokjuk',
			attachedEntity: true
		}
	],
	684: [
		{
			//EF_POK_VALEN	   Happy Valentine Day Banner
			type: 'SPR',
			file: '\xc6\xf8\xc1\xd7\x5f\xb9\xdf\xb7\xbb\xc5\xb8\xc0\xce',
			attachedEntity: true
		},
		{
			type: 'STR',
			file: 'itempokjuk',
			wav: 'effect/itempokjuk',
			attachedEntity: true
		}
	],
	685: [
		{
			//EF_POK_BIRTH	   Happy Birthday Banner
			type: 'SPR',
			file: '\xc6\xf8\xc1\xd7\x5f\xbb\xfd\xc0\xcf',
			attachedEntity: true
		},
		{
			type: 'STR',
			file: 'itempokjuk',
			wav: 'effect/itempokjuk',
			attachedEntity: true
		}
	],
	686: [
		{
			//EF_POK_CHRISTMAS	   Merry Christmas Banner
			type: 'SPR',
			file: '\xc6\xf8\xc1\xd7\x5f\xc5\xa9\xb8\xae\xbd\xba\xb8\xb6\xbd\xba',
			attachedEntity: true
		},
		{
			type: 'STR',
			file: 'itempokjuk',
			wav: 'effect/itempokjuk',
			attachedEntity: true
		}
	],
	//687: [{}],	//EF_MAP_MAGICZONE3	   Cast Circle-Like effect 1
	//688: [{}],	//EF_MAP_MAGICZONE4	   Cast Circle-Like effect 2
	//689: [{}],	//EF_DUST	   Endless Tower Map Effect
	//690: [{}],	//EF_TORCH_RED	   Burning Flame (Red)
	//691: [{}],	//EF_TORCH_GREEN	   Burning Flame (Green)
	//692: [{}],	//EF_MAP_GHOST	   Unknown Aura Bubbles (Small ghosts)
	//693: [{}],	//EF_GLOW1	   Translucent yellow circle
	//694: [{}],	//EF_GLOW2	   Translucent green circle
	//695: [{}],	//EF_GLOW4	   Rotating green light
	//696: [{}],	//EF_TORCH_PURPLE	   The same of 690 and 691 but Blue/Purple

	697: [
		{
			//EF_CLOUD7	   (Night Clouds [Dark Blue])
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				CloudWeatherEffect.startOrRestart(Params);
			}
		}
	],

	698: [
		{
			//EF_CLOUD8	   (Sunset Clouds [Orange])
			type: 'FUNC',
			attachedEntity: false,
			func: function (Params) {
				CloudWeatherEffect.startOrRestart(Params);
			}
		}
	],

	699: [
		{
			//EF_FLOWERLEAF	Fall of powder from the sky and raise of some leaf
			type: 'STR',
			file: 'flower_leaf',
			attachedEntity: true
		}
	],

	//700: [{}],	//EF_MAPSPHERE2	   Big Colored Green Sphere.
	//701: [{}],	//EF_GLOW11	   Huge Blue Sphere
	//702: [{}],	//EF_GLOW12	   Little Colored Violet Sphere
	//703: [{}],	//EF_CIRCLELIGHT	   Light Infiltration with fall of pownder

	704: [
		{
			//EF_ITEM315	Client Error (mobile_ef02.str)
			type: 'STR',
			file: 'mobile_ef02',
			attachedEntity: true
		}
	],

	705: [
		{
			//EF_ITEM316	Client Error (mobile_ef01.str)
			type: 'STR',
			file: 'mobile_ef01',
			attachedEntity: true
		}
	],

	706: [
		{
			//EF_ITEM317	Client Error (mobile_ef03.str)
			type: 'STR',
			file: 'mobile_ef03',
			attachedEntity: true
		}
	],

	//707: [{}],	//EF_ITEM318	   Client Crash :P

	708: [
		{
			//EF_STORM_MIN	Storm Gust (same as 89)
			type: 'STR',
			file: 'storm_min',
			wav: 'effect/wizard_stormgust',
			attachedEntity: true
		}
	],

	709: [
		{
			//EF_POK_JAP	A Firework that split in 4 mini fireworks
			type: 'STR',
			file: 'pokjuk_jap',
			attachedEntity: false
		}
	],

	//710: [{}],	//EF_MAP_GREENLIGHT	   A Sphere like Effect 701 but Green, and a bit more larger
	//711: [{}],	//EF_MAP_MAGICWALL	   A big violet wall
	//712: [{}],	//EF_MAP_GREENLIGHT2	   A Little Flame Sphere
	//713: [{}],	//EF_YELLOWFLY1	   A lot of Very Small and Yellow Sphere
	//714: [{}],	//EF_YELLOWFLY2	   (Nothing)
	715: [
		// EF_BOTTOM_BLUE - Little blue Basilica
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 1.5,
			topSize: 1.5,
			height: 2.0,
			alphaMax: 40 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			red: 0.2,
			green: 0.6,
			blue: 1.0,
			duration: 20000,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 1.58,
			topSize: 1.58,
			height: 2.1,
			alphaMax: 32 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 10,
			red: 0.2,
			green: 0.6,
			blue: 1.0,
			duration: 20000,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 1.65,
			topSize: 1.65,
			height: 2.0,
			alphaMax: 15 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 26.6,
			red: 0.1,
			green: 0.4,
			blue: 1.0,
			duration: 20000,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 1.65,
			topSize: 1.65,
			height: 2.0,
			alphaMax: 15 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 79.8,
			red: 0.1,
			green: 0.4,
			blue: 1.0,
			duration: 20000,
			attachedEntity: true
		}
	],

	716: [
		// EF_BOTTOM_BLUE2 - Little blue Basilica
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 1.5,
			topSize: 1.5,
			height: 2.0,
			alphaMax: 40 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			red: 0.2,
			green: 0.6,
			blue: 1.0,
			duration: 20000,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 1.58,
			topSize: 1.58,
			height: 2.1,
			alphaMax: 32 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 10,
			red: 0.2,
			green: 0.6,
			blue: 1.0,
			duration: 20000,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 1.65,
			topSize: 1.65,
			height: 2.0,
			alphaMax: 15 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 26.6,
			red: 0.1,
			green: 0.4,
			blue: 1.0,
			duration: 20000,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 1.65,
			topSize: 1.65,
			height: 2.0,
			alphaMax: 15 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 79.8,
			red: 0.1,
			green: 0.4,
			blue: 1.0,
			duration: 20000,
			attachedEntity: true
		}
	],

	717: [
		{
			//EF_WEWISH	Christmas Carol (copy of Angelus)
			type: 'STR',
			file: 'angelus',
			wav: 'effect/wewish',
			min: 'jong_mini',
			attachedEntity: true
		}
	],

	718: [
		{
			//EF_FIREPILLARON2	   Judex (Visual Effect)
			wav: 'effect/ab_judex'
		},
		{
			type: 'CYLINDER',
			attachedEntity: false,
			topSize: 0.5,
			bottomSize: 0.4,
			textureName: 'ring_white',
			height: 3.5,
			duration: 1000,
			rotate: true
		},
		{
			type: 'CYLINDER',
			attachedEntity: false,
			topSize: 0.75,
			bottomSize: 0.45,
			textureName: 'ring_white',
			height: 2.5,
			duration: 1000,
			rotate: true
		},
		{
			type: 'CYLINDER',
			attachedEntity: false,
			topSize: 1,
			bottomSize: 0.5,
			textureName: 'ring_white',
			height: 1.5,
			duration: 1000,
			rotate: true
		}
	],

	719: [
		{
			//EF_FORESTLIGHT5	   Renovatio (light beam)
			wav: 'effect/ab_renovation'
		}
	],
	//720: [{}],	//EF_SOULBREAKER3	   Yellow version of Soul Breaker

	721: [
		{
			//EF_ADO_STR	Adoramus (lightning bolt)
			type: 'STR',
			file: 'ado',
			wav: 'effect/ab_adoramus',
			attachedEntity: true
		}
	],

	722: [
		{
			//EF_IGN_STR	Ignition Break (big explosion)
			type: 'STR',
			file: '\xc0\xcc\xb1\xd7\xb4\xcf\xbc\xc7\xba\xea\xb7\xb9\xc0\xcc\xc5\xa9',
			wav: 'effect/wl_jackfrost',
			attachedEntity: true
		}
	],

	//723: [{}],	//EF_CHIMTO2	   Hundred Spear (sound effect)
	//724: [{}],	//EF_WINDCUTTER	   Green version of Detecting
	//725: [{}],	//EF_DETECT2	   Oratorio (like Detecting)
	726: [
		{
			//EF_FROSTMYSTY	   Frost Misty (blue vapor and bubbles)
			wav: 'effect/t_\xbf\xa1\xb3\xca\xc1\xf6\xb9\xe6\xc3\xe2'
		}
	],

	727: [
		{
			//EF_CRIMSON_STR	Crimson Rock
			type: 'STR',
			file: 'crimson_r',
			wav: 'effect/crimson_r',
			attachedEntity: true
		}
	],

	728: [
		{
			//EF_HELL_STR	Small fire (part of Hell Inferno)
			type: 'STR',
			file: 'hell_in',
			attachedEntity: true
		}
	],

	729: [
		{
			//EF_SPR_MASH	   Marsh of Abyss (like Close Confine)
			type: 'SPR',
			file: 'mashofa'
		}
	],
	//730: [{}],	//EF_SPR_SOULE	   Small, cartoony explosion (part of Soul Expansion)

	731: [
		{
			//EF_DHOWL_STR	Dragon Howling (blinking, expanding circle)
			type: 'STR',
			file: 'dragon_h',
			wav: 'dragon_h',
			attachedEntity: true
		}
	],

	732: [
		{
			//EF_EARTHWALL	   Spike from the ground
			type: 'QuadHorn',
			textureFile: 'effect/stone.bmp',
			attachedEntity: false,
			duration: 1000,
			height: [0.75, 1.2],
			offsetX: [0.2, 0.2],
			offsetY: [0.2, 0.2],
			offsetZ: -0.1,
			bottomSize: [0.4, 0.9],
			blendMode: 1,
			rotateY: [1, 360],
			rotateZ: [-8.0, 8.0],
			color: [1.0, 1.0, 1.0, 1.0],
			animation: 3,
			animationSpeed: 250,
			animationOut: true,
			wav: 'effect/wizard_earthspike'
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				const duration = 200;
				Camera.setQuake(start, duration);
			}
		}
	],

	//733: [{}],	//EF_SOULBREAKER4	   Fluffy Ball flying by

	734: [
		{
			//EF_CHAINL_STR	Chain Lightning
			type: 'STR',
			file: 'chainlight',
			wav: 'effect/chainlight',
			attachedEntity: true
		}
	],

	//735: [{}],	//EF_CHOOKGI_FIRE	   (Nothing) - Used for elemental spheres (warlock)
	//736: [{}],	//EF_CHOOKGI_WIND	   (Nothing) - Used for elemental spheres (warlock)
	//737: [{}],	//EF_CHOOKGI_WATER	   (Nothing) - Used for elemental spheres (warlock)
	//738: [{}],	//EF_CHOOKGI_GROUND	   (Nothing) - Used for elemental spheres (warlock)
	//739: [{}],	//EF_MAGENTA_TRAP	   Old Magenta Trap
	//740: [{}],	//EF_COBALT_TRAP	   Old Cobald Trap
	//741: [{}],	//EF_MAIZE_TRAP	   Old Maize Trap
	//742: [{}],	//EF_VERDURE_TRAP	   Old Verdure Trap
	//743: [{}],	//EF_NORMAL_TRAP	   White Ranger Trap
	//744: [{}],	//EF_CLOAKING2	   Camouflage

	745: [
		{
			//EF_AIMED_STR	Aimed Bolt (crosshairs)
			type: 'STR',
			file: 'aimed',
			attachedEntity: true
		}
	],

	746: [
		{
			//EF_ARROWSTORM_STR	Arrow Storm
			type: 'STR',
			file: 'arrowstorm',
			attachedEntity: true
		}
	],

	747: [
		{
			//EF_LAULAMUS_STR	Falling white feathers
			type: 'STR',
			file: 'laulamus',
			attachedEntity: true
		}
	],

	748: [
		{
			//EF_LAUAGNUS_STR	Falling blue feathers
			type: 'STR',
			file: 'lauagnus',
			attachedEntity: true
		}
	],

	749: [
		{
			//EF_MILSHIELD_STR	Millennium Shield
			type: 'STR',
			file: 'mil_shield',
			attachedEntity: true
		}
	],

	750: [
		{
			//EF_CONCENTRATION2	Detonator (blue sparkles)
			type: 'STR',
			file: 'concentration',
			attachedEntity: true
		}
	],

	//751: [{}],	//EF_FIREBALL2	   Releasing summoned warlock spheres
	//752: [{}],	//EF_BUNSINJYUTSU2	   Like Energy Coat, but not as dark
	//753: [{}],	//EF_CLEARTIME	   Clearance

	754: [
		{
			//EF_GLASSWALL3	   Green warp portal (root of Epiclesis)
			alphaMax: 0.4,
			animation: 4,
			attachedEntity: true,
			blendMode: 2,
			red: 0.6,
			green: 1.0,
			blue: 0.6,
			bottomSize: 2.4,
			duration: 500,
			duplicate: 150,
			fadeOut: true,
			height: 0.1,
			posZ: 0.1,
			rotate: true,
			textureName: 'magic_green',
			timeBetweenDupli: 200,
			topSize: 3.9,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.4,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			red: 0.6,
			green: 1.0,
			blue: 0.6,
			bottomSize: 0.6,
			duration: 30000,
			fade: true,
			height: 7,
			rotate: true,
			textureName: 'magic_green',
			topSize: 0.6,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.4,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			red: 0.6,
			green: 1.0,
			blue: 0.6,
			bottomSize: 0.8,
			duration: 30000,
			fade: true,
			height: 6,
			rotate: true,
			textureName: 'magic_green',
			topSize: 0.8,
			type: 'CYLINDER'
		},
		{
			alphaMax: 0.5,
			animation: 0,
			attachedEntity: true,
			blendMode: 2,
			red: 0.6,
			green: 1.0,
			blue: 0.6,
			bottomSize: 1,
			duration: 30000,
			fade: true,
			height: 1,
			posZ: 2,
			rotate: true,
			totalCircleSides: 20,
			circleSides: 10,
			repeatTextureX: 2,
			textureName: 'alpha1',
			topSize: 1,
			type: 'CYLINDER'
		}
	],

	//755: [{}],	//EF_ORATIO	   Oratio (spinning blue symbol)

	756: [
		{
			//EF_POTION_BERSERK2	Enchant Blade (like Berserk Potion)
			type: 'STR',
			file: '\xb9\xf6\xbc\xad\xc5\xa9',
			attachedEntity: true
		}
	],

	//757: [{}],	//EF_CIRCLEPOWER	   Third Class Aura (Middle)
	758: [
		{
			//EF_ROLLING1	   Rolling Cutter - Spin Count 1
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd1.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd1.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd1.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd1.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd1.tga'
		}
	],

	759: [
		{
			//EF_ROLLING2	   Rolling Cutter - Spin Count 2
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd2.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd2.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd2.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd2.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd2.tga'
		}
	],

	760: [
		{
			//EF_ROLLING3	   Rolling Cutter - Spin Count 3
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd3.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd3.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd3.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd3.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd3.tga'
		}
	],

	761: [
		{
			//EF_ROLLING4	   Rolling Cutter - Spin Count 4
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd4.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd4.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd4.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd4.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd4.tga'
		}
	],

	762: [
		{
			//EF_ROLLING5	   Rolling Cutter - Spin Count 5
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd5.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd5.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd5.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd5.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd5.tga'
		}
	],

	763: [
		{
			//EF_ROLLING6	   Rolling Cutter - Spin Count 6
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd6.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd6.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd6.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd6.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd6.tga'
		}
	],

	764: [
		{
			//EF_ROLLING7	   Rolling Cutter - Spin Count 7
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd7.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd7.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd7.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd7.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd7.tga'
		}
	],

	765: [
		{
			//EF_ROLLING8	   Rolling Cutter - Spin Count 8
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd8.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd8.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd8.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd8.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd8.tga'
		}
	],

	766: [
		{
			//EF_ROLLING9	   Rolling Cutter - Spin Count 9
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd9.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd9.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd9.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd9.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd9.tga'
		}
	],

	767: [
		{
			//EF_ROLLING10	   Rolling Cutter - Spin Count 10
			alphaMax: 1,
			blendMode: 1,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 200,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd10.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.7,
			green: 0.7,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 220,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd10.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.5,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 240,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd10.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.3,
			green: 0.3,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 260,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd10.tga'
		},
		{
			alphaMax: 0.2,
			blendMode: 2,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			attachedEntity: true,
			duration: 1000,
			fadeIn: true,
			fadeOut: true,
			posz: 4,
			sizeEnd: 20,
			sizeSmooth: true,
			sizeStart: 280,
			type: '3D',
			zIndex: 10,
			file: 'effect/\xc8\xb8\xc0\xfc\xc4\xab\xbf\xee\xc5\xcd10.tga'
		}
	],

	//768: [{}],	//EF_PURPLEBODY	   Blinking
	//769: [{}],	//EF_STIN6	   Cross Ripper Slasher (flying knives)
	//770: [{}],	//EF_RG_COIN4	   Strip sound
	//771: [{}],	//EF_POISONWAV	   Poison sound
	//772: [{}],	//EF_POISONSMOKE	   Poison particles
	//773: [{}],	//EF_GUMGANG4	   Expanding purple aura (part of Phantom Menace)
	//774: [{}],	//EF_SHIELDBOOMERANG4	   Axe Boomerang

	775: [
		{
			//EF_CASTSPIN2	   Spinning character sprite
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				const duration = 500;
				const count = 8;
				const delay = duration / count;

				for (let i = 0; i < count; i++) {
					const delta = 1;

					Events.setTimeout(function () {
						entity.direction = Math.floor(entity.direction + delta) % 8;
					}, delay * i);
				}
			}
		}
	],

	//776: [{}],	//EF_VULCANWAV	   Like Desperado sound effect
	//777: [{}],	//EF_AGIUP2	   Faded light from the ground [S]
	//778: [{}],	//EF_DETECT3	   Expanding white aura (like Clearance)
	//779: [{}],	//EF_AGIUP3	   Faded light from the ground [S]
	//780: [{}],	//EF_DETECT4	   Expanding red aura (from Infrared Scan)
	//781: [{}],	//EF_ELECTRIC3	   Magnetic Field (purple chains)
	//782: [{}],	//EF_GUARD4	   All-around shield [S]
	//783: [{}],	//EF_BOTTOM_BARRIER	   Yellow shaft of light
	//784: [{}],	//EF_BOTTOM_STEALTH	   White shaft of light
	//785: [{}],	//EF_REPAIRTIME	   Upward flying wrenches
	//786: [{}],	//EF_NC_ANAL	   Symbol with bleeping sound [S]
	//787: [{}],	//EF_FIRETHROW	   Flare Launcher (line of fire)
	//788: [{}],	//EF_VENOMIMPRESS	   Venom Impress (green skull)
	//789: [{}],	//EF_FROSTMISTY	   Freezing Status Effect (two ancillas)
	//790: [{}],	//EF_BURNING	   Burning Status Effect (flame symbol)
	//791: [{}],	//EF_COLDTHROW	   Two ice shots
	//792: [{}],	//EF_MAKEHALLU	   Upward streaming white particles
	//793: [{}],	//EF_HALLUTIME	   Same, but more brief
	//794: [{}],	//EF_INFRAREDSCAN	   Infrared Scan (red lasers)

	795: [
		{
			//EF_CRASHAXE	Power Swing (axe crash)
			type: 'STR',
			file: 'powerswing',
			attachedEntity: true
		}
	],

	//796: [{}],	//EF_GTHUNDER	   Spinning blue triangles
	//797: [{}],	//EF_STONERING	   Stapo
	//798: [{}],	//EF_INTIMIDATE2	   Red triangles (like Intimidate)
	799: [
		{
			//EF_STASIS	   Stasis (expanding blue mist) [S]
			wav: 'effect/wl_stasis'
		}
	],
	//800: [{}],	//EF_REDLINE	   Hell Inferno (red lights)
	//801: [{}],	//EF_FROSTDIVER3	   Jack Frost unit (ice spikes)

	802: [
		//EF_BOTTOM_BASILICA2	   White Imprison
		// F1=0, GI[0] - Alpha 65, Height 12.5
		{
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 2.2,
			topSize: 2.2,
			height: 3.0,
			alphaMax: 65 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			duration: 20000,
			attachedEntity: true
		},
		{
			// F1=0, GI[1] - Alpha 65, Height 12.9
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 2.25,
			topSize: 2.25,
			height: 3.1,
			alphaMax: 65 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 10,
			duration: 20000,
			attachedEntity: true
		},
		{
			// F1=0, GI[2] - Alpha 15, Distance 0
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 2.3,
			topSize: 2.3,
			height: 3.0,
			alphaMax: 15 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			duration: 20000,
			attachedEntity: true
		},
		{
			// F1=0, GI[3] - Alpha 15, Distance 53.2
			type: 'CYLINDER',
			textureName: 'alpha_down',
			totalCircleSides: 4,
			circleSides: 4,
			bottomSize: 2.3,
			topSize: 2.3,
			height: 3.0,
			alphaMax: 15 / 255,
			blendMode: 2,
			rotateWithCamera: true,
			angleY: 53.2,
			duration: 20000,
			attachedEntity: true
		},
		{
			wav: 'effect/wl_whiteimprison'
		}
	],

	803: [
		{
			//EF_RECOGNIZED	   Recognized Spell
			wav: 'effect/wl_recognizedspell'
		}
	],

	804: [
		{
			//EF_TETRA	   Tetra Vortex [S]
			wav: 'effect/wl_tetravortex'
		}
	],

	805: [
		{
			//EF_TETRACASTING	   Tetra Vortex cast animation (blinking colors)
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					if (!entity.cast.display) {
						//we don't know cast time here so.. let's hack
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return true;
					}
					entity._flashColor[0] = Math.random();
					entity._flashColor[1] = Math.random();
					entity._flashColor[2] = Math.random();
					entity._flashColor[3] = 0.5 + Math.random() / 2;
					entity.recalculateBlendingColor();
				});
			}
		}
	],

	//806: [{}],	//EF_FIREBALL3	   Flying by as fast as a rocket
	//807: [{}],	//EF_INTIMIDATE3	   Kidnapping sound
	//808: [{}],	//EF_RECOGNIZED2	   Like Recognized Spell, but one symbol
	//809: [{}],	//EF_CLOAKING3	   Shadowy filter [S]
	//810: [{}],	//EF_INTIMIDATE4	   Damp thud sound [S]

	811: [
		{
			//EF_STRETCH	   Body Painting
			wav: 'effect/bodypaint'
		}
	],

	//812: [{}],	//EF_BLACKBODY	   Black expanding aura

	813: [
		{
			//EF_ENERVATION	Masquerade - Enervation
			type: 'STR',
			file: 'enervation',
			attachedEntity: true
		}
	],

	814: [
		{
			//EF_ENERVATION2	Masquerade - Groomy
			type: 'STR',
			file: 'groomy',
			attachedEntity: true
		}
	],

	815: [
		{
			//EF_ENERVATION3	Masquerade - Ignorance
			type: 'STR',
			file: 'ignorance',
			attachedEntity: true
		}
	],

	816: [
		{
			//EF_ENERVATION4	Masquerade - Laziness
			type: 'STR',
			file: 'laziness',
			wav: 'effect/laziness',
			attachedEntity: true
		}
	],

	817: [
		{
			//EF_ENERVATION5	Masquerade - Unlucky
			type: 'STR',
			file: 'unlucky',
			attachedEntity: true
		}
	],

	818: [
		{
			//EF_ENERVATION6	Masquerade - Weakness
			type: 'STR',
			file: 'weakness',
			attachedEntity: true
		}
	],

	//819: [{}],	//EF_LINELINK4	   (Nothing)
	//820: [{}],	//EF_RG_COIN5	   Strip Accessory
	//821: [{}],	//EF_WATERFALL_ANI	   Waterfall

	822: [
		{
			wav: 'effect/dimension'
		}
	], //EF_BOTTOM_MANHOLE	   Dimension Door (spinning blue aura)

	823: [
		{
			//EF_MANHOLE	   in-the-manhole effect
			wav: 'effect/manhole'
		}
	],

	//824: [{}],	//EF_MAKEFEINT	   Some filter
	825: [
		{
			wav: 'effect/dimension'
		}
	], //EF_FORESTLIGHT6	   Dimension Door (aura + blue light)
	//826: [{}],	//EF_DARKCASTING2	   Expanding black casting anim:

	827: [
		{
			//EF_BOTTOM_ANI	   Chaos Panic (spinning brown aura)
			wav: 'effect/chaospanic'
		}
	],

	828: [
		{
			//EF_BOTTOM_MAELSTROM	   Maelstrom (spinning pink aura)
			wav: 'effect/maelstrom'
		}
	],

	829: [
		{
			//EF_BOTTOM_BLOODYLUST	   Bloody Lust (spinning red aura)
			wav: 'effect/bloodylust'
		}
	],

	//830: [{}],	//EF_BEGINSPELL_N1	   Blue aura (Arch Bishop cast animation)
	//831: [{}],	//EF_BEGINSPELL_N2	   Blue cone [S]

	832: [
		{
			//EF_HEAL_N	   Sonic Wave
			wav: 'effect/\xb1\xe2\xb0\xf8\xc6\xf7'
		}
	],

	833: [
		{
			//EF_CHOOKGI_N	   (Nothing) - Used for spheres (royal guard?)
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const spiritNum = Params.Init.spiritNum || 0;
				const Spheres = new SpiritSphere(Params.Init.ownerEntity, spiritNum, false);
				this.add(Spheres, Params);
			}
		}
	],

	//834: [{}],	//EF_JOBLVUP50_2	   Light shooting away circlish
	//835: [{}],	//EF_CHEMICAL2DASH2	   Fastness yellow-reddish
	//836: [{}],	//EF_CHEMICAL2DASH3	   Fastness yellow-pinkish
	//837: [{}],	//EF_ROLLINGCAST	   Casting [S]
	//838: [{}],	//EF_WATER_BELOW	   Watery aura
	//839: [{}],	//EF_WATER_FADE	   [Client Error]
	//840: [{}],	//EF_BEGINSPELL_N3	   Red cone
	//841: [{}],	//EF_BEGINSPELL_N4	   Green cone
	//842: [{}],	//EF_BEGINSPELL_N5	   Yellow cone
	//843: [{}],	//EF_BEGINSPELL_N6	   White cone
	//844: [{}],	//EF_BEGINSPELL_N7	   Purple cone
	//845: [{}],	//EF_BEGINSPELL_N8	   light-bluish turquoise cone
	//846: [{}],	//EF_WATER_SMOKE	   (Nothing)

	847: [
		{
			//EF_DANCE1	   Gloomy Day (white/red light rays)
			wav: 'effect/\xbc\xf6\xc1\xdd\xc0\xba\xc7\xcf\xb7\xe7\xc0\xc7\xbf\xec\xbf\xef'
		}
	],

	//848: [{}],	//EF_DANCE2	   Gloomy Day (white/blue light rays)
	//849: [{}],	//EF_LINKPARTICLE	   (Nothing)
	//850: [{}],	//EF_SOULLIGHT2	   (Nothing)
	//851: [{}],	//EF_SPR_PARTICLE	   Green mushy-foggy stuff (dull)
	//852: [{}],	//EF_SPR_PARTICLE2	   Green mushy-foggy stuff (bright)
	//853: [{}],	//EF_SPR_PLANT	   Bright green flower area
	//854: [{}],	//EF_CHEMICAL_V	   Blue beam of light with notes
	//855: [{}],	//EF_SHOOTPARTICLE	   (Nothing)

	856: [
		{
			//EF_BOT_REVERB	   Reverberation (red eighth notes)
			type: '3D',
			file: 'effect/melody_b.bmp',
			red: 1,
			green: 0.6,
			blue: 0.6,
			alphaMax: 0.6,
			attachedEntity: true,
			duration: 100,
			size: 50,
			posz: 0.5,
			repeat: true
		},
		{
			wav: 'effect/reverberation'
		}
	],

	857: [
		{
			//EF_RAIN_PARTICLE	   Severe Rainstorm (falling red and blue beams)
			wav: 'effect/rainstorm'
		}
	],

	858: [
		{
			//EF_CHEMICAL_V2	   Deep Sleep Lullaby (two red beams and music notes)
			wav: 'effect/\xbe\xc8\xbd\xc4\xc0\xc7\xc0\xda\xc0\xe5\xb0\xa1'
		}
	],

	//859: [{}],	//EF_SECRA	   Holograph of text (blue)

	860: [
		{
			//EF_BOT_REVERB2	   Distorted note (blue)
			type: '3D',
			file: 'effect/melody_a.bmp',
			red: 0.6,
			green: 0.6,
			blue: 1,
			alphaMax: 0.6,
			attachedEntity: true,
			duration: 100,
			size: 50,
			posz: 0.5,
			repeat: true
		},
		{
			wav: 'effect/\xb3\xaa\xb6\xf4\xc0\xc7\xb3\xeb\xb7\xa1'
		}
	],

	861: [
		{
			//EF_CIRCLEPOWER2	   Green aura (from Circle of Life's Melody)
			wav: 'effect/\xbc\xf8\xc8\xaf\xc7\xcf\xb4\xc2\xc0\xda\xbf\xac\xc0\xc7\xbc\xd2\xb8\xae'
		}
	],

	862: [
		{
			//EF_SECRA2	   Randomize Spell (holograph of text)
			attachedEntity: true,
			wav: 'effect/ab_ancilla'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.55,
			blue: 0.55,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 850,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.6,
			blue: 0.6,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 800,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.65,
			blue: 0.65,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 750,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.7,
			blue: 0.7,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 700,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.75,
			blue: 0.75,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 650,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.8,
			blue: 0.8,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 600,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.85,
			blue: 0.85,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 550,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.9,
			blue: 0.9,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 500,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 0.95,
			blue: 0.95,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 450,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		},
		{
			alphaMax: 0.3,
			blendMode: 2,
			red: 1.0,
			green: 1.0,
			blue: 1.0,
			attachedEntity: true,
			duration: 1500,
			fadeIn: true,
			fadeOut: true,
			posz: 7,
			sizeEnd: 100,
			sizeSmooth: true,
			sizeStart: 400,
			zIndex: 10,
			type: '3D',
			file: 'effect/priest_spell.bmp'
		}
	],

	//863: [{}],	//EF_CHEMICAL_V3	   Dominion Impulse (two spears of light)
	//864: [{}],	//EF_ENERVATION7	   Gloomy Day (colorful lines)
	//865: [{}],	//EF_CIRCLEPOWER3	   Blue aura (from Song of Mana)

	866: [
		{
			//EF_SPR_PLANT2	   Dance with a Warg (Wargs)
			wav: 'effect/\xbf\xf6\xb1\xd7\xbf\xcd\xc7\xd4\xb2\xb2\xc3\xe3\xc0\xbb'
		}
	],

	//867: [{}],	//EF_CIRCLEPOWER4	   Yellow aura (from Dance with a Warg)

	868: [
		{
			//EF_SPR_PLANT3	   Song of Mana (Violies)
			wav: 'effect/\xb8\xb6\xb3\xaa\xc0\xc7\xb3\xeb\xb7\xa1'
		}
	],

	//869: [{}],	//EF_RG_COIN6	   Strip sound [S]

	870: [
		{
			//EF_SPR_PLANT4	   Ghostly Succubuses of fire
			wav: 'effect/\xbb\xf5\xc5\xcd\xb5\xa5\xc0\xcc\xb3\xaa\xc0\xcc\xc6\xae\xc7\xc7\xb9\xf6'
		}
	],

	//871: [{}],	//EF_CIRCLEPOWER5	   Red aura (from Lerad's Dew)

	872: [
		{
			//EF_SPR_PLANT5	   Lerad's Dew (Minerals)
			wav: 'effect/\xb7\xb9\xb6\xf3\xb5\xe5\xc0\xc7\xc0\xcc\xbd\xbd'
		}
	],

	//873: [{}],	//EF_CIRCLEPOWER6	   Stargate-wormhole stuff (bright purple)

	874: [
		{
			//EF_SPR_PLANT6	   Melody of Sink (Ktullanuxes)
			wav: 'effect/\xb8\xe1\xb7\xce\xb5\xf0\xbf\xc0\xba\xea\xbd\xcc\xc5\xa9' //'effect/\xb3\xaa\xb6\xf4\xc0\xc7\xb3\xeb\xb7\xa1'
		}
	],

	//875: [{}],	//EF_CIRCLEPOWER7	   Stargate-wormhole stuff (bright turquoise)

	876: [
		{
			//EF_SPR_PLANT7	   Warcry of Beyond (Garms)
			wav: 'effect/\xba\xf1\xbf\xe6\xb5\xe5\xbf\xc0\xba\xea\xbf\xf6\xc5\xa9\xb6\xf3\xc0\xcc'
		}
	],

	//877: [{}],	//EF_CIRCLEPOWER8	   Stargate-wormhole stuff (white)

	878: [
		{
			//EF_SPR_PLANT8	   Unlimited Humming Voice (Miyabi Ningyos)
			wav: 'effect/\xbe\xf0\xb8\xae\xb9\xcc\xc6\xbc\xb5\xe5\xc7\xe3\xb9\xd6\xba\xb8\xc0\xcc\xbd\xba'
		}
	],

	879: [
		{
			//EF_HEARTASURA	   Siren's Voice (heart-like)
			wav: 'effect/\xbc\xbc\xc0\xcc\xb7\xbb\xc0\xc7\xb8\xf1\xbc\xd2\xb8\xae'
		}
	],

	//880: [{}],	//EF_BEGINSPELL_150	   Bluish castish cone
	//881: [{}],	//EF_LEVEL99_150	   Blue aura
	//882: [{}],	//EF_PRIMECHARGE	   Whirl of fireflies (red)

	883: [
		{
			//EF_GLASSWALL4	   Epiclesis (transparent green tree)
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 30000,
			red: 0.0001,
			green: 1.0,
			blue: 0.0001,
			size: 400,
			posz: 7,
			zIndex: 1,
			wav: 'effect/ef_readyportal',
			file: 'effect/ef_epitree.tga'
		},
		{
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 990,
			duplicate: 15,
			timeBetweenDupli: 2000,
			red: 0.0001,
			green: 1.0,
			blue: 0.0001,
			sizeStart: 380,
			sizeEnd: 420,
			posz: 7,
			zIndex: 1,
			file: 'effect/ef_epitree.tga'
		},
		{
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 990,
			duplicate: 15,
			timeBetweenDupli: 2000,
			delayOffset: 1000,
			red: 0.0001,
			green: 1.0,
			blue: 0.0001,
			sizeStart: 420,
			sizeEnd: 380,
			posz: 7,
			zIndex: 1,
			file: 'effect/ef_epitree.tga'
		}
	],

	//884: [{}],	//EF_GRADIUS_LASER	   Green beam
	885: [
		{
			//bash3d6	//EF_BASH3D6	   Blue light beams (can be unnoficial)
			wav: 'effect/bash3d', // incorrect wav name
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				entity.animations.add(function (tick) {
					// Halo (Double Body) - Blue variant
					entity._enableHalo = tick >= 150 && tick <= 450;

					if (tick >= 150 && tick <= 450) {
						// Blue tint
						entity._flashColor[0] = 0.5;
						entity._flashColor[1] = 0.5;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
					} else {
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
					}
					return tick > 500;
				});
			}
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			delayStart: 200,
			duplicate: 5,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 4.5,
			animation: 2,
			zIndex: 1.1,
			red: 0.3,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true
		},
		{
			type: 'CYLINDER',
			textureName: 'alpha_center',
			alphaMax: 0.6,
			duration: 175,
			duplicate: 5,
			delayStart: 200,
			timeBetweenDupli: 0,
			totalCircleSides: 30,
			circleSides: 1,
			fade: true,
			angleX: -90,
			angleZRandom: 360,
			fixedPerspective: true,
			posZ: 1.5,
			height: 0,
			bottomSize: 0.01,
			topSize: 7.2,
			animation: 2,
			zIndex: 1.2,
			red: 0.3,
			green: 0.5,
			blue: 1.0,
			attachedEntity: true
		}
	],
	//886: [{}],	//EF_GUMGANG5	   Blue castish cone
	//887: [{}],	//EF_HITLINE8	   Wavy sparks

	888: [
		{
			//EF_ELECTRIC4	   Earth Shaker (same as 432)
			wav: 'effect/sr_earthshaker'
		}
	],

	889: [
		//EF_TEIHIT1T	   Fast light beams (blue)
		{
			wav: 'effect/\x6d\x6f\x6e\x5f\xbe\xc6\xbc\xf6\xb6\xf3\x20\xc6\xd0\xc8\xb2\xb1\xc7',
			type: '3D',
			duplicate: 24,
			timeBetweenDupli: 0,
			file: 'effect/lens1.tga',
			delayStart: 100,
			duration: 250,
			alphaMax: 0.8,
			fadeIn: true,
			fadeOut: true,
			posxStart: 0,
			posyStart: 0,
			posxEndRand: 40,
			posyEndRand: 40,
			sizeStartX: 10,
			sizeStartY: 150,
			sizeEndX: 10,
			sizeEndY: 150,
			blendMode: 2,
			attachedEntity: true,
			red: 0.1,
			green: 0.1,
			blue: 1.0,
			overlay: true,
			rotateToTarget: true,
			rotateWithCamera: true
		}
	],
	//890: [{}],	//EF_SPINMOVE	   Rotation
	//891: [{}],	//EF_FIREBALL4	   Magic shots [S]
	//892: [{}],	//EF_TRIPLEATTACK4	   Fastness with hitting sound[S]
	//893: [{}],	//EF_CHEMICAL3S	   Blue-white light passing by
	//894: [{}],	//EF_GROUNDSHAKE	   (Nothing)
	//895: [{}],	//EF_DQ9_CHARGE	   Big wheel of flat light beams
	//896: [{}],	//EF_DQ9_CHARGE2	   Still sun shaped lightning aura
	//897: [{}],	//EF_DQ9_CHARGE3	   Animated sun shaped lightning aura
	//898: [{}],	//EF_DQ9_CHARGE4	   Animated, curvy sun shaped lightning aura
	//899: [{}],	//EF_BLUELINE	   White/red light shots from below
	//900: [{}],	//EF_SELFSCROLL	   Animated, slow curvy sun shaped lightning aura
	//901: [{}],	//EF_SPR_LIGHTPRINT	   Explosion
	//902: [{}],	//EF_PNG_TEST	   Floating bedtable texture
	//903: [{}],	//EF_BEGINSPELL_YB	   Castish flamey cone
	//904: [{}],	//EF_CHEMICAL2DASH4	   Yellow/pink lights passing by
	//905: [{}],	//EF_GROUNDSHAKE2	   Expanding circle

	906: [
		{
			//EF_PRESSURE2	   Shield Press (falling shield)
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 500,
			rotate: true,
			angle: 0,
			toAngle: -611,
			poszStart: 20,
			poszEnd: 5,
			size: 100,
			zIndex: 1,
			file: 'effect/shield.bmp',
			wav: 'effect/\xc7\xc1\xb7\xb9\xbc\xc5'
		},
		{
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 500,
			delayOffset: 501,
			fadeOut: true,
			angle: -611,
			posz: 5,
			size: 100,
			zIndex: 1,
			file: 'effect/shield.bmp',
			delayWav: 500,
			wav: 'effect/lg_shieldpress'
		}
	],

	//907: [{}],	//EF_RG_COIN7	   Chainy, metalish sound [S]

	908: [
		{
			//EF_PRIMECHARGE2	   Prestige (sphere of yellow particles)
			wav: 'effect/lg_prestige'
		}
	],

	909: [
		{
			//EF_PRIMECHARGE3	   Banding (sphere of red particles)
			wav: 'effect/lg_banding'
		}
	],

	910: [
		{
			//EF_PRIMECHARGE4	   Inspiration (sphere of blue particles)
			wav: 'effect/lg_inspiration'
		}
	],

	//911: [{}],	//EF_GREENCASTING	   Green castish animation [S]
	//912: [{}],	//EF_WALLOFTHORN	   Wall of Thorns unit (green fog cloud)
	//913: [{}],	//EF_FIREBALL5	   Magic projectiles
	//914: [{}],	//EF_THROWITEM11	   (Nothing)
	//915: [{}],	//EF_SPR_PLANT9	   Crazy Weed
	//916: [{}],	//EF_DEMONICFIRE	   Demonic Fire
	//917: [{}],	//EF_DEMONICFIRE2	   More angry, demonic flames
	//918: [{}],	//EF_DEMONICFIRE3	   Fire Insignia (demonic flames)
	//919: [{}],	//EF_HELLSPLANT	   Hell's Plant (green snapping plant)

	920: [
		{
			//EF_FIREWALL2	Fire Walk unit
			type: 'STR',
			file: 'firewall_per',
			attachedEntity: true
		}
	],

	//921: [{}],	//EF_VACUUM	   Vacuum Extreme (whirlwind)
	922: [
		{
			//EF_SPR_PLANT10	   Psychic Wave
			wav: 'effect/s\xbb\xe7\xc0\xcc\xc5\xb1\xbf\xfe\xc0\xcc\xba\xea'
		}
	],
	//923: [{}],	//EF_SPR_LIGHTPRINT2	   Poison Buster
	//924: [{}],	//EF_POISONSMOKE2	   Poisoning animation
	//925: [{}],	//EF_MAKEHALLU2	   Some filter

	926: [
		{
			//EF_SHOCKWAVE2	Electric Walk unit
			type: 'STR',
			file: 'hunter_shockwave_blue',
			attachedEntity: true
		}
	],

	//927: [{}],	//EF_SPR_PLANT11	   Earth Grave (speary roots)
	928: [
		{
			//EF_COLDTHROW2	   Ice cloud projectiles
			wav: 'effect/wl_jackfrost'
		}
	],
	929: [
		{
			//EF_DEMONICFIRE4	   Warmer (field of flames)
			wav: 'effect/s\xbf\xf6\xb8\xd3'
		}
	],

	930: [
		{
			//EF_PRESSURE3	   Varetyr Spear (falling spear)
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 500,
			rotate: true,
			angle: 0,
			toAngle: -611,
			poszStart: 20,
			poszEnd: 5,
			size: 100,
			zIndex: 1,
			file: 'effect/cross1.bmp',
			wav: 'effect/\xc7\xc1\xb7\xb9\xbc\xc5'
		},
		{
			type: '3D',
			alphaMax: 0.6,
			attachedEntity: true,
			blendMode: 2,
			duration: 500,
			delayOffset: 501,
			fadeOut: true,
			angle: -611,
			posz: 5,
			size: 100,
			zIndex: 1,
			file: 'effect/cross1.bmp'
		}
	],

	1130: [
		{
			//EF_BAKURETSU_HADOU - Hat effect example (SPR)
			type: 'SPR',
			file: 'bakuretsu_hadou/bakuretsu_hadou',
			texturePath: 'bakuretsu_hadou/',
			attachedEntity: true,
			repeat: true,
			head: true,
			yOffset: -50
		}
	],

	//931: [{}],	//EF_LINKPARTICLE2	   (Nothing)
	//932: [{}],	//EF_SOULLIGHT3	   Firefly
	//933: [{}],	//EF_CHAREFFECT	   [Client Crash]
	//934: [{}],	//EF_GUMGANG6	   White, castishly expanding cone
	//935: [{}],	//EF_FIREBALL6	   Green magic projectile
	//936: [{}],	//EF_GUMGANG7	   Red, castishly expanding cone
	//937: [{}],	//EF_GUMGANG8	   Yellow, castishly expanding cone
	//938: [{}],	//EF_GUMGANG9	   Dark-red, castishly expanding cone
	//939: [{}],	//EF_BOTTOM_DE2	   Blue, conish aura
	//940: [{}],	//EF_COLDSTATUS	   Snow flake
	//941: [{}],	//EF_SPR_LIGHTPRINT3	   Explosion of red, demonic fire
	//942: [{}],	//EF_WATERBALL3	   Expanding, white dome
	//943: [{}],	//EF_HEAL_N2	   Green, fluffy projectile
	//944: [{}],	//EF_RAIN_PARTICLE2	   Falling gems
	//945: [{}],	//EF_CLOUD9	   (Nothing)
	//946: [{}],	//EF_YELLOWFLY3	   Floating lights
	//947: [{}],	//EF_EL_GUST	   Blue lightning sphere
	//948: [{}],	//EF_EL_BLAST	   Two blue lightning spheres
	//949: [{}],	//EF_EL_AQUAPLAY	   Flat, spinning diamond
	//950: [{}],	//EF_EL_UPHEAVAL	   Circling, planetlike spheres
	//951: [{}],	//EF_EL_WILD_STORM	   Three lightning spheres
	//952: [{}],	//EF_EL_CHILLY_AIR	   Flat, spinning gem and two lightning spheres
	//953: [{}],	//EF_EL_CURSED_SOIL	   Spinning, planetlike spheres
	//954: [{}],	//EF_EL_COOLER	   Two lightblue glowing spheres
	//955: [{}],	//EF_EL_TROPIC	   Three spinning flame spheres
	//956: [{}],	//EF_EL_PYROTECHNIC	   Flame
	//957: [{}],	//EF_EL_PETROLOGY	   Spinning planetlike sphere
	//958: [{}],	//EF_EL_HEATER	   Two flames

	959: [
		{
			//EF_POISON_MIST	Purple flame
			type: 'STR',
			file: 'poison_mist',
			attachedEntity: true
		}
	],

	960: [
		{
			//EF_ERASER_CUTTER	Small yellow explosion
			type: 'STR',
			file: 'eraser_cutter',
			attachedEntity: true
		}
	],

	//961: [{}],	//EF_SILENT_BREEZE	   Cartoony whirlwind
	//962: [{}],	//EF_MAGMA_FLOW	   Rising fire
	//963: [{}],	//EF_GRAYBODY	   Dark filter (like Stone Curse)

	964: [
		{
			//EF_LAVA_SLIDE	Same as 920
			type: 'STR',
			file: 'lava_slide',
			attachedEntity: true
		}
	],

	965: [
		{
			//EF_SONIC_CLAW	Small white explosion
			type: 'STR',
			file: 'sonic_claw',
			attachedEntity: true
		}
	],

	966: [
		{
			//EF_TINDER_BREAKER	Bone crack
			type: 'STR',
			file: 'tinder',
			attachedEntity: true
		}
	],

	967: [
		{
			//EF_MIDNIGHT_FRENZY	Another little explosion
			type: 'STR',
			file: 'mid_frenzy',
			attachedEntity: true
		}
	],

	975: [
		{
			//EF_VOLCANIC_ASH
			type: 'STR',
			file: 'vash00',
			attachedEntity: true
		}
	],

	987: [
		{
			//EF_2011RWC
			type: 'STR',
			file: 'rwc2011',
			attachedEntity: true
		}
	],

	988: [
		{
			//EF_2011RWC2
			type: 'STR',
			file: 'rwc2011_2',
			attachedEntity: true
		}
	],

	1015: [
		{
			//EF_RUN_MAKE_OK
			type: 'STR',
			file: 'rune_success',
			attachedEntity: true
		}
	],

	1016: [
		{
			//EF_RUN_MAKE_FAILURE
			type: 'STR',
			file: 'rune_fail',
			attachedEntity: true
		}
	],

	1017: [
		{
			//EF_MIRESULT_MAKE_OK
			type: 'STR',
			file: 'changematerial_su',
			attachedEntity: true
		}
	],

	1018: [
		{
			//EF_MIRESULT_MAKE_FAIL
			type: 'STR',
			file: 'changematerial_fa',
			attachedEntity: true
		}
	],

	1019: [
		{
			//EF_ALL_RAY_OF_PROTECTION
			type: 'STR',
			file: 'guardian',
			attachedEntity: true
		}
	],

	1020: [
		{
			//EF_VENOMFOG
			type: 'STR',
			file: 'bubble%d_1',
			rand: [1, 4],
			attachedEntity: true
		}
	],

	1021: [
		{
			//EF_DUSTSTORM
			type: 'STR',
			file: 'dust',
			attachedEntity: true
		}
	],

	1029: [
		{
			//EF_DANCE_BLADE_ATK
			type: 'STR',
			file: 'dancingblade',
			attachedEntity: true
		}
	],

	1031: [
		{
			//EF_INVINCIBLEOFF2
			type: 'STR',
			file: 'invincibleoff2',
			attachedEntity: true
		}
	],

	1033: [
		{
			//EF_DEATHSUMMON
			type: 'STR',
			file: 'devil',
			attachedEntity: true
		}
	],

	1040: [
		{
			//EF_GC_DARKCROW
			type: 'STR',
			file: 'gc_darkcrow',
			//wav: 'effect/gc_darkcrow',
			attachedEntity: true
		}
	],

	1042: [
		{
			//EF_ALL_FULL_THROTTLE
			type: 'STR',
			file: 'all_full_throttle',
			wav: 'effect/all_full_throttle',
			attachedEntity: true
		}
	],

	1043: [
		{
			//EF_SR_FLASHCOMBO
			type: 'STR',
			file: 'sr_flashcombo',
			wav: 'effect/sr_flashcombo',
			attachedEntity: true
		}
	],

	1044: [
		{
			//EF_RK_LUXANIMA
			type: 'STR',
			file: 'rk_luxanima',
			attachedEntity: true
		}
	],

	1046: [
		{
			//EF_SO_ELEMENTAL_SHIELD
			type: 'STR',
			file: 'so_elemental_shield',
			wav: 'effect/so_elemental_shield',
			attachedEntity: true
		}
	],

	1047: [
		{
			//EF_AB_OFFERTORIUM
			type: 'STR',
			file: 'ab_offertorium',
			wav: 'effect/ab_offertorium',
			attachedEntity: true
		}
	],

	1048: [
		{
			//EF_WL_TELEKINESIS_INTENSE
			type: 'STR',
			file: 'wl_telekinesis_intense',
			wav: 'effect/wl_telekinesis_intense',
			attachedEntity: true
		}
	],

	1049: [
		{
			//EF_GN_ILLUSIONDOPING
			type: 'STR',
			file: 'gn_illusiondoping',
			wav: 'effect/gn_illusiondoping',
			attachedEntity: true
		}
	],

	1050: [
		{
			//EF_NC_MAGMA_ERUPTION
			type: 'STR',
			file: 'nc_magma_eruption',
			wav: 'effect/nc_magma_eruption',
			attachedEntity: true
		}
	],

	1055: [
		{
			//EF_NPC_CHILL
			type: 'STR',
			file: 'chill',
			attachedEntity: true
		}
	],

	1057: [
		{
			//EF_AB_OFFERTORIUM_RING
			type: 'STR',
			file: 'ab_offertorium_ring',
			attachedEntity: true
		}
	],

	1062: [
		{
			//EF_HAMMER_OF_GOD
			type: 'STR',
			file: 'stormgust',
			wav: 'effect/RL_HAMMER_OF_GOD',
			attachedEntity: true
		}
	],

	1094: [
		{
			//EF_ACH_COMPLETE
			type: 'STR',
			file: 'ach_complete/ppring3',
			texturePath: 'ach_complete/',
			attachedEntity: true
		}
	],

	1111: [
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;
				entity._flashColor[0] = 1.0;
				entity._flashColor[1] = 0.0;
				entity._flashColor[2] = 0.0;
				entity._flashColor[3] = 0.0;
				entity.recalculateBlendingColor();

				entity.animations.add(function (tick) {
					entity._flashColor[3] = 0.0 + tick / 100;
					entity.recalculateBlendingColor();
					if (tick > 300) {
						entity._flashColor[0] = 1.0;
						entity._flashColor[1] = 1.0;
						entity._flashColor[2] = 1.0;
						entity._flashColor[3] = 1.0;
						entity.recalculateBlendingColor();
						return true;
					}
				});
			}
		}
	],

	1186: [
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_pink/dropitem_pink/dropitem_pink',
			texturePath: 'new_dropitem/dropitem_pink/dropitem_pink/',
			renderBeforeEntities: true
		},
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_pink/dropitem_pink_bottom/dropitem_pink_bottom',
			texturePath: 'new_dropitem/dropitem_pink/dropitem_pink_bottom/',
			renderBeforeEntities: true
		},
		{
			wav: 'effect/drop_pink'
		}
	],
	1189: [
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_yellow/dropitem_yellow/dropitem_yellow',
			texturePath: 'new_dropitem/dropitem_yellow/dropitem_yellow/'
		},
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_yellow/dropitem_yellow_bottom/dropitem_yellow_bottom',
			texturePath: 'new_dropitem/dropitem_yellow/dropitem_yellow_bottom/'
		},
		{
			wav: 'effect/drop_yellow'
		}
	],
	1190: [
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_purple/dropitem_purple/dropitem_purple',
			texturePath: 'new_dropitem/dropitem_purple/dropitem_purple/'
		},
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_purple/dropitem_purple_bottom/dropitem_purple_bottom',
			texturePath: 'new_dropitem/dropitem_purple/dropitem_purple_bottom/'
		},
		{
			wav: 'effect/drop_purple'
		}
	],

	1240: [
		{
			//EF_DIGITAL_SPACE - Hat effect example (SPR)
			type: 'SPR',
			file: 'digital_space/digital_space',
			texturePath: 'digital_space/',
			attachedEntity: true,
			repeat: true,
			renderBeforeEntities: true
		}
	],

	1869: [
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_blue/dropitem_blue/dropitem_blue',
			texturePath: 'new_dropitem/dropitem_blue/dropitem_blue/'
		},
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_blue/dropitem_blue_bottom/dropitem_blue_bottom',
			texturePath: 'new_dropitem/dropitem_blue/dropitem_blue_bottom/'
		},
		{
			wav: 'effect/drop_blue'
		}
	],
	1870: [
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_green/dropitem_green/dropitem_green',
			texturePath: 'new_dropitem/dropitem_green/dropitem_green/'
		},
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_green/dropitem_green_bottom/dropitem_green_bottom',
			texturePath: 'new_dropitem/dropitem_green/dropitem_green_bottom/'
		},
		{
			wav: 'effect/drop_green'
		}
	],
	1871: [
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_red/dropitem_red/dropitem_red',
			texturePath: 'new_dropitem/dropitem_red/dropitem_red/'
		},
		{
			type: 'STR',
			file: 'new_dropitem/dropitem_red/dropitem_red_bottom/dropitem_red_bottom',
			texturePath: 'new_dropitem/dropitem_red/dropitem_red_bottom/'
		},
		{
			wav: 'effect/drop_red'
		}
	],
	1872: [
		{
			// EF_NEW_SUCCESS
			type: 'STR',
			file: 'grade_enchant/new_success/new_success',
			texturePath: 'grade_enchant/new_success/'
		}
	],
	1873: [
		{
			// EF_NEW_FAILURE
			type: 'STR',
			file: 'grade_enchant/new_failed/new_failed',
			texturePath: 'grade_enchant/new_failed/'
		}
	],
	1874: [
		{
			// EF_NEW_INTRO
			type: 'STR',
			file: 'grade_enchant/new_intro/new_intro',
			texturePath: 'grade_enchant/new_intro/'
		}
	],
	1875: [
		{
			// EF_UI_ENCHANT_INTRO_YELLOW
			type: 'STR',
			file: 'ui_enchant/ui_intro_yellow/ui_intro_yellow',
			texturePath: 'ui_enchant/ui_intro_yellow/'
		}
	],
	1876: [
		{
			// EF_UI_ENCHANT_SUCCESS
			type: 'STR',
			file: 'ui_enchant/ui_enchant_success/ui_enchant_success',
			texturePath: 'ui_enchant/ui_enchant_success/'
		}
	],
	1877: [
		{
			// EF_UI_ENCHANT_FAIL
			type: 'STR',
			file: 'ui_enchant/ui_fail/ui_enchant_fail',
			texturePath: 'ui_enchant/ui_fail/'
		}
	],
	1878: [
		{
			// EF_UI_ENCHANT_INTRO_BLUE
			type: 'STR',
			file: 'ui_enchant/ui_intro_blue/ui_intro_blue',
			texturePath: 'ui_enchant/ui_intro_blue/'
		}
	],
	1879: [
		{
			// EF_UI_ENCHANT_UP_SUCCESS
			type: 'STR',
			file: 'ui_enchant/ui_levelup_success/ui_levelup_success',
			texturePath: 'ui_enchant/ui_levelup_success/'
		}
	],
	1880: [
		{
			// EF_UI_ENCHANT_UP_FAIL
			type: 'STR',
			file: 'ui_enchant/ui_fail/ui_levelup_fail',
			texturePath: 'ui_enchant/ui_fail/'
		}
	],
	1881: [
		{
			// EF_UI_ENCHANT_INTRO_GREEN
			type: 'STR',
			file: 'ui_enchant/ui_intro_green/ui_intro_green',
			texturePath: 'ui_enchant/ui_intro_green/'
		}
	],
	1882: [
		{
			// EF_UI_ENCHANT_RESET_SUCCESS
			type: 'STR',
			file: 'ui_enchant/ui_reset_success/ui_reset_success',
			texturePath: 'ui_enchant/ui_reset_success/'
		}
	],
	1883: [
		{
			// EF_UI_ENCHANT_RESET_FAIL
			type: 'STR',
			file: 'ui_enchant/ui_fail/ui_reset_fail',
			texturePath: 'ui_enchant/ui_fail/'
		}
	],

	maximize_power_sounds: [
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function MaximizePowerSounds(Params) {
				Events.setTimeout(function () {
					SoundManager.play('effect/black_maximize_power_circle.wav');
				}, 1);
				Events.setTimeout(function () {
					SoundManager.play('effect/black_maximize_power_sword.wav');
				}, 550);
				Events.setTimeout(function () {
					SoundManager.play('effect/black_maximize_power_sword.wav');
				}, 700);
				Events.setTimeout(function () {
					SoundManager.play('effect/black_maximize_power_sword_bic.wav');
				}, 950);
			}
		}
	],

	white_pulse: [
		{
			//type: 'FUNC',
			attachedEntity: true
		}
	],

	yellow_pulse: [
		{
			//type: 'FUNC',
			attachedEntity: true
		}
	],

	black_pulse: [
		{
			//type: 'FUNC',
			attachedEntity: true
		}
	],

	spear_hit_sound: [
		{
			wav: '_hit_spear',
			attachedEntity: true
		}
	],

	enemy_hit_normal1: [
		{
			wav: '_enemy_hit_normal1',
			attachedEntity: true
		}
	],

	ef_hit2_sound: [
		{
			wav: 'effect/ef_hit2',
			attachedEntity: true
		}
	],

	ef_greed_sound: [
		{
			wav: 'effect/ef_entry',
			attachedEntity: true
		}
	],

	ef_rush_windmill: [
		{
			wav: 'effect/\xc7\xb3\xc2\xf7\xb8\xa6\xc7\xe2\xc7\xd8\xb5\xb9\xb0\xdd', //ÇłÂ÷¸¦ÇâÇŘµą°Ý
			attachedEntity: true
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			delayLate: 500,
			duplicate: 7,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			red: 0.5,
			green: 0.01,
			blue: 0.9,
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 0.75,
			attachedEntity: true,
			duration: 1000,
			delayOffset: 400,
			duplicate: 3,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			duplicate: 10,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init;

				entity.animations.add(function (tick) {
					if (tick < 500) {
						if (tick % 2 == 0) {
							entity._flashColor[0] = 1;
							entity._flashColor[1] = 1;
							entity._flashColor[2] = 1;
							entity._flashColor[3] = 0.4;
							entity.recalculateBlendingColor();
						} else {
							entity._flashColor[0] = 1;
							entity._flashColor[1] = 1;
							entity._flashColor[2] = 1;
							entity._flashColor[3] = 1;
							entity.recalculateBlendingColor();
						}
					} else {
						entity._flashColor[0] = 1;
						entity._flashColor[1] = 1;
						entity._flashColor[2] = 1;
						entity._flashColor[3] = 1;
						entity.recalculateBlendingColor();
						return true;
					}
				});
			}
		}
	],

	ef_swing_dance: [
		{
			wav: 'effect/\xbd\xba\xc0\xae\xb4\xed\xbd\xba',
			attachedEntity: true
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			delayLate: 500,
			duplicate: 7,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			red: 1,
			green: 0.1,
			blue: 0.5,
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 0.75,
			attachedEntity: true,
			duration: 1000,
			delayOffset: 400,
			duplicate: 3,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			alphaMax: 1,
			attachedEntity: true,
			duration: 1000,
			duplicate: 10,
			fadeOut: true,
			file: 'effect/ac_center2.tga',
			posxRand: 1.5,
			posyRand: 1,
			poszEndRand: 1,
			poszEndRandMiddle: 6,
			poszStartRand: 1,
			poszStartRandMiddle: 1,
			sizeRandY: 15,
			sizeRandYMiddle: 45,
			sizeX: 2.5,
			type: '3D',
			zIndex: 0
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function EffectBodyColor(Params) {
				const entity = Params.Init.ownerEntity;

				entity.animations.add(function (tick) {
					if (tick < 500) {
						if (tick % 2 == 0) {
							entity._flashColor[0] = 1;
							entity._flashColor[1] = 0.1;
							entity._flashColor[2] = 0.5;
							entity._flashColor[3] = 0.4;
							entity.recalculateBlendingColor();
						} else {
							entity._flashColor[0] = 1;
							entity._flashColor[1] = 1;
							entity._flashColor[2] = 1;
							entity._flashColor[3] = 1;
							entity.recalculateBlendingColor();
						}
					} else {
						entity._flashColor[0] = 1;
						entity._flashColor[1] = 1;
						entity._flashColor[2] = 1;
						entity._flashColor[3] = 1;
						entity.recalculateBlendingColor();
						return true;
					}
				});
			}
		}
	],

	ef_magicpower: [
		{
			wav: 'effect/\xb8\xb6\xb9\xfd\xb7\xc2\x20\xc1\xf5\xc6\xf8',
			attachedEntity: true
		}
	],

	ef_metalicsound: [
		{
			wav: 'effect/metalic',
			attachedEntity: true
		}
	],

	ef_ancilla: [
		{
			wav: 'effect/ab_ancilla',
			attachedEntity: true
		}
	],

	ef_thorntrap: [
		{
			wav: 'effect/\xb0\xa1\xbd\xc3\xb3\xaa\xb9\xab\xb5\xa3',
			attachedEntity: true
		}
	],

	ef_banishingpoint: [
		{
			wav: 'effect/lg_banishingpoint',
			attachedEntity: true
		}
	],

	ef_cannonspear: [
		{
			wav: 'effect/lg_cannonspear',
			attachedEntity: true
		}
	],

	ef_earthdrive: [
		{
			wav: 'effect/lg_earthdrive',
			attachedEntity: true
		}
	],

	ef_exceedbreak: [
		{
			wav: 'effect/lg_exceedbreak',
			attachedEntity: true
		}
	],

	ef_hesperuslit: [
		{
			wav: 'effect/lg_hesperuslit',
			attachedEntity: true
		}
	],

	ef_kings_grace: [
		{
			wav: 'effect/lg_kings_grace',
			attachedEntity: true
		}
	],

	ef_moonslasher: [
		{
			wav: 'effect/lg_moonslasher',
			attachedEntity: true
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				const duration = 500;
				const count = 8;
				const delay = duration / count;

				for (let i = 0; i < count; i++) {
					const delta = 1;

					Events.setTimeout(function () {
						entity.direction = Math.floor(entity.direction + delta) % 8;
					}, delay * i);
				}
			}
		}
	],

	ef_overbrand: [
		{
			wav: 'effect/lg_overbrand',
			attachedEntity: true
		}
	],

	ef_piety: [
		{
			wav: 'effect/lg_piety',
			attachedEntity: true
		}
	],

	ef_pinpointattack: [
		{
			wav: 'effect/lg_pinpointattack',
			attachedEntity: true
		}
	],

	ef_rageburst: [
		{
			wav: 'effect/lg_rageburst',
			attachedEntity: true
		}
	],

	ef_rayofgenesis: [
		{
			wav: 'effect/lg_rayofgenesis',
			attachedEntity: true
		}
	],

	ef_reflectdamage: [
		{
			wav: 'effect/lg_reflectdamage',
			attachedEntity: true
		}
	],

	ef_shieldspell: [
		{
			wav: 'effect/lg_shieldspell',
			attachedEntity: true
		}
	],

	ef_trample: [
		{
			wav: 'effect/lg_trample',
			attachedEntity: true
		}
	],

	ef_crescentelbow: [
		{
			wav: 'effect/sr_crescentelbow',
			attachedEntity: true
		}
	],

	ef_cursedcircle: [
		{
			wav: 'effect/sr_cursedcircle',
			attachedEntity: true
		}
	],

	ef_dragoncombo: [
		{
			wav: 'effect/sr_dragoncombo',
			attachedEntity: true
		}
	],

	ef_fallenempire: [
		{
			wav: 'effect/sr_fallenempire',
			attachedEntity: true
		}
	],

	ef_gateofhell: [
		{
			wav: 'effect/sr_gateofhell',
			attachedEntity: true
		}
	],

	ef_howlingoflion: [
		{
			wav: 'effect/sr_howlingoflion',
			attachedEntity: true
		}
	],

	ef_knucklearrow: [
		{
			wav: 'effect/sr_knucklearrow',
			attachedEntity: true
		}
	],

	ef_lightningwalk: [
		{
			wav: 'effect/sr_lightningwalk',
			attachedEntity: true
		}
	],

	ef_powervelocity: [
		{
			wav: 'effect/sr_powervelocity',
			attachedEntity: true
		}
	],

	ef_raisingdragon: [
		{
			wav: 'effect/sr_raisingdragon',
			attachedEntity: true
		}
	],

	ef_rampageblaster: [
		{
			wav: 'effect/sr_rampageblaster',
			attachedEntity: true
		}
	],

	ef_rideinlightning: [
		{
			wav: 'effect/sr_rideinlightning',
			attachedEntity: true
		}
	],

	ef_skynetblow: [
		{
			wav: 'effect/sr_skynetblow',
			attachedEntity: true
		}
	],

	ef_tigercannon: [
		{
			wav: 'effect/sr_tigercannon',
			attachedEntity: true
		}
	],

	ef_windmill: [
		{
			wav: 'effect/sr_windmill',
			attachedEntity: true
		},
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				const duration = 500;
				const count = 8;
				const delay = duration / count;

				for (let i = 0; i < count; i++) {
					const delta = 1;

					Events.setTimeout(function () {
						entity.direction = Math.floor(entity.direction + delta) % 8;
					}, delay * i);
				}
			}
		}
	],

	ef_jackfrost: [
		{
			wav: 'effect/wl_jackfrost',
			attachedEntity: true
		}
	],

	ef_siennaexecrate: [
		{
			wav: 'effect/wl_siennaexecrate',
			attachedEntity: true
		}
	],

	ef_frigg_song: [
		{
			wav: 'effect/wm_frigg_song',
			attachedEntity: true
		}
	],

	ef_duplelight: [
		{
			wav: 'effect/ab_duplelight',
			attachedEntity: true
		}
	],

	ef_wugbite: [
		{
			wav: 'wug_bite',
			attachedEntity: true
		}
	],

	ef_wugstrike: [
		{
			wav: 'wug_strike',
			attachedEntity: true
		}
	],

	ef_wugrider: [
		{
			wav: 'wolf_stand',
			attachedEntity: true
		}
	],

	ef_dragonbreath_water: [
		{
			type: 'SPR',
			file: 'rk_dragonbreath_water',
			wav: 'dragon_breath',
			attachedEntity: true
		}
	],

	ef_hallucinationwalk: [
		{
			wav: 'effect/hallucinationwalk',
			attachedEntity: true
		}
	],

	ef_s_storm: [
		{
			type: 'STR',
			file: 'S_STORM',
			wav: 'effect/RL_S_STORM',
			attachedEntity: true
		}
	],

	ef_firebreath: [
		{
			wav: 'effect/ef_firewall',
			attachedEntity: true
		}
	],

	ef_moonlit_serenade: [
		{
			wav: 'effect/\xb4\xde\xba\xfb\xbc\xbc\xb7\xb9\xb3\xaa\xb5\xa5',
			attachedEntity: true
		}
	],

	ef_sound_of_destruction: [
		{
			wav: 'effect/\xbb\xe7\xbf\xee\xb5\xe5\xbf\xc0\xba\xea\xb5\xf0\xbd\xba\xc6\xae\xb7\xb0\xbc\xc7',
			attachedEntity: true
		}
	],

	ef_great_echo: [
		{
			wav: 'effect/\xb1\xd7\xb7\xb9\xc0\xcc\xc6\xae\xbf\xa1\xc4\xda',
			attachedEntity: true
		}
	],

	ef_valley_of_death: [
		{
			wav: 'effect/\xbb\xe7\xb8\xc1\xc0\xc7\xb0\xf1\xc2\xa5\xb1\xe2\xbf\xa1\xbc\xad',
			attachedEntity: true
		}
	],

	ef_harmonize: [
		{
			wav: 'effect/\xc7\xcf\xb8\xf0\xb3\xaa\xc0\xcc\xc1\xee',
			attachedEntity: true
		}
	],

	ef_echo_song: [
		{
			wav: 'effect/\xb8\xde\xbe\xc6\xb8\xae\xc0\xc7\xb3\xeb\xb7\xa1',
			attachedEntity: true
		}
	],

	ef_symphony_of_lovers: [
		{
			wav: 'effect/\xbf\xac\xc0\xce\xb5\xe9\xc0\xbb\xc0\xa7\xc7\xd1\xbd\xc9\xc6\xf7\xb4\xcf',
			attachedEntity: true
		}
	],

	ef_arrow_projectile: [
		{
			// effect to show the arrow projectile (normal attack or skill with arrow projectile)
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			absoluteSpriteName: 'data/sprite/npc/skel_archer_arrow',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_arrow_shower_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: false,
			duration: 140,
			duplicate: 10,
			timeBetweenDupli: 0,
			fadeIn: true,
			fadeOut: true,
			absoluteSpriteName: 'data/sprite/npc/skel_archer_arrow',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1,
			posxEndRand: 1.5,
			posyEndRand: 1.5
		}
	],

	ef_spear_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '\xc3\xa2',
			playSprite: true,
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_shield_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/shield_boomerang.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 1,
			size: 50,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_tomahawk_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			file: 'effect/\xc5\xe4\xb8\xb6.bmp',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			rotate: true,
			nbOfRotation: 1,
			size: 50,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_parasite_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/parasite_bullet',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_stone_shooter_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/stone_shooter_bullet',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_wootan_shooter_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/wootan_shooter_bullet',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_peach_tree_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/peach_tree_bullet',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_hermit_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/ginseng_bullet',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_canon_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/canon_bullet',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_canon1_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/canon_bullet_1',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_canon2_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/canon_bullet_2',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_canon3_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/canon_bullet_3',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_tanee_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/lady_tanee_bullet',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_ragged_projectile: [
		{
			type: '3D',
			alphaMax: 1,
			angle: 180,
			attachedEntity: true,
			duration: 140,
			fadeIn: true,
			fadeOut: true,
			spriteName: '../npc/ragged_zombie_bullet',
			toSrc: true,
			rotateToTarget: true,
			rotateWithCamera: true,
			size: 100,
			zOffset: 1,
			zIndex: 1
		}
	],

	ef_mandragora_attack: [
		{
			type: 'SPR',
			file: '../npc/mandragora_atk',
			attachedEntity: false
		}
	],

	ef_hydra_attack: [
		{
			type: 'SPR',
			file: '../npc/hydra_atk',
			attachedEntity: false
		}
	],

	ef_odium_attack: [
		{
			type: 'SPR',
			file: '../npc/tha_odium_ef',
			attachedEntity: false
		}
	],

	ef_drosera_attack: [
		{
			type: 'SPR',
			file: '../npc/drosera_bullet',
			attachedEntity: false
		}
	],

	ef_mavka_attack: [
		{
			type: 'SPR',
			file: '../npc/mavka_bullet',
			attachedEntity: false
		}
	],

	ef_entweihen_attack: [
		{
			type: 'SPR',
			file: '../npc/entweihen_bullet',
			attachedEntity: false
		}
	],

	ef_c_marker1: [
		{
			type: 'STR',
			file: 'RL_C_MAKER/deffender',
			attachedEntity: true
		},
		{
			type: 'STR',
			file: 'RL_C_MAKER/cm',
			attachedEntity: true
		},
		{
			wav: 'effect/RL_C_MARKER'
		}
	],

	ef_c_marker2: [
		{
			type: '2D',
			file: 'effect/RL_C_MAKER/ome.bmp',
			blendMode: 2,
			alphaMax: 0.8,
			duration: 500,
			posz: 5,
			sizeStart: 50,
			sizeEnd: 100,
			attachedEntity: true
		},
		{
			type: '2D',
			file: 'effect/RL_C_MAKER/ome.bmp',
			blendMode: 2,
			alphaMax: 0.8,
			delayOffset: 500,
			duration: 500,
			posz: 5,
			sizeStart: 100,
			sizeEnd: 50,
			attachedEntity: true
		}
	],

	temporary_warlock_sphere: [
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const entity = Params.Init.ownerEntity;
				const spheres = [];
				if (entity.Summon1) {
					spheres.push(entity.Summon1);
				}
				if (entity.Summon2) {
					spheres.push(entity.Summon2);
				}
				if (entity.Summon3) {
					spheres.push(entity.Summon3);
				}
				if (entity.Summon4) {
					spheres.push(entity.Summon4);
				}
				if (entity.Summon5) {
					spheres.push(entity.Summon5);
				}

				const wl_spheres = new WarlockSphere(entity, spheres);

				this.add(wl_spheres, Params);
			}
		}
	],

	quake: [
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				Camera.setQuake(start);
			}
		}
	],

	quake_magnum: [
		{
			type: 'FUNC',
			attachedEntity: true,
			func: function (Params) {
				const start = Params.Inst.startTick;
				const duration = 50;
				Camera.setQuake(start, duration);
			}
		}
	],

	ef_oldtrap_default: [
		{
			type: 'SPR',
			file: '\xb1\xe2\xba\xbb\xc6\xae\xb7\xa6',
			attachedEntity: true
		}
	],

	ef_oldtrap_yellow: [
		{
			type: 'SPR',
			file: '\xb4\xe3\xc8\xb2\xbb\xf6\xc6\xae\xb7\xa6',
			attachedEntity: true
		}
	],

	ef_oldtrap_blue: [
		{
			type: 'SPR',
			file: '\xbe\xcf\xc3\xbb\xbb\xf6\xc6\xae\xb7\xa6',
			attachedEntity: true
		}
	],

	ef_oldtrap_red: [
		{
			type: 'SPR',
			file: '\xc0\xda\xc8\xab\xbb\xf6\xc6\xae\xb7\xa6',
			attachedEntity: true
		}
	],

	ef_oldtrap_green: [
		{
			type: 'SPR',
			file: '\xc3\xbb\xb7\xcf\xbb\xf6\xc6\xae\xb7\xa6',
			attachedEntity: true
		}
	],

	ef_trap_01: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa601',
			attachedEntity: true
		}
	],

	ef_trap_02: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa602',
			attachedEntity: true
		}
	],

	ef_trap_03: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa603',
			attachedEntity: true
		}
	],

	ef_trap_03_2: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa603_2',
			attachedEntity: true
		}
	],

	ef_trap_03_3: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa603_3',
			attachedEntity: true
		}
	],

	ef_trap_03_4: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa603_4',
			attachedEntity: true
		}
	],

	ef_trap_03_5: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa603_5',
			attachedEntity: true
		}
	],

	ef_trap_03_6: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa603_6',
			attachedEntity: true
		}
	],

	ef_trap_04: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa604',
			attachedEntity: true
		}
	],

	ef_trap_05: [
		{
			type: 'RSM',
			file: '\xbf\xdc\xba\xce\xbc\xd2\xc7\xb0\x5c\xc6\xae\xb7\xa605',
			attachedEntity: true
		}
	],

	ef_trap_3_thorn: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xb0\xa1\xbd\xc301',
			attachedEntity: true
		}
	],

	ef_trap_3_cobalt: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xba\xaf\xbc\xf601',
			attachedEntity: true
		}
	],

	ef_trap_3_maze: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xba\xaf\xc1\xf601',
			attachedEntity: true
		}
	],

	ef_trap_3_verdure: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xba\xaf\xc7\xb301',
			attachedEntity: true
		}
	],

	ef_trap_3_magenta: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xba\xaf\xc8\xad01',
			attachedEntity: true
		}
	],

	ef_trap_3_ice: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xbc\xf601',
			attachedEntity: true
		}
	],

	ef_trap_3_cluster: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xc1\xf601',
			attachedEntity: true
		}
	],

	ef_trap_3_shock: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xc7\xb301',
			attachedEntity: true
		}
	],

	ef_trap_3_fire: [
		{
			type: 'RSM',
			file: 'event/3\xc2\xf7\xc6\xae\xb7\xa6_\xc8\xad01',
			attachedEntity: true
		}
	],

	ef_: [
		{
			wav: 'effect/',
			attachedEntity: true
		}
	]
};
