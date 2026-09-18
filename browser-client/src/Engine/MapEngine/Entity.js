/**
 * Engine/MapEngine/Entity.js
 *
 * Manage Entity based on received packets from server
 *
 * @author Vincent Thibault
 */

/**
 * Load dependencies
 */
import DB from 'DB/DBManager.js';
import SkillId from 'DB/Skills/SkillConst.js';
import SkillInfo from 'DB/Skills/SkillInfo.js';
import StatusConst from 'DB/Status/StatusConst.js';
import StatusState from 'DB/Status/StatusState.js';
import Emotions from 'DB/Emotions.js';
import SkillEffect from 'DB/Skills/SkillEffect.js';
import SkillActionTable from 'DB/Skills/SkillAction.js';
import EffectConst from 'DB/Effects/EffectConst.js';
import PetMessageConst from 'DB/Pets/PetMessageConst.js';
import JobId from 'DB/Jobs/JobConst.js';
import AttackEffect from 'DB/Monsters/AttackEffectTable.js';
import Sound from 'Audio/SoundManager.js';
import Events from 'Core/Events.js';
import Guild from 'Engine/MapEngine/Guild.js';
import Session from 'Engine/SessionStorage.js';
import Network from 'Network/NetworkManager.js';
import PACKETVER from 'Network/PacketVerManager.js';
import PACKET from 'Network/PacketStructure.js';
import Altitude from 'Renderer/Map/Altitude.js';
import Renderer from 'Renderer/Renderer.js';
import EntityManager from 'Renderer/EntityManager.js';
import Entity from 'Renderer/Entity/Entity.js';
import EffectManager from 'Renderer/EffectManager.js';
import Damage from 'Renderer/Effects/Damage.js';
import MagicTarget from 'Renderer/Effects/MagicTarget.js';
import LockOnTarget from 'Renderer/Effects/LockOnTarget.js';
import MagicRing from 'Renderer/Effects/MagicRing.js';
import BasicInfo from 'UI/Components/BasicInfo/BasicInfo.js';
import ChatBox from 'UI/Components/ChatBox/ChatBox.js';
import ChatRoom from 'UI/Components/ChatRoom/ChatRoom.js';
import Escape from 'UI/Components/Escape/Escape.js';
import HomunInformations from 'UI/Components/HomunInformations/HomunInformations.js';
import MercenaryInformations from 'UI/Components/MercenaryInformations/MercenaryInformations.js';
import Inventory from 'UI/Components/Inventory/Inventory.js';
import ShortCut from 'UI/Components/ShortCut/ShortCut.js';
import StatusIcons from 'UI/Components/StatusIcons/StatusIcons.js';
import MiniMap from 'UI/Components/MiniMap/MiniMap.js';
import PartyFriends from 'UI/Components/PartyFriends/PartyFriends.js';
import Equipment from 'UI/Components/Equipment/Equipment.js';
import ScreenEffectManager from 'Renderer/ScreenEffectManager.js';

// Excludes for skill name display
const SkillNameDisplayExclude = [
	//Hiding skills
	SkillId.TF_HIDING,
	SkillId.AS_CLOAKING,
	SkillId.ST_CHASEWALK,
	SkillId.GC_CLOAKINGEXCEED,
	SkillId.RA_CAMOUFLAGE,
	SkillId.NC_STEALTHFIELD,
	SkillId.SC_SHADOWFORM,
	SkillId.SC_INVISIBILITY,
	SkillId.KO_YAMIKUMO,

	//Talking
	SkillId.BA_FROSTJOKE,
	SkillId.DC_SCREAM,

	//3rd job extra skills
	SkillId.LG_OVERBRAND_BRANDISH,
	SkillId.LG_OVERBRAND_PLUSATK,
	SkillId.WM_REVERBERATION_MELEE,
	SkillId.WM_REVERBERATION_MAGIC,
	SkillId.WL_TETRAVORTEX_FIRE,
	SkillId.WL_TETRAVORTEX_WATER,
	SkillId.WL_TETRAVORTEX_WIND,
	SkillId.WL_TETRAVORTEX_GROUND,
	SkillId.WL_SUMMON_ATK_FIRE,
	SkillId.WL_SUMMON_ATK_WIND,
	SkillId.WL_SUMMON_ATK_WATER,
	SkillId.WL_SUMMON_ATK_GROUND
];

// Skills that display blue crit like combo damage
const SkillBlueCombo = [
	SkillId.TK_STORMKICK,
	SkillId.TK_DOWNKICK,
	SkillId.TK_TURNKICK,
	SkillId.TK_COUNTER,
	SkillId.TK_JUMPKICK,
	SkillId.SR_RAMPAGEBLASTER
];

const C_MULTIHIT_DELAY = 200; // PLUSATTACKED_MOTIONTIME
const C_DEATH_SYNC_OFFSET = 200; // extra ms after the hit before the death animation
const AVG_ATTACK_SPEED = 432;
const AVG_ATTACKED_SPEED = 288;
const MAX_ATTACKMT = AVG_ATTACK_SPEED * 2;

/**
 * List of players and the respective clan emblem
 */
const clanEmblems = {};

/**
 * Spam an entity on the map
 * Generic packet handler
 */
function onEntitySpam(pkt) {
	let entity = EntityManager.get(pkt.GID);

	if (entity) {
		entity.set(pkt);
	} else {
		entity = new Entity();
		entity.set(pkt);
		if (pkt.job == 45) {
			const EF_Init_Par = {
				ownerAID: entity.GID,
				position: entity.position
			};

			if (PACKETVER.value < 20030715) {
				EF_Init_Par.effectId = EffectConst.EF_WARPZONE;
				EffectManager.spam(EF_Init_Par);
			} else {
				EF_Init_Par.effectId = EffectConst.EF_WARPZONE2;
				EffectManager.spam(EF_Init_Par);
			}
		}
		EntityManager.add(entity);
		const cachedLife = EntityManager.getLife(entity.GID);
		if (cachedLife && entity.life.hp <= -1) {
			if (cachedLife.hp !== undefined) entity.life.hp = cachedLife.hp;
			if (cachedLife.hp_max !== undefined) entity.life.hp_max = cachedLife.hp_max;
			if (cachedLife.sp !== undefined) entity.life.sp = cachedLife.sp;
			if (cachedLife.sp_max !== undefined) entity.life.sp_max = cachedLife.sp_max;
			if (cachedLife.hunger !== undefined) entity.life.hunger = cachedLife.hunger;
			if (cachedLife.hunger_max !== undefined) entity.life.hunger_max = cachedLife.hunger_max;
			if (entity.life.hp > -1 && entity.life.hp_max > -1) {
				entity.life.update();
				entity.life.display = true;
			}
		}
	}

	if (pkt.effectState & StatusState.EffectState.FALCON && DB.isHunter(pkt.job)) {
		if (!entity.falcon) {
			entity.falcon = new Entity();
		}

		entity.falcon.set({
			objecttype: entity.falcon.constructor.TYPE_FALCON,
			GID: entity.GID + '_FALCON',
			PosDir: [entity.position[0], entity.position[1], 0],
			job: entity._job + '_FALCON',
			speed: Math.max(entity.walk.speed - 50, 1),
			name: '',
			hp: -1,
			maxhp: -1,
			hideShadow: true
		});
		EntityManager.add(entity.falcon);
	}
	if (pkt.effectState & StatusState.EffectState.WUG) {
		if (!entity.wug) {
			entity.wug = new Entity();
		}

		entity.wug.set({
			objecttype: entity.wug.constructor.TYPE_WUG,
			GID: entity.GID + '_WUG',
			PosDir: [entity.position[0], entity.position[1], 0],
			job: entity._job + '_WUG',
			speed: Math.max(entity.walk.speed - 50, 1),
			name: '',
			hp: -1,
			maxhp: -1
		});
		EntityManager.add(entity.wug);
	}

	if (entity.objecttype === Entity.TYPE_PC) {
		// don't know why switch from katar to sword, knife server put it on the left hand instead of right hand first.
		// so we have to swap it. maybe have a better solution.
		if (pkt.weapon == 0 && pkt.shield != 0 && !DB.isShield(pkt.shield)) {
			pkt.weapon = pkt.shield;
			pkt.shield = 0;
		}

		const weaponType = DB.getWeaponType(pkt.weapon, true);
		const viewId = DB.getWeaponViewID(pkt.weapon);
		if (DB.isAssassin(entity.job) && pkt.shield !== 0 && !DB.isShield(pkt.shield) && pkt.weapon !== 0) {
			const secondaryWeaponType = DB.getWeaponType(pkt.shield, true);
			entity.weapon = DB.mountWeapon(weaponType, secondaryWeaponType);
			entity.shield = 0;
		} else if (DB.isKatar(weaponType)) {
			entity.weapon = viewId;
			entity.shield = viewId;
		} else {
			entity.weapon = viewId;
			entity.shield = pkt.shield;
		}
		if (PACKETVER.value > 20150513) {
			if (entity.body !== pkt.body) {
				entity.body = pkt.body;
			}
		}
	}

	if (
		entity.objecttype === Entity.TYPE_PC &&
		!(entity._effectState & StatusState.EffectState.INVISIBLE) &&
		(pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY ||
			pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY2 ||
			pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY3 ||
			pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY4 ||
			pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY5 ||
			pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY6 ||
			pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY7 ||
			pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY10 ||
			pkt instanceof PACKET.ZC.NOTIFY_NEWENTRY11)
	) {
		const EF_Init_Par = {
			ownerAID: entity.GID,
			position: entity.position
		};

		if (PACKETVER.value < 20090922) {
			EF_Init_Par.effectId = EffectConst.EF_ENTRY;
			EffectManager.spam(EF_Init_Par);
		} else {
			EF_Init_Par.effectId = EffectConst.EF_ENTRY2;
			EffectManager.spam(EF_Init_Par);
		}
	}

	if (entity.objecttype === Entity.TYPE_HOM && pkt.GID === Session.homunId) {
		HomunInformations.startAI();
	}

	if (entity.objecttype === Entity.TYPE_MERC && pkt.GID === Session.mercId) {
		MercenaryInformations.startAI();
	}

	if (entity.objecttype === Entity.TYPE_MOB && pkt.isBoss) {
		entity.mobtype = pkt.isBoss;
	}

	// if it is listed in clanEmblems set emblem
	if (entity.GID in clanEmblems) {
		const clanId = clanEmblems[entity.GID];
		DB.loadClanEmblem(clanId, image => {
			entity.clanId = clanId;
			entity.setEntityGuildEmblem(image);
			updateEntityStyle(entity);
		});

		// remove from clanEmblems
		delete clanEmblems[entity.GID];
	}

	if (entity.GUID) {
		Guild.requestGuildEmblem(entity.GUID, entity.GEmblemVer, (image, gif) => {
			entity.setEntityGuildEmblem(image, gif);
			if (Session.mapState.isSiege && entity.GUID !== Session.Entity.GUID) {
				entity.emblem.display = true;
			}
		});
	}

	// load others aura
	entity.aura.load(EffectManager);
}

/**
 * Remove an entity from the map
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_VANISH
 */
