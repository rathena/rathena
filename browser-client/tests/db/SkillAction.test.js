import { describe, it, expect, vi } from 'vitest';

vi.hoisted(() => {
	if (typeof globalThis.localStorage === 'undefined' || typeof globalThis.localStorage.getItem !== 'function') {
		const store = {};
		globalThis.localStorage = {
			getItem: (key) => (key in store ? store[key] : null),
			setItem: (key, val) => {
				store[key] = String(val);
			},
			removeItem: (key) => {
				delete store[key];
			},
			clear: () => {
				for (const k in store) delete store[k];
			}
		};
	}
});

import DB from 'DB/DBManager.js';
import JobId from 'DB/Jobs/JobConst.js';
import WeaponType from 'DB/Items/WeaponType.js';
import SK from 'DB/Skills/SkillConst.js';
import SkillAction from 'DB/Skills/SkillAction.js';

describe('SkillAction & Attack Slices', () => {
	it('identifies Monk jobs correctly in DB.isMonk', () => {
		expect(DB.isMonk(JobId.MONK)).toBe(true);
		expect(DB.isMonk(JobId.MONK_H)).toBe(true);
		expect(DB.isMonk(JobId.MONK_B)).toBe(true);
		expect(DB.isMonk(JobId.SURA)).toBe(true);
		expect(DB.isMonk(JobId.SURA_H)).toBe(true);
		expect(DB.isMonk(JobId.SURA_B)).toBe(true);
		expect(DB.isMonk(JobId.INQUISITOR)).toBe(true);

		expect(DB.isMonk(JobId.KNIGHT)).toBe(false);
		expect(DB.isMonk(JobId.PRIEST)).toBe(false);
	});

	it('returns correct attack slice for Monk / Champion / Sura with knuckles or bare-handed', () => {
		const knuckleSlice = DB.getAttackSlice(JobId.MONK, WeaponType.KNUKLE);
		expect(knuckleSlice).toEqual({ frame: 0, length: 5 });

		const bareHandedSlice = DB.getAttackSlice(JobId.MONK_H, WeaponType.NONE);
		expect(bareHandedSlice).toEqual({ frame: 0, length: 5 });

		const suraSlice = DB.getAttackSlice(JobId.SURA, WeaponType.KNUKLE);
		expect(suraSlice).toEqual({ frame: 0, length: 5 });
	});

	it('returns correct attack slice for Doram / Summoner', () => {
		const doramSlice = DB.getAttackSlice(JobId.DO_SUMMONER, WeaponType.NONE);
		expect(doramSlice).toEqual({ frame: 0, length: 4 });
	});

	it('returns null for standard single-sheet attack classes', () => {
		const knightSlice = DB.getAttackSlice(JobId.KNIGHT, WeaponType.SWORD);
		expect(knightSlice).toBeNull();

		const priestSlice = DB.getAttackSlice(JobId.PRIEST, WeaponType.MACE);
		expect(priestSlice).toBeNull();
	});

	it('defaults Monk generic skills to IDLE via DEFAULT_MONK and mapped IDLE skills', () => {
		const mockEntity = {
			ACTION: { ATTACK: 5, READYFIGHT: 4, IDLE: 0, SKILL: 12 }
		};

		const defaultMonkAction = SkillAction['DEFAULT_MONK'](mockEntity, 1000);
		expect(defaultMonkAction.action).toBe(0);

		// Monk buff/stance skills stay in IDLE
		const callSpirits = SkillAction[SK.MO_CALLSPIRITS](mockEntity, 1000);
		expect(callSpirits.action).toBe(0);

		const absorbSpirits = SkillAction[SK.MO_ABSORBSPIRITS](mockEntity, 1000);
		expect(absorbSpirits.action).toBe(0);

		const steelBody = SkillAction[SK.MO_STEELBODY](mockEntity, 1000);
		expect(steelBody.action).toBe(0);

		const snap = SkillAction[SK.MO_BODYRELOCATION](mockEntity, 1000);
		expect(snap.action).toBe(0);

		const fury = SkillAction[SK.MO_EXPLOSIONSPIRITS](mockEntity, 1000);
		expect(fury.action).toBe(0);

		const risingDragon = SkillAction[SK.SR_RAISINGDRAGON](mockEntity, 1000);
		expect(risingDragon.action).toBe(0);
	});

	it('maps AL_INCAGI to IDLE and AL_BLESSING to SKILL per canonical C++ client source', () => {
		const mockEntity = {
			ACTION: { ATTACK: 5, READYFIGHT: 4, IDLE: 0, SKILL: 12 }
		};

		const incAgi = SkillAction[SK.AL_INCAGI](mockEntity, 1000);
		expect(incAgi.action).toBe(0); // ACTION.IDLE

		const blessing = SkillAction[SK.AL_BLESSING](mockEntity, 1000);
		expect(blessing.action).toBe(12); // ACTION.SKILL
	});

	it('defines correct motion slices for Monk combo skills', () => {
		const mockEntity = {
			ACTION: { ATTACK: 5, READYFIGHT: 4, IDLE: 0 }
		};

		const tripleAtk = SkillAction[SK.MO_TRIPLEATTACK](mockEntity, 1000);
		expect(tripleAtk.frame).toBe(5);
		expect(tripleAtk.length).toBe(4);
		expect(tripleAtk.action).toBe(5);

		const chainCombo = SkillAction[SK.MO_CHAINCOMBO](mockEntity, 1000);
		expect(chainCombo.frame).toBe(9);
		expect(chainCombo.length).toBe(4);

		const comboFinish = SkillAction[SK.MO_COMBOFINISH](mockEntity, 1000);
		expect(comboFinish.frame).toBe(13);
		expect(comboFinish.length).toBe(2);

		const extremityFist = SkillAction[SK.MO_EXTREMITYFIST](mockEntity, 1000);
		expect(extremityFist.frame).toBe(13);
		expect(extremityFist.length).toBe(2);
	});
});
