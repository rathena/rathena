import { beforeEach, describe, expect, it, vi } from 'vitest';

const mocks = vi.hoisted(() => {
	const ids = {
		NOVICE: 0,
		CRUSADER: 14,
		DIVINE_PROTECTION: 22,
		DEMON_BANE: 23,
		HEAL: 28,
		CURE: 35,
		FAITH: 248
	};

	const skillInfo = {
		[ids.FAITH]: {
			Name: 'CR_TRUST',
			SkillName: 'Faith',
			MaxLv: 10,
			bSeperateLv: false
		},
		[ids.DEMON_BANE]: {
			Name: 'AL_DEMONBANE',
			SkillName: 'Demon Bane',
			MaxLv: 10,
			bSeperateLv: false,
			_NeedSkillList: [[ids.DIVINE_PROTECTION, 3]]
		},
		[ids.CURE]: {
			Name: 'AL_CURE',
			SkillName: 'Cure',
			MaxLv: 1,
			bSeperateLv: false,
			NeedSkillList: {
				[ids.CRUSADER]: [[ids.FAITH, 5]]
			}
		},
		[ids.DIVINE_PROTECTION]: {
			Name: 'AL_DP',
			SkillName: 'Divine Protection',
			MaxLv: 10,
			bSeperateLv: false,
			NeedSkillList: {
				[ids.CRUSADER]: [[ids.CURE, 1]]
			}
		},
		[ids.HEAL]: {
			Name: 'AL_HEAL',
			SkillName: 'Heal',
			MaxLv: 10,
			bSeperateLv: true,
			NeedSkillList: {
				[ids.CRUSADER]: [
					[ids.FAITH, 10],
					[ids.DEMON_BANE, 5]
				]
			}
		}
	};

	const skillTreeView = {
		[ids.NOVICE]: {
			list: 1,
			beforeJob: null
		},
		[ids.CRUSADER]: {
			list: 2,
			beforeJob: ids.NOVICE,
			[ids.FAITH]: 0,
			[ids.CURE]: 7,
			[ids.DIVINE_PROTECTION]: 14,
			[ids.DEMON_BANE]: 21,
			[ids.HEAL]: 28
		}
	};

	class MockGUIComponent {
		constructor() {
			this._host = document.createElement('div');
			this.ui = {
				show: vi.fn(() => {
					this._host.style.display = 'block';
				}),
				hide: vi.fn(() => {
					this._host.style.display = 'none';
				}),
				is: vi.fn(() => this._host.style.display !== 'none')
			};
		}

		getRoot() {
			return this._host;
		}

		draggable() {}

		focus() {}

		parseHTML() {}
	}

	return {
		ids,
		skillInfo,
		skillTreeView,
		MockGUIComponent,
		session: {
			Character: { job: ids.CRUSADER },
			Entity: { _job: ids.CRUSADER }
		}
	};
});

vi.mock('DB/DBManager.js', () => ({ default: { INTERFACE_PATH: 'data/texture/À¯ÀúÀÎÅÍÆäÀÌ½º/' } }));
vi.mock('DB/Skills/SkillInfo.js', () => ({ default: mocks.skillInfo }));
vi.mock('DB/Skills/SkillTreeView.js', () => ({ default: mocks.skillTreeView }));
vi.mock('Engine/SessionStorage.js', () => ({ default: mocks.session }));
vi.mock('Core/Client.js', () => ({
	default: {
		loadFile(_path, callback) {
			callback?.('data:image/gif;base64,R0lGODlhAQABAIAAAAUEBA==');
		}
	}
}));
vi.mock('Core/Preferences.js', () => ({
	default: {
		get() {
			return {
				x: 0,
				y: 0,
				width: 17,
				height: 12,
				show: true,
				mini: false,
				skillInfo: false,
				save: vi.fn()
			};
		}
	}
}));
vi.mock('Renderer/Renderer.js', () => ({ default: { width: 1200, height: 800 } }));
vi.mock('Controls/MouseEventHandler.js', () => ({ default: { screen: { x: 0, y: 0 } } }));
vi.mock('UI/GUIComponent.js', () => ({ default: mocks.MockGUIComponent }));
vi.mock('UI/UIManager.js', () => ({
	default: {
		addComponent(component) {
			component.getRoot().innerHTML = component.render();
			component.init();
			return component;
		}
	}
}));
vi.mock('UI/Elements/Elements.js', () => ({}));
vi.mock('UI/Components/SkillTargetSelection/SkillTargetSelection.js', () => ({
	default: {
		TYPE: { SELF: 1, TARGET: 2 },
		append: vi.fn(),
		set: vi.fn()
	}
}));
vi.mock('UI/Components/SkillDescription/SkillDescription.js', () => ({
	default: {
		uid: null,
		append: vi.fn(),
		remove: vi.fn(),
		setSkill: vi.fn()
	}
}));

const { createSkillList } = await import('UI/Components/SkillList/SkillListCommon.js');

function getFixtureHTML() {
	const cells = positions => positions.map(position => `<div class="skillCol s${position}"></div>`).join('');

	return `
		<div id="SkillListV2Test">
			<div class="titlebar">
				<button class="base"></button>
				<span class="text"></span>
				<button class="base mini"></button>
				<button class="base close"></button>
				<input type="checkbox" class="view_skill_info" />
			</div>
			<div class="content">
				<button class="btn levelup"></button>
				<table id="minitab1"></table>
				<table id="minitab2"></table>
			</div>
			<div class="contentbig">
				<input type="radio" id="tab-1" class="tab-switch" checked />
				<input type="radio" id="tab-2" class="tab-switch" />
				<div id="positionSkills1">${cells([0, 7, 14, 21, 28])}</div>
				<div id="positionSkills2">${cells([0, 7, 14, 21, 28])}</div>
			</div>
			<div class="footer">
				<span class="skpoints_count"></span>
				<button class="btn apply"></button>
				<button class="btn reset"></button>
				<div class="extend"></div>
			</div>
		</div>`;
}