function onEntityVanish(pkt) {
	const entity = EntityManager.get(pkt.GID);
	if (entity) {
		if (entity.objecttype === Entity.TYPE_PC && pkt.GID === Session.Entity.GID) {
			//death animation only for myself
			const EF_Init_Par = {
				effectId: EffectConst.EF_DEVIL,
				ownerAID: entity.GID
			};

			EffectManager.spam(EF_Init_Par);
		}

		if (entity.objecttype === Entity.TYPE_HOM && pkt.GID === Session.homunId) {
			HomunInformations.stopAI();
		}

		if (entity.objecttype === Entity.TYPE_MERC && pkt.GID === Session.mercId) {
			MercenaryInformations.stopAI();
		}

		EffectManager.remove(null, pkt.GID, [
			EffectConst.EF_CHOOKGI,
			EffectConst.EF_CHOOKGI2,
			EffectConst.EF_CHOOKGI3,
			EffectConst.EF_CHOOKGI_N
		]); // Spirit spheres
		EffectManager.remove(null, pkt.GID, [
			EffectConst.EF_CHOOKGI_FIRE,
			EffectConst.EF_CHOOKGI_WIND,
			EffectConst.EF_CHOOKGI_WATER,
			EffectConst.EF_CHOOKGI_GROUND,
			'temporary_warlock_sphere'
		]); // Elemental spheres (Warlock)

		switch (pkt.type) {
			case Entity.VT.EXIT:
			case Entity.VT.TELEPORT:
				if (!(entity._effectState & StatusState.EffectState.INVISIBLE)) {
					const EF_Init_Par = { position: entity.position };

					if (PACKETVER.value < 20030715) {
						EF_Init_Par.effectId = EffectConst.EF_TELEPORTATION;
					} else {
						EF_Init_Par.effectId = EffectConst.EF_TELEPORTATION2;
					}
					EffectManager.spam(EF_Init_Par);
				}
				if (entity.falcon) {
					entity.falcon.remove(pkt.type);
					entity.falcon = null;
				}
				if (entity.wug) {
					entity.wug.remove(pkt.type);
					entity.wug = null;
				}

			case Entity.VT.OUTOFSIGHT:
				EffectManager.remove(null, pkt.GID, null);
				if (entity.falcon) {
					entity.falcon.remove(pkt.type);
					entity.falcon = null;
				}
				if (entity.wug) {
					entity.wug.remove(pkt.type);
					entity.wug = null;
				}

			case Entity.VT.DEAD:
				// remove aura on non-PC death
				if (entity.objecttype !== Entity.TYPE_PC) {
					entity.aura.remove(EffectManager);
				}
				if (pkt.type === Entity.VT.DEAD) {
					EntityManager.removeLife(pkt.GID);
				}
		}

		// Show escape menu
		if (pkt.GID === Session.Entity.GID && pkt.type === 1) {
			Escape.showDeathMenu(haveSiegfriedItem());
		}

		// Sync the death animation to the killer attack's impact tick so it
		// lands with the hit instead of playing instantly. Only applies to a
		// VT.DEAD vanish for a non-PC entity whose death is driven by a recent
		// damaging attack; all other vanish types (exit/teleport/out-of-sight)
		// and stale/zero sync ticks fall through to an immediate removal.
		const isSyncedDeath =
			pkt.type === Entity.VT.DEAD &&
			entity.objecttype !== Entity.TYPE_PC &&
			entity._deathSyncTick > Renderer.tick;

		const deathDelay = isSyncedDeath ? entity._deathSyncTick - Renderer.tick + C_DEATH_SYNC_OFFSET : 0;

		// Free the GID immediately so it can be reused; removeGID only drops the
		// lookup entry and keeps the entity in the render list, so the death /
		// fade-out animation continues independently. Deferring removeGID would
		// leave the GID mapped to a dying entity and let a reused GID collide.
		EntityManager.removeGID(pkt.GID);

		const playDeath = () => {
			entity.remove(pkt.type);
		};

		if (deathDelay > 0) {
			entity._deathSyncTick = 0;
			Events.setTimeout(playDeath, deathDelay);
		} else {
			playDeath();
		}
	}
}

/**
 * An entity start walking
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_MOVE
 */
function onEntityMove(pkt) {
	const entity = EntityManager.get(pkt.GID);
	if (entity) {
		entity.walkTo(
			pkt.MoveData[0],
			pkt.MoveData[1],
			pkt.MoveData[2],
			pkt.MoveData[3],
			undefined,
			pkt.moveStartTime,
			pkt.moveServerEndTime || pkt.moveEndTime
		);
	}
}

/**
 * Entity stop walking
 *
 * @param {object} pkt - PACKET.ZC.STOPMOVE
 */
function onEntityStopMove(pkt) {
	const entity = EntityManager.get(pkt.AID);
	if (entity) {
		if (entity.action === entity.ACTION.WALK) {
			entity.setAction({
				action: entity.ACTION.IDLE,
				frame: 0,
				repeat: true,
				play: true
			});
		}

		entity.resetRoute();
		entity.position[0] = pkt.xPos;
		entity.position[1] = pkt.yPos;
		entity.position[2] = Altitude.getCellHeight(pkt.xPos, pkt.yPos);
	}
}

/**
 * Move entity to a point
 *
 * @param {object} pkt - PACKET_ZC_HIGHJUMP
 */
function onEntityJump(pkt) {
	const entity = EntityManager.get(pkt.AID);
	if (entity) {
		entity.resetRoute();
		entity.position[0] = pkt.xPos;
		entity.position[1] = pkt.yPos;
		entity.position[2] = Altitude.getCellHeight(pkt.xPos, pkt.yPos);
	}
}

/**
 * Fast relocation packet support (e.g. Body Relocation, Fallen Angel)
 *
 * @param {object} pkt - PACKET.ZC.FASTMOVE
 */
function onEntityFastMove(pkt) {
	const entity = EntityManager.get(pkt.AID);
	if (entity && entity.fastMoveTo(pkt.targetXpos, pkt.targetYpos, 15, null, false)) {
		if (entity.objecttype === entity.constructor.TYPE_PC) {
			if (DB.isMonk(entity.job)) {
				entity._fastMoveTrail = true;
				entity.setAction({
					action: entity.ACTION.ATTACK,
					frame: 0,
					repeat: false,
					play: false
				});
			} else if (DB.isGunslinger(entity.job)) {
				entity._fastMoveTrail = true;
				entity.setAction({
					action: entity.ACTION.SKILL,
					frame: 0,
					repeat: false,
					play: false
				});
			}
		}
	}
}

/**
 * Perform Entity Action with forced position relocation (knockback / slide)
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_ACT_POSITION
 */
function onEntityActionPosition(pkt) {
	if (typeof pkt.xPos === 'number' && typeof pkt.yPos === 'number' && (pkt.xPos !== 0 || pkt.yPos !== 0)) {
		const targetEntity = EntityManager.get(pkt.targetGID);
		if (targetEntity) {
			targetEntity.fastMoveTo(pkt.xPos, pkt.yPos, 20, null, true);
		}
	}

	const srcEntity = EntityManager.get(pkt.GID);
	const attackSpeed =
		srcEntity && typeof srcEntity.attack_speed === 'number' && srcEntity.attack_speed > 0
			? srcEntity.attack_speed
			: AVG_ATTACK_SPEED;

	const normalizedPkt = {
		...pkt,
		attackMT: typeof pkt.attackMT === 'number' && pkt.attackMT > 0 ? pkt.attackMT : attackSpeed,
		attackedMT: typeof pkt.attackedMT === 'number' && pkt.attackedMT > 0 ? pkt.attackedMT : AVG_ATTACKED_SPEED,
		leftDamage: typeof pkt.leftDamage === 'number' ? pkt.leftDamage : 0,
		count: typeof pkt.count === 'number' && pkt.count > 0 ? pkt.count : 1
	};

	onEntityAction(normalizedPkt);
}

/**
 * Display entity's emotion
 *
 * @param {object} pkt - PACKET.ZC.EMOTION
 */
function onEntityEmotion(pkt) {
	const entity = EntityManager.get(pkt.GID);
	if (entity && pkt.type in Emotions.indexes) {
		entity.attachments.add({
			frame: Emotions.indexes[pkt.type],
			file: 'emotion',
			play: true,
			head: true,
			depth: 5.0
		});
	}
}

/**
 * Resurect an entity
 *
 * @param {object} pkt - PACKET_ZC_RESURRECTION
 */
function onEntityResurect(pkt) {
	const entity = EntityManager.get(pkt.AID);

	if (!entity) {
		return;
	}

	// There is always a packet to use the skill "Resurection"
	// on yourself after, but what if this packet isn't here ?
	// The entity will stay die ? So update the action just in case
	entity.setAction({
		action: entity.ACTION.IDLE,
		frame: 0,
		repeat: true,
		play: true
	});

	if (entity.wug) {
		entity.wug.setAction({
			action: entity.ACTION.IDLE,
			frame: 0,
			repeat: true,
			play: true
		});
	}

	// If it's our main character update Escape ui
	if (entity === Session.Entity) {
		Escape.resetMenu();
	}
}

/**
 * Perform Entity Action
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_ACT
 */
