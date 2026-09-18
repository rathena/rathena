/**
 * UI/Components/SkillList/SkillListCommon.js
 *
 * Shared factory for all SkillList versions (SkillList, SkillListV2) and the
 * Homunculus/Mercenary SkillListMH windows.
 *
 * Chararacter Skill Window
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import 'UI/Elements/Elements.js';

import Client from 'Core/Client.js';
import DB from 'DB/DBManager.js';
import GUIComponent from 'UI/GUIComponent.js';
import Mouse from 'Controls/MouseEventHandler.js';
import Preferences from 'Core/Preferences.js';
import Renderer from 'Renderer/Renderer.js';
import Session from 'Engine/SessionStorage.js';
import SkillDescription from 'UI/Components/SkillDescription/SkillDescription.js';
import SkillInfo from 'DB/Skills/SkillInfo.js';
import SkillTargetSelection from 'UI/Components/SkillTargetSelection/SkillTargetSelection.js';
import SkillTreeView from 'DB/Skills/SkillTreeView.js';
import {
	createSkillUpgradeOrder,
	resolveSkillRequirements,
	stageSkillPlan
} from 'UI/Components/SkillList/SkillRequirements.js';
import UIManager from 'UI/UIManager.js';

function _escapeHTML(text) {
	const div = document.createElement('div');
	div.textContent = text;
	return div.innerHTML;
}

function _isNumeric(val) {
	return !isNaN(parseFloat(val)) && isFinite(val);
}

export function createSkillList({
	name,
	htmlText,
	cssText,
	hasTabs = false,
	showDescOnMiniHover = false,
	touchDrag = false,
	guardMissingJob = false,
	readdSkillOnUpdate = false,
	listOnly = false,
	dragFrom = null,
	titlebarText = null,
	containerSelector = null,
	preferenceDefaults = {
		x: 100,
		y: 200,
		width: 8,
		height: 8,
		show: false,
		mini: true,
		skillInfo: false
	}
}) {
	const Component = new GUIComponent(name, cssText);
	Component.render = () => htmlText;

	const _dragFrom = dragFrom ?? name;
	const _containerSelector = containerSelector ?? `#${name}`;

	const _preferences = Preferences.get(name, preferenceDefaults, 1.0);

	const _list = [];
	let _btnIncSkill;
	let _points = 0;
	let totalCounter = 0;
	let _btnLevelUp;
	let _lArrow, _rArrow;
	let skillPosition = [];
	const skillDependencyTree = [];
	let skillJobId = null;
	let rememberChoice = new Map();
	const hasSkills = [];
	let _justDragged = false;

	const _touchDrag = {
		timer: null,
		dragging: false,
		ghost: null,
		startX: 0,
		startY: 0
	};

	Component.init = function init() {
		const root = this.getRoot();

		if (titlebarText) {
			const titleEl = root.querySelector('.titlebar .text');
			if (titleEl) {
				titleEl.textContent = titlebarText;
			}
		}

		root.querySelector('.titlebar .base')?.addEventListener('mousedown', e => {
			e.stopImmediatePropagation();
		});
		root.querySelector('.footer .extend')?.addEventListener('mousedown', e => {
			onResize(e, this);
		});
		root.querySelector('.titlebar .close')?.addEventListener('click', () => {
			onResetChoice(this);
			this.ui.hide();
		});
		root.querySelector('.titlebar .mini')?.addEventListener('click', () => {
			onMini(this);
		});
		root.querySelector('.view_skill_info')?.addEventListener('change', function () {
			_preferences.skillInfo = !!this.checked;
			_preferences.save();
		});
		if (!listOnly) {
			root.querySelector('.reset')?.addEventListener('click', () => {
				onResetChoice(this);
			});
			root.querySelector('.apply')?.addEventListener('click', () => {
				onApplyChoice(this);
			});
		}

		const levelupBtn = root.querySelector('.btn.levelup');
		if (levelupBtn) {
			_btnIncSkill = levelupBtn.cloneNode(true);
			levelupBtn.remove();
			_btnIncSkill.addEventListener('click', function () {
				const index = this.parentNode.parentNode.getAttribute('data-index');
				Component.onIncreaseSkill(parseInt(index, 10));
			});
		}

		const lvlupBtn = root.querySelector('#lvlup_job');
		if (lvlupBtn) {
			_btnLevelUp = lvlupBtn;
			if (!listOnly) {
				_btnLevelUp.style.zIndex = '51';
				_btnLevelUp.style.position = 'absolute';
				_btnLevelUp.style.right = '0px';
				_btnLevelUp.style.bottom = '0px';
				_btnLevelUp.style.width = '43px';
				_btnLevelUp.style.height = '43px';
				_btnLevelUp.style.border = 'none';
				_btnLevelUp.style.backgroundColor = 'transparent';
				_btnLevelUp.style.backgroundRepeat = 'no-repeat';
			}
			_btnLevelUp.remove();
			_btnLevelUp.addEventListener('click', () => {
				if (_btnLevelUp.parentNode) {
					_btnLevelUp.remove();
				}
				Component.ui.show();
			});
			_btnLevelUp.addEventListener('mousedown', e => {
				e.stopImmediatePropagation();
			});
		}

		const container = root.querySelector(_containerSelector) || root;

		container.addEventListener('dblclick', e => {
			const target = e.target.closest('.skill .icon, .skill .name');
			if (target) {
				let main = target.parentElement;
				if (!main.classList.contains('skill')) {
					main = main.parentElement;
				}
				Component.useSkillID(parseInt(main.getAttribute('data-index'), 10));
			}
		});

		container.addEventListener('contextmenu', e => {
			const target = e.target.closest('.skill .icon, .skill .name');
			if (target) {
				const skillID = _resolveSkillID(target);
				if (SkillDescription.uid === skillID) {
					SkillDescription.remove();
					return;
				}
				SkillDescription.append();
				SkillDescription.setSkill(skillID);
			}
		});

		container.addEventListener('mousedown', e => {
			const target = e.target.closest('.selectable');
			if (target) {
				let main = target.parentElement;
				if (!main.classList.contains('skill')) {
					main = main.parentElement;
				}
				for (const el of root.querySelectorAll('.skill')) {
					el.classList.remove('selected');
				}
				main.classList.add('selected');
			}
		});

		if (!listOnly) {
			container.addEventListener('mouseover', e => {
				const target = e.target.closest('.skillCol .skill .icon, .skill .name');
				if (target) {
					if (_preferences.skillInfo || (showDescOnMiniHover && _preferences.mini)) {
						const skillID = _resolveSkillID(target);
						if (SkillDescription.uid !== skillID) {
							SkillDescription.append();
							SkillDescription.setSkill(skillID);
						}
					}
					onNecessarySkills(target, root);
				}
			});

			container.addEventListener('mouseout', e => {
				const target = e.target.closest('.skillCol .skill .icon, .skill .name');
				if (target) {
					if (_preferences.skillInfo || (showDescOnMiniHover && _preferences.mini)) {
						SkillDescription.remove();
					}
					root.querySelectorAll('.needleSkill').forEach(el => el.classList.remove('needleSkill'));
					root.querySelectorAll('.counterSkill').forEach(el => el.remove());
				}
			});

			container.addEventListener('click', e => {
				const target = e.target.closest('.skillCol .skill .icon, .skill .name');
				if (target) {
					onRememberChoice(target, root);
				}
			});
		}

		container.addEventListener('dragstart', e => {
			const skillEl = e.target.closest('.skill');
			if (!skillEl) {
				return;
			}
			const index = parseInt(skillEl.getAttribute('data-index'), 10);
			const skill = getSkillById(index);

			if (!skill || !skill.level || !skill.type) {
				e.stopImmediatePropagation();
				e.preventDefault();
				return;
			}

			const img = new Image();
			img.decoding = 'async';
			img.src = skillEl.querySelector('.icon img')?.src || '';

			e.dataTransfer.setDragImage(img, 12, 12);
			e.dataTransfer.setData(
				'Text',
				JSON.stringify(
					(window._OBJ_DRAG_ = {
						type: 'skill',
						from: _dragFrom,
						data: skill
					})
				)
			);
		});

		container.addEventListener('dragend', () => {
			delete window._OBJ_DRAG_;
			_justDragged = true;
			setTimeout(() => {
				_justDragged = false;
			}, 0);
		});

		if (touchDrag) {
			container.addEventListener('touchstart', e => {
				const iconTarget = e.target.closest('.skill .icon');
				if (iconTarget) {
					onSkillTouchStart(e, iconTarget);
				}
			});

			container.addEventListener('touchmove', e => {
				const iconTarget = e.target.closest('.skill .icon');
				if (iconTarget) {
					onSkillTouchMove(e);
				}
			});

			container.addEventListener('touchend', e => {
				const iconTarget = e.target.closest('.skill .icon');
				if (iconTarget) {
					onSkillTouchEnd(e);
				}
			});
		}

		this.draggable('.titlebar');

		Client.loadFile(`${DB.INTERFACE_PATH}basic_interface/arw_right.bmp`, data => {
			_rArrow = `url(${data})`;
		});
		Client.loadFile(`${DB.INTERFACE_PATH}basic_interface/arw_left.bmp`, data => {
			_lArrow = `url(${data})`;
		});
	};

	Component.onAppend = function onAppend() {
		if (!_preferences.show) {
			this.ui.hide();
		}

		resize(this, _preferences.width, _preferences.height);
		this._host.style.top = `${Math.min(Math.max(0, _preferences.y), Renderer.height - 100)}px`;
		this._host.style.left = `${Math.min(Math.max(0, _preferences.x), Renderer.width - 100)}px`;

		const root = this.getRoot();
		const cb = root.querySelector('.view_skill_info');
		if (cb) {
			cb.checked = _preferences.skillInfo;
		}
	};

	Component.onRemove = function onRemove() {
		if (_btnLevelUp && _btnLevelUp.parentNode) {
			_btnLevelUp.remove();
		}

		_preferences.show = this.ui.is(':visible');
		_preferences.y = parseInt(this._host.style.top, 10) || 0;
		_preferences.x = parseInt(this._host.style.left, 10) || 0;

		const root = this.getRoot();
		const content = root.querySelector('.content');
		if (content) {
			_preferences.width = Math.floor(parseInt(content.style.width, 10) / 32) || preferenceDefaults.width;
			_preferences.height = Math.floor(parseInt(content.style.height, 10) / 32) || preferenceDefaults.height;
		}
		_preferences.save();
	};

	Component.toggle = function toggle() {
		if (this.ui.is(':visible')) {
			onResetChoice(this);
			this.ui.hide();
			if (_btnLevelUp && _btnLevelUp.parentNode) {
				_btnLevelUp.remove();
			}
		} else {
			this.ui.show();
			this.focus();
		}
	};

	Component.onShortCut = function onShortCut(key) {
		switch (key.cmd) {
			case 'TOGGLE':
				this.toggle();
				break;
		}
	};

	Component.setSkills = function setSkills(skills) {
		const root = this.getRoot();

		if (listOnly) {
			for (let i = 0, count = _list.length; i < count; ++i) {
				this.onUpdateSkill(_list[i].SKID, 0);
			}

			_list.length = 0;
			const table = root.querySelector('.content table');
			if (table) {
				table.innerHTML = '';
			}

			for (let i = 0, count = skills.length; i < count; ++i) {
				this.addSkill(skills[i]);
			}
			return;
		}

		root.querySelectorAll('.upgradable').forEach(el => el.classList.remove('upgradable'));

		// The skill tree follows the real job, never a transformation/disguise
		const entity = Session.Entity;
		skillJobId = entity ? entity._job || entity.job : 0;

		skillPosition = getSkillPosition(skillJobId);
		skillDependencyTree.length = 0;
		createSkillDependencyTree();

		for (let i = 0, count = _list.length; i < count; ++i) {
			this.onUpdateSkill(_list[i].SKID, 0);
		}

		_list.length = 0;
		hasSkills.length = 0;
		if (hasTabs) {
			root.querySelectorAll('.content table').forEach(t => {
				t.innerHTML = '';
			});
		} else {
			const table = root.querySelector('.content table');
			if (table) {
				table.innerHTML = '';
			}
		}
		root.querySelectorAll('.skillCol').forEach(el => {
			el.innerHTML = '';
		});
		if (hasTabs) {
			root.querySelectorAll('.extraRow').forEach(el => el.remove());
		}

		for (let i = 0, count = skills.length; i < count; ++i) {
			this.addSkill(skills[i]);
			hasSkills[skills[i].SKID] = skills[i];
		}

		skillPosition.forEach((items, list) => {
			Component.prepareSkillTree(items, list);
		});

		if (hasTabs) {
			for (let i = 2; i <= 4; i++) {
				const miniRows = root.querySelectorAll(`#minitab${i} tr`);
				const bigSkills = root.querySelectorAll(`#positionSkills${i} .skill`);

				const tabminil = root.querySelector(`#tabminil${i}`);
				if (tabminil) {
					tabminil.style.display = miniRows.length > 0 ? '' : 'none';
				}

				const tabl = root.querySelector(`#tabl${i}`);
				if (tabl) {
					tabl.style.display = bigSkills.length > 0 ? '' : 'none';
				}
			}
		}

		onResetChoice(this);
	};

	function createSkillDependencyTree() {
		skillPosition.forEach((items, list) => {
			Object.entries(items).forEach(([skid, pos]) => {
				if (!_isNumeric(skid)) {
					return;
				}

				const sk = SkillInfo[skid];

				if (hasTabs) {
					if (sk?.MaxLv) {
						skillDependencyTree[skid] = {
							dependency: [],
							position: pos,
							list: list,
							MaxLv: sk.MaxLv
						};

						resolveSkillRequirements(sk, skillJobId, SkillTreeView).forEach(item => {
							skillDependencyTree[skid]['dependency'][item[0]] = item[1];
						});
					} else {
						console.error('Something wrong with this skill: %d', skid);
					}
				} else {
					skillDependencyTree[skid] = {
						dependency: [],
						position: pos,
						list: undefined,
						MaxLv: sk.MaxLv
					};

					resolveSkillRequirements(sk, skillJobId, SkillTreeView).forEach(item => {
						skillDependencyTree[skid]['dependency'][item[0]] = item[1];
					});
				}
			});
		});
	}

	function highlightNecessarySkill(skillId, count, root) {
		skillPosition.forEach((items, list) => {
			if (items[skillId] !== undefined) {
				const skillbox = root.querySelector(`#positionSkills${list} .s${items[skillId]}`);
				if (skillbox) {
					skillbox.classList.add('needleSkill');
					if (count !== null && count !== undefined) {
						const counterEl = document.createElement('div');
						counterEl.className = 'counterSkill';
						counterEl.textContent = count;
						skillbox.appendChild(counterEl);
					}
				}
			}
		});
	}

	function collectNecessarySkills(skillId, requirements, visiting = new Set()) {
		if (visiting.has(skillId)) {
			return;
		}

		visiting.add(skillId);
		skillDependencyTree[skillId]?.dependency.forEach((level, requiredSkillId) => {
			requirements.set(requiredSkillId, Math.max(requirements.get(requiredSkillId) ?? 0, level));
			collectNecessarySkills(requiredSkillId, requirements, visiting);
		});
		visiting.delete(skillId);
	}

	function onRememberChoice(target, root) {
		if (_justDragged) {
			return;
		}
		let main = target.parentElement;
		if (!main.classList.contains('skill')) {
			main = main.parentElement;
		}
		const skillId = parseInt(main.getAttribute('data-index'), 10);
		const result = stageSkillPlan({
			plan: rememberChoice,
			skillId,
			ownedSkills: hasSkills,
			skillInfo: SkillInfo,
			skillTreeView: SkillTreeView,
			jobId: skillJobId,
			availablePoints: _points
		});
		if (!result) {
			return;
		}

		rememberChoice = result.plan;
		totalCounter = result.cost;
		renderRememberChoice(root);
	}

	function renderRememberChoice(root) {
		rememberChoice.forEach((choice, skillId) => {
			if (choice.isQuest) {
				return;
			}

			const skill = hasSkills[skillId];
			root.querySelectorAll(`.skill.id${skillId}`).forEach(element => {
				element.classList.remove('active', 'passive', 'disabled');
				element.classList.add(skill?.type ? 'active' : 'passive');

				const levelEl = element.querySelector('.level');
				if (levelEl) {
					levelEl.style.display = '';
				}
				element.querySelectorAll('.current, .max').forEach(level => {
					level.textContent = choice.count;
				});
			});
		});

		const skpointsEl = root.querySelector('.skpoints_count');
		if (skpointsEl) {
			skpointsEl.textContent = `${_points - totalCounter}/${_points}`;
		}
	}

	function getSkillPosition(JobId) {
		const positions = [];

		if (guardMissingJob) {
			if (!(JobId in SkillTreeView)) {
				console.error(`Unimplemented JobId ${JobId} in SkillTree!`);
				return positions;
			}
		}

		positions[SkillTreeView[JobId]['list']] = SkillTreeView[JobId];

		if (SkillTreeView[JobId]['beforeJob'] !== null) {
			const beforeJob = SkillTreeView[JobId]['beforeJob'];
			getSkillPosition(beforeJob).forEach((items, list) => {
				positions[list] = positions[list] ? Object.assign(positions[list], items) : items;
			});
		}

		return positions;
	}

	Component.addSkill = function addSkill(skill) {
		if (!(skill.SKID in SkillInfo)) {
			return;
		}

		const root = this.getRoot();
		if (root.querySelector(`.skill.id${skill.SKID}`)) {
			this.updateSkill(skill);
			return;
		}

		if (!listOnly) {
			this.addSkillBig(skill);
		}
		this.addSkillMini(skill);
	};

	Component.prepareSkillTree = function prepareSkillTree(items, list) {
		const root = this.getRoot();
		Object.entries(items).forEach(([key, value]) => {
			const sk = SkillInfo[key];
			const className = 'disabled';
			if (sk !== undefined) {
				const element = document.createElement('div');
				element.className = `skill id${key} ${className}`;
				element.setAttribute('data-index', key);
				element.setAttribute('draggable', 'true');
				element.innerHTML =
					`<div class="name">${_escapeHTML(sk.SkillName).substr(0, 7)}...<br/></div>` +
					'<div class="icon"><img src="data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==" width="24" height="24" /></div>' +
					'<div class=selectable>' +
					'<span class="level" style="display: none">' +
					(sk.bSeperateLv
						? `<button class="currentDown"></button><span class="current">0</span> / <span class="max">0</span><button class="currentUp"></button>`
						: '<span class="current">0</span>') +
					'</span></div>';

				const upBtn = element.querySelector('.level .currentUp');
				if (upBtn) {
					if (_rArrow) {
						upBtn.style.backgroundImage = _rArrow;
					}
					upBtn.addEventListener('click', () => skillLevelSelectUp(0, root));
				}
				const downBtn = element.querySelector('.level .currentDown');
				if (downBtn) {
					if (_lArrow) {
						downBtn.style.backgroundImage = _lArrow;
					}
					downBtn.addEventListener('click', () => skillLevelSelectDown(0, root));
				}

				if (value !== undefined) {
					const box = root.querySelector(`#positionSkills${list} .s${value}`);
					if (box) {
						if (parseInt(key, 10) < 41) {
							box.parentElement.style.display = '';
						}
						if (!box.hasChildNodes()) {
							box.appendChild(element);
						}

						if (hasTabs) {
							const miniBox = root.querySelector(`#minitab${list}`);
							if (miniBox && !miniBox.querySelector(`.skill.id${key}`)) {
								const miniTr = document.createElement('tr');
								miniTr.className = `skill id${key} disabled`;
								miniTr.setAttribute('data-index', key);
								miniTr.innerHTML =
									'<td class="icon"><img src="data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==" width="24" height="24" /></td>' +
									'<td class="levelupcontainer"></td>' +
									'<td class="selectable">' +
									`<div class="name">${_escapeHTML(sk.SkillName)}<br/>` +
									'<span class="level">Lv : <span class="current">0</span></span>' +
									'</div></td>' +
									'<td class="selectable type">' +
									'<div class="consume">Passive</div></td>';

								miniBox.appendChild(miniTr);

								Client.loadFile(`${DB.INTERFACE_PATH}item/${sk.Name}.bmp`, data => {
									const img = miniTr.querySelector('.icon img');
									if (img) {
										img.src = data;
									}
								});
							}
						}
					}
				}

				Client.loadFile(`${DB.INTERFACE_PATH}item/${sk.Name}.bmp`, data => {
					const img = element.querySelector('.icon img');
					if (img) {
						img.src = data;
					}
				});
			}
		});
	};

	Component.addSkillBig = function addSkillBig(skill) {
		const root = this.getRoot();
		const sk = SkillInfo[skill.SKID];
		const className = !skill.level ? 'disabled' : skill.type ? 'active' : 'passive';
		const element = document.createElement('div');
		element.className = `skill id${skill.SKID} ${className}`;
		element.setAttribute('data-index', skill.SKID);
		element.setAttribute('draggable', 'true');
		element.innerHTML =
			`<div class="name">${_escapeHTML(sk.SkillName).substr(0, 7)}...<br/></div>` +
			'<div class="icon"><img src="data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==" width="24" height="24" /></div>' +
			'<div class="levelupcontainer"></div>' +
			'<div class=selectable>' +
			'<span class="level">' +
			(sk.bSeperateLv
				? `<button class="currentDown"></button><span class="current">${skill.level}</span> / <span class="max">${skill.level}</span><button class="currentUp"></button>`
				: `<span class="current">${skill.level}</span>`) +
			'</span></div>';

		const upBtn = element.querySelector('.level .currentUp');
		if (upBtn) {
			if (_rArrow) {
				upBtn.style.backgroundImage = _rArrow;
			}
			upBtn.addEventListener('click', () => skillLevelSelectUp(skill, root));
		}
		const downBtn = element.querySelector('.level .currentDown');
		if (downBtn) {
			if (_lArrow) {
				downBtn.style.backgroundImage = _lArrow;
			}
			downBtn.addEventListener('click', () => skillLevelSelectDown(skill, root));
		}

		skillPosition.forEach((items, list) => {
			if (items[skill.SKID] !== undefined) {
				let box = root.querySelector(`#positionSkills${list} .s${items[skill.SKID]}`);

				if (hasTabs && items[skill.SKID] > 41 && !box) {
					const startPos = items[skill.SKID] - (items[skill.SKID] % 7);
					const rowId = Math.floor(items[skill.SKID] / 7);
					const newRow = document.createElement('div');
					newRow.className = 'skillRow extraRow';
					newRow.setAttribute('data-order', rowId);
					let rowHTML = '';
					for (let c = 0; c < 7; c++) {
						rowHTML += `<div class="skillCol s${startPos + c}"></div>`;
					}
					newRow.innerHTML = rowHTML;

					const container = root.querySelector(`#positionSkills${list}`);
					if (container) {
						const rows = container.querySelectorAll('.skillRow');
						let insertBefore = null;
						for (const row of rows) {
							if (parseInt(row.getAttribute('data-order'), 10) > rowId) {
								insertBefore = row;
								break;
							}
						}
						if (insertBefore) {
							container.insertBefore(newRow, insertBefore);
						} else {
							container.appendChild(newRow);
						}
					}

					box = root.querySelector(`#positionSkills${list} .s${items[skill.SKID]}`);
				}

				if (box) {
					if (box.hasChildNodes()) {
						box.innerHTML = '';
					}
					box.appendChild(element);

					if (skill.upgradable) {
						box.classList.add('upgradable');
					}
				}
			}
		});

		if (hasTabs) {
			if (!root.querySelector(`.contentbig .skill.id${skill.SKID}`)) {
				const etcContainer = root.querySelector('#etcBIG5');
				if (etcContainer) {
					const pos = etcContainer.querySelectorAll('.skill').length;

					if (pos > 41 && (pos - 41) % 7 === 1) {
						if (!root.querySelector(`#etcBIG5 .s${pos}`)) {
							const rowId = Math.floor(pos / 7);
							const newRow = document.createElement('div');
							newRow.className = 'skillRow extraRow';
							newRow.setAttribute('data-order', rowId);
							let rowHTML = '';
							for (let c = 0; c < 7; c++) {
								rowHTML += `<div class="skillCol s${pos + c}"></div>`;
							}
							newRow.innerHTML = rowHTML;
							etcContainer.appendChild(newRow);
						}
					}

					const etcBox = root.querySelector(`#etcBIG5 .s${pos}`);
					if (etcBox) {
						etcBox.appendChild(element);
					}
				}
			}
		}

		Client.loadFile(`${DB.INTERFACE_PATH}item/${sk.Name}.bmp`, data => {
			const img = element.querySelector('.icon img');
			if (img) {
				img.src = data;
			}
		});
	};

	Component.addSkillMini = function addSkillMini(skill) {
		const root = this.getRoot();
		const sk = SkillInfo[skill.SKID];
		const levelup = _btnIncSkill.cloneNode(true);
		levelup.addEventListener('click', function () {
			const index = this.parentNode.parentNode.getAttribute('data-index');
			Component.onIncreaseSkill(parseInt(index, 10));
		});
		const className = !skill.level ? 'disabled' : skill.type ? 'active' : 'passive';
		const tr = document.createElement('tr');
		tr.className = `skill id${skill.SKID} ${className}`;
		tr.setAttribute('data-index', skill.SKID);
		tr.setAttribute('draggable', 'true');
		tr.innerHTML =
			'<td class="icon"><img src="data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==" width="24" height="24" /></td>' +
			'<td class="levelupcontainer"></td>' +
			'<td class=selectable>' +
			`<div class="name">${_escapeHTML(sk.SkillName)}<br/>` +
			'<span class="level">' +
			(sk.bSeperateLv
				? `<button class="currentDown"></button>Lv : <span class="current">${skill.level}</span> / <span class="max">${skill.level}</span><button class="currentUp"></button>`
				: `Lv : <span class="current">${skill.level}</span>`) +
			'</span></div></td>' +
			'<td class="selectable type">' +
			`<div class="consume">${skill.type ? `Sp : <span class="spcost">${skill.spcost}</span>` : 'Passive'}</div>` +
			'</td>';

		if (!skill.upgradable || !_points) {
			levelup.style.display = 'none';
		}

		tr.querySelector('.levelupcontainer').appendChild(levelup);

		const upBtn = tr.querySelector('.level .currentUp');
		if (upBtn) {
			if (_rArrow) {
				upBtn.style.backgroundImage = _rArrow;
			}
			upBtn.addEventListener('click', () => skillLevelSelectUp(skill, root));
		}
		const downBtn = tr.querySelector('.level .currentDown');
		if (downBtn) {
			if (_lArrow) {
				downBtn.style.backgroundImage = _lArrow;
			}
			downBtn.addEventListener('click', () => skillLevelSelectDown(skill, root));
		}

		if (hasTabs) {
			skillPosition.forEach((items, list) => {
				if (items[skill.SKID] !== undefined) {
					const miniTable = root.querySelector(`#minitab${list}`);
					if (miniTable) {
						miniTable.appendChild(tr);
					}
				}
			});

			if (!root.querySelector(`.content .skill.id${skill.SKID}`)) {
				const etcTable = root.querySelector('#minitab5');
				if (etcTable) {
					etcTable.appendChild(tr);
				}
			}
		} else {
			const table = root.querySelector('.content table');
			if (table) {
				table.appendChild(tr);
			}
		}

		this.parseHTML.call(levelup);

		Client.loadFile(`${DB.INTERFACE_PATH}item/${sk.Name}.bmp`, data => {
			const img = tr.querySelector('.icon img');
			if (img) {
				img.src = data;
			}
		});

		_list.push(skill);
		this.onUpdateSkill(skill.SKID, skill.level);
	};

	Component.removeSkill = function removeSkill() {};

	Component.updateSkill = function updateSkill(skill) {
		let target = getSkillById(skill.SKID);

		const root = this.getRoot();
		if (!target) {
			if (readdSkillOnUpdate && root.querySelector(`.skill.id${skill.SKID}`)) {
				_list.push(skill);
				this.onUpdateSkill(skill.SKID, 0);
				hasSkills[skill.SKID] = skill;
				target = skill;
			} else {
				return;
			}
		}

		target.level = skill.level;
		target.spcost = skill.spcost;
		target.attackRange = skill.attackRange;
		target.upgradable = skill.upgradable;
		if (Number.isInteger(skill.type)) {
			target.type = skill.type;
		}

		const elements = root.querySelectorAll(`.skill.id${skill.SKID}`);
		elements.forEach(element => {
			for (const el of element.querySelectorAll('.level .current, .level .max')) {
				el.textContent = skill.level;
			}
			if (skill.selectedLevel) {
				const current = element.querySelector('.level .current');
				if (current) {
					current.textContent = skill.selectedLevel;
				}
			}
			const spcost = element.querySelector('.spcost');
			if (spcost) {
				spcost.textContent = skill.spcost;
			}

			element.classList.remove('active', 'passive', 'disabled');
			element.classList.add(!skill.level ? 'disabled' : skill.type ? 'active' : 'passive');

			const levelupEl = element.querySelector('.levelup');
			if (levelupEl) {
				levelupEl.style.display = skill.upgradable && _points ? '' : 'none';
			}
		});

		this.onUpdateSkill(skill.SKID, skill.level);
	};

	Component.useSkillID = function useSkillID(id, level) {
		const skill = getSkillById(id);
		if (!skill || !skill.level || !skill.type) {
			return;
		}
		Component.useSkill(skill, level ? level : skill.selectedLevel);
	};

	Component.useSkill = function useSkill(skill, level) {
		if (skill.type & SkillTargetSelection.TYPE.SELF) {
			this.onUseSkill(skill.SKID, level ? level : skill.level);
		}

		skill.useLevel = level;

		if (skill.type & SkillTargetSelection.TYPE.TARGET) {
			SkillTargetSelection.append();
			SkillTargetSelection.set(skill, skill.type);
		}
	};

	Component.setPoints = function setPoints(amount) {
		const root = this.getRoot();
		const el = root.querySelector('.skpoints_count');
		if (el) {
			el.textContent = amount;
		}

		if (!_points === !amount) {
			_points = amount;
			return;
		}

		_points = amount;
		const count = _list.length;

		for (let i = 0; i < count; ++i) {
			const levelups = root.querySelectorAll(`.skill.id${_list[i].SKID} .levelup`);
			levelups.forEach(lu => {
				lu.style.display = _list[i].upgradable && amount ? '' : 'none';
			});
		}
	};

	Component.onLevelUp = function onLevelUp() {
		if (_btnLevelUp) {
			document.body.appendChild(_btnLevelUp);
		}
	};

	function getSkillById(id) {
		const count = _list.length;
		for (let i = 0; i < count; ++i) {
			if (_list[i].SKID === id) {
				return _list[i];
			}
		}
		return null;
	}

	function onResize(e, comp) {
		e.stopImmediatePropagation();
		const top = parseInt(comp._host.style.top, 10) || 0;
		const left = parseInt(comp._host.style.left, 10) || 0;
		let lastWidth = 0;
		let lastHeight = 0;

		const resizing = () => {
			const extraX = -6;
			const extraY = 32;

			let w = Math.floor((Mouse.screen.x - left - extraX) / 32);
			let h = Math.floor((Mouse.screen.y - top - extraY) / 32);

			w = Math.min(Math.max(w, 8), 8);
			h = Math.min(Math.max(h, 4), 10);

			if (w === lastWidth && h === lastHeight) {
				return;
			}

			resize(comp, w, h);
			lastWidth = w;
			lastHeight = h;
		};

		const interval = setInterval(resizing, 30);

		const onMouseUp = event => {
			if (event.which === 1) {
				clearInterval(interval);
				window.removeEventListener('mouseup', onMouseUp);
			}
		};
		window.addEventListener('mouseup', onMouseUp);
	}

	function resize(comp, width, height) {
		const root = comp.getRoot();

		if (listOnly) {
			width = Math.min(Math.max(width, 8), 8);
			height = Math.min(Math.max(height, 4), 10);
			const content = root.querySelector('.content');
			if (content) {
				content.style.width = `${width * 32}px`;
				content.style.height = `${height * 32}px`;
			}
			return;
		}

		if (_preferences.mini) {
			width = Math.min(Math.max(width, 8), 8);
			height = Math.min(Math.max(height, 4), 10);
			const extend = root.querySelector('.extend');
			if (extend) {
				extend.style.display = '';
			}
			const content = root.querySelector('.content');
			if (content) {
				content.style.display = '';
			}

			if (hasTabs) {
				const checkedBig = root.querySelector('.tab-switch:checked');
				if (checkedBig) {
					const tabId = checkedBig.id;
					const i = parseInt(tabId.split('-')[1], 10);
					const miniRadio = root.querySelector(`#tab-${i}-mini`);
					if (miniRadio) {
						miniRadio.checked = true;
					}
				}
			}

			const contentbig = root.querySelector('.contentbig');
			if (contentbig) {
				contentbig.style.display = 'none';
			}
			root.querySelectorAll('.footer .btn').forEach(el => {
				el.style.display = 'none';
			});

			if (content) {
				content.style.width = `${width * 32}px`;
				content.style.height = `${height * 32}px`;
			}
			if (hasTabs) {
				root.querySelectorAll('.tab-content-mini').forEach(el => {
					el.style.width = `${width * 32}px`;
					el.style.height = `${height * 32}px`;
				});
			}
		} else {
			width = 17;
			height = 12;
			const extend = root.querySelector('.extend');
			if (extend) {
				extend.style.display = 'none';
			}
			const content = root.querySelector('.content');
			if (content) {
				content.style.display = 'none';
			}

			if (hasTabs) {
				const checkedMini = root.querySelector('.tab-switch-mini:checked');
				if (checkedMini) {
					const tabId = checkedMini.id;
					const i = parseInt(tabId.split('-')[1], 10);
					const bigRadio = root.querySelector(`#tab-${i}`);
					if (bigRadio) {
						bigRadio.checked = true;
					}
				}
			}

			const contentbig = root.querySelector('.contentbig');
			if (contentbig) {
				contentbig.style.display = '';
				contentbig.style.width = `${width * 32}px`;
				contentbig.style.height = `${height * 32}px`;
			}
			root.querySelectorAll('.footer .btn').forEach(el => {
				el.style.display = 'block';
			});
		}
	}

	function onMini(comp) {
		_preferences.mini = !_preferences.mini;
		_preferences.save();
		resize(comp, _preferences.width, _preferences.height);
	}

	function onApplyChoice(comp) {
		const order = createSkillUpgradeOrder({
			plan: rememberChoice,
			ownedSkills: hasSkills,
			skillInfo: SkillInfo,
			skillTreeView: SkillTreeView,
			jobId: skillJobId,
			availablePoints: _points
		});
		if (!order) {
			onResetChoice(comp);
			return;
		}

		order.forEach(skillId => Component.onIncreaseSkill(skillId));
		onResetChoice(comp);
	}

	function onResetChoice(comp) {
		const root = comp.getRoot();
		rememberChoice.forEach((_choice, skillId) => {
			const skill = hasSkills[skillId];
			const level = skill?.level ?? 0;

			root.querySelectorAll(`.skill.id${skillId}`).forEach(element => {
				element.classList.remove('active', 'passive', 'disabled');
				element.classList.add(level ? (skill?.type ? 'active' : 'passive') : 'disabled');

				const selectable = element.querySelector('.selectable');
				if (selectable) {
					selectable.style.display = '';
				}
				element.querySelectorAll('.current, .max').forEach(value => {
					value.textContent = level;
				});

				const levelEl = element.querySelector('.level');
				if (levelEl) {
					levelEl.style.display =
						!level && element.parentElement?.classList.contains('skillCol') ? 'none' : '';
				}
			});
		});
		totalCounter = 0;
		const skpointsEl = root.querySelector('.skpoints_count');
		if (skpointsEl) {
			skpointsEl.textContent = _points;
		}
		rememberChoice = new Map();
	}

	function onNecessarySkills(target, root) {
		let main = target.parentElement;
		if (!main.classList.contains('skill')) {
			main = main.parentElement;
		}
		const skillId = parseInt(main.getAttribute('data-index'), 10);
		const requirements = new Map();
		collectNecessarySkills(skillId, requirements);
		highlightNecessarySkill(skillId, null, root);
		requirements.forEach((level, requiredSkillId) => {
			highlightNecessarySkill(requiredSkillId, level, root);
		});
	}

	function _resolveSkillID(el) {
		let main = el.parentElement;
		if (!main.classList.contains('skill')) {
			main = main.parentElement;
		}
		const id = parseInt(main.getAttribute('data-index'), 10);
		const skill = getSkillById(id);
		return skill?.SKID ?? id;
	}

	function onSkillTouchStart(event, iconEl) {
		const touch = event.touches[0];
		const skillDiv = iconEl.closest('.skill');
		const index = parseInt(skillDiv.getAttribute('data-index'), 10);
		const skill = getSkillById(index);

		if (!skill || !skill.level || !skill.type) {
			return;
		}

		_touchDrag.startX = touch.pageX;
		_touchDrag.startY = touch.pageY;
		_touchDrag.ghost = null;
		_touchDrag.dragging = false;

		_touchDrag.timer = setTimeout(() => {
			_touchDrag.dragging = true;

			const ghost = iconEl.cloneNode(true);
			ghost.classList.add('drag-ghost');
			ghost.style.position = 'absolute';
			ghost.style.zIndex = '10000';
			ghost.style.left = `${touch.pageX - 12}px`;
			ghost.style.top = `${touch.pageY - 12}px`;
			ghost.style.opacity = '0.8';
			ghost.style.pointerEvents = 'none';
			document.body.appendChild(ghost);
			_touchDrag.ghost = ghost;

			window._OBJ_DRAG_ = {
				type: 'skill',
				from: _dragFrom,
				data: skill
			};
		}, 300);
	}

	function onSkillTouchMove(event) {
		if (!_touchDrag.timer && !_touchDrag.dragging) {
			return;
		}

		const touch = event.touches[0];

		if (_touchDrag.dragging) {
			event.preventDefault();
			if (_touchDrag.ghost) {
				_touchDrag.ghost.style.left = `${touch.pageX - 12}px`;
				_touchDrag.ghost.style.top = `${touch.pageY - 12}px`;
			}
		} else {
			const dx = touch.pageX - _touchDrag.startX;
			const dy = touch.pageY - _touchDrag.startY;
			if (dx * dx + dy * dy > 100) {
				clearTimeout(_touchDrag.timer);
				_touchDrag.timer = null;
			}
		}
	}

	function onSkillTouchEnd(event) {
		if (_touchDrag.timer) {
			clearTimeout(_touchDrag.timer);
			_touchDrag.timer = null;
		}

		if (_touchDrag.dragging) {
			_touchDrag.dragging = false;

			if (_touchDrag.ghost) {
				_touchDrag.ghost.remove();
				_touchDrag.ghost = null;
			}

			const touch = event.changedTouches[0];
			const target = document.elementFromPoint(touch.clientX, touch.clientY);

			if (target) {
				const dropTarget = target.closest('.container');
				if (dropTarget) {
					const dropEvent = new Event('drop', { bubbles: true });
					dropEvent.dataTransfer = {
						getData(type) {
							if (type === 'Text') {
								return JSON.stringify(window._OBJ_DRAG_);
							}
							return '';
						}
					};
					dropTarget.dispatchEvent(dropEvent);
				}
			}

			delete window._OBJ_DRAG_;
		}
	}

	function skillLevelSelectUp(skill, root) {
		const level = skill.selectedLevel ? skill.selectedLevel : skill.level;
		if (level < skill.level) {
			skill.selectedLevel = level + 1;
			const elements = root.querySelectorAll(`.skill.id${skill.SKID}`);
			elements.forEach(element => {
				const current = element.querySelector('.level .current');
				if (current) {
					current.textContent = skill.selectedLevel;
				}
			});
		}
	}

	function skillLevelSelectDown(skill, root) {
		const level = skill.selectedLevel ? skill.selectedLevel : skill.level;
		if (level > 1) {
			skill.selectedLevel = level - 1;
			const elements = root.querySelectorAll(`.skill.id${skill.SKID}`);
			elements.forEach(element => {
				const current = element.querySelector('.level .current');
				if (current) {
					current.textContent = skill.selectedLevel;
				}
			});
		}
	}

	Component.onUseSkill = function onUseItem() {};
	Component.onIncreaseSkill = function onIncreaseSkill() {};
	Component.onUpdateSkill = function onUpdateSkill() {};
	Component.getSkillById = getSkillById;

	return UIManager.addComponent(Component);
}