function createComponent({ faith = 10, points = 10 } = {}) {
	const component = createSkillList({
		name: 'SkillListV2Test',
		htmlText: getFixtureHTML(),
		cssText: '',
		hasTabs: true,
		guardMissingJob: true,
		readdSkillOnUpdate: true
	});

	component.setSkills([
		{ SKID: mocks.ids.FAITH, level: faith, type: 0, upgradable: faith < 10, spcost: 0 },
		{ SKID: mocks.ids.CURE, level: 0, type: 1, upgradable: true, spcost: 15 },
		{ SKID: mocks.ids.DIVINE_PROTECTION, level: 0, type: 0, upgradable: true, spcost: 0 },
		{ SKID: mocks.ids.DEMON_BANE, level: 0, type: 0, upgradable: true, spcost: 0 },
		{ SKID: mocks.ids.HEAL, level: 0, type: 1, upgradable: true, spcost: 13 }
	]);
	component.setPoints(points);
	return component;
}

function getTreeSkill(root, skillId) {
	return root.querySelector(`#positionSkills2 .skill.id${skillId}`);
}

describe('SkillListV2 prerequisite planning', () => {
	beforeEach(() => {
		document.body.innerHTML = '';
	});

	it('shows and transactionally stages the complete Heal chain', () => {
		const component = createComponent();
		const root = component.getRoot();
		const heal = getTreeSkill(root, mocks.ids.HEAL);

		heal.querySelector('.icon').dispatchEvent(new MouseEvent('mouseover', { bubbles: true }));

		expect(root.querySelector('#positionSkills2 .s0 .counterSkill').textContent).toBe('10');
		expect(root.querySelector('#positionSkills2 .s7 .counterSkill').textContent).toBe('1');
		expect(root.querySelector('#positionSkills2 .s14 .counterSkill').textContent).toBe('3');
		expect(root.querySelector('#positionSkills2 .s21 .counterSkill').textContent).toBe('5');

		heal.querySelector('.icon').click();

		expect(getTreeSkill(root, mocks.ids.FAITH).querySelector('.current').textContent).toBe('10');
		expect(getTreeSkill(root, mocks.ids.CURE).querySelector('.current').textContent).toBe('1');
		expect(getTreeSkill(root, mocks.ids.DIVINE_PROTECTION).querySelector('.current').textContent).toBe('3');
		expect(getTreeSkill(root, mocks.ids.DEMON_BANE).querySelector('.current').textContent).toBe('5');
		expect(getTreeSkill(root, mocks.ids.HEAL).querySelector('.current').textContent).toBe('1');
		expect(root.querySelector('.skpoints_count').textContent).toBe('0/10');
	});

	it('leaves the tree unchanged when the complete chain is unaffordable', () => {
		const component = createComponent({ points: 6 });
		const root = component.getRoot();

		getTreeSkill(root, mocks.ids.HEAL).querySelector('.icon').click();

		expect(getTreeSkill(root, mocks.ids.DEMON_BANE).querySelector('.current').textContent).toBe('0');
		expect(getTreeSkill(root, mocks.ids.HEAL).querySelector('.current').textContent).toBe('0');
		expect(root.querySelector('.skpoints_count').textContent).toBe('6');
	});

	it('applies prerequisites first and immediately restores authoritative levels', () => {
		const component = createComponent();
		const root = component.getRoot();
		component.onIncreaseSkill = vi.fn();

		getTreeSkill(root, mocks.ids.HEAL).querySelector('.icon').click();
		root.querySelector('.apply').click();

		expect(component.onIncreaseSkill.mock.calls.map(([skillId]) => skillId)).toEqual([
			mocks.ids.CURE,
			...Array(3).fill(mocks.ids.DIVINE_PROTECTION),
			...Array(5).fill(mocks.ids.DEMON_BANE),
			mocks.ids.HEAL
		]);
		expect(getTreeSkill(root, mocks.ids.CURE).querySelector('.current').textContent).toBe('0');
		expect(getTreeSkill(root, mocks.ids.DIVINE_PROTECTION).querySelector('.current').textContent).toBe('0');
		expect(getTreeSkill(root, mocks.ids.DEMON_BANE).querySelector('.current').textContent).toBe('0');
		expect(getTreeSkill(root, mocks.ids.HEAL).querySelector('.current').textContent).toBe('0');
		expect(root.querySelector('.skpoints_count').textContent).toBe('10');
	});

	it('resets the correct tab and discards the plan on close and reopen', () => {
		const component = createComponent();
		const root = component.getRoot();
		const stageHeal = () => getTreeSkill(root, mocks.ids.HEAL).querySelector('.icon').click();

		stageHeal();
		root.querySelector('.reset').click();

		expect(getTreeSkill(root, mocks.ids.DEMON_BANE).querySelector('.current').textContent).toBe('0');
		expect(getTreeSkill(root, mocks.ids.HEAL).querySelector('.current').textContent).toBe('0');
		expect(root.querySelector('.skpoints_count').textContent).toBe('10');
		expect(root.querySelector('#positionSkills1 .s28').children).toHaveLength(0);

		stageHeal();
		root.querySelector('.close').click();

		expect(component.ui.hide).toHaveBeenCalledOnce();
		expect(getTreeSkill(root, mocks.ids.HEAL).querySelector('.current').textContent).toBe('0');
		expect(root.querySelector('.skpoints_count').textContent).toBe('10');

		component.toggle();
		expect(getTreeSkill(root, mocks.ids.HEAL).querySelector('.current').textContent).toBe('0');
	});
});