function onEntityAction(pkt) {
	const srcEntity = EntityManager.get(pkt.GID);
	// Entity out of the screen ?
	if (!srcEntity) {
		return;
	}
	const dstEntity = EntityManager.get(pkt.targetGID);
	let target;
	const srcWeapon = srcEntity.weapon ? srcEntity.weapon : 0;
	const srcWeaponLeft = srcEntity.shield ? srcEntity.shield : 0;

	srcEntity.targetGID = pkt.targetGID;

	switch (pkt.action) {
		// Damage
		case 0: // regular [DMG_NORMAL]
		//case 1: // [DMG_PICKUP_ITEM]
		//case 2: // [DMG_SIT_DOWN]
		//case 3: // [DMG_STAND_UP]
		case 4: // absorbed [DMG_ENDURE]
		//case 5: [DMG_SPLASH]
		//case 5: [DMG_SKILL]
		//case 7: [DMG_REPEAT]
		//case 11: [DMG_TOUCH] probably something new.
		case 8: // double attack [DMG_MULTI_HIT]
		case 9: // endure [DMG_MULTI_HIT_ENDURE]
		case 10: // critital [DMG_CRITICAL]
		case 11: // lucky
		case 13: {
			const attackMT =
				typeof pkt.attackMT === 'number' && pkt.attackMT > 0
					? pkt.attackMT
					: (srcEntity && srcEntity.attack_speed) || AVG_ATTACK_SPEED;
			const baseAttackMT = Math.min(attackMT, MAX_ATTACKMT);
			pkt.attackMT = baseAttackMT;
			pkt.attackedMT =
				typeof pkt.attackedMT === 'number' && pkt.attackedMT > 0 ? pkt.attackedMT : AVG_ATTACKED_SPEED;
			pkt.leftDamage = typeof pkt.leftDamage === 'number' ? pkt.leftDamage : 0;
			pkt.count = typeof pkt.count === 'number' && pkt.count > 0 ? pkt.count : 1;

			srcEntity.attack_speed = baseAttackMT;

			let animSpeed = 0;
			let delayTime = pkt.attackMT;

			const _WSnd = DB.getWeaponSound(srcWeapon);
			/*var weaponSound = WSnd ? WSnd[0] : false;
				let weaponSoundRelease = WSnd ? WSnd[1] : false;*/ // UNUSED

			const _WSndL = DB.getWeaponSound(srcWeaponLeft);
			/*var weaponSoundLeft = WSndL ? WSndL[0] : false;
				let weaponSoundReleaseLeft = WSndL ? WSndL[1] : false;*/ // UNUSED

			if (srcEntity.objecttype === Entity.TYPE_PC) {
				const factorOfmotionSpeed = pkt.attackMT / AVG_ATTACK_SPEED;
				const isDualWeapon = DB.isDualWeapon(srcEntity._job, srcEntity._sex, srcEntity.weapon);
				const m_attackMotion = DB.getPCAttackMotion(
					srcEntity._job,
					srcEntity._sex,
					srcEntity.weapon,
					isDualWeapon
				);
				let m_motionSpeed = 1; // need to find out where is it come from? maybe from act delay with some calculate //actRes->GetDelay(action); [MrUnzO]
				m_motionSpeed *= factorOfmotionSpeed;

				const _soundTime = (delayTime = m_attackMotion * m_motionSpeed * 24.0);
				animSpeed = pkt.attackMT / m_attackMotion;

				// Display throw arrow effect when using bows, not an elegant conditional but it works.. [Waken]
				if (DB.isBow(DB.getWeaponType(srcEntity.weapon, true))) {
					delayTime = (m_attackMotion + 8 / m_motionSpeed) * m_motionSpeed * 24.0;
					pkt.attackMT += delayTime;
					const EF_Init_Par = {
						effectId: 'ef_arrow_projectile',
						ownerAID: dstEntity.GID,
						otherAID: srcEntity.GID,
						startTick: Renderer.tick + pkt.attackMT,
						otherPosition: srcEntity.position
					};
					EffectManager.spam(EF_Init_Par);
				}
			} else if (srcEntity.job in AttackEffect.PROJECTILE) {
				// Non player projectiles
				const EF_Init_Par = {
					effectId: AttackEffect.PROJECTILE[srcEntity.job],
					ownerAID: dstEntity.GID,
					otherAID: srcEntity.GID,
					startTick: Renderer.tick + pkt.attackMT,
					otherPosition: srcEntity.position
				};
				EffectManager.spam(EF_Init_Par);
			} else if (srcEntity.job in AttackEffect.SPAWN) {
				// Non player special ranged attack
				const EF_Init_Par = {
					effectId: AttackEffect.SPAWN[srcEntity.job],
					ownerAID: dstEntity.GID,
					otherAID: srcEntity.GID,
					startTick: Renderer.tick + pkt.attackMT,
					otherPosition: srcEntity.position
				};
				EffectManager.spam(EF_Init_Par);
			}

			if (dstEntity) {
				// only if damage and do not have endure
				// and damage isn't absorbed (healing)

				// Update entity position
				srcEntity.lookTo(dstEntity.position[0], dstEntity.position[1]);

				// Will be hit actions
				onEntityWillBeHitSub(pkt, dstEntity);

				// damage blocking status effect display
				if (pkt.action == 0 && pkt.damage == 0 && pkt.leftDamage == 0 && dstEntity.isGuard) {
					// Show guard effect (Kyrie Eleison, Auto Guard, Parrying, etc.)
					const EF_Init_Par = {
						effectId: EffectConst.EF_GUARD,
						ownerAID: pkt.targetGID,
						startTick: Renderer.tick + pkt.attackMT
					};
					EffectManager.spam(EF_Init_Par);
				}

				target = pkt.damage ? dstEntity : srcEntity;

				// damage or miss display
				if (
					dstEntity.objecttype === Entity.TYPE_MOB ||
					dstEntity.objecttype === Entity.TYPE_NPC_ABR ||
					dstEntity.objecttype === Entity.TYPE_NPC_BIONIC
				) {
					if (pkt.damage > 0) {
						const EF_Init_Par = {
							effectId: EffectConst.EF_HIT1,
							ownerAID: pkt.targetGID,
							startTick: Renderer.tick + pkt.attackMT
						};
						EffectManager.spam(EF_Init_Par);
					}
				}

				let type = null;
				switch (pkt.action) {
					// Single damage
					case 10: // critical
						type = Damage.TYPE.CRIT;
					case 0: // regular damage
					case 4: // regular damage (endure)
						Damage.add(pkt.damage, target, Renderer.tick + pkt.attackMT, srcWeapon, type);
						if (pkt.leftDamage) {
							Damage.add(
								pkt.leftDamage,
								target,
								Renderer.tick + pkt.attackMT + C_MULTIHIT_DELAY * 1.75,
								srcWeapon,
								type
							);
						}
						break;

					// Combo
					case 13: //multi-hit critical
						type = Damage.TYPE.CRIT;
					case 8: // multi-hit damage
					case 9: {
						// multi-hit damage (endure)
						// Display combo only if entity is mob and the attack don't miss
						if (
							(dstEntity.objecttype === Entity.TYPE_MOB ||
								dstEntity.objecttype === Entity.TYPE_NPC_ABR ||
								dstEntity.objecttype === Entity.TYPE_NPC_BIONIC) &&
							pkt.damage > 0
						) {
							if (pkt.damage > 1) {
								// Can't divide 1 damage
								Damage.add(
									pkt.damage / 2,
									dstEntity,
									Renderer.tick + pkt.attackMT,
									srcWeapon,
									Damage.TYPE.COMBO
								);
							}
							if (pkt.leftDamage) {
								Damage.add(
									pkt.damage,
									dstEntity,
									Renderer.tick + pkt.attackMT + C_MULTIHIT_DELAY / 2,
									srcWeapon,
									Damage.TYPE.COMBO
								);
								Damage.add(
									pkt.damage + pkt.leftDamage,
									dstEntity,
									Renderer.tick + pkt.attackMT + C_MULTIHIT_DELAY * 1.75,
									srcWeapon,
									Damage.TYPE.COMBO | Damage.TYPE.COMBO_FINAL
								);
							} else {
								Damage.add(
									pkt.damage,
									dstEntity,
									Renderer.tick + pkt.attackMT + C_MULTIHIT_DELAY,
									srcWeapon,
									Damage.TYPE.COMBO | Damage.TYPE.COMBO_FINAL
								);
							}
						}

						let div = 1;
						if (pkt.damage > 1) {
							// Can't divide 1 damage
							div = 2;
							Damage.add(pkt.damage / div, target, Renderer.tick + pkt.attackMT, srcWeapon, type);
						}
						if (pkt.leftDamage) {
							Damage.add(
								pkt.damage / div,
								target,
								Renderer.tick + pkt.attackMT + C_MULTIHIT_DELAY / 2,
								srcWeapon,
								type
							);
							Damage.add(
								pkt.leftDamage,
								target,
								Renderer.tick + pkt.attackMT + C_MULTIHIT_DELAY * 1.75,
								srcWeapon,
								type
							);
						} else {
							Damage.add(
								pkt.damage / div,
								target,
								Renderer.tick + pkt.attackMT + C_MULTIHIT_DELAY,
								srcWeapon,
								type
							);
						}
						break;
					}
					// TODO: lucky miss
					case 11:
						dstEntity.attachments.add({
							frame: 3,
							file: 'msg',
							uid: 'lucky',
							play: true,
							head: true,
							repeat: false
						});
						break;
				}
			}

			if (pkt.leftDamage) {
				// KAGEROU, OBORO does not use ATTCK3 for left
				const useATTACK =
					srcEntity.job == JobId.KAGEROU ||
					srcEntity.job == JobId.KAGEROU_B ||
					srcEntity.job == JobId.OBORO ||
					srcEntity.job == JobId.OBORO_B;
				srcEntity.setAction({
					action: useATTACK ? srcEntity.ACTION.ATTACK : srcEntity.ACTION.ATTACK3,
					frame: 0,
					repeat: false,
					play: true,
					next: {
						delay: Renderer.tick + pkt.attackMT + delayTime,
						action: srcEntity.ACTION.READYFIGHT,
						frame: 0,
						repeat: true,
						play: true,
						next: false
					}
				});
			} else {
				srcEntity.setAction({
					action: srcEntity.ACTION.ATTACK,
					speed: animSpeed,
					frame: 0,
					repeat: false,
					play: true,
					next: {
						delay: Renderer.tick + pkt.attackMT,
						action: srcEntity.ACTION.READYFIGHT,
						frame: 0,
						repeat: true,
						play: true,
						next: false
					}
				});
			}

			// Talk sometime
			if (
				Session.Entity &&
				srcEntity.GID === Session.Entity.GID &&
				Session.pet &&
				Session.pet.friendly > 900 &&
				(Session.pet.lastTalk || 0) + 10000 < Date.now()
			) {
				const talkRate = parseInt(Math.random() * 10);
				if (talkRate < 3) {
					const hunger = DB.getPetHungryState(Session.pet.oldHungry);
					const talk = DB.getPetTalkNumber(Session.pet.job, PetMessageConst.PM_HUNTING, hunger);

					const talkPkt = new PACKET.CZ.PET_ACT();
					talkPkt.data = talk;
					Network.sendPacket(talkPkt);
					Session.pet.lastTalk = Date.now();
				}
			}

			break;
		}
		// Pickup item
		case 1:
			srcEntity.setAction({
				action: srcEntity.ACTION.PICKUP,
				frame: 0,
				repeat: false,
				play: true,
				next: {
					action: srcEntity.ACTION.IDLE,
					frame: 0,
					repeat: true,
					play: true,
					next: false
				}
			});
			if (dstEntity) {
				srcEntity.lookTo(dstEntity.position[0], dstEntity.position[1]);
			}
			break;

		// Sit Down
		case 2:
			srcEntity.setAction({
				action: srcEntity.ACTION.SIT,
				frame: 0,
				repeat: true,
				play: true
			});
			break;

		// Stand up
		case 3:
			srcEntity.setAction({
				action: srcEntity.ACTION.IDLE,
				frame: 0,
				repeat: true,
				play: true
			});
			break;
	}

	if (pkt?.damage > 0) {
		if (Session.Entity && srcEntity.GID === Session.Entity.GID) {
			// I deal damage
			ChatBox.addText(
				DB.getMessage(1607).replace('%s', dstEntity.display.name).replace('%d', pkt.damage),
				ChatBox.TYPE.INFO,
				ChatBox.FILTER.BATTLE
			);
		} else if (Session.Entity && dstEntity.GID === Session.Entity.GID) {
			// I receive damage
			ChatBox.addText(
				DB.getMessage(1605).replace('%s', srcEntity.display.name).replace('%d', pkt.damage),
				ChatBox.TYPE.INFO,
				ChatBox.FILTER.BATTLE
			);
		} else if (
			srcEntity.GID === Session.homunId ||
			srcEntity.GID === Session.merId ||
			srcEntity.GID === Session.petId ||
			srcEntity.GID === Session.elemId
		) {
			// My buddy deals damage
			ChatBox.addText(
				DB.getMessage(1608)
					.replace('%s', srcEntity.display.name)
					.replace('%s', dstEntity.display.name)
					.replace('%d', pkt.damage),
				ChatBox.TYPE.INFO,
				ChatBox.FILTER.BATTLE
			);
		} else if (
			dstEntity.GID === Session.homunId ||
			dstEntity.GID === Session.merId ||
			dstEntity.GID === Session.petId ||
			dstEntity.GID === Session.elemId
		) {
			// My buddy receives damage
			ChatBox.addText(
				DB.getMessage(1606)
					.replace('%s', dstEntity.display.name)
					.replace('%s', srcEntity.display.name)
					.replace('%d', pkt.damage),
				ChatBox.TYPE.INFO,
				ChatBox.FILTER.BATTLE
			);
		} else if (PartyFriends.isGroupMember(srcEntity.display?.name)) {
			// Party member deals damage
			ChatBox.addText(
				DB.getMessage(1608)
					.replace('%s', srcEntity.display.name)
					.replace('%s', dstEntity.display.name)
					.replace('%d', pkt.damage),
				ChatBox.TYPE.INFO,
				ChatBox.FILTER.PARTY_BATTLE
			);
		} else if (PartyFriends.isGroupMember(dstEntity.display?.name)) {
			// Party member receives damage
			ChatBox.addText(
				DB.getMessage(1606)
					.replace('%s', dstEntity.display.name)
					.replace('%s', srcEntity.display.name)
					.replace('%d', pkt.damage),
				ChatBox.TYPE.INFO,
				ChatBox.FILTER.PARTY_BATTLE
			);
		}
	}
}

/**
 * Entity say something
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_CHAT
 */
function onEntityTalk(pkt) {
	let type;

	// Remove "pseudo : |00Dialogue
	pkt.msg = pkt.msg.replace(/\: \|\d{2}/, ': ');

	if (ChatRoom.isOpen) {
		ChatRoom.message(pkt.msg);
		return;
	}

	type = ChatBox.TYPE.PUBLIC;
	const entity = EntityManager.get(pkt.GID);

	ChatBox.addText(pkt.msg, type, ChatBox.FILTER.PUBLIC_CHAT, null, false);

	if (entity) {
		pkt.msg = pkt.msg.replace(
			/<ITEMLINK>.*?<\/ITEMLINK>|<ITEML>.*?<\/ITEML>|<ITEM>.*?<\/ITEM>/gi,
			function (match) {
				return '<' + DB.getItemNameFromLink(match) + '>';
			}
		);

		entity.dialog.set(pkt.msg);

		// Should not happen
		if (entity === Session.Entity) {
			type |= ChatBox.TYPE.SELF;
		} else if (entity.isAdmin) {
			type |= ChatBox.TYPE.ADMIN;
		}
	}
}

/**
 * Entity say something in color (channel system)
 *
 * @param {object} pkt - PACKET.ZC.NPC_CHAT
 */
function onEntityTalkColor(pkt) {
	const color =
		'rgb(' +
		[pkt.color & 0x000000ff, (pkt.color & 0x0000ff00) >> 8, (pkt.color & 0x00ff0000) >> 16].join(',') +
		')'; // bgr to rgb.

	// Remove "pseudo : |00Dialogue"
	pkt.msg = pkt.msg.replace(/\: \|\d{2}/, ': ');

	const entity = EntityManager.get(pkt.accountID);
	if (entity) {
		entity.dialog.set(pkt.msg);
	}
	ChatBox.addText(pkt.msg, ChatBox.TYPE.PUBLIC, ChatBox.FILTER.PUBLIC_CHAT, color);
}

/**
 * Display entity's name
 *
 * @param {object} pkt - PACKET.ZC.ACK_REQNAME
 */
function onEntityIdentity(pkt) {
	const entity = EntityManager.get(pkt.AID);
	if (entity) {
		if (entity.display.name) {
			entity.display.fakename = pkt.CName;
		} else {
			entity.display.name = pkt.CName;
		}

		if (PACKETVER.value >= 20170208 && pkt.TitleID > 0) {
			const titleText = DB.getTitleString(pkt.TitleID);
			entity.display.title_name = titleText;
		} else {
			entity.display.title_name = '';
		}

		entity.display.party_name = pkt.PName || '';
		entity.display.guild_name = pkt.GName || '';
		entity.display.guild_rank = pkt.RName || '';

		entity.display.load = entity.display.TYPE.COMPLETE;

		if (entity.GUID) {
			Guild.requestGuildEmblem(entity.GUID, entity.GEmblemVer, (image, gif) => {
				entity.setEntityGuildEmblem(image, gif);
				if (Session.mapState.isSiege && entity.GUID !== Session.Entity.GUID) {
					entity.emblem.display = true;
				}
			});
		} else if (pkt.GID) {
			DB.loadGroupEmblem(pkt.GID, image => {
				entity.setEntityGuildEmblem(image);
			});
		} else if (entity.mobtype) {
			DB.loadMobEmblem(entity.mobtype, image => {
				entity.setEntityGuildEmblem(image);
			});
		} else if (entity.clanId) {
			DB.loadClanEmblem(entity.clanId, image => {
				entity.setEntityGuildEmblem(image);
			});
		} else {
			entity.setEntityGuildEmblem(null, null);
		}

		// Update display
		updateEntityStyle(entity);

		if (EntityManager.getOverEntity() === entity) {
			entity.display.add();
		}
	}
}

