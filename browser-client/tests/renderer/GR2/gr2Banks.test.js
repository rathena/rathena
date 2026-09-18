import { describe, it, expect } from 'vitest';
import { bankPathsFor } from 'Renderer/GR2/gr2Banks.js';

describe('gr2Banks.bankPathsFor', () => {
	it('guardians (setId 7/8/9) declare all four external banks', () => {
		const banks = bankPathsFor('data/model/3dmob/sguardian90_9.gr2');
		expect(banks.map(b => b.name).sort()).toEqual(['attack', 'damage', 'dead', 'move']);
		expect(banks).toContainEqual({ name: 'move', path: 'data/model/3dmob_bone/9_move.gr2' });
		expect(banks).toContainEqual({ name: 'attack', path: 'data/model/3dmob_bone/9_attack.gr2' });
		// setId comes from the filename suffix.
		expect(bankPathsFor('data/model/3dmob/kguardian90_7.gr2')[0].path).toContain('3dmob_bone/7_');
		expect(bankPathsFor('data/model/3dmob/aguardian90_8.gr2')[0].path).toContain('3dmob_bone/8_');
	});

	it('treasure box (setId 2) declares only dead + damage', () => {
		const banks = bankPathsFor('data/model/3dmob/treasurebox_2.gr2');
		expect(banks.map(b => b.name).sort()).toEqual(['damage', 'dead']);
		expect(banks).toContainEqual({ name: 'dead', path: 'data/model/3dmob_bone/2_dead.gr2' });
	});

	it('emperium (0) and flag (1) declare no external banks (embedded standby only)', () => {
		expect(bankPathsFor('data/model/3dmob/empelium90_0.gr2')).toEqual([]);
		expect(bankPathsFor('data/model/3dmob/guildflag90_1.gr2')).toEqual([]);
	});

	it('a path without a numeric setId suffix yields no banks', () => {
		expect(bankPathsFor('data/model/3dmob/dragon_5.gr2')).toEqual([]);
		expect(bankPathsFor('nonsense.gr2')).toEqual([]);
	});

	it('resolves the bank set through the string setId key (regex captures a string)', () => {
		// setId is captured as a string from the filename regex, and GR2_BANKS is keyed by strings
		// to match. Guards against a future numeric-only-key or Map swap where a string lookup would
		// miss: a declared setId ('9') must still resolve, an undeclared one ('0') must stay empty.
		expect(bankPathsFor('data/model/3dmob/sguardian90_9.gr2').length).toBeGreaterThan(0);
		expect(bankPathsFor('data/model/3dmob/empelium90_0.gr2')).toEqual([]);
	});
});