function updateEntityStyle(entity) {
	entity.display.update(
		entity.objecttype === Entity.TYPE_MOB
			? entity.display.STYLE.MOB
			: entity.objecttype === Entity.TYPE_NPC_ABR
				? entity.display.STYLE.MOB
				: entity.objecttype === Entity.TYPE_NPC_BIONIC
					? entity.display.STYLE.MOB
					: entity.objecttype === Entity.TYPE_DISGUISED
						? entity.display.STYLE.MOB
						: entity.objecttype === Entity.TYPE_NPC
							? entity.display.STYLE.NPC
							: entity.objecttype === Entity.TYPE_NPC2
								? entity.display.STYLE.NPC
								: entity.objecttype === Entity.TYPE_PC && entity.isAdmin
									? entity.display.STYLE.ADMIN
									: entity.display.STYLE.DEFAULT
	);
}

function onTitleChangeAck(pkt) {
	if (pkt.result === 0) {
		const comp = Equipment.getUI();
		if (comp && typeof comp.setTitle === 'function') {
			comp.setTitle(pkt.title_id);
		}
	}
}

/**
 * Update entity's life
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_MONSTER_HP
 */
function onEntityLifeUpdate(pkt) {
	EntityManager.storeLife(pkt.AID, { hp: pkt.hp, hp_max: pkt.maxhp });

	const entity = EntityManager.get(pkt.AID);
	if (entity) {
		entity.life.hp = pkt.hp;
		entity.life.hp_max = pkt.maxhp;
		entity.life.update();
		entity.life.display = true;
	}
}

/**
 * Update entity's life (Tiny)
 *
 * @param {object} pkt - PACKET.ZC.HP_INFO_TINY
 */
function onEntityLifeUpdateTiny(pkt) {
	const hp = pkt.hp * 5;
	EntityManager.storeLife(pkt.GID, { hp: hp, hp_max: 100 });

	const entity = EntityManager.get(pkt.GID);
	if (entity) {
		entity.life.hp = hp;
		entity.life.hp_max = 100;
		entity.life.update();
		entity.life.display = true;
	}
}

/**
 * Shows notification effect for quests and events
 *
 * @param {object} pkt - PACKET.ZC.QUEST_NOTIFY_EFFECT
 */
function onEntityQuestNotifyEffect(pkt) {
	const entity = EntityManager.get(pkt.npcID);
	let color = 0;

	if (pkt.effect !== 9999) {
		const emotionId = pkt.effect + 81;

		if (entity && pkt.effect in Emotions.indexes) {
			entity.attachments.add({
				frame: Emotions.indexes[emotionId],
				file: 'emotion',
				play: true,
				head: true,
				repeat: true,
				depth: 5.0
			});
		}
	}

	switch (pkt.color + 1) {
		case 1:
			// yellow
			color = 0xffff00;
			break;
		case 2:
			// orange
			color = 0xffa500;
			break;
		case 3:
			// green
			color = 0x00e16a;
			break;
		case 4:
			// purple
			color = 0x800080;
			break;
		case 0:
		default:
			return;
	}

	MiniMap.getUI().addNpcMark(pkt.npcID, pkt.xPos, pkt.yPos, color, Infinity);
}

/**
 * Updating entity direction
 *
 * @param {object} pkt - PACKET.ZC.CHANGE_DIRECTION
 */
function onEntityDirectionChange(pkt) {
	const entity = EntityManager.get(pkt.AID);
	if (entity) {
		entity.direction = [4, 3, 2, 1, 0, 7, 6, 5][pkt.dir];
		entity.headDir = pkt.headDir;
	}
}

/**
 * Update Entity's visual look
 *
 * @param {object} pkt - PACKET.ZC.SPRITE_CHANGE2
 */
function onEntityViewChange(pkt) {
	const entity = EntityManager.get(pkt.GID);

	if (!entity) {
		return;
	}

	switch (pkt.type) {
		case 0:
			if (
				entity.objecttype === Entity.TYPE_EFFECT ||
				entity.objecttype === Entity.TYPE_UNIT ||
				entity.objecttype === Entity.TYPE_TRAP
			) {
				EffectManager.spamSkillZone(
					pkt.value,
					entity.position[0],
					entity.position[1],
					pkt.GID,
					entity.creatorGID
				);
			} else {
				// Don't override job visuals if transformation is active
				if (entity._job_transform || entity._monster_transform || entity._active_monster_transform) {
					entity._job = pkt.value;
				} else {
					entity.job = pkt.value;
				}
				if (entity === Session.Entity) {
					//Interchange UI depending on Job
					if (PACKETVER.value >= 20200520) {
						BasicInfo.getUI().remove();
						BasicInfo.selectUIVersionWithJob(DB.getJobClass(pkt.value));
						BasicInfo.getUI().prepare();
						BasicInfo.getUI().update('blvl', Session.Entity.clevel);
						BasicInfo.getUI().update('jlvl', Session.Entity.joblevel);
						BasicInfo.getUI().update('zeny', Session.Entity.money);
						BasicInfo.getUI().update('name', Session.Entity.display.name);
						BasicInfo.getUI().update('bexp', BasicInfo.getUI().base_exp, BasicInfo.getUI().base_exp_next);
						BasicInfo.getUI().append();
					}
					// Update UI for all client versions
					BasicInfo.getUI().update('job', pkt.value);
				}
			}
			break;

		case 1:
			entity.head = pkt.value;
			break;

		case 2:
			// In packet PACKET.ZC.SPRITE_CHANGE2, weapon and shield values are
			// stored in a long value (uint16 and uint16 in uint32)
			// source: https://github.com/rathena/rathena/blob/master/src/map/clif.c#L3162
			if (pkt instanceof PACKET.ZC.SPRITE_CHANGE2) {
				// don't know why switch from katar to sword, knife server put it on the left hand instead of right hand first.
				// so we have to swap it. maybe have a better solution.
				if (pkt.value === 0 && pkt.value2 !== 0 && !DB.isShield(pkt.value2)) {
					pkt.value = pkt.value2;
					pkt.value2 = 0;
				}

				const weaponType = DB.getWeaponType(pkt.value, true);
				const viewId = DB.getWeaponViewID(pkt.value);

				if (DB.isAssassin(entity.job) && pkt.value2 !== 0 && !DB.isShield(pkt.value2)) {
					const secondaryWeaponType = DB.getWeaponType(pkt.value2, true);
					entity.weapon = DB.mountWeapon(weaponType, secondaryWeaponType);
					entity.shield = 0;
				} else if (DB.isKatar(weaponType)) {
					entity.weapon = viewId;
					entity.shield = viewId;
				} else {
					entity.weapon = viewId;
					entity.shield = pkt.value2;
				}
			} else {
				entity.weapon = pkt.value;
				entity.shield = pkt.value2;
			}

			// load self aura
			entity.aura.load(EffectManager);

			if (entity.falcon) {
				entity.falcon.set({
					PosDir: [entity.position[0], entity.position[1], 0],
					job: entity._job + '_FALCON'
				});
			}
			if (entity.wug) {
				entity.wug.set({
					PosDir: [entity.position[0], entity.position[1], 0],
					job: entity._job + '_WUG'
				});
			}
			break;

		case 3:
			entity.accessory = pkt.value;
			break;
		case 4:
			entity.accessory2 = pkt.value;
			break;
		case 5:
			entity.accessory3 = pkt.value;
			break;
		case 6:
			entity.headpalette = pkt.value;
			break;
		case 7:
			entity.bodypalette = pkt.value;
			break;
		case 8:
			entity.shield = pkt.value;
			break;
		case 9:
			break; // LOOK_SHOES ??
		case 10:
			break; // LOOK_BODY ??
		case 11:
			break; // LOOK_RESET_COSTUMES (Makes all headgear sprites on player vanish when activated.)
		case 12:
			entity.robe = pkt.value;
			break;
		case 13:
			entity.body = pkt.value;
			break; // LOOK_BODY2
		case 14:
			break; // LOOK_FLOOR ??
	}
}

/**
 * Update NPC's visual look
 *
 * @param {object} pkt - PACKET.ZC.NPCSPRITE_CHANGE
 */
function onNPCViewChange(pkt) {
	const entity = EntityManager.get(pkt.GID);

	// Type is fixed 1 and no other values. No need to do anything with it
	if (entity) {
		entity.job = pkt.value;
	}
}

/**
 * Entity use skill on another entity (no damage : heal, boost, etc.)
 *
 * @param {object} pkt - PACKET.ZC.USE_SKILL
 */
function onEntityUseSkill(pkt) {
	const srcEntity = EntityManager.get(pkt.srcAID);
	const dstEntity = EntityManager.get(pkt.targetAID);

	// Don't display skill names for mobs and hiding skills
	if (
		srcEntity &&
		(srcEntity.objecttype === Entity.TYPE_PC ||
			srcEntity.objecttype === Entity.TYPE_DISGUISED ||
			srcEntity.objecttype === Entity.TYPE_PET ||
			srcEntity.objecttype === Entity.TYPE_HOM ||
			srcEntity.objecttype === Entity.TYPE_MERC ||
			srcEntity.objecttype === Entity.TYPE_ELEM)
	) {
		if (!SkillNameDisplayExclude.includes(pkt.SKID)) {
			srcEntity.dialog.set(
				((SkillInfo[pkt.SKID] && SkillInfo[pkt.SKID].SkillName) || 'Unknown Skill') + ' !!',
				'white'
			);
		}
	}

	//Action handling
	if (srcEntity) {
		if (srcEntity.action !== srcEntity.ACTION.DIE && srcEntity.action !== srcEntity.ACTION.SIT) {
			if (pkt.SKID in SkillActionTable) {
				const action = SkillActionTable[pkt.SKID];
				if (action) {
					srcEntity.setAction(action(srcEntity, Renderer.tick, pkt));
				}
			} else {
				if (DB.isDoram(srcEntity.job)) {
					srcEntity.setAction(SkillActionTable['DEFAULT_DORAM'](srcEntity, Renderer.tick, pkt));
				} else if (DB.isMonk(srcEntity.job)) {
					srcEntity.setAction(SkillActionTable['DEFAULT_MONK'](srcEntity, Renderer.tick, pkt));
				} else {
					srcEntity.setAction(SkillActionTable['DEFAULT'](srcEntity, Renderer.tick, pkt));
				}
			}
		}
	}

	if (dstEntity) {
		if (srcEntity && dstEntity !== srcEntity) {
			srcEntity.lookTo(dstEntity.position[0], dstEntity.position[1]);
		}

		// In healing skill, the level parameter stored the healed value
		if (pkt.SKID === SkillId.AL_HEAL || pkt.SKID === SkillId.AB_HIGHNESSHEAL || pkt.SKID === SkillId.AB_CHEAL) {
			Damage.add(pkt.level, dstEntity, Renderer.tick, null, Damage.TYPE.HEAL);
			Sound.playPosition('_heal_effect.wav', dstEntity.position); // healing on neutral targets got another effect than undeads
		}

		// Steal Coin zeny
		if (pkt.SKID === SkillId.RG_STEALCOIN) {
			ChatBox.addText('You got ' + pkt.level + ' zeny.', ChatBox.TYPE.BLUE, ChatBox.FILTER.ITEM);
		}

		if (pkt.SKID === SkillId.GC_ROLLINGCUTTER) {
			if (dstEntity.RollCounter) {
				const EF_Init_Par = {
					effectId: EffectConst.EF_ROLLING1 + dstEntity.RollCounter - 1,
					ownerAID: dstEntity.GID
				};

				EffectManager.spam(EF_Init_Par);
			}
		}

		if (pkt.SKID === SkillId.TK_SEVENWIND) {
			if (pkt.level) {
				const EF_Init_Par = {
					effectId: EffectConst.EF_BEGINASURA1 + pkt.level - 1,
					ownerAID: dstEntity.GID
				};

				EffectManager.spam(EF_Init_Par);
			}
		}

		EffectManager.spamSkill(pkt.SKID, pkt.targetAID, null, null, pkt.srcAID);

		if (pkt.result == 1) {
			EffectManager.spamSkillSuccess(pkt.SKID, pkt.targetAID, null, pkt.srcAID);
		}
	}
}

/**
 * Entity just finish casting a skill to position
 *
 * @param {object} pkt - PACKET.ZC.SKILL_ENTRY
 */
function onSkillAppear(pkt) {
	EffectManager.spamSkillZone(pkt.job, pkt.xPos, pkt.yPos, pkt.AID, pkt.creatorAID);
}

/**
 * Remove a skill from screen
 *
 * @param {object} pkt - PACKET.ZC.SKILL_DISAPPEAR
 */
function onSkillDisapear(pkt) {
	EffectManager.remove(null, pkt.AID);
	const entity = EntityManager.get(pkt.AID);
	if (entity) {
		entity.remove();
	}
}

/**
 * Entity use skill on another entity with damage
 *
 * @param {object} pkt - PACKET.ZC.NOTIFY_SKILL
 */
function onEntityUseSkillToAttack(pkt) {
	const SkillAction = {}; //Corresponds to e_damage_type in clif.hpp
	SkillAction.NORMAL = 0; /// damage [ damage: total damage, div: amount of hits, damage2: assassin dual-wield damage ]
	SkillAction.PICKUP_ITEM = 1; /// pick up item
	SkillAction.SIT_DOWN = 2; /// sit down
	SkillAction.STAND_UP = 3; /// stand up
	SkillAction.ENDURE = 4; /// damage (endure)
	SkillAction.SPLASH = 5; /// (splash?)
	SkillAction.SKILL = 6; /// (skill?)
	SkillAction.REPEAT = 7; /// (repeat damage?)
	SkillAction.MULTI_HIT = 8; /// multi-hit damage
	SkillAction.MULTI_HIT_ENDURE = 9; /// multi-hit damage (endure)
	SkillAction.CRITICAL = 10; /// critical hit
	SkillAction.LUCY_DODGE = 11; /// lucky dodge
	SkillAction.TOUCH = 12; /// (touch skill?)
	SkillAction.MULTI_HIT_CRITICAL = 13; /// multi-hit critical

	const srcEntity = EntityManager.get(pkt.AID);
	const dstEntity = EntityManager.get(pkt.targetID);
	let srcWeapon;

	if (srcEntity) {
		pkt.attackMT = Math.min(9999, pkt.attackMT); // FIXME: cap value ?
		pkt.attackMT = Math.max(1, pkt.attackMT);
		srcEntity.attack_speed = pkt.attackMT;

		srcEntity.amotionTick = Renderer.tick + pkt.attackMT * 2; // Add amotion delay

		srcWeapon = 0;
		if (srcEntity.weapon) {
			srcWeapon = srcEntity.weapon;
		}

		// Don't display skill names for
		//  - hiding skills
		//  - non-player or player owned entity
		//  - skill level < 0
		//  - skill ID < 0
		if (
			!SkillNameDisplayExclude.includes(pkt.SKID) &&
			(srcEntity.objecttype === Entity.TYPE_PC ||
				srcEntity.objecttype === Entity.TYPE_DISGUISED ||
				srcEntity.objecttype === Entity.TYPE_PET ||
				srcEntity.objecttype === Entity.TYPE_HOM ||
				srcEntity.objecttype === Entity.TYPE_MERC ||
				srcEntity.objecttype === Entity.TYPE_ELEM) &&
			!(pkt.level < 0) &&
			!(pkt.SKID < 0)
		) {
			srcEntity.dialog.set(((SkillInfo[pkt.SKID] && SkillInfo[pkt.SKID].SkillName) || 'Unknown Skill') + ' !!');
		}

		//Action handling
		if (srcEntity.action !== srcEntity.ACTION.DIE && srcEntity.action !== srcEntity.ACTION.SIT) {
			if (pkt.SKID in SkillActionTable) {
				const action = SkillActionTable[pkt.SKID];
				if (action) {
					srcEntity.setAction(action(srcEntity, Renderer.tick, pkt));
				}
			} else {
				if (DB.isDoram(srcEntity.job)) {
					srcEntity.setAction(SkillActionTable['DEFAULT_DORAM'](srcEntity, Renderer.tick, pkt));
				} else if (DB.isMonk(srcEntity.job)) {
					srcEntity.setAction(SkillActionTable['DEFAULT_MONK'](srcEntity, Renderer.tick, pkt));
				} else {
					srcEntity.setAction(SkillActionTable['DEFAULT'](srcEntity, Renderer.tick, pkt));
				}
			}

			//Pet Talk
			if (
				srcEntity.GID === Session.Entity.GID &&
				Session.pet.friendly > 900 &&
				(Session.pet.lastTalk || 0) + 10000 < Date.now()
			) {
				const talkRate = parseInt(Math.random() * 10);
				if (talkRate < 3) {
					const hunger = DB.getPetHungryState(Session.pet.oldHungry);
					const talk = DB.getPetTalkNumber(Session.pet.job, PetMessageConst.PM_HUNTING, hunger);

					const talkPkt = new PACKET.CZ.PET_ACT();
					talkPkt.data = talk;
					Network.sendPacket(talkPkt);
					Session.pet.lastTalk = Date.now();
				}
			}
		}

		if (srcEntity.falcon) {
			if (pkt.SKID == SkillId.HT_BLITZBEAT || pkt.SKID == SkillId.SN_FALCONASSAULT) {
				srcEntity.falcon.action = srcEntity.action;
				srcEntity.falcon.walk.speed = 35;

				srcEntity.falcon.walkToNonWalkableGround(
					srcEntity.falcon.position[0],
					srcEntity.falcon.position[1],
					dstEntity.position[0],
					dstEntity.position[1],
					0,
					true,
					true
				);
			}
		}

		if (srcEntity.wug) {
			if (pkt.SKID == SkillId.RA_WUGSTRIKE || pkt.SKID == SkillId.RA_WUGBITE) {
				srcEntity.wug.action = srcEntity.action;
				srcEntity.wug.walk.speed = 35;

				srcEntity.wug.walkToNonWalkableGround(
					srcEntity.wug.position[0],
					srcEntity.wug.position[1],
					dstEntity.position[0],
					dstEntity.position[1],
					1,
					false,
					true
				);
			}
		}
	}

	if (dstEntity) {
		const target = pkt.damage ? dstEntity : srcEntity;

		if (pkt.damage && target && !(srcEntity == dstEntity && pkt.action == SkillAction.SKILL)) {
			// Will be hit actions
			onEntityWillBeHitSub(pkt, dstEntity);

			const isCombo = target.objecttype !== Entity.TYPE_PC && pkt.count > 1;
			const isBlueCombo = SkillBlueCombo.includes(pkt.SKID);

			const addDamage = function (i, startTick) {
				if (pkt.damage) {
					// Only if hits
					EffectManager.spamSkillHit(pkt.SKID, pkt.targetID, startTick, pkt.AID);
				}

				if (!isCombo && isBlueCombo) {
					// Blue 'crit' non-combo EG: Rampage Blaster that hits
					Damage.add(
						pkt.damage / pkt.count,
						target,
						startTick,
						srcWeapon,
						Damage.TYPE.COMBO_B | (i + 1 === pkt.count ? Damage.TYPE.COMBO_FINAL : 0)
					);
				} else {
					Damage.add(pkt.damage / pkt.count, target, startTick, srcWeapon); // Normal
				}

				// Only display combo if the target is not entity and
				// there are multiple attacks and actually hits
				if (isCombo) {
					Damage.add(
						(pkt.damage / pkt.count) * (i + 1),
						target,
						startTick,
						srcWeapon,
						(isBlueCombo ? Damage.TYPE.COMBO_B : Damage.TYPE.COMBO) |
							(i + 1 === pkt.count ? Damage.TYPE.COMBO_FINAL : 0)
					);
				}
			};

			for (let i = 0; i < pkt.count; ++i) {
				EffectManager.spamSkillBeforeHit(pkt.SKID, pkt.targetID, Renderer.tick + C_MULTIHIT_DELAY * i, pkt.AID);
				addDamage(i, Renderer.tick + pkt.attackMT + C_MULTIHIT_DELAY * i);
			}
		}

		if (typeof pkt.xPos === 'number' && typeof pkt.yPos === 'number' && (pkt.xPos !== 0 || pkt.yPos !== 0)) {
			const pushedEntity = dstEntity || srcEntity;
			if (pushedEntity) {
				pushedEntity.fastMoveTo(pkt.xPos, pkt.yPos, 20, null, true);
			}
		}
	}

	if (srcEntity && dstEntity && pkt.action != SkillAction.SPLASH) {
		// && pkt.action != SkillAction.MULTI_HIT
		EffectManager.spamSkill(pkt.SKID, pkt.targetID, null, Renderer.tick + pkt.attackMT, pkt.AID);
	}
}

/**
 * Cast a skill to someone
 * @param {object} pkt - pkt PACKET.ZC.USESKILL_ACK
 */
function onEntityCastSkill(pkt) {
	// property:
	//     0 = Yellow cast aura
	//     1 = Water elemental cast aura
	//     2 = Earth elemental cast aura
	//     3 = Fire elemental cast aura
	//     4 = Wind elemental cast aura
	//     5 = Poison elemental cast aura
	//     6 = Holy elemental cast aura
	//     7 = Shadow/Dark elemental cast aura
	//     8 = Ghost elemental cast aura (same as 6?)
	//     9 = Undead elemental cast aura
	// is disposable:
	//     0 = yellow chat text "[src name] will use skill [skill name]."
	//     1 = no text

	const srcEntity = EntityManager.get(pkt.AID);
	const dstEntity = EntityManager.get(pkt.targetID);

	if (!srcEntity) {
		return;
	}

	let hideCastBar = false;
	let hideCastAura = false;
	let isPlay = true;
	let next = {
		action: srcEntity.ACTION.READYFIGHT,
		frame: 0,
		repeat: true,
		play: true,
		next: false
	};

	if (pkt.delayTime) {
		// Check if cast bar needs to be hidden
		hideCastBar = pkt.SKID in SkillEffect && SkillEffect[pkt.SKID].hideCastBar;

		if (!hideCastBar) {
			srcEntity.cast.set(pkt.delayTime);
		}
		isPlay = false;
		next = false;
	}

	if (srcEntity.objecttype === Entity.TYPE_PC) {
		//monsters don't use ACTION.SKILL animation

		const action = (SkillInfo[pkt.SKID] && SkillInfo[pkt.SKID].ActionType) || 'SKILL';

		srcEntity.setAction({
			action: srcEntity.ACTION[action],
			frame: 0,
			repeat: false,
			play: isPlay,
			next: next
		});
	}

	Session.Entity.isCastingSkill = true;
	Session.Entity.lastSKID = pkt.SKID;

	// Hardcoded version of Auto Counter casting bar
	// It's dont gey any delayTime so we need to handle it diffrent:
	// if the monster hit us then PACKET_ZC_DISPEL is received (to force cast bar to cancel)
	// if not it's end by itself (on kRO Renewal you can move during AC to cancel it but it's not implemented on privates yet)
	if (pkt.SKID == SkillId.KN_AUTOCOUNTER) {
		srcEntity.cast.set(1000);
		if (srcEntity === Session.Entity) {
			Session.underAutoCounter = true;
		}
	}

	if (pkt.SKID == SkillId.HT_DETECTING && srcEntity.falcon) {
		srcEntity.falcon.walk.speed = 35;
		srcEntity.falcon.walkToNonWalkableGround(
			srcEntity.falcon.position[0],
			srcEntity.falcon.position[1],
			pkt.xPos,
			pkt.yPos,
			0,
			true,
			true
		);
	}

	// Only mob to don't display skill name ?
	if (
		srcEntity.objecttype === Entity.TYPE_PC ||
		srcEntity.objecttype === Entity.TYPE_DISGUISED ||
		srcEntity.objecttype === Entity.TYPE_PET ||
		srcEntity.objecttype === Entity.TYPE_HOM ||
		srcEntity.objecttype === Entity.TYPE_MERC ||
		srcEntity.objecttype === Entity.TYPE_ELEM
	) {
		if (!SkillNameDisplayExclude.includes(pkt.SKID)) {
			srcEntity.dialog.set(
				((SkillInfo[pkt.SKID] && SkillInfo[pkt.SKID].SkillName) || 'Unknown Skill') + ' !!',
				'white'
			);
		}
	}

	//Spells like Bash, Hide, Double Strafe etc. has special casting effect
	EffectManager.spamSkillCast(pkt.SKID, pkt.AID, null, pkt.targetID);

	if (dstEntity && dstEntity !== srcEntity) {
		srcEntity.lookTo(dstEntity.position[0], dstEntity.position[1]);
		if (pkt.delayTime) {
			const EF_Init_Par = {
				effectId: EffectConst.EF_LOCKON,
				ownerAID: dstEntity.GID,
				position: dstEntity.position,
				duration: pkt.delayTime
			};

			EffectManager.spam(EF_Init_Par);
		}
	} else if (pkt.xPos && pkt.yPos) {
		srcEntity.lookTo(pkt.xPos, pkt.yPos);
		if (pkt.delayTime) {
			const EF_Init_Par = {
				effectId: EffectConst.EF_GROUNDSAMPLE,
				skillId: pkt.SKID,
				position: [pkt.xPos, pkt.yPos, Altitude.getCellHeight(pkt.yPos, pkt.yPos)],
				duration: pkt.delayTime,
				otherAID: srcEntity.GID
			};

			EffectManager.spam(EF_Init_Par);
		}
	}

	// Check if cast aura needs to be hidden
	hideCastAura = pkt.SKID in SkillEffect && SkillEffect[pkt.SKID].hideCastAura;

	// Cast aura
	if (pkt.delayTime && !hideCastAura) {
		const EF_Init_Par = {
			effectId: EffectConst.EF_BEGINSPELL, // Default
			ownerAID: srcEntity.GID,
			position: srcEntity.position,
			duration: pkt.delayTime
		};

		switch (pkt.property) {
			case 0:
				EF_Init_Par.effectId = EffectConst.EF_BEGINSPELL;
				break;
			case 1:
				EF_Init_Par.effectId = EffectConst.EF_BEGINSPELL2;
				break;
			case 2:
				EF_Init_Par.effectId = EffectConst.EF_BEGINSPELL5;
				break;
			case 3:
				EF_Init_Par.effectId = EffectConst.EF_BEGINSPELL3;
				break;
			case 4:
				EF_Init_Par.effectId = EffectConst.EF_BEGINSPELL4;
				break;
			case 5:
				EF_Init_Par.effectId = EffectConst.EF_BEGINSPELL7;
				break;
			case 6:
				EF_Init_Par.effectId = EffectConst.EF_BEGINSPELL6;
				break;
			case 7:
				EF_Init_Par.effectId = EffectConst.EF_DARKCASTING;
				break;
			case 8:
				EF_Init_Par.effectId = EffectConst.EF_BEGINSPELL6;
				break;
			case 9:
				EF_Init_Par.effectId = EffectConst.EF_DARKCASTING;
				break;
		}

		EffectManager.spam(EF_Init_Par);
	}
}

/**
 * A cast from an entity just canceled
 *
 * @param {object} pkt - PACKET.ZC.DISPEL
 */
function onEntityCastCancel(pkt) {
	const entity = EntityManager.get(pkt.AID);
	if (entity) {
		entity.cast.clean();

		// Cancel effects
		EffectManager.remove(null, entity.GID, [12, 54, 55, 56, 57, 58, 59, 454, 60, 513]);
		EffectManager.remove(LockOnTarget, entity.GID);
		EffectManager.remove(MagicTarget, entity.GID);
		EffectManager.remove(MagicRing, entity.GID);

		if (entity === Session.Entity) {
			// Autocounter hardcoded animation (any better place to put this?)
			if (Session.underAutoCounter) {
				if (Session.Entity.life.hp > 0) {
					const EF_Init_Par = {
						effectId: EffectConst.EF_AUTOCOUNTER,
						ownerAID: pkt.AID
					};
					EffectManager.spam(EF_Init_Par);
				}
				Session.underAutoCounter = false;
			}
		}
	}
}

/**
 * Update Player status
 *
 * @param {object} pkt - PACKET.ZC.MSG_STATE_CHANGE
 */
function onEntityStatusChange(pkt) {
	const entity = EntityManager.get(pkt.AID);

	// Monster/Active monster transformations - special handling
	if (pkt.index === StatusConst.MONSTER_TRANSFORM || pkt.index === StatusConst.ACTIVE_MONSTER_TRANSFORM) {
		if (!entity) {
			const isActive = pkt.state == 1 || (pkt.val && pkt.val[0] == 1);
			const key = pkt.index === StatusConst.MONSTER_TRANSFORM ? 'monster_transform' : 'active_monster_transform';
			EntityManager.storePendingTransform(pkt.AID, key, isActive ? (pkt.val ? pkt.val[0] : 0) : null);
			return;
		}
	}
	// Job form transformations (WEREWOLF, WERERAPTOR)
	else if (pkt.index === StatusConst.WEREWOLF || pkt.index === StatusConst.WERERAPTOR) {
		if (!entity) {
			const isActive = pkt.state == 1 || (pkt.val && pkt.val[0] == 1);
			EntityManager.storePendingTransform(
				pkt.AID,
				'job_transform',
				isActive ? (pkt.index === StatusConst.WEREWOLF ? JobId.WEREWOLF : JobId.WERERAPTOR) : null
			);
			return;
		}
	}

	if (!entity) {
		if (pkt.index >= StatusConst.SWORDCLAN && pkt.index <= StatusConst.CROSSBOWCLAN) {
			// EFST CLAN - save it to when actor spawn, why server sometimes send this packet before entity spawn?
			clanEmblems[pkt.AID] = pkt.index - StatusConst.SWORDCLAN + 1;
		}
		return;
	}

	// TODO: add other status
	switch (pkt.index) {
		// Maya purple card
		case StatusConst.CLAIRVOYANCE:
			if (entity === Session.Entity) {
				Session.Entity.intravision = pkt.state;
				EntityManager.forEach(_entity => {
					/** @type {*} Intentional self-assignment to trigger effectState updates. */
					// eslint-disable-next-line no-self-assign
					_entity.effectState = _entity.effectState;
				});
			}
			break;

		// Show cart
		case StatusConst.ON_PUSH_CART:
			entity.hasCart = pkt.state || !pkt.hasOwnProperty('state');
			if (pkt.val && (pkt.state || !pkt.hasOwnProperty('state'))) {
				entity.CartNum = pkt.val[0];
			}
			break;

		case StatusConst.HIDING: {
			const EF_Init_Par = {
				effectId: EffectConst.EF_SUMMONSLAVE,
				ownerAID: pkt.AID
			};

			if (pkt.state == 1) {
				EF_Init_Par.effectId = EffectConst.EF_BASH;
			}

			EffectManager.spam(EF_Init_Par);
			break;
		}

		case StatusConst.FALCON:
			if (pkt.state || !pkt.hasOwnProperty('state')) {
				if (!entity.falcon) {
					entity.falcon = new Entity();
				}

				entity.falcon.set({
					objecttype: entity.falcon.constructor.TYPE_FALCON,
					GID: entity.GID + '_FALCON',
					PosDir: [entity.position[0], entity.position[1], 0],
					job: entity._job + '_FALCON',
					speed: Math.max(entity.walk.speed - 50, 1),
					name: '',
					hp: -1,
					maxhp: -1,
					hideShadow: true
				});
				EntityManager.add(entity.falcon);
			}

			break;

		case StatusConst.EXPLOSIONSPIRITS: //state: 1 ON  0 OFF
		case StatusConst.MARIONETTE_MASTER:
		case StatusConst.MARIONETTE:
		case StatusConst.TWOHANDQUICKEN:
		case StatusConst.ONEHANDQUICKEN:
		case StatusConst.SPEARQUICKEN:
		case StatusConst.LKCONCENTRATION:
		case StatusConst.BERSERK:
		case StatusConst.ENERGYCOAT:
		case StatusConst.OVERTHRUST:
		case StatusConst.OVERTHRUSTMAX:
		case StatusConst.SWOO:
		case StatusConst.SKE:
		case StatusConst.NJ_BUNSINJYUTSU:
		case StatusConst.STEELBODY:
		case StatusConst.AURABLADE:
		case StatusConst.ASSUMPTIO:
		case StatusConst.ASSUMPTIO2:
		case StatusConst.SG_WARM:
		case StatusConst.SG_SUN_WARM:
		case StatusConst.SG_MOON_WARM:
		case StatusConst.SG_STAR_WARM:
		case StatusConst.KAITE:
		case StatusConst.SOULLINK:
		case StatusConst.PROPERTYUNDEAD:
		case StatusConst.DA_CONTRACT:
		case StatusConst.HALLUCINATIONWALK:
			//CG_MOONLIT Moonlit Water Mill
			//SC_MERC_QUICKEN
			//SC_SKA
			//SC_INCATKRATE
			entity.toggleOpt3(pkt.index, pkt.state);
			if (entity === Session.Entity && [StatusConst.SOULLINK, StatusConst.SKE].includes(pkt.index)) {
				ScreenEffectManager.setNight(pkt.state === 1);
			}
			break;

		case StatusConst.RUN: {
			//state: 1 ON  0 OFF
			const EF_Init_Par = {
				effectId: EffectConst.EF_STOPEFFECT,
				ownerAID: pkt.AID
			};

			if (pkt.state == 1) {
				EF_Init_Par.effectId = EffectConst.EF_RUN;
				//todo: draw footprints on the floor
			}

			EffectManager.spam(EF_Init_Par);
			break;
		}

		case StatusConst.TING: {
			const EF_Init_Par = {
				effectId: EffectConst.EF_QUAKEBODY,
				ownerAID: pkt.AID
			};

			EffectManager.spam(EF_Init_Par);
			break;
		}

		case StatusConst.STORMKICK_ON:
		case StatusConst.STORMKICK_READY:
			entity.setAction({
				action: entity.ACTION.SKILL,
				frame: 0,
				repeat: false,
				play: false,
				next: false
			});
			break;

		case StatusConst.DOWNKICK_ON:
		case StatusConst.DOWNKICK_READY:
			entity.setAction({
				action: entity.ACTION.SKILL,
				frame: 2,
				repeat: false,
				play: false,
				next: false
			});
			break;

		case StatusConst.TURNKICK_ON:
		case StatusConst.TURNKICK_READY:
			entity.setAction({
				action: entity.ACTION.SKILL,
				frame: 3,
				repeat: false,
				play: false,
				next: false
			});
			break;
		case StatusConst.TRUESIGHT:
			entity.setAction({
				action: entity.ACTION.IDLE,
				frame: 3,
				repeat: true,
				play: true,
				next: false
			});
			break;

		case StatusConst.COUNTER_ON:
		case StatusConst.COUNTER_READY:
			entity.setAction({
				action: entity.ACTION.SKILL,
				frame: 4,
				repeat: false,
				play: false,
				next: false
			});
			break;

		case StatusConst.DODGE_ON:
		case StatusConst.DODGE_READY:
			entity.setAction({
				action: entity.ACTION.PICKUP,
				frame: 1,
				repeat: false,
				play: false,
				next: false
			});
			break;

		case StatusConst.ROLLINGCUTTER:
			if (pkt.state == 1) {
				entity.RollCounter = pkt.val[0];
			} else {
				entity.RollCounter = 0;
			}
			break;

		case StatusConst.CAMOUFLAGE:
			if (pkt.state == 1) {
				entity.Camouflage = pkt.val[0];
			} else {
				entity.Camouflage = 0;
			}
			// eslint-disable-next-line no-self-assign
			entity.effectState = entity.effectState;
			break;

		case StatusConst.SUMMON1:
			if (pkt.state == 1) {
				entity.Summon1 = pkt.val[0];
			} else {
				entity.Summon1 = 0;
			}
			updateWarlockSpheres(entity);
			break;

		case StatusConst.SUMMON2:
			if (pkt.state == 1) {
				entity.Summon2 = pkt.val[0];
			} else {
				entity.Summon2 = 0;
			}
			updateWarlockSpheres(entity);
			break;

		case StatusConst.SUMMON3:
			if (pkt.state == 1) {
				entity.Summon3 = pkt.val[0];
			} else {
				entity.Summon3 = 0;
			}
			updateWarlockSpheres(entity);
			break;

		case StatusConst.SUMMON4:
			if (pkt.state == 1) {
				entity.Summon4 = pkt.val[0];
			} else {
				entity.Summon4 = 0;
			}
			updateWarlockSpheres(entity);
			break;

		case StatusConst.SUMMON5:
			if (pkt.state == 1) {
				entity.Summon5 = pkt.val[0];
			} else {
				entity.Summon5 = 0;
			}
			updateWarlockSpheres(entity);
			break;

		case StatusConst.STEALTHFIELD:
			if (pkt.state == 1) {
				entity.Stealthfield = pkt.val[0];
			} else {
				entity.Stealthfield = 0;
			}
			// eslint-disable-next-line no-self-assign
			entity.effectState = entity.effectState;
			break;

		case StatusConst.SHADOWFORM:
			if (pkt.state == 1) {
				entity.Shadowform = pkt.val[0];
			} else {
				entity.Shadowform = 0;
			}
			// eslint-disable-next-line no-self-assign
			entity.effectState = entity.effectState;
			break;

		case StatusConst.TRICKDEAD:
			if (pkt.state == 1) {
				entity.setAction({
					action: entity.ACTION.DIE,
					frame: 0,
					repeat: false,
					play: true,
					next: false
				});
			}
			if (pkt.state == 0) {
				entity.setAction({
					action: entity.ACTION.IDLE,
					frame: 0,
					repeat: false,
					play: true,
					next: false
				});
			}
			break;

		// Monster transformations (generic)
		case StatusConst.MONSTER_TRANSFORM:
			if (pkt.state == 1 || (pkt.val && pkt.val[0] == 1)) {
				// Transform into monster
				const monsterId = pkt.val ? pkt.val[0] : 0;
				entity.monster_transform = monsterId;
			} else {
				// Remove monster transformation
				entity.monster_transform = null;
			}
			break;

		// Active monster transformations (higher priority)
		case StatusConst.ACTIVE_MONSTER_TRANSFORM:
			if (pkt.state == 1 || (pkt.val && pkt.val[0] == 1)) {
				// Active transform into monster
				const monsterId = pkt.val ? pkt.val[0] : 0;
				entity.active_monster_transform = monsterId;
			} else {
				// Remove active monster transformation
				entity.active_monster_transform = null;
			}
			break;

		// Job form transformations (WEREWOLF, WERERAPTOR)
		case StatusConst.WEREWOLF: // Werewolf transformation
			if (pkt.state == 1 || (pkt.val && pkt.val[0] == 1)) {
				entity.job_transform = JobId.WEREWOLF;
			} else {
				entity.job_transform = null;
			}
			break;

		case StatusConst.WERERAPTOR: // Wereraptor transformation
			if (pkt.state == 1 || (pkt.val && pkt.val[0] == 1)) {
				entity.job_transform = JobId.WERERAPTOR;
			} else {
				entity.job_transform = null;
			}
			break;

		case StatusConst.ILLUSION:
			if (pkt.state == 1) {
				entity.isHallucinating = true;
			} else {
				entity.isHallucinating = false;
			}
			break;

		case StatusConst.STOP:
			if (pkt.state == 1) {
				entity.attachments.add({
					repeat: true,
					uid: 'status-stop',
					file: '\xbd\xba\xc5\xe9'
				});
			} else {
				entity.attachments.remove('status-stop');
			}
			break;

		case StatusConst.C_MARKER:
			if (pkt.state == 1) {
				const EF_Init_Par = {
					effectId: 'ef_c_marker2',
					ownerAID: pkt.AID
				};

				EffectManager.spam(EF_Init_Par);
			}
			break;

		case StatusConst.SUHIDE:
			if (pkt.state == 1) {
				entity.setAction({
					action: entity.ACTION.SKILL,
					frame: 4,
					repeat: false,
					play: false,
					next: false
				});
			} else if (pkt.state == 0) {
				entity.setAction({
					action: entity.ACTION.IDLE,
					frame: 0,
					repeat: false,
					play: true,
					next: false
				});
			}
			break;

		case StatusConst.SU_STOOP:
			if (pkt.state == 1) {
				entity.setAction({
					action: entity.ACTION.SKILL,
					frame: 1,
					length: 3,
					repeat: true,
					play: true,
					next: false
				});
			} else if (pkt.state == 0) {
				entity.setAction({
					action: entity.ACTION.IDLE,
					frame: 0,
					repeat: false,
					play: true,
					next: false
				});
			}
			break;

		// Cast a skill, TODO: add progressbar in shortcut
		case StatusConst.GROUNDMAGIC:
		case StatusConst.POSTDELAY:
			if (pkt.RemainMS && entity == Session.Entity) {
				ShortCut.setGlobalSkillDelay(pkt.RemainMS);
			}
			break;

		case StatusConst.ALL_RIDING:
			if (pkt.hasOwnProperty('state')) {
				entity.allRidingState = pkt.state;
			} else if (pkt.hasOwnProperty('val')) {
				entity.allRidingState = pkt.val[0];
			}
			break;

		case StatusConst.WEIGHTOVER90:
			entity.isOverWeight = pkt.state;
			break;

		case StatusConst.SWORDCLAN:
		case StatusConst.ARCWANDCLAN:
		case StatusConst.GOLDENMACECLAN:
		case StatusConst.CROSSBOWCLAN: {
			const clanId = pkt.index - StatusConst.SWORDCLAN + 1;
			DB.loadClanEmblem(clanId, image => {
				entity.clanId = clanId;
				entity.setEntityGuildEmblem(image);
				updateEntityStyle(entity);
			});
			break;
		}
		case StatusConst.CLAN_INFO:
			DB.loadClanEmblem(pkt.val[1], image => {
				entity.clanId = pkt.val[1];
				entity.setEntityGuildEmblem(image);
				updateEntityStyle(entity);
			});
			break;
	}

	processBlockStatus(entity, pkt);

	// Modify icon
	if (entity === Session.Entity) {
		StatusIcons.update(pkt.index, pkt.state, pkt.RemainMS);
	}
}

//Warlock sphere summons update
function updateWarlockSpheres(entity) {
	if (entity.Summon1 || entity.Summon2 || entity.Summon3 || entity.Summon4 || entity.Summon5) {
		const EF_Init_Par = {
			effectId: 'temporary_warlock_sphere',
			ownerAID: entity.GID,
			persistent: false
		};

		EffectManager.spam(EF_Init_Par);

		entity.WarlockSpheres = true;
	} else if (entity.WarlockSpheres) {
		EffectManager.remove(null, entity.GID, 'temporary_warlock_sphere');
		entity.WarlockSpheres = false;
	}
}

/**
 * Update player option
 *
 * @param {object} pkt - PACKET.ZC.STATE_CHANGE
 */
function onEntityOptionChange(pkt) {
	const entity = EntityManager.get(pkt.AID);
	if (!entity) {
		return;
	}

	//display/remove special effects on state change
	addStateEffect(pkt.AID, entity._effectState, pkt.effectState, StatusState.EffectState.SIGHT, EffectConst.EF_SIGHT);
	addStateEffect(
		pkt.AID,
		entity._effectState,
		pkt.effectState,
		StatusState.EffectState.RUWACH,
		EffectConst.EF_RUWACH
	);

	entity.bodyState = pkt.bodyState;
	entity.healthState = pkt.healthState;
	entity.effectState = pkt.effectState;
	entity.isPKModeON = pkt.isPKModeON;

	// for changes in effectState (HIDING, CLOAK)
	entity.aura.load(EffectManager);

	if (!entity.falcon && entity.effectState & StatusState.EffectState.FALCON) {
		entity.falcon = new Entity();
		entity.falcon.set({
			objecttype: entity.falcon.constructor.TYPE_FALCON,
			GID: entity.GID + '_FALCON',
			PosDir: [entity.position[0], entity.position[1], 0],
			job: entity._job + '_FALCON',
			speed: Math.max(entity.walk.speed - 50, 1),
			name: '',
			hp: -1,
			maxhp: -1,
			hideShadow: true
		});
		EntityManager.add(entity.falcon);
	} else if (entity.falcon && !(entity.effectState & StatusState.EffectState.FALCON)) {
		entity.falcon.remove();
		entity.falcon = null;
	}

	if (!entity.wug && entity.effectState & StatusState.EffectState.WUG) {
		entity.wug = new Entity();
		entity.wug.set({
			objecttype: entity.wug.constructor.TYPE_WUG,
			GID: entity.GID + '_WUG',
			PosDir: [entity.position[0], entity.position[1], 0],
			job: entity._job + '_WUG',
			speed: Math.max(entity.walk.speed - 50, 1),
			name: '',
			hp: -1,
			maxhp: -1
		});
		EntityManager.add(entity.wug);
	} else if (entity.wug && !(entity.effectState & StatusState.EffectState.WUG)) {
		entity.wug.remove();
		entity.wug = null;
	}
}

/**
 * Display a shop above entity's head
 *
 * @param {object} pkt - PACKET.ZC.STORE_ENTRY / PACKET.ZC.DISAPPEAR_BUYING_STORE_ENTRY
 */
function onEntityCreateRoom(pkt) {
	let entity;

	if (pkt instanceof PACKET.ZC.STORE_ENTRY) {
		entity = EntityManager.get(pkt.makerAID);
		if (entity) {
			entity.room.create(pkt.storeName, pkt.makerAID, entity.room.constructor.Type.BUY_SHOP, true);
		}
		return;
	}

	if (pkt instanceof PACKET.ZC.BUYING_STORE_ENTRY) {
		entity = EntityManager.get(pkt.makerAID);
		if (entity) {
			entity.room.create(pkt.storeName, pkt.makerAID, entity.room.constructor.Type.SELL_SHOP, true);
		}
		return;
	}

	if (pkt instanceof PACKET.ZC.ROOM_NEWENTRY) {
		entity = EntityManager.get(pkt.AID);
		if (entity) {
			let type = entity.room.constructor.Type.PUBLIC_CHAT;
			let title = pkt.title + ' (' + pkt.curcount + '/' + pkt.maxcount + ')';

			switch (pkt.type) {
				case 0: // password
					type = entity.room.constructor.Type.PRIVATE_CHAT;
					break;

				case 1:
					break; // public
				case 2:
					break; // arena (npc waiting room)

				case 3: // PK zone - non clickable ???
					title = pkt.title; // no user limit
					break;
			}

			entity.room.title = pkt.title;
			entity.room.limit = pkt.maxcount;
			entity.room.count = pkt.curcount;

			entity.room.create(title, pkt.roomID, type, true);
		}
	}
}

/**
 * Remove entity room
 *
 * @param {object} pkt - PACKET.ZC.DISAPPEAR_ENTRY
 */
function onEntityDestroyRoom(pkt) {
	if ('roomID' in pkt) {
		EntityManager.forEach(function (entity) {
			if (entity.room.id === pkt.roomID) {
				entity.room.remove();
				return false;
			}
			return true;
		});
		return;
	}

	const entity = EntityManager.get(pkt.makerAID);
	if (entity) {
		entity.room.remove();
	}
}

/**
 * "Blade Stop" / "Root" visual
 */

function onBladeStopVisual(srcEntity, dstEntity, state) {
	srcEntity.lookTo(dstEntity.position[0], dstEntity.position[1]);
	srcEntity.toggleOpt3(StatusConst.BLADESTOP, state);
	if (state == 1) {
		srcEntity.setAction({
			action: srcEntity.ACTION.READYFIGHT,
			frame: 0,
			repeat: false,
			play: true,
			next: false
		});
	}
	if (state == 0) {
		srcEntity.setAction({
			action: srcEntity.ACTION.IDLE,
			frame: 0,
			repeat: false,
			play: true,
			next: false
		});
	}
}

/**
 * "Blade Stop" / "Root" skill status
 *
 * @param {object} pkt - PACKET.ZC.BLADESTOP
 */
function onBladeStopPacket(pkt) {
	const srcEntity = EntityManager.get(pkt.srcAID);
	const dstEntity = EntityManager.get(pkt.destAID);
	if (srcEntity && dstEntity) {
		onBladeStopVisual(srcEntity, dstEntity, pkt.flag);
		onBladeStopVisual(dstEntity, srcEntity, pkt.flag);
	}
}

/**
 * Notify experience gained
 *
 * @param {object} pkt - PACKET_ZC_NOTIFY_EXP
 */

function onNotifyExp(pkt) {
	switch (pkt.expType) {
		case 0:
			if (pkt.varID === 1) {
				ChatBox.addText(DB.getMessage(1613).replace('%d', pkt.amount), ChatBox.TYPE.INFO, ChatBox.FILTER.EXP);
			} else if (pkt.varID === 2) {
				ChatBox.addText(DB.getMessage(1614).replace('%d', pkt.amount), ChatBox.TYPE.INFO, ChatBox.FILTER.EXP);
			}
			break;
		case 1:
			if (pkt.varID === 1) {
				ChatBox.addText(
					'Experience gained from Quest, Base:' + pkt.amount,
					null,
					ChatBox.FILTER.EXP,
					'#A442DC'
				);
			} else if (pkt.varID === 2) {
				ChatBox.addText('Experience gained from Quest, Job:' + pkt.amount, null, ChatBox.FILTER.EXP, '#A442DC');
			}
			break;
	}
}

/**
 * Mark MVP position on map (Convex Mirror item)
 * it's show small icon  at minimap when MVP is spawned (but I will use cross, it's more accurate)
 * @param {object} pkt - PACKET_ZC_BOSS_INFO
 *
 *   probably it's not updated with Tombstone system, but Tombstones are fail...
 */

function onMarkMvp(pkt) {
	MiniMap.getUI().removeNpcMark('mvp'); //hack for mark system (todo: debug this)
	if (pkt.infoType == 1) {
		MiniMap.getUI().addNpcMark('mvp', pkt.xPos, pkt.yPos, 0x0ff0000, Infinity);
		/**if(!MiniMap.isNpcMarkExist('mvp')) {    // wtf marker is pushed with delay??
				ChatBox.addText( pkt.name+' is already spawned at ('+pkt.xPos+','+pkt.yPos+')', null, ChatBox.FILTER.PUBLIC_LOG, '#FFFF63');
			}*/
	}
	if (pkt.infoType == 0) {
		ChatBox.addText('Boss monster not found.', ChatBox.TYPE.ERROR, ChatBox.FILTER.PUBLIC_LOG);
	}
}

/**
 * Show MvP reward Effect
 *
 * @param {object} pkt - PACKET.ZC.MVP
 */
function onEntityMvpReward(pkt) {
	const EF_Init_Par = {
		effectId: EffectConst.EF_MVP,
		ownerAID: pkt.AID
	};
	EffectManager.spam(EF_Init_Par);
}

/**
 * Show MvP item message
 *
 * @param {object} pkt - PACKET.ZC.MVP_GETTING_ITEM
 */
function onEntityMvpRewardItemMessage(pkt) {
	const item = DB.getItemInfo(pkt.ITID);
	ChatBox.addText(DB.getMessage(143), ChatBox.TYPE.BLUE, ChatBox.FILTER.ITEM);
	ChatBox.addText(item.identifiedDisplayName, ChatBox.TYPE.BLUE, ChatBox.FILTER.ITEM);
}

/**
 * Entity will be hit, handle getting hit and after actions
 *
 * @param pkt - packet PACKET.ZC.NOTIFY_ACT or PACKET.ZC.NOTIFY_SKILL
 * @param dstEntity - Reveiver entity
 */
function onEntityWillBeHitSub(pkt, dstEntity) {
	// only if has damage > 0 and type is not endure and not lucky
	if ((pkt.damage > 0 || pkt.leftDamage > 0) && pkt.action !== 4 && pkt.action !== 9 && pkt.action !== 11) {
		const count = pkt.count || 1;

		// Remember the impact tick of the (last) hit so a death triggered by
		// this attack can be synced to land with the hit instead of instantly.
		const lastHitDelay = pkt.attackMT + C_MULTIHIT_DELAY * (pkt.leftDamage ? 1.75 : 1) * (count - 1);
		dstEntity._deathSyncTick = Math.max(dstEntity._deathSyncTick || 0, Renderer.tick + lastHitDelay);

		function impendingAttack() {
			// Get hurt when attack happens
			if (dstEntity.action !== dstEntity.ACTION.DIE) {
				dstEntity.setAction({
					action: dstEntity.ACTION.HURT,
					frame: 0,
					repeat: false,
					play: true,
					next: {
						action: dstEntity.ACTION.READYFIGHT, // Wiggle-wiggle
						delay: pkt.attackedMT + 0,
						frame: 0,
						repeat: true,
						play: true
					}
				});
			}
		}

		function resumeWalk() {
			// Try resuming walk when targeting something.
			if (
				dstEntity.action !== dstEntity.ACTION.DIE &&
				EntityManager.getFocusEntity() &&
				dstEntity.walk.index < dstEntity.walk.total
			) {
				dstEntity.setAction({
					action: dstEntity.ACTION.WALK,
					frame: 0,
					repeat: false,
					play: true
				});
			}
		}

		for (let i = 0; i < count; i++) {
			if (pkt.damage) {
				Events.setTimeout(impendingAttack, pkt.attackMT + C_MULTIHIT_DELAY * i);
			}
			if (pkt.leftDamage) {
				Events.setTimeout(impendingAttack, pkt.attackMT + C_MULTIHIT_DELAY * 1.75 * i);
			}
		}

		Events.setTimeout(
			resumeWalk,
			pkt.attackMT + C_MULTIHIT_DELAY * (pkt.leftDamage ? 1.75 : 1) * (count - 1) + pkt.attackedMT
		);
	}
}

/**
 * Does player have a Token of Siegfried?
 */
function haveSiegfriedItem() {
	const itemInfo = Inventory.getUI().getItemById(7621);

	if (Session.IsPKZone || Session.IsSiegeMode || Session.IsEventPVPMode) {
		return false;
	} else if (itemInfo && itemInfo.count > 0) {
		return true;
	} else {
		return false;
	}
}

/**
 * Add/remove effect when state changes
 */
function addStateEffect(AID, OldState, NewState, Status, EffectId) {
	// Kinda weird logic, but it is what it is... always remove if present in OldState, and add/re-add if present in NewState

	// Remove
	if (OldState & Status) {
		EffectManager.remove(null, AID, EffectId);
	}

	// Apply
	if (NewState & Status) {
		const EF_Init_Par = {
			effectId: EffectId,
			ownerAID: AID
		};
		EffectManager.spam(EF_Init_Par);
	}
}

/**
 * Handle hat effect packet - add or remove hat effects on entity
 */
function onHatEffects(pkt) {
	const entity = EntityManager.get(pkt.GID);
	if (!entity) {
		return;
	}

	let i, count, hatEffectID, hatEffect;

	// Remove effects when disabled
	if (pkt.enabled !== 1) {
		if (pkt.hatEffectIDs && pkt.hatEffectIDs.length) {
			for (i = 0, count = pkt.hatEffectIDs.length; i < count; ++i) {
				EffectManager.removeHatEffect(pkt.GID, Number(pkt.hatEffectIDs[i]));
				if (entity._hatEffects) {
					delete entity._hatEffects[pkt.hatEffectIDs[i]];
				}
			}
		}
		return;
	}

	// Initialize tracking object
	if (!entity._hatEffects) {
		entity._hatEffects = {};
	}

	// Add new effects from packet
	const effectIds = pkt.hatEffectIDs || [];
	for (i = 0, count = effectIds.length; i < count; ++i) {
		hatEffectID = effectIds[i];

		// Skip if already present
		if (entity._hatEffects[hatEffectID]) {
			continue;
		}

		hatEffect = DB.getHatResource(hatEffectID);
		if (!hatEffect) {
			continue;
		}

		EffectManager.spamHatEffect({
			Init: { ownerAID: pkt.GID, ownerEntity: entity, startTick: Renderer.tick },
			effect: {
				effectID: hatEffectID,
				hatEffectID: hatEffect.hatEffectID,
				file: hatEffect.resourceFileName,
				xOffset: hatEffect.hatEffectPosX,
				yOffset: hatEffect.hatEffectPos,
				isAttachedHead: hatEffect.isAttachedHead,
				isRenderBeforeCharacter: hatEffect.isRenderBeforeCharacter,
				head: hatEffect.isAttachedHead,
				isAdjustPositionWhenShrinkState: hatEffect.isAdjustPositionWhenShrinkState,
				isAdjustSizeWhenShrinkState: hatEffect.isAdjustSizeWhenShrinkState
			}
		});
	}
}

function processBlockStatus(entity, pkt) {
	const guardStatuses = [StatusConst.KYRIE, StatusConst.AUTOGUARD, StatusConst.PARRYING];

	if (guardStatuses.includes(pkt.index)) {
		if (!entity._guardStatuses) entity._guardStatuses = new Set();
		if (pkt.state && !entity._guardStatuses.has(pkt.index)) {
			entity._guardStatuses.add(pkt.index);
		} else if (!pkt.state) {
			entity._guardStatuses.delete(pkt.index);
		}
		entity.isGuard = entity._guardStatuses.size > 0;
	}
}
/**
 * Initialize
 */
export default function EntityEngine() {
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_ACTENTRY, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY_NPC, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY2, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY2, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY2, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY3, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY3, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY3, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY4, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY4, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY4, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY5, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY5, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY5, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY6, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY6, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY6, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY7, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY8, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY9, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY10, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY10, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY10, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_NEWENTRY11, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVEENTRY11, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_STANDENTRY11, onEntitySpam);
	Network.hookPacket(PACKET.ZC.STORE_ASSISTANT_ENTRY, onEntitySpam);
	Network.hookPacket(PACKET.ZC.STORE_ASSISTANT_ENTRY_V2, onEntitySpam);
	Network.hookPacket(PACKET.ZC.NOTIFY_VANISH, onEntityVanish);
	Network.hookPacket(PACKET.ZC.STORE_ASSISTANT_DISAPPEAR, onEntityVanish);
	Network.hookPacket(PACKET.ZC.NOTIFY_MOVE, onEntityMove);
	Network.hookPacket(PACKET.ZC.STOPMOVE, onEntityStopMove);
	Network.hookPacket(PACKET.ZC.NOTIFY_ACT, onEntityAction);
	Network.hookPacket(PACKET.ZC.NOTIFY_ACT2, onEntityAction);
	Network.hookPacket(PACKET.ZC.NOTIFY_ACT3, onEntityAction);
	Network.hookPacket(PACKET.ZC.NOTIFY_ACT_POSITION, onEntityActionPosition);
	Network.hookPacket(PACKET.ZC.NOTIFY_CHAT, onEntityTalk);
	Network.hookPacket(PACKET.ZC.SHOWSCRIPT, onEntityTalk);
	Network.hookPacket(PACKET.ZC.NPC_CHAT, onEntityTalkColor);
	Network.hookPacket(PACKET.ZC.ACK_REQNAME, onEntityIdentity);
	Network.hookPacket(PACKET.ZC.ACK_REQNAMEALL, onEntityIdentity);
	Network.hookPacket(PACKET.ZC.ACK_REQNAMEALL2, onEntityIdentity);
	Network.hookPacket(PACKET.ZC.ACK_REQNAMEALL3, onEntityIdentity);
	Network.hookPacket(PACKET.ZC.CHANGE_DIRECTION, onEntityDirectionChange);
	Network.hookPacket(PACKET.ZC.SPRITE_CHANGE, onEntityViewChange);
	Network.hookPacket(PACKET.ZC.SPRITE_CHANGE2, onEntityViewChange);
	Network.hookPacket(PACKET.ZC.NPCSPRITE_CHANGE, onNPCViewChange);
	Network.hookPacket(PACKET.ZC.USE_SKILL, onEntityUseSkill);
	Network.hookPacket(PACKET.ZC.USE_SKILL2, onEntityUseSkill);
	Network.hookPacket(PACKET.ZC.NOTIFY_SKILL, onEntityUseSkillToAttack);
	Network.hookPacket(PACKET.ZC.NOTIFY_SKILL2, onEntityUseSkillToAttack);
	Network.hookPacket(PACKET.ZC.NOTIFY_SKILL_POSITION, onEntityUseSkillToAttack);
	Network.hookPacket(PACKET.ZC.USESKILL_ACK, onEntityCastSkill);
	Network.hookPacket(PACKET.ZC.USESKILL_ACK2, onEntityCastSkill);
	Network.hookPacket(PACKET.ZC.USESKILL_ACK3, onEntityCastSkill); // New "Use Skill" packet. Fixes issues with Casting.
	Network.hookPacket(PACKET.ZC.STATE_CHANGE, onEntityOptionChange);
	Network.hookPacket(PACKET.ZC.STATE_CHANGE3, onEntityOptionChange);
	Network.hookPacket(PACKET.ZC.MSG_STATE_CHANGE, onEntityStatusChange);
	Network.hookPacket(PACKET.ZC.MSG_STATE_CHANGE2, onEntityStatusChange);
	Network.hookPacket(PACKET.ZC.MSG_STATE_CHANGE3, onEntityStatusChange);
	Network.hookPacket(PACKET.ZC.MSG_STATE_CHANGE4, onEntityStatusChange);
	Network.hookPacket(PACKET.ZC.MSG_STATE_CHANGE5, onEntityStatusChange);
	Network.hookPacket(PACKET.ZC.STORE_ENTRY, onEntityCreateRoom);
	Network.hookPacket(PACKET.ZC.DISAPPEAR_ENTRY, onEntityDestroyRoom);
	Network.hookPacket(PACKET.ZC.BUYING_STORE_ENTRY, onEntityCreateRoom);
	Network.hookPacket(PACKET.ZC.DISAPPEAR_BUYING_STORE_ENTRY, onEntityDestroyRoom);
	Network.hookPacket(PACKET.ZC.ROOM_NEWENTRY, onEntityCreateRoom);
	Network.hookPacket(PACKET.ZC.DESTROY_ROOM, onEntityDestroyRoom);
	Network.hookPacket(PACKET.ZC.SKILL_ENTRY, onSkillAppear);
	Network.hookPacket(PACKET.ZC.SKILL_ENTRY2, onSkillAppear);
	Network.hookPacket(PACKET.ZC.SKILL_ENTRY3, onSkillAppear);
	Network.hookPacket(PACKET.ZC.SKILL_ENTRY4, onSkillAppear);
	Network.hookPacket(PACKET.ZC.SKILL_ENTRY5, onSkillAppear);
	Network.hookPacket(PACKET.ZC.SKILL_DISAPPEAR, onSkillDisapear);
	Network.hookPacket(PACKET.ZC.DISPEL, onEntityCastCancel);
	Network.hookPacket(PACKET.ZC.HIGHJUMP, onEntityJump);
	Network.hookPacket(PACKET.ZC.FASTMOVE, onEntityFastMove);
	Network.hookPacket(PACKET.ZC.RESURRECTION, onEntityResurect);
	Network.hookPacket(PACKET.ZC.EMOTION, onEntityEmotion);
	Network.hookPacket(PACKET.ZC.NOTIFY_MONSTER_HP, onEntityLifeUpdate);
	Network.hookPacket(PACKET.ZC.HP_INFO_TINY, onEntityLifeUpdateTiny);
	Network.hookPacket(PACKET.ZC.QUEST_NOTIFY_EFFECT, onEntityQuestNotifyEffect);
	Network.hookPacket(PACKET.ZC.BLADESTOP, onBladeStopPacket);
	Network.hookPacket(PACKET.ZC.NOTIFY_EXP, onNotifyExp);
	Network.hookPacket(PACKET.ZC.NOTIFY_EXP2, onNotifyExp);
	Network.hookPacket(PACKET.ZC.BOSS_INFO, onMarkMvp);
	Network.hookPacket(PACKET.ZC.MVP, onEntityMvpReward);
	Network.hookPacket(PACKET.ZC.MVP_GETTING_ITEM, onEntityMvpRewardItemMessage);
	Network.hookPacket(PACKET.ZC.ACK_CHANGE_TITLE, onTitleChangeAck);
	Network.hookPacket(PACKET.ZC.HAT_EFFECT, onHatEffects);
}

export { onEntityActionPosition, onEntityAction };
